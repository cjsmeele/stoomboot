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
	.disk_id:         resb 1 ; BIOS drive number (80h, 81h, ... for harddisks).
	.sector_size:     resw 1
	.sector_count:    resd 1
	.bus_type:        resb 4
	.interface:       resb 8
	.partition_count: resb 1
	.partitions:
endstruc

;; Partition information.
struc t_part_info
	.disk_info_ptr: resw 1
	.part_number:   resb 1
	.system_id:     resb 1
	.sector_start:  resq 1
	.sector_count:  resq 1
	.label:         resb 17
	.bootable:      resb 1
endstruc

absolute MEM_START + MEM_DISK_IO_BUFFER_SIZE
	struct_disks_info: resb MEM_MBR - struct_disks_info ; Reserve memory all the way up to our bootsector.
section .text

%define DISKINFO(s) struct_disks_info + t_disks_info. %+ s


;; Returns a pointer to the disk_info structure for the given disk number.
;; (disk_number)
FUNCTION get_diskinfo_struct, dx, di
	mov ax, t_disk_info_size
	mov dx, ARG(1)
	mul dx
	lea di, [DISKINFO(disks)]
	add di, ax

	; TODO: Skip the dynamically sized partitions property of disk info structs.

	RETURN di, dx, di
END

;; Returns a pointer to the part_info structure for the given disk number and partition.
;; (disk_number, partition_number)
FUNCTION get_partinfo_struct
	; TODO
	RETURN 0
END

;; Fill a disk_info structure by parsing a disk's partition table.
;; (disk_number)
FUNCTION disk_explore, si, di
	VARS
		.s_disk_explore: db "Exploring disk %xh (disk_info at 0x%x)", CRLF, 0
		.s_disk_mbr_read_error: db \
			"warning: A disk read error occurred while trying to read the MBR of disk %xh", 0

		.u64_lba:        dq 0
		.u8_disk_number: db 0
	ENDVARS

	mov ax, ARG(1)
	mov [.u8_disk_number], al

	; Load the current disk info structure address into DI.
	INVOKE get_diskinfo_struct, ax
	mov di, ax

	; Store the BIOS drive number.
	mov ax, ARG(1)
	or al, 0x80
	mov [di + t_disk_info.disk_id], al

	INVOKE printf, .s_disk_explore, ax, di

	; TODO:
	;       1. Get drive parameters
	;       2. Fill disk_info structure
	;   x   3. Read sector 0

	; Read sector 0 (.u64_lba should already be 0, but zero it anyway).
	push di
	mov di, .u64_lba
	mov ax, 0
	times 4 stosw
	pop di

	xor ax, ax
	mov al, [di + t_disk_info.disk_id]
	INVOKE disk_read_sectors, ax, .u64_lba, 1, 0, 0, buf_disk_io
	or ax, ax
	jz .read_done

	.disk_mbr_read_error:
		xor ax, ax
		mov al, [di + t_disk_info.disk_id]
		INVOKE printf, .s_disk_mbr_read_error, ax
		RETURN 1, si, di

.read_done:
	; Blindly assume that this is a DOS style MBR.
	mov ax, ARG(1)
	INVOKE disk_dos_mbr_parse, ax

	RETURN 0, si, di
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
