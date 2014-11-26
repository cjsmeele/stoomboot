%ifndef _DISK_INFO_ASM
%define _DISK_INFO_ASM

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

;; Disk information list header.
struc t_disks_info
	.disk_count: resb 1
	.disks:
endstruc

;; Disk information.
struc t_disk_info
	.disk_id:           resb 1 ; BIOS drive number (80h, 81h, ... for harddisks).
	.sector_size:       resw 1
	.sector_count:      resd 1
	.bus_type:          resb 4
	.interface:         resb 8
	.extended_part_lba: resd 1
	.partition_count:   resb 1
	.partitions:
endstruc

;; Partition information.
struc t_part_info
	.system_id:    resb 1
	.lba_start:    resq 1
	.sector_count: resq 1
	.label:        resb 17
	.active:       resb 1
endstruc

absolute MEM_DISK_IO_BUFFER + MEM_DISK_IO_BUFFER_SIZE
	struct_disks_info: resb MEM_MBR - struct_disks_info ; Reserve memory all the way up to our bootsector.
section .text

%define DISKINFO(s) struct_disks_info + t_disks_info. %+ s


;; Returns a pointer to the disk_info structure for the given disk number.
;; Note that disk_info structs of disks with lower disk numbers must already
;; be loaded for this to work.
;; (disk_number)
FUNCTION get_diskinfo_struct, cx, dx, di
	mov ax, t_disk_info_size
	lea di, [DISKINFO(disks)]

	; disk_info structures are sized according to the amount of partitions.
	; Loop through lower disk numbers to get the address of the requested
	; disk_info.
	xor cx, cx
.next_disk:
	cmp cx, ARG(1)
	jge .done

	xor ax, ax
	mov al, [di + t_disk_info.partition_count]

	mov dx, t_part_info_size
	mul dx

	add ax, t_disk_info_size
	add di, ax

	inc cx
	jmp .next_disk

.done:

	RETURN di, cx, dx, di
END

;; Returns a pointer to the part_info structure for the given disk number and partition.
;; (disk_number, partition_number)
FUNCTION get_partinfo_struct
	; TODO
	RETURN 0
END

;; Fill a disk_info structure by parsing a disk's partition table.
;; (disk_number)
FUNCTION disk_explore, cx, dx, si, di
	VARS
		.s_disk_explore: db "Exploring disk %xh (disk_info at 0x%x)", CRLF, 0
		.s_disk_mbr_read_error: db \
			"warning: A disk read error occurred while trying to read the MBR of disk %xh", CRLF, 0

		.u64_lba:        dq 0
		.u8_disk_number: db 0

		.s_disk_info:      db "- found %u partition(s):", CRLF, 0
		.s_partition_info: db "  - %x: active: %x, type: %x, start: 0x%x%x size: 0x%x%x", CRLF, 0
	ENDVARS

	mov ax, ARG(1)
	mov [.u8_disk_number], al

	; Load the current disk info structure address into DI.
	INVOKE get_diskinfo_struct, ax
	mov di, ax

	mov dword [di + t_disk_info.extended_part_lba], 0

	; Store the BIOS drive number.
	mov ax, ARG(1)
	or al, 0x80
	mov [di + t_disk_info.disk_id], al

	INVOKE printf, .s_disk_explore, ax, di

	; TODO:
	;       1. Get drive parameters
	;       2. Fill disk_info structure
	;   x   3. Read sector 0

	xor ax, ax
	mov al, [di + t_disk_info.disk_id]
	INVOKE disk_read_sectors, ax, .u64_lba, 1, 0, 0, buf_disk_io
	or ax, ax
	jz .read_done

	.disk_mbr_read_error:
		xor ax, ax
		mov al, [di + t_disk_info.disk_id]
		INVOKE printf, .s_disk_mbr_read_error, ax
		RETURN 1, cx, dx, si, di

.read_done:
	; Blindly assume that this is a DOS style MBR.
	INVOKE disk_dos_mbr_parse, di

	lea si, [di + t_disk_info.partitions]
	mov al, [di + t_disk_info.partition_count]
	xor ah, ah
	INVOKE printf, .s_disk_info, ax

	xor cx, cx
.partloop:
	cmp cl, [di + t_disk_info.partition_count]
	jge .endpartloop

	mov dx, [si + t_part_info.sector_count]
	push dx
	mov dx, [si + t_part_info.sector_count + 2]
	push dx
	mov dx, [si + t_part_info.lba_start]
	push dx
	mov dx, [si + t_part_info.lba_start + 2]
	push dx
	mov dl, [si + t_part_info.system_id]
	xor dh, dh
	mov al, [si + t_part_info.active]
	xor ah, ah

	INVOKE printf, .s_partition_info, cx, ax, dx
	add sp, 4*2
	add si, t_part_info_size

	inc cx
	jmp .partloop

.endpartloop:

	call putbr

	RETURN 0, cx, dx, si, di
END

;; Detects all fixed disks and fills disk info structures.
FUNCTION disk_detect_all, cx, dx
	VARS
		.s_disks_detected: db "Detected %u fixed disk(s).", CRLF, 0
	ENDVARS

	mov dl, [BDA(hd_count)]
	mov [DISKINFO(disk_count)], dl

	xor dh, dh
	INVOKE printf, .s_disks_detected, dx

	; Loop through the disks.
	xor cx, cx
.loop:
	cmp dx, cx
	jle .done

	INVOKE disk_explore, cx

	inc cx
	jmp .loop
.done:

	RETURN 0, cx, dx
END

%endif ; _DISK_INFO_ASM
