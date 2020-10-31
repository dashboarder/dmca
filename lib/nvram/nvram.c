/*
 * Copyright (c) 2007-2012 Apple Inc. All rights reserved.
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
#include <list.h>
#include <lib/blockdev.h>
#include <lib/cksum.h>
#include <lib/env.h>
#include <lib/libiboot.h>
#include <lib/nvram.h>
#include <string.h>
#include <sys/menu.h>

/*
 * Regardless of whether it is NOR or NAND providing the backing
 * store, NVRAM_BANK_SIZE controls default size of each nvram bank.
 */
#define NVRAM_BANK_SIZE 0x2000

/*
 * Support for CHRP NVRAM hosting our environment.
 */

/* this is a somewhat arbitrary and historical value */
#define NVRAM_MAX_ENV_PARTITION	0x7f0

/* Panic info partition */
#define NVRAM_PANIC_NAME "APL,OSXPanic"
#define NVRAM_PANIC_NAME_TRUNCATED "APL,OSXPani"
#define NVRAM_PANIC_SIZE 0x800
#define NVRAM_PANIC_SIG 0xa1

struct chrp_nvram_header {
	uint8_t sig;
	uint8_t cksum;
	uint16_t len;
	char    name[12];
	uint8_t data[0];
};

struct apple_nvram_header {
	struct chrp_nvram_header chrp;
	uint32_t adler;
	uint32_t generation;
	uint8_t padding[8];
};

/* in memory representation of a nvram partition */
struct nvram_partition {
	struct list_node node;

	uint8_t chrp_sig;
	size_t len;
	uint8_t *data;

	char name[16];
};

/* in memory representation of a nvram bank (may be multiple ones) */
struct nvram_bank {
	struct list_node node;

	struct list_node part_list;
	struct blockdev *dev;
	off_t  offset;
	size_t len;

	uint32_t generation;
};

static struct list_node nvram_bank_list = LIST_INITIAL_VALUE(nvram_bank_list);
static struct nvram_bank *newest_bank;
static struct nvram_bank *oldest_bank;

static uint8_t chrp_checksum(const struct chrp_nvram_header *hdr)
{
	uint16_t sum;
	const uint8_t *p;

	/* checksum the header (minus the checksum itself) */
	sum = hdr->sig;
	for (p = (const uint8_t *)&hdr->len; p < hdr->data; p++)
		sum += *p;
	while (sum > 0xff)
		sum = (sum & 0xff) + (sum >> 8);
	return sum;
}

static struct nvram_partition *find_part_in_bank(const struct nvram_bank *bank, const char *name)
{
	struct nvram_partition *part;

	list_for_every_entry(&bank->part_list, part, struct nvram_partition, node) {
		if (!strcmp(name, part->name))
			return part;
	}
	return NULL;
}

/* find the oldest and newest banks */
static void sort_bank_age(void)
{
	struct nvram_bank *bank;

	list_for_every_entry(&nvram_bank_list, bank, struct nvram_bank, node) {
		if (!oldest_bank || bank->generation < oldest_bank->generation)
			oldest_bank = bank;
		if (!newest_bank || bank->generation > newest_bank->generation)
			newest_bank = bank;
	}
}

