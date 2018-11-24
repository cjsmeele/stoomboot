/**
 * \file
 * \brief     ELF Loader.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015-2018, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "elf.h"
#include "disk/disk.h"
#include "console.h"
#include "memmap.h"
#include "far.h"

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
	elf32_addr_t pAddr;    ///< Physical address destination.
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
	elf64_addr_t  pAddr;    ///< Physical address destination.
	elf64_xword_t sizeFile; ///< Segment size within the ELF binary.
	elf64_xword_t sizeMem;  ///< Segment size in memory.
	elf64_xword_t align;
} __attribute__((packed)) Elf64PhEntry;


uint64_t loadElf(FileInfo *file) {

	Partition *part = file->partition;
	size_t bufferSize = part->disk->blockSize;
	uint8_t buffer[bufferSize]; ///< We'll use this for IO.

	if (file->size < sizeof(Elf64Header))
		goto readError;

	// XXX: We assume that we will never have to seek backwards in the file.
	//      This is a pretty big assumption, that as far as I can remember does
	//      not have any grounds in the ELF file format.
	//      However, all binaries I've built so far (using GCC's and LLVM's
	//      toolchains) have allowed for me to make this assumption.
	//      (that is, the program headers are in the expected order)
	// Our current position in the file.
	size_t bytesRead = 0;

	// Read one block. Fail on premature EOF or and IO errors.
#define READ_NEXT_BLOCK() \
		{ if (bytesRead >= file->size)                                   goto readError; \
		  if (part->fsDriver->readFileBlock(file, buffer) != FS_SUCCESS) goto readError; \
		  bytesRead += part->disk->blockSize; }

	// Read the first block.
	READ_NEXT_BLOCK();

	bool is64;
	uint64_t entryPoint;
	uint16_t phEntSize;
	uint16_t phNum;
	uint64_t phOff;
	uint16_t objType;

	// Parse the ELF header to produce values for the variables above.
	{
		// The header can be either of these.
		Elf32Header *header32 = (Elf32Header*)buffer;
		Elf64Header *header64 = (Elf64Header*)buffer;

		if (!strneq(header32->ident.magic, "\x7f" "ELF", 4)) {
			printf("error: Invalid ELF magic\n");
			return NULL;
		}

		// We support both 32-bit and 64-bit ELF.
		if (
			   header32->ident.class       < 1 || header32->ident.class > 2
			|| header32->ident.endianness != 1
		) {
			printf("error: Incompatible platform in ELF header.\n");
			return NULL;
		}

		// Extract the info we need from the ELF header.
		is64 = header32->ident.class == 2;

		entryPoint = is64 ? header64->entry     : header32->entry;
		phEntSize  = is64 ? header64->phEntSize : header32->phEntSize;
		phNum      = is64 ? header64->phNum     : header32->phNum;
		phOff      = is64 ? header64->phOff     : header32->phOff;
		objType    = is64 ? header64->type      : header32->type;

		// Note that while we can read ELF64 files, we cannot directly execute 64-bit code.
		// A loaded 64-bit program must include 32-bit protected mode code to make the switch to long mode.
		// Additionally, loads to physical addresses outside the 32-bit address space are not currently possible for Stoomboot.
	}

	// Sanity checks.

	if (objType != 2) {
		printf("error: ELF is not executable.\n");
		return NULL;
	}

	if (phOff >> 32) {
		printf("error: ELF too large (phOff >> 32).\n");
		return NULL;
	}

	if (
		   ( is64 && phEntSize != sizeof(Elf64PhEntry))
		|| (!is64 && phEntSize != sizeof(Elf32PhEntry))
	) {
		printf("error: Invalid ELF program header size (%u).\n", phEntSize);
		return NULL;
	}

	if (phNum * phEntSize > 2048) {
		// Looks like a sane limit.
		printf("error: ELF program header too large > 2048.\n");
		return NULL;
	}

	{
		// Obtain information about the segments we need to load into memory,
		// either from disk or by writing null bytes.
		struct LoadableSegments {
			uint32_t fileOffset;
			uint32_t memAddr;
			uint32_t fileSize;
			uint32_t memSize;  // memSize > fileSize => we need to clear remaining memory.
		} loadableSegments[phNum];

		uint8_t  phBuffer[phEntSize];
		uint32_t phEntriesProcessed = 0;
		uint32_t bufferOff          = 0;

		uint32_t totalMemSize      = 0;
		uint32_t totalFileCopySize = 0;

	// This reads the minimum amount of blocks necessary in order to get the
	// byte at "off" in the buffer. Used for seeking.
	// bufferOff will be filled with the position off the byte at "off" within "buffer".
#define READ_UP_TO_AND_INCLUDING(off) \
		{ while (bytesRead < (off) + ((uint32_t)(off) % part->disk->blockSize ? 0 : 1)) \
			READ_NEXT_BLOCK(); \
		bufferOff = (uint32_t)(off) % part->disk->blockSize; }

		// Read until we have read the start of the program headers.
		READ_UP_TO_AND_INCLUDING(phOff);

		for (uint32_t i = 0; i < phNum; ++i) {
			for (uint32_t j = 0; j < phEntSize; ++j) {
				if (bufferOff >= MIN(part->disk->blockSize, file->size - (bytesRead - part->disk->blockSize))) {
					READ_NEXT_BLOCK();
					bufferOff = 0;
				}
				phBuffer[j] = buffer[bufferOff++];
			}

			Elf32PhEntry *phEnt32 = (Elf32PhEntry*)phBuffer;
			Elf64PhEntry *phEnt64 = (Elf64PhEntry*)phBuffer;;

			uint32_t type   = is64 ? phEnt64->type   : phEnt32->type;

			uint64_t offset   = is64 ? phEnt64->offset   : phEnt32->offset;
			uint64_t paddr    = is64 ? phEnt64->pAddr    : phEnt32->pAddr;
			uint64_t sizeFile = is64 ? phEnt64->sizeFile : phEnt32->sizeFile;
			uint64_t sizeMem  = is64 ? phEnt64->sizeMem  : phEnt32->sizeMem;

			if (type == 1) { // 1 == PT_LOAD.
				loadableSegments[phEntriesProcessed].memAddr    = (uint32_t)paddr;
				loadableSegments[phEntriesProcessed].memSize    = (uint32_t)sizeMem;
				loadableSegments[phEntriesProcessed].fileOffset = (uint32_t)offset;
				loadableSegments[phEntriesProcessed].fileSize   = (uint32_t)sizeFile;

				totalMemSize      += sizeMem;
				totalFileCopySize += sizeFile;

				// We need to check this against the memory map provided to us by the BIOS.
				if (!isMemAvailable(paddr, sizeMem)) {
					printf(
						"error: Insufficient available memory for segment %#08x-%#08x\n",
						loadableSegments[phEntriesProcessed].memAddr,
						loadableSegments[phEntriesProcessed].memSize
					);
					return NULL;
				}
				phEntriesProcessed++;
			}
		}

		phNum = phEntriesProcessed; // Filter out non-LOAD segments.

		bool showProgress = totalFileCopySize > 1024*1024;
		int progressWidth = 20;
		int progressI     =  0;
		uint32_t totalFileCopied = 0;

		if (showProgress) {
			printf("Loading [");
			for (int j = 0; j < progressWidth; ++j)
				putch(' ');
			putch(']');
		}

		for (uint32_t i = 0; i < phNum; ++i) {
			//printf("\nseg nr %d/%d\n", i+1, phNum);
			struct LoadableSegments *seg = &loadableSegments[i];
			uint16_t bs = part->disk->blockSize;

			if (!seg->memAddr || !seg->memSize)
				continue; // An empty segment? Done.

			if (seg->fileOffset && seg->fileSize) {
				// We have stuff to load from disk.

				READ_UP_TO_AND_INCLUDING(seg->fileOffset);

				// Copy the first part.
				uint32_t memI = MIN(bs - bufferOff, seg->fileSize);
				farcpy(
					seg->memAddr,
					(uint32_t)buffer + bufferOff,
					memI // = keeping track of the amount of copied bytes.
				);
				totalFileCopied += memI;

				while (memI < seg->fileSize) {
					// Read a block.
					READ_NEXT_BLOCK();
					uint32_t toRead = MIN(bs, seg->fileSize - memI);
					// Copy it to the right address.
					farcpy(
						seg->memAddr + memI,
						(uint32_t)buffer,
						toRead
					);
					memI            += toRead;
					totalFileCopied += toRead;

					// Show progress indicator.
					if (showProgress && (bytesRead & 0x7fff) == 0) {
						int dots = 1+ (totalFileCopied * progressWidth) / (totalFileCopySize);
						printf("\rLoading [");
						for (int j = 0; j < dots-1; ++j)
							putch('=');
						const char *x = "/-\\|";
						putch(x[progressI % 4]);
						for (int j = dots; j < progressWidth; ++j)
							putch(' ');
						printf("] %6dK", totalFileCopied/1024);
						++progressI;
					}
				}
			}

			// Clear the remaining memory.
			// This is how .bss sections work, for example.
			if (seg->memSize - seg->fileSize > 0) {
				msleep(500);
				farzero(
					seg->memAddr + seg->fileSize,
					seg->memSize - seg->fileSize
				);
			}
		}

		return entryPoint;
	}

readError:
	printf("error: Could not read ELF binary\n");
	return NULL;
}
