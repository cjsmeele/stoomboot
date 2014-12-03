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
END

;; Prints its argument and a newline.
FUNCTION putln

	mov ax, ARG(1)
	INVOKE puts, ax
	call   putbr

	RETURN_VOID
END

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
END

;; Print a 16-bit number in hexadecimal.
FUNCTION putword, dx

	mov dx, ARG(1)
	shr dx, 8
	INVOKE putbyte, dx

	mov dx, ARG(1)
	INVOKE putbyte, dx

	RETURN_VOID dx
END

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
	xor dx, dx
	div bx ; ax = quotient, dx = remainder.
	add dl, '0'
	dec di
	mov [di], dl
	or ax, ax
	jnz .loop

	INVOKE puts, di

	RETURN_VOID bx, dx, di
END

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
END

; }}}
; Printf {{{

; The printf flag bits are as follows:
;
; 0000.0000.0000.0001
;             || |||`Alternative format (i.e. '0x' prefix for hex)
;             || ||`Uppercase hexadecimal
;             || |`Left-adjusted (pad right side with blanks)
;             || `Pad with zeroes instead of blanks
;             |`Print a space before positive numbers and empty strings
;             `Group digits (e.g. 1'234'567 or 0x12ab.34cd)
;
; A '1' indicates that the feature has been implemented.

;; Print a number in hexadecimal.
;; Must be called in printf context.
printf_hex:
	push cx
	push dx

	test word [func_printf.u16_flags], 0x01
	jz .put
	.alt:
		mov ax, 0x0e00 | '0'
		int 0x10
		mov ax, 0x0e00 | 'x'
		int 0x10

.put:
	call func_printf.f_loadnum
	mov cx, [func_printf.u16_length]

	; Rotate the number to the right to move the highest byte to AL.
.rotloop:
	dec cx
	jz .endrotloop
	ror eax, 8
	jmp .rotloop
.endrotloop:

	mov cx, [func_printf.u16_length]
.byteloop:
	; putbyte overwrites AX.
	push ax
	INVOKE putbyte, ax
	pop ax
	rol eax, 8 ; Load the next byte in AL.
	loop .byteloop

	pop dx
	pop cx
	ret

printf_dec:
	; TODO
	ret


;; A very basic printf implementation.
;; Parameters are passed using the stack and are at least 16-bit in size.
FUNCTION printf, bx, si, di
	VARS
		.u16_flags:  dw 0
		.u16_length: dw 0
	ENDVARS

	xor bx, bx ; state: 0 = default, 1 = got % char.

	mov di, bp
	add di, 6 ; Address of the second parameter.

	mov si, ARG(1)

.loop:
	lodsb
	or al, al
	jz .done

	or bx, bx
	jnz .parse_format
	mov word [.u16_flags], 0
	mov word [.u16_length], 2

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
		; Check for flags.
		cmp al, '#'
		je .flag_alt

		; Check for length modifiers.
		cmp al, 'h'
		je .length_8
		cmp al, 'l'
		je .length_32

		; Check for conversion specifiers.
		cmp al, 'd'
		je .signed
		cmp al, 'u'
		je .unsigned
		cmp al, 'x'
		je .hex
		cmp al, 's'
		je .string
		cmp al, 'c'
		je .char
		jmp .format_done ; Invalid format string, back to echo-mode.

		;; Loads the next parameter in eax, uses length variable.
		.f_loadnum:
			cmp word [.u16_length], 1
			je .num_8
			cmp word [.u16_length], 2
			je .num_16
			.num_32
				mov eax, [ss:di] ; Load parameter value.
				add di, 4 ; Increment to next parameter address.
				ret
			.num_16
				mov ax, [ss:di] ; Load parameter value.
				add di, 2
				ret
			.num_8
				mov ax, [ss:di]
				add di, 2 ; Parameters are at least 16 bits wide.
				xor ah, ah
				ret

		.flag_alt:
			or word [.u16_flags], 0x01
			jmp .loop

		; TODO: Use length variable when printing.
		.length_8:
			mov word [.u16_length], 1
			jmp .loop
		.length_32:
			mov word [.u16_length], 4
			jmp .loop

		.unsigned:
			call .f_loadnum
			INVOKE putunum, ax
			jmp .format_done
		.signed:
			mov ax, [ss:di] ; Load parameter value (16-bit signed number).
			add di, 2
			INVOKE putnum, ax
			jmp .format_done
		.hex:
			call printf_hex
			jmp .format_done
		.string:
			mov ax, [ss:di] ; Load parameter value (string address).
			add di, 2
			INVOKE puts, ax
			jmp .format_done
		.char:
			mov ax, [ss:di] ; Load parameter value (string address).
			add di, 2
			mov ah, 0x0e
			int 0x10
			;jmp .format_done ; Fall through.
		.format_done:
			dec bx
			jmp .loop
.done:
	RETURN_VOID bx, si, di
END

; }}}

;; Change the cursor shape.
FUNCTION set_cursor_shape, cx

	mov ah, 0x01
	mov ch, ARG(1)
	mov cl, ARG(2)
	int 0x10

	RETURN_VOID cx
END

%endif ; _IO_CONSOLE_ASM
