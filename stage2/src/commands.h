/**
 * \file
 * \brief     Commands available to the shell and the config system.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "common.h"

#define CMD_FUNCTION(name) _command_##name
#define CMD_DECL(name) int CMD_FUNCTION(name)(int argc, char *argv[], bool interactive)
#define CMD_DEF(name)  CMD_DECL(name)

/**
 * \brief Defines a command.
 *
 * \param argc the argument count (includes the command name)
 * \param argv[] a list of parameters
 * \param interactive whether this command is being run in interactive (shell) mode
 *
 * \return zero on success, non-zero on failure
 */
typedef int (*CommandFunction)(int argc, char *argv[], bool interactive);

/**
 * \brief Command type.
 */
typedef struct {
	const char *name;
	const char *helpText; ///< Shown when 'help [name]' is run.
	CommandFunction function;
} Command;

extern Command commands[];

CMD_DECL(boot);
CMD_DECL(cls);
CMD_DECL(disk_info);
CMD_DECL(halt);
CMD_DECL(hello);
CMD_DECL(help);
CMD_DECL(hang);
CMD_DECL(set);
CMD_DECL(unset);

/**
 * \brief Get a command from commands[] by its name.
 *
 * \param name
 *
 * \return a command struct, or NULL if it wasn't found
 */
Command *getCommand(const char *name);

#endif /* _COMMANDS_H */
