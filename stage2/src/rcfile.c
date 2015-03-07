/**
 * \file
 * \brief     Run-Commands / configuration file support.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "rcfile.h"
#include "command.h"
#include "shell.h"
#include "console.h"

int parseRcFile(FileInfo *file) {
	assert(file->type == FILE_TYPE_REGULAR);

	Partition *part = file->partition;

	uint8_t buffer[part->disk->blockSize];
	char  cmdLine[SHELL_INPUT_BUFFER_SIZE] = { };
	char *argList[SHELL_MAX_ARG_COUNT]     = { };

	size_t cmdLineIndex = 0;
	bool   inComment = false;

	for (size_t i=0; i<file->size; i+=part->disk->blockSize) {
		int ret = part->fsDriver->readFileBlock(file, buffer);
		if (ret != FS_SUCCESS)
			panic("Could not read bootloader config file");

		for (size_t j=0; j<MIN(file->size - i, part->disk->blockSize); j++) {
			if (buffer[j] == '\n') {
				cmdLine[cmdLineIndex] = 0;
				if (strlen(cmdLine)) {
					int argCount = splitCommandLine(argList, ELEMS(argList), cmdLine);

					if (argCount) {
						Command *command = getCommand(argList[0]);
						if (command)
							command->function(argCount, argList, false);
						else
							printf("No such command in loader rc: '%s'\n", argList[0]);
					}
				}
				cmdLineIndex = 0;
				inComment = false;

			} else if (!inComment) {
				if (!cmdLineIndex && (buffer[j] == ' ' || buffer[j] == '\t')) {
					// Ignore indentation.
				} else if (!cmdLineIndex && buffer[j] == '#') {
					inComment = true;
				} else if (buffer[j] >= ' ' && buffer[j] <= '~') {
					cmdLine[cmdLineIndex++] = buffer[j];
					if (cmdLineIndex >= ELEMS(cmdLine)) {
						printf("warning: Too long loader rc line dropped\n");
						printf("(limit is %u chars)\n", ELEMS(cmdLine));
						cmdLineIndex = 0;
					}
				}
			}
		}
	}
	if (cmdLineIndex)
		// All non-empty text files must have an EOL char at the end.
		printf("warning: Unterminated final loader rc line dropped\n");

	return 0;
}
