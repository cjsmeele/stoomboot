/**
 * \file
 * \brief     Memory mapping.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "memmap.h"
#include "console.h"

MemMap memMap;

int makeMemMap() {
	memset(&memMap, 0, sizeof(MemMap));
	uint32_t contVal = 0;

	for (uint32_t i=0; i<MEMMAP_MAX_REGIONS; i++) {
		do {
			uint32_t success = 0x0000e820;
			asm volatile (
				"int $0x15\n"
				"jnc .no_e820_error\n"
				"xor %%eax, %%eax\n"
				".no_e820_error:\n"
				: "+b" (contVal),
				  "+a" (success)
				: "c" (0x00000020), // Buffer size.
				  "d" (0x534d4150), // "SMAP".
				  "D" (&memMap.regions[i])
				: "cc", "memory"
			);

			if (!success)
				return 1;

			// Skip zero-length entries.
		} while (!memMap.regions[i].length);

		if (contVal == 0)
			// Indicates end of list.
			return 0;
	}

	printf("warning: Memory map truncated.\n");
	return 0;
}
