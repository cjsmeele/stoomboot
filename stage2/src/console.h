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

#ifndef CONFIG_CONSOLE_SERIAL_IO
#define CONFIG_CONSOLE_SERIAL_IO 0
#endif /* CONFIG_CONSOLE_SERIAL_IO */

extern uint8_t consolePage;
extern uint8_t consoleWidth;
extern uint8_t consoleHeight;
extern uint8_t consoleAttribute; ///< (0) if not using colors.

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
bool getKey(Key *key, bool wait);

/**
 * \brief Get a single line of user input.
 *
 * \param line
 * \param size
 *
 * \return
 */
size_t getLine(char *line, size_t size);

/**
 * \brief Move the cursor.
 *
 * \param x
 * \param y
 */
void setCursorPosition(uint8_t x, uint8_t y);

/**
 * \brief Clear the text-mode screen.
 */
void cls();

/**
 * \brief Get the current page number.
 */
uint8_t getVideoPageNumber();

/**
 * \brief Set the video mode.
 *
 * \param mode
 */
void setVideoMode(uint8_t mode);

/**
 * \brief Set up the console.
 */
void initConsole();

#endif /* _CONSOLE_H */
