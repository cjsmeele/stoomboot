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

; Printing strings {{{

;; Prints its argument as a string.
FUNCTION puts, si

	mov si, ARG(1)

.loop:
	lodsb
	or al, al
	jz .done

	mov ah, 0x0e
	int 0x10
	jmp .loop

.done:
	RETURN_VOID si

;; Prints its argument and a newline.
FUNCTION putln

	mov ax, ARG(1)
	INVOKE puts, ax
	call   putbr

	RETURN_VOID

; }}}
; Printing hexadecimal numbers {{{

;; Print a byte in hexadecimal.
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

;; Print a 16-bit number in hexadecimal.
FUNCTION putword, dx

	mov dx, ARG(1)
	shr dx, 8
	INVOKE putbyte, dx

	mov dx, ARG(1)
	INVOKE putbyte, dx

	RETURN_VOID dx

; }}}
; Printing decimal numbers {{{

;; Print an unsigned 16-bit number.
FUNCTION putunum, bx, dx, di
	VARS
		.buffer: times 6 db 0
	ENDVARS

	mov ax, ARG(1)

.init: ; putnum, the signed variant, jumps in here.
	mov bx, 10
	mov di, .buffer
	add di, 5

.loop:
	; Fill the buffer from back to front, leaving the terminating NUL byte.
	mov dx, 0
	div bx ; ax = quotient, dx = remainder
	add dl, '0'
	dec di
	mov [di], dl
	or ax, ax
	jnz .loop

	INVOKE puts, di

	RETURN_VOID bx, dx, di

;; Print a signed 16-bit number.
FUNCTION putnum, bx, dx, di

	mov ax, ARG(1)
	or ax, ax
	jns .positive

	.negative:
		push ax
		mov ah, 0x0e
		mov al, '-'
		int 0x10
		pop ax
		dec ax
		not ax

	.positive:

	jmp func_putunum.init ; Continue in putunum.

	NORETURN

; }}}

;; A very basic printf implementation.
FUNCTION printf, bx, si, di

	mov bx, 0 ; state: 0 = default, 1 = got % char.

	mov di, bp
	add di, 6 ; Address of the second parameter.

	mov si, ARG(1)

.loop:
	lodsb
	or al, al
	jz .done

	or bx, bx
	jnz .parse_format

	cmp al, '%'
	jne .echo

	.format_next:
		inc bx ; Now awaiting format options.
		jmp .loop
	.echo:
		mov ah, 0x0e
		int 0x10
		jmp .loop
	.parse_format:
		dec bx

		cmp al, 'd'
		je .signed
		cmp al, 'u'
		je .unsigned
		cmp al, 'x'
		je .hex
		cmp al, 's'
		je .string
		jmp .echo

		.unsigned:
			mov ax, [ss:di] ; Load parameter value (16-bit unsigned number).
			add di, 2       ; Increment to next parameter address.
			INVOKE putunum, ax
			jmp .loop
		.signed:
			mov ax, [ss:di] ; Load parameter value (16-bit signed number).
			add di, 2
			INVOKE putnum, ax
			jmp .loop
		.hex:
			mov ax, [ss:di] ; Load parameter value (16-bit unsigned number).
			add di, 2
			INVOKE putword, ax
			jmp .loop
		.string:
			mov ax, [ss:di] ; Load parameter value (string address).
			add di, 2
			INVOKE puts, ax
			jmp .loop

	jmp .loop
.done:
	RETURN_VOID bx, si, di

;; Change the cursor shape.
FUNCTION set_cursor_shape, cx

	mov ah, 0x01
	mov ch, ARG(1)
	mov cl, ARG(2)
	int 0x10

	RETURN_VOID cx

%endif ; _IO_CONSOLE_ASM
