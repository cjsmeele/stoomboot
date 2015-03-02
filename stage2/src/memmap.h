/**
 * \file
 * \brief     Memory mapping.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _MEMMAP_H
#define _MEMMAP_H

#include "common.h"

#define MEMMAP_MAX_REGIONS 12

#define MEMORY_REGION_TYPE_FREE             1
#define MEMORY_REGION_TYPE_RESERVED         2
#define MEMORY_REGION_TYPE_ACPI_RECLAIMABLE 3
#define MEMORY_REGION_TYPE_ACPI_NVS         4
#define MEMORY_REGION_TYPE_BAD              5

typedef struct {
	uint64_t start;
	uint64_t length;
	uint32_t type;
} __attribute__((packed)) MemMapRegion;

typedef struct {
	MemMapRegion regions[MEMMAP_MAX_REGIONS];
} MemMap;

extern MemMap memMap;

/**
 * \brief Makes a memory map using BIOS calls.
 *
 * \return zero on success, non-zero if the int 15h 0xe820 BIOS call is unsupported.
 */
int makeMemMap();

#endif /* _MEMMAP_H */
