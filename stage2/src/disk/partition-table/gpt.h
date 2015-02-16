/**
 * \file
 * \brief     GUID Partition Table scanner.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _DISK_PARTITION_TABLE_GPT_H
#define _DISK_PARTITION_TABLE_GPT_H

#include "common.h"
#include "partition-table.h"
#include "disk/disk.h"

/**
 * \brief Detects and scans a GUID Partition Table.
 *
 * \param disk
 * \param lbaStart where the GPT starts (usually 1)
 * \param blockCount the size of the disk section covered by the GPT (usually $BLOCKCOUNT - 1 for the protective MBR)
 *
 * \return zero on success, non-zero on failure
 * \retval DISK_PART_SCAN_OK
 * \retval DISK_PART_SCAN_ERR_TRY_OTHER
 * \retval DISK_PART_SCAN_ERR_CORRUPT
 * \retval DISK_PART_SCAN_ERR_IO
 */
int gptScan(Disk *disk, uint64_t lbaStart, uint64_t blockCount);

#endif /* _DISK_PARTITION_TABLE_GPT_H */
