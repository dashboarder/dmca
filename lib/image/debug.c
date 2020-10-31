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
#include <sys/boot.h>
#include <sys/menu.h>
#include <lib/image.h>
#include <lib/env.h>
#include <lib/fs.h>
#include <platform/memmap.h>

#if defined(WITH_MENU) && WITH_MENU

static int do_image(int argc, struct cmd_arg *args)
{

	bool detailed;
	int ret_val;

	if (argc < 2) {
		printf("not enough arguments.\n");
		usage:
		printf("%s list [<detailed>]\n", args[0].str);
		printf("%s load [<type>|memory] [<address>]\n", args[0].str);
		return -1;
	}

	if (!strcmp("list", args[1].str)) {
		if (argc > 2) 
			detailed = true;
		else 
			detailed = false;
		image_dump_list(detailed);
	} else if (!strcmp("load", args[1].str)) {
		struct image_info *image;
		addr_t source_address;
		size_t filesize;
		void *addr;
		size_t len = 0xFFFFFFFF;
		u_int32_t type;
		u_int32_t *types = NULL;	/* default to any image type */
		u_int32_t count = 0;

		if (argc < 3) {
			printf("not enough arguments.\n");
			goto usage;
		}

		if (!strcmp(args[2].str, "memory")) {
			/* load image from memory */

			source_address = env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);
			filesize = env_get_uint("filesize", DEFAULT_KERNEL_SIZE);
			
			if ((image = image_create_from_memory((void *)source_address, filesize, 0)) == NULL) {
				printf("image creation failed\n");
				return -1;
			}
		} else {
			/* search for image */
		
			type = htonl(*(uint32_t *)args[2].str);

			image = image_find(type);
			if (!image) {
				printf("could not find image\n");
				return -1;
			}

			/* use the image type since it is known */
			types = &type;
			count = 1;
		}

		if (argc < 4)
			addr = (void *)DEFAULT_KERNEL_ADDRESS;
		else
			addr = (void *)args[3].u;

		ret_val = image_load(image, types, count, NULL, &addr, &len);
		if ( 0 <= ret_val )
			printf("successfully loaded image\n");
		else
			printf("unable to load image\n");
	} else {
		printf("unrecognized command.\n");
		goto usage;
	}

	return 0;
}

MENU_COMMAND_DEBUG(image, do_image, "flash image inspection", NULL);

#if WITH_IMAGE4

static int do_image4_test(int argc, struct cmd_arg *args)
{
	int result = -1, i;
	char file_path[FS_MAXPATHLEN];
	bool file_option = false;
	u_int32_t image_type;
	u_int32_t *image_types = NULL;
	u_int32_t image_types_count = 0;
	u_int32_t options = 0;
	addr_t from_address, to_address = DEFAULT_KERNEL_ADDRESS;
	size_t from_size, to_size = DEFAULT_LOAD_SIZE;
	bool load_image_type = false;

	from_address = env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);
        from_size = env_get_uint("filesize", DEFAULT_LOAD_SIZE);

	for (i = 1; i < argc; i++) {
		if (strcmp(args[i].str, "-options") == 0) {
			options = args[++i].u;
		}
		else if (strcmp(args[i].str, "-type") == 0) {
			image_type = htonl(*(uint32_t *)args[++i].str);
			image_types = &image_type;
			image_types_count = 1;
		}
		else if (strcmp(args[i].str, "-file") == 0) {
			snprintf(file_path, FS_MAXPATHLEN, "/boot%s", args[++i].str);
			file_option = true;
		}
		else if (strcmp(args[i].str, "-addr") == 0) {
			from_address = args[++i].u;
		}
		else if (strcmp(args[i].str, "-size") == 0) {
			from_size = args[++i].u;
		}
		else if (strcmp(args[i].str, "-load-type") == 0) {
			load_image_type = true;
		}
		else {
			goto print_usage;
		}
	}

	if (file_option) {
#if WITH_FS		
		result = mount_bootfs();
		if (result == 0) {
			result = image_load_file(file_path, &to_address, &to_size, image_types, image_types_count, NULL, options);
			fs_unmount("/boot");
		}
		else {
			dprintf(DEBUG_INFO, "root filesystem mount failed\n");
		}
#else
		dprintf(DEBUG_INFO, "file-system support not available\n");
#endif
	}
	else if (load_image_type) {
		if (image_types_count != 0)
			result = image_load_type(&to_address, &to_size, image_type, options);
		else 
			dprintf(DEBUG_INFO, "image type not specified\n");
	}
	else {
		result = image_load_memory(from_address, from_size, &to_address, &to_size, image_types, image_types_count, NULL, options);
	}

	if (result == 0)
		dprintf(DEBUG_INFO, "image_load test succeed\n");
	else
		dprintf(DEBUG_INFO, "image_load test failed, result:%d\n", result);

	security_restore_environment();

	return result;

print_usage:
	dprintf(DEBUG_INFO, "image4_test [-file <file_path>] [-options <options>] [-type <image_type>] [-load-type] [-addr <address>] [-size <size>] [-help]\n");
	dprintf(DEBUG_INFO, "\t- address defaults to $loadadr and size defaults to $filesize\n");
	dprintf(DEBUG_INFO, "\t- load-type option loads image of type specified with '-type' option from nand_firmare partition\n");
	dprintf(DEBUG_INFO, "\t- list option list images stored in nand_firmare partition\n");
	dprintf(DEBUG_INFO, "\t- various options -> require_trust:(1<<1), local_storage:(1<<3), new_trust_chain:(1<<4) \n");
	dprintf(DEBUG_INFO, "\t- various types -> dtre, rdtr, krnl, rkrn, ibss, ibec, illb, ibot, rdsk, logo, rlgo, recm, diag, 0 (any)  \n");
	return -1;
}

MENU_COMMAND_DEBUG(image4_test, do_image4_test, "load and validate image4", NULL);

#endif

#endif
