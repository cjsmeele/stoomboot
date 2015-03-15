/**
 * \file
 * \brief     Functions used to boot the system.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "boot.h"
#include "console.h"
#include "fs/fs.h"
#include "elf.h"
#include "config.h"
#include "vbe.h"
#include "multiboot.h"
#include "stage2.h"

void boot(BootOption *bootOption) {

	Partition *kernelPartition = bootOption->kernel.partition;

	if (!kernelPartition->fsDriver) {
		printf("error: Kernel partition has no FS driver\n");
		return;
	}

	FileInfo fileInfo;
	memset(&fileInfo, 0, sizeof(FileInfo));

	int ret = kernelPartition->fsDriver->getFile(
		kernelPartition,
		&fileInfo,
		bootOption->kernel.path
	);

	if (ret == FS_SUCCESS && fileInfo.type == FILE_TYPE_REGULAR) {
		ConfigOption *videoWidth  = getConfigOption("video-width");
		ConfigOption *videoHeight = getConfigOption("video-height");
		ConfigOption *videoBpp    = getConfigOption("video-bbp");
		ConfigOption *videoMode   = getConfigOption("video-mode");

		bool modeSet = false;

		if (videoMode->value.valInt32) {
			modeSet = !vbeSetMode(videoMode->value.valInt32);
		} else if (videoWidth->value.valInt32 && videoHeight->value.valInt32) {
			uint16_t mode = vbeGetModeFromModeInfo(
				videoWidth->value.valInt32,
				videoHeight->value.valInt32,
				videoBpp->value.valInt32
			);

			if (mode != 0xffff)
				modeSet = !vbeSetMode(mode);
		}

		// Note: We do not read the kernel's multiboot header; We simply assume that
		//       the kernel will be an ELF image and supply a memory map.
		//       Any video mode switching must be specified in the loader.rc file.
		generateMultibootInfo(kernelPartition);

		loadElf(&fileInfo);

		// Boot failed. :(

		if (modeSet) {
			// Reset display to 80x25 text mode.
			msleep(3000);
			setVideoMode(3);
		}
	} else if (ret == FS_FILE_NOT_FOUND || fileInfo.type != FILE_TYPE_REGULAR) {
		printf(
			"error: Kernel binary not found at hd%u:%u:%s\n",
			kernelPartition->disk->diskNo,
			kernelPartition->partitionNo,
			bootOption->kernel.path
		);
	} else {
		printf("An error occurred while locating the kernel binary on disk\n");
	}
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

	} else if (strneq(str, "FSID=", 5)) {
		str += 5;
		uint32_t idLen = strchr(str, ':') - str;
		if (!idLen || !str[idLen+1])
			goto invalidFormat;

		str[idLen] = '\0';

		if (idLen < 1 || idLen > 16)
			goto invalidFormat;

		toLowerCase(str);

		uint64_t id = 0;
		for (uint32_t i=0; i<idLen; i++) {
			if (
				   (str[i] >= '0' && str[i] <= '9')
				|| (str[i] >= 'a' && str[i] <= 'f')
			) {
				id |= (uint64_t)(
					str[i] >= '0' && str[i] <= '9'
					? str[i] - '0'
					: str[i] - 'a' + 10
				) << ((idLen - (i+1)) * 4);
			}
		}

		bootFile->partition = getPartitionByFsId(id);
		if (!bootFile->partition) {
			printf("error: No filesystem with id %08x-%08x was found\n", (uint32_t)(id >> 32), (uint32_t)id);
			return 1;
		}

		bootFile->path = str + idLen + 1;

	} else if (strneq(str, "FSLABEL=", 8)) {
		str += 8;
		uint32_t labelLen = strchr(str, ':') - str;
		if (!labelLen || !str[labelLen+1])
			goto invalidFormat;

		str[labelLen] = '\0';

		bootFile->partition = getPartitionByFsLabel(str);
		if (!bootFile->partition) {
			printf("error: No filesystem with label '%s' was found\n", str);
			return 1;
		}

		bootFile->path = str + labelLen + 1;

	} else if (str[0] == '/') {
		if (loaderPart) {
			bootFile->path      = str;
			bootFile->partition = loaderPart;
		} else {
			printf("error: Boot file path is incomplete\n");
			return 1;
		}
	} else {
		goto invalidFormat;
	}

	return 0;

invalidFormat:
	printf("error: Invalid boot file path format\n");
	return 1;
}
