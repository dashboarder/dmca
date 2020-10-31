/*
 * Copyright (c) 2009-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#if WITH_NVRAM

/* ========================================================================== */

#include <platform.h>
#include <debug.h>

#include <lib/blockdev.h>
#include <lib/nvram.h>
#include <lib/env.h>
#include <lib/libc.h>

#include <sys/menu.h>
#include <sys/task.h>

#include <platform/soc/hwclocks.h>
#include <sys/types.h>

#include "nand_part.h"
#include "nand_part_interface.h"
#include "nand_export.h"
#include "nand_nvram_impl.h"
#include "nand_nvram_core.h"
#include "nand_nvram_dump.h"

/* ========================================================================== */

#define DEBUG_NONDESTRUCTIVE 0

/* ========================================================================== */

#define TRACE_ENABLE 0
#if TRACE_ENABLE

// FWiW: tracing is relying on single-threaded behavior for correctness

static utime_t       trace_time = 0;
static unsigned int  trace_line = 0;

#define TRACE_BEFORE()                                                  \
	do {								\
		trace_line = __LINE__;					\
		trace_time = system_time();				\
	} while (0)

#define TRACE_AFTER()							\
	do {								\
		utime_t elapsed = system_time() - trace_time;		\
		dprintf(DEBUG_CRITICAL,					\
			"\nTRACE - %s@%u-%u: %u.%03u ms\n",		\
			__func__,					\
			trace_line,					\
			__LINE__,					\
			elapsed / 1000,					\
			elapsed % 1000);				\
	} while (0)

#else /* !TRACE_ENABLE */

#define TRACE_BEFORE() /* do nothing */
#define TRACE_AFTER() /* do nothing */

#endif

/* ========================================================================== */

typedef struct {

	NandPartInterface * npi;
	uint32_t            part;
	bool                is_initialized;
	nand_nvram_t *      nvram;
	struct blockdev     nvram_bdev;
	
} _context_t;

/* ========================================================================== */

static _context_t _context = { .part = ~0 };


/* ========================================================================== */

static int flash_nand_nvram_read(struct blockdev *bdev, void *buffer, off_t offset, uint64_t length);
static int flash_nand_nvram_read_block(struct blockdev *bdev, void *buffer, block_addr block, uint32_t count);
static int flash_nand_nvram_write(struct blockdev *bdev, const void *buffer, off_t offset, uint64_t length);
static int flash_nand_nvram_write_block(struct blockdev *bdev, const void *buffer, block_addr block, uint32_t count);

extern bool
flash_nand_nvram_init(NandPartInterface *npi, uint32_t part, bool isNewborn)
{
	nand_nvram_geometry_t geometry;
	uint32_t page_size = 0;

	if (_context.is_initialized)
		goto done;

	dprintf(DEBUG_SPEW, "initializing nand nvram...\n");

	TRACE_BEFORE();
	if (isNewborn || (NULL == npi)) {

		dprintf(DEBUG_SPEW, "nand partition table unavailable; returning failure\n");
		goto done;

	} else {

		_context.npi = npi;
		_context.part = part;

		page_size = npi_get_bytes_per_page(npi, part);

		/* specify nand geometry information */
		geometry.bytes_per_meta_buffer = kNAND_NVRAM_META_SIZE;
		geometry.bytes_per_meta_actual = kNAND_NVRAM_META_SIZE;
		geometry.number_of_banks = npi_get_bank_width(npi, part);
		geometry.blocks_per_bank = npi_get_block_depth(npi, part);
		geometry.pages_per_block = npi_get_pages_per_block(npi, part);
		geometry.data_pages_per_block = geometry.pages_per_block - kNAND_NVRAM_PAGE_RESERVED_IN_BLOCK;
		geometry.bytes_per_page = page_size - geometry.bytes_per_meta_buffer;
		
		if (!nand_nvram_open(&_context.nvram, &_context, &geometry, kNAND_NVRAM_SIZE)) {
			dprintf(DEBUG_CRITICAL, "couldn't open nand nvram device\n");
			goto done;
		}

		dprintf(DEBUG_SPEW, "nand nvram initialized\n");
	}

	construct_blockdev(&_context.nvram_bdev, "nvram", kNAND_NVRAM_SIZE, 1);
	_context.nvram_bdev.read_hook = &flash_nand_nvram_read;
	_context.nvram_bdev.read_block_hook = &flash_nand_nvram_read_block;
	_context.nvram_bdev.write_hook = &flash_nand_nvram_write;
	_context.nvram_bdev.write_block_hook = &flash_nand_nvram_write_block;
	register_blockdev(&_context.nvram_bdev);

	_context.is_initialized = true;

done:
	TRACE_AFTER();
	return (_context.is_initialized);
}

