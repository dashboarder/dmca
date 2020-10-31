/*
 * Copyright (C) 2007-2013 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __LIB_IMAGE_H
#define __LIB_IMAGE_H

#include <sys/types.h>
#include <lib/blockdev.h>
#include <list.h>

__BEGIN_DECLS

struct image_info {
	u_int32_t	imageLength;
	u_int32_t	imageAllocation;
	u_int32_t	imageType;
	u_int32_t	imagePrivateMagic;
#define IMAGE_MEMORY_INFO_MAGIC		'Memz'
#define IMAGE2_IMAGE_INFO_MAGIC		'img2'
#define IMAGE3_IMAGE_INFO_MAGIC		'img3'
#define IMAGE4_IMAGE_INFO_MAGIC		'img4'
	u_int32_t	imageOptions;
	void		*imagePrivate;
};

// images we might be interested in
#define IMAGE_TYPE_ANY			0	// any image type
#define IMAGE_TYPE_DIAG			'diag'	// diagnostics
#define IMAGE_TYPE_LLB			'illb'	// iboot first-stage loader
#define IMAGE_TYPE_IBOOT		'ibot'	// iboot second-stage loader
#define IMAGE_TYPE_IBSS			'ibss'	// iboot single stage
#define IMAGE_TYPE_IBEC			'ibec'	// iboot epoch change
#define IMAGE_TYPE_DALI			'dali'	// dali
#define IMAGE_TYPE_DEVTREE		'dtre'	// darwin device tree
#define	IMAGE_TYPE_DEVTREE_RESTORE	'rdtr'	// darwin device tree for restore
#define IMAGE_TYPE_RAMDISK		'rdsk'	// darwin ram disk for restore
#define IMAGE_TYPE_KERNELCACHE		'krnl'	// darwin kernel cache
#define	IMAGE_TYPE_KERNELCACHE_RESTORE	'rkrn'	// darwin kernel cache for restore
#define IMAGE_TYPE_LOGO			'logo'	// boot logo image
#define IMAGE_TYPE_RECMODE		'recm'	// recovery mode image
#define IMAGE_TYPE_NEEDSERVICE		'nsrv'	// need service image
#define IMAGE_TYPE_GLYPHCHRG		'glyC'	// glyph charge image
#define IMAGE_TYPE_GLYPHPLUGIN		'glyP'	// glyph plug in image
#define IMAGE_TYPE_BATTERYCHARGING0	'chg0'  // battery charging image - bright
#define IMAGE_TYPE_BATTERYCHARGING1	'chg1'  // battery charging image - dim
#define IMAGE_TYPE_BATTERYLOW0		'bat0'	// battery low image - empty
#define IMAGE_TYPE_BATTERYLOW1		'bat1'	// battery low image - red (composed onto empty)
#define IMAGE_TYPE_BATTERYFULL		'batF'	// battery full image list
#define IMAGE_TYPE_BATTERYFULL_N_TOTAL  (18)	// total number of images
#define IMAGE_TYPE_BATTERYFULL_N_START	(2)	// first image that's part of batF list (first two are bat0 and bat1)
#define IMAGE_TYPE_BATTERYFULL_N_RED	(4)	// last red battery image
#define IMAGE_TYPE_BATTERYFULL_N_FULL	(IMAGE_TYPE_BATTERYFULL_N_TOTAL-1)	// battery full image
#define IMAGE_TYPE_ENV			'ienv'	// environment vars
#define IMAGE_TYPE_TSYS			'tsys'	// tsys tester
#define IMAGE_TYPE_MONITOR		'hypr'	// monitor/hypervisor
#define IMAGE_TYPE_OS_RESTORE		'rosi'	// OS image for restore
#define IMAGE_TYPE_SEP_OS		'sepi'	// SEP OS image
#define IMAGE_TYPE_SEP_OS_RESTORE	'rsep'	// SEP OS image for restore
#define IMAGE_TYPE_CFE_LOADER		'cfel'	// SiVal's CFE loader
#define IMAGE_TYPE_RBM			'rbmt'	// SiVal's RBM test
#define IMAGE_TYPE_PHLEET		'phlt'	// SiVal's PHLEET test
#define IMAGE_TYPE_PE_RTOS		'pert'	// PE's RTOS environment
#define IMAGE_TYPE_HAMMER		'hmmr'	// PE's Hammer test
#define IMAGE_TYPE_FDRT			'fdrt'	// FDR Trust object for AP
#define IMAGE_TYPE_FDRS			'fdrs'	// FDR Trust object for SEP

// image options
#define IMAGE_OPTION_GREATER_EPOCH	(1 << 0)	// Allow platform epoch or greater
#define IMAGE_OPTION_REQUIRE_TRUST	(1 << 1)	// Regardless of security, require image
							// trust
#define IMAGE_OPTION_LOCAL_STORAGE	(1 << 2)	// Image came from local (personalised) storage
#define IMAGE_OPTION_NEW_TRUST_CHAIN	(1 << 3)	// New chain of trust. Image load library can use this information
							// enforce various policies.

#define IMAGE_OPTION_JUST_LOAD		(1 << 8)	// Just load the whole image, don't validate or look for a payload
#define IMAGE_OPTION_MEMORY		(1 << 9)	// Image comes from a memory bdev, so its hash isn't personalized

// XXX backwards compatibility
#define IMAGE_OPTION_ANY_EPOCH		IMAGE_OPTION_GREATER_EPOCH	// Allow any epoch for this image


// image keybag selectors
#define IMAGE_KEYBAG_SELECTOR_PROD	(1)
#define IMAGE_KEYBAG_SELECTOR_DEV	(2)


/**
 * Load an image from a file.
 *
 * \param[in]	path		Path including mountpoint from which to read.
 * \param[inout] address	Address for the buffer used both for file reading and into which
 *				the resulting image data will be placed.  Referenced pointer will
 * 				be zeroed on failure.
 * \param[inout] length		On input, the size of the buffer.  The file must file entirely
 *				within this size.  Updated on return to the size of the image
 *				data extracted from the image.
 * \param[in]	types		An array of types that the image must conform to one of,
 *				or NULL if no type checking should be performed.
 * \param[in]	count		The count of image types in the types array,
 *				or zero if NULL was passed.
 * \param[out]	actual		A pointer to receive the actual type for given image.
 *				Optional if count is 0 or 1.
 * \param[in]	options		Options to be passed when creating the image.
 *
 * \return	0 on success.
 */
