# Copyright (c) 2014, Chris Smeele
# All rights reserved.

# Parameters {{{

ifdef VERBOSE
Q :=
E := @true 
else
Q := @
E := @echo 
MAKEFLAGS += --no-print-directory
endif

DISKSIZE_M   ?= 128
# Assume the default shell supports arithmetic expansion.
PART_SECTORS := $$(($(DISKSIZE_M) * 1024 * 2 - 2048))

# }}}
# Package metadata {{{

PACKAGE_NAME    := takos
PACKAGE_VERSION := $(shell git describe --always --dirty)

ifndef PACKAGE_VERSION
PACKAGE_VERSION := 9999
endif

# }}}
# Directories {{{

STAGE1DIR := stage1
STAGE2DIR := stage2
KERNELDIR := kernel

FSDIR  := disk
IMGDIR := img

# }}}
# Output files {{{

DISKFILE := $(IMGDIR)/$(PACKAGE_NAME)-disk.img

OUTFILES := $(DISKFILE)

# }}}
# Source and intermediate files {{{

STAGE1_MBR_BIN   := $(STAGE1DIR)/bin/stage1-mbr.bin
STAGE1_FAT32_BIN := $(STAGE1DIR)/bin/stage1-fat32.bin
STAGE2_BIN       := $(STAGE2DIR)/bin/stage2.bin
KERNEL_BIN       := $(KERNELDIR)/bin/kernel.elf

INFILES := $(STAGE1_MBR_BIN) $(STAGE1_FAT32_BIN) $(STAGE2_BIN) $(KERNEL_BIN)

# }}}
# Toolkit {{{

QEMU   ?= qemu-system-i386
SFDISK ?= sfdisk
DD     ?= dd

# }}}
# Toolkit flags {{{

QEMUFLAGS   := -m 64M -name "osdev" -net none -serial stdio -vga std \
	-drive file=$(DISKFILE),if=scsi,media=disk,format=raw

SFDISKFLAGS := -H 64 -S 32

# }}}

-include Makefile.local

# Make targets {{{

.PHONY: all clean clean-all run disk stage1-mbr-bin stage1-fat32-bin stage2-bin kernel-bin

all: $(DISKFILE)

run: $(DISKFILE)
	$(E) "  QEMU     $<"
	$(Q)$(QEMU) $(QEMUFLAGS)

clean:
	$(E) "  CLEAN    $(OUTFILES)"
	$(Q)rm -f $(OUTFILES)

clean-all: clean
	$(Q)$(MAKE) -C $(STAGE1DIR) clean
	$(Q)$(MAKE) -C $(STAGE2DIR) clean
	$(Q)$(MAKE) -C $(KERNELDIR) clean

disk: $(DISKFILE)

# TODO: Install stage1 in FAT32 bootsector.

$(DISKFILE): $(STAGE1_MBR_BIN) $(STAGE2_BIN) $(KERNEL_BIN)
	$(E) ""
	$(E) "Disk Image"
	$(E) "=========="
	$(Q)mkdir -p $(@D)
	$(E) "  DD       $@"
	$(Q)$(DD) $(DDFLAGS) if=/dev/zero of=$@ bs=1M count=$(DISKSIZE_M) 2>/dev/null
	$(E) "  SFDISK   $@"
	$(Q)/bin/echo -e \
	"unit: sectors\n"\
	"\n"\
	"boot.img1 : start = 2048, size= "$(PART_SECTORS)", Id= c, bootable\n"\
	"boot.img2 : start =    0, size= 0, Id= 0\n"\
	"boot.img3 : start =    0, size= 0, Id= 0\n"\
	"boot.img4 : start =    0, size= 0, Id= 0"\
	| $(SFDISK) $(SFDISKFLAGS) $@ >/dev/null
	$(E) "  INSTALL  $@"
	$(Q)perl tools/loader-install.pl \
		--mbr \
		--stage1-mbr $(STAGE1_MBR_BIN) \
		--stage2 $(STAGE2_BIN) \
		--stage2-lba 1 \
		--img $@

$(STAGE1_MBR_BIN): stage1-mbr-bin ;

stage1-mbr-bin:
	$(E) ""
	$(E) "Stage 1 (MBR)"
	$(E) "============="
	$(Q)$(MAKE) -C $(STAGE1DIR) bin-mbr

$(STAGE1_FAT32_BIN): stage1-fat32-bin ;

stage1-fat32-bin:
	$(E) ""
	$(E) "Stage 1 (FAT32)"
	$(E) "==============="
	$(Q)$(MAKE) -C $(STAGE1DIR) bin-fat32

$(STAGE2_BIN): stage2-bin ;

stage2-bin:
	$(E) ""
	$(E) "Stage 2"
	$(E) "======="
	$(Q)$(MAKE) -C $(STAGE2DIR) bin

$(KERNEL_BIN): kernel-bin ;

kernel-bin:
	$(E) ""
	$(E) "Kernel"
	$(E) "======"
	$(Q)$(MAKE) -C $(KERNELDIR) bin

# }}}
