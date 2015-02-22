/**
 * \file
 * \brief     Common filesystem functions.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "fs.h"
#include "vfat.h"
#include "console.h"

#define FS_SUCCESS         (0)
#define FS_IO_ERROR       (-1)
#define FS_FILE_NOT_FOUND (-2)

static FileSystemDriver fsDrivers[] = {
	{
		"vfat",
		vfatDetect,
		vfatGetFile,
		vfatReadFileBlock,
		vfatReadDir,
	},
};

int fsDetect(Partition *part) {
	for (size_t i=0; i<ELEMS(fsDrivers); i++) {
		if (fsDrivers[i].detect(part)) {
			part->fsDriver = &fsDrivers[i];
			return 0;
		}
	}

	return -1;
}
