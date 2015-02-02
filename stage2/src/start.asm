; @file
; @brief     Loads the C portion of stage2
; @author    Chris Smeele
; @copyright Copyright (c) 2015, Chris Smeele

[bits 16]

extern stage2Main

global start

SECTION .data

s_loading: db "stage2", 0x0d, 0x0a, 0

SECTION .text

db 0xfa, 0xf4     ; cli, hlt
db "STAGE2", 0, 0 ; magic!

start:
	push ax ; boot disk number (usually 80h).
	mov si, s_loading
	call puts
	pop ax

	mov ebp, esp

	push ax
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
