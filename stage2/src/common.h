/**
 * @file
 * @brief
 * @author    Chris Smeele
 * @copyright Copyright (c) 2015, Chris Smeele
 */
#ifndef _COMMON_H
#define _COMMON_H

#ifndef asm
#define asm __asm__
#endif

#include "types.h"

void halt();
void hang() __attribute__((noreturn));

#endif /* _COMMON_H */
