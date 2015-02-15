/**
 * \file
 * \brief     DOS MBR scanner.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "dos-mbr.h"
#include "console.h"

int dosMbrScan(Disk *disk, uint64_t lbaStart, uint64_t blockCount) {
	uint8_t mbrBuffer[DISK_MAX_BLOCK_SIZE];
	memset(mbrBuffer, 0, DISK_MAX_BLOCK_SIZE);

	if (diskRead(disk, lbaStart, (uint32_t)mbrBuffer, 1))
		return DISK_PART_SCAN_ERR_IO;

	// This might not be DOS/MBR specific.
	static const char MBR_MAGIC[] = { 0x55, 0xaa };

	if (memeq(&mbrBuffer[512 - sizeof(MBR_MAGIC)], MBR_MAGIC, sizeof(MBR_MAGIC))) {
		printf("Meh, never mind.\n");

		/// @todo Do actual parsing stuffs.

		return DISK_PART_SCAN_OK;
	} else {
		return DISK_PART_SCAN_ERR_TRY_OTHER;
	}
}
