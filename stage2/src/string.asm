%ifndef _STRING_ASM
%define _STRING_ASM

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

;; Returns the length of a string.
;; (str)
FUNCTION strlen, cx, si

	mov si, ARG(1)
	xor cx, cx
.loop:
	inc cx
	lodsb
	or al, al
	jnz .loop
.end:
	dec cx

	RETURN cx, cx, si
END

;; Compares 2 strings, stops at the first NUL byte.
;; Returns non-zero if they differ, zero otherwise.
;; (str1, str2)
FUNCTION strcmp, si, di
	mov si, ARG(1)
	mov di, ARG(2)
	xor ax, ax

.loop:
	lodsb
	inc di
	cmp byte [di-1], 0
	jz .at_di_end
	sub al, [di-1]
	jz .loop
	jmp .end
.at_di_end:
	sub al, [di-1]
.end:
	RETURN ax, si, di
END

;; Compares 2 strings, stops at the first word boundary (\0, \r, \n, or ' ').
;; Returns non-zero if they differ, zero otherwise.
;; (str1, str2)
FUNCTION strcmp_word, cx, si, di
	mov si, ARG(1)
	mov di, ARG(2)
	xor cx, cx ; Whether one (1) or both (2) of the strings have encountered a word boundary.

.loop:
	lodsb
	call .check_boundary

	push ax
	mov al, [di]
	call .check_boundary
	pop ax

	inc di
	cmp al, [di-1]
	je .next

.differ:
	cmp cx, 2
	je .equal ; Both differing chars are word delimiters.
	RETURN 1, cx, si, di ; One (or both) of the differing chars are not word delimiters.

.next:
	jcxz .loop ; Not at a word boundary yet, continue.
.equal:
	RETURN 0, cx, si, di ; Both characters are word delimiters.

	;; Check whether AL contains a non-word character.
	.check_boundary:
		cmp al, 0
		je .yes
		cmp al, ' '
		je .yes
		cmp al, 0x0a
		je .yes
		cmp al, 0x0d
		je .yes
		ret
		.yes:
			inc cx
			ret
END

%endif ; _STRING_ASM
