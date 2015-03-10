# Copyright (c) 2014, 2015, Chris Smeele
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

PACKAGE_NAME    := havik-kernel
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
# Source and intermediate files {{{

ASMFILES := $(shell find $(SRCDIR) -name '*.asm')
CXXFILES := $(shell find $(SRCDIR) -name '*.cc')
HFILES   := $(shell find $(SRCDIR) -name '*.hh')
OBJNAMES := $(ASMFILES:$(SRCDIR)/%.asm=%) $(CXXFILES:$(SRCDIR)/%.cc=%)
OBJFILES := $(OBJNAMES:%=$(OBJDIR)/%.o)
DEPFILES := $(CXXFILES:$(SRCDIR)/%.cc=$(DEPDIR)/%.d)

LDFILE := kernel.ld

INFILES := $(ASMFILES) $(CXXFILES) $(HFILES)

# }}}
# Output files {{{

BINFILE  := $(BINDIR)/$(PACKAGE_NAME).elf

OUTFILES = $(OBJFILES) $(DEPFILES) $(BINFILE)

# }}}
# Toolkit {{{

AS  := yasm
CXX := i686-elf-g++
LD  := i686-elf-ld

# }}}
# Toolkit flags {{{

CXXWARNINGS := \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wpointer-arith \
	-Wcast-align \
	-Wwrite-strings \
	-Wmissing-declarations \
	-Wredundant-decls \
	-Winline \
	-Wno-long-long \
	-Wuninitialized \
	-Wconversion

ASFLAGS  := -f elf32
CXXFLAGS := \
	-fdiagnostics-color=auto \
	-O0 \
	-g3 \
	-m32 \
	-march=i686 \
	-nostdlib \
	-ffreestanding \
	-fno-stack-protector \
	-Wfatal-errors \
	-fno-exceptions \
	-fno-rtti \
	-std=c++11 \
	-I$(SRCDIR) \
	$(CXXWARNINGS)

LDLIBPATH :=
LDFLAGS    = $(addprefix -L, $(LDLIBPATH))

# }}}

-include Makefile.local

# Make targets {{{

.PHONY: all dep doc clean bin

all: bin

dep: $(DEPFILES)

# Include generated dependencies if we're compiling or linking.
ifeq (,$(filter $(MAKECMDGOALS),dep doc clean))
-include $(CXXFILES:$(SRCDIR)/%.cc=$(DEPDIR)/%.d)
endif

bin: $(BINFILE)

$(BINFILE): $(OBJFILES) $(LDFILE)
	$(E) "  LD   $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(LD) $(LDFLAGS) -T$(LDFILE) -o $@ $(OBJFILES)

$(DEPDIR)/%.d: $(SRCDIR)/%.cc
	$(E)"  DEP  $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(CXX) $(CXXFLAGS) -MM -MT $(<:$(SRCDIR)/%.cc=$(OBJDIR)/%.o) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	$(E) "  AS   $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(AS) $(ASFLAGS) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	$(E)"  CXX  $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CXX) -o $@ -c $< $(CXXFLAGS)

clean:
	$(E) "  CLEAN    $(OUTFILES)"
	$(Q)rm -f $(OUTFILES)

# }}}
