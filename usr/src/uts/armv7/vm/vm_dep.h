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
 */

#ifndef _VM_DEP_H
#define	_VM_DEP_H

/*
 * UNIX machine dependent virtual memory support for ARMv7.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/param.h>
#include <sys/memnode.h>

/*
 * Do not use GETTICK. It is only meant to be used when timesource
 * synchronization is unimportant.
 */
#define	GETTICK()	gethrtime_unscaled()

/* tick value that should be used for random values */
extern u_longlong_t randtick(void);

#define	PLCNT_SZ(ctrs_sz)	panic("plcnt_sz")

#define	PLCNT_INIT(addr) panic("plcnt_init")

#define	PLCNT_INCR(pp, mnode, mtype, szc, flags)	panic("plcnt_incr")
#define	PLCNT_DECR(pp, mnode, mtype, szc, flags)	panic("plcnt_decr")

/*
 * Macro to update page list max counts. This is a no-op on x86, not on SPARC.
 * We panic for now on ARM. It's primarily used for kcage it appears.
 */
#define	PLCNT_XFER_NORELOC(pp)		panic("plcnt_xfer_noreloc")

/*
 * Macro to modify the page list max counts when memory is added to
 * the page lists during startup (add_physmem) or during a DR operation
 * when memory is added (kphysm_add_memory_dynamic) or deleted
 * (kphysm_del_cleanup).
 */
extern void plcnt_modify_max(pfn_t, long);
#define	PLCNT_MODIFY_MAX(pfn, cnt)	plcnt_modify_max(pfn, cnt)

/*
 * These macros are used in dealing with the page counters and its candidate
 * counters. These are used as a part of coalescing our free lists.
 */

/*
 * The maximum number of memory ranges that exist in the system. Consider i86pc,
 * there we have various ranges that exist due to legacy DMA. eg. < 16 Mb, < 4
 * Gb for PCI, etc. Like sun4, this may actually just be a single number, since
 * unlike on sun4, we're not going to pretend we have a kcage.
 */
#define	MAX_MNODE_MRANGES		1
#define	MNODE_RANGE_CNT(mnode)		1
#define	MNODE_MAX_MRANGE(mnode)		(MAX_MNODE_MRANGES - 1)
#define	MTYPE_2_MRANGE(mnode, mtype)	mtype


/*
 * XXX These are strawman definitions based on the i86pc versions of the
 * page_freelists and the page_cachelists; however, unlike i86pc we only have
 * one mtype, therefore we don't bother keeping around an index for it.
 *
 * We index into the freelist by [mmu_page_sizes][colors]. We index into the
 * cachelist by [colors].
 */
extern page_t ***page_freelists;
extern page_t **page_cachelists;

#define	PAGE_FREELISTS(mnode, szc, color, mtype)	\
	(*(page_freelists[szc] + (color)))
#define	PAGE_CACHELISTS(mnode, color, mtype) \
	(page_cachelists[color])

/*
 * XXX This set of locks needs to be rethought with respect to mandatory page
 * coloring. It was taken rather naively from i86pc
 */

/*
 * There are mutexes for both the page freelist
 * and the page cachelist.  We want enough locks to make contention
 * reasonable, but not too many -- otherwise page_freelist_lock() gets
 * so expensive that it becomes the bottleneck!
 */

#define	NPC_MUTEX	16

extern kmutex_t	*fpc_mutex[NPC_MUTEX];
extern kmutex_t	*cpc_mutex[NPC_MUTEX];

#define	PC_BIN_MUTEX(mnode, bin, flags) ((flags & PG_FREE_LIST) ?	\
	&fpc_mutex[(bin) & (NPC_MUTEX - 1)][mnode] :			\
	&cpc_mutex[(bin) & (NPC_MUTEX - 1)][mnode])

#define	FPC_MUTEX(mnode, i)	(&fpc_mutex[i][mnode])
#define	CPC_MUTEX(mnode, i)	(&cpc_mutex[i][mnode])

