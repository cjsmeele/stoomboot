[bits 16]
[org 0x7e00]

jmp start

%include "console.asm"

s_loading: db "stage2 ", 0
s_done:    db 0x0a, 0x0d, 0x0a, 0x0d, "done!", 0

start:
	mov si, s_loading
	call puts

	mov si, s_done
	call puts

	; TODO

	cli
	hlt
