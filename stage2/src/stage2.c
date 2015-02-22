/**
 * \file
 * \brief     Stage 2 C entrypoint.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "stage2.h"
#include "console.h"
#include "disk/disk.h"
#include "dump.h"


void stage2Main(uint32_t bootDiskNo, uint64_t loaderFsId) {

	printf("\nWelcome to the Havik bootloader.\n\n");

	if (bootDiskNo != 0x80)
		printf("warning: Boot disk (%02xh) is not 80h\n", bootDiskNo);

	if (disksDiscover() <= 0) {
		// We did not detect any usable disks, abort.
		panic("No usable disk drives detected.");
	}

	putch('\n');

	Partition *loaderPart = getPartitionByFsId(loaderFsId);

	if (loaderPart) {
		printf(
			"Loader fs found at hd%u:%u: fs type %s, id %08x-%08x, label '%s'\n",
			loaderPart->disk->diskNo,
			loaderPart->partitionNo,
			loaderPart->fsDriver->name,
			(uint32_t)(loaderPart->fsId >> 32),
			(uint32_t)loaderPart->fsId,
			loaderPart->fsLabel
		);

		FileInfo fileInfo;
		memset(&fileInfo, 0, sizeof(FileInfo));
		int ret = loaderPart->fsDriver->getFile(loaderPart, &fileInfo, CONFIG_LOADER_CONFIG_PATH);
		if (ret == FS_FILE_NOT_FOUND) {
			printf(
				"error: Bootloader configuration file not found at hd%u:%u:%s\n",
				loaderPart->disk->diskNo,
				loaderPart->partitionNo,
				CONFIG_LOADER_CONFIG_PATH
			);
			panic("Could not find bootloader configuration file");
		}
		assert(fileInfo.type == FILE_TYPE_REGULAR);

		/*
		printf("Dumping loader configuration:\n\n");

		uint8_t buffer[loaderPart->disk->blockSize];

		for (size_t i=0; i<fileInfo.size; i+=loaderPart->disk->blockSize) {
			int ret = loaderPart->fsDriver->readFileBlock(loaderPart, &fileInfo, buffer);
			if (ret != FS_SUCCESS)
				panic("Could not read bootloader config file");

			dump(buffer, MIN(fileInfo.size - i, loaderPart->disk->blockSize));
		}
		*/
	} else {
		printf(
			"error: Could not find bootloader file system with id %08x-%08x\n",
			(uint32_t)(loaderFsId >> 32),
			(uint32_t)loaderFsId
		);
		panic("No bootloader file system found");
	}

#if CONFIG_CONSOLE_SERIAL_IO
	printf("\nShutting down.\n");
	shutDown();
#else
	printf("\nHalting.\n");
	hang();
#endif /* CONFIG_CONSOLE_SERIAL_IO */
}
