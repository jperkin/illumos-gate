#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#

#
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# Copyright 2026 Edgecast Cloud LLC.
#

if [ $# != 1 ]; then
	echo expected one argument: '<'dtrace-path'>'
	exit 2
fi

dtrace=$1
DIR=/var/tmp/dtest.$$

mkdir $DIR
cd $DIR

cat > prov.d <<EOF
provider test_prov {
	probe zero();
	probe one(uintptr_t);
	probe u_nder(char *);
	probe d__ash(int **);
	probe konst(const char *);
	probe constptr(char * const);
	probe constboth(const char * const);
	probe constpp(char ** const);
	probe constmid(char * const *);
	probe vol(volatile int);
	probe restr(char * restrict);
};
EOF

$dtrace -h -s prov.d
if [ $? -ne 0 ]; then
	print -u2 "failed to generate header file"
	exit 1
fi

for pat in \
    '___konst(const char \*)' \
    '___constptr(char \*const)' \
    '___constboth(const char \*const)' \
    '___constpp(char \*\*const)' \
    '___constmid(char \*const \*)' \
    '___vol(volatile int)' \
    '___restr(char \*restrict)'; do
	if ! grep -q "$pat" prov.h; then
		print -u2 "wrong type qualifier in generated header: expected $pat"
		exit 1
	fi
done

cat > test.c <<EOF
#include <sys/types.h>
#include "prov.h"

int
main(int argc, char **argv)
{
	TEST_PROV_ZERO();
	TEST_PROV_ONE(1);
	TEST_PROV_U_NDER("hi there");
	TEST_PROV_D_ASH(argv);
	TEST_PROV_KONST("hi there");
	TEST_PROV_CONSTPTR(argv[0]);
	TEST_PROV_CONSTBOTH("hi there");
	TEST_PROV_CONSTPP(argv);
	TEST_PROV_VOL(1);
	TEST_PROV_RESTR(argv[0]);
}
EOF

gcc -m32 -c test.c
if [ $? -ne 0 ]; then
	print -u2 "failed to compile test.c"
	exit 1
fi
$dtrace -G -32 -s prov.d test.o
if [ $? -ne 0 ]; then
	print -u2 "failed to create DOF"
	exit 1
fi
gcc -m32 -o test test.o prov.o
if [ $? -ne 0 ]; then
	print -u2 "failed to link final executable"
	exit 1
fi

cat > agg.d <<EOF
provider agg_prov {
	probe cstruct(const struct cs_s { int a; } *);
	probe vunion(volatile union vu_u { int a; long b; } *);
	probe cenum(const enum ce_e { CE_A, CE_B } *);
};
EOF

$dtrace -h -s agg.d
if [ $? -ne 0 ]; then
	print -u2 "failed to generate header file"
	exit 1
fi

for pat in \
    '___cstruct(const struct cs_s \*)' \
    '___vunion(volatile union vu_u \*)' \
    '___cenum(const enum ce_e \*)'; do
	if ! grep -q "$pat" agg.h; then
		print -u2 "wrong type qualifier in generated header: expected $pat"
		exit 1
	fi
done

cd /
/usr/bin/rm -rf $DIR
