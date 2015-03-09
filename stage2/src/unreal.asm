;; \file
;; \brief     Switch to unreal mode.
;; \author    Chris Smeele
;; \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
;; \license   MIT. See LICENSE for the full license text.

[bits 16]

global enableUnrealMode

SECTION .data

struc GdtEntry
	.limitLow:    resw 1
	.baseLow:     resw 1
	.baseMiddle:  resb 1
	.accessFlags: resb 1
	.flags:       resb 1 ; 4 upper bits for flags, 4 lower bits for limitHigh
	.baseHigh:    resb 1
endstruc

gdt:
	dq 0 ; Null descriptor.
.dataSelector:
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

a20TestWord:
	dw 0x55aa

SECTION .text

enableUnrealMode:
	push ebp
	mov ebp, esp

	call enableA20

	cli

	push ds

	lgdt [gdtPtr]
	mov eax, cr0
	or al, 1
	mov cr0, eax ; Enable protected mode.

	mov bx, 1 * 8
	mov ds, bx ; Load data segment selector.

	and al, 0xfe
	mov cr0, eax ; Disable protected mode.

	pop ds

	sti

	mov esp, ebp
	pop ebp
	ret

testA20:
	push gs
	push edi
	push esi

	mov ax, 0xffff
	mov gs, ax
	mov esi, a20TestWord
	mov edi, a20TestWord + 16

	; When A20 is disabled, ffff:edi should wrap around to 0000:esi.

	mov ax, [ds:si]
	cmp ax, [gs:di]
	mov ax, 1
	jne .notEqual

.equal:
	; There's a small chance that a >1M address contains the same value as a20TestWord,
	; double check by rotating a20TestWord and comparing the values again.
	ror word [ds:si], 8

	mov ax, [ds:si]
	cmp ax, [gs:di]
	mov ax, 1
	jne .notEqualAfterModify

.equalAgain:
	xor ax, ax

.notEqualAfterModify:
	rol word [ds:si], 8

.notEqual:
	pop esi
	pop edi
	pop gs

	ret

SECTION .data

s_a20Error: db "stage2 fatal: Could not enable a20 gate!", 0x0d, 0x0a, 0

SECTION .text

enableA20:
	cli

	call testA20
	or ax, ax
	jnz .haveA20

	mov ax, 0x2401 ; Enable A20 gate using BIOS calls.
	int 0x15

	call testA20
	or ax, ax
	jnz .haveA20

	in al, 0x92 ; Enable A20 gate using System Control Port A.
	or al, 2
	and al, 0xfe ; Avoid triggering a system reset through bit 0.
	out 0x92, al

	call testA20
	or ax, ax
	jnz .haveA20

	mov eax, s_a20Error ; Give up.
	push eax
	extern puts
	call long puts

.hang:
	cli
	hlt
	jmp .hang

.haveA20:
	sti
	ret
