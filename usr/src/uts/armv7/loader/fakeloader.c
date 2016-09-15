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
 * Copyright (c) 2014 Joyent, Inc.  All rights reserved.
 * Copyright (c) 2015 Josef 'Jeff' Sipek <jeffpc@josefsipek.net>
 */

#include "fakeloader.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/elf.h>
#include <sys/atag.h>
#include <sys/sysmacros.h>
#include <sys/machparam.h>

#include <vm/pte.h>

/*
 * This is the stock ARM fake uniboot loader.
 *
 * Here's what we have to do:
 *   o Read the atag header and find the combined archive header
 *   o Determine the set of mappings we need to add for the following:
 *   		- unix
 *   		- boot_archive
 *   		- atags
 *   o Enable unaligned access
 *   o Enable the caches + virtual memory
 *
 * There are several important constraints that we have here:
 *
 *   o We cannot use any .data! Several loaders that come before us are broken
 *     and only provide us with the ability to map our .text and potentially our
 *     .bss. We should strive to avoid even that if we can.
 */

#ifdef	DEBUG
#define	FAKELOAD_DPRINTF(x)	fakeload_puts(x)
#else
#define	FAKELOAD_DPRINTF(x)
#endif 	/* DEBUG */

/*
 * XXX ASSUMES WE HAVE Free memory following the boot archive
 */
static uintptr_t freemem;
static uintptr_t pt_arena;
static uintptr_t pt_arena_max;
static uint32_t *pt_addr;
static int nl2pages;

/* Simple copy routines */
void
bcopy(const void *s, void *d, size_t n)
{
	const char *src = s;
	char *dest = d;

	if (n == 0 || s == d)
		return;

	if (dest < src && dest + n < src) {
		/* dest overlaps with the start of src, copy forward */
		for (; n > 0; n--, src++, dest++)
			*dest = *src;
	} else {
		/* src overlaps with start of dest or no overlap, copy rev */
		src += n - 1;
		dest += n - 1;
		for (; n > 0; n--, src--, dest--)
			*dest = *src;
	}
}

void
bzero(void *s, size_t n)
{
	char *c = s;
	while (n > 0) {
		*c = 0;
		c++;
		n--;
	}
}

static void
fakeload_puts(const char *str)
{
	while (*str != '\0') {
		fakeload_backend_putc(*str);
		str++;
	}
}

static void
fakeload_panic(const char *reason)
{
	fakeload_puts("panic!\n");
	fakeload_puts(reason);
	fakeload_puts("\n");
	fakeload_puts("spinning forever... goodbye...\n");
	for (;;)
		;
}

static void
fakeload_ultostr(unsigned long value)
{
	char buf[16];
	ulong_t t, val = (ulong_t)value;
	char c;
	char *ptr = &(buf[14]);
	buf[15] = '\0';

	do {
		c = (char)('0' + val - 16 * (t = (val >> 4)));
		if (c > '9')
			c += 'A' - '9' - 1;
		*--ptr = c;
	} while ((val = t) != 0);

	*--ptr = 'x';
	*--ptr = '0';
	fakeload_puts(ptr);
}

static void
fakeload_selfmap(atag_header_t *chain)
{
	atag_illumos_mapping_t aim;

	aim.aim_header.ah_size = ATAG_ILLUMOS_MAPPING_SIZE;
	aim.aim_header.ah_tag = ATAG_ILLUMOS_MAPPING;
	aim.aim_paddr = 0x7000;
	aim.aim_vaddr = aim.aim_paddr;
	aim.aim_plen = 0x3000;
	aim.aim_vlen = aim.aim_plen;
	aim.aim_mapflags = PF_R | PF_X | PF_LOADER;
	atag_append(chain, &aim.aim_header);
}

