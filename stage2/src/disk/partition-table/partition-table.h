/**
 * \file
 * \brief     Common functions related to partition tables.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _DISK_PARTITION_TABLE_PARTITION_TABLE_H
#define _DISK_PARTITION_TABLE_PARTITION_TABLE_H

#include "common.h"
#include "disk/disk.h"

/**
 * \brief Verify if a partition layout looks sane.
 *
 * Checks for overlapping and out of bounds partitions.
 *
 * \param disk
 *
 * \return whether the partition layout makes sense
 */
bool verifyPartitionLayout(Disk *disk);

#endif /* _DISK_PARTITION_TABLE_PARTITION_TABLE_H */