int
image_load_file(
	const char *path, 
	addr_t *address,
	size_t *length,
	const uint32_t *types,
	uint32_t count,
	uint32_t *actual,
	uint32_t options);

/**
 * Load an image from memory.
 *
 * \param[in]	fromAddress	Address of the buffer containing the image.
 * \param[in]	fromLength	Size of the buffer containing the image.
 * \param[inout] address	Address for the buffer into which the resulting image data will
 *				be placed.  Referenced pointer will be zeroed on failure.
 * \param[inout] length		On input, the size of the destination buffer.  The image must file 
 * 				entirely within this size.  Updated on return to the size of the image
 *				data extracted from the image.
 * \param[in]	types		An array of types that the image must conform to one of,
 *				or NULL if no type checking should be performed.
 * \param[in]	count		The count of image types in the types array,
 *				or zero if NULL was passed.
 * \param[out]	actual		A pointer to receive the actual type for given image.
 *				Optional if count is 0 or 1.
 * \param[in]	options		Options to be passed when creating the image.
 *
 * \return	0 on success.
 */
int
image_load_memory(
	addr_t fromAddress,
	size_t fromLength,
	addr_t *address,
	size_t *length,
	const uint32_t *types,
	uint32_t count,
	uint32_t *actual,
	uint32_t options);


/**
 * Load an image of a given type from the set of known images.
 *
 * \param[inout] address	Address for the buffer into which the resulting image data will
 *				be placed.  Referenced pointer will be zeroed on failure.
 * \param[inout] length		On input, the size of the destination buffer.  The image must file 
 * 				entirely within this size.  Updated on return to the size of the image
 *				data extracted from the image.
 * \param[in]	type		The image type to be looked up.
 * \param[in]	options		Options to be passed when creating the image.
 *
 * \return	0 on success.
 */
int
image_load_type(
	addr_t *address,
	size_t *length,
	uint32_t type,
	uint32_t options);

/*
 * Lower-level primitives - avoid when possible.
 */
int
image_search_bdev(
	struct blockdev *bdev,
	off_t map_offset,
	uint32_t imageOptions);

void
image_free_bdev(
	struct blockdev *bdev);

int
image_load(
	struct image_info *image,
	const uint32_t *types,
	uint32_t count,
	uint32_t *actual,
	void **load_addr,
	size_t *load_len);

struct image_info *
image_find(
	u_int32_t type);

void
image_dump_list(
	bool detailed);

struct image_info *
image_create_from_memory(
	void *address,
	size_t length,
	uint32_t imageOptions);

void
image_free(
	struct image_info *image);

__END_DECLS

#endif
