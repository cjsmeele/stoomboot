/**
 * \file
 * \brief     I/O port functions.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _IOPORT_H
#define _IOPORT_H

#include "common.h"

/**
 * \brief Write a byte to an I/O port.
 *
 * \param port
 * \param value
 */
void outb(uint16_t port, uint8_t value);

/**
 * \brief Write a word to an I/O port.
 *
 * \param port
 * \param value
 */
void outw(uint16_t port, uint16_t value);

/**
 * \brief Read a byte from an I/O port.
 *
 * \param port
 *
 * \return
 */
uint8_t inb(uint16_t port);

/**
 * \brief Read a word from an I/O port.
 *
 * \param port
 *
 * \return
 */
uint16_t inw(uint16_t port);

/**
 * \brief Wait for an I/O port operation to complete.
 */
void iowait();

#endif /* _IOPORT_H */
