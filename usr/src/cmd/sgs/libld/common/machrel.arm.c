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
 * Portions:
 * Copyright (c) 1989, 2010, Oracle and/or its affiliates. All rights reserved.
 */

/*
 * XXXARM: Luckily, to some degree, a lot of the target-specific things in the
 * link-editor aren't _exactly_ target specific, and a reasonable
 * approximation of them can be derived from the implementations of the other
 * targets.  That is what we're doing in this file right now -- Large chunks
 * are directly derived from the intel implementation.
 *
 * It is possible, and in fact likely, that I have misunderstood the
 * commonality of various pieces of this with the intel implementation and in
 * doing so have introduced bugs.
 *
 * I should also state that the comments describing various functions are
 * actually describing my understanding thereof.  It is not unlikely that my
 * understanding is flawed.
 */

#define	DO_RELOC_LIBLD_ARM

#include	<sys/elf_ARM.h>
#include	<stdio.h>
#include	<debug.h>
#include	<reloc.h>
#include	<arm/machdep_arm.h>
#include	"msg.h"
#include	"_libld.h"

static Word
ld_init_rel(Rel_desc *reld, Word *typedata, void *reloc)
{
	Rel	*rel = (Rel *)reloc;

	reld->rel_rtype = (Word)ELF_R_TYPE(rel->r_info, M_MACH);
	reld->rel_roffset = rel->r_offset;
	reld->rel_raddend = 0;
	*typedata = (Word)ELF_R_TYPE_DATA(rel->r_info);

	return ((Word)ELF_R_SYM(rel->r_info));
}

static void
ld_mach_eflags(Ehdr *ehdr, Ofl_desc *ofl)
{
	/*
	 * XXXARM: We want some kind of compatibility checking for input flags
	 * here, and to bail if we it's wrong.
	 */
	ofl->ofl_dehdr->e_flags |= ehdr->e_flags;

}

static void
ld_mach_make_dynamic(Ofl_desc *ofl, size_t *cnt)
{
	if (!(ofl->ofl_flags & FLG_OF_RELOBJ)) {
		/* Create this entry if we are going to create a PLT. */
		if (ofl->ofl_pltcnt > 0)
			(*cnt)++; /* DT_PLTGOT */
	}
}

static void
ld_mach_update_odynamic(Ofl_desc *ofl, Dyn **dyn)
{
	if (((ofl->ofl_flags & FLG_OF_RELOBJ) == 0) && ofl->ofl_pltcnt) {
		(*dyn)->d_tag = DT_PLTGOT;
		if (ofl->ofl_osgot)
			(*dyn)->d_un.d_ptr = ofl->ofl_osgot->os_shdr->sh_addr;
		else
			(*dyn)->d_un.d_ptr = 0;
		(*dyn)++;
	}
}

static Xword
ld_calc_plt_addr(Sym_desc *sdp, Ofl_desc *ofl)
{
	Xword	value;

	value = (Xword)(ofl->ofl_osplt->os_shdr->sh_addr) +
	    M_PLT_RESERVSZ + ((sdp->sd_aux->sa_PLTndx - 1) * M_PLT_ENTSIZE);
	return (value);
}

/* ARGSUSED */
static Gotndx *
ld_find_got_ndx(Alist *alp, Gotref gref, Ofl_desc *ofl, Rel_desc *rdesc)
{
	Aliste	indx;
	Gotndx	*gnp;

	if ((gref == GOT_REF_TLSLD) && ofl->ofl_tlsldgotndx)
		return (ofl->ofl_tlsldgotndx);

	for (ALIST_TRAVERSE(alp, indx, gnp)) {
		if (gnp->gn_gotref == gref)
			return (gnp);
	}

	return (NULL);
}

static Xword
ld_calc_got_offset(Rel_desc *rdesc, Ofl_desc *ofl)
{
	Os_desc		*osp = ofl->ofl_osgot;
	Sym_desc	*sdp = rdesc->rel_sym;
	Xword		gotndx;
	Gotref		gref;
	Gotndx		*gnp;

	if (rdesc->rel_flags & FLG_REL_DTLS)
		gref = GOT_REF_TLSGD;
	else if (rdesc->rel_flags & FLG_REL_MTLS)
		gref = GOT_REF_TLSLD;
	else if (rdesc->rel_flags & FLG_REL_STLS)
		gref = GOT_REF_TLSIE;
	else
		gref = GOT_REF_GENERIC;

	gnp = ld_find_got_ndx(sdp->sd_GOTndxs, gref, ofl, NULL);
	assert(gnp);

	gotndx = (Xword)gnp->gn_gotndx;

	if ((rdesc->rel_flags & FLG_REL_DTLS) &&
	    (rdesc->rel_rtype == R_ARM_TLS_DTPOFF32))
		gotndx++;

	return ((Xword)(osp->os_shdr->sh_addr + (gotndx * M_GOT_ENTSIZE)));
}

/*
 * Build a single PLT entry.  See the comment for ld_fillin_pltgot() for a
 * more complete description.
 */
/* ARGSUSED */
static void
plt_entry(Ofl_desc *ofl, Sym_desc *sdp)
{
	uchar_t	*pltent, *gotent;
	Word	plt_off;
	Word	got_off;
	Word	got_disp;
	Boolean	bswap = (ofl->ofl_flags1 & FLG_OF1_ENCDIFF) != 0;
	Addr	got_addr, plt_addr;

	got_off = sdp->sd_aux->sa_PLTGOTndx * M_GOT_ENTSIZE;
	plt_off = M_PLT_RESERVSZ + ((sdp->sd_aux->sa_PLTndx - 1) *
	    M_PLT_ENTSIZE);

	pltent = (uchar_t *)(ofl->ofl_osplt->os_outdata->d_buf) + plt_off;
	gotent = (uchar_t *)(ofl->ofl_osgot->os_outdata->d_buf) + got_off;

	got_addr = ofl->ofl_osgot->os_shdr->sh_addr + got_off;
	plt_addr = ofl->ofl_osplt->os_shdr->sh_addr + plt_off;
	got_disp = got_addr - (plt_addr + 8); /* adjusted for %pc offset */

	/* LINTED */
	*(Word *)gotent = ofl->ofl_osplt->os_shdr->sh_addr;
	if (bswap)
		/* LINTED */
		*(Word *)gotent = ld_bswap_Word(*(Word *)gotent);

	/* add ip, pc, #0 | ...  */
	/* LINTED */
	*(Word *)pltent = 0xe28fc600 | ((got_disp & 0xfff00000) >> 20);
	if (bswap)
		/* LINTED */
		*(Word *)pltent = ld_bswap_Word(*(Word *)pltent);
	pltent += M_PLT_INSSIZE;

	/* add ip, ip, #0 | ... */
	/* LINTED */
	*(Word *)pltent = 0xe28cca00 | ((got_disp & 0x000ff000) >> 12);
	if (bswap)
		/* LINTED */
		*(Word *)pltent = ld_bswap_Word(*(Word *)pltent);
	pltent += M_PLT_INSSIZE;

	/* ldr pc, [ip, #0]! | ...  */
	/* LINTED */
	*(Word *)pltent = 0xe5bcf000 | (got_disp & 0x00000fff);
	if (bswap)
		/* LINTED */
		*(Word *)pltent = ld_bswap_Word(*(Word *)pltent);
	pltent += M_PLT_INSSIZE;
}

