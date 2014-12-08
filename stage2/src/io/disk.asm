%ifndef _IO_DISK_ASM
%define _IO_DISK_ASM

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

VARS
	;; Disk Address Packet structure.
	struct_dap:
		.length:    db 0x10 ; DAP length. 0x10 or 0x18, if the 64-bit flat destination is used.
		.reserved1: db 0
		.blocks:    db 0 ; Amount of blocks to read / write, max. 127.
		.reserved2: db 0

		.dest_offset:  dw 0 ; Destination address. Set to ffff:ffff if the 64-bit flat address is to be used.
		.dest_segment: dw 0

		.lba:       dq 0
		.dest_flat: dq 0 ; Optional.
ENDVARS

;; Reads one or more disk sectors.
;; (disk_id, lba_ptr, blocks, [dest_flat_ptr | 0, dest_segment, dest_offset])
FUNCTION disk_read_sectors, dx, si, di
	mov si, ARG(2) ; Pointer to LBA.
	mov di, struct_dap.lba
	times 2 movsd ; Copy it over.

	; TODO: Detect out-of-bounds LBA and panic.

	mov ax, ARG(3)
	mov [struct_dap.blocks], ax

	mov si, ARG(4) ; Pointer to linear destination address.
	or si, si
	jz .no_flat

	.has_flat:
		mov byte [struct_dap.length], 0x18
		mov ax, 0xffff
		mov [struct_dap.dest_segment], ax
		mov [struct_dap.dest_offset],  ax

		mov di, struct_dap.dest_flat
		times 2 movsd ; Copy it over.

		jmp .have_dest

	.no_flat: ; Flat address is zero, accept segment:offset arguments.
		mov byte [struct_dap.length], 0x10
		mov ax, ARG(5)
		mov [struct_dap.dest_segment], ax
		mov ax, ARG(6)
		mov [struct_dap.dest_offset],  ax

.have_dest:
	mov si, struct_dap
	mov dx, ARG(1) ; Disk number.
	mov ah, 0x42
	int 0x13
	jc .error

	RETURN 0, dx, si, di

	.error:
		RETURN ax, dx, si, di
END

;; Gets drive parameters.
;; (disk_id, buf_ptr, buf_len).
FUNCTION disk_get_params, dx, si
	VARS
		.s_param_error: db "error: Could not get drive parameters: AX=0x%x", CRLF, 0
	ENDVARS

	mov si, ARG(2)
	mov ax, ARG(3)
	mov [si], ax

	mov ah, 0x48
	mov dx, ARG(1)
	int 0x13
	jc .error

	RETURN 0, dx, si

	.error:
		INVOKE printf, .s_param_error, ax
		RETURN 1, dx, si
END

%endif ; _IO_DISK_ASM
