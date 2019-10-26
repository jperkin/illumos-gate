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
 * XXX What am I?
 */

#include <sys/types.h>
#include <aboot/aboot.h>
#include <sys/devicetree.h>
#include <sys/null.h>

static fdt_t aboot_fdt;

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

		if (fdt_prop_len(fdt, prop, &len)) {
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

	aboot_puts("looking at fdt\n");
	node = fdt_next_node(fdt, NULL);
	if (node != NULL) {
		aboot_print_node(fdt, node, 0);
	} else {
		aboot_puts("found no node\n");
	}

	node = fdt_next_node(fdt, node);
	if (node != NULL) {
		aboot_puts("somehow found next node of /!\n");
	}
}

void
aboot_main(uintptr_t arg0, uintptr_t type, uintptr_t fdt)
{
	if (fdt_init(fdt, &aboot_fdt) != FDT_E_OK) {
		aboot_panic("fdt_init failed!");
	}

	aboot_print_fdt(&aboot_fdt);

	aboot_panic("main!");
}
