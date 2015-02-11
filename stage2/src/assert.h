/**
 * \file
 * \brief
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _ASSERT_H
#define _ASSERT_H

#include "panic.h"

#define _ASSERT_GLUE2(x) #x
#define _ASSERT_GLUE1(x) _ASSERT_GLUE2(x)

#define assert(x) do { \
		if (!(x)) { \
			panic("Assertion failed at " __FILE__ ":" _ASSERT_GLUE1(__LINE__) ": (" #x ")"); \
		} \
	} while(0)

#endif /* _ASSERT_H */
