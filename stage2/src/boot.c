/**
 * \file
 * \brief     Functions used to boot the system.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "boot.h"
#include "console.h"

void boot(BootOption *bootOption) {
	panic("'boot' unimplemented.");
	/// @todo Boot!
}

int parseBootPathString(BootFilePath *bootFile, char *str) {
	if (strneq(str, "hd", 2)) {
		str += 2;
		uint32_t numLen = strchr(str, ':') - str;
		if (!numLen || !str[numLen+1])
			goto invalidFormat;

		str[numLen] = '\0';

		uint32_t diskNo = atoi(str);
		str   += numLen + 1;
		numLen = strchr(str, ':') - str;
		if (!numLen || !str[numLen+1])
			goto invalidFormat;

		str[numLen] = '\0';

		uint32_t partNo = atoi(str);
		str   += numLen + 1;

		if (diskNo >= diskCount || partNo >= disks[diskNo].partitionCount) {
			printf("error: Boot file path disk/partition number out of range\n");
			return 1;
		}
		bootFile->path      = str;
		bootFile->partition = &disks[diskNo].partitions[partNo];

		printf("Selected boot file path: hd%u:%u, <%s>\n", diskNo, partNo, str);

	} else {
		printf("error: Sorry, only hdN:M:/path boot file path formats are currently supported\n");
		return 1;
	}

	return 0;

invalidFormat:
	printf("error: Invalid boot file path format\n");
	return 1;
}
