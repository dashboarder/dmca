/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef IMAGE4_WRAPPER_H

/*
 * image4_process_superblock
 *
 * Searches the given bdev looking for image4 images.  Returns a count of found
 * images.
 */
extern int
image4_process_superblock(void *sblock, struct blockdev *bdev, off_t offset, uint32_t imageOptions);

/*
 * image4_free_bdev
 *
 * Frees resources for any previously found images on the specified bdev.
 * Useful for starting afresh with a new image superblock.
 */
extern void
image4_free_bdev(struct blockdev *bdev);

/*
 * image4_load
 *
 * Given an image_info handle and type from a previous image_find, load the image and 
 * return the size and address of the payload (DATA tag).
 */
extern int
image4_load(struct image_info *image, const u_int32_t *types, u_int32_t count, u_int32_t *actual, void **load_addr, size_t *load_len);

/*
 * image4_dump_list
 *
 * Print the list of images from the currently active bdev, if any.
 */
extern void
image4_dump_list(bool detailed);


/*
 * image4_find
 *
 * Returns the image_info handle to the first image of the specified type
 * from the currently active bdev, if any.
 */
extern struct image_info *
image4_find(u_int32_t image_type);

#endif // defined(IMAGE4_WRAPPER_H)
