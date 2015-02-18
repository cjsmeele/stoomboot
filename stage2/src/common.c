/**
 * \file
 * \brief     Commonly used system, memory, string and math functions.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "common.h"

void halt() {
	asm volatile (
		"hlt"
	);
}

void hang() {
	for (;;)
		asm volatile (
			"cli\n"
			"hlt\n"
		);
}

void *memset(void *mem, uint8_t c, size_t length) {
	if (length)
		asm volatile (
			".mloop1: stosb\n"
			"loop .mloop1\n"
			:
			: "a" (c),
			  "c" (length),
			  "D" (mem)
			: "memory"
		);

	return mem;
}

void *memcpy(void *dest, const void *source, size_t length) {
	if (length)
		asm volatile (
			".mloop2: movsb\n"
			"loop .mloop2\n"
			:
			: "c" (length),
			  "D" (dest),
			  "S" (source)
			: "memory"
		);

	return dest;
}

bool memeq(const void *source1, const void *source2, size_t length) {
	uint8_t equal = 1;

	if (length) {
		asm volatile (
			".mloop3: cmpsb\n"
			"jne .unequal\n"
			"loop .mloop3\n"
			"jmp .equal\n"
			".unequal:\n"
			"xor %0, %0\n"
			".equal:\n"
			: "=r" (equal)
			: "c" (length),
			  "D" (source1),
			  "S" (source2)
			: "memory", "cc"
		);
	}

	return equal;
}

size_t strlen(const char *str) {
	uint32_t i = 0;
	while (*str++)
		i++;
	return i;
}

char *strchr(const char *str, char ch) {
	size_t slen = strlen(str);
	for (size_t i=0; i<=slen; i++, str++) {
		if (*str == ch)
			break;
	}

	return (char*)str;
}

char *strncpy(char *dest, const char *src, size_t length) {
	for (size_t i=0; i<length; i++) {
		dest[i] = src[i];
		if (!src[i])
			break;
	}
	return dest;
}

static uint32_t powU(uint32_t x, uint32_t y){
	if(!y)
		return 1;
	uint32_t z = x;
	for(uint32_t i=1; i<y; i++)
		z *= x;
	return z;
}

int atoi(const char *buf) {
	bool negative = false;
	int num = 0;

	if (!buf[0])
		return 0;

	if (buf[0] == '-') {
		negative = true;
		buf++;
	}

	size_t len = strlen(buf);
	for (ssize_t i=len-1; i>=0; i--) {
		if (buf[i] < '0' || buf[i] > '9') {
			return 0;
		} else {
			num += (buf[i] - '0') * powU(10, len-1-i);
		}
	}

	return (negative ? -num : num);
}

int abs(int num) {
	return num < 0 ? -num : num;
}