static void
fakeload_map_1mb(uintptr_t pa, uintptr_t va, int prot)
{
	int entry;
	armpte_t *pte;
	arm_l1s_t *l1e;

	entry = ARMPT_VADDR_TO_L1E(va);
	pte = &pt_addr[entry];
	if (ARMPT_L1E_ISVALID(*pte))
		fakeload_panic("armboot_mmu: asked to map a mapped region!\n");
	l1e = (arm_l1s_t *)pte;
	*pte = 0;
	l1e->al_type = ARMPT_L1_TYPE_SECT;

	if (prot & PF_DEVICE) {
		l1e->al_bbit = 1;
		l1e->al_cbit = 0;
		l1e->al_tex = 0;
		l1e->al_sbit = 1;
	} else {
		l1e->al_bbit = 1;
		l1e->al_cbit = 1;
		l1e->al_tex = 1;
		l1e->al_sbit = 1;
	}

	if (!(prot & PF_X))
		l1e->al_xn = 1;
	l1e->al_domain = 0;

	if (prot & PF_W) {
		l1e->al_ap2 = 1;
		l1e->al_ap = 1;
	} else {
		l1e->al_ap2 = 0;
		l1e->al_ap = 1;
	}
	l1e->al_ngbit = 0;
	l1e->al_issuper = 0;
	l1e->al_addr = ARMPT_PADDR_TO_L1SECT(pa);
}

/*
 * Set freemem to be 1 MB aligned at the end of boot archive. While the L1 Page
 * table only needs to be 16 KB aligned, we opt for 1 MB alignment so that way
 * we can map it and all the other L2 page tables we might need. If we don't do
 * this, it'll become problematic for unix to actually modify this.
 */
static void
fakeload_pt_arena_init(const atag_initrd_t *aii)
{
	int entry, i;
	armpte_t *pte;
	arm_l1s_t *l1e;

	pt_arena = aii->ai_start + aii->ai_size;
	if (pt_arena & MMU_PAGEOFFSET1M) {
		pt_arena &= MMU_PAGEMASK1M;
		pt_arena += MMU_PAGESIZE1M;
	}
	pt_arena_max = pt_arena + 4 * MMU_PAGESIZE1M;
	freemem = pt_arena_max;

	/* Set up the l1 page table by first invalidating it */
	pt_addr = (armpte_t *)pt_arena;
	pt_arena += ARMPT_L1_SIZE;
	bzero(pt_addr, ARMPT_L1_SIZE);
	for (i = 0; i < 4; i++)
		fakeload_map_1mb((uintptr_t)pt_addr + i * MMU_PAGESIZE1M,
		    (uintptr_t)pt_addr + i * MMU_PAGESIZE1M,
		    PF_R | PF_W);
}

/*
 * This is our generally entry point. We're passed in the entry point of the
 * header.
 */
