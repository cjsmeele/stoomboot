/**
 * \file
 * \brief     Functions related to command lines.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "command.h"

/**
 * \brief Get the position of the first separator character in cmdLine.
 *
 * \param cmdLine
 *
 * \return
 */
static size_t getNextSplit(char *cmdLine) {
	bool escaped = false;
	size_t slen = strlen(cmdLine);

	size_t i;
	for (i=0; i<slen; i++) {
		if (cmdLine[i] == '\0') {
			return i;
		} else if (
				!escaped && (
					   cmdLine[i] == ' '
					|| cmdLine[i] == '\t'
					|| cmdLine[i] == '\v'
					|| cmdLine[i] == '\n'
				)
			) {
			return i;
		} else if (escaped) {
			strncpy(cmdLine + i - 1, cmdLine + i, strlen(cmdLine));
			escaped = false;
			i--;
		} else if (cmdLine[i] == '\\') {
			escaped = true;
		}
	}

	return i;
}

int splitCommandLine(char *argList[], int maxArgs, char *cmdLine) {
	int argCount = 0;
	size_t offset = 0;

	while (cmdLine[offset] && argCount < maxArgs) {
		// Skip to the next non-separator character.
		size_t nextSplit = 0;
		while (cmdLine[offset] && (nextSplit = getNextSplit(cmdLine + offset)) == 0)
			cmdLine[offset++] = '\0';

		if (nextSplit) {
			argList[argCount++] = cmdLine + offset;
			offset += nextSplit;
		}
	}

	return argCount;
}
