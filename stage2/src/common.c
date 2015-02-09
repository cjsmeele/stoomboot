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
			"mloop: stosb\n"
			"loop mloop\n"
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
			"m2loop: movsb\n"
			"loop m2loop\n"
			:
			: "c" (length),
			  "D" (dest),
			  "S" (source)
			: "memory"
		);

	return dest;
}

size_t strlen(const char *str) {
	uint32_t i = 0;
	while (*str++)
		i++;
	return i;
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
