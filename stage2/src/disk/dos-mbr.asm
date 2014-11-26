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

struc t_dos_mbr
	.bootcode:   resb 436
	.disk_uuid:  resb 10
	.part_table: resb 64
	.magic:      resb 2
endstruc

struc t_dos_mbr_part
	.active:       resb 1
	.chs_start:    resb 3
	.system_id:    resb 1
	.chs_end:      resb 3
	.lba_start:    resd 1
	.sector_count: resd 1
endstruc

;; Parse a MBR partition table entry.
;; Creates one or more part_info structures at DI and adds their size to DI.
;; Returns the amount of partitions added (may be >1 in case of an extended partition (TODO)).
;; (dos_mbr_part_ptr) DI=part_info_ptr
FUNCTION dos_mbr_parse_part_entry, si
	VARS
		.s_gpt_warning: db "warning: Protective MBR detected. GPT partitions will not be parsed.", CRLF, 0
		.s_ebr_warning: db "warning: Extended partition detected. Logical partitions will not be parsed.", CRLF, 0
	ENDVARS

	mov si, ARG(1)

	; Detect PTE validity by checking the sector count.
	; This is probably not according to spec: Valid CHS values may exist, I'm
	; just not going to bother parsing them.
	mov eax, [si + t_dos_mbr_part.sector_count]
	or eax, eax
	jnz .has_sector_count

	.no_sector_count:
		RETURN 0, si

.has_sector_count:
	xor ax, ax
	mov al, [si + t_dos_mbr_part.system_id]

	cmp al, 0x05
	je .has_ebr
	cmp al, 0xee
	je .has_gpt

	.is_normal_primary_partition:
		mov [di + t_part_info.system_id], al

		mov al, [si + t_dos_mbr_part.active]
		mov [di + t_part_info.active], al

		mov eax, [si + t_dos_mbr_part.sector_count]
		mov [di + t_part_info.sector_count], eax
		mov eax, [si + t_dos_mbr_part.lba_start]
		mov [di + t_part_info.lba_start], eax

		add di, t_part_info_size

		RETURN 1, si

	.has_gpt:
		INVOKE puts, .s_gpt_warning
		RETURN 0, si
	.has_ebr:
		INVOKE puts, .s_ebr_warning
		RETURN 0, si

END

;; Parse a Master Boot Record's partition table stored in the disk io buffer (0x500)
;; and fill the disk's partition info list.
;; (disk_info_ptr)
FUNCTION disk_dos_mbr_parse, cx, dx, si, di
	VARS
		.u16_disk_info_ptr: dw 0
		.s_no_mbr: db "warning: Disk %xh does not have a valid MBR", CRLF, 0
	ENDVARS

	mov ax, ARG(1)
	mov [.u16_disk_info_ptr], ax
	mov di, ax

	mov ax, [MEM_DISK_IO_BUFFER + 510]
	cmp ax, 0xaa55
	je .has_valid_mbr

	.no_mbr:
		mov al, [di + t_disk_info.disk_id]
		xor ah, ah
		INVOKE printf, .s_no_mbr, ax
		RETURN 1, cx, dx, si, di

.has_valid_mbr
	lea di, [di + t_disk_info.partitions]
	lea si, [MEM_DISK_IO_BUFFER + t_dos_mbr.part_table]

	; Loop through primary partitions.
	xor cx, cx
.partloop:
	INVOKE dos_mbr_parse_part_entry, si
	push di
	mov di, [.u16_disk_info_ptr]
	add [di + t_disk_info.partition_count], al
	pop di

	add si, t_dos_mbr_part_size

	inc cx
	cmp cx, 4
	jl .partloop

	; TODO:
	;   x   1. Parse MBR
	;   x   2. Loop through primary partitions
	;       3. (optional) Detect extended partitions, loop through logical partitions
	;   x   4. (optional) Detect protective MBR / GPT.
	;   x   5. Fill part_info structures

	RETURN 0, cx, dx, si, di
END

%endif ; _DISK_DOS_MBR_ASM
