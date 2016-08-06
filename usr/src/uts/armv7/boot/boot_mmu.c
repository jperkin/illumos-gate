/*
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 */

/*
 * Copyright (c) 2013 Joyent, Inc.  All rights reserved.
 */

#include <sys/param.h>
#include <sys/bootconf.h>
#include <sys/atag.h>
#include <sys/elf.h>
#include <sys/systm.h>
#include <sys/machflush.h>
#include <vm/pte.h>

/*
 * Routines for mainpulating page tables during fakebop.
 */

/* Root of the boot page table and the L2 page table arenas */
static armpte_t *armboot_pt;
static uintptr_t armboot_pt_arena;
static uintptr_t armboot_pt_arena_max;

/* Include debug messages */
static int armboot_map_debug = 0;

/*
 * Get the root of the page table the loader gave us. Go through and remove the
 * mappings from the loader so we can reuse that physical memory.
 */
void
armboot_mmu_init(atag_header_t *chain)
{
	atag_illumos_status_t *aisp;

	aisp = (atag_illumos_status_t *)atag_find(chain, ATAG_ILLUMOS_STATUS);
	if (aisp == NULL)
		bop_panic("missing ATAG_ILLUMOS_STATUS!\n");
	armboot_pt = (armpte_t *)(uintptr_t)aisp->ais_ptbase;
	armboot_pt_arena = aisp->ais_pt_arena;
	armboot_pt_arena_max = aisp->ais_pt_arena_max;
	if (armboot_map_debug)
		bop_printf(NULL,
		    "root page table at %p\nl2pt arena from %p->%p\n",
		    armboot_pt, armboot_pt_arena, armboot_pt_arena_max);
	/* XXX Unmap the loader region */
}

static void
armboot_mmu_map_4k(armpte_t *l2table, uintptr_t pa, uintptr_t va, int prot)
{
	int entry;
	armpte_t *pte;
	armpte_t ptt;
	arm_l2e_t *l2pte;

	entry = ARMPT_VADDR_TO_L2E(va);
	pte = &l2table[entry];

	if ((*pte & ARMPT_L2_TYPE_MASK) != ARMPT_L2_TYPE_INVALID)
		bop_panic("asked to remap a valid 4k page!");

	if (armboot_map_debug) {
		bop_printf(NULL, "mapping 4k page from (p->v) %p->%p\n",
		    pa, va);
		bop_printf(NULL, "l2pt root: %p, offset: %d\n", l2table, entry);
	}

	ptt = 0;
	l2pte = (arm_l2e_t *)&ptt;
	if (!(prot & PF_X))
		l2pte->ale_xn = 1;
	l2pte->ale_ident = 1;
	/* XXX Assume it's not device memory */
	l2pte->ale_bbit = 1;
	l2pte->ale_cbit = 1;
	l2pte->ale_sbit = 1;
	l2pte->ale_ap = ARMPT_AP_PRIV;
	l2pte->ale_ap2 = ARMPT_AP2_RO;
	/* XXX: Permissions fault otherwise */
	//if (prot & PF_W)
		l2pte->ale_ap2 = ARMPT_AP2_RW;
	l2pte->ale_ngbit = 0;
	l2pte->ale_addr = ARMPT_PADDR_TO_L2ADDR(pa);

	*pte = ptt;
}

static void
armboot_mmu_map_1mb(uintptr_t pa, uintptr_t va, int prot)
{
	int entry;
	armpte_t *pte;
	armpte_t ptt;
	arm_l1s_t *l1e;

	entry = ARMPT_VADDR_TO_L1E(va);
	pte = &armboot_pt[entry];
	if (ARMPT_L1E_ISVALID(*pte))
		bop_panic("armboot_mmu: asked to map a mapped region!\n");

	if (armboot_map_debug) {
		bop_printf(NULL, "mapping 1MB page from (p->v) %p->%p\n",
		    pa, va);
	}

	ptt = 0;
	l1e = (arm_l1s_t *)&ptt;
	l1e->al_type = ARMPT_L1_TYPE_SECT;
	/* XXX Assume it's not device memory */
	l1e->al_bbit = 1;
	l1e->al_cbit = 1;
	l1e->al_sbit = 1;

	if (!(prot & PF_X))
		l1e->al_xn = 1;
	l1e->al_domain = 0;

	l1e->al_ap = ARMPT_AP_PRIV;
	l1e->al_ap2 = ARMPT_AP2_RO;
	if (prot & PF_W)
		l1e->al_ap2 = ARMPT_AP2_RW;
	l1e->al_ngbit = 0;
	l1e->al_issuper = 0;
	l1e->al_addr = ARMPT_PADDR_TO_L1SECT(pa);
	/* Now that we've set this up, assign it atomically */
	/* XXX tlb/cache maintenance? */
	*pte = ptt;
}

