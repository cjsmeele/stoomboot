STOOMBOOT
=========

![Picture of a stoomboot](res/stoomboot.png?raw=true)

NAME
----

Stoomboot - An x86 real mode bootloader

DESCRIPTION
-----------

Stoomboot is a lightweight bootloader for x86 systems, created because writing
a bootloader is a fun learning experience!

If you're looking for a bootloader for anything other than hobby OS
development, please have a look at the LIMITATIONS section for issues you might
encounter if you choose to use Stoomboot.

PROJECT STATUS
--------------

Stoomboot (version 1.3, 2018-11-27) is considered "feature-complete".
That means there is no on-going development right now.

However, if you find a bug, want to request a change, or if you simply
like to ask a question about the project, feel free to open an issue on
Github. Thanks in advance :-)

FEATURES
--------

- More or less Multiboot-compliant
  - Generates and passes to the kernel a `multiboot_info` structure containing
    the following information:
    - Memory map
    - Commandline
    - Boot device
    - Framebuffer and VBE mode info, if set
  - Does not validate or use flags in a multiboot-compliant kernel's multiboot
    header
- Completely runs in the first 64K memory segment (~32K stack, ~33K code + data)
- Supports up to 4 disks with 7 partitions per disk (due to memory constraints)
- Supports graphics / video modesetting using VBE
- Supports FAT32 filesystems
- Has a commandline interface
- Loads a configuration file from disk
- Loads and executes 32-bit ELF binaries from disk

LIMITATIONS
-----------

If you need any of the following features, Stoomboot is probably not for you:

- Support for more than 4 disks and 7 partitions per disk
- Support for any partition table format that isn't DOS MBR
- Long File Name support
- Support for any filesystem that isn't FAT32
- Support for a.out, PE, and other executable file formats
- Support for loading 64-bit kernels (you'll need to write a protected-mode
  stub that switches to long mode yourself)
- Support for fancy menus and colors / background images
- Chainloading, loading anything other than ELF binaries
- Extensibility

Pretty much all of these restrictions stem from the fact that GCC, the chosen
compiler, does not support x86 real mode all that well, and as a result
produces assembly that only works within the first 64K memory segment.

Also, every instruction that does something with 32-bit addresses or 32-bit
data needs a prefix in real mode, resulting in a lot of bloat in the stage2
binary.

Other than that though, it runs just fine :-)

HOW TO USE
----------

### Requirements

You need to have the following software installed to build Stoomboot:

- GNU make
- GNU binutils and a GCC cross-compiler (for i686-elf), see
  [the OSDev wiki](http://wiki.osdev.org/GCC_Cross-Compiler) for instructions
- yasm

To build Stoomboot, simply run:

```
make
```

This will build the stage1 MBR image and the stage2 binary.

If the build fails with relocation errors (*relocation truncated to fit*), this
means gcc failed to optimize enough for size.
You can work around this by decreasing the amount of supported disks or
partitions per disk in stage2/src/disk.h.

### Creating a boot disk

After building stoomboot, you may use it to create a boot disk as follows:

*These steps require mtools, parted and perl 5.10.1*

```bash
#!/bin/bash

# Disk parameters

DISK_SIZE_M=72
DISK_FILE=/tmp/disk.img
DISK_FS="${DISK_FILE}@@1M"
DISK_FSID=42424242
DISK_FSLABEL=STOOMLDR

# The config file, make sure it exists.
LOADER_RCFILE=./loader.rc

# Create an emtpy sparse disk and partition it.
truncate -s${DISK_SIZE_M}M "$DISK_FILE"
parted -s "$DISK_FILE" unit MiB mklabel msdos mkpart primary fat32 1 '100%'

# Format as FAT32.
mformat -i "$DISK_FS" -FN "$DISK_FSID" -v "$DISK_FSLABEL"

# Create /boot and install the configuration file.
# (make sure to customize it first, if needed)
mmd     -i "$DISK_FS" /boot
mcopy   -i "$DISK_FS" "$LOADER_RCFILE" ::/boot/loader.rc

# Install Stoomboot stage1 & stage2.
tools/loader-install.pl \
    --mbr \
    --stage1-mbr   stage1/bin/stoomboot-stage1-mbr.bin \
    --stage2       stage2/bin/stoomboot-stage2.bin \
    --stage2-lba   1 \
    --loader-fs-id "$DISK_FSID" \
    --img          "$DISK_FILE"
```

Then, once you have a built your multiboot-compliant kernel binary, you
can copy it to the disk like so:

```bash
mcopy   -i "$DISK_FS" YOUR_KERNEL_ELF ::/boot
```

Then, try loading the disk image into QEMU:

```bash
qemu-system-i386 -vga std disk.img
```

For large kernel images (e.g. >4M), you may want to enable
acceleration to improve boot times. For example:

```bash
qemu-system-x86_64 -vga std disk.img -enable-kvm
```

LICENSE
-------

This project is licensed under the MIT license.

The full license text can be found in [LICENSE](LICENSE).

AUTHOR
------

[Chris Smeele](https://github.com/cjsmeele)
