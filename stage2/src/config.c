/**
 * \file
 * \brief     Configuration options.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "config.h"
#include "console.h"

static char optionKernelBuffer[CONFIG_STRING_VALUE_BUFFER_SIZE];
static char optionInitrdBuffer[CONFIG_STRING_VALUE_BUFFER_SIZE];

ConfigOption configOptions[] = {
	{ "timeout",      CONFIG_OPTION_TYPE_INT32,  .value.valInt32 = -1                 },
	{ "kernel",       CONFIG_OPTION_TYPE_STRING, .value.valStr   = optionKernelBuffer },
	{ "initrd",       CONFIG_OPTION_TYPE_STRING, .value.valStr   = optionInitrdBuffer },
	{ "video-width",  CONFIG_OPTION_TYPE_INT32,  .value.valInt32 = 0                  },
	{ "video-height", CONFIG_OPTION_TYPE_INT32,  .value.valInt32 = 0                  },
	{ "video-bbp",    CONFIG_OPTION_TYPE_INT32,  .value.valInt32 = 0                  },
	{ "video-mode",   CONFIG_OPTION_TYPE_INT32,  .value.valInt32 = 0                  },
};
const size_t configOptionCount = ELEMS(configOptions);

ConfigOption *getConfigOption(const char *key) {
	for (size_t i=0; i<ELEMS(configOptions); i++) {
		if (streq(configOptions[i].key, key))
			return &configOptions[i];
	}

	return NULL;
}

void setConfigOption(const char *key, ConfigOptionValue value) {
	ConfigOption *option = getConfigOption(key);
	if (option) {
		       if (option->type == CONFIG_OPTION_TYPE_INT32) {
			option->value.valInt32 = value.valInt32;
		} else if (option->type == CONFIG_OPTION_TYPE_STRING) {
			if (value.valStr) {
				strncpy(option->value.valStr, value.valStr, CONFIG_STRING_VALUE_BUFFER_SIZE-1);
				if (strlen(value.valStr) >= CONFIG_STRING_VALUE_BUFFER_SIZE)
					printf("warning: Option value truncated\n");
			} else {
				strncpy(option->value.valStr, "", 1);
			}
		} else {
			printf("warning: Tried to set unknown config option datatype\n");
		}
	} else {
		printf("warning: Tried to set non-existent config option '%s'\n", key);
	}
}

void initConfig() {
	optionKernelBuffer[0] = '\0';
	optionInitrdBuffer[0] = '\0';
}
