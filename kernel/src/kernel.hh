/**
 * \file
 * \brief     Kernel startup code.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _KERNEL_HH
#define _KERNEL_HH

extern "C" void main(unsigned int loaderMagic, unsigned int bootInfoPtr) __attribute__((noreturn));

#endif /* _KERNEL_HH */
