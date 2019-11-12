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
 * Copyright 2019 Team Twist My ARM
 */

/*
 * aboot - ARM Boot
 *
 * This is a simple boot stub that is meant to get us off the ground. It assumes
 * that it has been loaded by something like QEMU (or u-boot) and has been given
 * a pointer to the FDT. The kernel, unix, needs to be found, and have virtual
 * addresses set up for it. We also need to get the processor into a reasonable
 * state for unix. This means we need to do the following:
 *
 * 1. Use the FDT to find the combined 'boot-archive'.
 * 2. Scan the FDT for memory nodes and make sure that the basic one is usable.
 * 3. Set up a series of mappings for the following:
 *       - unix
 *       - boot_archive
 *       - fdt
 *       - A temporary 1:1 mapping for ourselves
 *       - A struct of aboot data so unix knows what we did and can clean up
 * 4. Enable unaligned access
 * 5. Enable the caches
 * 6. Enable virtual memory
 * 7. Jump to unix
 *
 * We're going to construct the following physical layout in memory. Note that
 * the exact starting address is uncertain and the location of aboot here is
 * also uncertain:
 *
 *
 *        +--------------+ Boot Arhive End +32 MiB, freemem max
 *        |              |
 *        |              | + ? - Freemem end
 *        | Freemem      |
 *        +--------------+ + 1 MiB align, freemem start
 *        | aboot_info_t |
 *        +--------------+ + 64B align
 *        | FDT Copy     |
 *        +--------------+ + 4 MiB
 *        | L2 PT Arena  |
 *        +--------------+ +16 KiB
 *        | L1 PT Area   |
 *        +--------------+ Boot Archive End, aligned to 1 MiB
 *        +--------------+
 *        | Boot Archive |
 *        +--------------+
 *        |              |
 *        +--------------+ addr + 8 KiB
 *        | aboot binary |
 *        +--------------+ per-platform addr
 *
 */

#include <sys/types.h>
#include <aboot/aboot.h>
#include <sys/devicetree.h>
#include <sys/null.h>
#include <sys/byteorder.h>
#include <sys/sysmacros.h>
#include <sys/armv7_cpu.h>
#include <vm/pte.h>

/*
 * The build process currently needs to tell us the PA we start at and our total
 * length that the mapfiles will set. Maybe we'll find a better way to do this.
 * This is so we can create an aboot identity map.
 */
#if !defined(ABOOT_BINARY_START)
#error	"missing ABOOT_BINARY_START address"
#endif

#if !defined(ABOOT_BINARY_LENGTH)
#error	"missing ABOOT_BINARY_START address"
#endif

/*
 * Amount of memory we want to make sure that we have after the boot archive.
 */
#define	ABOOT_MEMSIZE	(32 * 1024 * 1024)
#define	ABOOT_MEGALIGN	(1024 * 1024)

#define	ABOOT_INFOALIGN	64

static fdt_t aboot_fdt;
static uintptr_t aboot_archive;
static size_t aboot_archivelen;
static uintptr_t aboot_l2pt;
static uintptr_t aboot_l2pt_max;
static armpte_t *aboot_ptroot;
static uintptr_t aboot_reloc_fdt;
static uintptr_t aboot_freemem;
aboot_info_t *aboot_infop;

static uint32_t aboot_acells;
static uint32_t aboot_scells;
static uint64_t aboot_memstart;
static uint64_t aboot_memlen;

static boolean_t aboot_debug_fdt = B_FALSE;

static void
aboot_puts(const char *s)
{
	while (*s != '\0') {
		aboot_backend_putc(*s);
		s++;
	}
}

static void
aboot_ultostr(unsigned long value)
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
	aboot_puts(ptr);
}

void
aboot_panic(const char *reason)
{
	aboot_puts("panic!\n");
	aboot_puts(reason);
	aboot_puts("\n");
	aboot_puts("May the next time be better!\n");
	for (;;) {

	}
}

static void
aboot_bzero(void *s, size_t n)
{
	char *c = s;
	while (n > 0) {
		*c = 0;
		c++;
		n--;
	}
}

