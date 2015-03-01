/**
 * \file
 * \brief     Console I/O functions.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "console.h"
#include <stdarg.h>

uint8_t consolePage      = 0;
uint8_t consoleWidth     = 80;
uint8_t consoleHeight    = 25;
uint8_t consoleAttribute = 0x00;

#if CONFIG_CONSOLE_SERIAL_IO
#include "bda.h"

static bool serialDeviceInitialized = false;

static void initCom0() {
	asm volatile (
		"int $0x14"
		:
		: "a" ((uint16_t)0b11100011),
		  "d" (0)
		: "cc"
	);
	serialDeviceInitialized = true;
}

static inline bool isSerialIoAvailable() {
	return bda->com0IoPort != 0;
}

#endif /* CONFIG_CONSOLE_SERIAL_IO */

static void doPutch(char ch) {
#if CONFIG_CONSOLE_SERIAL_IO
	if (isSerialIoAvailable()) {
		if (!serialDeviceInitialized)
			initCom0();

		asm volatile (
			"int $0x14"
			:
			: "a" (0x01 << 8 | ch),
			  "d" (0)
			: "cc"
		);
	} else {
#endif /* CONFIG_CONSOLE_SERIAL_IO */
	asm volatile (
		"int $0x10"
		:
		: "a" (0x0e << 8 | ch),
		  "b" (consolePage << 8 | (
			  consoleAttribute
			? consoleAttribute // Colors don't actually work in text-mode with this int call.
			: 0x07
		  ))
		: "cc"
	);
#if CONFIG_CONSOLE_SERIAL_IO
	}
#endif /* CONFIG_CONSOLE_SERIAL_IO */
}

void putch(char ch) {
	if (ch == '\n')
		doPutch('\r');
	doPutch(ch);
}

void puts(const char *str) {
	while (*str)
		putch(*str++);
}

#define CONSOLE_PRINTF_RESET_FORMAT() do { \
		inFormat = false; \
		memset(&flags, 0, sizeof(flags)); \
		width    = 0; \
		widthBufferIndex = 0; \
		lengthModifier   = 0; \
	} while (0)

typedef struct {
	bool alternative     : 1;
	bool uppercaseHex    : 1;
	bool leftAdjusted    : 1;
	bool padWithZeroes   : 1;
	bool groupDigits     : 1;
	bool alwaysPrintSign : 1;
	bool padSign         : 1;
} PrintfFlags;

static size_t printfDecimal(uint32_t num, bool sign, PrintfFlags *flags, size_t width) {
	size_t length = 0;

	if (sign) {
		if ((int32_t)num < 0) {
			putch('-');
			length++;
		} else if (flags->alwaysPrintSign) {
			putch('+');
			length++;
		} else if (flags->padSign) {
			putch(' ');
			length++;
		}

		num = abs((int32_t)num);
	}

	static char buffer[14] = { };
	size_t i = 13;

	do {
		if (flags->groupDigits && (i == 10 || i == 6 || i == 2))
			buffer[--i] = CONSOLE_PRINTF_DECIMAL_DIGIT_GROUP_CHAR;

		buffer[--i] = (num % 10) + '0';
		num /= 10;
	} while (num);

	length += 13 - i;

	if (width && width > length) {
		size_t j = 0;
		if (flags->leftAdjusted) {
			puts(&buffer[i]);

			for (; j<(width - length); j++)
				putch(' ');

		} else {
			for (; j<(width - length); j++)
				putch(flags->padWithZeroes ? '0' : ' ');

			puts(&buffer[i]);
		}

		length += j;
	} else {
		puts(&buffer[i]);
	}

	return length;
}

static size_t printfHex(uint32_t num, PrintfFlags *flags, size_t width) {
	size_t length = 0;

	static char buffer[20] = { };
	size_t i = 19;

	bool groupCharAdded = false;

	do {
		if (flags->groupDigits && i == 15) {
			buffer[--i] = CONSOLE_PRINTF_HEX_DIGIT_GROUP_CHAR;
			groupCharAdded = true;
		}

		uint8_t n = num & 0xf;
		buffer[--i] = n > 9 ? n - 10 + 'a' : n + '0';
		num >>= 4;
	} while (num);

	length += 19 - i - (groupCharAdded ? 1 : 0);

	if (flags->alternative)
		puts("0x");

	if (width && width > length) {
		size_t j = 0;
		if (flags->leftAdjusted) {
			puts(&buffer[i]);

			for (; j<(width - length); j++)
				putch(' ');

		} else {
			for (; j<(width - length); j++) {
				putch(flags->padWithZeroes ? '0' : ' ');
				if (flags->padWithZeroes && flags->groupDigits && width == 8 && j == 3) {
					putch(CONSOLE_PRINTF_HEX_DIGIT_GROUP_CHAR);
					groupCharAdded = true;
				}
			}

			if (groupCharAdded)
				length++;

			puts(&buffer[i]);
		}

		length += j;

	} else {
		puts(&buffer[i]);
	}

	if (flags->alternative)
		length += 2;

	return length;
}

