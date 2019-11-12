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

#ifndef _SYS_DEVICETREE_H
#define	_SYS_DEVICETREE_H

/*
 * This implements logic to take apart and iterate a flattened device tree. This
 * is similar in spirit to the traditional IEEE 1275 device tree; however, we
 * get it as a single flattened, binary data structure. On these platforms (ARM,
 * RISC-V, etc.) there is no firmware to call into like the PROM on SPARC.
 *
 * This contains a mixture of defintions for the flattened device tree (fdt) and
 * to interface with it. The assumption is that this is used early in boot by
 * tihngs before unix even exists and then again by unix.
 */

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This structure isn't opaque so that way boot can statically declare it.
 */
typedef struct fdt {
	uintptr_t fdt_header;
	void *fdt_memrsvd;
	void *fdt_structs;
	void *fdt_strings;
	uint32_t fdt_size;
	uint32_t fdt_structlen;
	uint32_t fdt_stringlen;
} fdt_t;

typedef enum {
	FDT_E_OK	= 0x00,
	FDT_E_BADMAGIC,
	FDT_E_BADVERSION,
	FDT_E_BADSIZE,
	FDT_E_BADSTRUCT,
	FDT_E_BADSTRING,
	FDT_E_BADMEMRSVD,
	FDT_E_INVALID
} fdt_error_t;

typedef struct fdt_node fdt_node_t;
typedef struct fdt_prop fdt_prop_t;
typedef struct fdt_memrsvd fdt_memrsvd_t;

extern fdt_error_t fdt_init(uintptr_t, fdt_t *);
extern uint32_t fdt_size(fdt_t *);

extern fdt_node_t *fdt_find_node(fdt_t *, const char *);
extern fdt_node_t *fdt_root_node(fdt_t *);
extern fdt_node_t *fdt_next_node(fdt_t *, fdt_node_t *);
extern fdt_node_t *fdt_child_node(fdt_t *, fdt_node_t *);
extern const char *fdt_node_name(fdt_node_t *);

extern fdt_prop_t *fdt_find_prop(fdt_t *, fdt_node_t *, const char *);
extern fdt_prop_t *fdt_next_prop(fdt_t *, fdt_node_t *, fdt_prop_t *);
extern const char *fdt_prop_name(fdt_t *, fdt_prop_t *);
extern boolean_t fdt_prop(fdt_t *, fdt_prop_t *, uint32_t *, void **);

extern fdt_memrsvd_t *fdt_next_memrsvd(fdt_t *, fdt_memrsvd_t *);
extern void fdt_memrsvd(fdt_memrsvd_t *, uint64_t *, uint64_t *);


#ifdef __cplusplus
}
#endif

#endif /* _SYS_DEVICETREE_H */
