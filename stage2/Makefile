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

# }}}
# Package metadata {{{

PACKAGE_NAME    := stage2
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

BINFILE := $(BINDIR)/$(PACKAGE_NAME).bin

OUTFILES := $(BINFILE)

# }}}
# Source and intermediate files {{{

STAGE2   := $(SRCDIR)/stage2.asm
ASMFILES := $(shell find -L src -iname '*.asm')

INFILES := $(ASMFILES)

# }}}
# Toolkit {{{

AS := yasm

# }}}
# Toolkit flags {{{

ASFLAGS :=

# }}}

-include Makefile.local

# Make targets {{{

.PHONY: all clean bin

all: bin

bin: $(BINFILE)

$(BINFILE): $(ASMFILES)
	$(Q)mkdir -p $(@D)
	$(E) "  AS       $(STAGE2)"
	$(Q)$(AS) $(ASFLAGS) -o $@ $(STAGE2)

clean:
	$(E) "  CLEAN    $(OUTFILES)"
	$(Q)rm -f $(OUTFILES)

# }}}
