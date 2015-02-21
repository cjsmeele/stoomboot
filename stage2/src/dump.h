/**
 * \file
 * \brief     Hex dumper.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _DUMP_H
#define _DUMP_H

#include "common.h"

/**
 * \brief Print a hex dump of the given memory region.
 *
 * \param ptr
 * \param length
 */
void dump(void *ptr, size_t length);

#endif /* _DUMP_H */
