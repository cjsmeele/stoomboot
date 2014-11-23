%ifndef _IO_DISK_ASM
%define _IO_DISK_ASM

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
	.disk_id:         resb 1 ; BIOS disk number (80h, 81h, ... for harddisks).
	.sector_size:     resw 1
	.sector_count:    resd 1
	.bus_type:        resb 4
	.interface:       resb 8
	.partition_count: resb 1
	.partitions:
endstruc

;; Partition information.
struc t_part_info
	; TODO
endstruc

absolute 0x500
	buf_disk_io:       resb 0x1000 ; 4K for large sectors.
	struct_disks_info: resb 0x66ff ; Reserve memory all the way up to 0x7c00, our bootsector.
section .text

%define DISKINFO(s) struct_disks_info + t_disks_info. %+ s


;; Disk Address Packet structure.
struct_dap:
	.length:    db 0x10 ; DAP length. 0x10 or 0x18, if the 64-bit flat destination is used.
	.reserved1: db 0
	.blocks:    db 0 ; Amount of blocks to read / write, max. 127.
	.reserved2: db 0

	.dest_offset:  dw 0 ; Destination address. Set to ffff:ffff if the 64-bit flat address is to be used.
	.dest_segment: dw 0

	.lba:       dq 0
	.dest_flat: dq 0 ; Optional.


;; Reads a disk sector. (UNTESTED)
;; (disk_i, lba_ptr, blocks, [dest_flat_ptr | 0, dest_segment, dest_offset])
FUNCTION disk_read_sector, dx, si, di
	mov si, ARG(2) ; Address where LBA is stored.
	mov di, struct_dap.lba
	times 4 movsw ; Copy it over.

	mov ax, ARG(3)
	mov [struct_dap.blocks], ax

	mov si, ARG(4) ; Address where a linear destination address is stored.
	or si, si
	jz .no_flat
	.has_flat:
		mov byte [struct_dap.length], 0x18
		mov ax, 0xffff
		mov [struct_dap.dest_segment], ax
		mov [struct_dap.dest_offset],  ax

		mov di, struct_dap.dest_flat
		times 4 movsw ; Copy it over.

		jmp .have_dest
	.no_flat: ; Flat address is zero, accept segment:offset arguments.
		mov byte [struct_dap.length], 0x10
		mov ax, ARG(5)
		mov [struct_dap.dest_segment], ax
		mov ax, ARG(6)
		mov [struct_dap.dest_offset],  ax

.have_dest:
	mov si, struct_dap
	mov dx, ARG(1) ; Disk number.
	mov ah, 0x42
	int 0x13
	jc .error
	RETURN 0, dx, si, di
	.error:
		RETURN 1, dx, si, di

;; Returns a pointer to the disk_info structure for the given disk number.
;; (disk_number)
FUNCTION get_diskinfo_struct, dx, di
	mov ax, t_disk_info_size
	mov dx, ARG(1)
	mul dx
	lea di, [DISKINFO(disks)]
	add di, ax

	RETURN di, dx, di

;; Returns a pointer to the part_info structure for the given disk number and partition.
;; (disk_number, partition_number)
FUNCTION get_partinfo_struct
	; TODO
	RETURN 0

;; Fill a disk_info structure by parsing a disk's partition table.
;; (disk_id)
FUNCTION disk_explore, dx, di
	VARS
		.s_disk_info: db "Exploring disk 0x%x.", CRLF, 0
		.s_disk_struct_addr: db "Disk structure at 0x%x.", CRLF, 0
		.u64_lba:     dq 0
		.u8_disk_id:  db 0 ; Disk number, starting from 0.
	ENDVARS

	mov ax, ARG(1)
	mov [.u8_disk_id], al
	or ax, 0x80
	INVOKE printf, .s_disk_info, ax

	; Load the current disk info structure address into DI.

	xor ax, ax
	mov al, [.u8_disk_id]
	INVOKE get_diskinfo_struct, ax
	mov di, ax

	INVOKE printf, .s_disk_struct_addr, di
	mov ax, ARG(1)
	or ax, 0x80
	mov [di + t_disk_info.disk_id], ax

	; TODO:
	;       1. Get drive parameters
	;       2. Fill disk_info structure
	;       3. Read sector 1
	;       4. Parse MBR
	;       5. Loop through primary partitions
	;       6. (optional) Detect extended partitions, loop through logical partitions
	;       7. Detect GPT and panic
	;       8. Fill part_info structures
	;       9. Find boot disk/partition (how? by UUID?)

	RETURN_VOID dx, di

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

%endif ; _IO_DISK_ASM
