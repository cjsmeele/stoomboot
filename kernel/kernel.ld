OUTPUT_FORMAT(elf32-i386)

SECTIONS {
	. = 0x00100000;
	_KERNEL_START = .;

	._multiboot_header : {
		*(._multiboot_header)
	}

	.text ALIGN (0x10) : {
		_TEXT_START = .;
		*(.text*)
		*(.gnu.linkonce.t*)
		_TEXT_END = .;
	}

	.rodata ALIGN (0x1000) : {
		_RODATA_START = .;
		*(.rodata*)
		*(.gnu.linkonce.r*)
		_RODATA_END = .;
	}

	.data ALIGN (0x1000) : {
		_DATA_START = .;
		*(.data)
		*(.gnu.linkonce.d*)
		_DATA_END = .;
	}

	.bss ALIGN (0x1000) : {
		_BSS_START = .;
		*(COMMON)
		*(.bss)
		*(.gnu.linkonce.b*)
		_BSS_END = .;
	}

	/DISCARD/ : {
		*(.comment)
		*(.eh_frame)
	}

	_KERNEL_END = ALIGN (0x1000);
}
