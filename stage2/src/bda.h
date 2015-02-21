/**
 * \file
 * \brief     BIOS Data Area.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _BDA_H
#define _BDA_H

#include "common.h"

/**
 * \brief BIOS Data Area structure.
 */
typedef struct {

	uint16_t com0IoPort;
	uint16_t com1IoPort;
	uint16_t com2IoPort;
	uint16_t com3IoPort;

	uint8_t _stuff[0x6d]; ///< @todo Describe other BDA fields.

	uint8_t hdCount; ///< Amount of installed hard disks.

} __attribute__((packed)) BDA;

extern const BDA *const bda;

#endif /* _BDA_H */
