[bits 16]
[org 0x7c00]

jmp 0x0000:start ; Far jump to start.

u8_boot_device:
	db 0

;; Disk address packet structure.
struct_dap:
	db 0x10
	db 0
	db 0 ; Blocks to read, to be filled in by the installer.
	db 0

	dw 0x7e00 ; Destination address.
	dw 0x0000 ; Segment.

	dq 0 ; The stage2 LBA, to be filled in by the installer.


s_loading:    db 0x0a, 0x0d, "Loading... stage1 ", 0
s_disk_error: db 0x0a, 0x0d, "Error: Could not read boot disk for stage2! :(", 0

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
	; Set up the stack.
	cli
	mov ax, 0x9000
	mov ss, ax
	mov sp, 0xfbff
	sti

	mov [u8_boot_device], dl

	mov si, s_loading
	call puts

	.load_stage2:
		mov si, struct_dap
		mov ah, 0x42
		mov dl, [u8_boot_device]
		int 0x13
		jc .error
		jmp 0x0000:0x7e00
		.error:
			mov si, s_disk_error
			call puts

	cli
	hlt

times 510 - ($ - $$) db 0
dw 0xaa55
