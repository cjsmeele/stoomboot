[bits 16]
[org 0x7c00]

jmp 0x0000:start ; Far jump to start.

u64_stage2_lba: dq 0 ; The 64-bit stage2 LBA, to be filled in by the installer.

s_loading: db "Loading... ", 0

;; Prints si.
puts:
	lodsb
	or al, al
	jz .done
	mov ah, 0x0e
	mov bx, 0x0007
	int 0x10
	jmp puts
	.done:
		ret

;; Entrypoint.
start:
	cli
	mov ax, 0x9000
	mov ss, ax
	mov sp, 0xfb00
	sti

	mov si, s_loading
	call puts

	cli
	hlt

times 510 - ($ - $$) db 0
dw 0xaa55