/*
 * Insert an appropriate dynamic relocation into the output image in the
 * appropriate relocation section.
 *
 * Primarily, this is not particularly target-specific, and involves
 * calculating the correct offset for the relocation entry to be written, and
 * accounting for some complicated edge cases.
 *
 * Heavily taken from the Intel implementation.
 */
static uintptr_t
ld_perform_outreloc(Rel_desc *orsp, Ofl_desc *ofl, Boolean *remain_seen)
{
	Os_desc		*relosp, *osp = NULL;
	Word		ndx, roffset, value;
	Rel		rea;
	char		*relbits;
	Sym_desc	*sdp, *psym = NULL;
	Boolean		sectmoved = FALSE;

	sdp = orsp->rel_sym;

	/*
	 * If the section this relocation is against has been dicarded
	 * (-zignore), then also discard the relocation itself.
	 */
	if ((orsp->rel_isdesc != NULL) && ((orsp->rel_flags &
	    (FLG_REL_GOT | FLG_REL_BSS | FLG_REL_PLT | FLG_REL_NOINFO)) == 0) &&
	    (orsp->rel_isdesc->is_flags & FLG_IS_DISCARD)) {
		DBG_CALL(Dbg_reloc_discard(ofl->ofl_lml, M_MACH, orsp));
		return (1);
	}

	/*
	 * If this is a relocation against a move table, or expanded move
	 * table, adjust the relocation entries.
	 */
	if (RELAUX_GET_MOVE(orsp) != NULL)
		ld_adj_movereloc(ofl, orsp);

	/*
	 * If this is a relocation against a section using a partially
	 * initialized symbol, adjust the embedded symbol info.
	 *
	 * The second argument of the am_I_partial() is the value stored at
	 * the target address to which the relocation is going to be
	 * applied.
	 */
	if (ELF_ST_TYPE(sdp->sd_sym->st_info) == STT_SECTION) {
		if (ofl->ofl_parsyms &&
		    (sdp->sd_isc->is_flags & FLG_IS_RELUPD) &&
		    /* LINTED */
		    (psym = ld_am_I_partial(orsp, *(Xword *)((uchar_t *)
		    (orsp->rel_isdesc->is_indata->d_buf) +
		    orsp->rel_roffset)))) {
			DBG_CALL(Dbg_move_outsctadj(ofl->ofl_lml, psym));
			sectmoved = TRUE;
		}
	}

	value = sdp->sd_sym->st_value;

	if (orsp->rel_flags & FLG_REL_GOT) {
		osp = ofl->ofl_osgot;
		roffset = (Word)ld_calc_got_offset(orsp, ofl);
	} else if (orsp->rel_flags & FLG_REL_PLT) {
		/*
		 * Note that relocations for PLTs actually cause a relocation
		 * against the GOT
		 */
		osp = ofl->ofl_osplt;
		roffset = (Word) (ofl->ofl_osgot->os_shdr->sh_addr) +
		    sdp->sd_aux->sa_PLTGOTndx * M_GOT_ENTSIZE;

		plt_entry(ofl, sdp);
	} else if (orsp->rel_flags & FLG_REL_BSS) {
		/*
		 * This must be an R_ARM_COPY.  For these set the roffset to
		 * point to the new symbol's location.
		 */
		osp = ofl->ofl_isbss->is_osdesc;
		roffset = (Word)value;
	} else {
		osp = RELAUX_GET_OSDESC(orsp);

		/*
		 * Calculate virtual offset of reference point; equals offset
		 * into section + vaddr of section for loadable sections, or
		 * offset plus section displacement for nonloadable
		 * sections.
		 */
		roffset = orsp->rel_roffset +
		    (Off)_elf_getxoff(orsp->rel_isdesc->is_indata);
		if (!(ofl->ofl_flags & FLG_OF_RELOBJ))
			roffset += orsp->rel_isdesc->is_osdesc->
			    os_shdr->sh_addr;
	}

	if ((osp == NULL) || ((relosp = osp->os_relosdesc) == NULL)) {
		relosp = ofl->ofl_osrel;
	}

	/*
	 * Assign the symbols index for the output relocation.  If the
	 * relocation refers to a SECTION symbol then it's index is based upon
	 * the output sections symbols index.  Otherwise the index can be
	 * derived from the symbols index itself.
	 */
	if (orsp->rel_rtype == R_ARM_RELATIVE) {
		ndx = STN_UNDEF;
	} else if ((orsp->rel_flags & FLG_REL_SCNNDX) ||
	    (ELF_ST_TYPE(sdp->sd_sym->st_info) == STT_SECTION)) {
		if (sectmoved == FALSE) {
			/*
			 * Check for a null input section.  This can occur if
			 * this relocation references a symbol generated by
			 * sym_add_sym()
			 */
			if ((sdp->sd_isc != NULL) &&
			    (sdp->sd_isc->is_osdesc != NULL)) {
				ndx = sdp->sd_isc->is_osdesc->os_identndx;
			} else {
				ndx = sdp->sd_shndx;
			}
		} else {
			ndx = ofl->ofl_parexpnndx;
		}
	} else {
		ndx = sdp->sd_symndx;
	}

	/*
	 * If we have a replacement value for the relocation target, put it in
	 * place now.
	 */
	if (orsp->rel_flags & FLG_REL_NADDEND) {
		Xword	addend = orsp->rel_raddend;
		uchar_t	*addr;

		/*
		 * Get the address of the data item we nede to modify.
		 */
		addr = (uchar_t *)((uintptr_t)orsp->rel_roffset +
		    (uintptr_t)_elf_getxoff(orsp->rel_isdesc->is_indata));
		addr += (uintptr_t)RELAUX_GET_OSDESC(orsp)->os_outdata->d_buf;
		if (ld_reloc_targval_set(ofl, orsp, addr, addend) == 0) {
			return (S_ERROR);
		}
	}

	relbits = (char *)relosp->os_outdata->d_buf;

	rea.r_info = ELF_R_INFO(ndx, orsp->rel_rtype);
	rea.r_offset = roffset;
	DBG_CALL(Dbg_reloc_out(ofl, ELF_DBG_LD, SHT_REL, &rea, relosp->os_name,
	    ld_reloc_sym_name(orsp)));

	/* Assert we haven't walked off the end of our relocation table. */
	assert(relosp->os_szoutrels <= relosp->os_shdr->sh_size);

	(void) memcpy((relbits + relosp->os_szoutrels),
	    (char *)&rea, sizeof (Rel));
	relosp->os_szoutrels += sizeof (Rel);

	/*
	 * Determine if this relocation is against a non-writable, allocatable
	 * section.  If so we may need to provide a text relocation
	 * diagnostic.
	 *
	 * Note that relocations against the .plt (R_ARM_JUMP_SLOT) actually
	 * result in modifications to the .got
	 */
	if (orsp->rel_rtype == R_ARM_JUMP_SLOT)
		osp = ofl->ofl_osgot;

	ld_reloc_remain_entry(orsp, osp, ofl, remain_seen);
	return (1);
}

