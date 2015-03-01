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
#include "disk/disk.h"
#include "boot.h"

#define CMD_INCLUDE(name, helpText) { \
		#name, \
		helpText, \
		CMD_FUNCTION(name) \
	}

Command commands[] = {
	CMD_INCLUDE(
		boot,
 "usage: boot\
\nExecutes the selected boot option.\
\n"
	),
	CMD_INCLUDE(
		cls,
 "usage: cls\
\nClears the screen.\
\n"
	),
	{
		"disk-info",
 "usage: disk-info [DISK_NO]\
\nPrints information about the given disk number.\
\n",
		CMD_FUNCTION(disk_info)
	},
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

CMD_DEF(boot) {
	ConfigOption *kernelOption = getConfigOption("kernel");
	assert(kernelOption != NULL && kernelOption->type == CONFIG_OPTION_TYPE_STRING);
	ConfigOption *initrdOption = getConfigOption("initrd");
	assert(initrdOption != NULL && initrdOption->type == CONFIG_OPTION_TYPE_STRING);

	if (strlen(kernelOption->value.valStr)) {
		BootOption bootOption;
		memset(&bootOption, 0, sizeof(BootOption));
		bool haveInitrd = false;

		if (parseBootPathString(&bootOption.kernel, kernelOption->value.valStr))
			return 1;
		if (strlen(initrdOption->value.valStr)) {
			if (parseBootPathString(&bootOption.initrd, initrdOption->value.valStr))
				return 1;
			haveInitrd = true;
		}

		boot(&bootOption); // This call should not return.

		printf("Boot failure. Please try a different boot option.\n");
		return 1;

	} else if(interactive) {
		printf("Please set the `kernel' option first. For example:\n");
		printf("  set kernel                 hd0:0:/boot/kernel.elf\n");
		printf("  set kernel FSID=0123456789abcdef:/boot/kernel.elf *\n");
		printf("  set kernel          FSLABEL=BOOT:/boot/kernel.elf *\n\n");
		printf("* not yet supported\n");
		return 1;
	} else {
		printf("error: `boot' without a kernel option set\n");
		return 1;
	}
}

CMD_DEF(cls) {
	if (!interactive)
		return 1;

	cls();

	return 0;
}

static void printDiskInfo(Disk *disk) {
	if (!disk->available) {
		printf("Disk hd%u is unavailable, possibly due to I/O errors.\n", disk->diskNo);
		return;
	}

	if (disk->partitionCount) {
		printf("Disk hd%u contains the following partitions:\n\n", disk->diskNo);
		int ret = printf(
			"      %15s   %15s %3s %1s %-10s %8s %12s\n",
			"LBA Start", "LBA End", "Id", "B", "Size", "FS", "Label"
		);
		for (int i=0; i<ret-1; i++)
			putch('-');
		putch('\n');

		for (uint32_t i=0; i<MIN(disk->partitionCount, DISK_MAX_PARTITIONS_PER_DISK); i++) {
			Partition *part = &disk->partitions[i];

			printf("hd%u:%u %#04x.%08x - %#04x.%08x %02xh %c %'9uM %8s %12s\n",
				disk->diskNo, part->partitionNo,
				(uint32_t)(part->lbaStart >> 32), (uint32_t)part->lbaStart,
				(uint32_t)((part->lbaStart + part->blockCount) >> 32),
				(uint32_t)(part->lbaStart + part->blockCount),
				part->type, part->active ? '*' : ' ',
				part->blockCount >> 32
					? 0
					: (uint32_t)part->blockCount / 1024 * 512 / 1024,
				part->fsDriver ? part->fsDriver->name : "",
				part->fsLabel
			);
		}
		putch('\n');
	} else {
		printf("No partitions were found on disk hd%u\n\n", disk->diskNo);
	}

	if (disk->blockCount >> 32) {
		printf(
			"Disk hd%u has more than %'uM blocks of %'u bytes,\ntotalling more than %'u GiB.\n",
			disk->diskNo,
			0xffffffff / 1024 / 1024, disk->blockSize,
			0xffffffff / 1024 / 1024 * disk->blockSize / 1024
		);
	} else {
		printf(
			"Disk hd%u has %'u blocks of %u Bytes, totalling %'u MiB\n",
			disk->diskNo,
			(uint32_t)disk->blockCount,
			disk->blockSize,
			((uint32_t)disk->blockCount) / 1024 * disk->blockSize / 1024
		);
	}
}

CMD_DEF(disk_info) {
	if (!interactive)
		return 1;

	if (argc == 1) {
		printf("A total of %u disks were detected.\n", diskCount);
		return 0;
	} else if (argc == 2) {
		uint32_t diskNo = atoi(argv[1]);
		if (diskNo < diskCount) {
			printDiskInfo(&disks[diskNo]);
			return 0;
		} else {
			printf("Disk number out of range\n");
			return 1;
		}
	} else {
		printf("usage: disk-info [DISK_NO]\n");
		return 1;
	}
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
		printf("Type `help [FUNCTION]' for information on that function.\n");
		printf("Spaces in parameters must be escaped with `\\' (quoting is not allowed).\n");
		printf("The following commands are supported by the bootloader:\n\n");

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
	case CONFIG_OPTION_TYPE_INT32:
		printf("%d", option->value.valInt32);
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
			case CONFIG_OPTION_TYPE_INT32:
				value.valInt32 = atoi(argv[2]);
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
