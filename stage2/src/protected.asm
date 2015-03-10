;; \file
;; \brief     Switch to protected mode and jump to a 32-bit entrypoint.
;; \author    Chris Smeele
;; \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
;; \license   MIT. See LICENSE for the full license text.

[bits 16]

global enterProtectedMode

SECTION .data

struc GdtEntry
	.limitLow:    resw 1
	.baseLow:     resw 1
	.baseMiddle:  resb 1
	.accessFlags: resb 1
	.flags:       resb 1 ; 4 upper bits for flags, 4 lower bits for limitHigh
	.baseHigh:    resb 1
endstruc

; A flat GDT.
gdt:
	dq 0 ; Null descriptor.

	; Code segment descriptor.
	istruc GdtEntry
		at GdtEntry.limitLow,    dw 0xffff
		at GdtEntry.baseLow,     dw 0x0000
		at GdtEntry.baseMiddle,  db 0x00
		at GdtEntry.accessFlags, db 10011000b        ; Present, ring 0, rx.
		at GdtEntry.flags,       db 1100b << 4 | 0xf ; Page granularity (4K), 32-bits mode.
		at GdtEntry.baseHigh,    db 0x00
	iend

	; Data segment descriptor.
	istruc GdtEntry
		at GdtEntry.limitLow,    dw 0xffff
		at GdtEntry.baseLow,     dw 0x0000
		at GdtEntry.baseMiddle,  db 0x00
		at GdtEntry.accessFlags, db 10010010b        ; Present, ring 0, rw.
		at GdtEntry.flags,       db 1100b << 4 | 0xf ; Page granularity (4K), 32-bits mode.
		at GdtEntry.baseHigh,    db 0x00
	iend
	.end:

gdtPtr:
	dw gdt.end - gdt - 1
	dd gdt

u32_magic:          dd 0x2badb002 ; Claim to be multiboot compliant.
u32_bootInfoStruct: dd 0x00020000 ; TODO: Generate a multiboot-info structure.

SECTION .text

enterProtectedMode:
	cli

	mov edx, [esp + 4]

	lgdt [gdtPtr]
	mov eax, cr0
	or al, 1
	mov cr0, eax ; Enable protected mode.

	; Load data segment selectors.
	mov ax, 2*8 ; The data segment descriptor.
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; Set up a temporary stack at 0x20000 (~64K above stage2).
	; The kernel should set up its own stack when started.
	mov esp, 0x20000

	; There's no way back now!

	mov eax, [u32_magic]
	mov ebx, [u32_bootInfoStruct]

	; Clean up after ourselves.
	xor ecx, ecx
	xor edi, edi
	xor esi, esi
	xor ebp, ebp

	jmp 0x08:.entry32

[bits 32]

.entry32:
	jmp edx ; Good luck, kernel!
