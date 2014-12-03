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

%include "config.asm"
%include "mem.asm"

[org MEM_STAGE2]

db 0xfa, 0xf4     ; cli, hlt
db "STAGE2", 0, 0 ; magic!

jmp start

%include "common.asm"
%include "panic.asm"
%include "bda.asm"
%include "io/console.asm"
%include "disk/info.asm"
%include "io/disk.asm"
%include "disk/dos-mbr.asm"

start:
	INVOKE main, ax
	INVOKE halt

FUNCTION main
	VARS
		.s_loading: db "stage2", 0
		.s_foobar: db "foobar: %#lx %#hx", CRLF, 0
		.s_foo: db "foo: %#hx, %#lx, %lx, %#x, %s, %x.", CRLF, 0
		.s_bar: db "bar", 0
	ENDVARS

	INVOKE putln, .s_loading
	call putbr

	INVOKEW 1, printf, .s_foobar, dword 0x2468ace5, 0xea
	mov eax, 0xfeedbeef
	INVOKEW 2, printf, .s_foo, 0x00ea, dword 0x2468ace5, eax, 0xfedc, .s_bar, 0xaa55

	INVOKE disk_detect_all

	; The boot device number is stored in ARG(1)

	; TODO:
	;   x   1.  Scan partition tables
	;       2.  Find FAT32 boot partition
	;       3.  Parse FAT and find boot config file
	;       4.  Parse config
	;       5.  Show an interactive boot menu (with cmdline?)
	;       6.  Find a kernel image
	;       7.  Parse kernel ELF
	;       8.  Create a multiboot_info-like structure
	;       9.  Create a memory map
	;       10. Change video mode if required
	;       11. Switch to protected mode
	;       12. Jump to the kernel

	RETURN_VOID
END

FUNCTION halt
	VARS
		.s_halt: db CRLF, "stage2: the system has been halted.", 0
	ENDVARS

	INVOKE putln, .s_halt
hang:
	cli
	hlt
	NORETURN
END
