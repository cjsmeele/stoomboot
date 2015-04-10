/**
 * \file
 * \brief     Functions used to boot the system.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _BOOT_H
#define _BOOT_H

#include "common.h"
#include "disk/disk.h"

typedef struct {
	Partition *partition;
	char *path;
} BootFilePath;

typedef struct {
	BootFilePath kernel;
	BootFilePath initrd;
} BootOption;

/**
 * \brief Try to boot the given boot option.
 *
 * If booting fails, this function will return if possible.
 */
void boot(BootOption *bootOption);

/**
 * \brief Parses a boot path string and fills a BootFilePath struct.
 *
 * The string should be of one of the following formats:
 *
 * - `hd0:0:/path/to/kernel.elf`
 * - `FSID=0123456789abcdef:/path/to/kernel.elf`
 * - `FSLABEL=STOOMLDR:/path/to/kernel.elf`
 * - `/path/on/loader/fs/to/kernel.elf`
 *
 * Note: str will be modified by this function.
 *
 * \param bootFile
 * \param str
 *
 * \return zero on success, non-zero on failure
 */
int parseBootPathString(BootFilePath *bootFile, char *str);

#endif /* _BOOT_H */
