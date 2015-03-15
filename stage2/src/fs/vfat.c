/**
 * \file
 * \brief     VFAT Filesytem.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 *
 * @todo Reading files is currently horribly inefficient, see VfatPartData.
 */
#include "vfat.h"
#include "console.h"
#include "dump.h"

#define VFAT_READ_ERROR (-1)
#define VFAT_READ_EOF   (-2)

/**
 * \brief VFAT BPB.
 */
typedef struct {
	uint8_t  _jump[3];
	char     oemName[8];
	uint16_t blockSize;
	uint8_t  clusterSize; ///< In blocks.
	uint16_t reservedBlocks;
	uint8_t  fatCount;
	uint16_t rootDirEntries;
	uint16_t _blockCountShort; // Short, do not use.
	uint8_t  mediaDescriptor;
	uint16_t _fatSizeShort;    // .
	uint16_t sectorsPerTrack;  // CHS stuff, do not use.
	uint16_t headCount;        // .
	uint32_t hiddenBlocks;
	uint32_t blockCount;

	// FAT32 EBPB.
	uint32_t fatSize; ///< In blocks.
	uint16_t flags1;
	uint16_t version;
	uint32_t rootDirCluster;
	uint16_t fsInfoBlock;
	uint16_t fatCopyBlock; ///< 3 blocks.
	uint8_t  _reserved1[12];
	uint8_t  driveNumber;
	uint8_t  _reserved2;
	uint8_t  extendedBootSignature; ///< 29 if the following fields are valid, 28 otherwise.
	uint32_t volumeId;
	char     volumeLabel[11];
	char     fsType[8];

	uint8_t _bootCode[420];
	uint8_t _signature[2];
} __attribute__((packed)) BiosParameterBlock;

/**
 * \brief Filesystem information used throughout FS operations.
 *
 * We have no heap, so to simplify memory management, a PartData structure
 * is regenerated on the stack on *every* FS call.
 *
 * This is very inefficient though. We might solve this by using a single static
 * VfatPartData structure.
 *
 * FS access is rare anyway.
 */
typedef struct {
	Partition *partition;
	uint32_t fatStart; ///< In blocks.
	uint32_t fatSize;  ///< In blocks.
	uint32_t fatCount;
	uint32_t dataStart;
	uint32_t rootDirCluster;

	uint32_t currentClusterNo;
	uint32_t currentFatBlockNo; ///< Partition LBA that contains the currently buffered FAT block.
	uint8_t  currentFatBlock[DISK_MAX_BLOCK_SIZE];
	uint32_t currentClusterBlockNo; ///< Block number within the current cluster.

	uint8_t  clusterSize;

	bool endOfCluster;

} __attribute__((packed)) VfatPartData;

/**
 * \brief FAT directory entry structure.
 */
typedef struct {
	char name[8];      ///< Padded with spaces.
	char extension[3]; ///< Padded with spaces.

	// Attribute byte 1.
	uint8_t attrReadOnly    : 1;
	uint8_t attrHidden      : 1;
	uint8_t attrSystem      : 1;
	uint8_t attrVolumeLabel : 1;
	uint8_t attrDirectory   : 1;
	uint8_t attrArchive     : 1;
	uint8_t attrDisk        : 1;
	uint8_t _attrReserved   : 1;

	uint8_t  attributes2; ///< Do not use.

	uint8_t  createTimeHiRes;
	uint16_t createTime;
	uint16_t createDate;
	uint16_t accessDate;
	uint16_t clusterNoHigh;
	uint16_t modifyTime;
	uint16_t modifyDate;
	uint16_t clusterNoLow;
	uint32_t fileSize; ///< In bytes.
} __attribute__((packed)) VfatDirEntry;

/**
 * \brief Convert a cluster number to a block address that contains the FAT entry for that cluster number.
 *
 * \param partData
 * \param clusterNo
 *
 * \return
 */
static inline uint32_t getFatBlockNoForClusterNo(VfatPartData *partData, uint32_t clusterNo) {
	return partData->fatStart + clusterNo / (partData->partition->disk->blockSize / 4);
};

/**
 * \brief Load the FAT block that describes the given cluster number.
 *
 * \param partData
 * \param clusterNo
 *
 * \return
 */
