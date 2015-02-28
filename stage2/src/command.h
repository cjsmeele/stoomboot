/**
 * \file
 * \brief     Functions related to command lines.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _COMMAND_H
#define _COMMAND_H

#include "common.h"
#include "commands.h"

/**
 * \brief Parse a command line.
 *
 * Note that the cmdLine parameter will be modified by this function.
 *
 * \param argList
 * \param maxArgs
 * \param cmdLine the command line string
 *
 * \return the amount of arguments detected (includes the command name itself)
 */
int splitCommandLine(char *argList[], int maxArgs, char *cmdLine);

#endif /* _COMMAND_H */
