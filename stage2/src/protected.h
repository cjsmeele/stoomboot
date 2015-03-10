/**
 * \file
 * \brief     Switch to protected mode.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _PROTECTED_H
#define _PROTECTED_H

#include "common.h"

/**
 * \brief Enable protected mode and jump to a 32-bit entrypoint.
 */
void enterProtectedMode(uint32_t entrypoint) __attribute__((noreturn));

#endif /* _PROTECTED_H */
