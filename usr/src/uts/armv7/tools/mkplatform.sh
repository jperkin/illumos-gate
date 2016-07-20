#!/bin/bash
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

# Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
# Copyright (c) 2013 Joyent, Inc.  All rights reserved.
# Use is subject to license terms.

#
# XXX This shouldn't live here, but for now it's in the tree so we can make
# building and development more convenient
#

unalias -a

mkp_arg0=$(basename $0)
mkp_tmpdir="/tmp/create_ramdisk.$$"
mkp_rdfile="$mkp_tmpdir/rd.file.32"
mkp_rdmnt="$mkp_tmpdir/rd.mnt.32"
mkp_size=$((1024*24))            # 24MB
mkp_lofi=
mkp_unix=
mkp_genunix=
mkp_upath=""
mkp_genpath="/kernel"

function fatal
{
	local msg="$*"
	[[ -z "$msg" ]] && msg="failed"
	echo "$msg" >&2
	exit 1
}

function cleanup
{
	umount -f "$mkp_rdfile" 2>/dev/null
	lofiadm -d ${mkp_rdfile} 2>/dev/null
	[[ -n "$mkp_tmpdir" ]] && rm -rf "$mkp_tmpdir" 2>/dev/null
}

function setup
{
	rm -rf "$mkp_tmpdir"
	mkdir "$mkp_tmpdir" || fatal "failed to create temp dir: $mkp_tmpdir"
	trap 'cleanup' EXIT
	mkfile "$mkp_size"k "$mkp_rdfile" || fatal "failed to create file"
	chmod 644 $mkp_rdfile
	chown ${USER:-root} $mkp_rdfile || fatal "failed to chown"
	mkdir $mkp_rdmnt || fatal "failed to make mount point"
}

function setupufs
{
	mkp_lofi=$(lofiadm -a "$mkp_rdfile")
	[[ $? -eq 0 ]] || fatal "failed to setup lofi"

	newfs -o space -m 0 -i 12248 -b 4096 $mkp_lofi < /dev/null
	[[ $? -eq 0 ]] || fatal "failed to newfs"
	mount -F ufs -o nologging $mkp_lofi $mkp_rdmnt
}

function copyfiles
{
	mkdir -p "$mkp_rdmnt/$mkp_upath" || fatal "failed to create $mkp_upath"
	cp $mkp_unix "$mkp_rdmnt/$mkp_upath/" || fatal "failed to copy unix"
	mkdir -p "$mkp_rdmnt/$mkp_genpath" || fatal \
	    "failed to create $mkp_genpath"
	cp $mkp_genunix "$mkp_rdmnt/$mkp_genpath/" || fatal "failed to copy unix"
}

function teardown
{
	umount -f "$mkp_rdmnt"
	rmdir "$mkp_rdmnt"	
	lofiadm -d "$mkp_rdfile"
	rm -f boot_archive
	cp $mkp_rdfile boot_archive
	chown ${USER:-root} boot_archive
}

while getopts u: name; do
    case $name in
        u) mkp_upath="$OPTARG";;
        ?) printf "Usage: %s -u <unix path>\n" $0
           exit 2;;
    esac
done

shift $(($OPTIND - 1))

[[ -n "$1" ]] || fatal "missing name of unix"
mkp_unix="$1"
[[ -n "$2" ]] || fatal "missing name of genunix"
mkp_genunix="$2"
setup
setupufs
copyfiles
teardown
exit 0
