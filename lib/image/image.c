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
#include <debug.h>
#include <stdlib.h>
#include <lib/profile.h>
#include <lib/env.h>
#include <lib/image.h>
#include <errno.h>
#include <platform.h>
#include <platform/breadcrumbs.h>

#if WITH_FS
#include <lib/fs.h>
#include <platform/memmap.h>
#endif

#if WITH_IMAGE3
# include "image3/image3_wrapper.h"
#endif
#if WITH_IMAGE4
# include "image4/image4_wrapper.h"
#endif


#define MAP_SIZE 512

#if WITH_IMAGE3 || WITH_IMAGE4 || WITH_ENV
static uint32_t image_version = 0;
#endif

/*
 * Look for a compatible image superblock in the supplied block device.
 */
int
image_search_bdev(struct blockdev *bdev, off_t map_offset, uint32_t imageOptions)
{
	int err;
	void *buf = 0;
	int images_found = 0;

	posix_memalign(&buf, CPU_CACHELINE_SIZE, MAP_SIZE);

	/*
	 * Read in the image superblock.
	 */
	err = blockdev_read(bdev, buf, map_offset, MAP_SIZE);
	if (err != MAP_SIZE)
		goto exit;

#if WITH_IMAGE4
	if (images_found == 0) {
		images_found = image4_process_superblock(buf, bdev, map_offset, imageOptions);
		if (images_found)
			image_version = 4;
	}
#endif
#if WITH_IMAGE3
	if (images_found == 0) {
		images_found = image3_process_superblock((Image2Superblock *)buf, bdev, map_offset, imageOptions);
		if (images_found)
			image_version = 3;
	}
#endif

#if WITH_ENV
	if (image_version)
		env_set_uint("image-version", image_version, 0);
#endif

exit:
	if (buf)
		free(buf);

	return images_found;
}

/*
 * Free all images found by image_search_bdev(), above, for the specified bdev.
 */
void
image_free_bdev(struct blockdev *bdev)
{
#if WITH_IMAGE4
	image4_free_bdev(bdev);
#endif
#if WITH_IMAGE3
	image3_free_bdev(bdev);
#endif
}

/*
 * Load an already-known image.
 *
 * This function is complicated by the desire to support multiple image formats
 * at the same time.  The logic for images of an unknown type being loaded from
 * memory is that if configured, handlers should be tried in a canonical order,
 * falling back to a previous version if the loader returns EINVAL.
 */
int
image_load(struct image_info *image, const u_int32_t *types, u_int32_t count, u_int32_t *actual, void **load_addr, size_t *load_len)
{
	void	*addr = *load_addr;
	size_t	len = *load_len;
	int	ret_val = -1;

	if (NULL == image)
		goto exit;

	if (image->imageAllocation > len) {
		dprintf(DEBUG_CRITICAL, "image_load: image too large\n");
		platform_record_breadcrumb_int("image_load_pre_fail", kBCImgLoadImageTooLarge);
		goto exit;
	}

	if ((types == NULL) && (count != 0)) {
		dprintf(DEBUG_CRITICAL, "image_load: count must be zero if types is NULL\n");
		platform_record_breadcrumb_int("image_load_pre_fail", kBCImgLoadCountNotZero);
		goto exit;
	}

	if ((count > 1) && (actual == NULL)) {
		dprintf(DEBUG_CRITICAL, "image_load: actual must not be NULL when types has multiple values\n");
		platform_record_breadcrumb_int("image_load_pre_fail", kBCImgLoadCountIsZero);
		goto exit;
	}

	if (actual != NULL) {
		*actual = IMAGE_TYPE_ANY;
	}

	dprintf(DEBUG_INFO, "loading image with private magic %x\n", image->imagePrivateMagic);
	switch (image->imagePrivateMagic) {
#if WITH_IMAGE4
	case IMAGE4_IMAGE_INFO_MAGIC:
		ret_val = image4_load(image, types, count, actual, &addr, &len);
		break;
#endif
#if WITH_IMAGE3
	case IMAGE3_IMAGE_INFO_MAGIC:
		ret_val = image3_load(image, types, count, actual, &addr, &len);
		break;
#endif
	case IMAGE_MEMORY_INFO_MAGIC:
		dprintf(DEBUG_INFO, "memory image\n");
#if WITH_IMAGE4
		/* image4 is forgiving, try it first */
		dprintf(DEBUG_INFO, "trying image4\n");
		ret_val = image4_load(image, types, count, actual, &addr, &len);
		if (ret_val == 0) {
			/* good image4 image */
			break;
		}
		dprintf(DEBUG_INFO, "image4_load failed\n");

		/* restore addr & len for next image format's try */
		addr = *load_addr;
		len = *load_len;
#endif
#if WITH_IMAGE3
		/* image3 is forgiving, try it second */
		dprintf(DEBUG_INFO, "trying image3\n");
		ret_val = image3_load(image, types, count, actual, &addr, &len);
		if (ret_val == 0) {
			/* good image3 image */
			break;
		}
		dprintf(DEBUG_INFO, "image3_load failed\n");

		/* restore addr & len for next image format's try */
		addr = *load_addr;
		len = *load_len;
#endif
#if !(WITH_IMAGE3 || WITH_IMAGE4)
		/* raw image */
		memcpy(addr, image->imagePrivate, len);
		ret_val = 0;
#endif
		break;

	default:
		break;
	}

exit:
	if (ret_val == 0) {
	   	*load_addr = addr;
		*load_len = len;
	} else {
	   	*load_addr = 0;
		*load_len = 0;

		/* any value other than zero is an error, our callers expect -1 */
		ret_val = -1;
	}

	return ret_val;
}

