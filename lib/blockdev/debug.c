/*
 * Copyright (C) 2007-2011, 2013-2016 Apple Inc. All rights reserved.
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
#include <lib/blockdev.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <sys/menu.h>
#include <drivers/usb/usb_debug.h>

#define USB_TRANSFER_BLOCKS (1000)

static int do_slurp (struct blockdev *dev, char *filename, const size_t block_offset) {
	int err = 0;
	off_t lba;
	off_t end;
	size_t buf_offset;
	const size_t buf_size = dev->block_size * USB_TRANSFER_BLOCKS;
	uint8_t * read_buf;
	uint8_t * base_buf;
	uintptr_t buf_physical;


	end = dev->total_len >> dev->block_shift;	
	base_buf = malloc(buf_size);
	if (NULL == base_buf) {
		printf ("malloc failed\n");
		return -1;
	}
	buf_physical = mem_static_map_physical((uintptr_t)base_buf);

	printf("slurping lba %zu to %llu\n", block_offset, end);

	buf_offset = 0;
	lba = block_offset;

	while (lba < end) {
		read_buf = base_buf + buf_offset;
		*(UInt32*)read_buf = 0xa5a5a5a5; /* poison buffer */
		err = blockdev_read(dev, read_buf, lba << dev->block_shift, dev->block_size);
		if (err < 0){
			printf("blockdev_read returns %d on lba %llu\n\n", err, lba);
			// don't stop slurping
		}
		buf_offset += dev->block_size;
		++lba;
		if (buf_offset == buf_size) {
			/* 
			 * This issues writes over the USB pipe for USB_TRANSFER_BLOCKS ' worth of
			 * device blocks. This could be (4096) * 1000 or (8192) * 1000 depending
			 * on device block size.
			 */
			err = usb_send_data_to_file(filename, buf_offset, buf_physical, 0);
			if (err) {
				printf("USB transfer error %d\n", err);
				goto out;
			}
			buf_offset = 0;
			printf("Completed to block offset %llu\r", lba);
		}
	}
	
	/* Send over the remainder of blocks.. */
	if (buf_offset > 0) {
		err = usb_send_data_to_file(filename, buf_offset, buf_physical, 0);
		if (err) {
			printf("USB transfer error %d\n", err);
			goto out;
		}
	}

out:
	printf("Completed to block offset %llu with code %d\n", lba, err);

	free(base_buf);
	return err;
}


