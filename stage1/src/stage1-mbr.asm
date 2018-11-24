;; \file
;; \brief     Bootloader stage1, MBR version.
;; \author    Chris Smeele
;; \copyright Copyright (c) 2014-2018 Chris Smeele. All rights reserved.
;; \license   MIT. See LICENSE for the full license text.

[bits 16]
[org 0x7c00]

jmp 0x0000:start ; Far jump to start.

;; Disk address packet structure.
struct_dap:
	db 0x10 ; DAP length.
	db 0
	db 0 ; Blocks to read, to be filled in by the installer (max 127).
	db 0

	dw 0x7e00 ; Destination offset.
	dw 0x0000 ; Destination segment.

	dq 0 ; The stage2 LBA, to be filled in by the installer.

u8_boot_device:
	db 0

u64_loader_fs_id:
	dq 0 ; Bootloader filesystem id, to be filled in by the installer.

s_loading:                  db "Loading... stage1 ", 0
s_err_no_int13h_extensions: db "Error: No int13h extensions present :(", 0
s_err_disk:                 db "Error: Could not read stage2 from boot disk 0x", 0
s_err_disk_2:               db ", AH=0x", 0
s_err_magic:                db "Error: No valid stage2 magic number found", 0
s_stage2_magic:             db 0xfa, 0xf4, "STAGE2", 2, 0
s_stage2_magic_end:

putbr:
	mov ax, 0x0e0a
	int 0x10
	mov ax, 0x0e0d
	int 0x10
	ret

;; Prints SI.
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

;; Prints a byte in DL in hexadecimal.
putbyte:
	mov dh, 1 ; Higher nibble.
	call .putnibble
	mov dh, 0 ; Lower nibble.
	call .putnibble
	ret

	.putnibble:
		mov ah, 0x0e
		mov al, dl

		or dh, dh
		jz .lower
		.higher:
			and al, 0xf0
			shr al, 4
			jmp .put
		.lower:
			and al, 0x0f
	.put:
		cmp al, 0x0a
		jb .decimal
		.hex:
			add al, 'a' - 10
			int 0x10
			ret
		.decimal:
			add al, '0'
			int 0x10
			ret

has_int13h_extensions:
	mov ah, 0x41
	mov bx, 0x55aa
	mov dl, [u8_boot_device]
	int 0x13
	jc .error
	mov ax, 1
	ret

	.error:
		xor ax, ax
		ret

;; Entrypoint.
start:
	cli
	; Set up the stack.
	mov ax, 0x9000
	mov ss, ax
	mov sp, 0xfbff
	; Reset segment registers.
	xor ax, ax
	; The code segment is already set to 0x0000 by the far jump.
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	sti

	mov [u8_boot_device], dl

	call putbr
	mov si, s_loading
	call puts

	call has_int13h_extensions
	or ax, ax
	jz .int13h_error

.load_stage2:
	mov si, struct_dap
	mov ah, 0x42
	mov dl, [u8_boot_device]
	int 0x13
	jc .error

.magic_check:
	mov si, s_stage2_magic
	mov di, 0x7e00
.magic_loop:
	lodsb
	mov bl, [di]
	inc di
	cmp al, bl
	jne .magic_error
	or al, al
	jnz .magic_loop

	; Jump to stage2. Pass the boot device number in AL and a pointer to the FS id in DX.
	mov al, [u8_boot_device]
	xor ah, ah
	lea dx, [u64_loader_fs_id]
	jmp 0x0000:(0x7e00 + s_stage2_magic_end - s_stage2_magic)

	.magic_error:
		call putbr
		mov si, s_err_magic
		call puts
		jmp halt

	.error:
		push ax
		call putbr
		mov si, s_err_disk
		call puts
		mov dl, [u8_boot_device]
		call putbyte

		mov si, s_err_disk_2
		call puts
		pop dx
		call putbyte

		jmp halt

	.int13h_error
		call putbr
		mov si, s_err_no_int13h_extensions
		call puts
		jmp halt

halt:
	cli
	hlt

times 510 - ($ - $$) db 0
dw 0xaa55