static int seekCluster(VfatPartData *partData, uint32_t clusterNo) {
	partData->currentClusterBlockNo = 0;
	partData->currentClusterNo      = clusterNo;
	partData->endOfCluster          = false;

	uint32_t fatBlockNo = getFatBlockNoForClusterNo(partData, partData->currentClusterNo);
	if (partData->currentFatBlockNo == fatBlockNo) {
		return 0;
	} else {
		partData->currentFatBlockNo = fatBlockNo;
		return partRead(
			partData->partition,
			(uint32_t)partData->currentFatBlock,
			partData->currentFatBlockNo,
			1
		);
	}
}

/**
 * \brief Get the next cluster number in the current cluster chain.
 *
 * \param partData
 *
 * \return a cluster number, or 0 if the end of the cluster chain was reached
 */
static uint32_t getNextClusterNo(VfatPartData *partData) {
	uint32_t nextClusterNo = ((uint32_t*)(partData->currentFatBlock))[
		partData->currentClusterNo % (partData->partition->disk->blockSize / 4)
	];

	if ((nextClusterNo & 0x0ffffff8) == 0x0ffffff8)
		// Anything higher than or equal to 0x0ffffff8 indicates the end of cluster chain.
		// '0' is not a valid cluster number.
		return 0;
	else
		return nextClusterNo;
}

/**
 * \brief Reads the next file block belonging to the sought cluster number.
 *
 * \param partData
 *
 * \return zero on success, non-zero on error or EOF
 * \retval 0
 * \retval VFAT_READ_ERROR
 * \retval VFAT_READ_EOF
 */
static int readClusterBlock(VfatPartData *partData, uint8_t *buffer) {

	assert(!partData->endOfCluster);

	if (partData->currentClusterBlockNo >= partData->clusterSize) {
		// End of cluster reached, seek to next cluster in chain.
		uint32_t nextClusterNo = getNextClusterNo(partData);
		if (nextClusterNo) {
			seekCluster(partData, nextClusterNo);
		} else {
			partData->endOfCluster = true;
			return VFAT_READ_EOF;
		}
	}

	if (partRead(
			partData->partition,
			(uint32_t)buffer,
			(
				partData->dataStart
				+ (partData->currentClusterNo - 2) * partData->clusterSize
				+ partData->currentClusterBlockNo++
			),
			1
		)) {
		return VFAT_READ_ERROR;
	} else {
		return 0;
	}
}

/**
 * \brief Initialize a VfatPartData structure.
 *
 * \param partData
 * \param part
 *
 * \return zero on success, non-zero on failure
 */
static int vfatInit (VfatPartData *partData, Partition *part) {
	uint8_t bpbBuffer[DISK_MAX_BLOCK_SIZE];
	if (partRead(part, (uint32_t)bpbBuffer, 0, 1))
		return -1;
	const BiosParameterBlock *bpb = (BiosParameterBlock*)bpbBuffer;

	if (bpb->blockSize != part->disk->blockSize) {
		printf("error: Unmatched vfat block size (%u != %u).\n", bpb->blockSize, part->disk->blockSize);
		return -1;
	}

	if (!part->fsInitialized) {
		char label[12] = { };
		strncpy(label, bpb->volumeLabel, ELEMS(label)-1);
		*strchr(label, ' ') = '\0';

		part->fsId = bpb->volumeId;
		strncpy(part->fsLabel, label, ELEMS(part->fsLabel)-1);
		part->fsInitialized = true;
	}

	partData->partition      = part;
	partData->fatStart       = bpb->reservedBlocks;
	partData->fatSize        = bpb->fatSize;
	partData->fatCount       = bpb->fatCount;
	partData->dataStart      = partData->fatStart + bpb->fatCount * bpb->fatSize;
	partData->clusterSize    = bpb->clusterSize;
	partData->rootDirCluster = bpb->rootDirCluster;

	return 0;
}

bool vfatDetect (Partition *part) {
	if (part->type != 0x0c)
		return false;

	VfatPartData partData;
	memset(&partData, 0, sizeof(VfatPartData));
	if (vfatInit(&partData, part))
		return false;

	return true;
}

/**
 * \brief Traverse a directory tree and fill the given FileInfo struct with the requested file.
 *
 * \param partData
 * \param fileInfo
 * \param rootCluster
 * \param path
 *
 * \return zero on success, non-zero on failure
 * \retval FS_SUCCESS
 * \retval FS_IO_ERROR
 * \retval FS_FILE_NOT_FOUND
 */
