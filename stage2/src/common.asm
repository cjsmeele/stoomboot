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

;; Access 16-bit function arguments on the stack (ARG(1) is the first argument).
;; (arg_number)
%define ARG(i) [bp + (i+1)*2]

;; Enter a subroutine. Push register arguments on the stack.
;; (func_name, clobbered_reg...)
%macro FUNCTION 1-*
	%push   function
	%define %$function_name %1

	func_%1:
		push bp
		mov bp, sp

	%rotate 1
	%rep %0-1
		push %1
		%rotate 1
	%endrep
%endmacro

;; Declare function-local variables.
%macro VARS 0
	[section .data]
%endmacro

%macro ENDVARS 0
	__SECT__
%endmacro

;; Do NOT return from a subroutine. Jumps to hang.
%macro NORETURN 0
	jmp hang
%endmacro

;; Return from a subroutine. Pop register arguments from the stack.
;; (clobbered_reg...)
%macro RETURN_VOID 0-*
	%if %0 > 0
		%rotate -1
	%endif

	%rep %0
		pop %1
		%rotate -1
	%endrep

	mov sp, bp
	pop bp
	ret
%endmacro

;; Return from a subroutine with a value. Pop register arguments from the stack.
;; (return_value, clobbered_reg...)
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

;; Pop something from the context stack. Used in function context.
%macro END 0
	%pop
	INVOKE panic ; Catch missing RETURNs.
%endmacro

;; Invoke a subroutine with zero or more 16-bit arguments.
;; (func_name, ...)
%macro INVOKE 1-*
	%push invoke

	%define %$to_invoke %1

	; Push arguments from last to first.
	%rotate -1
	%rep %0-1
		push %1
		%rotate -1
	%endrep

	call func_%$to_invoke
	add sp, (%0-1) * 2

	%undef %$to_invoke

	%pop
%endmacro

;; Invoke a subroutine with zero or more arguments,
;; with the first parameter specifying how many of them are dwords.
;; (dword_count, func_name, ...)
%macro INVOKEW 2-*
	%push invoke

	%define %$wide_args %1
	%define %$to_invoke %2

	%rotate -1
	%rep %0-2
		push %1
		%rotate -1
	%endrep

	call func_%$to_invoke
	add sp, (%0-2) * 2 + 2*%$wide_args

	%undef %$wide_args
	%undef %$to_invoke

	%pop
%endmacro

; }}}
; Constant values {{{

%define CRLF 0x0d, 0x0a

; }}}

%endif ; _COMMON_ASM
