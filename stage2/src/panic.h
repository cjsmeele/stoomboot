/**
 * \file
 * \brief     Panic function.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _PANIC_H
#define _PANIC_H

#include "common.h"

void panic(const char *msg) __attribute__((noreturn));

#endif /* _PANIC_H */
