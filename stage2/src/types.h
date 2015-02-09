/**
 * \file
 * \brief     Typedefs used throughout stage2.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _TYPES_H
#define _TYPES_H

#include "common.h"

typedef unsigned char bool;
#define true  (1)
#define false (0)

// Using a 64-bit size_t would be overkill in real-mode.
typedef          int ssize_t;
typedef unsigned int size_t;

typedef char      int8_t;
typedef short     int16_t;
typedef int       int32_t;
typedef long long int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

#endif /* _TYPES_H */
