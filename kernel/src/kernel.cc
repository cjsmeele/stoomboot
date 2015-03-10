/**
 * \file
 * \brief     Kernel startup code.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "kernel.hh"

static const char splash[] =
"                                                                                "
"                                                                                "
"                                                                                "
"                                                                                "
"                                                                                "
"                                                                                "
"                           __  __ ___  _    __ ____ __ __                       "
"                          / / / //   || |  / //  _// //_/  tm                   "
"                         / /_/ // /| || | / / / / / ,<                          "
"                        / __  // ___ || |/ /_/ / / /| |                         "
"                       /_/ /_//_/  |_||___//___//_/ |_|                         "
"                                                                                "
"                                                                                "
"                                                                                "
"                       Protected mode kernel says hello!                        "
"                                                                                "
"                                                                                "
"                                                                                "
"                                                                                "
"                                                                                "
"                                                                                "
"                                                                                "
"                                                                                "
"                                                                                "
"                                                                                ";

extern "C" void main(unsigned int loaderMagic, unsigned int bootInfoPtr) {

	short *vram = (short*)0xb8000;

	for (int i=0; i<80*25; i++)
		vram[i] = 0x00 << 8 | ' ';

	for (unsigned int i=0; i<sizeof(splash); i++)
		vram[i] = (short)((splash[i] == ' ' ? 0x99 : 0x9f) << 8 | splash[i]);

	for (unsigned int i=0; i<50; i++)
		vram[12*80 + 15 + i] = (short)0x9fc4;

	vram[ 1*80 +  1] = (short)0x9f01;
	vram[ 1*80 + 78] = (short)0x9f01;
	vram[23*80 +  1] = (short)0x9f01;
	vram[23*80 + 78] = (short)0x9f01;

	for (;;)
		asm volatile (
			"cli\n"
			"hlt\n"
		);
}
