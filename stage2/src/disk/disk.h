/**
 * \file
 * \brief     Disk I/O.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _DISK_DISK_H
#define _DISK_DISK_H

#include "common.h"
#include "fs/fs.h"

#define DISK_MAX_DISKS               8
#define DISK_MAX_PARTITIONS_PER_DISK 16
#define DISK_MAX_BLOCK_SIZE          4096

#define DISK_PART_SCAN_OK             (0)
#define DISK_PART_SCAN_ERR_TRY_OTHER (-1)
#define DISK_PART_SCAN_ERR_CORRUPT   (-2)
#define DISK_PART_SCAN_ERR_IO        (-3)

typedef struct Disk Disk; // Allows referring to Disk in the Partition struct.
typedef struct Partition Partition;

/**
 * \brief Disk partition.
 */
struct Partition {
	Disk             *disk;
	FileSystemDriver *fsDriver; ///< NULL if no usable FS was detected.
	uint64_t id; ///< Some FS' UUIDs may need to be trimmed to fit in this field.
	char     label[16];
	uint16_t partitionNo;
	uint64_t lbaStart;
	uint64_t blockCount;
	uint8_t  type;
	bool     active;
	bool     fsInitialized; ///< Whether FS code has initialized Partition fields (currently only applies to 'label' and 'id').
};

/**
 * \brief Hard disk.
 */
struct Disk {
	bool     available; ///< Whether we can read from this disk.
	uint16_t diskNo;    ///< 0, 1 ...
	uint8_t  biosId;    ///< 80h, 81h, ... (used for int13h).
	uint16_t blockSize; ///< Usually 512, may be 4096.
	uint64_t blockCount;
	uint16_t partitionCount;
	Partition partitions[DISK_MAX_PARTITIONS_PER_DISK];
};

extern uint32_t diskCount;
extern Disk     disks[];

/**
 * \brief Get a pointer to the Disk structure for the boot disk.
 *
 * \return
 */
Disk *getBootDisk();

/**
 * \brief Read blocks from a hard drive.
 *
 * \param disk a pointer to a disk structure
 * \param dest destination address
 * \param lba logical block address
 * \param blockCount amount of blocks to read
 *
 * \return zero on success, non-zero on error
 */
int diskRead(Disk *disk, uint64_t dest, uint64_t lba, uint64_t blockCount);

/**
 * \brief Read blocks from a partition.
 *
 * Does bound checks.
 *
 * \param part
 * \param dest
 * \param relLba
 * \param blockCount
 *
 * \return zero on success, non-zero on error
 */
int partRead(Partition *part, uint64_t dest, uint64_t relLba, uint64_t blockCount);

/**
 * \brief Detects disk drives, parses partition tables, fills Disk structs.
 *
 * \return the amount of available disk drives (disks we can read from)
 */
int disksDiscover();

#endif /* _DISK_DISK_H */