static int
flash_nand_nvram_read(struct blockdev *bdev, void *buffer, off_t offset, uint64_t length)
{
	bool ok = true;

	TRACE_BEFORE();
	if (!_context.is_initialized) {
		return 0;
	}

	ok = nand_nvram_read(_context.nvram, offset, buffer, length);
	TRACE_AFTER();

	return (ok ? length : 0);
}

static int
flash_nand_nvram_read_block(struct blockdev *bdev, void *buffer, block_addr block, uint32_t count)
{
	return flash_nand_nvram_read(bdev, buffer, block, count);
}

static int
flash_nand_nvram_write(struct blockdev *bdev, const void *buffer, off_t offset, uint64_t length)
{
	bool ok = true;

	TRACE_BEFORE();
	if (!_context.is_initialized) {
		return 0;
	}

	ok = nand_nvram_write(_context.nvram, offset, buffer, length);
	if (ok)
		ok = nand_nvram_sync(_context.nvram);
	TRACE_AFTER();

	return (ok ? length : 0);
}

static int
flash_nand_nvram_write_block(struct blockdev *bdev, const void *buffer, block_addr block, uint32_t count)
{
	return flash_nand_nvram_write(bdev, buffer, block, count);
}

/* ========================================================================== */

bool 
flash_nand_nvram_is_block_bad(void * context, uint32_t bank, uint32_t block)
{
	const _context_t * ctxt = (_context_t*)context;
	bool bad;

	TRACE_BEFORE();
	bad = npi_is_block_bad(ctxt->npi, ctxt->part, bank, block);
	TRACE_AFTER();

	return (bad);
}

bool 
flash_nand_nvram_read_page(void * context, uint32_t bank, uint32_t page, void * data, void * meta, bool * is_clean)
{
	const _context_t * ctxt = (_context_t*)context;
	const nand_nvram_geometry_t * geo = nand_nvram_get_geometry(ctxt->nvram);
	const uint32_t block = page / geo->pages_per_block;
	const uint32_t page_in_block = page % geo->pages_per_block;
	const uint32_t nand_page_size = npi_get_bytes_per_page(ctxt->npi, ctxt->part);
	UInt8 * buf = NULL;
	bool ok = false;

	TRACE_BEFORE();
	buf = (UInt8*)memalign(nand_page_size, CPU_CACHELINE_SIZE);

	if (npi_read_page(ctxt->npi, ctxt->part, bank, block, page_in_block, buf, is_clean)) {

		memcpy(meta, buf, geo->bytes_per_meta_actual);
		memcpy(data, buf + geo->bytes_per_meta_buffer, geo->bytes_per_page);

		ok = true;
	}

	free(buf);
	TRACE_AFTER();

	return (ok);
}

