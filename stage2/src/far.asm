;; \file
;; \brief     Memory operations using 32-bit pointers.
;; \author    Chris Smeele
;; \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
;; \license   MIT. See LICENSE for the full license text.

[bits 16]

global farcpy
global farzero

SECTION .text

farcpy:
	push ebp
	mov ebp, esp

	push edi
	push esi

	; Load counter.
	mov ecx, [ebp + 16]
	or ecx, ecx
	jz .end

	; Load dest and source.
	mov edi, [ebp +  8]
	mov esi, [ebp + 12]

.loop:
	mov al, [ds:esi]
	mov [ds:edi], al
	inc edi
	inc esi
	dec ecx
	jnz .loop

.end:
	pop esi
	pop edi

	mov esp, ebp
	pop ebp
	ret

farzero:
	push ebp
	mov ebp, esp

	push edi

	mov ecx, [ebp + 12]
	or ecx, ecx
	jz .end

	mov edi, [ebp +  8]
	xor al, al

.loop:
	mov [ds:edi], al
	inc edi
	dec ecx
	jnz .loop

.end:
	pop edi

	mov esp, ebp
	pop ebp
	ret
