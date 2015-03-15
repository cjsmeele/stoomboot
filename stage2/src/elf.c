/**
 * \file
 * \brief     ELF Loader.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "elf.h"
#include "disk/disk.h"
#include "console.h"
#include "dump.h"
#include "far.h"
#include "protected.h"

// ELF32 types.
typedef uint32_t elf32_addr_t;
typedef uint16_t elf32_half_t;
typedef uint32_t elf32_off_t;
typedef uint32_t elf32_word_t;
typedef  int32_t elf32_sword_t;

// ELF64 types.
typedef uint64_t elf64_addr_t;
typedef uint64_t elf64_off_t;
typedef uint16_t elf64_half_t;
typedef uint32_t elf64_word_t;
typedef  int32_t elf64_sword_t;
typedef uint64_t elf64_xword_t;
typedef  int64_t elf64_sxword_t;

/**
 * \brief ELF32 Header.
 */
typedef struct {
	struct {
		char    magic[4];
		uint8_t class;      ///< 1 = 32-bit, 2 = 64-bit.
		uint8_t endianness; ///< 1 = little-endian, 2 = big-endian.
		uint8_t version;
		uint8_t _reserved1[9];
	} ident;

	elf32_half_t type;
	elf32_half_t machine;
	elf32_word_t version;
	elf32_addr_t entry;
	elf32_off_t  phOff; ///< Program Header offset.
	elf32_off_t  shOff; ///< Section Header offset.
	elf32_word_t flags;
	elf32_half_t ehSize;
	elf32_half_t phEntSize;
	elf32_half_t phNum;
	elf32_half_t shEntSize;
	elf32_half_t shEntNum;
	elf32_half_t shStrIndex;
} __attribute__((packed)) Elf32Header;

/**
 * \brief ELF64 Header.
 */
typedef struct {
	struct {
		char    magic[4];
		uint8_t class;      ///< 1 = 32-bit, 2 = 64-bit.
		uint8_t endianness; ///< 1 = little-endian, 2 = big-endian.
		uint8_t version;
		uint8_t _reserved1[9];
	} ident;

	elf64_half_t type;
	elf64_half_t machine;
	elf64_word_t version;
	elf64_addr_t entry;
	elf64_off_t  phOff; ///< Program Header offset.
	elf64_off_t  shOff; ///< Section Header offset.
	elf64_word_t flags;
	elf64_half_t ehSize;
	elf64_half_t phEntSize;
	elf64_half_t phNum;
	elf64_half_t shEntSize;
	elf64_half_t shEntNum;
	elf64_half_t shStrIndex;
} __attribute__((packed)) Elf64Header;

/**
 * \brief ELF32 Program Header table entry.
 */
typedef struct {
	elf32_word_t type;
	elf32_off_t  offset;   ///< Offset in this file.
	elf32_addr_t vAddr;    ///< Virtual address destination.
	elf32_addr_t pAddr;    ///< Do not use.
	elf32_word_t sizeFile; ///< Segment size within the ELF binary.
	elf32_word_t sizeMem;  ///< Segment size in memory.
	elf32_word_t flags;
	elf32_word_t align;
} __attribute__((packed)) Elf32PhEntry;

/**
 * \brief ELF64 Program Header table entry.
 */
typedef struct {
	elf64_word_t  type;
	elf64_word_t  flags;
	elf64_off_t   offset;   ///< Offset in this file.
	elf64_addr_t  vAddr;    ///< Virtual address destination.
	elf64_addr_t  pAddr;    ///< Do not use.
	elf64_xword_t sizeFile; ///< Segment size within the ELF binary.
	elf64_xword_t sizeMem;  ///< Segment size in memory.
	elf64_xword_t align;
} __attribute__((packed)) Elf64PhEntry;


