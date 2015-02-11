/**
 * \file
 * \brief     Commonly used system, memory, string and math functions.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _COMMON_H
#define _COMMON_H

#ifndef asm
#define asm __asm__
#endif

#include "types.h"
#include "panic.h"
#include "assert.h"

/**
 * \brief Halt the CPU, wait for the next interrupt.
 */
void halt();

/**
 * \brief Hang forever
 */
void hang() __attribute__((noreturn));

/**
 * \brief Set length bytes starting at mem to c.
 *
 * \param mem
 * \param c
 * \param length
 *
 * \return a pointer to mem
 */
void *memset(void *mem, uint8_t c, size_t length);

/**
 * \brief Copy length bytes from source to dest.
 *
 * \param dest
 * \param source
 * \param length
 *
 * \return a pointer to dest
 */
void *memcpy(void *dest, const void *source, size_t length);

/**
 * \brief Returns the length of the input string.
 *
 * \param str a null-terminated string
 *
 * \return
 */
size_t strlen(const char *str);

/**
 * \brief Extract an integer from a string.
 *
 * \param buf a null-terminated string containing only a decimal numbers, and optonally a '-' sign
 *
 * \return
 */
int atoi(const char *buf);

/**
 * \brief Get the absolute value of an integer.
 *
 * \param num
 *
 * \return
 */
int abs(int num);

#endif /* _COMMON_H */
