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

struc t_dos_ebr
	.bootcode: resb 446
	.pte:      resb 16
	.next:     resb 16
	.unused:   resb 32
	.magic:    resb 2
endstruc

; EBR PTEs and MBR PTEs are essentially the same.
%define t_dos_ebr_part t_dos_mbr_part

;; Parse an EBR chain. See dos_mbr_parse_part_entry.
;; (disk_info_ptr, lba_ptr, sector_count_ptr) DI=part_info_ptr
FUNCTION dos_mbr_parse_ebr_chain, bx, dx, si
	VARS
		.s_read_error: db "warning: Could not read EBR sector, AX=%xh", CRLF, 0
		.s_ebr_explore: db "Parsing EBR", CRLF, 0
		.u32_lba: dd 0
		.u64_lba: dq 0
		.u64_sector_count: dq 0 ; Currently unused.
		;.s_next_ebr: db "Loading next EBR, LBA @ 0x%x", CRLF, 0
	ENDVARS

	; The MBR only supports 32-bit LBAs and sector counts,
	; copy them over to 64-bit pointers.
	mov bx, ARG(2)
	mov eax, [bx]
	mov [.u64_lba], eax
	mov bx, ARG(3)
	mov eax, [bx]
	mov [.u64_sector_count + 4], eax

	push di
	mov di, ARG(1)
	xor ax, ax
	mov al, [di + t_disk_info.disk_id]

	INVOKE disk_read_sectors, ax, .u64_lba, 1, 0, 0, MEM_DISK_IO_BUFFER + 512
	pop di
	or ax, ax
	jz .got_ebr

	.read_error:
		INVOKE printf, .s_read_error, ax
		RETURN 0, bx, dx, si

.got_ebr:
	mov si, MEM_DISK_IO_BUFFER + 512
	push si
	lea si, [si + t_dos_ebr.pte]
	mov eax, [si + t_dos_mbr_part.sector_count]
	mov ebx, [si + t_dos_mbr_part.lba_start]
	mov dx, 0 ; Whether we got a partition.
	or eax, eax
	jz .next_ebr

.add_partition:
	inc dx
	mov al, [si + t_dos_mbr_part.system_id]
	mov [di + t_part_info.system_id], al

	mov al, [si + t_dos_mbr_part.active]
	mov [di + t_part_info.active], al

	mov eax, [si + t_dos_mbr_part.sector_count]
	mov [di + t_part_info.sector_count], eax
	push di
	mov di, ARG(2)
	mov eax, [di]
	add eax, [si + t_dos_mbr_part.lba_start]
	pop di
	mov [di + t_part_info.lba_start], eax

	add di, t_part_info_size

.next_ebr:
	pop si
	lea si, [si + t_dos_ebr.next]
	mov eax, [si + t_dos_mbr_part.sector_count]
	or eax, eax
	jz .was_last_ebr

.got_next_ebr:
	push di
	mov di, ARG(1)
	mov eax, [di + t_disk_info.extended_part_lba]
	add eax, [si + t_dos_mbr_part.lba_start]
	mov [.u32_lba], eax
	pop di

	; TODO: Check for overflows etc? (Should not happen with non-corrupted partition tables).

	;push ax
	;INVOKE printf, .s_next_ebr, .u32_lba
	;pop ax

	push dx
	mov dx, ARG(1)
	lea si, [si + t_dos_mbr_part.sector_count]
	lea ax, [si + t_dos_mbr_part.sector_count]
	INVOKE dos_mbr_parse_ebr_chain, dx, .u32_lba, ax
	pop dx
	add dx, ax

.was_last_ebr:

	RETURN dx, bx, dx, si
END

;; Parse a MBR partition table entry.
;; Creates one or more part_info structures at DI and adds their size to DI.
;; Returns the amount of partitions added (may be >1 in case of an extended partition).
;; (disk_info_ptr, dos_mbr_part_ptr) DI=part_info_ptr
FUNCTION dos_mbr_parse_part_entry, bx, si
	VARS
		.s_gpt_warning: db "warning: Protective MBR detected. GPT partitions will not be parsed.", CRLF, 0
	ENDVARS

	mov si, ARG(2)

	; Detect PTE validity by checking the sector count.
	; This is probably not according to spec: Valid CHS values may exist, I'm
	; just not going to bother parsing them.
	mov eax, [si + t_dos_mbr_part.sector_count]
	or eax, eax
	jnz .has_sector_count

	.no_sector_count:
		RETURN 0, bx, si

.has_sector_count:
	xor ax, ax
	mov al, [si + t_dos_mbr_part.system_id]

	cmp al, 0x05
	je .has_ebr
	cmp al, 0xee
	je .has_gpt

	.is_normal_primary_partition:
		; Copy partition info to part_info structs.
		mov [di + t_part_info.system_id], al

		mov al, [si + t_dos_mbr_part.active]
		mov [di + t_part_info.active], al

		mov eax, [si + t_dos_mbr_part.sector_count]
		mov [di + t_part_info.sector_count], eax
		mov eax, [si + t_dos_mbr_part.lba_start]
		mov [di + t_part_info.lba_start], eax

		add di, t_part_info_size

		RETURN 1, bx, si

	.has_gpt:
		INVOKE puts, .s_gpt_warning
		RETURN 0, bx, si

	.has_ebr:
		lea bx, [si + t_dos_mbr_part.lba_start]
		lea ax, [si + t_dos_mbr_part.sector_count]

		mov si, ARG(1)
		push bx
		mov ebx, [bx]
		mov [si + t_disk_info.extended_part_lba], ebx
		pop bx

		INVOKE dos_mbr_parse_ebr_chain, si, bx, ax

		RETURN ax, bx, si
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
	mov ax, ARG(1)
	INVOKE dos_mbr_parse_part_entry, ax, si, 0
	push di
	mov di, [.u16_disk_info_ptr]
	add [di + t_disk_info.partition_count], al
	pop di

	add si, t_dos_mbr_part_size

	inc cx
	cmp cx, 4
	jl .partloop

	RETURN 0, cx, dx, si, di
END

%endif ; _DISK_DOS_MBR_ASM
