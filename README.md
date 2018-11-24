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

Stoomboot (version 1.2, 2018-11-24) is considered "feature-complete".
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
    - VBE mode info, if set
  - Does not validate or use flags in a multiboot-compliant kernel's multiboot
    header
- Completely runs in the first 64K memory segment (~32K stack, ~33K code + data)
- Supports up to 4 disks with 7 partitions per disk (due to memory constraints)
- Supports video modesetting using VBE
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

To install the bootloader onto a disk image, one might do the following:

```bash
DISK_SIZE_M=64
PART_SIZE_S=$(($DISK_SIZE_M*1024*1024 / 512 - 2048))
IMG_FILE=/tmp/disk.img
FAT_FILE=/tmp/fs.img

# Create a blank 64M disk image.
truncate -s${DISK_SIZE_M}M "$IMG_FILE"

# Partition the disk image. Reserve 2048 sectors before the first partition.
#echo "2048 $PART_SIZE_S c" | sfdisk -H64 -S32 -uS "$IMG_FILE"
echo "2048 $PART_SIZE_S c" | sfdisk -uS "$IMG_FILE"

# Create a separate image to build a FAT32 filesystem on.
truncate -s$(($PART_SIZE_S * 512)) "$FAT_FILE"

# Format the fat image.
mkfs.vfat -F 32 -i 42424242 -n STOOMLDR "$FAT_FILE"

# Copy the loader configuration file onto the fat image (you may want to customize it first).
mmd /boot -i "$FAT_FILE"

mcopy loader.rc ::/boot -i "$FAT_FILE"

# Copy a kernel binary onto the disk (optional - you may load a kernel from any FAT32 partition on any disk).
mcopy kernel.elf ::/boot -i "$FAT_FILE"

# Install the FAT32 partition into the disk image.
dd if="$FAT_FILE" conv=sparse of="$IMG_FILE" conv=notrunc bs=512 seek=2048 count="$PART_SIZE_S"

# Install stage1 and stage2 onto the disk.
stoomboot/tools/loader-install.pl \
    --mbr \
    --stage1-mbr   stage1/bin/stoomboot-stage1-mbr.bin \
    --stage2       stage2/bin/stoomboot-stage2.bin \
    --stage2-lba   1 \
    --loader-fs-id 42424242 \
    --img          "$IMG_FILE"
```

*Running the example above also requires mtools, sfdisk and perl 5.10.1.*

Try loading the disk image into QEMU:

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
