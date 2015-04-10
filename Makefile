# Copyright (c) 2014, 2015, Chris Smeele
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

# }}}
# Package metadata {{{

PACKAGE_NAME    := stoomboot
PACKAGE_VERSION := $(shell git describe --always --dirty)

ifndef PACKAGE_VERSION
PACKAGE_VERSION := 9999
endif

# }}}
# Directories {{{

STAGE1DIR := stage1
STAGE2DIR := stage2

# }}}
# Source and intermediate files {{{

STAGE1_MBR_BIN   := $(STAGE1DIR)/bin/$(PACKAGE_NAME)-stage1-mbr.bin
STAGE1_FAT32_BIN := $(STAGE1DIR)/bin/$(PACKAGE_NAME)-stage1-fat32.bin
STAGE2_BIN       := $(STAGE2DIR)/bin/$(PACKAGE_NAME)-stage2.bin

# }}}

-include Makefile.local

# Make targets {{{

.PHONY: all clean clean-all stage1-mbr-bin stage1-fat32-bin stage2-bin

all: $(STAGE1_MBR_BIN) $(STAGE2_BIN)

clean:
	$(E) "  CLEAN    $(OUTFILES)"
	$(Q)rm -f $(OUTFILES)

clean-all: clean
	$(Q)$(MAKE) -C $(STAGE1DIR) clean
	$(Q)$(MAKE) -C $(STAGE2DIR) clean

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

# }}}
