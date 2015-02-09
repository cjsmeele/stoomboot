;; \file
;; \brief     Loads the C portion of stage2
;; \author    Chris Smeele
;; \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
;; \license   MIT. See LICENSE for the full license text.

[bits 16]

extern stage2Main

global start

SECTION .data

s_loading: db "stage2", 0x0d, 0x0a, 0

SECTION .text

db 0xfa, 0xf4     ; cli, hlt
db "STAGE2", 0, 0 ; magic!

jmp start

u16_boot_device: dw 0

start:
	mov [u16_boot_device], ax

	cli
	; Set up the stack, again.
	; Both clang and GCC seem to have problems when the stack is in a different
	; segment than the rest of the program.
	xor eax, eax
	mov ss, eax
	mov esp, 0x7dff ; Right below the start of stage2, overwriting stage1.
	; Reset segment registers, just in case.
	xor eax, eax
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	sti

	mov esi, s_loading
	call long puts

	xor eax, eax
	mov ax, [u16_boot_device]

	mov ebp, esp
	push eax
	call long stage2Main ; Call into C.

	cli
	hlt

;; Prints a string in si.
puts:
.loop:
	lodsb
	or al, al
	jz .done

	mov ah, 0x0e
	int 0x10
	jmp .loop
.done:
	ret