/*
 * The way we handle relocations takes a bit of explaining, in comparison to
 * how the AEABI documents (and most others) document them.
 *
 * ld_do_activerlocs handles active (in the AEABI document "static")
 * relocations.  That is relocations which are resolved in or under this call
 * by the link-editor.
 *
 * There are also output relocations, in the AEABI document "dynamic"
 * relocations, those placed into the output image and resolved by the linker
 * at runtime.  You should see ld_add_outrel and ld_perform_outreloc for
 * descriptions of these.
 *
 * Support for actually performing relocations is split into two parts.  This
 * function (and counterparts in rtld and krtld) calculate the relocation's
 * value, without reference to the addend.  The other part, implemented as a
 * common do_reloc_* function (with 3 names, based on the linker using it),
 * actually includes the addend and updates the output location. (this code is
 * in uts/arm/krtld).
 *
 * This code, given: R_ARM_CALL == (((S + A) | T) - P)
 *
 * decomposes that into X = (S + A) (calculated in this function), and X + A
 * (calculated by do_reloc).
 *
 * T is always 0, except in the case of Thumb interworking, which we do not
 * support (if we wanted to support it, note that T always sets the low bit,
 * so is safely decomposable.)
 *
 * XXXARM: This code is, largely, taken after the intel implementation with
 * which we share sufficient similarily.  This means that there is support for
 * certain relocations here which we have not actually seen generated or used
 * yet.  While that code is _probably_ right, it is not _definitely_ right.
 */
