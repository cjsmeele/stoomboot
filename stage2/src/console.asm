%ifndef _CONSOLE_ASM
%define _CONSOLE_ASM

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

%endif ; _CONSOLE_ASM
