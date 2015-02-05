/**
 * @file
 * @brief
 * @author    Chris Smeele
 * @copyright Copyright (c) 2015, Chris Smeele
 */
#include "stage2.h"
#include "console.h"

/**
 * @brief Stage 2 entry point
 */
void stage2Main(uint16_t bootDiskNo) {
	puts("\r\nHello, world!\r\n");
	puts("Booting from disk: ");
	putnum(bootDiskNo);

	for (;;) {
		puts("\r\nPlease enter some text: ");
		char line[256];
		getLine(line, 256);

		if (line[0]) {
			puts("You entered: ");
			puts(line);
			puts("\r\n");
		} else {
			puts("You entered nothing.\r\n");
			break;
		}
	}

	puts("\r\n");

	uint16_t i = 0;
	for (;;) {
		Key key;
		if (getKey(&key, false) && key.chr == ' ') {
			puts("\rGot spacebar!                      \r\n");
			break;
		}
		puts("\rWaiting for spacebar... ");
		putnum(++i);
		halt();
	}

	puts("\r\nHalting.\r\n");
	hang();
}
