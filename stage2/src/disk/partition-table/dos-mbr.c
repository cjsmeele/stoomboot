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

	if (diskRead(disk, (uint32_t)mbrBuffer, lbaStart, 1))
		return DISK_PART_SCAN_ERR_IO;

	// This might not be DOS/MBR specific.
	static const char MBR_MAGIC[] = { 0x55, 0xaa };

	if (memeq(&mbrBuffer[512 - sizeof(MBR_MAGIC)], MBR_MAGIC, sizeof(MBR_MAGIC))) {

		const DosMbr *mbr = (DosMbr*)mbrBuffer;
		const MbrPartitionTable *table = &mbr->partitionTable;

		for (int i=0; i<4 && disk->partitionCount < DISK_MAX_PARTITIONS_PER_DISK; i++) {
			const MbrPartitionTableEntry *pte = &table->entries[i];

			if (pte->lbaStart && pte->blockCount) {
				if (pte->systemId == MBR_SYSTEM_ID_EXTENDED) {
					// Parse an EBR chain.

					uint64_t extStart   = pte->lbaStart;   ///< Start of the extended partition.
					uint64_t extSize    = pte->blockCount; ///< Extended partition size.
					uint64_t nextEbr    = extStart;        ///< Start of the next EBR.
					uint64_t lastEbrEnd = 0;               ///< End of the previous EBR's logical partition.

					uint8_t ebrBuffer[DISK_MAX_BLOCK_SIZE];
					memset(ebrBuffer, 0, DISK_MAX_BLOCK_SIZE);

					for (; disk->partitionCount<DISK_MAX_PARTITIONS_PER_DISK;) {
						if (nextEbr > MIN(extStart + extSize, blockCount)) {
							printf(
								"warning: EBR out of range (%#08x > MIN(%#08x, %#08x))\n",
								(uint32_t)nextEbr, (uint32_t)(extStart + extSize), (uint32_t)blockCount
							);
							goto invalidPartitionLayout;
						}

						if (lastEbrEnd && nextEbr <= lastEbrEnd) {
							printf(
								"warning: EBRs out of order (last logpart end: %#08x, next: %#08x)\n",
								(uint32_t)lastEbrEnd, (uint32_t)nextEbr
							);
							goto invalidPartitionLayout;
						}

						if (diskRead(disk, (uint32_t)ebrBuffer, nextEbr, 1)) {
							disk->partitionCount = 0;
							memset(disk->partitions, 0, sizeof(disk->partitions));
							return DISK_PART_SCAN_ERR_IO;
						}
						DosMbr *ebr = (DosMbr*)ebrBuffer;

						if (!memeq(ebr->magic, MBR_MAGIC, sizeof(MBR_MAGIC)))
							goto invalidPartitionLayout;

						MbrPartitionTableEntry *logPte = &ebr->partitionTable.entries[0];
						MbrPartitionTableEntry *ebrPte = &ebr->partitionTable.entries[1];

						if (logPte->lbaStart && logPte->blockCount) {
							Partition *partition   = &disk->partitions[disk->partitionCount];
							partition->disk        = disk;
							partition->partitionNo = disk->partitionCount++;
							partition->lbaStart    = nextEbr + logPte->lbaStart;
							partition->blockCount  = logPte->blockCount;
							partition->type        = logPte->systemId;
							partition->active      = logPte->active & 0x80;
						} else {
							printf("warning: EBR without logical partition definition\n");
							goto invalidPartitionLayout;
						}

						if (!ebrPte->lbaStart || !ebrPte->blockCount)
							break;

						lastEbrEnd = nextEbr + 1 + logPte->blockCount;
						nextEbr    = extStart + ebrPte->lbaStart;
					}
				} else {
					Partition *partition   = &disk->partitions[disk->partitionCount];
					partition->disk        = disk;
					partition->partitionNo = disk->partitionCount++;
					partition->lbaStart    = pte->lbaStart;
					partition->blockCount  = pte->blockCount;
					partition->type        = pte->systemId;
					partition->active      = pte->active & 0x80;
				}
			}
		}

		if (verifyPartitionLayout(disk)) {
			return DISK_PART_SCAN_OK;
		} else {
		invalidPartitionLayout:
			disk->partitionCount = 0;
			memset(disk->partitions, 0, sizeof(disk->partitions));
			return DISK_PART_SCAN_ERR_CORRUPT;
		}
	} else {
		return DISK_PART_SCAN_ERR_TRY_OTHER;
	}
}
