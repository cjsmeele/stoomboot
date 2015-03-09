/**
 * \file
 * \brief     Stage 2 C entrypoint.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "stage2.h"
#include "unreal.h"
#include "console.h"
#include "memmap.h"
#include "rcfile.h"
#include "config.h"
#include "shell.h"
#include "boot.h"
#include "dump.h"
#include "far.h"

extern uint32_t _BSS_START;
extern uint32_t _BSS_END;

Partition *loaderPart = NULL;

void stage2Main(uint32_t bootDiskNo, uint64_t loaderFsId) {

	// Nobody cleared this area for us.
	memset((void*)_BSS_START, 0, _BSS_END - _BSS_START);

	initConfig();
	initConsole();

	enableUnrealMode();

	uint16_t a = 0x0a01;
	farcpy(0x000b8000, (uint32_t)&a, sizeof(a));
	segcpy(0xb8000002, 0xb0008000,   sizeof(a)); // Using segment:offset.
	farcpy(0x000b8004, 0x000b8002,   sizeof(a)); // Using 32-bit addresses.

	printf("\n  Welcome to the Havik bootloader.\n\n");

	if (makeMemMap())
		panic("error: int 15h 0xe820 for memory mapping is not supported by your BIOS.");

	if (disksDiscover() <= 0)
		// We did not detect any usable disks, abort.
		panic("No usable disk drives detected.");

	loaderPart = getPartitionByFsId(loaderFsId);

	if (loaderPart) {
		if (loaderPart->disk->diskNo != (bootDiskNo & 0x7f))
			printf("warning: Loader filesystem is on a different disk than stage2\n");

		FileInfo fileInfo;
		memset(&fileInfo, 0, sizeof(FileInfo));
		int ret = loaderPart->fsDriver->getFile(
			loaderPart,
			&fileInfo,
			CONFIG_LOADER_CONFIG_PATH
		);
		if (ret == FS_FILE_NOT_FOUND || fileInfo.type != FILE_TYPE_REGULAR) {
			printf(
				"warning: Bootloader configuration file not found at hd%u:%u:%s\n",
				loaderPart->disk->diskNo,
				loaderPart->partitionNo,
				CONFIG_LOADER_CONFIG_PATH
			);
		} else if (ret == FS_SUCCESS) {
			parseRcFile(&fileInfo);
		}
	} else {
		printf(
			"warning: Could not find bootloader file system with id %08x-%08x\n",
			(uint32_t)(loaderFsId >> 32),
			(uint32_t)loaderFsId
		);
	}

	ConfigOption *kernelOption = getConfigOption("kernel");
	assert(kernelOption != NULL);
	ConfigOption *initrdOption = getConfigOption("initrd");
	assert(initrdOption != NULL);

	if (strlen(kernelOption->value.valStr)) {
		ConfigOption *timeoutOption = getConfigOption("timeout");
		assert(timeoutOption != NULL);
		if (timeoutOption->value.valInt32 >= 0) {

			bool aborted = false;
			for (int32_t i=timeoutOption->value.valInt32; i>=1 && !aborted; i--) {
				printf("\rBooting in %d seconds, press ESC to abort...    \b\b\b\b", i);
				Key key;
				for (uint32_t j=0; j<10; j++) {
					if (getKey(&key, false)) {
						if (key.scanCode == 1 || key.chr == '\e') {
							aborted = true;
							break;
						}
					}
					msleep(100);
				}
			}
			printf("\r%79s\r", "");

			if (!aborted) {
				printf("Booting `%s'\n", kernelOption->value.valStr);

				BootOption bootOption;
				memset(&bootOption, 0, sizeof(BootOption));

				if (!parseBootPathString(&bootOption.kernel, kernelOption->value.valStr)) {
					if (
							   !strlen(initrdOption->value.valStr)
							|| !parseBootPathString(&bootOption.initrd, initrdOption->value.valStr)
						) {
						boot(&bootOption);
					}
				}
			}
		}
	}

	shell();

#if CONFIG_CONSOLE_SERIAL_IO
	printf("\nShutting down.\n");
	shutDown();
#else
	printf("\nHalting.\n");
	hang();
#endif /* CONFIG_CONSOLE_SERIAL_IO */
}
