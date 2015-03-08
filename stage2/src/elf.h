/**
 * \file
 * \brief     ELF Loader.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _ELF_H
#define _ELF_H

#include "common.h"
#include "fs/fs.h"

/**
 * \brief Loads an ELF binary into memory and runs it.
 *
 * Only returns if an error occurred.
 *
 * \param file
 *
 * \return non-zero on failure
 */
int loadElf(FileInfo *file);

#endif /* _ELF_H */
