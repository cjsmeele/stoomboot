%ifndef _DISK_DOS_MBR_ASM
%define _DISK_DOS_MBR_ASM

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

;; Parse a Master Boot Record's partition table stored in the disk io buffer (0x500)
;; and fill the disk's partition info list.
;; (disk_number)
FUNCTION disk_dos_mbr_parse, di
	VARS
		.u8_disk_number: db 0 ; 00h..*
		.u8_disk_id:     db 0 ; 80h..*
	ENDVARS

	mov ax, ARG(1)
	mov [.u8_disk_number], al

	xor ax, ax
	mov al, [.u8_disk_id]
	INVOKE get_diskinfo_struct, ax
	mov di, ax

	mov ax, [di + t_disk_info.disk_id]
	mov [.u8_disk_id], al

	; TODO:
	;       1. Parse MBR
	;       2. Loop through primary partitions
	;       3. (optional) Detect extended partitions, loop through logical partitions
	;       4. Detect GPT and panic
	;       5. Fill part_info structures

	RETURN 0, di
END

%endif ; _DISK_DOS_MBR_ASM
