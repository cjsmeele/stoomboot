/**
 * \file
 * \brief     VFAT Filesytem.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "vfat.h"
#include "console.h"

/**
 * \brief VFAT BPB.
 */
typedef struct {
	uint8_t  _jump[3];
	char     oemName[8];
	uint16_t phyBlockSize;
	uint8_t  logBlockSize; ///< In physical blocks.
	uint16_t reservedPhyBlocks;
	uint8_t  fatCount;
	uint16_t rootDirEntries;
	uint16_t _phyBlockCountShort; // Short, do not use.
	uint8_t  mediaDescriptor;
	uint16_t _fatSizeShort;       // .
	uint16_t sectorsPerTrack;     // CHS stuff, do not use.
	uint16_t headCount;           // .
	uint32_t hiddenPhyBlocks;
	uint32_t phyBlockCount;

	// FAT32 EBPB.
	uint32_t fatSize; ///< In physical blocks.
	uint16_t flags1;
	uint16_t version;
	uint32_t rootDirLogBlock;
	uint16_t fsInfoPhyBlock;
	uint16_t fatCopyPhyBlock; ///< 3 blocks.
	uint8_t  _reserved1[12];
	uint8_t  driveNumber;
	uint8_t  _reserved2;
	uint8_t  extendedBootSignature; ///< 29 if the following fields are valid, 28 otherwise.
	uint32_t volumeId;
	char     volumeLabel[11];
	char     fsType[8];
} __attribute__((packed)) BiosParameterBlock;

/**
 * \brief Filesystem information used throughout FS operations.
 *
 * We have no heap, so to simplify memory management, a PartData structure
 * is regenerated on the stack on every FS call.
 * FS access is rare anyway.
 */
typedef struct {
	Partition *partition;
	uint8_t    logBlockSize;
	uint32_t   fatSize; ///< In physical blocks.
	uint32_t   rootDirLogBlock;

	// Add here any BPB field that may be necessary for any FS call.

} vfatPartData;

/**
 * \brief Initialize a vfatPartData structure.
 *
 * \param partData
 * \param part
 *
 * \return zero on success, non-zero on failure
 */
static int vfatInit (vfatPartData *partData, Partition *part) {
	uint8_t bpbBuffer[DISK_MAX_BLOCK_SIZE];
	if (partRead(part, (uint32_t)bpbBuffer, 0, 1))
		return -1;
	const BiosParameterBlock *bpb = (BiosParameterBlock*)bpbBuffer;

	if (bpb->phyBlockSize != part->disk->blockSize) {
		printf("warning: vfat block size (%u) does not match disk block size (%u). FS dropped.\n", bpb->phyBlockSize, part->disk->blockSize);
		return -1;
	}

	if (!part->fsInitialized) {
		char label[12] = { };
		strncpy(label, bpb->volumeLabel, ELEMS(label)-1);
		*strchr(label, ' ') = '\0';

		part->id = bpb->volumeId;
		strncpy(part->label, label, ELEMS(part->label)-1);
		part->fsInitialized = true;
	}

	partData->partition       = part;
	partData->logBlockSize    = bpb->logBlockSize;
	partData->rootDirLogBlock = bpb->rootDirLogBlock;

	return 0;
}

bool vfatDetect (Partition *part) {
	if (part->type != 0x0c)
		return false;

	vfatPartData partData;
	if (vfatInit(&partData, part))
		return false;

	return true;
}
