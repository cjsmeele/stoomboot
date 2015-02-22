/**
 * \file
 * \brief     Stage 2 C entrypoint.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "stage2.h"
#include "console.h"
#include "disk/disk.h"


void stage2Main(uint32_t bootDiskNo, uint64_t loaderFsId) {

	if (bootDiskNo != 0x80)
		printf("warning: Boot disk (%02xh) is not 80h\n", bootDiskNo);

	if (disksDiscover() <= 0) {
		// We did not detect any usable disks, abort.
		panic("No usable disk drives detected.");
	}

	printf("\nShutting down.\n");
	shutDown();
}
