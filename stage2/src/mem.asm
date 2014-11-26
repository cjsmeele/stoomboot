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
; 0000.0400 | 0000.04ff | Bios Data Area
; 0000.0500 | 0000.44ff | Disk I/O buffer
; 0000.4500 | 0000.xxxx | Disk info structures
; 0000.7c00 | 0000.7dff | Boot disk MBR, stage1
; 0000.7e00 | 0000.xxxx | Stage2
; 0001.0000 | 0008.ffff | Boot info structure
; 0009.0000 | 0009.fbff | Stack
; 0010.0000 | xxxx.xxxx | Kernel

;; Location of the BIOS Data Area.
%define MEM_BDA 0x400

;; Location of the first usable memory area.
%define MEM_START 0x500

;; Location of the boot disk's master boot record.
%define MEM_MBR 0x7c00

;; Origin of this program.
%define MEM_STAGE2 (MEM_MBR + 0x200)

;; Disk I/O buffer.
%define MEM_DISK_IO_BUFFER MEM_START

;; Disk I/O buffer size in bytes.
%define MEM_DISK_IO_BUFFER_SIZE (CONFIG_DISK_IO_BUFFER_SIZE * 4096)

%endif ; _MEM_ASM
