/**
 * \file
 * \brief     VFAT Filesytem.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _FS_VFAT_H
#define _FS_VFAT_H

#include "common.h"
#include "fs/fs.h"

/**
 * \brief Detects a VFAT filesystem.
 *
 * \param part
 *
 * \return  whether the given partition contains a VFAT filesystem
 */
bool vfatDetect (Partition *part);

#endif /* _FS_VFAT_H */
