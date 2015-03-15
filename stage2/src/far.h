/**
 * \file
 * \brief     Memory operations using 32-bit pointers.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _FAR_H
#define _FAR_H

#include "common.h"

/**
 * \brief Copy data using 32-bit pointers.
 *
 * \param dest
 * \param src
 * \param length
 */
void farcpy(uint32_t dest, uint32_t src, uint32_t length);

/**
 * \brief Copy data across memory segments.
 *
 * Note that the format of the address parameters is `SEGMENT << 16 | OFFSET`.
 *
 * \param destSegOff
 * \param srcSegOff
 * \param length
 */
#define segcpy(destSegOff, srcSegOff, length) \
	farcpy( \
		(((destSegOff) >> 16) << 4) + ((destSegOff) & 0xffff), \
		(((srcSegOff)  >> 16) << 4) + ((srcSegOff)  & 0xffff), \
		(length) \
	)

/**
 * \brief Zeroes memory.
 *
 * \param dest
 * \param length
 */
void farzero(uint32_t dest, uint32_t length);

#endif /* _FAR_H */