/*
 * Memory node iterators. We may need something here related to colors, but we
 * may not. For the time being, just panic on use for ust to get back to later.
 */
#define	MEM_NODE_ITERATOR_DECL(it)	panic("mem_node_iterator_decl")
#define	MEM_NODE_ITERATOR_INIT(pfn, mnode, szc, it)	panic("mem_node_iterator_init")

/*
 * XXX Do we ever interleave memory ndoes on armv6? Probably not? Does coloring
 * come into play here?
 */
#define	HPM_COUNTERS_LIMITS(mnodes, pyysbase, physmax, first)	\
	panic("hpm_counters_list")

#define	PAGE_CTRS_WRITE_LOCK(mnode)	panic("page_ctrs_write_lock")
#define	PAGE_CTRS_WRITE_UNLOCK(mnode)	panic("page_ctrs_write_unlock")
#define	PAGE_CTRS_ADJUST(pfn, cnt, rv)	panic("page_cntrs_adjust")

/*
 * Coloring related macros. For more on coloring, see uts/armv7/vm/vm_machdep.c.
 */
#define	PAGE_GET_COLOR_SHIFT(szc, nszc)				\
	    (hw_page_array[(nszc)].hp_shift - hw_page_array[(szc)].hp_shift)

#define	PAGE_CONVERT_COLOR(ncolor, szc, nszc)			\
	    ((ncolor) << PAGE_GET_COLOR_SHIFT((szc), (nszc)))

#define	PFN_2_COLOR(pfn, szc, it)					\
	(((pfn) & page_colors_mask) >>			                \
	(hw_page_array[szc].hp_shift - hw_page_array[0].hp_shift))

#define	PNUM_SIZE(szc)							\
	(hw_page_array[(szc)].hp_pgcnt)
#define	PNUM_SHIFT(szc)							\
	(hw_page_array[(szc)].hp_shift - hw_page_array[0].hp_shift)
#define	PAGE_GET_SIZE(szc)						\
	(hw_page_array[(szc)].hp_size)
#define	PAGE_GET_SHIFT(szc)						\
	(hw_page_array[(szc)].hp_shift)
#define	PAGE_GET_PAGECOLORS(szc)					\
	(hw_page_array[(szc)].hp_colors)

#define	PAGE_NEXT_PFN_FOR_COLOR(pfn, szc, color, ceq_mask, color_mask, it) \
	panic("page_next_pfn_for_color")

/* get the color equivalency mask for the next szc */
#define	PAGE_GET_NSZ_MASK(szc, mask)                                         \
	((mask) >> (PAGE_GET_SHIFT((szc) + 1) - PAGE_GET_SHIFT(szc)))

/* get the color of the next szc */
#define	PAGE_GET_NSZ_COLOR(szc, color)                                       \
	((color) >> (PAGE_GET_SHIFT((szc) + 1) - PAGE_GET_SHIFT(szc)))

/* Find the bin for the given page if it was of size szc */
#define	PP_2_BIN_SZC(pp, szc)	(PFN_2_COLOR(pp->p_pagenum, szc, NULL))

#define	PP_2_BIN(pp)		(PP_2_BIN_SZC(pp, pp->p_szc))

#define	PP_2_MEM_NODE(pp)	(0)
#define	PP_2_MTYPE(pp)		(0)
#define	PP_2_SZC(pp)		(pp->p_szc)

#define	SZCPAGES(szc)		(1 << PAGE_BSZS_SHIFT(szc))
#define	PFN_BASE(pfnum, szc)	(pfnum & ~(SZCPAGES(szc) - 1))

/*
 * XXX These are total strawmen based on i86pc and sun4 for walking the page
 * tables.
 */