int do_blockdev(int argc, struct cmd_arg *args)
{
	if (!security_allow_modes(kSecurityModeDebugCmd)) {
		printf("Permission Denied\n");
		return -1;
	}

	if (argc < 2) {
		printf("not enough arguments\n");
usage:
		printf("usage:\n");
		printf("%s list\n", args[0].str);
		printf("%s read <dev> <offset> <len> [<address>]\n", args[0].str);
		printf("%s scan <dev> <lba> <num>\n", args[0].str);
		printf("%s slurp <dev> <file> \n", args[0].str);
		printf("%s write <dev> <offset> [<len>] [<address>]\n", args[0].str);
		printf("%s memcmp <dev> <offset> [<len>] [<address>]\n", args[0].str);
		printf("%s erase <dev> <offset> <len>\n", args[0].str);
		return -1;
	}

	if (!strcmp(args[1].str, "list")) {
		struct blockdev *dev;

		printf("block device list:\n");
		for (dev = first_blockdev(); dev; dev = next_blockdev(dev)) {
			printf("\tdevice '%s': size %lld, block_size %d\n", dev->name, dev->total_len, dev->block_size);
		}	
	} else if (!strcmp(args[1].str, "read") || !strcmp(args[1].str, "write") || !strcmp(args[1].str, "memcmp")) {
		if (argc < 4) {
			printf("not enough arguments.\n");
			goto usage;
		}

		off_t offset = args[3].u;
		size_t len;
		addr_t addr;
		int err;
		struct blockdev *dev;

		if (argc >= 5) {
			len = args[4].u;
		} else {
			len = env_get_uint("filesize", 0);
			if (len == 0) {
				printf("filesize variable invalid or not set, aborting\n");
				return -1;
			}
		}

		if (argc >= 6) {
			addr = args[5].u;
		} else {
			addr = env_get_uint("loadaddr", mib_get_addr(kMIBTargetDefaultLoadAddress));
		}

		if (!security_allow_memory((void *)addr, len)) {
			printf("Permission Denied\n");
			return -1;
		}

		dev = lookup_blockdev(args[2].str);
		if (!dev) {
			printf("couldn't find block device '%s'\n", args[2].str);
			return -1;
		}

		if (!strcmp(args[1].str, "read")) {
			printf("reading %zu bytes from device '%s' to address 0x%llx\n", len, args[2].str, (uint64_t)addr);
			err = blockdev_read(dev, (void *)addr, offset, len);
			printf("blockdev_read returns %d\n", err);
		} else if (!strcmp(args[1].str, "write")) {
			printf("writing %zu bytes to device '%s' from address 0x%llx\n", len, args[2].str, (uint64_t)addr);
			err = blockdev_write(dev, (void *)addr, offset, len);
			printf("blockdev_write returns %d\n", err);
		} else {
			printf("comparing %zu bytes from device '%s' to address 0x%llx\n", len, args[2].str, (uint64_t)addr);
			err = blockdev_compare(dev, (void *)addr, offset, len);
			printf("blockdev_compare returns %d\n", err);
		}
	} else if (!strcmp(args[1].str, "erase")) {
		if (argc < 5) {
			printf("not enough arguments.\n");
			goto usage;
		}

		off_t offset = args[3].u;
		size_t len = args[4].u;
		int err;
		struct blockdev *dev;
	   
		dev = lookup_blockdev(args[2].str);
		if (!dev) {
			printf("couldn't find block device '%s'\n", args[2].str);
			return -1;
		}

		err = blockdev_erase(dev, offset, len);
		printf("blockdev_erase returns %d\n", err);
	} else if (!strcmp(args[1].str, "scan")) {
		if (argc < 5) {
			printf("not enough arguments.\n");
			goto usage;
		}

		off_t lba = (off_t)args[3].u;
		size_t len = args[4].u;
		int err;
		struct blockdev *dev;
		off_t end;
		uint8_t *buffer;

		dev = lookup_blockdev(args[2].str);
		if (!dev) {
			printf("couldn't find block device '%s'\n", args[2].str);
			return -1;
		}

		if (!len || ((off_t)(lba + len) > (off_t)(dev->total_len >> dev->block_shift))){
			end = dev->total_len >> dev->block_shift;
		}else{
			end = lba + len;
		}

		buffer = malloc(dev->block_size);

		printf("scanning lba %llu to %llu\n", lba, end);
		while (lba < end)
		{
			*buffer = 0xa5;
			err = blockdev_read(dev, buffer, lba << dev->block_shift, dev->block_size);
			if (err < 0){
				printf("blockdev_read returns %d on lba %llu\n", err, lba);
				free(buffer);
				return 1;
			}
			if (lba % 1000 == 0)
				printf("lba %zu\r", (size_t)lba);
			++lba;
		}
		free(buffer);
		printf("blockdev scan completed with no errors\n");
	} 
	else if (!strcmp(args[1].str, "slurp")) {
		/* Is it a slurp? */
		if (argc < 3) {
			/* bdev slurp device filename */
			printf("slurp invalid arguments \n");
			goto usage;
		}

		struct blockdev *dev;
		size_t block_offset;

		dev = lookup_blockdev(args[2].str);
		block_offset = 0;

		if (!dev) {
			printf("could not find blk dev ' %s ' \n", args[2].str);
			goto usage;
		}

		return do_slurp (dev, args[3].str, block_offset);

	}
	else {
		printf("unrecognized subcommand.\n");
		goto usage;
	}

	return 0;
}

