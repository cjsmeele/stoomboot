/**
 * @file
 * @brief
 * @author    Chris Smeele
 * @copyright Copyright (c) 2015, Chris Smeele
 */
#include "common.h"

void halt() {
	asm volatile(
		"hlt"
	);
}

void hang() {
	for (;;)
		asm volatile(
			"cli\n"
			"hlt\n"
		);
}
