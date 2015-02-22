/**
 * \file
 * \brief     Hex dumper.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "dump.h"
#include "console.h"

void dump(void *_ptr, size_t length) {

	uint8_t *ptr = (uint8_t*)_ptr;

	for (size_t i=0; i<length; i+=16, ptr+=16) {
		printf("%08x  ", (uint32_t)ptr);

		uint32_t j;
		for (j=0; j<MIN(16, length-i); j++) {
			if (j == 8)
				putch(' ');
			printf("%02x ", ptr[j]);
		}

		if (j % 16) {
			for (uint32_t k=0; k<16-j; k++)
				puts("   ");
			if (j <= 8)
				putch(' ');
		}

		puts(" |");

		for (uint32_t k=0; k<j; k++) {
			char ch = ptr[k];
			putch(
				ch >= ' ' && ch <= '~'
				? ch
				: '.'
			);
		}
		puts("|\n");
	}
}
