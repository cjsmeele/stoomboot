/**
 * \file
 * \brief     Stage 2 C entrypoint.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "stage2.h"
#include "console.h"

/**
 * \brief Stage 2 entry point.
 */
void stage2Main(uint32_t bootDiskNo) {
	printf("\nHello, world!\n");
	printf("Booting from disk: %02xh\n", bootDiskNo);

	printf("\nHalting.\n");
	hang();
}
