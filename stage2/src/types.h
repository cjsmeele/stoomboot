/**
 * @file
 * @brief
 * @author    Chris Smeele
 * @copyright Copyright (c) 2015, Chris Smeele
 */
#ifndef _TYPES_H
#define _TYPES_H

#include "common.h"

#ifndef bool
typedef unsigned char bool;
#define true  (1)
#define false (0)
#endif

typedef char  int8_t;
typedef short int16_t;
typedef int   int32_t;

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

// We cannot use 64 integers in real mode.

#endif /* _TYPES_H */
