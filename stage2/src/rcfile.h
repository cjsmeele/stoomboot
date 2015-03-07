/**
 * \file
 * \brief     Run-Commands / configuration file support.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _RCFILE_H
#define _RCFILE_H

#include "common.h"
#include "fs/fs.h"

/**
 * \brief Parse a bootloader RC file.
 *
 * \param file the RC file, must be a regular file
 *
 * \return zero on success, non-zero on failure
 */
int parseRcFile(FileInfo *file);

#endif /* _RCFILE_H */
