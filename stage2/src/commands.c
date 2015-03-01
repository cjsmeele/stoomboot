/**
 * \file
 * \brief     Commands available to the shell and the config system.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "commands.h"
#include "console.h"
#include "config.h"

#define CMD_INCLUDE(name, helpText) { \
		#name, \
		helpText, \
		CMD_FUNCTION(name) \
	}

Command commands[] = {
	CMD_INCLUDE(
		cls,
 "usage: cls\
\nClears the screen.\
\n"
	),
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
	CMD_INCLUDE(
		set,
 "usage: set [OPTION [VALUE]]\
\nSets a configuration option, or shows option values.\
\n"
	),
	CMD_INCLUDE(
		unset,
 "usage: unset OPTION\
\nClears a configuration option (sets it to 0 or '').\
\n"
	),
};

#undef CMD_INCLUDE

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

CMD_DEF(cls) {
	if (!interactive)
		return 1;

	cls();

	return 0;
}

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
		printf("The following commands are supported by the bootloader:\n");
		printf("Type `help [FUNCTION]' for information on that function.\n");
		printf("Spaces in parameters must be escaped with '\\' (quoting is not allowed).\n\n");

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

static void printConfigOption(ConfigOption *option) {
	printf("%s = ", option->key);

	switch (option->type) {
	case CONFIG_OPTION_TYPE_UINT64:
		printf(
			"%#08x.%08x",
			(uint32_t)(option->value.valUInt64 >> 32),
			(uint32_t)option->value.valUInt64
		);
		break;
	case CONFIG_OPTION_TYPE_INT32:
		printf("%d", option->value.valInt32);
		break;
	case CONFIG_OPTION_TYPE_UINT32:
		printf("%u", option->value.valUInt32);
		break;
	case CONFIG_OPTION_TYPE_STRING:
		printf("'%s'", option->value.valStr);
		break;
	}

	putch('\n');
}

CMD_DEF(set) {
	if (argc == 1) {
		for (size_t i=0; i<configOptionCount; i++)
			printConfigOption(&configOptions[i]);
		return 0;

	} else if (argc == 2) {
		ConfigOption *option = getConfigOption(argv[1]);
		if (option) {
			printConfigOption(option);
			return 0;
		} else {
			printf("No such option '%s'\n", argv[1]);
			return 1;
		}
	} else if (argc == 3) {
		ConfigOption *option = getConfigOption(argv[1]);
		if (option) {
			ConfigOptionValue value;
			switch (option->type) {
			case CONFIG_OPTION_TYPE_UINT64:
				/// @todo FIXME: 64 bit numbers are truncated.
				value.valUInt64 = atoi(argv[2]);
				break;
			case CONFIG_OPTION_TYPE_INT32:
				value.valInt32 = atoi(argv[2]);
				break;
			case CONFIG_OPTION_TYPE_UINT32:
				value.valUInt32 = atoi(argv[2]);
				break;
			case CONFIG_OPTION_TYPE_STRING:
				value.valStr = argv[2];
				break;
			}
			setConfigOption(option->key, value);
			return 0;
		} else {
			printf("No such option '%s'\n", argv[1]);
			return 1;
		}
	} else if (interactive) {
		printf("usage: set [OPTION [VALUE]]\n");
		return 1;

	} else {
		printf("warning: Invalid set command ignored\n");
		return 1;
	}
}

CMD_DEF(unset) {
	if (argc == 2) {
		ConfigOption *option = getConfigOption(argv[1]);
		if (option) {
			ConfigOptionValue value;
			memset(&value, 0, sizeof(ConfigOptionValue));
			setConfigOption(option->key, value);
			return 0;
		} else {
			printf("No such option '%s'\n", argv[1]);
			return 1;
		}
	} else if (interactive) {
		printf("usage: unset [OPTION]\n");
		return 1;
	} else {
		printf("warning: Invalid unset command ignored\n");
		return 1;
	}
}

#pragma GCC diagnostic pop

Command *getCommand(const char *name) {
	for (size_t i=0; i<ELEMS(commands); i++) {
		if (streq(commands[i].name, name))
			return &commands[i];
	}

	return NULL;
}
