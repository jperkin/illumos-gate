## Cross Building For ARM

This guide details how to set everything up on SmartOS.  The procedure should
be reasonably portable to other illumos distributions, you may just need to
change some paths (and can probably avoid building illumos-extra and instead
just use the native GCC 4.4.4).

### Setup A SmartOS Build Zone

This is the same as a regular SmartOS build zone, as detailed in the wiki
page here: https://wiki.smartos.org/display/DOC/Building+SmartOS+on+SmartOS

For example:

```console
$ imgadm avail | grep e69a0
e69a0918-055d-11e5-8912-e3ceb6df4cf8  base-multiarch-lts      14.4.2      smartos  2015-05-28T17:20:56Z
```

```console
$ imgadm import e69a0918-055d-11e5-8912-e3ceb6df4cf8
```

```console
$ vmadm create <<-EOF
{
  "brand": "joyent",
  "alias": "illumos-arm",
  "image_uuid": "e69a0918-055d-11e5-8912-e3ceb6df4cf8",
  "fs_allowed": "ufs,pcfs,tmpfs",
  "max_physical_memory": 8192,
  "quota": 20480,
  "resolvers": [
    "8.8.8.8",
    "8.8.4.4"
  ],
  "dns_domain": "local",
  "nics": [
    {
      "nic_tag": "admin",
      "ip": "dhcp"
    }
  ]
}
EOF
Successfully created VM 2a02f926-ddf1-4387-987f-e3c7c4cd84c9
```

```console
$ zlogin 2a02f926-ddf1-4387-987f-e3c7c4cd84c9
```

### Install Required Software

For SmartOS:

```console
$ pkgin up
$ pkgin -y install bison flex git-base gcc47 gmake python26 p5-XML-Parser
```

This is very XXX, various ways to tidy this up, but for now it's necessary to
provide yacc for the "cpp" build.  We can't use byacc from pkgsrc as it
conflicts.

```console
$ curl -ko /var/tmp/sgstools.tgz https://download.joyent.com/pub/build/pkgsrc/sgstools.tgz
$ pkg_add -C /dev/null /var/tmp/sgstools.tgz
```

### Build illumos-extra

This is required on SmartOS to get a suitable illumos compiler.  You may
be able to skip this step on e.g. OmniOS and instead use the `/opt/gcc/4.4.4`
compiler.

```console
$ git clone -b arm-gate https://github.com/jperkin/illumos-extra.git ~/illumos-extra-arm
$ cd ~/illumos-extra-arm
$ gmake ARCH=i86pc STRAP=strap install
```

### Build illumos-gate With ARM Support

Fetch the repository into the same directory as illumos-extra-arm

```console
$ git clone -b arm-gate https://github.com/jperkin/illumos-gate.git ~/illumos-gate-arm
$ cd ~/illumos-gate-arm
```

There is an idempotent configure script which will set up the build environment
for you.

```console
$ ./configure
```

The final step of the configure script is to prepare two bldenv config files.
First, enter a native i386 bldenv environment to build native tools with ARM
support.

```console
$ ksh93 usr/src/tools/scripts/bldenv.sh bldenv-i386.conf
```

Build all the required parts.

```console
$ cd usr/src
$ dmake setup
$ cd lib/libc
$ dmake install
$ cd cmd/sgs
$ dmake install
```

Now install the resulting tools into /opt/armtc.

```console
$ mkdir -p /opt/armtc/lib/amd64 /opt/armtc/usr/bin/amd64
$ (cd /opt/armtc/lib/; ln -s amd64 64; cd ../usr/bin; ln -s amd64 64)
$ cd ../../../../proto/root_i386
$ for file in usr/bin/ld usr/bin/amd64/ld \
              lib/libld.so.4 lib/amd64/libld.so.4 \
              lib/liblddbg.so.4 lib/amd64/liblddbg.so.4 \
              lib/libelf.so.1 lib/amd64/libelf.so.1; do
    cp ${file} /opt/armtc/${file}
  done
```

