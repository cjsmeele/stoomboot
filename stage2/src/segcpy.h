/**
 * \file
 * \brief     Copy data across memory segments.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _SEGCPY_H
#define _SEGCPY_H

#include "common.h"

/**
 * \brief Copy data across memory segments.
 *
 * Note that the format of the address parameters is `SEGMENT << 16 | OFFSET`.
 *
 * \param destSegOff
 * \param srcSegOff
 * \param length
 */
void segcpy(uint32_t destSegOff, uint32_t srcSegOff, uint32_t length);

#endif /* _SEGCPY_H */