typedef struct page_list_walker {
	uint_t	plw_colors;		/* num of colors for szc */
	uint_t  plw_color_mask;		/* colors-1 */
	uint_t	plw_bin_step;		/* next bin: 1 or 2 */
	uint_t  plw_count;		/* loop count */
	uint_t	plw_bin0;		/* starting bin */
	uint_t  plw_bin_marker;		/* bin after initial jump */
	uint_t  plw_bin_split_prev;	/* last bin we tried to split */
	uint_t  plw_do_split;		/* set if OK to split */
	uint_t  plw_split_next;		/* next bin to split */
	uint_t	plw_ceq_dif;		/* number of different color groups */
					/* to check */
	uint_t	plw_ceq_mask[MMU_PAGE_SIZES + 1]; /* color equiv mask */
	uint_t	plw_bins[MMU_PAGE_SIZES + 1];	/* num of bins */
} page_list_walker_t;

extern void page_list_walk_init(uchar_t szc, uint_t flags, uint_t bin,
    int can_split, int use_ceq, page_list_walker_t *plw);

extern struct cpu	cpus[];
#define	CPU0		&cpus[0]

/*
 * XXX memory type initializaiton
 */
#define	MTYPE_INIT(mtype, vp, vaddr, flags, pgsz)	panic("mtype_init")
#define	MTYPE_START(mnode, mtype, flags)	panic("mtype_start")
#define	MTYPE_NEXT(mnode, mtype, flags)	panic("mtype_next")
#define	MTYPE_PGR_INIT(mtype, flags, pp, mnode, pgcnt)	panic("mtype_pgr_init")
#define	MNODETYPE_2_PFN(mnode, mtype, pfnlo, pfnhi)	panic("mnodetype_2_pfn")

#ifdef DEBUG
#define	CHK_LPG(pp, szc)	panic("chk_lpg")
#else
#define	CHK_LPG(pp, szc)
#endif

#define	FULL_REGION_CNT(rg_szc)	\
	(PAGE_GET_SIZE(rg_szc) >> PAGE_GET_SHIFT(rg_szc - 1))

/* Return the leader for this mapping size */
#define	PP_GROUPLEADER(pp, szc) \
	(&(pp)[-(int)((pp)->p_pagenum & (SZCPAGES(szc)-1))])

/* Return the root page for this page based on p_szc */
#define	PP_PAGEROOT(pp) ((pp)->p_szc == 0 ? (pp) : \
	PP_GROUPLEADER((pp), (pp)->p_szc))

/*
 * The counter base must be per page_counter element to prevent
 * races when re-indexing, and the base page size element should
 * be aligned on a boundary of the given region size.
 *
 * We also round up the number of pages spanned by the counters
 * for a given region to PC_BASE_ALIGN in certain situations to simplify
 * the coding for some non-performance critical routines.
 */
#define	PC_BASE_ALIGN		((pfn_t)1 << PAGE_BSZS_SHIFT(mmu_page_sizes-1))
#define	PC_BASE_ALIGN_MASK	(PC_BASE_ALIGN - 1)

/*
 * The following three constants describe the set of page sizes that are
 * supported by the hardware. Note that there is a notion of legacy page sizes
 * for certain applications. However, such applications don't exist on ARMv7, so
 * they'll always get the same data.
 */
extern uint_t mmu_page_sizes;
extern uint_t mmu_exported_page_sizes;
extern uint_t mmu_legacy_page_sizes;

/*
 * These macros are used for converting between userland page sizes and kernel
 * page sizes. However, these are the same on ARMv7 (just like i86pc).
 */
#define	USERSZC_2_SZC(userszc)	userszc
#define	SZC_2_USERSZC(szc)	szc

/*
 * for hw_page_map_t, sized to hold the ratio of large page to base
 * pagesize
 */
typedef	short	hpmctr_t;

/*
 * get the setsize of the current cpu
 *
 * This is complicated by the fact that the I-cache and D-cache may be
 * separate.
 */
extern int l2cache_sz, l2cache_linesz, l2cache_assoc;
#define	L2CACHE_ALIGN		l2cache_linesz
#define	L2CACHE_ALIGN_MAX	64
#define	CPUSETSIZE()		(l2cache_sz / l2cache_assoc)

