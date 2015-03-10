;; \file
;; \brief     Loads the Havik kernel.
;; \author    Chris Smeele
;; \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
;; \license   MIT. See LICENSE for the full license text.

[bits 32]

global start

extern main

jmp start

MULTIBOOT_MEMORY_INFO  equ  1 << 1
MULTIBOOT_MAGIC        equ  0x1badb002
MULTIBOOT_FLAGS        equ  MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM     equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; Be multiboot compatible.
section ._multiboot_header

align 4
	dd MULTIBOOT_MAGIC
	dd MULTIBOOT_FLAGS
	dd MULTIBOOT_CHECKSUM

start:
	push eax
	push ebx
	call main
.hang:
	cli
	hlt
	jmp .hang
