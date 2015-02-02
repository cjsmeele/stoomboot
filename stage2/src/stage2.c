/**
 * @file
 * @brief
 * @author    Chris Smeele
 * @copyright Copyright (c) 2015, Chris Smeele
 */
#include "stage2.h"

/**
 * @brief Stage 2 entry point
 */
void stage2Main(uint16_t bootDiskNo) {
	puts("\r\nHello, world!\r\n");
	puts("Booting from disk: ");
	putnum(bootDiskNo);

	puts("\r\nHalting.\r\n");
	hang();
}
