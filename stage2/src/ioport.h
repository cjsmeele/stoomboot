/**
 * @file
 * @brief
 * @author    Chris Smeele
 * @copyright Copyright (c) 2015, Chris Smeele
 */
#ifndef _IOPORT_H
#define _IOPORT_H

#include "common.h"

void outb(uint16_t port, uint8_t  value);
void outw(uint16_t port, uint16_t value);
uint8_t  inb(uint16_t port);
uint16_t inw(uint16_t port);

void iowait();

#endif /* _IOPORT_H */