int printf(const char *format, ...) {

	va_list vaList;
	va_start(vaList, format);
	// Format state {

	PrintfFlags flags;
	memset(&flags, 0, sizeof(flags));

	bool inFormat = false;
	size_t width  = 0; ///< Minimum formatted text length.
	static char widthBuffer[9] = { };
	size_t widthBufferIndex    = 0;
	uint8_t lengthModifier     = 0; ///< Amount of bytes as the size of the argument.

	// }

	int length = 0;

	char c;
	int i = 0;
	while ((c = format[i++])) {
		if (inFormat) {
			if (c == '%') {
				// Note: '%' can be used anywhere in a '%' format substring to cancel formatting.
				putch(c);
				CONSOLE_PRINTF_RESET_FORMAT();

			} else if ((widthBufferIndex && c == '0') || (c >= '1' && c <= '9')) {
				if (widthBufferIndex >= sizeof(widthBufferIndex)) {
					// Ignore silently.
				} else {
					widthBuffer[widthBufferIndex++] = c;
				}
			} else {
				if (widthBufferIndex) {
					widthBuffer[widthBufferIndex] = 0;
					widthBufferIndex = 0;
					width = atoi(widthBuffer);
				}
				// Flags {
				if (c == '-') {
					flags.leftAdjusted  = true;
					flags.padWithZeroes = false;
				} else if (c == '0') {
					flags.padWithZeroes = true;
					flags.leftAdjusted  = false;
				} else if (c == '#') {
					flags.alternative = true;
				} else if (c == '+') {
					flags.alwaysPrintSign = true;
					flags.padSign         = false;
				} else if (c == ' ') {
					flags.alwaysPrintSign = false;
					flags.padSign         = true;
				} else if (c == '\'') {
					flags.groupDigits = true;
				// }
				// Length modifiers {
				} else if (c == 'h') {
					lengthModifier = 2;
				} else if (c == 'H') {
					lengthModifier = 1;
				} else if (c == 'l') {
					lengthModifier = 8;
				// }
				// Conversion specifiers {
				} else {
					bool isConversion = false;
					static const char *conversionChars = "xdcups";
					for (size_t j=0; j<strlen(conversionChars); j++)
						if (c == conversionChars[j]) {
							isConversion = true;
							break;
						}

					if (!isConversion) {
						// Format error.
						CONSOLE_PRINTF_RESET_FORMAT();
						continue;
					}

					if (c == 'd') {
						int32_t num;
						if (lengthModifier == 1)
							num = (int8_t) va_arg(vaList, int);
						else if (lengthModifier == 2)
							num = (int16_t)va_arg(vaList, int);
						else
							num = (int32_t)va_arg(vaList, int);

						// Unfortunately, we cannot div / mod 64 bit numbers.
						length += printfDecimal(num, true, &flags, width);

					} else if (c == 'u' || c == 'x') {
						uint32_t num;
						if (lengthModifier == 1)
							num = (uint8_t) va_arg(vaList, unsigned int);
						else if (lengthModifier == 2)
							num = (uint16_t)va_arg(vaList, unsigned int);
						else
							num = (uint32_t)va_arg(vaList, unsigned int);

						if (c == 'u')
							length += printfDecimal(num, false, &flags, width);
						else if (c == 'x')
							length += printfHex(num, &flags, width);

					} else if (c == 's') {
						char *str = (char*)va_arg(vaList, char*);
						size_t slen = strlen(str);
						if (slen < width) {
							if (flags.leftAdjusted) {
								for (size_t j=0; j<(width - slen); j++)
									putch(' ');
								puts(str);
							} else {
								puts(str);
								for (size_t j=0; j<(width - slen); j++)
									putch(' ');
							}
							length += width;
						} else {
							puts(str);
							length += slen;
						}
					} else if (c == 'c') {
						char ch = (char)va_arg(vaList, int);
						putch(ch);
						length++;
					}

					CONSOLE_PRINTF_RESET_FORMAT();
				}
				// }
			}
		} else if (c == '%') {
			inFormat = true;
		} else if (c == '\n') {
			putch('\r');
			putch('\n');
			length++;
		} else {
			putch(c);
			length++;
		}
	}

	va_end(vaList);

	return length;
}

