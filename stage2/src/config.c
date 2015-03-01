/**
 * \file
 * \brief     Configuration options.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "config.h"
#include "console.h"

static char optionMessageBuffer[CONFIG_STRING_VALUE_BUFFER_SIZE] = { };

ConfigOption configOptions[] = {
	{ "timeout", CONFIG_OPTION_TYPE_INT32,  .value.valInt32 = -1                  },
	{ "message", CONFIG_OPTION_TYPE_STRING, .value.valStr   = optionMessageBuffer },
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
		       if (option->type == CONFIG_OPTION_TYPE_UINT64) {
			option->value.valUInt64 = value.valUInt64;
		} else if (option->type == CONFIG_OPTION_TYPE_INT32) {
			option->value.valInt32 = value.valInt32;
		} else if (option->type == CONFIG_OPTION_TYPE_UINT32) {
			option->value.valUInt32 = value.valUInt32;
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
