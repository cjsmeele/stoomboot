/**
 * @file
 * @brief
 * @author    Chris Smeele
 * @copyright Copyright (c) 2015, Chris Smeele
 */
#ifndef _COMMON_H
#define _COMMON_H

#include "types.h"

#ifndef asm
#define asm __asm__
#endif

void puts(const char *str);
void putunum(uint32_t num);
void putnum(int32_t num);

void hang() __attribute__((noreturn));

#endif /* _COMMON_H */