static int getFile(VfatPartData *partData, FileInfo *fileInfo, uint32_t rootCluster, const char *path) {
	size_t nextPathPartLength = strchr(path, '/') - path;
	char curPathPart[nextPathPartLength + 1];
	memset( curPathPart, 0,    sizeof(curPathPart));
	strncpy(curPathPart, path, sizeof(curPathPart)-1);

	if (seekCluster(partData, rootCluster))
		return FS_IO_ERROR;

	// Read the containing directory.

	uint8_t buffer[partData->partition->disk->blockSize];
	VfatDirEntry dentry;
	memset(&dentry, 0, sizeof(VfatDirEntry));
	int ret;
	bool endOfDirectory = false;

	while ((ret = readClusterBlock(partData, buffer)) != VFAT_READ_EOF && !endOfDirectory) {
		if (ret == VFAT_READ_ERROR) {
			printf("warning: Could not read cluster %#08x\n", rootCluster);
			return FS_IO_ERROR;
		}

		// For each directory entry...

		for (uint32_t i=0; i<partData->partition->disk->blockSize; i += sizeof(VfatDirEntry)) {
			VfatDirEntry *dentry = (VfatDirEntry*)&buffer[i];
			if (dentry->name[0] == 0) {
				// Entry is available, no more entries in this directory.
				// We didn't find the requested file.
				endOfDirectory = true;
				break;
			} else if (dentry->name[0] == '\xe5') {
				// Entry is available.
				continue;
			}

			if (dentry->attrVolumeLabel || dentry->attrDisk)
				// We are only interested in directories and regular files.
				continue;

			// Extract a file name from the directory entry.
			char fileName[13] = { };
			strncpy(fileName, dentry->name, 8);
			rtrim(fileName);

			if (dentry->extension[0] && dentry->extension[0] != ' ') {
				fileName[strlen(fileName)] = '.';
				strncpy(&fileName[strlen(fileName)], dentry->extension, 3);
				rtrim(fileName);
			}
			toLowerCase(fileName);

			if (streq(fileName, curPathPart)) {
				// Found it!
				uint32_t clusterNo = dentry->clusterNoHigh << 16 | dentry->clusterNoLow;

				if (streq(curPathPart, path)) {
					// This is the requested file.
					strncpy(fileInfo->name, fileName, sizeof(fileInfo->name));

					fileInfo->partition        = partData->partition;
					fileInfo->fsAddressStart   = clusterNo;
					fileInfo->fsAddressCurrent = (uint64_t)clusterNo << 32;
					fileInfo->size             = dentry->fileSize;

					fileInfo->type =
						dentry->attrDirectory
						? FILE_TYPE_DIRECTORY
						: FILE_TYPE_REGULAR;

					return FS_SUCCESS;

				} else {
					// We need to go deeper!
					/// @todo Prevent possible stack overflow in looping / very deep directories.
					return getFile(partData, fileInfo, clusterNo, path + strlen(curPathPart) + 1);
				}
			}
		}
	}

	return FS_FILE_NOT_FOUND;
}

int vfatGetFile(Partition *part, FileInfo *fileInfo, const char *path) {
	VfatPartData partData;
	memset(&partData, 0, sizeof(VfatPartData));
	if (vfatInit(&partData, part))
		return FS_INTERNAL_ERROR;

	assert(strlen(path) > 0);

	if (path[0] != '/') {
		printf("warning: Relative paths are not supported ('%s')\n", path);
		return FS_INTERNAL_ERROR;
	}

	return getFile(&partData, fileInfo, partData.rootDirCluster, path + 1);
}

int vfatReadFileBlock(FileInfo *fileInfo, uint8_t *buffer) {
	VfatPartData partData;
	memset(&partData, 0, sizeof(VfatPartData));
	if (vfatInit(&partData, fileInfo->partition))
		return FS_INTERNAL_ERROR;

	if (seekCluster(&partData, fileInfo->fsAddressCurrent >> 32))
		return FS_INTERNAL_ERROR;

	partData.currentClusterBlockNo = (uint32_t)fileInfo->fsAddressCurrent;

	if (readClusterBlock(&partData, buffer)) {
		return FS_IO_ERROR;
	} else {
		fileInfo->fsAddressCurrent = (uint64_t)partData.currentClusterNo << 32 | partData.currentClusterBlockNo;
		return FS_SUCCESS;
	}
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int vfatReadDir(FileInfo *fileInfo, FileInfo files[], size_t offset, size_t count) {
	panic("VFAT readDir() unimplemented.");
	return 0;
}

#pragma GCC diagnostic pop
