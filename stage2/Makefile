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

# TODO
STAGE2 := $(SRCDIR)/stage2.asm

INFILES := $(STAGE2)

# }}}
# Toolkit {{{

AS := nasm

# }}}
# Toolkit flags {{{

ASFLAGS :=

# }}}

-include Makefile.local

# Make targets {{{

.PHONY: all clean bin

all: bin

bin: $(BINFILE)

$(BINFILE): $(STAGE2)
	$(Q)mkdir -p $(@D)
	$(E) "  AS       $<"
	$(Q)$(AS) $(ASFLAGS) -o $@ $<

clean:
	$(E) "  CLEAN    $(OUTFILES)"
	$(Q)rm -f $(OUTFILES)

# }}}