static uintptr_t
fakeload_archive_mappings(atag_header_t *chain, const void *addr,
    atag_illumos_status_t *aisp)
{
	atag_illumos_mapping_t aim;
	fakeloader_hdr_t *hdr;
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *phdr;
	int nhdrs, i;
	uintptr_t ret;
	uintptr_t text = 0, data = 0;
	size_t textln = 0, dataln = 0;

	hdr = (fakeloader_hdr_t *)addr;

	if (hdr->fh_magic[0] != FH_MAGIC0)
		fakeload_panic("fh_magic[0] is wrong!\n");
	if (hdr->fh_magic[1] != FH_MAGIC1)
		fakeload_panic("fh_magic[1] is wrong!\n");
	if (hdr->fh_magic[2] != FH_MAGIC2)
		fakeload_panic("fh_magic[2] is wrong!\n");
	if (hdr->fh_magic[3] != FH_MAGIC3)
		fakeload_panic("fh_magic[3] is wrong!\n");

	if (hdr->fh_unix_size == 0)
		fakeload_panic("hdr unix size is zero\n");
	if (hdr->fh_unix_offset == 0)
		fakeload_panic("hdr unix offset is zero\n");
	if (hdr->fh_archive_size == 0)
		fakeload_panic("hdr archive size is zero\n");
	if (hdr->fh_archive_offset == 0)
		fakeload_panic("hdr archive_offset is zero\n");

	ehdr = (Elf32_Ehdr *)((uintptr_t)addr + hdr->fh_unix_offset);

	if (ehdr->e_ident[EI_MAG0] != ELFMAG0)
		fakeload_panic("magic[0] wrong");
	if (ehdr->e_ident[EI_MAG1] != ELFMAG1)
		fakeload_panic("magic[1] wrong");
	if (ehdr->e_ident[EI_MAG2] != ELFMAG2)
		fakeload_panic("magic[2] wrong");
	if (ehdr->e_ident[EI_MAG3] != ELFMAG3)
		fakeload_panic("magic[3] wrong");
	if (ehdr->e_ident[EI_CLASS] != ELFCLASS32)
		fakeload_panic("wrong elfclass");
	if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB)
		fakeload_panic("wrong encoding");
	if (ehdr->e_ident[EI_OSABI] != ELFOSABI_SOLARIS)
		fakeload_panic("wrong os abi");
	if (ehdr->e_ident[EI_ABIVERSION] != EAV_SUNW_CURRENT)
		fakeload_panic("wrong abi version");
	if (ehdr->e_type != ET_EXEC)
		fakeload_panic("unix is not an executable");
	if (ehdr->e_machine != EM_ARM)
		fakeload_panic("unix is not an ARM Executible");
	if (ehdr->e_version != EV_CURRENT)
		fakeload_panic("wrong version");
	if (ehdr->e_phnum == 0)
		fakeload_panic("no program headers");
	ret = ehdr->e_entry;

	FAKELOAD_DPRINTF("validated unix's headers\n");

	nhdrs = ehdr->e_phnum;
	phdr = (Elf32_Phdr *)((uintptr_t)addr + hdr->fh_unix_offset +
	    ehdr->e_phoff);
	for (i = 0; i < nhdrs; i++, phdr++) {
		if (phdr->p_type != PT_LOAD) {
			fakeload_puts("skipping non-PT_LOAD header\n");
			continue;
		}

		if (phdr->p_filesz == 0 || phdr->p_memsz == 0) {
			fakeload_puts("skipping PT_LOAD with 0 file/mem\n");
			continue;
		}

		/*
		 * Create a mapping record for this in the atags.
		 */
		aim.aim_header.ah_size = ATAG_ILLUMOS_MAPPING_SIZE;
		aim.aim_header.ah_tag = ATAG_ILLUMOS_MAPPING;
		aim.aim_paddr = (uintptr_t)addr + hdr->fh_unix_offset +
		    phdr->p_offset;
		aim.aim_plen = phdr->p_filesz;
		aim.aim_vaddr = phdr->p_vaddr;
		aim.aim_vlen = phdr->p_memsz;
		/* Round up vlen to be a multiple of 4k */
		if (aim.aim_vlen & 0xfff) {
			aim.aim_vlen &= ~0xfff;
			aim.aim_vlen += 0x1000;
		}
		aim.aim_mapflags = phdr->p_flags;
		atag_append(chain, &aim.aim_header);

		/*
		 * When built with highvecs we need to account for the fact that
		 * _edata, _etext and _end are built assuming that the highvecs
		 * are normally part of our segments. ld is not doing anything
		 * wrong, but this breaks the assumptions that krtld currently
		 * has. As such, unix will use this information to overwrite the
		 * normal entry points that krtld uses in a similar style to
		 * SPARC.
		 */
		if (aim.aim_vaddr != 0xffff0000) {
			if ((phdr->p_flags & PF_W) != 0) {
				data = aim.aim_vaddr;
				dataln = aim.aim_vlen;
			} else {
				text = aim.aim_vaddr;
				textln = aim.aim_vlen;
			}
		}
	}

	aisp->ais_stext = text;
	aisp->ais_etext = text + textln;
	aisp->ais_sdata = data;
	aisp->ais_edata = data + dataln;

	/* 1:1 map the boot archive */
	aim.aim_header.ah_size = ATAG_ILLUMOS_MAPPING_SIZE;
	aim.aim_header.ah_tag = ATAG_ILLUMOS_MAPPING;
	aim.aim_paddr = (uintptr_t)addr + hdr->fh_archive_offset;
	aim.aim_plen = hdr->fh_archive_size;
	aim.aim_vaddr = aim.aim_paddr;
	aim.aim_vlen = aim.aim_plen;
	aim.aim_mapflags = PF_R | PF_W | PF_X;
	atag_append(chain, &aim.aim_header);
	aisp->ais_archive = aim.aim_paddr;
	aisp->ais_archivelen = aim.aim_plen;

	return (ret);
}

static void
fakeload_mkatags(atag_header_t *chain)
{
	atag_illumos_status_t ais;
	atag_illumos_mapping_t aim;

	bzero(&ais, sizeof (ais));
	bzero(&aim, sizeof (aim));

	ais.ais_header.ah_size = ATAG_ILLUMOS_STATUS_SIZE;
	ais.ais_header.ah_tag = ATAG_ILLUMOS_STATUS;
	atag_append(chain, &ais.ais_header);
	aim.aim_header.ah_size = ATAG_ILLUMOS_MAPPING_SIZE;
	aim.aim_header.ah_tag = ATAG_ILLUMOS_MAPPING;
	atag_append(chain, &aim.aim_header);
}