void
image_dump_list(bool detailed)
{
#if WITH_IMAGE4
	image4_dump_list(detailed);
#endif
#if WITH_IMAGE3
	image3_dump_list(detailed);
#endif
}

/*
 * Return a handle to the first instance of an image of (type)
 */
struct image_info *
image_find(u_int32_t type)
{
	struct image_info *image = 0;

#if WITH_IMAGE4
	if (image == 0) image = image4_find(type);
#endif
#if WITH_IMAGE3
	if (image == 0) image = image3_find(type);
#endif

	return image;
}

/*
 * Create an image from a range of memory, rather than finding it on a bdev.
 */
struct image_info *
image_create_from_memory(void *address, size_t length, uint32_t imageOptions)
{
	struct image_info *image = NULL;

	image = malloc(sizeof(struct image_info));

	image->imageLength = length;
	image->imageAllocation = length;
	image->imagePrivateMagic = IMAGE_MEMORY_INFO_MAGIC;
	image->imageOptions = imageOptions;
	image->imagePrivate = address;

	return image;
}

#if WITH_FS
/*
 * Load an image from a file.
 */
int
image_load_file(const char *path, addr_t *address, size_t *length, const u_int32_t *types, u_int32_t count, u_int32_t *actual, u_int32_t options)
{
	struct image_info	*image;
	int			result;

	if (fs_load_file(path, DEFAULT_LOAD_ADDRESS, length))
		return(-1);
	if (NULL == (image = image_create_from_memory((void *)DEFAULT_LOAD_ADDRESS, *length, options))) {
		platform_record_breadcrumb_marker("ICFM-malloc-fail");
		return(-1);
	}
	result = image_load(image, types, count, actual, (void **)address, length);
	image_free(image);
	return(result);
}
#endif

/*
 * Load an image from a memory buffer.
 */
int
image_load_memory(addr_t fromAddress, size_t fromLength, addr_t *address, size_t *length, const u_int32_t *types, u_int32_t count, u_int32_t *actual, u_int32_t options)
{
	struct image_info	*image;
	int			result;

	PROFILE_ENTER('ILM');

	if (NULL == (image = image_create_from_memory((void *)fromAddress, fromLength, options)))
		return(-1);

	result = image_load(image, types, count, actual, (void **)address, length);
	image_free(image);

	PROFILE_EXIT('ILM');

	return(result);
}

/*
 * Find and load an image of the given type from the default image device.
 */
int
image_load_type(addr_t *address, size_t *length, uint32_t type, uint32_t options)
{
	struct image_info	*image;

	if (NULL == (image = image_find(type)))
		return(-1);

	return(image_load(image, &type, 1, NULL, (void **)address, length));
}

/*
 * Free an allocated image.
 */
void
image_free(struct image_info *image)
{
	if ((NULL != image) && (image->imagePrivateMagic == IMAGE_MEMORY_INFO_MAGIC)) {
		free(image);
	}
}
