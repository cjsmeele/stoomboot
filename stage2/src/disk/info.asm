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
	.sector_count_u64:  resq 1
	.sector_count:      resd 1 ; 32-bit sector count. Caps at 0xffff.ffff.
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

;; Drive Parameter Table.
struc t_disk_dpt
	.size:              resw 1
	.flags:             resw 1
	.cylinder_count:    resd 1
	.head_count:        resd 1
	.sectors_per_track: resd 1
	.sector_count:      resq 1
	.bytes_per_sector:  resw 1
endstruc


absolute MEM_DISK_INFO
	struct_disks_info: resb MEM_MBR - struct_disks_info ; Reserve memory all the way up to our bootsector.
section .text

%define DISKINFO(s) struct_disks_info + t_disks_info. %+ s


;; Returns a pointer to the disk_info structure for the given disk number.
;; Note that disk_info structs of disks with lower disk numbers must already
;; be loaded for this to work.
;; (disk_number)
FUNCTION get_diskinfo_struct, cx, dx, di
	VARS
		.s_disk_out_of_range: db "error: Disk number %u is out of range", CRLF, 0
	ENDVARS

	mov ax, ARG(1)
	mov dl, [MEM_DISK_INFO + t_disks_info.disk_count]
	xor dh, dh
	cmp ax, dx
	jl .disk_number_ok

	INVOKE printf, .s_disk_out_of_range, ax
	INVOKE panic

.disk_number_ok:
	mov ax, t_disk_info_size
	lea di, [MEM_DISK_INFO + t_disks_info.disks]

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

;; Returns a pointer to the part_info structure for the given disk and partition number.
;; (disk_number, partition_number)
FUNCTION get_partinfo_struct, dx, di
	VARS
		.s_part_out_of_range: db "error: Partition number %u for disk %u is out of range", CRLF, 0
	ENDVARS

	mov ax, ARG(1)
	INVOKE get_diskinfo_struct, ax
	mov di, ax

	mov ax, ARG(2)
	mov dl, [di + t_disk_info.partition_count]
	xor dh, dh
	cmp ax, dx
	jl .part_number_ok

	mov dx, ARG(1)
	INVOKE printf, .s_part_out_of_range, ax, dx
	INVOKE panic

.part_number_ok:
	lea di, [di + t_disk_info.partitions]
	mov ax, t_part_info_size
	mul word ARG(2)
	add di, ax

	RETURN di, dx, di
END

;; Gets disk parameters and saves them in a disk_info structure.
;; (diskinfo_ptr)
FUNCTION disk_save_params, di
	VARS
		.s_not_enough:  db "warning: Not enough drive parameters returned by int13h for disk %xh", CRLF, 0
		.s_unsupported_sector_size: db "warning: Support for sector sizes other than 512 (%u) is not yet implemented", CRLF, 0
	ENDVARS

	mov di, ARG(1)

	mov ax, [di + t_disk_info.disk_id]
	INVOKE disk_get_params, ax, MEM_DISK_PARAM_BUFFER, MEM_DISK_PARAM_BUFFER_SIZE

	cmp word [MEM_DISK_PARAM_BUFFER], 24 ; int13h fills this with the returned buffer length.
	jl .not_enough_params
	; We're actually only interested in the amount of sectors and sector size.
	; DPTE and DPI look useful but it doesn't seem like we can rely
	; on their presence. We'll stick with int13h for accessing disks anyway.

	; We might add support for the DPTE and DPI structures later as an optional
	; feature, and pass that information on to the booted kernel to help identify
	; the boot drive.

	; Let's see how many drives actually have sector sizes other than 512 before
	; we start supporting it.
	mov ax, [MEM_DISK_PARAM_BUFFER + t_disk_dpt.bytes_per_sector]
	cmp ax, 512
	jne .unsupported_sector_size

	mov eax, [MEM_DISK_PARAM_BUFFER + t_disk_dpt.sector_count]
	mov [di + t_disk_info.sector_count], eax

	; Keep a separate 64-bit sector count in disk_info.
	; DOS MBR appears to be limited to 32-bit LBAs, so we currently cannot access
	; sectors after 2 TiB (with 512-byte sectors).
	; If we're ever going to support newer partitions tables like GPT, we might
	; use this 64-bit value.

	mov eax, [MEM_DISK_PARAM_BUFFER + t_disk_dpt.sector_count]
	mov [di + t_disk_info.sector_count_u64], eax

	mov eax, [MEM_DISK_PARAM_BUFFER + t_disk_dpt.sector_count + 4]
	mov [di + t_disk_info.sector_count_u64 + 4], eax

	or eax, eax
	jnz .over_2tib

	.below_2tib:
		RETURN 0, di

	.over_2tib:
		; Pretend that we have the maximum amount of sectors for code using 32-bit addresses.
		mov dword [di + t_disk_info.sector_count], 0xffffffff
		RETURN 0, di

	.not_enough_params:
		mov ax, [di + t_disk_info.disk_id]
		INVOKE printf, .s_not_enough, ax
		RETURN 1, di

	.unsupported_sector_size:
		INVOKE printf, .s_unsupported_sector_size, ax
		RETURN 1, di
END

;; Fill a disk_info structure by parsing a disk's partition table.
;; (disk_number)
FUNCTION disk_explore, cx, dx, si, di
	VARS
		.s_disk_explore: db "Exploring disk %xh (disk_info at 0x%x)", CRLF, 0
		.s_sector_count: db "- disk has 0x%x%x 512-byte sectors", CRLF, 0
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

	push ax
	INVOKE disk_save_params, di
	or ax, ax
	jnz .endpartloop ; Skip this drive if we can't get the sector count.
	pop ax

	INVOKE printf, .s_disk_explore, ax, di

	mov ax, [di + t_disk_info.sector_count]
	mov dx, [di + t_disk_info.sector_count + 2]
	INVOKE printf, .s_sector_count, dx, ax

	xor ax, ax
	mov al, [di + t_disk_info.disk_id]
	INVOKE disk_read_sectors, ax, .u64_lba, 1, 0, 0, MEM_DISK_IO_BUFFER
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
	mov [MEM_DISK_INFO + t_disks_info.disk_count], dl

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
