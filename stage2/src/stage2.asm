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

[bits 16]
[org 0x7e00]

jmp start

%include "common.asm"
%include "bda.asm"
%include "io/console.asm"


start:
	INVOKE main
	INVOKE halt
	NORETURN

FUNCTION main
	VARS
		.s_loading: db "stage2", 0
		.s_kb_status: db 0x0d, "Keyboard flags: 0x%x (try pressing modifier keys)", 0
	ENDVARS

	INVOKE putln, .s_loading
	call putbr

.loop:
	mov dh, [BDA(kb_flags_1)]
	mov dl, [BDA(kb_flags_2)]
	INVOKE printf, .s_kb_status, dx
	hlt
	jmp .loop

	; TODO:
	;       1.  Scan partition table
	;       2.  Find FAT32 boot partition
	;       3.  Parse FAT and find boot config file
	;       4.  Parse config
	;       5.  Show an interactive boot menu (with cmdline?)
	;       6.  Find a kernel image
	;       7.  Parse kernel ELF
	;       8.  Parse multiboot header
	;       9.  Create a multiboot info struct
	;       10. Create a memory map
	;       11. Change video mode if requested
	;       12. Switch to protected mode
	;       13. Jump to the kernel

	RETURN_VOID

FUNCTION halt
	VARS
		.s_halt: db CRLF, "stage2: The system has been halted.", 0
	ENDVARS

	INVOKE putln, .s_halt
hang:
	cli
	hlt
	NORETURN