static void
aboot_print_node(fdt_t *fdt, fdt_node_t *node, uint_t indent)
{
	uint_t i;
	const char *name;
	fdt_node_t *child;
	fdt_prop_t *prop;

	for (i = 0; i < indent; i++) {
		aboot_puts(" ");
	}
	name = fdt_node_name(node);
	if (*name == '\0') {
		aboot_puts("/");
	} else {
		aboot_puts(name);
	}
	aboot_puts("\n");

	for (prop = fdt_next_prop(fdt, node, NULL); prop != NULL;
	    prop = fdt_next_prop(fdt, node, prop)) {
		uint32_t len;

		name = fdt_prop_name(fdt, prop);
		if (name == NULL) {
			aboot_puts("prop has no name");
			continue;
		}

		for (i = 0; i < indent + 2; i++) {
			aboot_puts(" ");
		}
		aboot_puts("property: ");
		aboot_puts(name);

		if (fdt_prop(fdt, prop, &len, NULL)) {
			aboot_puts(", length: ");
			aboot_ultostr(len);
		}

		aboot_puts("\n");
	}

	for (child = fdt_child_node(fdt, node); child != NULL;
	    child = fdt_next_node(fdt, child)) {
		aboot_print_node(fdt, child, indent + 2);
	}
}

static void
aboot_print_fdt(fdt_t *fdt)
{
	fdt_node_t *node;
	fdt_memrsvd_t *mem;

	aboot_puts("looking at fdt\n");
	node = fdt_next_node(fdt, NULL);
	if (node != NULL) {
		aboot_print_node(fdt, node, 0);
	} else {
		aboot_puts("found no node\n");
	}

	for (mem = fdt_next_memrsvd(fdt, NULL); mem != NULL;
	    mem = fdt_next_memrsvd(fdt, mem)) {
		uint64_t addr, len;

		fdt_memrsvd(mem, &addr, &len);
		aboot_puts("reservation: ");
		aboot_ultostr(addr);
		aboot_puts("/");
		aboot_ultostr(len);
		aboot_puts(" bytes\n");
	}
}

static void
aboot_init_fdt(fdt_t *fdt)
{
	fdt_node_t *root = fdt_root_node(fdt);
	fdt_prop_t *p;
	uint32_t len;
	void *data;

	if (root == NULL) {
		aboot_panic("failed to find root node");
	}

	p = fdt_find_prop(fdt, root, "#address-cells");
	if (p == NULL) {
		aboot_panic("failed to find #address-cells property");
	}
	if (!fdt_prop(fdt, p, &len, &data) || len != sizeof (uint32_t)) {
		aboot_panic("failed to get #address-cells property");
	}
	aboot_acells = be32toh(*(uint32_t *)data);

	p = fdt_find_prop(fdt, root, "#size-cells");
	if (p == NULL) {
		aboot_panic("failed to find #size-cells property");
	}
	if (!fdt_prop(fdt, p, &len, &data) || len != sizeof (uint32_t)) {
		aboot_panic("failed to get #size-cells property");
	}
	aboot_scells = be32toh(*(uint32_t *)data);

	if (aboot_acells != 1 && aboot_acells != 2) {
		aboot_panic("invalid #address-cells value");
	}

	if (aboot_scells != 1 && aboot_scells != 2) {
		aboot_panic("invalid #address-cells value");
	}
}

static void
aboot_init_archive(fdt_t *fdt, fdt_node_t *chosen)
{
	fdt_prop_t *initrd;
	uint32_t len, start, end;
	void *data;

	initrd = fdt_find_prop(fdt, chosen, "linux,initrd-start");
	if (initrd == NULL) {
		aboot_panic("failed to find initrd start property\n");
	}

	if (!fdt_prop(fdt, initrd, &len, &data)) {
		aboot_panic("failed to get initrd start prop info\n");
	}

	if (len != sizeof (uint32_t)) {
		aboot_panic("linux,initrd-start property has wrong size");
	}
	start = be32toh(*(const uint32_t *)data);

	initrd = fdt_find_prop(fdt, chosen, "linux,initrd-end");
	if (initrd == NULL) {
		aboot_panic("failed to find initrd end property");
	}

	if (!fdt_prop(fdt, initrd, &len, &data)) {
		aboot_panic("failed to get initrd end prop info");
	}

	if (len != sizeof (uint32_t)) {
		aboot_panic("linux,initrd-end property has wrong size");
	}
	end = be32toh(*(const uint32_t *)data);

	if (start >= end) {
		aboot_panic("boot archive start/end is impossible");
	}

	aboot_archive = start;
	aboot_archivelen = end - start;
}