/*
 * Return the log2(pagesize(szc) / MMU_PAGESIZE) --- or the shift count
 * for the number of base pages in this pagesize
 */
#define	PAGE_BSZS_SHIFT(szc) (PNUM_SHIFT(szc) - MMU_PAGESHIFT)

/*
 * Internal PG_ flags.
 */
#define	PGI_RELOCONLY	0x010000	/* opposite of PG_NORELOC */
#define	PGI_NOCAGE	0x020000	/* cage is disabled */
#define	PGI_PGCPHIPRI	0x040000	/* page_get_contig_page pri alloc */
#define	PGI_PGCPSZC0	0x080000	/* relocate base pagesize page */

/*
 * XXX Consider PGI flags for ourselves
 */

#define	AS_2_BIN(as, seg, vp, addr, bin, szc)	panic("as_2_bin")

/*
 * XXX For the moment, we'll use the same value for VM_CPU_DATA_PADSIZE that
 * is used on other platforms. We don't use this at all, but it's required for
 * stuff like vm_pagelist.c to build. We should figure out what the right answer
 * looks like here.
 */
/*
 * cpu private vm data - accessed thru CPU->cpu_vm_data
 *	vc_pnum_memseg: tracks last memseg visited in page_numtopp_nolock()
 *	vc_pnext_memseg: tracks last memseg visited in page_nextn()
 *	vc_kmptr: orignal unaligned kmem pointer for this vm_cpu_data_t
 *	vc_kmsize: orignal kmem size for this vm_cpu_data_t
 */

typedef struct {
	struct memseg	*vc_pnum_memseg;
	struct memseg	*vc_pnext_memseg;
	void		*vc_kmptr;
	size_t		vc_kmsize;
} vm_cpu_data_t;

/* allocation size to ensure vm_cpu_data_t resides in its own cache line */
#define	VM_CPU_DATA_PADSIZE						\
	(P2ROUNDUP(sizeof (vm_cpu_data_t), L2CACHE_ALIGN_MAX))

/*
 * When a bin is empty, and we can't satisfy a color request correctly,
 * we scan.  If we assume that the programs have reasonable spatial
 * behavior, then it will not be a good idea to use the adjacent color.
 * Using the adjacent color would result in virtually adjacent addresses
 * mapping into the same spot in the cache.  So, if we stumble across
 * an empty bin, skip a bunch before looking.  After the first skip,
 * then just look one bin at a time so we don't miss our cache on
 * every look. Be sure to check every bin.  Page_create() will panic
 * if we miss a page.
 *
 * This also explains the `<=' in the for loops in both page_get_freelist()
 * and page_get_cachelist().  Since we checked the target bin, skipped
 * a bunch, then continued one a time, we wind up checking the target bin
 * twice to make sure we get all of them bins.
 */
#define	BIN_STEP	19

/*
 * TODO We should re-evaluate this at some point. This is a reasonable set of
 * stats that both i86pc and sun4 have, which likely the common code all
 * requires. We may find that we want additional stats here.
 */
