/**
 * \file
 * \brief     DOS MBR scanner.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _DISK_PARTITION_TABLE_DOS_MBR_H
#define _DISK_PARTITION_TABLE_DOS_MBR_H

#include "common.h"
#include "partition-table.h"
#include "disk/disk.h"

#define MBR_SYSTEM_ID_EXTENDED 0x05
#define MBR_SYSTEM_ID_GPT      0xee

/**
 * \brief Detects and scans a DOS-style MBR partition table.
 *
 * Note that finding a protective MBR counts as a success; The caller should
 * check for the existence of a GPT partition and call the GPT scan function to
 * overwrite the disk's partition list.
 *
 * \param disk
 * \param lbaStart where the MBR covered disk section starts (usually 0)
 * \param blockCount the size of the disk section covered by the MBR (usually $BLOCKCOUNT)
 *
 * \return zero on success, non-zero on failure
 * \retval DISK_PART_SCAN_OK
 * \retval DISK_PART_SCAN_ERR_TRY_OTHER
 * \retval DISK_PART_SCAN_ERR_CORRUPT
 * \retval DISK_PART_SCAN_ERR_IO
 */
int dosMbrScan(Disk *disk, uint64_t lbaStart, uint64_t blockCount);

#endif /* _DISK_PARTITION_TABLE_DOS_MBR_H */
