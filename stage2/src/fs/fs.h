/**
 * \file
 * \brief     Common filesystem functions.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _FS_FS_H
#define _FS_FS_H

#include "common.h"

typedef struct FileSystemDriver FileSystemDriver;

#include "disk/disk.h"

typedef struct Partition Partition;

struct FileSystemDriver {
	const char *name;
	bool (*detect)(Partition *partition);
};


/**
 * \brief Detect a filesystem on the given partition.
 *
 * \param part
 *
 * \return zero if a filesystem was detected, non-zero on error
 */
int fsDetect(Partition *part);

#endif /* _FS_FS_H */