static uintptr_t
fakeload_alloc_l2pt(void)
{
	uintptr_t ret;

	if (pt_arena & ARMPT_L2_MASK) {
		ret = pt_arena;
		ret &= ~ARMPT_L2_MASK;
		ret += ARMPT_L2_SIZE;
		pt_arena = ret + ARMPT_L2_SIZE;
	} else {
		ret = pt_arena;
		pt_arena = ret + ARMPT_L2_SIZE;
	}
	if (pt_arena >= pt_arena_max) {
		fakeload_puts("pt_arena, max\n");
		fakeload_ultostr(pt_arena);
		fakeload_puts("\n");
		fakeload_ultostr(pt_arena_max);
		fakeload_puts("\n");
		fakeload_puts("l2pts alloced\n");
		fakeload_ultostr(nl2pages);
		fakeload_puts("\n");
		fakeload_panic("ran out of page tables!");
	}

	bzero((void *)ret, ARMPT_L2_SIZE);
	nl2pages++;
	return (ret);
}

/*
 * Finally, do all the dirty work. Let's create some page tables. The L1 page
 * table is full of 1 MB mappings by default. The L2 Page table is 1k in size
 * and covers that 1 MB. We're going to always create L2 page tables for now
 * which will use 4k and 64k pages.
 */
static void
fakeload_map(armpte_t *pt, uintptr_t pstart, uintptr_t vstart, size_t len,
    uint32_t prot)
{
	int entry, chunksize;
	armpte_t *pte, *l2pt;
	arm_l1pt_t *l1pt;

	/*
	 * Make sure both pstart + vstart are 4k aligned, along with len.
	 */
	if (pstart & MMU_PAGEOFFSET)
		fakeload_panic("pstart is not 4k aligned");
	if (vstart & MMU_PAGEOFFSET)
		fakeload_panic("vstart is not 4k aligned");
	if (len & MMU_PAGEOFFSET)
		fakeload_panic("len is not 4k aligned");

	/*
	 * We're going to logically deal with each 1 MB chunk at a time.
	 */
	while (len > 0) {
		if (vstart & MMU_PAGEOFFSET1M) {
			chunksize = MIN(len, MMU_PAGESIZE1M -
			    (vstart & MMU_PAGEOFFSET1M));
		} else {
			chunksize = MIN(len, MMU_PAGESIZE1M);
		}

		entry = ARMPT_VADDR_TO_L1E(vstart);
		pte = &pt[entry];

		if (!ARMPT_L1E_ISVALID(*pte)) {
			uintptr_t l2table;

			if (!(vstart & MMU_PAGEOFFSET1M) &&
			    !(pstart & MMU_PAGEOFFSET1M) &&
			    len >= MMU_PAGESIZE1M) {
				fakeload_map_1mb(pstart, vstart, prot);
				vstart += MMU_PAGESIZE1M;
				pstart += MMU_PAGESIZE1M;
				len -= MMU_PAGESIZE1M;
				continue;
			}

			l2table = fakeload_alloc_l2pt();
			*pte = 0;
			l1pt = (arm_l1pt_t *)pte;
			l1pt->al_type = ARMPT_L1_TYPE_L2PT;
			l1pt->al_ptaddr = ARMPT_ADDR_TO_L1PTADDR(l2table);
		} else if ((*pte & ARMPT_L1_TYPE_MASK) != ARMPT_L1_TYPE_L2PT) {
			fakeload_panic("encountered l1 entry that's not a "
			    "pointer to a level 2 table\n");
		} else {
			l1pt = (arm_l1pt_t *)pte;
		}

		/* Now that we have the l1pt fill in l2 entries */
		l2pt = (void *)(l1pt->al_ptaddr << ARMPT_L1PT_TO_L2_SHIFT);
		len -= chunksize;
		while (chunksize > 0) {
			arm_l2e_t *l2pte;

			entry = ARMPT_VADDR_TO_L2E(vstart);
			pte = &l2pt[entry];

#ifdef	MAP_DEBUG
			fakeload_puts("4k page pa->va, l2root, entry\n");
			fakeload_ultostr(pstart);
			fakeload_puts("->");
			fakeload_ultostr(vstart);
			fakeload_puts(", ");
			fakeload_ultostr((uintptr_t)l2pt);
			fakeload_puts(", ");
			fakeload_ultostr(entry);
			fakeload_puts("\n");
#endif

			if ((*pte & ARMPT_L2_TYPE_MASK) !=
			    ARMPT_L2_TYPE_INVALID)
				fakeload_panic("found existing l2 page table, "
				    "overlap in requested mappings detected!");
			/* Map vaddr to our paddr! */
			l2pte = ((arm_l2e_t *)pte);
			*pte = 0;
			if (!(prot & PF_X))
				l2pte->ale_xn = 1;
			l2pte->ale_ident = 1;
			if (prot & PF_DEVICE) {
				l2pte->ale_bbit = 1;
				l2pte->ale_cbit = 0;
				l2pte->ale_tex = 0;
				l2pte->ale_sbit = 1;
			} else {
				l2pte->ale_bbit = 1;
				l2pte->ale_cbit = 1;
				l2pte->ale_tex = 1;
				l2pte->ale_sbit = 1;
			}
			if (prot & PF_W) {
				l2pte->ale_ap2 = 1;
				l2pte->ale_ap = 1;
			} else {
				l2pte->ale_ap2 = 0;
				l2pte->ale_ap = 1;
			}
			l2pte->ale_ngbit = 0;
			l2pte->ale_addr = ARMPT_PADDR_TO_L2ADDR(pstart);

			chunksize -= MMU_PAGESIZE;
			vstart += MMU_PAGESIZE;
			pstart += MMU_PAGESIZE;
		}
	}
}