static int load_bank_partitions(struct nvram_bank *bank, uint8_t *buf)
{
	struct chrp_nvram_header *hdr;
	struct nvram_partition *part = NULL;
	size_t offset;

	offset = 0x20;
	while ((offset + sizeof(struct chrp_nvram_header)) <= bank->len) {
		hdr = (struct chrp_nvram_header *)(buf + offset);

#if 0
		dprintf(DEBUG_SPEW, "nvram: load_bank_partitions: looking at header at offset %u\n", offset);
		dhexdump(DEBUG_SPEW, hdr, sizeof(*hdr));
#endif

		/* verify the checksum */
		if (hdr->cksum != chrp_checksum(hdr)) {
#if !NVRAM_NOCHECKSUM
			printf("load_bank_partitions: checksum failure\n");
			goto error_exit;
#endif
		}

		/* is it free space? if so, we're done */
		if (hdr->sig == 0x7f)
			break;

		/* see if the length makes sense */
		if (((hdr->len * 0x10) + offset > bank->len) || (hdr->len < 1)) {
			printf("load_bank_partitions: partition len out of range\n");
			goto error_exit;
		}
	
		/* skip it if it's a known corrupted partition name */
		if (!memcmp(hdr->name, NVRAM_PANIC_NAME_TRUNCATED, 12)) {
			/* The last byte shouldn't have been terminated */
			offset += hdr->len * 0x10;
			/* This partition should disappear when written back */
			continue;
		}

		/* it's good, create a partition */
		part = malloc(sizeof(struct nvram_partition));

		/* note that 0x10 == sizeof(struct chrp_nvram_header) */
		part->chrp_sig = hdr->sig;
		part->len = (hdr->len * 0x10) - 0x10;
		ASSERT(sizeof(part->name) >= sizeof(hdr->name) + 1);
		memcpy(part->name, hdr->name, sizeof(hdr->name));
		part->name[sizeof(hdr->name)] = '\0';
		part->data = malloc(part->len);
		memcpy(part->data, buf + offset + 0x10, part->len);

		dprintf(DEBUG_SPEW, "found partition '%s' at offset %zu, len %zu\n", part->name, offset, part->len);

		/* add it to the bank list */
		list_add_tail(&bank->part_list, &part->node);

		offset += hdr->len * 0x10;
	}

	return 0;

error_exit:
	if (part != NULL) {
		if (part->data != NULL)
			free(part->data);
		
		free(part);
	}

	return -1;
}

static void process_loaded_bank(struct nvram_bank *bank)
{
	struct nvram_partition *part;

	/* look for the environment variable partition */
	part = find_part_in_bank(bank, "common");
	if (part) {

		env_unserialize(part->data, part->len);
	}
}

static int nvram_save_env(struct nvram_bank *bank)
{
	struct nvram_partition *part;
	size_t env_length;

	/* see if there's already a common partition */
	part = find_part_in_bank(bank, "common");
	if (part == NULL) {
		/* create a new one */
		part = calloc(1, sizeof(struct nvram_partition));
		part->chrp_sig = 0x70;
		strlcpy(part->name, "common", sizeof(part->name));
		part->len = 0;
		part->data = NULL;

		list_add_head(&bank->part_list, &part->node);
	}

	/* throw away the old data, we're going to overwrite it */
	if (part->data)
		free(part->data);

	/* allocate a new buffer for this data */
	part->len = NVRAM_MAX_ENV_PARTITION;
	part->data = malloc(part->len);

	/* serialise environment and adjust buffer size to suit */
	env_length = env_serialize(part->data, part->len);
	memset(part->data + env_length, 0, part->len - env_length);

	return 0;
}

static int nvram_prepare_bank(struct nvram_bank *bank, uint8_t *buf)
{
	int err = 0;
	struct nvram_partition *part;
	struct apple_nvram_header *apple_hdr;
	struct chrp_nvram_header *hdr;
	size_t offset;

	/* lay down an apple nvram header */
	apple_hdr = (struct apple_nvram_header *)buf;
	apple_hdr->chrp.sig = 0x5a;
	apple_hdr->chrp.len = 0x2; // 0x20 bytes
	memcpy(apple_hdr->chrp.name, "nvram", sizeof("nvram"));
	apple_hdr->chrp.cksum = chrp_checksum(&apple_hdr->chrp);
	apple_hdr->generation = bank->generation;

	/* iterate through each of the sections in the nvram, laying them out in the buffer */
	offset = 0x20;
	list_for_every_entry(&bank->part_list, part, struct nvram_partition, node) {

		/* partition must have data */
		if (0 == part->len)
			goto fail;
		
		/* make sure there is room to establish a header in the buffer */
		if ((offset + sizeof(struct chrp_nvram_header)) > bank->len)
			goto fail;
		
		hdr = (struct chrp_nvram_header *)&buf[offset];
		hdr->sig = part->chrp_sig;
		hdr->len = ROUNDUP(part->len + 0x10, 0x10) / 0x10;
		memcpy(hdr->name, part->name, sizeof(hdr->name));
		hdr->cksum = chrp_checksum(hdr);

		/* make sure there is room to copy data into the buffer */
		if ((offset + sizeof(struct chrp_nvram_header) + part->len) > bank->len)
			goto fail;

		memcpy(hdr->data, part->data, part->len);

		offset += hdr->len * 0x10;
	}

	/* write out a free space partition to cover the rest of the buffer, if there's free space */
	if ((offset + sizeof(struct chrp_nvram_header)) <= bank->len) {
		hdr = (struct chrp_nvram_header *)&buf[offset];
		hdr->sig = 0x7f;
		hdr->len = (bank->len - offset) / 0x10;
		memset(hdr->name, 0x77, sizeof(hdr->name));
		hdr->cksum = chrp_checksum(hdr);
	}

	/* fill in the adler32 checksum on the apple header */
	apple_hdr->adler = adler32(buf + 0x14, bank->len - 0x14);

out:
	return err;
fail:
	err = -1;
	goto out;
}

