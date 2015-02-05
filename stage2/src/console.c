/**
 * @file
 * @brief
 * @author    Chris Smeele
 * @copyright Copyright (c) 2015, Chris Smeele
 */
#include "console.h"

void putch(char ch) {
	asm volatile (
		"int $0x10"
		:
		: "a" (0x0e << 8 | ch),
		  "b" (0x07)
		: "memory", "cc"
	);
}

void puts(const char *str) {
	while (*str)
		putch(*str++);
}

void putunum(uint32_t num) {
	static char buffer[11] = { };
	int i = 10;
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

bool getKey(Key *key, bool wait) {
	if (!wait) {
		uint16_t ready = 0;
		asm volatile (
			"int $0x16\n"
			"jz _not_ready\n"
			"_ready:\n"
			"  inc %0\n"
			"_not_ready:"
			: "+r" (ready)
			: "a" (1 << 8)
			: "memory", "cc"
		);

		if (!ready)
			return false;
	}

	uint16_t keyInfo = 0;
	asm volatile (
		"int $0x16"
		: "+a" (keyInfo)
		:
		: "memory", "cc"
	);

	key->chr      = keyInfo & 0xff;
	key->scanCode = keyInfo >> 8;

	return true;
}

uint16_t getLine(char *line, uint16_t size) {
	int16_t i = 0;
	Key key;
	while (i < size - 1) {
		if (getKey(&key, true)) {
			if (key.chr >= ' ' && key.chr <= '~') {
				putch(key.chr);
				line[i++] = key.chr;
			} else if (key.chr == '\r') {
				puts("\r\n");
				break;
			} else if (key.chr == '\e') {
				line[0] = 0;
				puts("\r\n");
				break;
			} else if (key.chr == '\b') {
				if (i > 0) {
					putch('\b');
					putch(' ');
					putch('\b');
					i--;
					continue;
				}
			} else {
				// Ignore other non-printing characters.
			}
		}
	}

	line[i] = 0;

	return i;
}