static void
fakeload_create_map(armpte_t *pt, atag_illumos_mapping_t *aimp)
{
#ifdef MAP_DEBUG
	fakeload_puts("paddr->vaddr\n");
	fakeload_ultostr(aimp->aim_paddr);
	fakeload_puts("->");
	fakeload_ultostr(aimp->aim_vaddr);
	fakeload_puts("\n");
	fakeload_puts("plen-vlen\n");
	fakeload_ultostr(aimp->aim_plen);
	fakeload_puts("-");
	fakeload_ultostr(aimp->aim_vlen);
	fakeload_puts("\n");
#endif /* MAP_DEBUG */

	/*
	 * Can we map this in place or do we need to basically allocate a new
	 * region and bcopy everything into place for proper alignment?
	 *
	 * Criteria for this: we have a vlen > plen. plen is not page aligned.
	 */
	if (aimp->aim_vlen > aimp->aim_plen ||
	    (aimp->aim_paddr & MMU_PAGEOFFSET) != 0) {
		uintptr_t start;

		if (aimp->aim_mapflags & PF_NORELOC)
			fakeload_panic("tried to reloc unrelocatable mapping");
#ifdef	MAP_DEBUG
		FAKELOAD_DPRINTF("reloacting paddr\n");
#endif
		start = freemem;
		if (start & MMU_PAGEOFFSET) {
			start &= MMU_PAGEMASK;
			start += MMU_PAGESIZE;
		}
		bcopy((void *)aimp->aim_paddr, (void *)start,
		    aimp->aim_plen);
		if (aimp->aim_vlen > aimp->aim_plen) {
			bzero((void *)(start + aimp->aim_plen),
			    aimp->aim_vlen - aimp->aim_plen);
		}
		aimp->aim_paddr = start;
		freemem = start + aimp->aim_vlen;
#ifdef MAP_DEBUG
		fakeload_puts("new paddr: ");
		fakeload_ultostr(start);
		fakeload_puts("\n");
#endif /* MAP_DEBUG */
	}

	/*
	 * Now that everything has been set up, go ahead and map the new region.
	 */
	fakeload_map(pt, aimp->aim_paddr, aimp->aim_vaddr, aimp->aim_vlen,
	    aimp->aim_mapflags);
#ifdef MAP_DEBUG
	FAKELOAD_DPRINTF("\n");
#endif /* MAP_DEBUG */
}

