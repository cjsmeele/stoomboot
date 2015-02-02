/**
 * @file
 * @brief
 * @author    Chris Smeele
 * @copyright Copyright (c) 2015, Chris Smeele
 */
#include "common.h"

void putch(char ch) {
	asm(
		"int $10h"
		:
		: "a" (0x0e << 8 | ch)
		: "memory", "cc"
	);
}

void puts(const char *str) {
	while(*str++)
		putch(str[-1]);
}

void putunum(uint32_t num) {
	static char buffer[10] = { };
	int i = 9;
	do {
		buffer[--i] = (num % 10) + '0';
		num /= 10;
	} while (num);

	puts(&buffer[i]);
}

void putnum(int32_t num) {
	if (num < 0) {
		putch('-');
		putunum(-num);
	} else {
		putunum(num);
	}
}

void hang() {
	for (;;)
		asm volatile(
			"cli\n"
			"hlt\n"
		);
}
