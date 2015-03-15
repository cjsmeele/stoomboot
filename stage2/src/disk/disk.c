/**
 * \file
 * \brief     Disk I/O.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "disk.h"
#include "bda.h"
#include "console.h"
#include "partition-table/dos-mbr.h"
#include "partition-table/gpt.h"

uint32_t diskCount = 0;
Disk     disks[DISK_MAX_DISKS];

/**
 * \brief Drive parameters structure, as returned by int 13h AH=48h.
 */
typedef struct {
	uint16_t bufferSize;
	uint16_t flags;               ///< \todo Bitfieldify?
	uint32_t physCylinders;       ///< CHS stuff, do not use.
	uint32_t physHeads;           ///< CHS stuff, do not use.
	uint32_t physSectorsPerTrack; ///< CHS stuff, do not use.
	uint64_t physSectors;
	uint16_t bytesPerSector;
	uint32_t dptePtr;             ///< seg:off.
	uint16_t dpiKey;              ///< Should be BEDDh.
	uint8_t  dpiLength;           ///< Includes key length (=36).
	uint8_t  _reserved1;
	uint16_t _reserved2;
	char     busType[4];          ///< "PCI" or "ISA".
	char     interfaceType[8];    ///< "ATA", "ATAPI", "SCSI", "USB", "1394" or "FIBRE".
	uint64_t interfacePath;
	uint64_t devicePath;
	uint8_t  _reserved3;
	uint8_t  dpiChecksum; ///< Checksum for Device Path Information.
	                      ///  Includes the 0BEDDh signature (dpiKey).
	                      ///  2's complement of the sum of bytes 30-64.
	                      ///  The sum of bytes 30-65 is 0.
} __attribute__((packed)) DriveParameters;

/**
 * \brief Disk Address Packet for extended int13h calls.
 */
typedef struct {
	uint8_t length;     ///< Length of the DAP, either 0x10 or 0x18 when using the 64-bit destination address.
	uint8_t _reserved1;
	uint8_t blockCount; ///< The amount of blocks to read (max. 127).
	uint8_t _reserved2;
	uint16_t destinationOff; ///< Should be set to 0xffff when using 64-bit dest addresses.
	uint16_t destinationSeg; ///< Should be set to 0xffff when using 64-bit dest addresses.
	uint64_t lba;            ///< Logical Block Address.
	uint64_t destination;    ///< Flat 64-bit destination address.
} __attribute__((packed)) DiskAddressPacket;


Partition *getPartitionByFsId(uint64_t fsId) {
	for (uint32_t i=0; i<diskCount; i++) {
		for (uint32_t j=0; j<disks[i].partitionCount; j++) {
			Partition *part = &disks[i].partitions[j];
			if (part->fsDriver && part->fsId == fsId)
				return part;
		}
	}
	return NULL;
}

Partition *getPartitionByFsLabel(const char *fsLabel) {
	for (uint32_t i=0; i<diskCount; i++) {
		for (uint32_t j=0; j<disks[i].partitionCount; j++) {
			Partition *part = &disks[i].partitions[j];
			if (part->fsDriver && streq(part->fsLabel, fsLabel))
				return part;
		}
	}
	return NULL;
}

int diskRead(Disk *disk, uint64_t dest, uint64_t lba, uint64_t blockCount) {

	/// \note Using the 64-bit flat address doesn't seem to work in qemu and bochs.

	assert(blockCount <= 127);

	if (dest + (blockCount * disk->blockSize) >= 0x100000)
		panic("Reading from disk to extended memory is not supported.");

	DiskAddressPacket dap;
	memset(&dap, 0, sizeof(DiskAddressPacket));
	//dap.length      = 0x18;
	dap.length      = 0x10;
	dap.blockCount  = blockCount;
	//dap.destinationSeg = 0xffff; // Indicates that we want to use the flat address instead of seg:offset.
	//dap.destinationOff = 0xffff; // .
	dap.destinationSeg = (dest >> 4) & 0xf000;
	dap.destinationOff = dest;
	dap.lba         = lba;
	dap.destination = dest;

	uint16_t errorCode = 0;

	asm volatile (
		"int $0x13\n"
		"jc .error1\n"
		"xor %%ax, %%ax\n"
		".error1:\n"
		: "=a" (errorCode)
		: "a" (0x4200),
		  "d" (disk->biosId),
		  "S" (&dap)
		: "memory", "cc"
	);

	errorCode >>= 8;

	if (errorCode) {
		printf("warning: Disk I/O error: disk=%02xh, AH=%02xh\n", disk->biosId, errorCode);
		printf(
			"         lba: %#08x.%08x, block count %#08x.%08x\n",
			(uint32_t)(lba        >> 32), (uint32_t)lba,
			(uint32_t)(blockCount >> 32), (uint32_t)blockCount
		);
		return -1;
	} else {
		return 0;
	}
}