And exit the bldenv shell.

```console
$ exit
```

### Build ARM-native illumos-extra

We now go back into illumos-extra-arm but this time build native ARM tools
using the ARM cross bits we just built.

```console
$ cd ~/illumos-extra-arm
$ gmake ARCH=arm STRAP=strap LD_ALTEXEC=/opt/armtc/usr/bin/ld install
```

Once that's done, you'll need to fix up the rpath there.  On OmniOS you'll
probably want to substitute `/opt/gcc/4.4.4` for `/opt/local/lib` here:

```console
$ ./tools/setrpath proto-arm/usr/ /opt/armtc/usr/lib:/opt/local/lib:/lib:/usr/lib
```

Finally, you can copy all of that into your arm compiler toolchain directory
(use pfexec / sudo as appropriate):

```console
$ cp -r proto-arm/usr /opt/armtc/
```

### Build ARM illumos-gate

Now we can finally perform an ARM-native build using an ARM bldenv.

```console
$ cd ~/illumos-gate-arm
$ ksh93 usr/src/tools/scripts/bldenv.sh bldenv-arm.conf
```

As a part of this you should see an important two lines:

```
Cross-building enabled
Targeting arm on i386
```

If you don't, stop.  Something went wrong and `bldenv-arm.conf` is not
configured correctly.

Once you have that you can get going and build the kernel as far as we have it
for ARM.

```console
$ cd usr/src
$ dmake setup
$ cd uts
$ dmake install
```

You now have a lovely unix and boot_archive pair in bcm2835/unix (Raspberry
Pi) and qvpb/unix (qemu versatilepb).

### Boot

Now that you have the gate built, you can try to boot the kernel.  This is
where things diverge between qemu and the Raspberry Pi.

#### QEMU

Install some build dependencies.

```console
pkgin in pkg-config glib2 pixman
```

Get qemu and build it.

```console
$ git clone https://github.com/jperkin/qemu.git
$ cd qemu
$ git submodule update --init dtc
$ ./configure --cpu=i386 --disable-curses # XXX: build errors
$ gmake install
```

Booting qemu is very easy:

```console
$ qemu-system-arm \
	-kernel $PROTO/platform/qvpb/kernel/loader \
	-initrd $PROTO/platform/qvpb/kernel/initrd \
	-machine versatilepb \
	-cpu arm1176 \
	-m 512 \
	-no-reboot \
	-nographic \
	-append 'kernel /platform/qvpb/kernel/unix -Bconsole=uart'
```

The loader and kernel messages should appear in the same terminal.

#### Hardware

Booting on real hardware is a bit more involved.

  a) Create a FAT16 or FAT32 partition on the SD card.  You'll want it to be
     at least 40 MB.

  b) Create a config.txt on the partition:

	gpu_mem=64
	kernel=loader
	initramfs initrd 0x00800000

  c) Create a cmdline.txt on the partition:

	kernel /platform/bcm2835/kernel/unix -Bconsole=uart

  d) Place Raspberry Pi firmware onto the partition.  You can download
     latest firmware from
     https://github.com/raspberrypi/firmware/tree/master/boot. The firmware
     from January 24th, 2015 is known to work.

	0e52c8cdbfd21631746d6fcdc8f2750af39f4287  bootcode.bin
	aba25d795eaddafd5c8ece3de18873b9928eb6f7  fixup_cd.dat
	38e55d60f896738eec30d0ca4f62b68e48e99184  fixup.dat
	4867e6eab84bb4138e812993112b6a05b7930b89  fixup_x.dat
	fa993851acba366d9e37d59a1d9e9de84b19173f  start_cd.elf
	356060e0f44742d8835294a211b812efcac29f66  start.elf
	b7f01f90d995a36c9d765fd1f4d95a5fcdfd7e41  start_x.elf

  e) Copy $PROTO/platform/bcm2835/kernel/{loader,initrd} onto the partition.
