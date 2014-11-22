%ifndef _IO_CONSOLE_ASM
%define _IO_CONSOLE_ASM

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

;; Print a newline.
putbr:
	mov ax, 0x0e0a
	int 0x10
	mov ax, 0x0e0d
	int 0x10
	ret

;; Prints its argument as a string.
FUNCTION puts, si, bx

	mov si, ARG(1)

.loop:
	lodsb
	or al, al
	jz .done

	mov ah, 0x0e
	mov bx, 0x0007
	int 0x10
	jmp .loop

.done:
	RETURN_VOID si, bx

;; Prints its argument and a newline.
FUNCTION putln

	mov ax, ARG(1)
	INVOKE puts, ax
	call   putbr

	RETURN_VOID

FUNCTION set_cursor_shape, cx

	mov ah, 0x01
	mov ch, ARG(1)
	mov cl, ARG(2)
	int 0x10

	RETURN_VOID cx


FUNCTION putbyte, dx

	mov dx, ARG(1)
	mov dh, 1 ; Higher nibble.
	call .putnibble
	mov dh, 0 ; Lower nibble.
	call .putnibble

	RETURN_VOID dx

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

FUNCTION putword, dx

	mov dx, ARG(1)
	shr dx, 8
	INVOKE putbyte, dx

	mov dx, ARG(1)
	INVOKE putbyte, dx

	RETURN_VOID dx

%endif ; _IO_CONSOLE_ASM
