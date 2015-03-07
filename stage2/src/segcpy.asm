;; \file
;; \brief     Copy memory across segments.
;; \author    Chris Smeele
;; \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
;; \license   MIT. See LICENSE for the full license text.

[bits 16]

global segcpy

SECTION .text

segcpy:
	push ebp
	mov ebp, esp

	cli
	push fs
	push gs
	push di
	push si

	; Load counter.
	mov ecx, [ebp + 16]
	or ecx, ecx
	jz .end

	; Load dest into FS:DI.
	mov eax, [ebp+8]
	shr eax, 16
	mov fs, eax
	mov edi, [ebp+8]
	and edi, 0xffff

	; Load source into GS:SI.
	mov eax, [ebp+12]
	shr eax, 16
	mov gs, eax
	mov esi, [ebp+12]
	and esi, 0xffff

.loop:
	mov al, [gs:si]
	mov [fs:di], al
	inc di
	inc si
	dec ecx
	jnz .loop

.end:
	pop si
	pop di
	pop gs
	pop fs
	sti

	mov esp, ebp
	pop ebp
	ret
