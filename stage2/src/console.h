/**
 * @file
 * @brief
 * @author    Chris Smeele
 * @copyright Copyright (c) 2015, Chris Smeele
 */
#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "common.h"

typedef struct {
	char chr;
	uint8_t scanCode;
} Key;

// Functions to be replaced with printf() later on.
void putch(char ch);
void puts(const char *str);
void putunum(uint32_t num);
void putnum(int32_t num);

bool     getKey(Key *key, bool wait);
//uint16_t XgetKey(bool wait, bool echo);
uint16_t getLine(char *line, uint16_t size);

#endif /* _CONSOLE_H */