static int nvram_write_bank(struct nvram_bank *bank)
{
	int err;
	uint8_t *buf;

	/* allocate a buffer to stage this in */
	buf = calloc(1, bank->len);

	/* prepare buf with bank data */
	err = nvram_prepare_bank(bank, buf);
	if (err) {
		dprintf(DEBUG_CRITICAL, "unable to prepare buffer with nvram bank data\n");
		goto out;
	}

	/* write it out */
	if (bank->dev) {
		dprintf(DEBUG_INFO, "saving nvram contents at bdev '%s', offset 0x%llx, len 0x%zx, gen %d\n", 
				bank->dev->name, bank->offset, bank->len, bank->generation);
		err = blockdev_write(bank->dev, buf, bank->offset, bank->len);
	} else {
		err = 0;
		dprintf(DEBUG_CRITICAL, "nowhere to save environment\n");
	}
#if 0
	printf("would have written this to offset 0x%llx, len %d:\n", bank->offset, bank->len);
	hexdump(buf, bank->len);
#endif
out:
	free(buf);
	return err;
}

int nvram_save(void)
{
	int err;

	/* overwrite the 'oldest' bank (lowest generation count) */
	if (!oldest_bank) {
		printf("nvram_save: no oldest bank previously saved, dropping request\n");
		return 0;
	}

	/* find the next generation number to assign this bank */
	if (newest_bank)
		oldest_bank->generation = newest_bank->generation + 1;
	else
		oldest_bank->generation = 1;

	/* write out our environment */
	nvram_save_env(oldest_bank);

	/* write out the bank */
	err = nvram_write_bank(oldest_bank);
	if (err < 0)
		return err;

	/* we have a new oldest bank, reevaluate */
	sort_bank_age();

	return 0;
}

int nvram_set_panic(const void *panic_data, size_t length)
{
	struct nvram_partition *part;

	/* overwrite the 'oldest' bank (lowest generation count) */
	if (!oldest_bank) {
		printf("nvram_set_panic: no oldest bank previously saved, dropping request\n");
		return 0;
	}

	/* find a panic info partition */
	part = find_part_in_bank(oldest_bank, NVRAM_PANIC_NAME);
	if (part == NULL) {
		/* create a new one */
		part = calloc(1, sizeof(struct nvram_partition));
		part->chrp_sig = NVRAM_PANIC_SIG;
		strlcpy(part->name, NVRAM_PANIC_NAME, sizeof(part->name));
		part->len = NVRAM_PANIC_SIZE;
		part->data = malloc(NVRAM_PANIC_SIZE);

		list_add_head(&oldest_bank->part_list, &part->node);
	}
	if (part->len < 5) {
		/* partition not big enough to store anything */
		return -1;
	}

	/* the first 4 bytes are the panic length. adjust length to fit. */
	if (length + 4 > part->len) length = part->len - 4;
	*(uint32_t *) part->data = length;

	/* copy panic data into the partition. */
	memcpy(part->data + 4, panic_data, length);

	/* zero the remainder of the partition. */
	memset(part->data + 4 + length, 0, part->len - length - 4);

	return 0;
}

