/**
 * \file
 * \brief     VESA BIOS Extensions support.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2015, Chris Smeele. All rights reserved.
 * \license   MIT. See LICENSE for the full license text.
 */
#include "vbe.h"
#include "console.h"
#include "far.h"

uint16_t vbeCurrentMode = 0xffff;

typedef struct {
	char     vbeSignature[4];
	uint16_t vbeVersion;
	uint32_t oemStrPtr;
	uint32_t capabilities;
	uint32_t videoModePtr;
	uint16_t totalMemory; ///< In 64K blocks.
	uint16_t oemSoftwareRev;
	uint32_t oemVendorNamePtr;
	uint32_t oemProductNamePtr;
	uint32_t oemProductRevPtr;
	uint8_t  _reserved1[222];
	uint8_t  oemData[256];
} __attribute__((packed)) VbeInfoBlock;

int vbeGetModeInfo(VbeModeInfoBlock *modeInfo, uint16_t mode) {
	uint16_t status = 0x4f01;
	asm volatile (
		"int $0x10"
		: "+a" (status)
		: "D" (modeInfo),
		  "c" (mode)
		: "cc", "memory"
	);
	if (status == 0x004f) {
		return 0;
	} else {
		printf("VBE error (AX=%04xh)\n", status);
		return -1;
	}
}

static int getVbeInfo(VbeInfoBlock *vbeInfo) {
	memset(vbeInfo, 0, sizeof(VbeInfoBlock));

	// We want VBE v2 information.
	strncpy(vbeInfo->vbeSignature, "VBE2", 3);

	uint16_t status = 0x4f00;
	asm volatile (
		"int $0x10"
		: "+a" (status)
		: "D" (vbeInfo)
		: "cc", "memory"
	);

	if (status == 0x004f && strneq(vbeInfo->vbeSignature, "VESA", 4)) {
		return 0;
	} else {
		printf("VBE seems to be unsupported on your system (AX=%04xh)\n", status);
		return -1;
	}
}

uint16_t vbeGetModeFromModeInfo(uint32_t width, uint32_t height, uint32_t bbp) {
	VbeInfoBlock vbeInfo;
	if (getVbeInfo(&vbeInfo))
		return 0xffff;

	uint16_t *vbeModeList = (uint16_t*)vbeInfo.videoModePtr;

	VbeModeInfoBlock modeInfo;
	uint16_t requestedMode    = 0xffff;
	uint32_t requestedModeBbp = 0;

	while (true) {
		memset(&modeInfo, 0, sizeof(VbeModeInfoBlock));

		uint16_t mode;

		if ((uint32_t)vbeModeList >> 16) {
			// Is the mode list entry in a different segment?
			// Copy a single mode number to `mode`.
			segcpy((uint32_t)&mode, (uint32_t)vbeModeList, sizeof(mode));
			// It would be more efficient to copy multiple numbers at once,
			// but we are not in a hurry.
		} else {
			mode = *vbeModeList;
		}

		if (mode == 0xffff)
			break;

		if (vbeGetModeInfo(&modeInfo, mode))
			break;

		if (
			   modeInfo.xResolution  == width
			&& modeInfo.yResolution  == height
		) {
			if (modeInfo.bitsPerPixel == bbp) {
				return mode;
			} else if (!bbp && modeInfo.bitsPerPixel > requestedModeBbp) {
				requestedMode    = mode;
				requestedModeBbp = modeInfo.bitsPerPixel;
			}
		}

		vbeModeList++;
	}

	return requestedMode;
}

int vbeSetMode(uint16_t mode) {
	uint16_t status = 0x4f02;
	asm volatile (
		"int $0x10"
		: "+a" (status)
		: "b" (0b010000 << 10 | (mode & 0x3ff)), // Linear FB, clear display.
		  "D" (NULL)
		: "cc", "memory"
	);
	if (status == 0x004f) {
		vbeCurrentMode = mode;
		return 0;
	} else {
		printf("VBE error (AX=%04xh)\n", status);
		return -1;
	}
}

void vbeDumpModes() {

	uint16_t *vbeModeList = 0;

	VbeInfoBlock vbeInfo;

	if (getVbeInfo(&vbeInfo))
		return;

	vbeModeList = (uint16_t*)vbeInfo.videoModePtr;
	uint32_t vbeModeCount = 0;

	if (vbeModeList) {
		VbeModeInfoBlock modeInfo;

		int rowsPrinted = 0;

		while (true) {
			memset(&modeInfo, 0, sizeof(VbeModeInfoBlock));

			uint16_t mode;
			if ((uint32_t)vbeModeList >> 16) {
				segcpy((uint32_t)&mode, (uint32_t)vbeModeList, sizeof(mode));

			} else {
				mode = *vbeModeList;
			}

			if (mode == 0xffff)
				break;

			if (vbeGetModeInfo(&modeInfo, mode))
				break;

			vbeModeCount++;

			printf("VBE Mode: %4u (%04xh), %4u x %4u x %2u Attrs(%#04x)\n",
				mode,
				mode,
				modeInfo.xResolution,
				modeInfo.yResolution,
				modeInfo.bitsPerPixel,
				modeInfo.modeAttributes
			);

			if (rowsPrinted++ >= consoleHeight-3) {
				rowsPrinted = 0;
				puts("-- MORE --\r");
				Key key;
				getKey(&key, true);
				puts("          \r");
				if (key.chr == 'q')
					break;
			}

			vbeModeList++;
		}
		if (!vbeModeCount)
			printf("No VBE modes were detected\n");
	} else {
		printf("Invalid VBE mode list pointer\n");
	}
}
