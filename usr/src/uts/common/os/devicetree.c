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
 * Routines to work with the Flattened Device Tree specification that is found
 * across platforms like ARM and RISC-V. Don't worry, it's almost entirely like
 * IEEE 1275. This is used by both unix and various boot services.
 *
 * XXX Not all of the skipping logic is properly doing bounds checking on the
 * data from firmware. Hopefully it created something correct... This should be
 * hardened.
 */

#include <sys/byteorder.h>
#include <sys/devicetree.h>
#include <sys/null.h>
#include <sys/sysmacros.h>

/*
 * XXX ARM doesn't have enough around yet for sys/systm.h to work. For the
 * moment, just declare strlen extern.
 */
extern size_t strlen(const char *);

#define	FDT_MAGIC	0xd00dfeed

#define	FDT_TOK_BEGIN_NODE	0x01
#define	FDT_TOK_END_NODE	0x02
#define	FDT_TOK_PROP		0x03
#define	FDT_TOK_NOP		0x04
#define	FDT_TOK_END		0x09

#define	FDT_VERSION	0x11

/*
 * All values of the header are in big-endian.
 */
typedef struct fdt_header {
	uint32_t	fh_magic;
	uint32_t	fh_totalsize;
	uint32_t	fh_off_dt_struct;
	uint32_t	fh_off_dt_strings;
	uint32_t	fh_off_mem_rsvmap;
	uint32_t	fh_version;
	uint32_t	fh_last_comp_version;
	uint32_t	fh_boot_cpuid_phys;
	uint32_t	fh_size_dt_strings;
	uint32_t	fh_size_dt_struct;
} fdt_header_t;

/*
 * This is used for memory reservation entries.
 */
typedef struct fdt_reserve_entry {
	uint64_t	fre_address;
	uint64_t	fre_size;
} fdt_reserve_entry_t;

fdt_error_t
fdt_init(uintptr_t addr, fdt_t *fdtp)
{
	uint32_t size, structs, strings;
	uint32_t structlen, stringlen;
	uint32_t *sp;

	fdt_header_t *head = (fdt_header_t *)addr;

	if (be32toh(head->fh_magic) != FDT_MAGIC) {
		return (FDT_E_BADMAGIC);
	}

	if (be32toh(head->fh_version) != FDT_VERSION) {
		return (FDT_E_BADVERSION);
	}

	size = be32toh(head->fh_totalsize);

	if (size < sizeof (fdt_header_t)) {
		return (FDT_E_BADSIZE);
	}

	structs = be32toh(head->fh_off_dt_struct);
	strings = be32toh(head->fh_off_dt_strings);
	structlen = be32toh(head->fh_size_dt_struct);
	stringlen = be32toh(head->fh_size_dt_strings);

	if (structs > size || structlen > size ||
	    structs + structlen > size ||
	    structs + structlen < structs) {
		return (FDT_E_BADSTRUCT);
	}

	if (strings > size || stringlen > size ||
	    strings + stringlen > size ||
	    strings + stringlen < strings) {
		return (FDT_E_BADSTRING);
	}

	fdtp->fdt_header = addr;
	fdtp->fdt_structs = (void *)(addr + structs);
	fdtp->fdt_structlen = structlen;
	fdtp->fdt_strings = (void *)(addr + strings);
	fdtp->fdt_stringlen = stringlen;

	/*
	 * Check to make sure we start with a BEGIN NODE token and that the
	 * struct section ends with a STRUCT END section. There may be NOPs
	 * before the first BEGIN NODE.
	 */
	sp = fdtp->fdt_structs;
	while (be32toh(*sp) == FDT_TOK_NOP) {
		sp++;
		if ((uintptr_t)sp >= (uintptr_t)fdtp->fdt_structs +
		    fdtp->fdt_structlen) {
			return (FDT_E_BADSTRUCT);
		}
	}
	if (be32toh(*sp) != FDT_TOK_BEGIN_NODE) {
		return (FDT_E_BADSTRUCT);
	}
	sp = (uint32_t *)((uintptr_t)fdtp->fdt_structs + fdtp->fdt_structlen -
	    sizeof (uint32_t));
	if (be32toh(*sp) != FDT_TOK_END) {
		return (FDT_E_BADSTRUCT);
	}

	return (FDT_E_OK);
}

static const uint32_t *
fdt_skip_nops(const uint32_t *tokp)
{
	while (be32toh(*tokp) == FDT_TOK_NOP) {
		tokp++;
	}

	return (tokp);
}

static const uint32_t *
fdt_skip_node_name(fdt_t *fdt, const uint32_t *tokp)
{
	size_t len;

	/*
	 * We need to include the NUL character as part of the byte count.
	 */
	len = strlen((const char *)tokp) + 1;
	len = P2ROUNDUP(len, sizeof (uint32_t));
	if ((uintptr_t)tokp + len >= (uintptr_t)fdt->fdt_structs +
	    fdt->fdt_structlen) {
		return (NULL);
	}
	tokp += len / sizeof (uint32_t);

	return (fdt_skip_nops(tokp));
}

static const uint32_t *
fdt_skip_single_prop(fdt_t *fdt, const uint32_t *tokp)
{
	uint32_t len;

	if (be32toh(*tokp) != FDT_TOK_PROP) {
		return (NULL);
	}

	tokp++;
	len = be32toh(*tokp);
	tokp += 2;
	tokp += P2ROUNDUP(len, sizeof (uint32_t)) / sizeof (uint32_t);
	if ((uintptr_t)tokp >= (uintptr_t)fdt->fdt_structs +
	    fdt->fdt_structlen) {
		return (NULL);
	}

	return (fdt_skip_nops(tokp));
}

