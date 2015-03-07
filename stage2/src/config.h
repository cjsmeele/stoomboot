/**
 * \file
 * \brief     Configuration options.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _CONFIG_H
#define _CONFIG_H

#include "common.h"

#define CONFIG_STRING_VALUE_BUFFER_SIZE 128

typedef enum {
	CONFIG_OPTION_TYPE_INT32,
	CONFIG_OPTION_TYPE_STRING,
} ConfigOptionType;

typedef union {
	char    *valStr;
	 int32_t valInt32;
} ConfigOptionValue;

typedef struct {
	const char       *key;
	ConfigOptionType  type;
	ConfigOptionValue value;
} ConfigOption;

extern const size_t configOptionCount;
extern ConfigOption configOptions[];

/**
 * \brief Get a configuration option.
 *
 * \param key
 *
 * \return
 */
ConfigOption *getConfigOption(const char *key);

/**
 * \brief Set a configuration option.
 *
 * \param key
 * \param value
 */
void setConfigOption(const char *key, ConfigOptionValue value);

/**
 * \brief Clear configuration strings.
 */
void initConfig();

#endif /* _CONFIG_H */
