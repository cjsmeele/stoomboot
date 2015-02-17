/**
 * \file
 * \brief     VFAT Filesytem.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "vfat.h"
#include "console.h"

typedef struct {
	uint8_t jump[3];
	char oemName[8];
	uint16_t phyBlockSize;
	uint8_t  logBlockSize; ///< In physical blocks.
	uint16_t reservedPhyBlocks;
} __attribute__((packed)) BiosParameterBlock;

bool vfatDetect (Partition *part) {
	if (part->type != 0x0c)
		return false;

	{
		uint8_t bpbBuffer[DISK_MAX_BLOCK_SIZE];
		if (partRead(part, (uint32_t)bpbBuffer, 0, 1))
			return false;
		const BiosParameterBlock *bpb = (BiosParameterBlock*)bpbBuffer;
		char oemName[9];
		memcpy(oemName, bpb->oemName, 8);
		printf("hd%u:%u = vfat:\n", part->disk->diskNo, part->partitionNo);
		printf("  OEM: %s\n", oemName);
		printf("  logical block (cluster) size: %u\n", bpb->logBlockSize);
	}

	return true;
}