static const uint32_t *
fdt_skip_node_props(fdt_t *fdt, const uint32_t *tokp)
{
	/*
	 * For a given property there are always three uint32_t values that we
	 * need to skip plus a variable number based on the number of bytes in
	 * the property value.
	 */
	while (be32toh(*tokp) == FDT_TOK_PROP) {
		tokp = fdt_skip_single_prop(fdt, tokp);
		if (tokp == NULL) {
			return (NULL);
		}
	}

	return (fdt_skip_nops(tokp));
}

static const uint32_t *
fdt_skip_node(fdt_t *fdt, const uint32_t *tokp)
{
	if (be32toh(*tokp) != FDT_TOK_BEGIN_NODE) {
		return (NULL);
	}

	/*
	 * To find a child node, we must first skip past our node's name. Then
	 * we must skip past all of its properties. If we have a
	 * FDT_TOK_BEGIN_NODE then we are at our immediate child.
	 */
	tokp++;
	tokp = fdt_skip_node_name(fdt, tokp);
	if (tokp == NULL) {
		return (NULL);
	}

	tokp = fdt_skip_node_props(fdt, tokp);
	if (tokp == NULL) {
		return (NULL);
	}

	while (be32toh(*tokp) == FDT_TOK_BEGIN_NODE) {
		tokp = fdt_skip_node(fdt, tokp);
		if (tokp == NULL) {
			return (NULL);
		}
	}

	if (be32toh(*tokp) != FDT_TOK_END_NODE) {
		return (NULL);
	}
	tokp++;

	return (fdt_skip_nops(tokp));
}

fdt_node_t *
fdt_child_node(fdt_t *fdt, fdt_node_t *node)
{
	const uint32_t *tokp;

	tokp = (uint32_t *)node;
	if (be32toh(*tokp) != FDT_TOK_BEGIN_NODE) {
		return (NULL);
	}

	tokp++;
	tokp = fdt_skip_node_name(fdt, tokp);
	if (tokp == NULL) {
		return (NULL);
	}

	tokp = fdt_skip_node_props(fdt, tokp);
	if (tokp == NULL) {
		return (NULL);
	}

	if (be32toh(*tokp) == FDT_TOK_BEGIN_NODE) {
		return ((fdt_node_t *)tokp);
	} else {
		return (NULL);
	}
}

fdt_node_t *
fdt_next_node(fdt_t *fdtp, fdt_node_t *node)
{
	const uint32_t *tokp;

	/*
	 * If we haven't been given a node, go to the start of the tree and find
	 * the first node.
	 */
	if (node == NULL) {
		tokp = fdt_skip_nops(fdtp->fdt_structs);

		if (be32toh(*tokp) != FDT_TOK_BEGIN_NODE) {
			return (NULL);
		}

		return ((fdt_node_t *)tokp);
	}

	/*
	 * The next node is indicated by first finding the end of the current
	 * one. That means skipping over its name, properties, and all of its
	 * children.
	 */
	tokp = (const uint32_t *)node;
	tokp = fdt_skip_node(fdtp, tokp);
	if (tokp == NULL) {
		return (NULL);
	}

	if (be32toh(*tokp) == FDT_TOK_BEGIN_NODE) {
		return ((fdt_node_t *)tokp);
	} else {
		return (NULL);
	}
}

const char *
fdt_node_name(fdt_node_t *node)
{
	uint32_t *tokp = (uint32_t *)node;

	if (be32toh(*tokp) != FDT_TOK_BEGIN_NODE) {
		return (NULL);
	}
	tokp++;
	return ((const char *)tokp);
}

fdt_prop_t *
fdt_next_prop(fdt_t *fdt, fdt_node_t *node, fdt_prop_t *prop)
{
	const uint32_t *tokp;

	if (prop == NULL) {
		tokp = (uint32_t *)node;

		if (be32toh(*tokp) != FDT_TOK_BEGIN_NODE) {
			return (NULL);
		}

		tokp++;
		tokp = fdt_skip_node_name(fdt, tokp);
		if (tokp == NULL) {
			return (NULL);
		}

		if (be32toh(*tokp) == FDT_TOK_PROP) {
			return ((fdt_prop_t *)tokp);
		} else {
			return (NULL);
		}
	}

	tokp = (uint32_t *)prop;
	tokp = fdt_skip_single_prop(fdt, tokp);
	if (tokp == NULL) {
		return (NULL);
	}

	if (be32toh(*tokp) == FDT_TOK_PROP) {
		return ((fdt_prop_t *)tokp);
	} else {
		return (NULL);
	}
}

const char *
fdt_prop_name(fdt_t *fdt, fdt_prop_t *prop)
{
	const uint32_t *tokp;
	uint32_t nameoff;

	tokp = (const uint32_t *)prop;
	if (be32toh(*tokp) != FDT_TOK_PROP) {
		return (NULL);
	}
	tokp += 2;

	nameoff = be32toh(*tokp);
	if (nameoff >= fdt->fdt_stringlen) {
		return (NULL);
	}

	return ((const char *)((uintptr_t)fdt->fdt_strings + nameoff));
}

boolean_t
fdt_prop_len(fdt_t *fdt, fdt_prop_t *prop, uint32_t *lenp)
{
	const uint32_t *tokp;

	tokp = (const uint32_t *)prop;
	if (be32toh(*tokp) != FDT_TOK_PROP) {
		return (B_FALSE);
	}
	tokp++;

	*lenp = be32toh(*tokp);
	return (B_TRUE);
}
