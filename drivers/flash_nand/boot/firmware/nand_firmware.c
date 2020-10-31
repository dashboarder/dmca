/*
 * Copyright (c) 2008-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#if WITH_BLOCKDEV

#include <debug.h>
#include <sys/types.h>
#include <lib/blockdev.h>

#include "nand_part.h"
#include "nand_part_interface.h"
#include "nand_export.h"
#include "nand_firmware.h"

//#undef DEBUG_LEVEL
//#define DEBUG_LEVEL DEBUG_INFO

/* ========================================================================== */

#define kNandFirmwareBDevName "nand_firmware"
#define kNandLLBBDevName "nand_llb"

/* ========================================================================== */

static NandPartInterface * _npi;
static uint32_t _llb_index;
static uint32_t _firmware_index;
static bool _have_cache;
static uint32_t _cached_part;
static uint32_t _cached_block;
static uint32_t _cached_page;
static uint8_t * _page_cache;

/* ========================================================================== */

static bool nand_firmware_cache_page(uint32_t part, uint32_t block, uint32_t page);
static int nand_firmware_read_block_handler(struct blockdev *bdev, void *ptr, block_addr block, uint32_t count);
#if ALLOW_NAND_FIRMWARE_ERASE
static int nand_firmware_erase_block_handler(struct blockdev *bdev, off_t offset, uint64_t len);
#endif


/* ========================================================================== */

bool
nand_firmware_init(NandPartInterface *npi, uint32_t partIndex)
{
	const uint32_t page_size = npi_get_bytes_per_page(npi, partIndex);
	struct blockdev * _bdev;
	bool ok = false;

	_npi = npi;
	_have_cache = false;
	_cached_part = 0;
	_cached_block = 0;
	_cached_page = 0;
	_page_cache = NULL;
	_bdev = NULL;

	_bdev = malloc(sizeof(struct blockdev));
	_page_cache = (uint8_t*)memalign(page_size, CPU_CACHELINE_SIZE);
	const uint32_t blockCount = npi_get_block_depth(_npi, partIndex);
	const uint32_t pagesPerBlock = npi_get_pages_per_block(_npi, partIndex);
	const off_t len = (blockCount * pagesPerBlock * page_size);
	const NandPartEntry * entry = npi_get_entry(_npi, partIndex);

	switch (entry->content) {
	case kNandPartContentFirmware:
		_firmware_index = partIndex;
		construct_blockdev(_bdev, kNandFirmwareBDevName, len, kNandPartSectorSize);
		break;
	case kNandPartContentBootBlock:
		_llb_index = partIndex;
		construct_blockdev(_bdev, kNandLLBBDevName, len, kNandPartSectorSize);
		break;
	default:
		panic("Should never have been here\n");
	}

	_bdev->read_block_hook = &nand_firmware_read_block_handler;
#if ALLOW_NAND_FIRMWARE_ERASE
	_bdev->erase_hook = &nand_firmware_erase_block_handler;
#endif
	register_blockdev(_bdev);

	ok = true;
	return(ok);
}

static bool
nand_firmware_cache_page(uint32_t part, uint32_t block, uint32_t page)
{
	bool is_clean;
	uint32_t bank;

	if (!_have_cache || (_cached_part != part) || (_cached_block != block) || (_cached_page != page)) {
		_have_cache = false;
		for (bank = 0; bank < npi_get_bank_width(_npi, part); bank++) {
			if (npi_is_block_bad(_npi, part, bank, block)) {
				dprintf(DEBUG_SPEW,	
					"WARNING: bad block (%d:%d:%d)\n", 
					part, bank, block);
				continue;
			}
			if (npi_read_page(_npi, part, bank, block, page, _page_cache, &is_clean)) {
				_have_cache = true;
				_cached_part = part;
				_cached_block = block;
				_cached_page = page;
				break;
			} else {
				dprintf(DEBUG_SPEW,	
					"WARNING: page read (%d:%d:%d:%d) failed\n", 
					part, bank, block, page);
			}
		}
	}

	return(_have_cache);
}

