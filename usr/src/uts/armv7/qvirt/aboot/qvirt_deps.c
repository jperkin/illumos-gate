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
 * Copyright 2019 Robert Mustacchi
 */

/*
 * This implements the 'aboot' backend for the QEMU virtual board.
 *
 * XXX This should really just be replaced with a boot console driver managed by
 * aboot and the fdt.
 */

#include <aboot/aboot.h>

volatile unsigned int *const qvirt_uart = (void *)0x09000000;

void
aboot_backend_init(void)
{

}

void
aboot_backend_putc(int c)
{
	*qvirt_uart = c & 0x7f;
}

void
aboot_map_console(void)
{
	uintptr_t addr = (uintptr_t)qvirt_uart;
	aboot_map_1mb(addr, addr, ABOOT_P_R | ABOOT_P_W |
	    ABOOT_P_DEV);
	aboot_infop->abi_cons = addr;
	aboot_infop->abi_conslen = 1024 * 1024;
}