#ifdef VM_STATS
struct vmm_vmstats_str {
	ulong_t pgf_alloc[MMU_PAGE_SIZES];	/* page_get_freelist */
	ulong_t pgf_allocok[MMU_PAGE_SIZES];
	ulong_t pgf_allocokrem[MMU_PAGE_SIZES];
	ulong_t pgf_allocfailed[MMU_PAGE_SIZES];
	ulong_t	pgf_allocdeferred;
	ulong_t	pgf_allocretry[MMU_PAGE_SIZES];
	ulong_t pgc_alloc;			/* page_get_cachelist */
	ulong_t pgc_allocok;
	ulong_t pgc_allocokrem;
	ulong_t pgc_allocokdeferred;
	ulong_t pgc_allocfailed;
	ulong_t	pgcp_alloc[MMU_PAGE_SIZES];	/* page_get_contig_pages */
	ulong_t	pgcp_allocfailed[MMU_PAGE_SIZES];
	ulong_t	pgcp_allocempty[MMU_PAGE_SIZES];
	ulong_t	pgcp_allocok[MMU_PAGE_SIZES];
	ulong_t	ptcp[MMU_PAGE_SIZES];		/* page_trylock_contig_pages */
	ulong_t	ptcpfreethresh[MMU_PAGE_SIZES];
	ulong_t	ptcpfailexcl[MMU_PAGE_SIZES];
	ulong_t	ptcpfailszc[MMU_PAGE_SIZES];
	ulong_t	ptcpfailcage[MMU_PAGE_SIZES];
	ulong_t	ptcpok[MMU_PAGE_SIZES];
	ulong_t	pgmf_alloc[MMU_PAGE_SIZES];	/* page_get_mnode_freelist */
	ulong_t	pgmf_allocfailed[MMU_PAGE_SIZES];
	ulong_t	pgmf_allocempty[MMU_PAGE_SIZES];
	ulong_t	pgmf_allocok[MMU_PAGE_SIZES];
	ulong_t	pgmc_alloc;			/* page_get_mnode_cachelist */
	ulong_t	pgmc_allocfailed;
	ulong_t	pgmc_allocempty;
	ulong_t	pgmc_allocok;
	ulong_t	pladd_free[MMU_PAGE_SIZES];	/* page_list_add/sub */
	ulong_t	plsub_free[MMU_PAGE_SIZES];
	ulong_t	pladd_cache;
	ulong_t	plsub_cache;
	ulong_t	plsubpages_szcbig;
	ulong_t	plsubpages_szc0;
	ulong_t	pfs_req[MMU_PAGE_SIZES];	/* page_freelist_split */
	ulong_t	pfs_demote[MMU_PAGE_SIZES];
	ulong_t	pfc_coalok[MMU_PAGE_SIZES][MAX_MNODE_MRANGES];
	ulong_t	ppr_reloc[MMU_PAGE_SIZES];	/* page_relocate */
	ulong_t ppr_relocnoroot[MMU_PAGE_SIZES];
	ulong_t ppr_reloc_replnoroot[MMU_PAGE_SIZES];
	ulong_t ppr_relocnolock[MMU_PAGE_SIZES];
	ulong_t ppr_relocnomem[MMU_PAGE_SIZES];
	ulong_t ppr_relocok[MMU_PAGE_SIZES];
	ulong_t ppr_copyfail;
	/* page coalesce counter */
	ulong_t page_ctrs_coalesce[MMU_PAGE_SIZES][MAX_MNODE_MRANGES];
	/* candidates useful */
	ulong_t page_ctrs_cands_skip[MMU_PAGE_SIZES][MAX_MNODE_MRANGES];
	/* ctrs changed after locking */
	ulong_t page_ctrs_changed[MMU_PAGE_SIZES][MAX_MNODE_MRANGES];
	/* page_freelist_coalesce failed */
	ulong_t page_ctrs_failed[MMU_PAGE_SIZES][MAX_MNODE_MRANGES];
	ulong_t page_ctrs_coalesce_all;	/* page coalesce all counter */
	ulong_t page_ctrs_cands_skip_all; /* candidates useful for all func */
	ulong_t	restrict4gcnt;
	ulong_t	unrestrict16mcnt;	/* non-DMA 16m allocs allowed */
	ulong_t	pgpanicalloc;		/* PG_PANIC allocation */
	ulong_t	pcf_deny[MMU_PAGE_SIZES];	/* page_chk_freelist */
	ulong_t	pcf_allow[MMU_PAGE_SIZES];
};
extern struct vmm_vmstats_str vmm_vmstats;
#endif	/* VM_STATS */


#ifdef __cplusplus
}
#endif

#endif /* _VM_DEP_H */