static int 
nand_firmware_read_block_handler(struct blockdev *bdev, void *ptr, block_addr sector_addr, uint32_t sector_count)
{
	// this is a bit confusing, since a bdev "block" is not the same as a nand "block";
	// instead, the bdev "block" concept is mapped to a 512-byte virtual "sector"

	const uint32_t sector_base = 0;
	uint32_t sector_limit = 0;

	uint32_t block_count;
	uint32_t bytes_per_page;
	uint32_t pages_per_block;
	uint32_t bytes_per_block;
	uint32_t sectors_per_block;
	uint32_t sectors_per_page;

	uint32_t first_sector = 0;
	uint32_t last_sector = 0;

	uint32_t sector_idx;
	uint8_t * cursor = (uint8_t *)ptr;
	uint32_t sectors_read = 0;

	uint32_t part;

	if (strcmp(bdev->name, kNandFirmwareBDevName) == 0) {
		part = _firmware_index;
	} else if (strcmp(bdev->name, kNandLLBBDevName) == 0) {
		part = _llb_index;
	} else {
		/* Something wrong */
		dprintf(DEBUG_CRITICAL,
			"ERROR: bad bdev: %s\n",
			bdev->name);
		return(-1);
	}

	block_count = npi_get_block_depth(_npi, part);
	bytes_per_page = npi_get_bytes_per_page(_npi, part);
	pages_per_block = npi_get_pages_per_block(_npi, part);
	bytes_per_block = (bytes_per_page * pages_per_block);
	sectors_per_block = (bytes_per_block / kNandPartSectorSize);
	sectors_per_page = (bytes_per_page / kNandPartSectorSize);

	sector_limit = (block_count * sectors_per_block);

	first_sector = (sector_base + sector_addr);
	last_sector = (first_sector + sector_count - 1);

	if (last_sector > sector_limit) {
		dprintf(DEBUG_CRITICAL, 
			"ERROR: 0x%08X-0x%08X not within 0x%08X-0x%08X\n",
			first_sector, last_sector, sector_base, sector_limit);
		return(-1);
	}

	for (sector_idx = first_sector; sector_idx <= last_sector; sector_idx++) {
		const uint32_t block = sector_idx / sectors_per_block;
		const uint32_t sector_in_block = sector_idx % sectors_per_block;
		const uint32_t page = sector_in_block / sectors_per_page;
		const uint32_t offset = sector_in_block % sectors_per_page;
		if (!nand_firmware_cache_page(part, block, page)) {
			dprintf(DEBUG_SPEW,	
				"WARNING: unable to read firmware page 0x%X\n", 
				page);
			break;;
		}
		memcpy(cursor, &_page_cache[offset * kNandPartSectorSize], kNandPartSectorSize);
		sectors_read++;
		cursor += kNandPartSectorSize;
	}

	return(sectors_read);
}

#if ALLOW_NAND_FIRMWARE_ERASE
static int
nand_firmware_erase_block_handler(struct blockdev *bdev, off_t offset, uint64_t len)
{
	uint32_t block_base = 0;
	uint32_t block_num = 0;
	bool llb_nuke = false;
	uint32_t pages_per_block;
	uint32_t bytes_per_page;
	uint32_t bytes_per_block;
	uint32_t part;

	if (strcmp(bdev->name, kNandFirmwareBDevName) == 0) {
		part = _firmware_index;
	} else if (strcmp(bdev->name, kNandLLBBDevName) == 0) {
		part = _llb_index;
		llb_nuke = true;
	} else {
		/* Something wrong */
		dprintf(DEBUG_CRITICAL,
			"ERROR: bad bdev: %s\n",
			bdev->name);
		return(-1);
	}

	pages_per_block = npi_get_pages_per_block(_npi, part);
	bytes_per_page = npi_get_bytes_per_page(_npi, part);
	bytes_per_block = (pages_per_block * bytes_per_page);

	/* Sanity check offset and len before erase, need to be page-size aligned */
	ASSERT(offset >= 0);
	if ((uint64_t)offset >= bdev->total_len)
		return 0;
	if ((offset + len) > bdev->total_len)
		len = bdev->total_len - offset;
	if ((offset % bytes_per_block) || (len % bytes_per_block))
		return 0;

	block_num = (len / bytes_per_block);
	block_base = (offset / bytes_per_block);

	for (UInt16 bank = 0; bank < npi_get_bank_width(_npi, part); bank++) {
		/* Read the partition table in the case if LLB erase */
		if (llb_nuke) {
			npi_erase_boot_partition(_npi);
		} else {
			for (UInt16 block = block_base; block < (block_base + block_num); block++) {
				if (!npi_erase_block(_npi, part, bank, block)) { 
					dprintf(DEBUG_SPEW,
						"WARNING: Erase(%d:%d:%d) failed\n", part, bank, block);
				}
			}
		}
	}
	return(block_num * bytes_per_block);
}
#endif

#endif /* WITH_BLOCKDEV */

