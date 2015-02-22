/**
 * \file
 * \brief     Common filesystem functions.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _FS_FS_H
#define _FS_FS_H

#include "common.h"

typedef struct FileSystemDriver FileSystemDriver;

#include "disk/disk.h"

typedef struct Partition Partition;

#define FS_MAX_FILE_NAME_LENGTH 63

#define FS_SUCCESS         (0)
#define FS_IO_ERROR       (-1)
#define FS_INTERNAL_ERROR (-2)
#define FS_FILE_NOT_FOUND (-3)

typedef enum {
	FILE_TYPE_REGULAR,
	FILE_TYPE_DIRECTORY,
} FileType;

/**
 * \brief Generic file information structure, to be passed to FS functions of
 *        any FS driver.
 */
typedef struct {
	char     name[FS_MAX_FILE_NAME_LENGTH + 1];
	uint64_t fsAddressStart;   ///< An implementation dependent address.
	uint64_t fsAddressCurrent; ///< An implementation dependent address, used during reads.
	uint64_t size;
	FileType type;
} FileInfo;

/**
 * \brief A collection of properties and functions that describe a certain type
 *        of filesystem.
 */
struct FileSystemDriver {
	const char *name; ///< Name of the file system driver.

	/**
	 * \brief Detect this FS type on a partition.
	 *
	 * \param partition
	 *
	 * \return whether the FS type was detected
	 */
	bool (*detect)(
		Partition *partition
	);

	/**
	 * \brief Fill a FileInfo structure for the given path.
	 *
	 * \param partition
	 * \param fileInfo
	 * \param path
	 *
	 * \return zero on success, non-zero on failure
	 * \retval FS_SUCCESS
	 * \retval FS_FILE_NOT_FOUND
	 * \retval FS_INTERNAL_ERROR
	 * \retval FS_IO_ERROR
	 */
	int (*getFile)(
		Partition *partition, FileInfo *fileInfo,
		const char *path
	);

	/**
	 * \brief Read `blockSize` bytes from the given file.
	 *
	 * The buffer MUST be at least `partition->disk->blockSize` bytes long.
	 *
	 * The caller should check `fileInfo->size`; When attempting to read past
	 * the end of a file, the function will return `FS_IO_ERROR`.
	 *
	 * \param partition
	 * \param fileInfo
	 * \param buffer
	 *
	 * \return zero on success, non-zero on failure
	 * \retval FS_SUCCESS
	 * \retval FS_INTERNAL_ERROR
	 * \retval FS_IO_ERROR
	 */
	int (*readFileBlock)(
		Partition *partition, FileInfo *fileInfo,
		uint8_t *buffer
	);

	/**
	 * \brief Read at most `count` directory entries from the given directory.
	 *
	 * \param partition
	 * \param fileInfo
	 * \param files
	 * \param offset amount of directory entries to skip
	 * \param count
	 *
	 * \return zero on success, non-zero on failure
	 * \retval FS_SUCCESS
	 * \retval FS_INTERNAL_ERROR
	 * \retval FS_IO_ERROR
	 */
	int (*readDir)(
		Partition *partition, FileInfo *fileInfo,
		FileInfo files[],
		size_t offset,
		size_t count
	);
};


/**
 * \brief Detect a filesystem on the given partition.
 *
 * \param part
 *
 * \return zero if a filesystem was detected, non-zero on error
 */
int fsDetect(Partition *part);

#endif /* _FS_FS_H */