static uintptr_t
ld_do_activerelocs(Ofl_desc *ofl)
{
	Rel_desc	*arsp;
	Rel_cachebuf	*rcbp;
	Aliste		idx;
	uintptr_t	return_code = 1;
	ofl_flag_t	flags = ofl->ofl_flags;

	if (aplist_nitems(ofl->ofl_actrels.rc_list) != 0)
		DBG_CALL(Dbg_reloc_doact_title(ofl->ofl_lml));

	/* Process active relocations */
	REL_CACHE_TRAVERSE(&ofl->ofl_actrels, idx, rcbp, arsp) {
		uchar_t		*addr;
		Xword		value;
		Sym_desc	*sdp;
		const char	*ifl_name;
		Xword		refaddr;
		Boolean		moved = FALSE;
		Gotref		gref;
		Os_desc		*osp;

		/*
		 * If the section this relocation is against has been
		 * discarded (-zignore), then discard the relocation itself
		 */
		if ((arsp->rel_isdesc->is_flags & FLG_IS_DISCARD) &&
		    ((arsp->rel_flags & (FLG_REL_GOT | FLG_REL_BSS |
		    FLG_REL_PLT | FLG_REL_NOINFO)) == 0)) {
			DBG_CALL(Dbg_reloc_discard(ofl->ofl_lml, M_MACH, arsp));
			continue;
		}

		/*
		 * Determine the 'got reference' model
		 * XXX: If we have TLS fixups, this must happen first
		 */
		if (arsp->rel_flags & FLG_REL_DTLS)
			gref = GOT_REF_TLSGD;
		else if (arsp->rel_flags & FLG_REL_MTLS)
			gref = GOT_REF_TLSLD;
		else if (arsp->rel_flags & FLG_REL_STLS)
			gref = GOT_REF_TLSIE;
		else
			gref = GOT_REF_GENERIC;

		/* XXXARM: Unimplemented */
		if (arsp->rel_flags & FLG_REL_TLSFIX) {
			assert(0 && "Relocation claiming to need TLS fixups");
		}

		/*
		 * If this is a relocation against a move table, or expanded
		 * move table, adjust the relocation entries
		 */
		if (RELAUX_GET_MOVE(arsp))
			ld_adj_movereloc(ofl, arsp);

		sdp = arsp->rel_sym;
		refaddr = arsp->rel_roffset +
		    (Off)_elf_getxoff(arsp->rel_isdesc->is_indata);

		if (arsp->rel_flags & FLG_REL_CLVAL) {
			value = 0;
		} else if (ELF_ST_TYPE(sdp->sd_sym->st_info) ==
		    STT_SECTION) {
			/*
			 * XXXARM: This differs a lot _in code_ between
			 * platforms, but doesn't necessarily cover much in
			 * content.  We take it from intel, but it's risky.
			 *
			 * And also presumably a bit wrong?
			 */
			if (sdp->sd_isc->is_flags & FLG_IS_RELUPD) {
				Sym_desc	*sym;
				Xword		radd;
				uchar_t		*raddr = (uchar_t *)
				    arsp->rel_isdesc->is_indata->d_buf +
				    arsp->rel_roffset;

				/*
				 * This is a REL platform.  Hence, the second
				 * argument of ld_am_I_partial() is the value
				 * stored at the target address where the
				 * relocation is going to be applied.
				 */
				if (ld_reloc_targval_get(ofl, arsp, raddr,
				    &radd) == 0)
					return (S_ERROR);

				sym = ld_am_I_partial(arsp, radd);
				if (sym != NULL) {
					Sym	*osym = sym->sd_osym;

					/*
					 * The symbol was moved, so adjust the
					 * value relative to the new section.
					 */
					value = sym->sd_sym->st_value;
					moved = TRUE;

					/*
					 * The original raddend covers the
					 * displacement from the section start
					 * to the desired address. The value
					 * computed above gets us from the
					 * section start to the start of the
					 * symbol range. Adjust the old raddend
					 * to remove the offset from section
					 * start to symbol start, leaving the
					 * displacement within the range of
					 * the symbol.
					 */
					if (osym->st_value != 0) {
						radd -= osym->st_value;
						if (ld_reloc_targval_set(ofl,
						    arsp, raddr, radd) == 0)
							return (S_ERROR);
					}
				}
			}
			if (!moved) {
				value = _elf_getxoff(sdp->sd_isc->is_indata);

				if (sdp->sd_isc->is_shdr->sh_flags & SHF_ALLOC)
					value += sdp->sd_isc->
					    is_osdesc->os_shdr->sh_addr;
			}

			if (sdp->sd_isc->is_shdr->sh_flags & SHF_TLS)
				value -= ofl->ofl_tlsphdr->p_vaddr;
		} else if ((sdp->sd_flags & FLG_SY_CAP) &&
		    ((sdp->sd_aux != NULL) && sdp->sd_aux->sa_PLTndx)) {
			/*
			 * If relocation is against a capabilities symbol, we
			 * need to jump to an associated PLT, so that at runtime
			 * ld.so.1 is involved to determine the best binding
			 * choice. Otherwise, the value is the symbols value.
			 */
			value = ld_calc_plt_addr(sdp, ofl);
		} else {
			value = sdp->sd_sym->st_value;
		}

		/* Relocation against the GLOBAL_OFFSET_TABLE */
		if ((arsp->rel_flags & FLG_REL_GOT) &&
		    !ld_reloc_set_aux_osdesc(ofl, arsp, ofl->ofl_osgot))
			return (S_ERROR);
		osp = RELAUX_GET_OSDESC(arsp);

		/*
		 * If loadable and not producing a relocatable object add the
		 * sections virtual address to the reference address.
		 */
		if ((arsp->rel_flags & FLG_REL_LOAD) &&
		    ((flags & FLG_OF_RELOBJ) == 0))
			refaddr +=
			    arsp->rel_isdesc->is_osdesc->os_shdr->sh_addr;

		/*
		 * If this entry has a PLT assigned to it, its value is
		 * actually the address of the PLT (and not the address of the
		 * function).
		 */
		if (IS_PLT(arsp->rel_rtype)) {
			if ((sdp->sd_aux != NULL) && sdp->sd_aux->sa_PLTndx)
				value = ld_calc_plt_addr(sdp, ofl);
		}

		/*
		 * Determine whether the value needs further adjustment.
		 * Filter through the attributes of the relocation to
		 * determine what adjustment is required.  Note, many of the
		 * following cases are only applicable when a .got is present.
		 * As a .got is not generated when a relocatable object is
		 * being built, any adjustments that require a .got need to be
		 * skipped
		 */
		if ((arsp->rel_flags & FLG_REL_GOT) &&
		    ((flags & FLG_OF_RELOBJ) == 0)) {
			Xword		R1addr;
			uintptr_t	R2addr;
			Word		gotndx;
			Gotndx		*gnp;

			/*
			 * Perform relocation agaist GOT table.  Since this
			 * doesn't fit exactly into a relocation we place the
			 * appropriate byte in the GOT directly.
			 *
			 * Calculate offset into GOT at which to apply the
			 * relocation.
			 */
			gnp = ld_find_got_ndx(sdp->sd_GOTndxs, gref, ofl, NULL);
			assert(gnp);

			if (arsp->rel_rtype == R_ARM_TLS_DTPOFF32)
				gotndx = gnp->gn_gotndx + 1;
			else
				gotndx = gnp->gn_gotndx;

			R1addr = (Xword)(gotndx * M_GOT_ENTSIZE);

			/*
			 * Add the GOT's data's offset.
			 */
			R2addr = R1addr + (uintptr_t)osp->os_outdata->d_buf;

			DBG_CALL(Dbg_reloc_doact(ofl->ofl_lml, ELF_DBG_LD_ACT,
			    M_MACH, SHT_REL, arsp, R1addr, value,
			    ld_reloc_sym_name));

			/*
			 * And do it.
			 */
			if (ofl->ofl_flags1 & FLG_OF1_ENCDIFF)
				*(Xword *)R2addr = ld_bswap_Xword(value);
			else
				*(Xword *)R2addr = value;
			continue;
		} else if (IS_GOT_BASED(arsp->rel_rtype) &&
		    ((flags & FLG_OF_RELOBJ) == 0)) {
			value -= ofl->ofl_osgot->os_shdr->sh_addr;
		} else if (IS_GOT_PC(arsp->rel_rtype) &&
		    ((flags & FLG_OF_RELOBJ) == 0)) {
			value = (Xword)(ofl->ofl_osgot->os_shdr->sh_addr) -
			    refaddr;
		} else if ((IS_PC_RELATIVE(arsp->rel_rtype)) &&
		    (((flags & FLG_OF_RELOBJ) == 0) ||
		    (osp == sdp->sd_isc->is_osdesc))) {
			value -= refaddr;
		} else if (IS_TLS_INS(arsp->rel_rtype) &&
		    IS_GOT_RELATIVE(arsp->rel_rtype) &&
		    ((flags & FLG_OF_RELOBJ) == 0)) {
			assert(0 && "TLS_INS active relocation unimplemented");
		} else if (IS_GOT_RELATIVE(arsp->rel_rtype) &&
		    ((flags & FLG_OF_RELOBJ) == 0)) {
			Gotndx *gnp;

			gnp = ld_find_got_ndx(sdp->sd_GOTndxs,
			    GOT_REF_GENERIC, ofl, NULL);
			assert(gnp);
			value = (Xword)gnp->gn_gotndx * M_GOT_ENTSIZE;
		} else if ((arsp->rel_flags & FLG_REL_STLS) &&
		    ((flags & FLG_OF_RELOBJ) == 0)) {
			assert(0 && "FLG_REL_STLS unimplemented");
		} else if (arsp->rel_rtype == R_ARM_TLS_LE32 &&
		    ((flags & FLG_OF_RELOBJ) == 0)) {
			assert(0 && "R_ARM_TLS_LE32 unimplemented");
		}

		if (IS_SEG_RELATIVE(arsp->rel_rtype)) {
			Sg_desc	*oseg = NULL;

			/*
			 * XXXARM: For the NULL symbol and BASE_PREL, we're
			 * meant to act as for _GLOBAL_OFFSET_TABLE_
			 */
			if (sdp->sd_isc == NULL) {
				switch (sdp->sd_aux->sa_symspec) {
				case SDAUX_ID_GOT:
					oseg = ofl->ofl_osgot->os_sgdesc;
					break;
				case SDAUX_ID_PLT:
					oseg = ofl->ofl_osplt->os_sgdesc;
					break;
				case SDAUX_ID_DYN:
					oseg = ofl->ofl_osdynamic->os_sgdesc;
					break;
				default:
					assert(0 &&
					    "unsupported special symbol in "
					    "segment-relative relocation");
				}
			} else {
				oseg = sdp->sd_isc->is_osdesc->os_sgdesc;
			}

			value = oseg->sg_phdr.p_vaddr - refaddr;
		}

		if (arsp->rel_isdesc->is_file)
			ifl_name = arsp->rel_isdesc->is_file->ifl_name;
		else
			ifl_name = MSG_INTL(MSG_STR_NULL);

		/*
		 * Make sure we have data to relocate.  Compile and assembler
		 * developers have been known to generate relocations against
		 * invalid sections (normally .bss), so for their benefit give
		 * them sufficient information to help analyze the problem.
		 * End users should never see this.
		 */
		if (arsp->rel_isdesc->is_indata->d_buf == NULL) {
			Conv_inv_buf_t	inv_buf;

			ld_eprintf(ofl, ERR_FATAL, MSG_INTL(MSG_REL_EMPTYSEC),
			    conv_reloc_arm_type(arsp->rel_rtype, 0, &inv_buf),
			    ifl_name, ld_reloc_sym_name(arsp),
			    EC_WORD(arsp->rel_isdesc->is_scnndx),
			    arsp->rel_isdesc->is_name);
			return (S_ERROR);
		}

		/*
		 * Get the address of the data item we need to modify
		 */
		addr = (uchar_t *)((uintptr_t)arsp->rel_roffset +
		    (uintptr_t)_elf_getxoff(arsp->rel_isdesc->is_indata));

		DBG_CALL(Dbg_reloc_doact(ofl->ofl_lml, ELF_DBG_LD_ACT,
		    M_MACH, SHT_REL, arsp, EC_NATPTR(addr), value,
		    ld_reloc_sym_name));
		addr += (uintptr_t)osp->os_outdata->d_buf;

		if ((((uintptr_t)addr - (uintptr_t)ofl->ofl_nehdr) >
		    ofl->ofl_size) || (arsp->rel_roffset >
		    osp->os_shdr->sh_size)) {
			Conv_inv_buf_t	inv_buf;
			int		class;

			if (((uintptr_t)addr - (uintptr_t)ofl->ofl_nehdr) >
			    ofl->ofl_size)
				class = ERR_FATAL;
			else
				class = ERR_WARNING;

			ld_eprintf(ofl, class, MSG_INTL(MSG_REL_INVALOFFSET),
			    conv_reloc_arm_type(arsp->rel_rtype, 0, &inv_buf),
			    ifl_name, EC_WORD(arsp->rel_isdesc->is_scnndx),
			    arsp->rel_isdesc->is_name, ld_reloc_sym_name(arsp),
			    EC_ADDR((uintptr_t)addr -
			    (uintptr_t)ofl->ofl_nehdr));

			if (class == ERR_FATAL) {
				return_code = S_ERROR;
				continue;
			}
		}

		/*
		 * The relocation is additive.  ignore the previous symbol
		 * value if this local partial symbol is expanded.
		 */
		if (moved)
			value -= *addr;

		/*
		 * If we have a replacement value for the relocation target,
		 * put it in place now.
		 */
		if (arsp->rel_flags & FLG_REL_NADDEND) {
			Xword addend = arsp->rel_raddend;

			if (ld_reloc_targval_set(ofl, arsp, addr, addend) == 0)
				return (S_ERROR);
		}

		/*
		 * If '-z noreloc' is specified - skip the do_reloc_ld stage.
		 */
		if (OFL_DO_RELOC(ofl)) {
			if (do_reloc_ld(arsp, addr, &value, ld_reloc_sym_name,
			    ifl_name, OFL_SWAP_RELOC_DATA(ofl, arsp),
			    ofl->ofl_lml) == 0) {
				ofl->ofl_flags |= FLG_OF_FATAL;
				return_code = S_ERROR;
			}
		}
	}

	return (return_code);
}

