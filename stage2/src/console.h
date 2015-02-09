/**
 * \file
 * \brief     Console I/O functions.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "common.h"

#define CONSOLE_PRINTF_DECIMAL_DIGIT_GROUP_CHAR '\''
#define CONSOLE_PRINTF_HEX_DIGIT_GROUP_CHAR     '.'

/**
 * \brief A keyboard key.
 */
typedef struct {
	char chr; ///< The character associated with a keyboard key.
	uint8_t scanCode;
} Key;

/**
 * \brief Prints a single character.
 *
 * Replaces `\n` with `\r\n`.
 *
 * \param ch
 */
void putch(char ch);

/**
 * \brief Prints a string.
 *
 * \param str a null-terminated string
 *
 * \see putch
 */
void puts(const char *str);

/**
 * \brief Print a formatted string.
 *
 * Tries to conform to the standard, but does not implement some features.
 * Feeding it a bogus combination of formatting options may produce unexpected
 * results.
 *
 * Supported flags are as follows:
 *
 * - `-`: Format left-adjusted text (pad the right side).
 * - `0`: Pad with zeroes instead of spaces.
 * - `#`: (\%x only) Alternative format (prefix hex strings with `0x`).
 * - `+`: (\%d, \%u only) Always print a sign char.
 * - ` ` (space): (\%d, \%u only) Leave a space in front of positive numbers.
 * - `'`: (\%d, \%u, \%x only) Group digits. Adds thousands separators according to
 *        CONSOLE_PRINTF_DECIMAL_DIGIT_GROUP_CHAR or
 *        CONSOLE_PRINTF_HEX_DIGIT_GROUP_CHAR.
 *        Note that "thousands separators" for hex strings are only added when
 *        the field with is set to 8.
 *
 * Supported length modifiers are as follows:
 *
 * - `H`: (\%d, \%u, \%x only) Parameter is 8 bits wide.
 * - `h`: (\%d, \%u, \%x only) Parameter is 16 bits wide.
 * - `l`: (unsupported) Parameter is 64 bits wide.
 *
 * The default parameter length is 32 bits.
 *
 * Supported conversion specifiers are as follows:
 *
 * - `d`: Signed decimal numbers
 * - `u`: Unsigned decimal numbers
 * - `x`: Unsigned hexadecimal numbers
 * - `s`: Strings
 * - `c`: Single characters
 * - `%`: Resets format options, prints a `%`
 *
 * See also printf(3).
 *
 * \param format a format string, see printf(3)
 * \param ... format parameters
 *
 * \return the length of the printed string
 */
int printf(const char *format, ...) __attribute__((format(printf,1,2)));

/**
 * \brief Get a keypress from user input.
 *
 * \param key
 * \param wait whether to block and wait for a keypress, if false, returns false when no key is available
 *
 * \return true if a key was read and stored in the key parameter
 */
bool     getKey(Key *key, bool wait);
uint16_t getLine(char *line, size_t size);

#endif /* _CONSOLE_H */
