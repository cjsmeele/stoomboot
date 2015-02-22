/**
 * \file
 * \brief     GUID Partition Table scanner.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "gpt.h"
#include "console.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int gptScan(Disk *disk, uint64_t lbaStart, uint64_t blockCount) {
	/// @todo Parse GPT.

	printf("warning: GPT scanner unimplemented.\n");

	return DISK_PART_SCAN_ERR_TRY_OTHER;
}

#pragma GCC diagnostic pop
