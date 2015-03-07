/**
 * \file
 * \brief     VESA BIOS Extensions support.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#ifndef _VBE_H
#define _VBE_H

#include "common.h"

extern uint16_t vbeCurrentMode;

typedef struct {
	//uint16_t modeAttributes;
	union {
		struct {
			bool attrSupported                     : 1;
			bool _reserved1                        : 1;
			bool attrTtySupported                  : 1;
			bool attrColor                         : 1;
			bool attrGraphics                      : 1;
			bool attrVgaCompatible                 : 1;
			bool attrVgaCompatibleWMMA             : 1;
			bool attrLinearFBAvailable             : 1;
			bool attrDoubleScanAvailable           : 1;
			bool attrInterlaceAvailable            : 1;
			bool attrTripleBufferAvailable         : 1;
			bool attrStereoScopicAvailable         : 1;
			bool attrDualDisplayStartAddrAvailable : 1;
			bool _reserved2                        : 3;
		};
		uint16_t modeAttributes;
	};

	uint8_t  winAAttributes;
	uint8_t  winBAttributes;
	uint16_t winGranularity;
	uint16_t winSize;
	uint16_t winASegment;
	uint16_t winBSegment;
	uint32_t winFuncPtr;
	uint16_t bytesPerScanLine;
	uint16_t xResolution;
	uint16_t yResolution;
	uint8_t  xCharSize;
	uint8_t  yCharSize;
	uint8_t  planeCount;
	uint8_t  bitsPerPixel;
	uint8_t  bankCount;
	uint8_t  memoryModel;
	uint8_t  bankSize; ///< In KiB.
	uint8_t  imagePaneCount;
	uint8_t _reserved3;

	uint8_t _directColorFields[9];

	uint32_t physBasePtr;
	uint32_t _reserved4;
	uint16_t _reserved5;

	uint8_t _vbe3Info[16];

	uint8_t _reserved6[189];

} __attribute__((packed)) VbeModeInfoBlock;

/**
 * \brief Finds the best matching video mode for the given parameters.
 *
 * If `bbp` is specified, the first perfect match will be returned.
 * If `bbp` is 0, the matching mode with the highest BBP will be returned.
 *
 * \param width
 * \param height
 * \param bbp
 *
 * \return a mode number, or 0xffff on failure
 */
uint16_t vbeGetModeFromModeInfo(uint32_t width, uint32_t height, uint32_t bbp);

/**
 * \brief Sets a video mode.
 *
 * \param mode a VBE mode number
 *
 * \return zero on success, non-zero on failure
 */
int vbeSetMode(uint16_t mode);

/**
 * \brief Gets VBE mode information for the given video mode.
 *
 * \param modeInfo
 * \param mode
 *
 * \return zero on success, non-zero on failure
 */
int vbeGetModeInfo(VbeModeInfoBlock *modeInfo, uint16_t mode);

/**
 * \brief Dumps VBE mode information for all VBE modes.
 */
void vbeDumpModes();

#endif /* _VBE_H */
