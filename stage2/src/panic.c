/**
 * \file
 * \brief
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "panic.h"
#include "console.h"

void panic(const char *msg) {
	printf("\npanic: %s\n", msg ? msg : "Something went terribly wrong :(");
	hang();
}
