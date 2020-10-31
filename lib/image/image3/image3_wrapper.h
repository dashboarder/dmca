/*
 * Copyright (C) 2007-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <lib/Image2Format.h>

/*
 * image3_process_superblock
 *
 * Searches the given bdev looking for image3 images.  Returns a count of found
 * images.
 */
extern int
image3_process_superblock(Image2Superblock *sblock, struct blockdev *bdev, off_t offset, uint32_t imageOptions);

/*
 * image3_free_bdev
 *
 * Frees resources for any previously found images on the specified bdev.
 * Useful for starting afresh with a new image superblock.
 */
extern void
image3_free_bdev(struct blockdev *bdev);

/*
 * image3_load
 *
 * Given an image_info handle and type from a previous image_find, load the image and 
 * return the size and address of the payload (DATA tag).
 */
extern int
image3_load(struct image_info *image, const u_int32_t *types, u_int32_t count, u_int32_t *actual, void **load_addr, size_t *load_len);

/*
 * image3_dump_list
 *
 * Print the list of images from the currently active bdev, if any.
 */
extern void
image3_dump_list(bool detailed);


/*
 * image3_find
 *
 * Returns the image_info handle to the first image of the specified type
 * from the currently active bdev, if any.
 */
extern struct image_info *
image3_find(u_int32_t image_type);
