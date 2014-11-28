%ifndef _MEM_ASM
%define _MEM_ASM

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

; Bootloader memory layout
; ========================
;
; start     | end       | description
; ----------+-----------+------------------------------
; 0000.0400 | 0000.0500 | Bios Data Area
; 0000.0580 | 0000.05ff | Drive parameters buffer
; 0000.0600 | 0000.15ff | Disk I/O buffer
; 0000.1600 | 0000.xxxx | Disk info structures
; 0000.7c00 | 0000.7dff | Boot disk MBR, stage1
; 0000.7e00 | 0000.xxxx | Stage2
; 0001.0000 | 0008.ffff | Boot info structure
; 0009.0000 | 0009.fbff | Stack
; 0010.0000 | xxxx.xxxx | Kernel

;; Location of the BIOS Data Area.
%define MEM_BDA 0x400
%define MEM_BDA_SIZE 0x101

;; Location of the first usable memory area.
;; Set to the end of the BDA + 127 bytes (at 0x580)
%define MEM_START (MEM_BDA + MEM_BDA_SIZE + 127)

;; Drive parameters buffer.
%define MEM_DISK_PARAM_BUFFER MEM_START
%define MEM_DISK_PARAM_BUFFER_SIZE 128

;; Disk I/O buffer.
%define MEM_DISK_IO_BUFFER (MEM_DISK_PARAM_BUFFER + MEM_DISK_PARAM_BUFFER_SIZE)
%define MEM_DISK_IO_BUFFER_SIZE (CONFIG_DISK_IO_BUFFER_SIZE * 0x200)

;; Disk info structures.
%define MEM_DISK_INFO (MEM_DISK_IO_BUFFER + MEM_DISK_IO_BUFFER_SIZE)
%define MEM_DISK_INFO_SIZE (MEM_MBR - MEM_DISK_INFO)

;; Location of the boot disk's master boot record.
%define MEM_MBR 0x7c00
%define MEM_MBR_SIZE 0x200

;; Origin of this program.
%define MEM_STAGE2 (MEM_MBR + MEM_MBR_SIZE)

%endif ; _MEM_ASM
