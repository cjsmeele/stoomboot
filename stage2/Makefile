# Copyright (c) 2014, 2015 Chris Smeele
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

PACKAGE_NAME    := havik-stage2
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
CFILES   := $(shell find $(SRCDIR) -name '*.c')
HFILES   := $(shell find $(SRCDIR) -name '*.h')
OBJNAMES := $(ASMFILES:$(SRCDIR)/%.asm=%) $(CFILES:$(SRCDIR)/%.c=%)
OBJFILES := $(OBJNAMES:%=$(OBJDIR)/%.o)
DEPFILES := $(CFILES:$(SRCDIR)/%.c=$(DEPDIR)/%.d)

LDFILE := stage2.ld

INFILES := $(ASMFILES) $(CFILES) $(HFILES)

# }}}
# Output files {{{

BINFILE  := $(BINDIR)/$(PACKAGE_NAME).bin

OUTFILES = $(OBJFILES) $(DEPFILES) $(BINFILE)

# }}}
# Toolkit {{{

AS := yasm
# Unfortunately, using clang with any -O option breaks stage2. I'm unable to
# figure out what might be causing the issue, so we're simply going to use GCC
# here since it works.
CC := i686-elf-gcc
LD := i686-elf-ld

# }}}
# Toolkit flags {{{

CWARNINGS := \
	-Wall \
	-Wextra \
	-Wpointer-arith \
	-Wwrite-strings \
	-Wmissing-declarations \
	-Wno-long-long \
	-Wuninitialized \
	-Wpadded \
	-Wno-format

ASFLAGS := -f elf32
CFLAGS  := \
	-fdiagnostics-color=auto \
	-Os \
	-g0 \
	-m16 \
	-nostdlib \
	-ffreestanding \
	-fno-stack-protector \
	-Wfatal-errors \
	-std=c11 \
	-I$(SRCDIR) \
	$(CWARNINGS)

ifdef SERIAL_IO
CFLAGS += -DCONFIG_CONSOLE_SERIAL_IO=1
endif

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
-include $(CFILES:$(SRCDIR)/%.c=$(DEPDIR)/%.d)
endif

bin: $(BINFILE)

$(BINFILE): $(OBJFILES) $(LDFILE)
	$(E) "  LD   $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(LD) $(LDFLAGS) -T$(LDFILE) -o $@ $(OBJFILES)

$(DEPDIR)/%.d: $(SRCDIR)/%.c
	$(E) "  DEP  $@"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(CFLAGS) -MM -MT $(<:$(SRCDIR)/%.c=$(OBJDIR)/%.o) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	$(Q)mkdir -p $(@D)
	$(E) "  AS   $<"
	$(Q)$(AS) $(ASFLAGS) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(Q)mkdir -p $(@D)
	$(E) "  CC   $<"
	$(Q)$(CC) -o $@ -c $< $(CFLAGS)

doc: $(INFILES) doxygen.conf
	$(Q)doxygen doxygen.conf

clean:
	$(E) "  CLEAN    $(OUTFILES)"
	$(Q)rm -f $(OUTFILES)

# }}}