static void
aboot_init_memory(fdt_t *fdt, fdt_node_t *mem)
{
	fdt_prop_t *reg;
	void *data;
	const uint32_t *regp;
	uint32_t len, nranges, i, reglen;
	uint32_t targ_end;

	reg = fdt_find_prop(fdt, mem, "reg");
	if (reg == NULL) {
		aboot_panic("Can't find 'reg' property");
	}

	if (!fdt_prop(fdt, reg, &len, &data)) {
		aboot_panic("failed to get reg property data");
	}

	targ_end = aboot_archive + aboot_archivelen;
	targ_end = P2ROUNDUP(targ_end, 1024 * 1024);
	targ_end += ABOOT_MEMSIZE;

	/*
	 * We need to pick a token memory range for us to use. We want to
	 * ideally find the one with the boot archive in it, assuming that we
	 * have at least 32 MiB free after the archive. First try to find the
	 * one with the boot archive in it. If that fails, take a second pass
	 * through the range.
	 */
	reglen = aboot_acells + aboot_scells;
	nranges = len / reglen;
	for (regp = data, i = 0; i < nranges; i++, regp += reglen) {
		uint32_t off = 0;
		uint32_t alow, ahigh, slow, shigh;
		uint64_t addr, size;

		if (aboot_acells == 1) {
			addr = be32toh(regp[off]);
			off += 1;
		} else {
			ahigh = be32toh(regp[off]);
			alow = be32toh(regp[off + 1]);
			addr = ((uint64_t)ahigh << 32) + alow;
			off += 2;
		}

		if (aboot_scells == 1) {
			size = be32toh(regp[off]);
		} else {
			shigh = be32toh(regp[off]);
			slow = be32toh(regp[off + 1]);
			size = ((uint64_t)shigh << 32) + slow;
		}

		if (aboot_archive >= addr && targ_end <= (addr + size)) {
			aboot_memstart = addr;
			aboot_memlen = size;
			break;
		}
	}

	if (aboot_memstart == 0 && aboot_memlen == 0) {
		aboot_panic("failed to initialize memory");
	}
}

void
aboot_map_1mb(uintptr_t pa, uintptr_t va, aboot_prot_t prot)
{
	uint_t entry;
	armpte_t *pte;
	arm_l1s_t *l1e;

	entry = ARMPT_VADDR_TO_L1E(va);
	pte = &aboot_ptroot[entry];
	if (ARMPT_L1E_ISVALID(*pte)) {
		aboot_panic("aboot: asked to map a mapped region!\n");
	}
	*pte = 0;
	l1e = (arm_l1s_t *)pte;
	l1e->al_type = ARMPT_L1_TYPE_SECT;

	if ((prot & ABOOT_P_DEV) != 0) {
		l1e->al_bbit = 1;
		l1e->al_cbit = 0;
		l1e->al_tex = 0;
		l1e->al_sbit = 0;
	} else {
		l1e->al_bbit = 1;
		l1e->al_cbit = 1;
		l1e->al_tex = 1;
		l1e->al_sbit = 1;
	}

	if (!(prot & ABOOT_P_X))
		l1e->al_xn = 1;
	l1e->al_domain = 0;

	if (prot & ABOOT_P_W) {
		l1e->al_ap2 = 1;
		l1e->al_ap = 1;
	} else {
		l1e->al_ap2 = 0;
		l1e->al_ap = 1;
	}
	l1e->al_ngbit = 0;
	l1e->al_issuper = 0;
	l1e->al_addr = ARMPT_PADDR_TO_L1SECT(pa);

	aboot_puts("mapping PA->VA: ");
	aboot_ultostr(pa);
	aboot_puts("->");
	aboot_ultostr(va);
	aboot_puts("\n");
}

