/**
 * \file
 * \brief     Multiboot compatibility.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "multiboot.h"
#include "memmap.h"
#include "disk/disk.h"
#include "vbe.h"
#include "config.h"

struct multiboot_info multibootInfo;

static const char *BOOTLOADER_NAME = "Stoomboot 0.1";

static VbeInfoBlock     vbeBootInfo;
static VbeModeInfoBlock vbeBootModeInfo;

void generateMultibootInfo(Partition *bootPartition) {

	memset(&multibootInfo, 0, sizeof(multibootInfo));

	multibootInfo.flags =
		  MULTIBOOT_INFO_BOOTDEV
		| MULTIBOOT_INFO_CMDLINE
		| MULTIBOOT_INFO_MEM_MAP
		| MULTIBOOT_INFO_BOOT_LOADER_NAME;

	multibootInfo.boot_device =
		  (uint32_t)bootPartition->disk->biosId << 24
		| (uint32_t)bootPartition->partitionNo  << 16
		| 0xffff; // "sub-partitions" are not supported.

	ConfigOption *cmdLine = getConfigOption("cmdline");
	multibootInfo.cmdline = (uint32_t)cmdLine->value.valStr;

	// Modules are not supported.
	multibootInfo.mods_count = 0;
	multibootInfo.mods_addr  = 0;

	// Not supported.
	//memset(&multibootInfo.u.elf_sec, 0, sizeof(multibootInfo.u.elf_sec));

	multibootInfo.mmap_length = memMap.regionCount * (sizeof(MemMapRegion) - sizeof(uint32_t));
	multibootInfo.mmap_addr   = (uint32_t)memMap.regions;

	multibootInfo.boot_loader_name = (uint32_t)BOOTLOADER_NAME;

	// Not supported.
	multibootInfo.apm_table = 0;

	memset(&vbeBootInfo,     0, sizeof(vbeBootInfo));
	memset(&vbeBootModeInfo, 0, sizeof(vbeBootModeInfo));

	vbeGetInfo(&vbeBootInfo);
	multibootInfo.vbe_control_info = (uint32_t)&vbeBootInfo;

	if (vbeCurrentMode == 0xffff) {
		multibootInfo.vbe_mode_info = 0;
	} else {
		vbeGetModeInfo(&vbeBootModeInfo, vbeCurrentMode);
		multibootInfo.vbe_mode_info = (uint32_t)&vbeBootModeInfo;
		multibootInfo.flags |= MULTIBOOT_INFO_VIDEO_INFO;
	}

	multibootInfo.vbe_mode = vbeCurrentMode;

	// VBE PMI is not supported.
	multibootInfo.vbe_interface_seg = 0;
	multibootInfo.vbe_interface_off = 0;
	multibootInfo.vbe_interface_len = 0;
}