static uintptr_t
armboot_mmu_l2pt_alloc(void)
{
	uintptr_t ret;

	if (armboot_pt_arena & ARMPT_L2_MASK) {
		ret = armboot_pt_arena;
		ret &= ~ARMPT_L2_MASK;
		ret += ARMPT_L2_SIZE;
		armboot_pt_arena = ret + ARMPT_L2_SIZE;
	} else {
		ret = armboot_pt_arena;
		armboot_pt_arena = ret + ARMPT_L2_SIZE;
	}
	if (armboot_pt_arena >= armboot_pt_arena_max) {
		bop_panic("ran out of l2 page tables!");
	}

	if (armboot_map_debug)
		bop_printf(NULL, "allocating l2pt at %p\n", ret);

	bzero((void *)ret, ARMPT_L2_SIZE);
	return (ret);
}


void
armboot_mmu_map(uintptr_t pa, uintptr_t va, size_t len, int prot)
{
	int entry;
	armpte_t *pte, *l2table;
	armpte_t ptt;
	arm_l1pt_t *l1pt;

	if (pa & MMU_PAGEOFFSET)
		bop_panic("armboot_mmu_map: pa should be 4k aligned\n");
	if (va & MMU_PAGEOFFSET)
		bop_panic("armboot_mmu_map: va should be 4k aligned\n");
	if (len & MMU_PAGEOFFSET)
		bop_panic("armboot_mmu_map: len should be 4k aligned\n");

	if (armboot_map_debug)
		bop_printf(NULL, "mapping 0x%x->0x%x, len 0x%x\n", va, pa, len);

	while (len > 0) {
		/* Can we map a 1 MB page? */
		if (!(pa & MMU_PAGEOFFSET1M) && !(va & MMU_PAGEOFFSET1M) &&
		    !(len & MMU_PAGEOFFSET1M)) {
			armboot_mmu_map_1mb(pa, va, prot);
			len -= MMU_PAGESIZE1M;
			pa += MMU_PAGESIZE1M;
			va += MMU_PAGESIZE1M;
			continue;
		}

		/* It's time to 4k page it up */
		entry = ARMPT_VADDR_TO_L1E(va);
		pte = &armboot_pt[entry];

		if (!(ARMPT_L1E_ISVALID(*pte))) {
			l2table = (armpte_t *)armboot_mmu_l2pt_alloc();
			ptt = 0;
			l1pt = (arm_l1pt_t *)&ptt;
			l1pt->al_type = ARMPT_L1_TYPE_L2PT;
			l1pt->al_ptaddr =
			    ARMPT_ADDR_TO_L1PTADDR((uintptr_t)l2table);
			*pte = ptt;

			if (armboot_map_debug)
				bop_printf(NULL, "set up l2pt at entry 0x%x\n",
				    entry);

		} else if ((*pte & ARMPT_L1_TYPE_MASK) != ARMPT_L1_TYPE_L2PT) {
			bop_panic("expected l2 table, but found a l1 entry\n");
		} else {
			l1pt = (arm_l1pt_t *)pte;
			l2table = (armpte_t *)(l1pt->al_ptaddr <<
			    ARMPT_L1PT_TO_L2_SHIFT);
		}
		armboot_mmu_map_4k(l2table, pa, va, prot);
		len -= MMU_PAGESIZE;
		pa += MMU_PAGESIZE;
		va += MMU_PAGESIZE;
	}

	armv7_tlb_sync();
}
