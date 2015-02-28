/**
 * \file
 * \brief     Commands available to the shell and the config system.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "commands.h"
#include "console.h"

#define CMD_INCLUDE(name, helpText) { \
		#name, \
		helpText, \
		CMD_FUNCTION(name) \
	}

Command commands[] = {
	CMD_INCLUDE(
		halt,
 "usage: halt\
\nShuts down the computer.\
\n"
	),
	CMD_INCLUDE(
		hello,
 "usage: hello\
\nPrints 'Hello, world!'.\
\n"
	),
	CMD_INCLUDE(
		help,
 "usage: help [FUNCTION]\
\nShows a list of commands, or, when a function name is given, the help text\
\nassociated with the given function name.\
\n"
	),
	CMD_INCLUDE(
		hang,
 "usage: hang\
\nTurns off interrupts and halts the processor.\
\n"
	),
};

#undef CMD_INCLUDE

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

CMD_DEF(halt) {
	if (!interactive)
		return 1;

	shutDown();
}

CMD_DEF(hello) {
	if (!interactive)
		return 1;

	printf("Hello, world!\n");

	return 0;
}

CMD_DEF(help) {
	if (!interactive)
		return 1;

	if (argc == 1) {
		for (size_t i=0; i<ELEMS(commands); i+=5) {
			for (size_t j=0; j<MIN(ELEMS(commands) - i, 5); j++)
				printf("%15s", commands[i+j].name);
			putch('\n');
		}
		return 0;

	} else if (argc == 2) {
		Command *command = getCommand(argv[1]);
		if (command) {
			puts(command->helpText);
			return 0;
		} else {
			printf("No such command '%s'\n", argv[1]);
			return 1;
		}
	} else {
		printf("usage: %s [FUNCTION]\n", argv[0]);
		return 1;
	}
}

CMD_DEF(hang) {
	if (!interactive)
		return 1;

	hang();
}

#pragma GCC diagnostic pop

Command *getCommand(const char *name) {
	for (size_t i=0; i<ELEMS(commands); i++) {
		if (streq(commands[i].name, name))
			return &commands[i];
	}

	return NULL;
}