static void
aboot_init_arenas(fdt_t *fdt)
{
	uint_t i;
	uintptr_t info;

	aboot_l2pt = aboot_archive + aboot_archivelen;
	aboot_l2pt = P2ROUNDUP(aboot_l2pt, ABOOT_MEGALIGN);
	aboot_l2pt_max = aboot_l2pt + 4 * MMU_PAGESIZE1M;

	aboot_reloc_fdt = aboot_l2pt_max;
	info = aboot_reloc_fdt + fdt_size(fdt);
	info = P2ROUNDUP(info, ABOOT_INFOALIGN);
	aboot_infop = (aboot_info_t *)info;
	info += sizeof (aboot_info_t);
	aboot_freemem = P2ALIGN(info, ABOOT_MEGALIGN);

	/*
	 * We're going to use the first 16 KiB from the L2 pagetable arena to
	 * represent the root of the page table. 
	 */
	aboot_ptroot = (armpte_t *)aboot_l2pt;
	aboot_l2pt += ARMPT_L1_SIZE;
	aboot_bzero(aboot_ptroot, ARMPT_L1_SIZE);
	for (i = 0; i < 4; i++) {
		aboot_map_1mb((uintptr_t)aboot_ptroot + i * MMU_PAGESIZE1M,
		    (uintptr_t)aboot_ptroot + i * MMU_PAGESIZE1M,
		    ABOOT_P_R | ABOOT_P_W);
	}
}

/*
 * Map aboot itself in 1 MiB chunks. This needs to be an identity mapping. This
 * will be removed and the memory reclaimed by unix once it starts up. We add
 * one additional MiB here to cover out stack (though our stack should be rather
 * small).
 */
static void
aboot_map_self(void)
{
	uintptr_t addr = ABOOT_BINARY_START - ABOOT_MEGALIGN;
	size_t len = P2ROUNDUP(ABOOT_BINARY_LENGTH, ABOOT_MEGALIGN) +
	    ABOOT_MEGALIGN;

	aboot_infop->abi_aboot = addr;
	aboot_infop->abi_abootlen = len;

	while (len >= ABOOT_MEGALIGN) {
		aboot_map_1mb(addr, addr, ABOOT_P_R | ABOOT_P_W | ABOOT_P_X);
		addr += ABOOT_MEGALIGN;
		len -= ABOOT_MEGALIGN;
	}
}

/*
 * We've been asked to turn on the MMU. At this point all of the page tables
 * should be ready to go. To do this, architecturally we need to do the
 * following:
 *
 *  o Make sure any I/D caches are flushed, disabled, and invalidated.
 *  o Program the TTCBR registers with the root of the page table and such that
 *    TTCBR.N only points to TTCBR 0. As part of this, we need to make sure
 *    domain zero is in manager mode.
 *  o Re-enable the I/D caches.
 */
static void
aboot_mmu_init(void)
{
	armv7_dcache_disable();
	armv7_dcache_wbinval();
	armv7_icache_disable();
	armv7_bpcache_disable();
	armv7_icache_inval();

	/* XXX Program MMU */
	aboot_puts("Turning on the MMU\n");
	aboot_mmu_program((uintptr_t)aboot_ptroot);
	aboot_mmu_enable();
	aboot_puts("Successfully enabled the MMU\n");

	armv7_icache_enable();
	armv7_bpcache_enable();
	armv7_dcache_enable();
}

void
aboot_main(uintptr_t arg0, uintptr_t type, uintptr_t raw_fdt)
{
	fdt_node_t *mem, *chosen;
	fdt_t *fdt;

	aboot_unaligned_enable();

	fdt = &aboot_fdt;
	if (fdt_init(raw_fdt, fdt) != FDT_E_OK) {
		aboot_panic("fdt_init failed!");
	}

	if (aboot_debug_fdt) {
		aboot_print_fdt(fdt);
	}
	aboot_init_fdt(fdt);

	/*
	 * XXX Check that the root #address-cells and #size-cells are actually
	 * describing values in a single uint32_t. We're assuming this.
	 */
	chosen = fdt_find_node(fdt, "/chosen");
	if (chosen == NULL) {
		aboot_panic("no /chosen node, can't find boot_archive");
	}
	aboot_init_archive(fdt, chosen);

	mem = fdt_find_node(fdt, "/memory");
	if (mem == NULL) {
		aboot_panic("no /memory node");
	}
	aboot_init_memory(fdt, mem);
	aboot_init_arenas(fdt);

	/*
	 * XXX Map ourselves and the console and test VM.
	 */
	aboot_map_self();
	aboot_map_console();

	aboot_mmu_init();

	aboot_panic("main!");
}