#undef CONSOLE_PRINTF_RESET_FORMAT

bool getKey(Key *key, bool wait) {

#if CONFIG_CONSOLE_SERIAL_IO
	if (isSerialIoAvailable()) {
		if (!serialDeviceInitialized)
			initCom0();

		// Check if input is available before trying to read from the serial port.
		do {
			uint16_t status = 0x0300;

			// Get serial port status.
			asm volatile (
				"int $0x14"
				: "+a" (status)
				: "d" (0)
				: "cc"
			);

			if (status & 1 << 8) {
				// Bit 8 is 1: Data available for reading.
				break;
			} else {
				if (!wait)
					return false;
				halt();
			}
		} while (true);

		uint16_t ret = 0x0200;

		asm volatile (
			"int $0x14"
			: "+a" (ret)
			: "d"  (0)
			: "cc"
		);

		key->chr      = ret & 0xff;
		key->scanCode = 0; // Not applicable to serial I/O.

	} else {
#endif /* CONFIG_CONSOLE_SERIAL_IO */

	// Check if input is available before trying to read from the keyboard buffer.
	do {
		uint16_t ready = 0;
		asm volatile (
			"int $0x16\n"
			"jz _not_ready\n"
			"_ready:\n"
			"  inc %0\n"
			"_not_ready:"
			: "+r" (ready)
			: "a" (1 << 8)
			: "cc"
		);

		if (ready) {
			break;
		} else {
			if (!wait)
				return false;
			halt();
		}
	} while (true);

	uint16_t keyInfo = 0;
	asm volatile (
		"int $0x16"
		: "+a" (keyInfo)
		:
		: "cc"
	);

	key->chr      = keyInfo & 0xff;
	key->scanCode = keyInfo >> 8;

#if CONFIG_CONSOLE_SERIAL_IO
	}
#endif /* CONFIG_CONSOLE_SERIAL_IO */

	return true;
}

size_t getLine(char *line, size_t size) {
	size_t i = 0;
	Key key;
	while (i < size - 1) {
		if (getKey(&key, true)) {
			if (key.chr >= ' ' && key.chr <= '~') {
				putch(key.chr);
				line[i++] = key.chr;
			} else if (key.chr == '\r') {
				puts("\r\n");
				break;
			} else if (key.chr == '\e') {
				line[0] = 0;
				puts("\r\n");
				break;
			} else if (key.chr == '\b' || key.chr == '\x7f') {
				if (i > 0) {
					putch('\b');
					putch(' ');
					putch('\b');
					i--;
					continue;
				}
			} else {
				// Ignore other non-printing characters.
			}
		}
	}

	line[i] = 0;

	return i;
}

void setCursorPosition(uint8_t x, uint8_t y) {
#if CONFIG_CONSOLE_SERIAL_IO
	if (isSerialIoAvailable()) {
		printf("\e[%u;%uf", y, x);
	} else {
#endif /* CONFIG_CONSOLE_SERIAL_IO */

	asm volatile (
		"int $0x10"
		:
		: "a" (0x0200),
		  "b" (consolePage << 8),
		  "d" (MIN(y, consoleHeight-1) << 8 | MIN(x, consoleWidth-1))
		: "cc"
	);

#if CONFIG_CONSOLE_SERIAL_IO
	}
#endif /* CONFIG_CONSOLE_SERIAL_IO */
}

void cls() {
#if CONFIG_CONSOLE_SERIAL_IO
	if (isSerialIoAvailable()) {
		// This should clear most terminals.
		puts("\e[2J");
	} else {
#endif /* CONFIG_CONSOLE_SERIAL_IO */

	asm volatile (
		"int $0x10"
		:
		: "a" (0x0600),
		  "b" (consoleAttribute << 8), // Attribute.
		  "c" (0x0000),
		  "d" (25 << 8 | 80)
		: "cc"
	);

#if CONFIG_CONSOLE_SERIAL_IO
	}
#endif /* CONFIG_CONSOLE_SERIAL_IO */

	setCursorPosition(0, 0);
}

uint8_t getVideoPageNumber() {
	uint16_t pageNumber;

	asm volatile (
		"int $0x10"
		: "=b" (pageNumber)
		: "a"  (0x0f00)
		: "cc"
	);

	return pageNumber >> 8;
}

void setVideoMode(uint8_t mode) {
	asm volatile (
		"int $0x10"
		:
		: "a" (mode & 0x07)
		: "cc"
	);
}

void initConsole() {
	setVideoMode(3); // 80x25, with colors.
	consolePage      = getVideoPageNumber();
	consoleWidth     = 80;
	consoleHeight    = 25;
	consoleAttribute = 0x07;
	cls();
}