int loadElf(FileInfo *file) {

	/// \todo This function should be split up.

	Partition *part = file->partition;
	uint8_t buffer[part->disk->blockSize];

	if (file->size < sizeof(Elf64Header)) {
		printf("ELF binary too small\n");
		return -1;
	}

	size_t bytesRead = 0;

	int ret = part->fsDriver->readFileBlock(file, buffer);
	if (ret != FS_SUCCESS) {
		printf("Could not read ELF binary");
		return -1;
	}

	bytesRead += part->disk->blockSize;

	Elf32Header *header32 = (Elf32Header*)buffer;
	Elf64Header *header64 = (Elf64Header*)buffer;

	if (!strneq(header32->ident.magic, "\x7f" "ELF", 4)) {
		printf("error: Invalid ELF magic\n");
		return -1;
	}

	if (
		   header32->ident.class       < 1 || header32->ident.class > 2
		|| header32->ident.endianness != 1
	) {
		// Support both 32-bit and 64-bit ELF.
		printf("error: Incompatible platform in ELF header.\n");
		return -1;
	}

	bool is64 = header32->ident.class == 2;

	uint64_t entryPoint = is64 ? header64->entry     : header32->entry;
	uint16_t phEntSize  = is64 ? header64->phEntSize : header32->phEntSize;
	uint16_t phNum      = is64 ? header64->phNum     : header32->phNum;
	uint64_t phOff      = is64 ? header64->phOff     : header32->phOff;
	uint16_t objType    = is64 ? header64->type      : header32->type;

	if (objType != 2) {
		printf("error: ELF binary is not executable.\n");
		return -1;
	}

	if (phOff >> 32) {
		printf("error: ELF binary too large (phOff >> 32).\n");
		return -1;
	}

	if (
		   ( is64 && phEntSize != sizeof(Elf64PhEntry))
		|| (!is64 && phEntSize != sizeof(Elf32PhEntry))
	) {
		printf("error: Invalid ELF program header size (%u).\n", phEntSize);
		return -1;
	}

	if (phNum * phEntSize > 2048) {
		// Looks like a sane limit.
		printf("error: ELF program header too large > 2048.\n");
		return -1;
	}

	struct {
		uint32_t fileOffset;
		uint32_t memAddr;
		uint32_t fileSize;
		uint32_t memSize;
	} loadableSegments[phNum];

	memset(loadableSegments, 0, sizeof(loadableSegments));

	uint8_t  phBuffer[phEntSize];
	uint32_t phEntriesProcessed = 0;
	uint32_t bufferOff   = 0;

	while (
		bytesRead < phOff
		+ ((uint32_t)phOff % part->disk->blockSize ? 0 : 1)
	) {
		if (bytesRead >= file->size) {
			printf("error: Unexpected EOF in ELF file\n");
			return -1;
		}

		if (part->fsDriver->readFileBlock(file, buffer) != FS_SUCCESS) {
			printf("error: Could not read ELF binary\n");
			return -1;
		}

		bytesRead += part->disk->blockSize;
	}

	bufferOff = (uint32_t)phOff % part->disk->blockSize;

	for (uint32_t i=0; i<phNum; i++) {
		for (uint32_t j=0; j<phEntSize; j++) {
			if (bufferOff >= MIN(part->disk->blockSize, file->size - (bytesRead - part->disk->blockSize))) {
				if (bytesRead >= file->size) {
					printf("error: Unexpected EOF in ELF file\n");
					return -1;
				}

				if (part->fsDriver->readFileBlock(file, buffer) != FS_SUCCESS) {
					printf("error: Could not read ELF binary\n");
					return -1;
				}

				bytesRead += part->disk->blockSize;
				bufferOff = 0;
			}
			phBuffer[j] = buffer[bufferOff++];
		}

		Elf32PhEntry *phEnt32 = (Elf32PhEntry*)phBuffer;
		Elf64PhEntry *phEnt64 = (Elf64PhEntry*)phBuffer;;

		uint32_t type   = is64 ? phEnt64->type   : phEnt32->type;

		uint64_t offset   = is64 ? phEnt64->offset   : phEnt32->offset;
		uint64_t vaddr    = is64 ? phEnt64->vAddr    : phEnt32->vAddr;
		uint64_t sizeFile = is64 ? phEnt64->sizeFile : phEnt32->sizeFile;
		uint64_t sizeMem  = is64 ? phEnt64->sizeMem  : phEnt32->sizeMem;

		if (type == 1) { // 1 == PT_LOAD.
			loadableSegments[phEntriesProcessed].memAddr    = (uint32_t)vaddr;
			loadableSegments[phEntriesProcessed].memSize    = (uint32_t)sizeMem;
			loadableSegments[phEntriesProcessed].fileOffset = (uint32_t)offset;
			loadableSegments[phEntriesProcessed].fileSize   = (uint32_t)sizeFile;
		}

		phEntriesProcessed++;

		if (type != 1)
			continue;
	}

	if (phEntriesProcessed != phNum) {
		printf("error: Failed to read all program header entries\n");
		return -1;
	}

	/// \todo Check memory map before loading kernel segments.

	uint8_t segmentBuffer[512];

	for (uint32_t i=0; i<phNum; i++) {
		if (!loadableSegments[i].memAddr || !loadableSegments[i].memSize)
			continue;

		if (loadableSegments[i].fileOffset && loadableSegments[i].fileSize) {
			while (
				bytesRead <
					  loadableSegments[i].fileOffset
					+ (loadableSegments[i].fileOffset % part->disk->blockSize ? 0 : 1)
			) {
				if (bytesRead >= file->size) {
					printf("error: Unexpected EOF in ELF file\n");
					return -1;
				}

				if (part->fsDriver->readFileBlock(file, buffer) != FS_SUCCESS) {
					printf("error: Could not read ELF binary\n");
					return -1;
				}

				bytesRead += part->disk->blockSize;
			}

			bufferOff = (uint32_t)loadableSegments[i].fileOffset % part->disk->blockSize;

			for (uint32_t j=0; j<loadableSegments[i].fileSize; j += ELEMS(segmentBuffer)) {
				for (uint32_t k=0; k<ELEMS(segmentBuffer); k++) {
					if (bufferOff >= MIN(part->disk->blockSize, file->size - (bytesRead - part->disk->blockSize))) {
						if (bytesRead >= file->size) {
							printf("error: Unexpected EOF in ELF file\n");
							return -1;
						}

						if (part->fsDriver->readFileBlock(file, buffer) != FS_SUCCESS) {
							printf("error: Could not read ELF binary\n");
							return -1;
						}

						bytesRead += part->disk->blockSize;
						bufferOff = 0;
						if (!(bytesRead % 2048))
							putch('.');
					}
					segmentBuffer[k] = buffer[bufferOff++];
				}
				farcpy(
					loadableSegments[i].memAddr + j,
					(uint32_t)segmentBuffer,
					MIN(ELEMS(segmentBuffer), loadableSegments[i].fileSize - j)
				);
			}

			/// \todo Clear remaining segment memory.
		}
	}

	enterProtectedMode(entryPoint);

	return -1;
}
