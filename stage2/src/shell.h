/**
 * \file
 * \brief     The bootloader CLI.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _SHELL_H
#define _SHELL_H

#include "common.h"

#define SHELL_INPUT_BUFFER_SIZE 256
#define SHELL_QUERY_STRING "havik-loader> "
#define SHELL_MAX_ARG_COUNT 16

/**
 * \brief Prompt the user for input.
 *
 * \param query a string printed before user input
 * \param buffer where the input is stored
 * \param length the maximum input length
 */
void prompt(const char *query, char *buffer, size_t length);

/**
 * \brief Starts the shell.
 */
void shell();

#endif /* _SHELL_H */
