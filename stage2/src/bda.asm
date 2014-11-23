%ifndef _BDA_ASM
%define _BDA_ASM

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

%define BDA_START 0x0400

;; Bios Data Area structure.
;;
;; Sources consulted for field names:
;; - http://www.bioscentral.com/misc/bda.htm
;; - http://www.nondot.org/sabre/os/files/Booting/BIOS_SEG.txt
struc t_bda
	; 00h
	.com1_io resw 1
	.com2_io resw 1
	.com3_io resw 1
	.com4_io resw 1
	.lpt1_io resw 1
	.lpt2_io resw 1
	.lpt3_io resw 1
	.ebda_segment resw 1 ; May contain lpt4 I/O address on some systems.

	; 10h
	.installed_hardware resw 1
	.u8__1              resb 1
	.lower_mem_size     resw 1
	.u16__2             resw 1
	.kb_flags_1         resb 1
	.kb_flags_2         resb 1
	.u8__3              resb 1 ; Something keypad related?
	.kb_next            resw 1 ; Points to next character in keyboard buffer.
	.kb_last            resw 1 ; Points to last character in keyboard buffer.
	.kb_buffer          resb 32 ; Circular key buffer.

	; 3eh
	.fd_calibration_status resb 1
	.fd_motor_status       resb 1
	.fd_motor_timeout      resb 1
	.fd_status             resb 1
	.disk_status           resb 1
	.fd_status_1           resb 1
	.fd_status_2           resb 1
	.fd_cylinders          resb 1
	.fd_heads              resb 1
	.fd_sectors            resb 1
	.fd_bytes_written      resb 1

	; 49h
	.video_mode         resb 1
	.video_cols         resw 1
	.video_page_size    resw 1
	.video_page_offset  resw 1 ; Relative to VRAM base address.
	.video_page0_cursor resw 1
	.video_page1_cursor resw 1
	.video_page2_cursor resw 1
	.video_page3_cursor resw 1
	.video_page4_cursor resw 1
	.video_page5_cursor resw 1
	.video_page6_cursor resw 1
	.video_page7_cursor resw 1
	.video_cursor_shape resw 1
	.video_active_page  resb 1
	.video_io           resw 1
	.video_mode_reg     resb 1
	.video_palette_reg  resb 1

	; 67h
	.u16__4         resw 1
	.u16__5         resw 1
	.last_interrupt resb 1
	.timer_ticks    resd 1
	.timer_24h      resb 1
	.kb_break       resb 1
	.soft_reset     resw 1

	; 74h
	.hd_op_status resb 1
	.hd_count     resb 1
	.hd_control   resb 1
	.hd_io_offset resb 1
	.lpt1_timeout resb 1
	.lpt2_timeout resb 1
	.lpt3_timeout resb 1
	.vds_support  resb 1 ; May contain lpt4 timeout.
	.com1_timeout resb 1
	.com2_timeout resb 1
	.com3_timeout resb 1
	.com4_timeout resb 1

	; 80h
	.kb_buffer_start   resw 1
	.kb_buffer_end     resw 1
	.video_lastrow     resb 1 ; Amount of rows minus one.
	.video_char_height resw 1 ; In scan-lines.
	.video_options     resb 1
	.video_switches    resb 1
	.video_flags_1     resb 1
	.video_dcd_index   resb 1 ; Display Combination Code table?

	; 8bh
	.fd_config           resb 1
	.hd_status           resb 1
	.hd_error            resb 1
	.hd_done             resb 1
	.fd_info             resb 1
	.fd0_status          resb 1
	.fd1_status          resb 1
	.fd0_op_start_status resb 1
	.fd1_op_start_status resb 1
	.kb_flags_3          resb 1
	.kb_flags_4          resb 1

	; 98h
	.wait_complete_address resd 1
	.wait_duration         resd 1 ; In microseconds.
	.wait_active           resb 1
	.u8__6                 resb 1 ; Something to do with LAN?
	.video_spt_offset      resw 1
	.video_spt_segment     resw 1

	; And possibly more (uninteresting) fields up to ffh.

endstruc

%define BDA(s)  BDA_START + t_bda. %+ s

%endif ; _BDA_ASM
