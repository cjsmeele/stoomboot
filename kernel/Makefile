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

PACKAGE_NAME    := kernel
PACKAGE_VERSION := $(shell git describe --always --dirty)

ifndef PACKAGE_VERSION
PACKAGE_VERSION := 9999
endif

# }}}
# Directories {{{

SRCDIR := src
OBJDIR := obj
BINDIR := bin
DEPDIR := dep

# }}}
# Output files {{{

BINFILE := $(BINDIR)/$(PACKAGE_NAME).elf

OUTFILES := $(BINFILE)

# }}}
# Source and intermediate files {{{

# TODO
ASMFILES := 
CXXFILES := 

INFILES := $(ASMFILES) $(CXXFILES)

# }}}
# Toolkit {{{

AS  := nasm
CXX := i686-elf-g++

# }}}
# Toolkit flags {{{

ASFLAGS  :=
CXXFLAGS :=

# }}}

-include Makefile.local

# Make targets {{{

.PHONY: all clean bin

all: bin

bin: $(BINFILE)

# TODO
$(BINFILE): $(INFILES)
	$(Q)mkdir -p $(@D)
	$(Q)touch $@

clean:
	$(E) "  CLEAN    $(OUTFILES)"
	$(Q)rm -f $(OUTFILES)

# }}}
