/**
 * @file
 * @brief
 * @author    Chris Smeele
 * @copyright Copyright (c) 2015, Chris Smeele
 */
#include "ioport.h"

void outb(uint16_t port, uint8_t value) {
	asm volatile (
		"outb %0, %1"
		:
		: "a" (value),
		  "d" (port)
	);
}

void outw(uint16_t port, uint16_t value) {
	asm volatile (
		"outw %0, %1"
		:
		: "a" (value),
		  "d" (port)
	);
}

uint8_t inb(uint16_t port) {
	uint8_t value;

	asm volatile (
		"outb %0, %1"
		: "=a" (value)
		: "d" (port)
	);

	return value;
}

uint16_t inw(uint16_t port) {
	uint16_t value;

	asm volatile (
		"inw %1, %0"
		: "=a" (value)
		: "d" (port)
	);

	return value;
}

void iowait() {
	outb(0x80, 0);
}
