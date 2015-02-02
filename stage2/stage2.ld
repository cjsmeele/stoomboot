OUTPUT_FORMAT(binary)

SECTIONS {
	. = 0x00007e00;
	_STAGE2_START = .;

	.text ALIGN (0x10) : {
		_TEXT_START = .;
		*(.text*)
		*(.gnu.linkonce.t*)
		_TEXT_END = .;
	}

	.rodata ALIGN (0x10) : {
		_RODATA_START = .;
		*(.rodata*)
		*(.gnu.linkonce.r*)
		_RODATA_END = .;
	}

	.data ALIGN (0x10) : {
		_DATA_START = .;
		*(.data)
		*(.gnu.linkonce.d*)
		_DATA_END = .;
	}

	.bss ALIGN (0x10) : {
		_BSS_START = .;
		*(COMMON)
		*(.bss)
		*(.gnu.linkonce.b*)
		_BSS_END = .;
	}

	/DISCARD/ : {
		*(.comment)
	}

	_STAGE2_END = ALIGN (0x1000);
}