/*
 * Record an output relocation to be entered into the output file, update any
 * metadata regarding it, set any dynamic flags as appropriate and provide
 * diagnostics about comprimised displacement.
 *
 * The relocation is actually placed into the output image by
 * ld_perform_outreloc().
 *
 * XXXARM: This is in almost every respect not actually target-specific, and
 * taken from the intel implementation.
 */
static uintptr_t
ld_add_outrel(Word flags, Rel_desc *rsp, Ofl_desc *ofl)
{
	Rel_desc	*orsp;
	Sym_desc	*sdp = rsp->rel_sym;

	/*
	 * Static executables *do not* want any relocations against them.
	 * Since our engine still creates relocations against a WEAK UNDEFINED
	 * symbol in a static executable, it's best to disable them here
	 * instead of through out the relocation code.
	 */
	if (OFL_IS_STATIC_EXEC(ofl))
		return (1);

	/*
	 * If we are adding an output relocation against a section symbol
	 * (non-RELATIVE) then mark that section.  These sections will be
	 * added to the .dynsym symbol table
	 */
	if ((sdp != NULL) && (rsp->rel_rtype != M_R_RELATIVE) &&
	    ((flags & FLG_REL_SCNNDX) ||
	    (ELF_ST_TYPE(sdp->sd_sym->st_info) == STT_SECTION))) {
		/*
		 * If this is a COMMON symbol - no output section exists yet -
		 * (it's created as part of sym_validate()).  So - we mark
		 * here that when it's created it should be tagged with the
		 * FLG_OS_OUTREL flag.
		 */
		if ((sdp->sd_flags & FLG_SY_SPECSEC) &&
		    (sdp->sd_sym->st_shndx == SHN_COMMON)) {
			if (ELF_ST_TYPE(sdp->sd_sym->st_info) != STT_TLS)
				ofl->ofl_flags1 |= FLG_OF1_BSSOREL;
			else
				ofl->ofl_flags1 |= FLG_OF1_TLSOREL;
		} else {
			Os_desc *osp;
			Is_desc *isp = sdp->sd_isc;

			if ((isp != NULL) && ((osp = isp->is_osdesc) != NULL) &&
			    ((osp->os_flags & FLG_OS_OUTREL) == 0)) {
				ofl->ofl_dynshdrcnt++;
				osp->os_flags |= FLG_OS_OUTREL;
			}
		}
	}

	/* Enter it into the output relocation cache */
	if ((orsp = ld_reloc_enter(ofl, &ofl->ofl_outrels, rsp, flags)) == NULL)
		return (S_ERROR);

	if (flags & FLG_REL_GOT)
		ofl->ofl_relocgotsz += (Xword)sizeof (Rel);
	if (flags & FLG_REL_PLT)
		ofl->ofl_relocpltsz += (Xword)sizeof (Rel);
	if (flags & FLG_REL_BSS)
		ofl->ofl_relocbsssz += (Xword)sizeof (Rel);
	if (flags & FLG_REL_NOINFO)
		ofl->ofl_relocrelsz += (Xword)sizeof (Rel);
	else
		RELAUX_GET_OSDESC(orsp)->os_szoutrels += (Xword)sizeof (Rel);

	if (orsp->rel_rtype == M_R_RELATIVE)
		ofl->ofl_relocrelcnt++;

	/*
	 * We don't perform sorting on PLT relocations because they have
	 * already been assigned a PLT index and if we were to sort them we
	 * would have to re-assign the plt indexes.
	 */
	if (!(flags & FLG_REL_PLT))
		ofl->ofl_reloccnt++;

	/* Ensure a GLOBAL_OFFSET_TABLE is generated if required. */
	if (IS_GOT_REQUIRED(orsp->rel_rtype))
		ofl->ofl_flags |= FLG_OF_BLDGOT;

	/* Identify and possibly warn of a displacement relocation */
	if (orsp->rel_flags & FLG_REL_DISP) {
		ofl->ofl_dtflags_1 |= DF_1_DISPRELPND;

		if (ofl->ofl_flags & FLG_OF_VERBOSE)
			ld_disp_errmsg(MSG_INTL(MSG_REL_DISPREL4), orsp, ofl);
	}
	DBG_CALL(Dbg_reloc_ors_entry(ofl->ofl_lml, ELF_DBG_LD, SHT_REL,
	    M_MACH, orsp));

	return (1);
}


