/**
 * \file
 * \brief     DOS MBR scanner.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "dos-mbr.h"
#include "console.h"

/**
 * \brief A Partition Table Entry in an MBR partition table.
 */
typedef struct {
	uint8_t active;
	uint8_t headStart;
	uint16_t sectorAndCylinderStart; // We might split this using bitfields,
	                                 // but we're not going to use it anyway.
	uint8_t systemId;
	uint8_t headEnd;
	uint16_t sectorAndCylinderEnd;
	uint32_t lbaStart;
	uint32_t blockCount;
} __attribute__((packed)) MbrPartitionTableEntry;

/**
 * \brief The partition table in an MBR.
 */
typedef struct {
	MbrPartitionTableEntry entries[4];
} __attribute__((packed)) MbrPartitionTable;

/**
 * \brief The Master Boot Record.
 */
typedef struct {
	uint8_t bootCode[436];
	uint8_t diskId[10]; ///< @note Not to be relied on.
	MbrPartitionTable partitionTable;
	uint8_t magic[2];
} __attribute__((packed)) DosMbr;


int dosMbrScan(Disk *disk, uint64_t lbaStart, uint64_t blockCount) {
	uint8_t mbrBuffer[DISK_MAX_BLOCK_SIZE];
	memset(mbrBuffer, 0, DISK_MAX_BLOCK_SIZE);

	if (diskRead(disk, lbaStart, (uint32_t)mbrBuffer, 1))
		return DISK_PART_SCAN_ERR_IO;

	// This might not be DOS/MBR specific.
	static const char MBR_MAGIC[] = { 0x55, 0xaa };

	if (memeq(&mbrBuffer[512 - sizeof(MBR_MAGIC)], MBR_MAGIC, sizeof(MBR_MAGIC))) {

		const DosMbr *mbr = (DosMbr*)mbrBuffer;
		const MbrPartitionTable *table = &mbr->partitionTable;

		for (int i=0; i<4; i++) {
			const MbrPartitionTableEntry *pte = &table->entries[i];

			if (pte->lbaStart && pte->blockCount && pte->systemId) {
				Partition *partition   = &disk->partitions[disk->partitionCount];
				partition->partitionNo = disk->partitionCount++;
				partition->lbaStart    = pte->lbaStart;
				partition->blockCount  = pte->blockCount;
				partition->type        = pte->systemId;
				partition->active      = pte->active & 0x80;
			}
		}

		if (verifyPartitionLayout(disk)) {
			return DISK_PART_SCAN_OK;
		} else {
			disk->partitionCount = 0;
			memset(disk->partitions, 0, sizeof(disk->partitions));
			return DISK_PART_SCAN_ERR_CORRUPT;
		}
	} else {
		return DISK_PART_SCAN_ERR_TRY_OTHER;
	}
}
