/**
 * \file
 * \brief     VFAT Filesytem.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 *
 * @todo Implement LFN support (vfat.h is now actually fat32.h).
 */
#ifndef _FS_VFAT_H
#define _FS_VFAT_H

#include "common.h"
#include "fs/fs.h"

bool vfatDetect(Partition *part);

int vfatGetFile(Partition *part, FileInfo *fileInfo, const char *path);

int vfatReadFileBlock(Partition *part, FileInfo *fileInfo, uint8_t *buffer);

int vfatReadDir(Partition *part, FileInfo *fileInfo, FileInfo files[], size_t offset, size_t count);

#endif /* _FS_VFAT_H */
