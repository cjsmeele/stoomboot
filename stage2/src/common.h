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

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(n, min, max) (MIN(MAX((n), (min)), (max)))
#define ELEMS(a) (sizeof(a) / sizeof((a)[0]))

#include "types.h"
#include "panic.h"
#include "assert.h"

/**
 * \brief Halt the CPU, wait for the next interrupt.
 */
void halt();

/**
 * \brief Hang forever.
 */
void hang() __attribute__((noreturn));

/**
 * \brief Use a BIOS interrupt to shut down the computer.
 */
void shutDown() __attribute__((noreturn));

/**
 * \brief Wait the specified amount of milliseconds.
 *
 * \param millis
 */
void msleep(uint32_t millis);

/**
 * \brief Prints the stack pointer for debugging.
 */
void printStackState();

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
 * \brief Check if the contents of two memory regions are equal.
 *
 * \param source1
 * \param source2
 * \param length
 *
 * \return whether the memory regions are equal
 */
bool memeq(const void *source1, const void *source2, size_t length);

/**
 * \brief Returns the length of the input string.
 *
 * \param str a null-terminated string
 *
 * \return
 */
size_t strlen(const char *str);

/**
 * \brief Find the first occurrence of a char in a string.
 *
 * \param str
 * \param ch
 *
 * \return a pointer to the location of ch in str, or a pointer to
 *         the terminating NULL byte if the character was not found
 */
char *strchr(const char *str, char ch);

/**
 * \brief Check if two strings are equal.
 *
 * \param str1
 * \param str2
 *
 * \return
 */
bool streq(const char *str1, const char *str2);

/**
 * \brief Check if the first `length` chars of the given strings are equal.
 *
 * \param str1
 * \param str2
 * \param length
 *
 * \return
 */
bool strneq(const char *str1, const char *str2, size_t length);

/**
 * \brief Copy a string.
 *
 * \param dest
 * \param src
 * \param length the maximum amount of bytes to copy
 *
 * \return a pointer to dest
 */
char *strncpy(char *dest, const char *src, size_t length);

/**
 * \brief Strip whitespace off the right side of a string.
 *
 * \param str
 */
void rtrim(char *str);

/**
 * \brief Convert a string to lower case.
 *
 * \param str
 */
void toLowerCase(char *str);

/**
 * \brief Convert a string to upper case.
 *
 * \param str
 */
void toUpperCase(char *str);

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