void
fakeload_init(void *ident, void *ident2, void *atag)
{
	atag_header_t *hdr;
	atag_header_t *chain = (atag_header_t *)atag;
	const atag_initrd_t *initrd;
	atag_illumos_status_t *aisp;
	atag_illumos_mapping_t *aimp;
	uintptr_t unix_start;

	fakeload_backend_init();
	fakeload_puts("Hello from the loader\n");
	fakeload_leave_hyp();

	initrd = (atag_initrd_t *)atag_find(chain, ATAG_INITRD2);
	if (initrd == NULL)
		fakeload_panic("missing the initial ramdisk\n");

	/*
	 * Create the status atag header and the initial mapping record for the
	 * atags. We'll hold onto both of these.
	 */
	fakeload_mkatags(chain);
	aisp = (atag_illumos_status_t *)atag_find(chain, ATAG_ILLUMOS_STATUS);
	if (aisp == NULL)
		fakeload_panic("can't find ATAG_ILLUMOS_STATUS");
	aimp = (atag_illumos_mapping_t *)atag_find(chain, ATAG_ILLUMOS_MAPPING);
	if (aimp == NULL)
		fakeload_panic("can't find ATAG_ILLUMOS_MAPPING");
	FAKELOAD_DPRINTF("created proto atags\n");

	fakeload_pt_arena_init(initrd);

	fakeload_selfmap(chain);

	/*
	 * Map the boot archive and all of unix
	 */
	unix_start = fakeload_archive_mappings(chain,
	    (const void *)(uintptr_t)initrd->ai_start, aisp);
	FAKELOAD_DPRINTF("filled out unix and the archive's mappings\n");

	/*
	 * Fill in the atag mapping header for the atags themselves. 1:1 map it.
	 */
	aimp->aim_paddr = (uintptr_t)chain & ~0xfff;
	aimp->aim_plen = atag_length(chain) & ~0xfff;
	aimp->aim_plen += 0x1000;
	aimp->aim_vaddr = aimp->aim_paddr;
	aimp->aim_vlen = aimp->aim_plen;
	aimp->aim_mapflags = PF_R | PF_W | PF_NORELOC;

	/*
	 * Let the backend add mappings
	 */
	fakeload_backend_addmaps(chain);

	/*
	 * Turn on unaligned access
	 */
	FAKELOAD_DPRINTF("turning on unaligned access\n");
	fakeload_unaligned_enable();
	FAKELOAD_DPRINTF("successfully enabled unaligned access\n");

	/*
	 * To turn on the MMU we need to do the following:
	 *  o Program all relevant CP15 registers
	 *  o Program 1st and 2nd level page tables
	 *  o Invalidate and Disable the I/D-cache
	 *  o Fill in the last bits of the ATAG_ILLUMOS_STATUS atag
	 *  o Turn on the MMU in SCTLR
	 *  o Jump to unix
	 */

	/* Last bits of the atag */
	aisp->ais_freemem = freemem;
	aisp->ais_version = 1;
	aisp->ais_ptbase = (uintptr_t)pt_addr;

	/*
	 * Our initial page table is a series of 1 MB sections. While we really
	 * should map 4k pages, for the moment we're just going to map 1 MB
	 * regions, yay team!
	 */
	hdr = chain;
	FAKELOAD_DPRINTF("creating mappings\n");
	while (hdr != NULL) {
		if (hdr->ah_tag == ATAG_ILLUMOS_MAPPING)
			fakeload_create_map(pt_addr,
			    (atag_illumos_mapping_t *)hdr);
		hdr = atag_next(hdr);
	}

	/*
	 * Now that we've mapped everything, update the status atag.
	 */
	aisp->ais_freeused = freemem - aisp->ais_freemem;
	aisp->ais_pt_arena = pt_arena;
	aisp->ais_pt_arena_max = pt_arena_max;

	/* Cache disable */
	FAKELOAD_DPRINTF("Flushing and disabling caches\n");
	armv7_dcache_flush();
	armv7_dcache_disable();
	armv7_dcache_inval();
	armv7_icache_disable();
	armv7_icache_inval();

	/* Program the page tables */
	FAKELOAD_DPRINTF("programming cp15 regs\n");
	fakeload_pt_setup((uintptr_t)pt_addr);


	/* MMU Enable */
	FAKELOAD_DPRINTF("see you on the other side\n");
	fakeload_mmu_enable();

	FAKELOAD_DPRINTF("why helo thar\n");

	/* Renable caches */
	armv7_dcache_enable();
	armv7_icache_enable();

	/* we should never come back */
	fakeload_exec(ident, ident2, chain, unix_start);
	fakeload_panic("hit the end of the world\n");
}
