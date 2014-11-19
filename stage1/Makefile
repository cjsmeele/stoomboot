# Copyright (c) 2014, Chris Smeele
# All rights reserved.

# Parameters {{{

ifdef VERBOSE
	Q :=
	E := @true 
else
	Q := @
	E := @echo 
endif

# }}}
# Package metadata {{{

PACKAGE_NAME    := stage1
PACKAGE_VERSION := $(shell git describe --always --dirty)

ifndef PACKAGE_VERSION
	PACKAGE_VERSION := 9999
endif

# }}}
# Directories {{{

SRCDIR := src
BINDIR := bin

# }}}
# Output files {{{

BINFILE_MBR   := $(BINDIR)/$(PACKAGE_NAME)-mbr.bin
BINFILE_FAT32 := $(BINDIR)/$(PACKAGE_NAME)-fat32.bin

OUTFILES := $(BINFILE_MBR) $(BINFILE_FAT32)

# }}}
# Source and intermediate files {{{

STAGE1_MBR   := $(SRCDIR)/stage1-mbr.asm
STAGE1_FAT32 := $(SRCDIR)/stage1-fat32.asm

INFILES := $(STAGE1_MBR) $(STAGE1_FAT32)

# }}}
# Toolkit {{{

AS := nasm

# }}}
# Toolkit flags {{{

ASFLAGS :=

# }}}

-include Makefile.local

# Make targets {{{

.PHONY: all clean bin-mbr bin-fat32

all: bin-mbr bin-fat32

bin-mbr: $(BINFILE_MBR)

bin-fat32: $(BINFILE_FAT32)

$(BINFILE_MBR): $(STAGE1_MBR)
	$(Q)mkdir -p $(@D)
	$(E) "  AS       $<"
	$(Q)$(AS) $(ASFLAGS) -o $@ $<

$(BINFILE_FAT32): $(STAGE1_FAT32)
	$(Q)mkdir -p $(@D)
	$(E) "  AS       $<"
	$(Q)$(AS) $(ASFLAGS) -o $@ $<

clean:
	$(E) "  CLEAN    $(OUTFILES)"
	$(Q)rm -f $(OUTFILES)

# }}}