/*
 * Deal with relocations against symbols which are bound locally, as described
 * ld_process_sym_reloc()
 *
 * Symbols which must be bound locally need to be treated specially to make
 * sure that they, post-relocation, actually do refer to their locally
 * appropriate values.  In most cases we end up doing this with an active
 * relocation resolved during the link-edit, if that for some reason can't be
 * done, we emit R_ARM_RELATIVE to ensure we reach the right symbol.
 *
 * XXXARM: The implementation here is, again, a carbon copy of the intel
 * implementation, which is an almost copy of the SPARC implementation (where
 * more relocations avoid R_..._RELATIVE).
 */
static uintptr_t
ld_reloc_local(Rel_desc *rsp, Ofl_desc *ofl)
{
	ofl_flag_t	flags = ofl->ofl_flags;
	Sym_desc	*sdp = rsp->rel_sym;
	Word		shndx = sdp->sd_sym->st_shndx;

	/*
	 * if ((shared object) and (not pc relative relocation) and
	 *    (not against ABS symbol))
	 * then
	 *    build R_ARM_RELATIVE
	 * fi
	 */
	if ((flags & FLG_OF_SHAROBJ) && (rsp->rel_flags & FLG_REL_LOAD) &&
	    !(IS_PC_RELATIVE(rsp->rel_rtype)) && !(IS_SIZE(rsp->rel_rtype)) &&
	    !(IS_GOT_BASED(rsp->rel_rtype)) &&
	    !(rsp->rel_isdesc != NULL &&
	    (rsp->rel_isdesc->is_shdr->sh_type == SHT_SUNW_dof)) &&
	    (((sdp->sd_flags & FLG_SY_SPECSEC) == 0) ||
	    (shndx != SHN_ABS) || (sdp->sd_aux && sdp->sd_aux->sa_symspec))) {
		Word	ortype = rsp->rel_rtype;

		rsp->rel_rtype = R_ARM_RELATIVE;
		if (ld_add_outrel(0, rsp, ofl) == S_ERROR)
			return (S_ERROR);
		rsp->rel_rtype = ortype;
		/*
		 * XXXARM: Everyone except intel does this.  I bet it's a bug
		 * intel doesn't.
		 */
		return (1);

	}

	/*
	 * If the relocation is against a 'non-allocatable' section
	 * and we can not resolve it now - then give a warning
	 * message.
	 *
	 * We can not resolve the symbol if either:
	 *	a) it's undefined
	 *	b) it's defined in a shared library and a
	 *	   COPY relocation hasn't moved it to the executable
	 *
	 * Note: because we process all of the relocations against the text
	 *	segment before any others - we known whether or not a copy
	 *	relocation will be generated before we get here (see
	 *	reloc_init()->reloc_segments()).
	 */
	if (!(rsp->rel_flags & FLG_REL_LOAD) &&
	    ((shndx == SHN_UNDEF) ||
	    ((sdp->sd_ref == REF_DYN_NEED) &&
	    ((sdp->sd_flags & FLG_SY_MVTOCOMM) == 0)))) {
		Conv_inv_buf_t	inv_buf;
		Os_desc	*osp = RELAUX_GET_OSDESC(rsp);

		/*
		 * If the relocation is against a SHT_SUNW_ANNOTATE section -
		 * then silently ignore that the relocation cannot be
		 * resolved
		 */
		if ((osp != NULL) && (osp->os_shdr->sh_type ==
		    SHT_SUNW_ANNOTATE))
			return (0);

		ld_eprintf(ofl, ERR_WARNING, MSG_INTL(MSG_REL_EXTERNSYM),
		    conv_reloc_arm_type(rsp->rel_rtype, 0, &inv_buf),
		    rsp->rel_isdesc->is_file->ifl_name,
		    ld_reloc_sym_name(rsp), osp->os_name);
		return (1);
	}

	/*
	 * Perform relocation
	 */
	return (ld_add_actrel(0, rsp, ofl));
}

