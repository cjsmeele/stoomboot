%ifndef _COMMON_ASM
%define _COMMON_ASM

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

; Calling conventions {{{

%define ARG(i) [bp + (i+1)*2]

;; Enter a subroutine. Push register arguments on the stack.
%macro FUNCTION 0-*
	push bp
	mov bp, sp

	%rep %0
		push %1
		%rotate 1
	%endrep
%endmacro

;; Return from a subroutine. Pop register arguments from the stack.
%macro RETURN_VOID 0-*
	%rep %0
		pop %1
		%rotate 1
	%endrep

	mov sp, bp
	pop bp
	ret
%endmacro

;; Return from a subroutine with a value. Pop register arguments from the stack.
%macro RETURN 1-*
	mov ax, %1

	%rotate -1

	%rep %0-1
		%ifidni %1, ax
			%error Restoring ax before ret overwrites your return value
		%endif

		pop %1
		%rotate -1
	%endrep

	mov sp, bp
	pop bp

	ret
%endmacro

;; Invoke a subroutine with zero or more arguments.
%macro INVOKE 1-*
	%xdefine _SUB_NAME %1

	%rotate -1
	%rep %0-1
		push %1
		%rotate -1
	%endrep

	call _SUB_NAME
	add sp, (%0-1) * 2
%endmacro

; }}}

%endif ; _COMMON_ASM
