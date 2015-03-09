/**
 * \file
 * \brief     Enable unreal mode.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _UNREAL_H
#define _UNREAL_H

#include "common.h"

/**
 * \brief Enable unreal mode to allow 32-bit data address offsets.
 *
 * Also enables the A20 address line.
 */
void enableUnrealMode();

#endif /* _UNREAL_H */