/* ARGSUSED */
static uintptr_t
ld_reloc_GOTOP(Boolean local, Rel_desc *rsp, Ofl_desc *ofl)
{
	assert(0 && "ld_reloc_GOTOP");
	return (0);
}

/* ARGSUSED */
static uintptr_t
ld_reloc_TLS(Boolean local, Rel_desc *rsp, Ofl_desc *ofl)
{
	assert(0 && "ld_reloc_TLS");
	return (0);
}

/*
 * XXXARM: This is taken directly from the x86 version, which happens to
 * work in the non-TLS case, but probably is wrong for TLS.  Which I haven't
 * even begun to care about.
 */
/* ARGSUSED */
static uintptr_t
ld_assign_got_ndx(Alist **alpp, Gotndx *pgnp, Gotref gref, Ofl_desc *ofl,
    Rel_desc *rsp, Sym_desc *sdp)
{
	Gotndx	gn, *gnp;
	uint_t	gotents;

	if (pgnp)
		return (1);

	if ((gref == GOT_REF_TLSGD) || (gref == GOT_REF_TLSLD))
		gotents = 2;
	else
		gotents = 1;

	gn.gn_addend = 0;
	gn.gn_gotndx = ofl->ofl_gotcnt;
	gn.gn_gotref = gref;

	ofl->ofl_gotcnt += gotents;

	if (gref == GOT_REF_TLSLD) {
		if (ofl->ofl_tlsldgotndx == NULL) {
			if ((gnp = libld_malloc(sizeof (Gotndx))) == NULL)
				return (S_ERROR);
			(void) memcpy(gnp, &gn, sizeof (Gotndx));
			ofl->ofl_tlsldgotndx = gnp;
		}
		return (1);
	}

	/*
	 * GOT indexes are maintained on an Alist, where there is typically
	 * only one index.  The use of this list is to scan the list to find
	 * an index, and then apply that index immediately to a relocation.
	 * Thus there are no external references to these GOT index structures
	 * that can be comprimised by the Alist being reallocated.
	 */
	if (alist_append(alpp, &gn, sizeof (Gotndx), AL_CNT_SDP_GOT) == NULL)
		return (S_ERROR);

	return (1);
}

static void
ld_assign_plt_ndx(Sym_desc * sdp, Ofl_desc *ofl)
{
	sdp->sd_aux->sa_PLTndx = 1 + ofl->ofl_pltcnt++;
	sdp->sd_aux->sa_PLTGOTndx = ofl->ofl_gotcnt++;
	ofl->ofl_flags |= FLG_OF_BLDGOT;
}

/*
 * Set up the PLT/PLTGOT.
 *
 * Each entry in the PLT is, from the AEABI:
 *
 *	add	ip,  pc, #__PLTGOT(X) & 0x0ff00000
 *	add	ip,  ip, #__PLTGOT(X) & 0x000ff000
 *	ldr	pc, [ip, #__PLTGOT(X) & 0x00000fff]!
 *
 * Where __PLTGOT(X) is the displacement between the GOT entry for X and the
 * PLT entry for X.  Thus, ip = pc+<displacement>, and pc = *ip, branching to
 * the address pointed to by the GOT entry matching this PLT entry.
 *
 * At startup, the GOT entry for every entry in the PLT is the address of
 * plt[0], a reserved entry containing the code:
 *
 *	push	{lr}		;
 *	ldr	lr, [pc, #4]	; Load effective PLT / GOT displacement
 *	add	lr, pc, lr	; lr = &_GLOBAL_OFFSET_TABLE_
 *	ldr	pc, [lr, #8]!	; pc = _GLOBAL_OFFSET_TABLE_[2]
 *	.word	_GLOBAL_OFFSET_TABLE_ - .
 *
 *	NB: ip is preserved.
 *
 * GOT[2] is M_GOT_XRTLD, and will be the address of elf_rtbndr.
 *
 * Thus when we call 'foo':
 *   - we execute the PLT entry
 *      - store to ip the address of the GOT entry
 *      - jump to plt[0]
 *   - plt[0] jumps to elf_rtbndr, preserving ip
 *   - elf_rtbndr stores the address of 'foo' in *ip,
 *     such that future calls are direct, then jumps to it calling 'foo'.
 */
static uintptr_t
ld_fillin_gotplt(Ofl_desc *ofl)
{
	ofl_flag_t	flags = ofl->ofl_flags;
	int		bswap = (ofl->ofl_flags1 & FLG_OF1_ENCDIFF) != 0;

	if (ofl->ofl_osgot) {
		Sym_desc	*sdp;

		if ((sdp = ld_sym_find(MSG_ORIG(MSG_SYM_DYNAMIC_U),
		    SYM_NOHASH, NULL, ofl)) != NULL) {
			uchar_t	*genptr;

			genptr = ((uchar_t *)ofl->ofl_osgot->os_outdata->d_buf +
			    (M_GOT_XDYNAMIC * M_GOT_ENTSIZE));
			/* LINTED */
			*(Word *)genptr = (Word)sdp->sd_sym->st_value;
			if (bswap)
				/* LINTED */
				*(Word *)genptr =
				    /* LINTED */
				    ld_bswap_Word(*(Word *)genptr);
		}
	}

	if ((flags & FLG_OF_DYNAMIC) && (ofl->ofl_osplt != NULL)) {
		static uint32_t entry[] = {
			0xe52de004, /* push	{lr} */
			0xe59fe004, /* ldr	lr, [pc, #4] */
			0xe08fe00e, /* add	lr, pc, lr */
			0xe5bef008, /* ldr	pc, [lr, #8]! */
			0x00000000, /* .word	GOT - . ; (placeholder) */

		};
		uchar_t *plt0 = (uchar_t *)ofl->ofl_osplt->os_outdata->d_buf;
		int i;

		entry[4] = (uint32_t)(ofl->ofl_osgot->os_shdr->sh_addr -
		    (ofl->ofl_osplt->os_shdr->sh_addr + 0x10));

		for (i = 0; i < 5; i++, plt0 += M_PLT_INSSIZE) {
			/* LINTED */
			*(Word *)plt0 = entry[i];
			if (bswap)
				/* LINTED */
				*(Word *)plt0 = ld_bswap_Word(*(Word *)plt0);
		}
	}

	return (1);
}

