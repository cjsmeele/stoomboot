HAVIK
=====

NAME
----

Havik - An x86 OS development project

DESCRIPTION
-----------

Havik is yet another attempt at creating a simple but functional x86 operating
system, for fun!

This project will consist of the following components:

- A two-stage bootloader with FAT32 support
- A microkernel that supports paged memory management, preemptive multitasking,
  and a good IPC system
- Drivers for disk access, graphics, and more
- Some userspace utilities, if we ever get that far :)

PROJECT STATUS
--------------

Feature / Component       | Status
------------------------- | ------
*Bootloader*              |
-- Stage 1 MBR            | Done
-- Stage 1 FAT32          | Optional, unimplemented
-- Console I/O            | Done
-- Disk I/O               | Done
-- MBR part-table parsing | Done
-- GPT part-table parsing | Optional, unimplemented
-- FAT32 driver           | Done
-- Config file support    | Done
-- Command-line           | Done
-- Memory detection       | Done
-- ELF loading            | In progress
-- Video mode setting     | Done
-- Protected mode switch  | In progress
*Kernel*                  |
-- "Hello world"          |
-- ...                    |

HOW TO RUN
----------

### Requirements

You need to have the following software installed to build Havik:

- GNU make
- A recent GNU binutils and GCC cross-compiler (for i686-elf), see
  [the OSDev wiki](http://wiki.osdev.org/GCC_Cross-Compiler) for instructions
- yasm

To install Havik onto a disk image, you need the following:

- sfdisk - for partitioning disk images
- dosfstools - for creating FAT32 filesystems
- mtools - for copying files onto a FAT32 image
- perl 5.10.1 +

If you plan to debug Havik or run it in a virtual machine, you may also want to
install the following tools:

- bochs
- qemu
- gdb

### Building / Installing

You can build individual Havik components using the following commands:

    make stage1-mbr-bin
    make stage2-bin
    make kernel-bin

Run the following command to create a 128M disk image containing Havik and its
bootloader:

    make disk

You can burn the image to a disk drive or flash drive using dd:

    sudo dd if=img/havik-disk.img of=/dev/$BLOCK_DEVICE

... or simply load it in a virtual machine. The following command runs Havik in
qemu:

    make run

To run Havik headless in qemu using serial I/O over stdio, build and run Havik with
`SERIAL_IO=1` in your environment.

LICENSE
-------

This project is licensed under the MIT license.

The full license text can be found in [LICENSE](LICENSE).

AUTHOR
------

[Chris Smeele](https://github.com/cjsmeele)
