/*
 * Copyright (C) 2007-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _IMAGE3_SUPER_BLOCK_H_
#define _IMAGE3_SUPER_BLOCK_H_

/*
 * All structures defined here are on-media formats.
 */
#pragma pack(push, 1)

/*
 * The I3 layout is described by the superblock, which is located at
 * kI2SuperblockOffset from the top of the media.  The superblock is a write-once
 * item; it should only be updated when the bootblock code is rewritten, implying
 * a complete re-initialisation of the media.
 *
 * Following the superblock there is a reserved area for boot code, after which
 * image headers describe individual images.
 *
 *            0x0      --------------------
 *
 * 		       Image3Superblock
 *			 ...
 *			 ...
 *            0x8000   --------------------
 *                       Image3Header
 *                       ...
 *			 Image3Header
 *			 ...
 *
 */

typedef struct {
	u_int32_t	isMagic;
#define kImage2SuperMagic		0x494d4732	/* "IMG2" */
	u_int32_t	isImageGranule;			/* fundamental block size (bytes) */
	u_int32_t	isImageOffset;			/* image header offset within granule (image granules) */
	u_int32_t	isBootBlockSize;		/* size of the bootblock (image granules) */
	u_int32_t	isImageAvail;			/* total granules available for images. */
	u_int32_t	isNvramGranule;			/* size of NVRAM blocks (bytes) */
	u_int32_t	isNvramOffset;			/* offset to first NVRAM block (nvram granules) */
	u_int32_t	isFlags;			/* flags field reserved for future use */
	u_int32_t	isRsvd1;			/* reserved 1 for future use */
	u_int32_t	isRsvd2;			/* reserved 2 for future use */
	u_int32_t	isRsvd3;			/* reserved 3 for future use */
	u_int32_t	isRsvd4;			/* reserved 4 for future use */
	u_int32_t	isCheck;			/* CRC-32 of header fields preceding this one */
} Image3Superblock;

/* Image types */
#define IMAGE_TYPE_DIAG         'diag' // diagnostics
#define IMAGE_TYPE_IBOOT_LLB    'illb' // iboot (darwin) first-stage loader
#define IMAGE_TYPE_IBOOT        'ibot' // iboot (darwin) loader
#define IMAGE_TYPE_DEVTREE      'dtre' // darwin device tree
#define IMAGE_TYPE_LOGO         'logo' // boot logo image
#define IMAGE_TYPE_RECMODE      'recm' // recovery mode image
#define IMAGE_TYPE_NEEDSERVICE  'nsrv' // recovery mode image
#define IMAGE_TYPE_BATTERYLOW0  'batl' // battery low image - empty
#define IMAGE_TYPE_BATTERYLOW1  'batL' // battery low image - red
#define IMAGE_TYPE_BATTERYCHRG  'batC' // battery charge image
#define IMAGE_TYPE_ENV          'ienv' // environment vars
#define IMAGE_TYPE_TSYS         'tsys' // tsys tester
#define IMAGE_TYPE_CHIME        'beep' // boot chime

/* restore structure packing */
#pragma pack(pop)

#endif /* _IMAGE3_SUPER_BLOCK_H_ */