/* void (*)(void) function */
static const uchar_t nullfunc_tmpl[] = {
	0xe5, 0x2d, 0xb0, 0x04,	/* push	{fp} */
	0xe2, 0x8d, 0xb0, 0x00, /* add	fp, sp, #0 */
	0xe2, 0x8b, 0xd0, 0x00, /* add	sp, fp, #0 */
	0xe8, 0xbd, 0x08, 0x00, /* pop	{fp} */
	0xe1, 0x2f, 0xff, 0x1e, /* bx	lr */
};

const Target *
ld_targ_init_arm(void)
{
	static const Target _ld_targ = {
		.t_m = {
			.m_mach			= M_MACH,
			.m_machplus		= M_MACHPLUS,
			.m_flagsplus		= M_FLAGSPLUS,
			.m_class		= M_CLASS,
			.m_data			= M_DATA,

			.m_segm_align		= M_SEGM_ALIGN,
			.m_segm_origin		= M_SEGM_ORIGIN,
			.m_segm_aorigin		= M_SEGM_AORIGIN,
			.m_dataseg_perm		= M_DATASEG_PERM,
			.m_stack_perm		= M_STACK_PERM,
			.m_word_align		= M_WORD_ALIGN,
			.m_def_interp		= MSG_ORIG(MSG_PTH_RTLD),

			.m_r_arrayaddr		= M_R_ARRAYADDR,
			.m_r_copy		= M_R_COPY,
			.m_r_glob_dat		= M_R_GLOB_DAT,
			.m_r_jmp_slot		= M_R_JMP_SLOT,
			.m_r_num		= M_R_NUM,
			.m_r_none		= M_R_NONE,
			.m_r_relative		= M_R_RELATIVE,
			.m_r_register		= M_R_REGISTER,

			.m_rel_dt_count		= M_REL_DT_COUNT,
			.m_rel_dt_ent		= M_REL_DT_ENT,
			.m_rel_dt_size		= M_REL_DT_SIZE,
			.m_rel_dt_type		= M_REL_DT_TYPE,
			.m_rel_sht_type		= M_REL_SHT_TYPE,

			.m_got_entsize		= M_GOT_ENTSIZE,
			.m_got_xnumber		= M_GOT_XNumber,

			.m_plt_align		= M_PLT_ALIGN,
			.m_plt_entsize		= M_PLT_ENTSIZE,
			.m_plt_reservsz		= M_PLT_RESERVSZ,
			.m_plt_shf_flags	= M_PLT_SHF_FLAGS,

			.m_sht_unwind		= SHT_PROGBITS,

			.m_dt_register		= M_DT_REGISTER
		},
		.t_id = {
			.id_array	= M_ID_ARRAY,
			.id_bss		= M_ID_BSS,
			.id_cap		= M_ID_CAP,
			.id_capinfo	= M_ID_CAPINFO,
			.id_capchain	= M_ID_CAPCHAIN,
			.id_data	= M_ID_DATA,
			.id_dynamic	= M_ID_DYNAMIC,
			.id_dynsort	= M_ID_DYNSORT,
			.id_dynstr	= M_ID_DYNSTR,
			.id_dynsym	= M_ID_DYNSYM,
			.id_dynsym_ndx	= M_ID_DYNSYM_NDX,
			.id_got		= M_ID_GOT,
			.id_gotdata	= M_ID_UNKNOWN,
			.id_hash	= M_ID_HASH,
			.id_interp	= M_ID_INTERP,
			.id_lbss	= M_ID_UNKNOWN,
			.id_ldynsym	= M_ID_LDYNSYM,
			.id_note	= M_ID_NOTE,
			.id_null	= M_ID_NULL,
			.id_plt		= M_ID_PLT,
			.id_rel		= M_ID_REL,
			.id_strtab	= M_ID_STRTAB,
			.id_syminfo	= M_ID_SYMINFO,
			.id_symtab	= M_ID_SYMTAB,
			.id_symtab_ndx	= M_ID_SYMTAB_NDX,
			.id_text	= M_ID_TEXT,
			.id_tls		= M_ID_TLS,
			.id_tlsbss	= M_ID_TLSBSS,
			.id_unknown	= M_ID_UNKNOWN,
			.id_unwind	= M_ID_UNWIND,
			.id_unwindhdr	= M_ID_UNWINDHDR,
			.id_user	= M_ID_USER,
			.id_version	= M_ID_VERSION,
		},
		.t_nf = {
			.nf_template	= nullfunc_tmpl,
			.nf_size	= sizeof (nullfunc_tmpl),
		},
		.t_ff = {
			/*
			 * XXXARM: This will use 0x0, which is a nop-ish
			 * andeq.  The _preferred_ nop is mov r0, r0
			 */
			.ff_execfill	= NULL,
		},
		.t_mr = {
			.mr_reloc_table			= reloc_table,
			.mr_init_rel			= ld_init_rel,
			.mr_mach_eflags			= ld_mach_eflags,
			.mr_mach_make_dynamic		= ld_mach_make_dynamic,
			.mr_mach_update_odynamic	= ld_mach_update_odynamic,
			.mr_calc_plt_addr		= ld_calc_plt_addr,
			.mr_perform_outreloc		= ld_perform_outreloc,
			.mr_do_activerelocs		= ld_do_activerelocs,
			.mr_add_outrel			= ld_add_outrel,
			.mr_reloc_register		= NULL,
			.mr_reloc_local			= ld_reloc_local,
			.mr_reloc_GOTOP			= ld_reloc_GOTOP,
			.mr_reloc_TLS			= ld_reloc_TLS,
			.mr_assign_got			= NULL,
			.mr_find_got_ndx		= ld_find_got_ndx,
			.mr_calc_got_offset		= ld_calc_got_offset,
			.mr_assign_got_ndx		= ld_assign_got_ndx,
			.mr_assign_plt_ndx		= ld_assign_plt_ndx,
			.mr_allocate_got		= NULL,
			.mr_fillin_gotplt		= ld_fillin_gotplt,
		},
		.t_ms = {
			.ms_reg_check		= NULL,
			.ms_mach_sym_typecheck	= NULL,
			.ms_is_regsym		= NULL,
			.ms_reg_find		= NULL,
			.ms_reg_enter		= NULL,
		}
	};

	return (&_ld_targ);
}