int partRead(Partition *part, uint64_t dest, uint64_t relLba, uint64_t blockCount) {
	if (
			   relLba              >= part->blockCount
			|| blockCount          >  part->blockCount
			|| relLba + blockCount >  part->blockCount
		) {
		printf("warning: Tried to read outside of partition boundaries\n");
		return -1;
	}

	return diskRead(part->disk, dest, part->lbaStart + relLba, blockCount);
}

/**
 * \brief Get a drive's parameters.
 *
 * \param disk
 *
 * \return zero on success, non-zero on error
 */
static int diskGetParams(Disk *disk) {
	DriveParameters params;
	memset(&params, 0, sizeof(DriveParameters));

	params.bufferSize = sizeof(DriveParameters);

	uint16_t errorCode = 0;

	asm volatile (
		"int $0x13\n"
		"jc .error2\n"
		"xor %%ax, %%ax\n"
		".error2:\n"
		: "=a" (errorCode)
		: "a" (0x4800),
		  "d" (disk->biosId),
		  "S" (&params)
		: "memory", "cc"
	);

	errorCode >>= 8;

	if (errorCode) {
		printf("warning: Could not get drive parameters for drive %02xh, error %02xh\n", disk->biosId, errorCode);
		return -1;
	}

	disk->blockSize  = params.bytesPerSector;
	disk->blockCount = params.physSectors;

	if (
			  !disk->blockSize
			|| disk->blockSize % 512
			|| disk->blockSize > DISK_MAX_BLOCK_SIZE
		){
		printf("warning: Block size %u not supported on disk %02xh\n", disk->blockSize, disk->biosId);
		return -1;
	}

	/// @todo Extract DPI?

	return 0;
}

/**
 * \brief A named pointer to a function that parses partition tables.
 */
typedef struct {
	const char *name;
	int (*scannerFunc)(Disk*, uint64_t lbaStart, uint64_t blockCount);
} DiskScanner;

/**
 * \brief Collection of partition table scanner functions for each supported
 *        partition table type.
 */
static const DiskScanner diskScanners[] = {
	{ "dos-mbr", dosMbrScan },
	{ "gpt",     gptScan    },
};

/**
 * \brief Get drive parameters and scan a disk's partition table.
 *
 * \param disk a disk structure
 *
 * \return the amount of partitions detected, or -1 on error
 */
static int diskScan(Disk *disk) {

	//printf("Scanning disk %02xh\n", disk->biosId);

	if (diskGetParams(disk))
		// Something's not right with the drive parameters, drop the disk.
		return -1;

	int ret = DISK_PART_SCAN_ERR_TRY_OTHER;

	for (size_t i=0; i<ELEMS(diskScanners); i++) {
		//printf("-> scanning partition table type '%s'\n", diskScanners[i].name);
		ret = diskScanners[i].scannerFunc(disk, 0, disk->blockCount);
		if (ret == DISK_PART_SCAN_OK) {
			/// @todo Check for GPT partition, etc.
			break;
		} else if (ret == DISK_PART_SCAN_ERR_CORRUPT) {
			// The partition table was recognized by the scanner, but contains errors.
			printf(
				"warning: Partition table for disk %02xh is corrupt\n",
				disk->biosId
			);
			break;
		} else if (ret == DISK_PART_SCAN_ERR_IO) {
			// No use in retrying different scanners on a bad disk.
			disk->available = false;
			break;
		} else if (ret == DISK_PART_SCAN_ERR_TRY_OTHER) {
			continue;
		}
	}

	if (ret == DISK_PART_SCAN_ERR_TRY_OTHER) {
		printf("warning: Could not detect partition table on disk %02xh\n", disk->biosId);
		return 0; // No partitions detected, but we might want to chainload.
	} else if (ret == DISK_PART_SCAN_ERR_CORRUPT) {
		return 0; // .
	} else if (ret == DISK_PART_SCAN_ERR_IO) {
		return -1; // Disk is unusable.
	} else if (ret == DISK_PART_SCAN_OK) {
		return disk->partitionCount;
	} else {
		return -1;
	}
}

int disksDiscover() {
	diskCount = bda->hdCount;
	uint32_t availableDisks = 0;

	if (diskCount > DISK_MAX_DISKS)
		printf("Scanning %u disk(s) out of %u\n", MIN(diskCount, DISK_MAX_DISKS), diskCount);

	diskCount = MIN(diskCount, DISK_MAX_DISKS);

	// Detect partitions and filesystems.
	for (uint32_t i=0; i<MIN(diskCount, DISK_MAX_DISKS); i++) {
		memset(&disks[i], 0, sizeof(Disk));

		disks[i].diskNo = i;
		disks[i].biosId = 0x80 + i;

		int partitionCount = diskScan(&disks[i]);
		if (partitionCount > 0) {
			disks[i].available = true;
			availableDisks++;

			for (uint32_t j=0; j<disks[i].partitionCount; j++)
				fsDetect(&disks[i].partitions[j]);

		} else if (partitionCount == 0) {
			printf("warning: No partitions found on disk %02xh\n", disks[i].biosId);
			availableDisks++; // We might want to chainload to this disk.

		}
	}

	return availableDisks;
}
