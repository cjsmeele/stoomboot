/**
 * \file
 * \brief     Common filesystem functions.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "fs.h"
#include "console.h"

static FileSystemDriver fsDrivers[] = {
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
