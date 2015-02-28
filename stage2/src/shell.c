/**
 * \file
 * \brief     The bootloader CLI.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "shell.h"
#include "console.h"
#include "command.h"
#include "commands.h"

void prompt(const char *query, char *buffer, size_t length) {
	puts(query);
	getLine(buffer, length);
}

void shell() {
	char  cmdLine[SHELL_INPUT_BUFFER_SIZE];
	char *argList[SHELL_MAX_ARG_COUNT];

	while (true) {
		memset(cmdLine, 0, ELEMS(cmdLine));
		prompt(SHELL_QUERY_STRING, cmdLine, ELEMS(cmdLine)-1);

		if (streq(cmdLine, "exit"))
			break;

		int argCount = splitCommandLine(argList, SHELL_MAX_ARG_COUNT, cmdLine);

		if (argCount) {
			Command *command = getCommand(argList[0]);
			if (command)
				command->function(argCount, argList, true);
			else
				printf("No such command '%s'\n", argList[0]);
		}
	}
}
