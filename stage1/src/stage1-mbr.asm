; Copyright (c) 2014 Chris Smeele
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
; THE SOFTWARE.

[bits 16]
[org 0x7c00]

jmp 0x0000:start ; Far jump to start.

u8_boot_device:
	db 0

;; Disk address packet structure.
struct_dap:
	db 0x10 ; DAP length.
	db 0
	db 0 ; Blocks to read, to be filled in by the installer.
	db 0

	dw 0x7e00 ; Destination address.
	dw 0x0000 ; Segment.

	dq 0 ; The stage2 LBA, to be filled in by the installer.


s_loading:                  db "Loading... stage1 ", 0
s_err_no_int13h_extensions: db "Error: No int13h extensions present for LBA disk access. :(", 0
s_err_disk:                 db "Error: Could not read stage2 from boot disk 0x", 0
s_err_disk_2:               db ", AH=0x", 0

putbr:
	mov ax, 0x0e0a
	int 0x10
	mov ax, 0x0e0d
	int 0x10
	ret

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

;; Prints a byte in dl in hexadecimal.
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
	; Set up the stack.
	cli
	mov ax, 0x9000
	mov ss, ax
	mov sp, 0xfbff
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

		; Jump to stage2.
		jmp 0x0000:0x7e00

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
