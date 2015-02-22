/**
 * \file
 * \brief     Stage 2 C entrypoint.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _STAGE2_H
#define _STAGE2_H

#include "common.h"

#ifndef CONFIG_LOADER_CONFIG_PATH
#define CONFIG_LOADER_CONFIG_PATH "/boot/loader/boot.cfg"
#endif /* CONFIG_LOADER_CONFIG_PATH */

/**
 * \brief Stage 2 C entrypoint.
 *
 * \param bootDiskNo the BIOS boot disk number (usually 80h)
 * \param loaderFsId the bootloader's filesystem UUID
 */
void stage2Main(uint32_t bootDiskNo, uint64_t loaderFsId) __attribute__((noreturn));

#endif /* _STAGE2_H */