bool 
flash_nand_nvram_write_page(void * context, uint32_t bank, uint32_t page, void * data, void * meta)
{
	const _context_t * ctxt = (_context_t*)context;
	const nand_nvram_geometry_t * geo = nand_nvram_get_geometry(ctxt->nvram);
	const uint32_t block = page / geo->pages_per_block;
	const uint32_t page_in_block = page % geo->pages_per_block;
	const uint32_t nand_page_size = npi_get_bytes_per_page(ctxt->npi, ctxt->part);
	UInt8 * buf = NULL;
	bool ok = false;

	TRACE_BEFORE();
	buf = (UInt8*)memalign(nand_page_size, CPU_CACHELINE_SIZE);

	memcpy(buf, meta, geo->bytes_per_meta_actual);
	memcpy(buf + geo->bytes_per_meta_buffer, data, geo->bytes_per_page);

	ok = npi_write_page(ctxt->npi, ctxt->part, bank, block, page_in_block, buf);

	free(buf);
	TRACE_AFTER();

	return (ok);
}

bool
flash_nand_nvram_request_ptab_diff(void * context, void * data, uint32_t size)
{
	const _context_t * ctxt = (_context_t*)context;
	bool ok;

	TRACE_BEFORE();
	ok = npi_request_ptab_diff(ctxt->npi, ctxt->part, data, size);
	TRACE_AFTER();

	return (ok);
}

bool
flash_nand_nvram_provide_ptab_diff(void * context, void * data, uint32_t size)
{
	const _context_t * ctxt = (_context_t*)context;
	bool ok;

	TRACE_BEFORE();
	ok = npi_provide_ptab_diff(ctxt->npi, ctxt->part, data, size);
	TRACE_AFTER();

	return (ok);
}

bool 
flash_nand_nvram_erase_block(void * context, uint32_t bank, uint32_t block)
{
	const _context_t * ctxt = (_context_t*)context;
	bool ok;

	TRACE_BEFORE();
	ok = npi_erase_block(ctxt->npi, ctxt->part, bank, block);
	TRACE_AFTER();

	return (ok);
}

bool flash_nand_nvram_copy_ptab(void * context, void * buf)
{
	const _context_t * ctxt = (_context_t*)context;
	bool ok = false;

	TRACE_BEFORE();
	if (NULL != buf) {
		ok = npi_get_ptab_bytes(ctxt->npi, buf, npi_get_bytes_per_page(ctxt->npi, ctxt->part));
	}
	TRACE_AFTER();

	return(ok);
}

bool flash_nand_nvram_read_ptab(void * context, uint32_t bank, uint32_t page, void * buf)
{
	const _context_t * ctxt = (_context_t*)context;
	const nand_nvram_geometry_t * geo = nand_nvram_get_geometry(ctxt->nvram);
	const uint32_t block = page / geo->pages_per_block;
	const uint32_t page_in_block = page % geo->pages_per_block;
	bool is_clean;
	bool ok = false;

	TRACE_BEFORE();
	if (NULL != buf) {
		ok = npi_read_page(ctxt->npi, ctxt->part, bank, block, page_in_block, buf, &is_clean); 
	}
	TRACE_AFTER();

	return (ok);
}

bool flash_nand_nvram_write_ptab(void * context, uint32_t bank, uint32_t page, void * buf)
{
	const _context_t * ctxt = (_context_t*)context;
	const nand_nvram_geometry_t * geo = nand_nvram_get_geometry(ctxt->nvram);
	const uint32_t block = page / geo->pages_per_block;
	const uint32_t page_in_block = page % geo->pages_per_block;
	bool ok = false;

	TRACE_BEFORE();
	if (NULL != buf) {
		ok = npi_write_page(ctxt->npi, ctxt->part, bank, block, page_in_block, buf); 
	}
	TRACE_AFTER();

	return (ok);
}

/* ========================================================================== */

static int do_nvram_dump(int argc, struct cmd_arg *args)
{
	return (nand_nvram_dump(_context.nvram) ? 0 : -1);
}

MENU_COMMAND_DEVELOPMENT(nvramdump, do_nvram_dump, "dump nand-based nvram pages", NULL);

/* ========================================================================== */

#endif // WITH_NVRAM