static int nvram_load_dev(struct blockdev *dev, off_t offset, size_t banklen, int count)
{
	int err = 0;
	int i;
	uint8_t *buf;
	int banks_found = 0;

	if (offset < 0) {
		offset = dev->total_len + offset;
	}

	buf = malloc(banklen);

	for (i=0; i < count; i++, offset += banklen) {
		struct nvram_bank *bank;
		struct apple_nvram_header *hdr;

		/* read in potential nvram bank */
		if (dev == NULL) {
			dprintf(DEBUG_INFO, "nowhere to load environment from; using transient buffer-only solution\n");
			goto bank_fail;
		}
			
		err = blockdev_read(dev, buf, offset, banklen);
		if (err <= 0) {
			dprintf(DEBUG_INFO, "nvram_load: failed to read potential nvram bank at offset 0x%llx\n", offset);
			continue;
		}

#if 0
		printf("potential nvram bank: \n");
		hexdump(buf, banklen);
#endif

		/* see if it starts with a valid header */
		hdr = (struct apple_nvram_header *)buf;
		if (hdr->chrp.cksum != chrp_checksum(&hdr->chrp)) {
#if !NVRAM_NOCHECKSUM
			dprintf(DEBUG_INFO, "nvram_load: nvram partition at offset 0x%llx failed checksum\n", offset);
			goto bank_fail;
#endif
		}

		/* do an adler32 of the entire buffer */
		uint32_t adler = adler32(buf + 0x14, banklen - 0x14);
		if (adler != hdr->adler) {
#if !NVRAM_NOCHECKSUM
			dprintf(DEBUG_INFO, "nvram_load: nvram bank fails adler32\n");
			goto bank_fail;
#endif
		}

		/* valid header, lets start building a bank structure */
		bank = malloc(sizeof(struct nvram_bank));

		list_initialize(&bank->part_list);
		bank->dev = dev;
		bank->offset = offset;
		bank->len = banklen;
		bank->generation = hdr->generation;

		/* read in the partitions within the bank */
		err = load_bank_partitions(bank, buf);
		if (err < 0) {
			// error parsing partitions, free this bank and continue
			free(bank);
			goto bank_fail;
		}

		/* add the bank to the list */
		list_add_tail(&nvram_bank_list, &bank->node);

		banks_found++;
		continue;

bank_fail:
		/* we failed to load a bank at a nvram slot, allocate a blank bank */
		bank = malloc(sizeof(struct nvram_bank));
		list_initialize(&bank->part_list);
		bank->dev = dev;
		bank->offset = offset;
		bank->len = banklen;
		bank->generation = 0;

		dprintf(DEBUG_INFO, "nvram_load: found invalid or empty bank at 0x%llx, creating blank bank\n", offset);

		/* add the bank to the list */
		banks_found++;
		list_add_tail(&nvram_bank_list, &bank->node);
	}

	dprintf(DEBUG_INFO, "nvram_load: found %d banks of nvram\n", banks_found);

	if (banks_found) {
		/* search through the bank list, sorting by age */
		sort_bank_age();

		/* do any bank processing on the current (newest) bank */
		process_loaded_bank(newest_bank);
	}

	free(buf);
	return banks_found;
}

int nvram_load(void)
{
	int result;
	struct blockdev *bdev;

	dprintf(DEBUG_INFO, "loading nvram\n");

	// Look for a real NVRAM block device
	bdev = lookup_blockdev("nvram");
	if (bdev) {
		result = nvram_load_dev(bdev, 0, NVRAM_BANK_SIZE, 1);
		return result;
	}

	/* Fall back to just creating a temporary nvram not backed by any block device */
	return (nvram_load_dev(NULL, 0, NVRAM_BANK_SIZE, 1) ? 0 : -1);
}

int nvram_update_devicetree(dt_node_t *node, const char *propname)
{
	uint8_t *buf;
	int err = -1;

	if (!oldest_bank) {
		panic("nvram_update_devicetree: no bank previously saved\n");
	} else if (!newest_bank) {
		dprintf(DEBUG_CRITICAL, "nvram_update_devicetree: no content available\n");
	} else {
		buf = malloc(newest_bank->len);
		/* prepare device tree proper buffer with bank data */
		err = nvram_prepare_bank(newest_bank, buf);

		if (err) {
			dprintf(DEBUG_CRITICAL, "nvram_update_devicetree: unable to update devicetree with nvram content\n");
		} else {
			dt_set_prop(node, propname, buf, newest_bank->len);
			dprintf(DEBUG_INFO, "nvram_update_devicetree: updated devicetree with nvram generation %d\n",
				newest_bank->generation);
		}

		free(buf);
	}

	return err;
}
