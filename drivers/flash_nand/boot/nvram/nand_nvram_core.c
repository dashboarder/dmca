/*
 * Copyright (c) 2008-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "nand_part.h"
#include "nand_nvram_platform.h"
#include "nand_nvram_impl.h"
#include "nand_nvram_core.h"

/* ========================================================================== */

/*!
  @group private platform-independent type declarations
*/

typedef enum _log_level log_level_t;

/* ========================================================================== */

/*!
  @group private platform-independent constant definitions
*/

/*!
  @const kNAND_NVRAM_MUTE

  @abstract Flag used to eliminate all output.	Note that you probably
  don't want to use this normally and should instead adjust
  kNAND_NVRAM_LOG_LEVEL below.
*/
#define kNAND_NVRAM_MUTE 0

/*!
  @const kNAND_NVRAM_LOG_LEVEL

  @abstract Default cutoff level for debug output.
*/
#define kNAND_NVRAM_LOG_LEVEL LOG_LEVEL_INFO

/* ========================================================================== */

/*!
  @group private platform-independent enumerations
*/

enum _log_level
{
	LOG_LEVEL_NONE,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_SPEW,
	LOG_LEVEL_ALL
};

/* ========================================================================== */

/*!
  @group private platform-independent macro definitions
*/

#if kNAND_NVRAM_MUTE

#define log_printf(nv, lvl, ...)

#define dump(nv, lvl, indent, ...)

#define nand_nvram_fail(nv, val)		\
	do {					\
		val = false;			\
	} while (0)

#else /* kNAND_NVRAM_MUTE */

#define log_printf(nv, lvl, ...)					\
	do {								\
		if ((lvl) <= kNAND_NVRAM_LOG_LEVEL)			\
			nand_nvram_printf(nv->context, __VA_ARGS__);	\
	} while (0)

#define dump(nv, lvl, indent, ...)				\
	do {							\
		int _idx;					\
		for (_idx = 0; _idx < (int)(indent); _idx++)	\
			log_printf((nv), (lvl), "%20s", "");	\
		log_printf((nv), (lvl), __VA_ARGS__);		\
	} while (0)

#define nand_nvram_fail(nv, val)				\
	do {							\
		log_printf((nv), LOG_LEVEL_ERROR,		\
			   "[NAND_NVRAM] FAIL -> %s@%d\n",	\
			   __FILE__, __LINE__);			\
		val = false;					\
	} while (0)

#endif /* kNAND_NVRAM_MUTE */
			
/* ========================================================================== */

/*!
  @group static wrappers around system portability hooks
*/

static inline void * alloc_mem(nand_nvram_t * nv, uint32_t size);
static inline signed compare_mem(nand_nvram_t * nv, void * left, void * right, uint32_t size);
static inline void copy_mem(nand_nvram_t * nv, void * dest, const void * src, uint32_t size);
static inline void set_mem(nand_nvram_t * nv, void * mem, uint8_t value, uint32_t size);
static inline void free_mem(nand_nvram_t * nv, void * mem, uint32_t size);
static inline bool is_block_bad(nand_nvram_t * nv, uint32_t bank, uint32_t block);
static inline bool read_page(nand_nvram_t * nv, uint32_t bank, uint32_t page, void * data, void * meta, bool * is_clean);
static inline bool write_page(nand_nvram_t * nv, uint32_t bank, uint32_t page, void * data, void * meta);
static inline bool erase_block(nand_nvram_t * nv, uint32_t bank, uint32_t block);
static inline bool request_ptab_diff(nand_nvram_t * nv, void * data, uint32_t size);
static inline bool provide_ptab_diff(nand_nvram_t * nv, void * data, uint32_t size);
static inline bool copy_ptab(nand_nvram_t * nv, void * buf);
static inline bool read_ptab(nand_nvram_t * nv, uint32_t bank, uint32_t page, void * buf);
static inline bool write_ptab(nand_nvram_t * nv, uint32_t bank, uint32_t page, void * buf);

/* ========================================================================== */

/*!
  @group implementation functions used to commit of nvram data to nand store
*/

/*!
  @function commit_shadow

  @abstract Commit new generation of shadowed nvram area from
  ram buffers to nand array.

  @result Returns bool indicating whether function succeeded committing at generation to at least one bank.
*/
static bool commit_shadow(nand_nvram_t * nv);

/*!
  @function commit_to_bank

  @abstract Commit new generation to specified bank.

  @param bank

  @result Returns bool indicating whether function succeeded committing generation to bank.
*/
static bool commit_to_bank(nand_nvram_t * nv, uint32_t bank, blob_t * scratch);

/*!
  @function prepare_next_commit_block

  @abstract Find and prepare next block in a bank for generation commit.

  @param bank Bank to prepare.

  @param block In/out reference to current commit block and, upon return, to newly prepared block.

  @param attempts In/out reference used by caller to accumulate total block per bank commit attempts.

  @result Returns bool indicating whether function succeeded in preparing a commit block.
*/
static bool prepare_next_commit_block(nand_nvram_t * nv, uint32_t bank, uint32_t * block, uint32_t * attempts);

/*!
  @function commit_to_block_in_bank

  @abstract Attempt to commit generation to specific bank in block starting at the specified page offset in the block.

  @param bank

  @param block

  @param start_page_in_block

  @result Returns bool indicating whether function succeeded committing to specified block in specified bank.
*/
static bool commit_to_block_in_bank(nand_nvram_t * nv, uint32_t bank, uint32_t block, uint32_t start_page_in_block, blob_t * scratch);

static void increment_generation(nand_nvram_t * nv);

/* ========================================================================== */

/*!
  @group implementation methods used to restore nvram data from nand store
*/

/*!
  @function restore_shadow

  @abstract Restore youngest generation of nvram data from nand array.

  @result Returns bool indicating whether function succeeded.
*/
static bool restore_shadow(nand_nvram_t * nv);

/*!
  @function init_meta
*/
static void init_meta(nand_nvram_t * nv, meta_t * meta, uint32_t generation, uint32_t page_count, uint32_t page_idx);

/*!
  @function validate_meta

  @abstract Check that individual meta data struct is
  internally self-consistent.

  @param page_meta Pointer to page meta struct.

  @result Returns bool indicating whether function succeeded.
*/
static bool validate_meta(nand_nvram_t * nv, meta_t * page_meta, uint32_t idx);

/*!
  @function calc_page_count_from_size

  @abstract Calculate number of pages required to contain
  given size of data.

  @param size Size, in bytes, of data to be stored in nand pages.

  @result Returns number of pages.
*/
static uint32_t calc_page_count_from_size(nand_nvram_t * nv, uint32_t size);

static uint32_t calc_extra_offset(nand_nvram_t * nv);
static uint32_t calc_extra_size(nand_nvram_t * nv);

/*!
  @function calc_block_from_page

  @abstract Calculate block offset within bank from page number.

  @param page_num Zero-based page number.

  @result Returns block offset within bank.
*/
static uint32_t calc_block_from_page(nand_nvram_t * nv, uint32_t page_num);

/*!
  @function calc_page_in_block_from_page

  @abstract Calculate page offset within block from page number.

  @param page_num Zero-based page number.

  @result Returns page offset within block.
*/
static uint32_t calc_page_in_block_from_page(nand_nvram_t * nv, uint32_t page_num);

/*!
  @function calc_base_page_from_block

  @abstract Calculate page number of base page in block given
  block number.

  @param block_num Zero-based block number.

  @result Returns page number.
*/
static uint32_t calc_base_page_from_block(nand_nvram_t * nv, uint32_t block_num);

/*!
  @function get_blob_page_data

  @abstract Return pointer from within data buffer to indexed
  page's data bytes.

  @param page_index Index of page within shadow for which
  reference should be returned.

  @result Returns pointer to data bytes within blob page.
*/
static uint8_t * get_blob_page_data(nand_nvram_t * nv, blob_t * blob, uint32_t page_index);

/*!
  @function get_blob_page_meta

  @abstract Return pointer from within meta buffer to indexed
  page's meta struct.

  @param page_index Index of page within shadow for which
  reference should be returned.

  @result Return pointer to meta bytes within blob page.
*/
static meta_t * get_blob_page_meta(nand_nvram_t * nv, blob_t * blob, uint32_t page_index);

/*!
  @function get_blob_generation
*/
static uint32_t get_blob_generation(nand_nvram_t * nv, blob_t * blob);

/*!
  @function get_blob_page_count
*/
static uint32_t get_blob_page_count(nand_nvram_t * nv, blob_t * blob);

/*!
  @function alloc_blob
*/
static bool alloc_blob(nand_nvram_t * nv, blob_t ** blob_ref, uint32_t page_count);

/*!
  @function free_blob
*/
static void free_blob(nand_nvram_t * nv, blob_t ** blob_ref, uint32_t page_count);

/*!
  @function read_blob
*/
static bool read_blob(nand_nvram_t * nv, blob_t * blob, uint32_t bank, uint32_t block, uint32_t base, uint32_t offset);

/*!
  @function alloc_page_bufs
*/
static bool alloc_page_bufs(nand_nvram_t * nv, uint32_t page_count, void ** meta_bufs_ref, void ** data_bufs_ref);

/*!
  @function free_page_bufs
*/
static void free_page_bufs(nand_nvram_t * nv, uint32_t page_count, void ** meta_bufs_ref, void ** data_bufs_ref);

static uint32_t calc_watermark_index(nand_nvram_t * nv, uint32_t bank, uint32_t block);
static uint32_t get_watermark(nand_nvram_t * nv, uint32_t bank, uint32_t block);
static void update_watermark(nand_nvram_t * nv, uint32_t bank, uint32_t block, uint32_t new_watermark);
static void clear_watermark(nand_nvram_t * nv, uint32_t bank, uint32_t block);


/* ========================================================================== */

static bool is_block_closed(nand_nvram_t * nv, uint32_t bank, uint32_t block);
static bool close_block(nand_nvram_t * nv, uint32_t bank, uint32_t block, uint32_t offset);
static bool fill_pages_with_ptab(nand_nvram_t * nv, uint32_t bank, uint32_t block, uint32_t offset, uint32_t count);
static bool beget_newborn(nand_nvram_t * nv);
static void migrate_forward(nand_nvram_t * nv);
static bool find_youngest(nand_nvram_t * nv, uint32_t bank, uint32_t block, uint32_t* ancestor, uint32_t* youngest);

/* ========================================================================== */

#if kNAND_NVRAM_MUTE

#define dump_geometry(nv, lvl, indent, geo)
#define dump_meta(nv, lvl, indent, meta)
#define dump_data(nv, lvl, indent, data)
#define dump_blob(nv, lvl, indent, blob, page_count)
#define dump_nvram(nv, lvl, indent)

#else

static void _dump_geometry(nand_nvram_t * nv, uint32_t lvl, uint32_t indent, nand_nvram_geometry_t * geo);
static void _dump_meta(nand_nvram_t * nv, uint32_t lvl, uint32_t indent, meta_t * meta);
static void _dump_data(nand_nvram_t * nv, uint32_t lvl, uint32_t indent, uint8_t * data);
static void _dump_blob(nand_nvram_t * nv, uint32_t lvl, uint32_t indent, blob_t * blob, uint32_t page_count);
static void _dump_nvram(nand_nvram_t * nv, uint32_t lvl, uint32_t indent);

#define dump_geometry(nv, lvl, indent, geo)			\
	do {							\
		if ((lvl) <= kNAND_NVRAM_LOG_LEVEL)		\
			_dump_geometry(nv, lvl, indent, geo);	\
	} while (0)

#define dump_meta(nv, lvl, indent, meta)			\
	do {							\
		if ((lvl) <= kNAND_NVRAM_LOG_LEVEL)		\
			_dump_meta(nv, lvl, indent, meta);	\
	} while (0)

#define dump_data(nv, lvl, indent, data)			\
	do {							\
		if ((lvl) <= kNAND_NVRAM_LOG_LEVEL)		\
			_dump_data(nv, lvl, indent, data);	\
	} while (0)

#define dump_blob(nv, lvl, indent, blob, page_count)			\
	do {								\
		if ((lvl) <= kNAND_NVRAM_LOG_LEVEL)			\
			_dump_blob(nv, lvl, indent, blob, page_count);	\
	} while (0)

#define dump_nvram(nv, lvl, indent)				\
	do {							\
		if ((lvl) <= kNAND_NVRAM_LOG_LEVEL)		\
			_dump_nvram(nv, lvl, indent);		\
	} while (0)

#endif

/* ========================================================================== */

bool
nand_nvram_open(nand_nvram_t ** nvram, void * context, nand_nvram_geometry_t * geometry, uint32_t size)
{
	bool ret = true;
	bool is_opened = false;

	const uint32_t watermarks_size = geometry->number_of_banks * geometry->blocks_per_bank * sizeof(int32_t);
	const uint32_t cursors_size = geometry->number_of_banks * sizeof(uint32_t);
	const uint32_t copies_size = geometry->number_of_banks * sizeof(bool);

	nand_nvram_t * nv;

	if (NULL == (nv = (nand_nvram_t *) nand_nvram_alloc_mem(context, sizeof(nand_nvram_t)))) {

		ret = false;

	} else {

		*nvram = nv;
		
		set_mem(nv, nv, 0, sizeof(nand_nvram_t));
		
		nv->context = context;
		nv->size = size;
		nv->is_dirty = false;
		nv->is_born = false;
		copy_mem(nv, &nv->geo, geometry, sizeof(nand_nvram_geometry_t));

		if (sizeof(meta_t) > geometry->bytes_per_meta_actual) {

			nand_nvram_fail(nv, ret);

		} else if (size > (geometry->data_pages_per_block * geometry->bytes_per_page)) {

			nand_nvram_fail(nv, ret);

		} else if (NULL == (nv->watermarks = (uint32_t *) alloc_mem(nv, watermarks_size))) {
		
			nand_nvram_fail(nv, ret);

		} else if (NULL == (nv->cursors = (uint32_t *) alloc_mem(nv, cursors_size))) {
		
			nand_nvram_fail(nv, ret);

		} else if (NULL == (nv->copies = (bool *) alloc_mem(nv, copies_size))) {

			nand_nvram_fail(nv, ret);

		} else if (!alloc_blob(nv, &nv->shadow, calc_page_count_from_size(nv, size))) {

			nand_nvram_fail(nv, ret);

		} else {

			uint32_t bank;
			uint32_t block;

			for (bank = 0; bank < geometry->number_of_banks; bank++) {
				for (block = 0; block < geometry->blocks_per_bank; block++) {
					clear_watermark(nv, bank, block);
				}
			}

			set_mem(nv, nv->cursors, 0, cursors_size);
			set_mem(nv, nv->copies, 0, copies_size);
			is_opened = true;
		}
	}

	if (!is_opened) {

		nand_nvram_close(nvram);

	} else {

		dump_nvram(nv, LOG_LEVEL_SPEW, 0);
	}

	return (ret);
}

bool
nand_nvram_read(nand_nvram_t * nv, uint32_t offset, uint8_t * buffer, uint32_t length)
{
	bool ret = true;
	const uint32_t extent = offset + length - 1;

	/*
	 * Bounds check the parameters, and make certain that the blob
	 * has been correctly restored from nand to shadow memory.
	 */
	if ((buffer == NULL) || 
	    (length == 0) || 
	    (extent >= nv->size)) {

		log_printf(nv, LOG_LEVEL_SPEW, "buffer: 0x%08X, length: %d, extent: %d\n",
			   (uint32_t) buffer, length, extent);
		nand_nvram_fail(nv, ret);

	} else if (!restore_shadow(nv)) {

		nand_nvram_fail(nv, ret);

	} else {

		/* Then, copy data to buffer from shadow memory. */
		copy_mem(nv, buffer, nv->shadow->data_bufs + offset, length);
	}

	dump_nvram(nv, LOG_LEVEL_SPEW, 0);

	return (ret);
}

bool
nand_nvram_write(nand_nvram_t * nv, uint32_t offset, const uint8_t * buffer, uint32_t length)
{
	bool ret = true;
	const uint32_t extent = offset + length - 1;

	/*
	 * Bounds check the parameters, and make certain that the blob
	 * has been correctly restored from nand to shadow memory.
	 */
	if ((buffer == NULL) || 
	    (length == 0) || 
	    (extent >= nv->size)) {
		
		nand_nvram_fail(nv, ret);

	} else if (!restore_shadow(nv)) {

		nand_nvram_fail(nv, ret);

	} else {

		/* Write data from buffer to shadow memory descriptor. */
		copy_mem(nv, nv->shadow->data_bufs + offset, buffer, length);

		/* Mark blob as dirty. */
		nv->is_dirty = true;
	}

	dump_nvram(nv, LOG_LEVEL_SPEW, 0);

	return (ret);
}

bool
nand_nvram_sync(nand_nvram_t * nv)
{
	bool ret = true;

	/*
	 * If shadow is dirty, sync current state of blob to nand, thus
	 * creating a new generation.
	 */
	if (!restore_shadow(nv)) {
		
		nand_nvram_fail(nv, ret);

	} else if (nv->is_dirty && !commit_shadow(nv)) {

		nand_nvram_fail(nv, ret);

	} else {

		nv->is_dirty = false;
	}

	dump_nvram(nv, LOG_LEVEL_SPEW, 0);

	return (ret);
}

const nand_nvram_geometry_t *
nand_nvram_get_geometry(nand_nvram_t * nv)
{
	return (&nv->geo);
}

void
nand_nvram_close(nand_nvram_t ** nvram)
{
	nand_nvram_t * nv = *nvram;

	/* Free allocated memory. */
	if (NULL != nv) {

		free_blob(nv, &nv->shadow, get_blob_page_count(nv, nv->shadow));

		if (NULL != nv->cursors)
			free_mem(nv, nv->cursors, nv->geo.number_of_banks * sizeof(uint32_t));

		free_mem(nv, nv, sizeof(nand_nvram_t));
		*nvram = NULL;
	}
}

/* ========================================================================== */

static bool
restore_shadow(nand_nvram_t * nv)
{
	bool ret = true;
	uint32_t copy_count = 0;
	uint32_t bank;
	uint32_t block;
	uint32_t * ancestors = NULL;
	uint32_t youngest = kNAND_NVRAM_GENERATION_UNKNOWN;

	if (kNAND_NVRAM_GENERATION_UNKNOWN != get_blob_generation(nv, nv->shadow)) {
		log_printf(nv, LOG_LEVEL_SPEW, "shadow already restored from nand\n");
	} else if (NULL == (ancestors = (uint32_t*)alloc_mem(nv, nv->geo.number_of_banks * sizeof(uint32_t)))) {
		log_printf(nv, LOG_LEVEL_SPEW, "alloc failed\n");
		ret = false;
	} else {

		log_printf(nv, LOG_LEVEL_SPEW, "restoring nvram shadow from nand\n");

		for (bank = 0; bank < nv->geo.number_of_banks; bank++) {
			ancestors[bank] = kNAND_NVRAM_GENERATION_UNKNOWN;
			for (block = 0; block < nv->geo.blocks_per_bank; block++) {
				if (!is_block_bad(nv, bank, block) && !is_block_closed(nv, bank, block)) {
					find_youngest(nv, bank, block, &ancestors[bank], &youngest);
				}
			}
		}

		if (kNAND_NVRAM_GENERATION_UNKNOWN != youngest) {

			const uint8_t version_minor = get_blob_page_meta(nv, nv->shadow, 0)->version_minor;
			const uint32_t page_count = get_blob_page_count(nv, nv->shadow);

			nv->is_born = true;

			/* Ensure that a new generation is created if restored generation is
			 * from an older minor revision of driver.
			 */
			if (kNAND_NVRAM_VERSION_MINOR > version_minor) {
				nv->is_dirty = true;
			} else {
				nv->is_dirty = false;
			}

			if (kNAND_NVRAM_FEATURE_EXTRA_AREA_VERSION_MINOR <= version_minor) {

				const uint32_t extra_offset = calc_extra_offset(nv);
				const uint32_t extra_size = calc_extra_size(nv);
				const uint32_t last_page_in_blob = page_count - 1;
				uint8_t * last_page_data = get_blob_page_data(nv, nv->shadow, last_page_in_blob);
				uint8_t * extra_data = last_page_data + extra_offset;

				log_printf(nv, LOG_LEVEL_SPEW, 
					   "providing partition table differences stored in extra area of current generation");

				if (!provide_ptab_diff(nv, extra_data, extra_size)) {
					log_printf(nv, LOG_LEVEL_ERROR,
						   "failed attempt to provide %d bytes stored in "
						   "extra area at end of page %d in blob\n",
						   extra_size, last_page_in_blob);
					dump_data(nv, LOG_LEVEL_WARN, 1, last_page_data);
				} else {
					// change log level to watch this happening during debug
					dump_data(nv, LOG_LEVEL_SPEW, 1, last_page_data);
				}
			}

			for (bank = 0; bank < nv->geo.number_of_banks; bank++) {

				const bool is_youngest = (ancestors[bank] == youngest);

				log_printf(nv, LOG_LEVEL_WARN, 
					   "%s generation %d @ %dp0x%06X\n", 
					   (is_youngest ? "current" : "prior"),
					   ancestors[bank], 
					   bank, 
					   nv->cursors[bank] - page_count);

				if (ancestors[bank] == youngest) {
					nv->copies[bank] = true;
					copy_count++;
				}

				if (kNAND_NVRAM_GENERATION_UNKNOWN == ancestors[bank]) {
					log_printf(nv, LOG_LEVEL_SPEW,
						   "no valid generation found for bank %d\n",
						   bank);
				}
			}

			log_printf(nv, LOG_LEVEL_SPEW, 
				   "restored %d copies of nvram v%d.%d, generation %d, from nand\n",
				   copy_count,
				   get_blob_page_meta(nv, nv->shadow, 0)->version_major,
				   get_blob_page_meta(nv, nv->shadow, 0)->version_minor,
				   get_blob_generation(nv, nv->shadow));
			dump_nvram(nv, LOG_LEVEL_SPEW, 0);

		} else {

			nv->is_dirty = true;
			nv->is_born = false;
			log_printf(nv, LOG_LEVEL_SPEW, "nand nvram doesn't yet have any generations\n");
		}
		free_mem(nv, ancestors, nv->geo.number_of_banks * sizeof(uint32_t));
	}

	return (ret);
}

static bool 
read_blob(nand_nvram_t * nv, blob_t * blob, uint32_t bank, uint32_t block, uint32_t base, uint32_t offset)
{
	bool is_read = true;
	uint32_t idx;
	uint8_t * data;
	meta_t * meta;
	bool is_clean;

	for (idx = 0; idx < get_blob_page_count(nv, blob); idx++) {

		const int32_t page_in_block = offset + idx;
		const uint32_t page = base + page_in_block;

		data = get_blob_page_data(nv, blob, idx);
		meta = get_blob_page_meta(nv, blob, idx); 

		// if any page in blob could not be read or failed
		// validation, blob is considered unreadable
		if (!read_page(nv, bank, page, data, meta, &is_clean) ||
		    !validate_meta(nv, meta, idx)) {

			is_read = false;
			break;
		}
	}

	return (is_read);
}

/* ========================================================================== */

static bool 
commit_shadow(nand_nvram_t * nv)
{   
	bool ret = true;
	uint32_t commit_count = 0;
	blob_t * scratch = NULL;
	bool is_newborn = !nv->is_born;
	uint32_t bank;

	/* 
	 * Allocate temporary blob for readback verification during
	 * commit.  Note that we only ever need temporary storage for
	 * one page at a time during commit, so there is no need to
	 * allocate anything larger.
	 */
	if (is_newborn && !beget_newborn(nv)) {

		nand_nvram_fail(nv, ret);

	} else if (!alloc_blob(nv, &scratch, 1)) {

		nand_nvram_fail(nv, ret);

	} else {

		/* Migrate forward from previous minor version, if necessary. */
		migrate_forward(nv);
		
		/* Whether virgin or not, increment nvram generation once for every commit. */
		increment_generation(nv);
		
		/*
		 * Commit pages of blob to each bank, recording whether any
		 * bank was able to commit.
		 */
		for (bank = 0; bank < nv->geo.number_of_banks; bank++) {

			if (commit_to_bank(nv, bank, scratch)) {
			
				commit_count++;
			}
		}

		/* Free temporary blob. */
		free_blob(nv, &scratch, 1);

		/* Fail if we were unable to commit the blob to any bank. */
		if (0 == commit_count) {

			nand_nvram_fail(nv, ret);

		} else {

			log_printf(nv, LOG_LEVEL_SPEW,
				   "committed %d copies of nvram generation %d\n",
				   commit_count, get_blob_generation(nv, nv->shadow));
		}
	}

	return (ret);
}

static bool 
commit_to_bank(nand_nvram_t * nv, uint32_t bank, blob_t * scratch)
{
	const uint32_t blob_page_count = get_blob_page_count(nv, nv->shadow);
	bool is_committed = false;
	uint32_t attempts = 0;
	const uint32_t start_page = nv->cursors[bank];
	uint32_t block = calc_block_from_page(nv, start_page);
	uint32_t page_in_block = calc_page_in_block_from_page(nv, start_page);
	uint32_t watermark;

	while (!is_committed && (attempts < nv->geo.blocks_per_bank)) {

		if (is_block_bad(nv, bank, block)) {

			log_printf(nv, LOG_LEVEL_SPEW,
				   "block %d:%d known to be bad, skipping\n", 
				   bank, block);
			page_in_block = nv->geo.pages_per_block;

		} else if ((watermark = get_watermark(nv, bank, block)) != page_in_block) {

			const uint32_t extra = (watermark % blob_page_count);
			if ((0 != extra) && (watermark < nv->geo.data_pages_per_block)) {

				// address case where prior commit left partial
				// generation (e.g. due to power loss)
				const uint32_t missing = (blob_page_count - extra);
				log_printf(nv, LOG_LEVEL_INFO, 
					   "prior commit left unaligned watermark at %d:%d:%d\n", 
					   bank, block, watermark);
				fill_pages_with_ptab(nv, bank, block, watermark, missing);
				watermark = get_watermark(nv, bank, block);
			}

			if (page_in_block < watermark) {

				// address case where youngest generation in block has
				// written pages after it (e.g. due to corrupted generation
				// or after filling partial generation above)
				log_printf(nv, LOG_LEVEL_INFO,
					   "skipping %d pages, %d:%d:%d-%d\n",
					   watermark - page_in_block, bank, block, page_in_block, watermark - 1);
				page_in_block = watermark;

			} else if (page_in_block > watermark) {

				log_printf(nv, LOG_LEVEL_ERROR,
					   "ERROR: in bank %d, page_in_block > watermark (%d > %d)\n",
					   bank, page_in_block, watermark);
				dump_nvram(nv, LOG_LEVEL_SPEW, 0);
				goto out;
			}
		}

		if ((page_in_block + blob_page_count) > nv->geo.data_pages_per_block) {
			if (!prepare_next_commit_block(nv, bank, &block, &attempts)) {
				log_printf(nv, LOG_LEVEL_SPEW, 
					   "unable to prepare any block for bank %d\n", bank);
				break;
			} else {
				page_in_block = 0;
			}
		}

		if (commit_to_block_in_bank(nv, bank, block, page_in_block, scratch)) {
			is_committed = true;
		} else {
			log_printf(nv, LOG_LEVEL_SPEW, 
				   "unable to commit: bank %d, block 0x%08X, page_in_block %d\n",
				   bank, block, page_in_block);
		}
	}

out:
	return (is_committed);
}

static bool 
prepare_next_commit_block(nand_nvram_t * nv, uint32_t bank, uint32_t * block, uint32_t *attempts)
{
	bool is_prepared = false;

	/* Start trying at the next block. */
	bool may_close_this;
	bool may_close_next = true;
	uint32_t this_block = *block;
	uint32_t next_block = 0;
	uint32_t total_attempts = *attempts;

	while (!is_prepared && (total_attempts < nv->geo.blocks_per_bank)) {

		may_close_this = may_close_next;
		next_block = this_block + 1;

		if (next_block >= nv->geo.blocks_per_bank) {

			next_block = 0;
			log_printf(nv, LOG_LEVEL_SPEW,
				   "rolling around to block 0 of bank %d\n", bank);

		}

		if (is_block_bad(nv, bank, next_block)) {
				
			may_close_next = false;
			log_printf(nv, LOG_LEVEL_SPEW,
				   "block %d of bank %d known to be bad, skipping\n", 
				   next_block, bank);

		} else {

			may_close_next = true;

			if (erase_block(nv, bank, next_block)) {
				
				is_prepared = true;
				log_printf(nv, LOG_LEVEL_SPEW,
					   "erased block %d of bank %d in preparation for commit\n", 
					   next_block, bank);

			} else {

				log_printf(nv, LOG_LEVEL_SPEW,
					   "unable to erase block %d of bank %d, skipping\n", 
					   next_block, bank);
			}
		}

		if (may_close_this) {
			close_block(nv, bank, this_block, nv->geo.data_pages_per_block);
		}

		this_block = next_block;
		total_attempts++;
	}

	*block = next_block;
	*attempts = total_attempts;

	return (is_prepared);
}

static bool 
commit_to_block_in_bank(nand_nvram_t * nv, uint32_t bank, uint32_t block, uint32_t start_page_in_block, blob_t * scratch)
{
	uint8_t * blob_data = NULL;
	meta_t * blob_meta = NULL;

	uint8_t * scratch_data = get_blob_page_data(nv, scratch, 0);
	meta_t * scratch_meta = get_blob_page_meta(nv, scratch, 0);

	const uint32_t page_count = get_blob_page_count(nv, nv->shadow);
	const uint32_t base_page = calc_base_page_from_block(nv, block);
	const uint32_t start_page = start_page_in_block + base_page;
	
	uint32_t page_in_blob = 0;
	uint32_t pages_committed = 0;

	/*
	 * Write out each page of the blob, read it back, and verify
	 * the page read is the same as what was just written,
	 * accumulating a count of how many of the pages were
	 * successfully committed.
	 */
	for (page_in_blob = 0; page_in_blob < page_count; page_in_blob++) {
		
		const uint32_t current_page_in_block = start_page_in_block + page_in_blob;
		const uint32_t page = base_page + current_page_in_block;

		blob_data = get_blob_page_data(nv, nv->shadow, page_in_blob);
		blob_meta = get_blob_page_meta(nv, nv->shadow, page_in_blob);

		if ((page_count - 1) == page_in_blob) {

			/* We are at the last page of this NVRAM blob, and we will use the extra space 
			 * to store information on behalf of the partition scheme. Currently we have more 
			 * than enough extra space left, but we may run into problem if we change
			 * the size of NVRAM or page size.
			 */
			const uint32_t extra_offset = calc_extra_offset(nv);
			const uint32_t extra_size = calc_extra_size(nv);
			uint8_t * extra_data = blob_data + extra_offset;
            
			log_printf(nv, LOG_LEVEL_SPEW, 
				   "requesting partition table differences to store in extra area of current generation");

			if (!request_ptab_diff(nv, extra_data, extra_size)) {
				log_printf(nv, LOG_LEVEL_ERROR,
					"failed to request %d bytes to store in extra area at end of page %d in blob\n",
					extra_size, page_in_blob);
				dump_data(nv, LOG_LEVEL_WARN, 1, blob_data);
				break;
			} else {
				// change log level to watch this happening during debug
				dump_data(nv, LOG_LEVEL_SPEW, 1, blob_data);
			}
		}

		/*
		 * Write page from nv to nand, read back page, and
		 * verify contents.
		 */
		if (!write_page(nv, bank, page, blob_data, blob_meta)) {

			log_printf(nv, LOG_LEVEL_SPEW, 
				   "commit failed write of page %d in blob to bank %d page 0x%08X\n",
				   page_in_blob, bank, page);
			break;
			
		} else if (!read_page(nv, bank, page, scratch_data, scratch_meta, NULL)) {

			log_printf(nv, LOG_LEVEL_SPEW, 
				   "commit failed read of page %d in blob from bank %d page 0x%08X\n",
				   page_in_blob, bank, page);
			break;

		} else if (0 != compare_mem(nv, blob_data, scratch_data, nv->geo.bytes_per_page)) {

			log_printf(nv, LOG_LEVEL_SPEW, 
				   "commit failed data verification of page %d in blob at bank %d page 0x%08X\n",
				   page_in_blob, bank, page);
			log_printf(nv, LOG_LEVEL_SPEW, "blob_data:\n");
			dump_data(nv, LOG_LEVEL_SPEW, 1, blob_data);
			log_printf(nv, LOG_LEVEL_SPEW, "scratch_data:\n");
			dump_data(nv, LOG_LEVEL_SPEW, 1, scratch_data);
			break;

		} else if (0 != compare_mem(nv, blob_meta, scratch_meta, sizeof(meta_t))) {

			log_printf(nv, LOG_LEVEL_SPEW, 
				   "commit failed meta verification of page %d in blob at bank %d page 0x%08X\n",
				   page_in_blob, bank, page);
			log_printf(nv, LOG_LEVEL_SPEW, "blob_meta:\n");
			dump_meta(nv, LOG_LEVEL_SPEW, 1, blob_meta);
			log_printf(nv, LOG_LEVEL_SPEW, "scratch_meta:\n");
			dump_meta(nv, LOG_LEVEL_SPEW, 1, scratch_meta);
			break;

		} else {
			
			/*
			 * If write, read, and verifications all
			 * succeeded, increment commit counter.
			 */
			pages_committed++;
		}
	}
	
	/* Update page number of commit cursor for this bank whether or
	 * not commit actually succeeded.
	 */
	nv->cursors[bank] = start_page + page_count;

	/*
	 * If all pages in blob were successfully committed, record
	 * the page number of the new new commit index for this bank,
	 * and return true to indicate that the commit succeeded.
	 */
	if (pages_committed == page_count) {
		return (true);
	}
	return (false);
}

static void
increment_generation(nand_nvram_t * nv)
{
	if ((NULL != nv->shadow) && (NULL != nv->shadow->meta_bufs)) {

		uint32_t page;
		meta_t * meta = get_blob_page_meta(nv, nv->shadow, 0);
		uint32_t next_gen = 1 + meta->generation;

		if (kNAND_NVRAM_GENERATION_ERASED == next_gen) {
			next_gen = 1;
		}
		for (page = 0; page < get_blob_page_count(nv, nv->shadow); page++) {
			meta = get_blob_page_meta(nv, nv->shadow, page);
			meta->generation = next_gen;
			meta->version_minor = kNAND_NVRAM_VERSION_MINOR;
		}
	}
}
	
/* ========================================================================== */
	
static void
init_meta(nand_nvram_t * nv, meta_t * meta, uint32_t generation, uint32_t page_count, uint32_t page_idx)
{
	meta->signature = kNAND_NVRAM_SIGNATURE;
	meta->version_major = kNAND_NVRAM_VERSION_MAJOR;
	meta->version_minor = kNAND_NVRAM_VERSION_MINOR;
	meta->generation = generation;
	meta->page_max = (uint8_t)(page_count - 1);
	meta->page_idx = (uint8_t)page_idx;
}

static bool
validate_meta(nand_nvram_t * nv, meta_t * meta, uint32_t idx)
{
	return ((meta->signature == kNAND_NVRAM_SIGNATURE) &&
		(meta->version_major == kNAND_NVRAM_VERSION_MAJOR) &&
		(meta->generation != kNAND_NVRAM_GENERATION_UNKNOWN) &&
		(meta->generation != kNAND_NVRAM_GENERATION_ERASED) &&
		(meta->page_max == (calc_page_count_from_size(nv, nv->size)) - 1) &&
		(meta->page_idx <= meta->page_max) &&
		(meta->page_idx == idx));
}

/* ========================================================================== */

static uint32_t
calc_block_from_page(nand_nvram_t * nv, uint32_t page_num)
{
	/* XXX bounds check page_num */
	return (page_num / nv->geo.pages_per_block);
}

static uint32_t
calc_page_in_block_from_page(nand_nvram_t * nv, uint32_t page_num)
{
	/* XXX bounds check page_num */
	return (page_num % nv->geo.pages_per_block);
}

static uint32_t
calc_base_page_from_block(nand_nvram_t * nv, uint32_t block_num)
{
	/* XXX bounds check block_num */
	return (block_num * nv->geo.pages_per_block);
}

static uint32_t
calc_page_count_from_size(nand_nvram_t * nv, uint32_t size)
{
	uint32_t remainder = ((0 != (size % nv->geo.bytes_per_page)) ? 1 : 0);
	return ((size / nv->geo.bytes_per_page) + remainder);
}

static uint32_t
calc_extra_offset(nand_nvram_t * nv)
{
	return (nv->size % nv->geo.bytes_per_page);
}

static uint32_t
calc_extra_size(nand_nvram_t * nv)
{
	return (nv->geo.bytes_per_page - calc_extra_offset(nv));
}

/* ========================================================================== */

static uint32_t
get_blob_generation(nand_nvram_t * nv, blob_t * blob)
{
	if ((NULL != blob) && (NULL != blob->meta_bufs)) {

		return (get_blob_page_meta(nv, blob, 0)->generation);
	}

	return kNAND_NVRAM_GENERATION_UNKNOWN;
}

static uint32_t
get_blob_page_count(nand_nvram_t * nv, blob_t * blob)
{
	return calc_page_count_from_size(nv, nv->size);
}

static uint8_t *
get_blob_page_data(nand_nvram_t * nv, blob_t * blob, uint32_t page_index)
{
	/* XXX assert bounds check on page_index and buf ptr */

	uint8_t * bytes = (uint8_t *)blob->data_bufs;
	return ((uint8_t*)&bytes[page_index * nv->geo.bytes_per_page]);
}

static meta_t *
get_blob_page_meta(nand_nvram_t * nv, blob_t * blob, uint32_t page_index)
{
	/* XXX assert bounds check on page_index and buf ptr */

	uint8_t * bytes = (uint8_t *)blob->meta_bufs;
	return ((meta_t *)&bytes[page_index * nv->geo.bytes_per_meta_buffer]);
}

static bool
alloc_blob(nand_nvram_t * nv, blob_t ** blob_ref, uint32_t page_count)
{
	bool ret = true;

	blob_t * blob = NULL;
	void * new_data = NULL;
	void * new_meta = NULL;
	uint32_t old_page_count = 0;
	bool is_realloc;

	/* Allocate shell to hold buffers, if necessary. */
	if (NULL == *blob_ref) {

		if (NULL == (*blob_ref = (blob_t *) alloc_mem(nv, sizeof(blob_t)))) {

			nand_nvram_fail(nv, ret);
			return (ret);
		}

		set_mem(nv, *blob_ref, 0, sizeof(blob_t));

		/*
		 * If this is a zero-length alloc (i.e. nv
		 * initialization), we're done.
		 */
		if (page_count == 0)
			return (ret);

	} else {
		old_page_count = get_blob_page_count(nv, *blob_ref);
	}

	blob = *blob_ref;
	is_realloc = (old_page_count != 0);

	/* Make certain that this isn't an improper reallocation. */
	if (is_realloc && (old_page_count > page_count)) {

		log_printf(nv, LOG_LEVEL_SPEW, 
			   "attempted invalid realloc from %d pages to %d pages\n",
			   old_page_count, page_count);
		nand_nvram_fail(nv, ret);

	} else if (is_realloc && (old_page_count == page_count)) {
		
		log_printf(nv, LOG_LEVEL_SPEW, 
			   "unnecessary realloc from %d pages same number of pages\n",
			   old_page_count);

	} else if (!alloc_page_bufs(nv, page_count, &new_meta, &new_data)) {

		free_blob(nv, blob_ref, page_count);
		nand_nvram_fail(nv, ret);

	} else if (0 != page_count) {

		const uint32_t old_data_size = old_page_count * nv->geo.bytes_per_page;	
		const uint32_t new_data_size = page_count * nv->geo.bytes_per_page;
		const uint32_t delta_data_size = new_data_size - old_data_size;

		uint32_t generation = kNAND_NVRAM_GENERATION_UNKNOWN;
		uint32_t page;
	
		if (is_realloc) {

			/* Copy old contents of blob to newly allocated buffers. */
			copy_mem(nv, new_data, blob->data_bufs, old_data_size);

			/* Record old blob generation. */
			generation = get_blob_generation(nv, blob);

			/* Free old data and meta buffers. */
			free_page_bufs(nv, old_page_count, &blob->meta_bufs, &blob->data_bufs);
		}
		
		/* Zero new pages of data buffers. */
		set_mem(nv, ((uint8_t *) new_data) + old_data_size, 0, delta_data_size);

		/* Record pointers to new buffers. */
		blob->data_bufs = new_data;
		blob->meta_bufs = new_meta;

		/* Init pages of meta buffers. */
		for (page = 0; page < page_count; page++) {

			meta_t * meta = get_blob_page_meta(nv, blob, page);
			init_meta(nv, meta, generation, page_count, page);
		}
	}

	return (ret);
}

static void
free_blob(nand_nvram_t * nv, blob_t ** blob_ref, uint32_t page_count)
{
	blob_t * blob = *blob_ref;

	if (NULL != blob) {

		free_page_bufs(nv, page_count, &blob->meta_bufs, &blob->data_bufs);
		free_mem(nv, blob, sizeof(blob_t));
		*blob_ref = NULL;

	} else {

		/* XXX warn that we tried to free an unallocated blob */
	}
}

/* ========================================================================== */

static bool
alloc_page_bufs(nand_nvram_t * nv, uint32_t page_count, void ** meta_bufs_ref, void ** data_bufs_ref)
{
	const uint32_t meta_bufs_size = page_count * nv->geo.bytes_per_meta_buffer;
	const uint32_t data_bufs_size = page_count * nv->geo.bytes_per_page;
	void * new_meta_bufs = NULL;
	void * new_data_bufs = NULL;

	/* XXX assert not already allocated */

	if ((NULL == (new_data_bufs = alloc_mem(nv, data_bufs_size))) ||
	    (NULL == (new_meta_bufs = alloc_mem(nv, meta_bufs_size)))) {
		
		/* Free buffers if not able to allocate both of them, then fail. */
		free_page_bufs(nv, page_count, &new_meta_bufs, &new_data_bufs);
		return (false);
	}

	*meta_bufs_ref = new_meta_bufs;
	*data_bufs_ref = new_data_bufs;

	return (true);
}

static void
free_page_bufs(nand_nvram_t * nv, uint32_t page_count, void ** meta_bufs_ref, void ** data_bufs_ref)
{
	if (NULL != *data_bufs_ref) {

		free_mem(nv, *data_bufs_ref, page_count * nv->geo.bytes_per_page);
		*data_bufs_ref = NULL;		    
	}

	if (NULL != *meta_bufs_ref) {

		free_mem(nv, *meta_bufs_ref, page_count * nv->geo.bytes_per_meta_buffer);
		*meta_bufs_ref = NULL;
	}
}

/* ========================================================================== */

static uint32_t
calc_watermark_index(nand_nvram_t * nv, uint32_t bank, uint32_t block)
{
	return ((nv->geo.blocks_per_bank * bank) + block);
}

static uint32_t
get_watermark(nand_nvram_t * nv, uint32_t bank, uint32_t block)
{
	return(nv->watermarks[calc_watermark_index(nv, bank, block)]);
}

static void
update_watermark(nand_nvram_t * nv, uint32_t bank, uint32_t block, uint32_t new_watermark)
{
	const uint32_t idx = calc_watermark_index(nv, bank, block);
	const uint32_t old_watermark = nv->watermarks[idx];
	if (new_watermark > old_watermark) {
		nv->watermarks[idx] = new_watermark;
	}
}

static void
clear_watermark(nand_nvram_t * nv, uint32_t bank, uint32_t block)
{
	nv->watermarks[calc_watermark_index(nv, bank, block)] = 0;
}

/* ========================================================================== */

static inline void *
alloc_mem(nand_nvram_t * nv, uint32_t size)
{
	return ((void *) nand_nvram_alloc_mem(nv->context, size));
}

static inline signed
compare_mem(nand_nvram_t * nv, void * left, void * right, uint32_t size)
{
	return (nand_nvram_compare_mem(nv->context, left, right, size));
}

static inline void
copy_mem(nand_nvram_t * nv, void * dest, const void * src, uint32_t size)
{
	nand_nvram_copy_mem(nv->context, dest, src, size);
}

static inline void
set_mem(nand_nvram_t * nv, void * mem, uint8_t value, uint32_t size)
{
	nand_nvram_set_mem(nv->context, mem, value, size);
}

static inline void
free_mem(nand_nvram_t * nv, void * mem, uint32_t size)
{
	nand_nvram_free_mem(nv->context, mem, size);
}

static inline bool
is_block_bad(nand_nvram_t * nv, uint32_t bank, uint32_t block)
{
	return (nand_nvram_is_block_bad(nv->context, bank, block));
}

static inline bool 
read_page(nand_nvram_t * nv, uint32_t bank, uint32_t page, void * data, void * meta, bool * is_clean)
{
	bool ok = nand_nvram_read_page(nv->context, bank, page, data, meta, is_clean);

	// if page read succeeds or is not clean, when clean checking
	// is enabled, update block watermark
	if (ok || ((NULL != is_clean) && !*is_clean)) {

		const uint32_t block = calc_block_from_page(nv, page);
		const uint32_t page_in_block = calc_page_in_block_from_page(nv, page);

		update_watermark(nv, bank, block, page_in_block + 1);
	}

	return (ok);
}

static inline bool 
write_page(nand_nvram_t * nv, uint32_t bank, uint32_t page, void * data, void * meta)
{
	const uint32_t block = calc_block_from_page(nv, page);
	const uint32_t page_in_block = calc_page_in_block_from_page(nv, page);

	update_watermark(nv, bank, block, page_in_block + 1);

	return (nand_nvram_write_page(nv->context, bank, page, data, meta));
}

static inline bool
request_ptab_diff(nand_nvram_t * nv, void * data, uint32_t size)
{
	return (nand_nvram_request_ptab_diff(nv->context, data, size));
}

static inline bool
provide_ptab_diff(nand_nvram_t * nv, void * data, uint32_t size)
{
	return (nand_nvram_provide_ptab_diff(nv->context, data, size));
}

static inline bool 
erase_block(nand_nvram_t * nv, uint32_t bank, uint32_t block)
{
	clear_watermark(nv, bank, block);
	return (nand_nvram_erase_block(nv->context, bank, block));
}

static inline bool 
copy_ptab(nand_nvram_t * nv, void * buf)
{
	return (nand_nvram_copy_ptab(nv->context, buf));
}

static inline bool 
read_ptab(nand_nvram_t * nv, uint32_t bank, uint32_t page, void * buf)
{
	bool ok = nand_nvram_read_ptab(nv->context, bank, page, buf);

	// if ptab read succeeds, update block watermark
	if (ok) {

		const uint32_t block = calc_block_from_page(nv, page);
		const uint32_t page_in_block = calc_page_in_block_from_page(nv, page);

		update_watermark(nv, bank, block, page_in_block + 1);
	}

	return (ok);
}

static inline bool 
write_ptab(nand_nvram_t * nv, uint32_t bank, uint32_t page, void * buf)
{
	const uint32_t block = calc_block_from_page(nv, page);
	const uint32_t page_in_block = calc_page_in_block_from_page(nv, page);

	update_watermark(nv, bank, block, page_in_block + 1);

	return (nand_nvram_write_ptab(nv->context, bank, page, buf));
}

/* ========================================================================== */

#if !kNAND_NVRAM_MUTE

static void
_dump_geometry(nand_nvram_t * nv, uint32_t lvl, uint32_t indent, nand_nvram_geometry_t * geo)
{
	dump(nv, lvl, indent, "%-20s%d\n", "number_of_banks:", geo->number_of_banks);
	dump(nv, lvl, indent, "%-20s%d\n", "blocks_per_bank:", geo->blocks_per_bank);
	dump(nv, lvl, indent, "%-20s%d\n", "pages_per_block:", geo->pages_per_block);
	dump(nv, lvl, indent, "%-20s%d\n", "data_pages_per_block:", geo->data_pages_per_block);
	dump(nv, lvl, indent, "%-20s%d\n", "bytes_per_page:", geo->bytes_per_page);
}

static void
_dump_meta(nand_nvram_t * nv, uint32_t lvl, uint32_t indent, meta_t * meta)
{
	dump(nv, lvl, indent, "%-20s0x%04X\n", "signature:", meta->signature);
	dump(nv, lvl, indent, "%-20s%d\n", "version_major:", meta->version_major);
	dump(nv, lvl, indent, "%-20s%d\n", "version_minor:", meta->version_minor);
	dump(nv, lvl, indent, "%-20s%d\n", "generation:", meta->generation);
	dump(nv, lvl, indent, "%-20s%d\n", "page_max:", meta->page_max);
	dump(nv, lvl, indent, "%-20s%d\n", "page_idx:", meta->page_idx);
}

#define _isprint(c) (((c) > 32) && ((c) < 127))
		
static void
_dump_data(nand_nvram_t * nv, uint32_t lvl, uint32_t indent, uint8_t * data)
{
	char ascii[17] = {0};
	uint32_t idx;

	for (idx = 0; idx < nv->geo.bytes_per_page; idx++) {
		uint32_t col = idx % 16;
		if (0 == col)
			dump(nv, lvl, indent, "%04X", idx);
		dump(nv, lvl, 0, " %02X", data[idx]);
		ascii[col] = (_isprint(data[idx]) ? (char) data[idx] : '.');
		if (15 == idx % 16)
			dump(nv, lvl, 0, " %s\n", ascii);
	}

	if (0 != idx % 16) {
		ascii[idx % 16] = 0;
		for (; 0 != idx % 16; idx++)
			dump(nv, lvl, 0, "   ");
		dump(nv, lvl, 0, " %s\n", ascii);
	}
}

static void
_dump_blob(nand_nvram_t * nv, uint32_t lvl, uint32_t indent, blob_t * blob, uint32_t page_count)
{
	uint32_t idx;

	dump(nv, lvl, indent, "%-20s0x%08X\n", "meta_bufs:" , (uint32_t) blob->meta_bufs);

	for (idx = 0; idx < page_count; idx++) {
		dump(nv, lvl, indent, "meta_bufs[%d]\n", idx);
		_dump_meta(nv, lvl, indent + 1, get_blob_page_meta(nv, blob, idx));
	}

	dump(nv, lvl, indent, "%-20s0x%08X\n", "data_bufs:" , (uint32_t) blob->data_bufs);

	for (idx = 0; idx < page_count; idx++) {
		dump(nv, lvl, indent, "data_bufs[%d]\n", idx);
		_dump_data(nv, lvl, indent + 1, get_blob_page_data(nv, nv->shadow, idx));
	}
}

static void
_dump_nvram(nand_nvram_t * nv, uint32_t lvl, uint32_t indent)
{
	unsigned idx;

	dump(nv, lvl, indent, "geo:\n");
	_dump_geometry(nv, lvl, indent + 1, &nv->geo);

	dump(nv, lvl, indent, "%-20s0x%08X\n", "context:", (uint32_t)nv->context);
	dump(nv, lvl, indent, "%-20s%d\n", "size:", nv->size);
	dump(nv, lvl, indent, "%-20s%s\n", "is_born:", nv->is_born ? "true" : "false");
	dump(nv, lvl, indent, "%-20s%s\n", "is_dirty:", nv->is_dirty ? "true" : "false");

	dump(nv, lvl, indent, "%-20s*0x%08X = {0x%08X", 
	     "watermarks:", (uint32_t)nv->watermarks, nv->watermarks[0]);
	for (idx = 1; idx < (nv->geo.number_of_banks * nv->geo.blocks_per_bank); idx++)
		dump(nv, lvl, 0, ", 0x%08X", nv->watermarks[idx]);
	dump(nv, lvl, 0, "}\n");

	dump(nv, lvl, indent, "%-20s*0x%08X = {0x%08X", 
	     "cursors:", (uint32_t)nv->cursors, nv->cursors[0]);
	for (idx = 1; idx < nv->geo.number_of_banks; idx++)
		dump(nv, lvl, 0, ", 0x%08X", nv->cursors[idx]);
	dump(nv, lvl, 0, "}\n");

	dump(nv, lvl, indent, "%-20s*0x%08X = {%s", 
	     "copies:", (uint32_t)nv->copies, nv->copies[0] ? "true" : "false");
	for (idx = 1; idx < nv->geo.number_of_banks; idx++)
		dump(nv, lvl, 0, ", %s", nv->copies[idx] ? "true" : "false");
	dump(nv, lvl, 0, "}\n");

	dump(nv, lvl, indent, "%-20s0x%08X\n", "shadow:", (uint32_t)nv->shadow);
	_dump_blob(nv, lvl, indent + 1, nv->shadow, get_blob_page_count(nv, nv->shadow));
}

#endif /* !kNAND_NVRAM_MUTE */

/* ========================================================================== */

static bool
is_block_closed(nand_nvram_t * nv, uint32_t bank, uint32_t block)
{
	bool ok = false;
	NandPartTable * ptab = NULL;
	uint32_t base = ((block * nv->geo.pages_per_block) + nv->geo.data_pages_per_block);
	uint32_t page;
	uint32_t idx;

	if (NULL == (ptab = (NandPartTable*)alloc_mem(nv, sizeof(NandPartTable)))) {
		panic("alloc failed");
	} else {
		for (idx = 0; idx < kNAND_NVRAM_PAGE_RESERVED_IN_BLOCK; idx++) {
			page = base + idx;
			if (read_ptab(nv, bank, page, ptab) &&
			    (kNandPartHeaderMagic == ptab->header.magic)) {
				ok = true;
				break;
			}
		}
		free_mem(nv, ptab, sizeof(NandPartTable));
	}

	return (ok);
}

static bool 
close_block(nand_nvram_t * nv, uint32_t bank, uint32_t block, uint32_t offset)
{
	const uint32_t count = (nv->geo.pages_per_block - offset);
	log_printf(nv, LOG_LEVEL_SPEW, "closing block %d:%d\n", bank, block);
	return(fill_pages_with_ptab(nv, bank, block, offset, count));
}

static bool 
fill_pages_with_ptab(nand_nvram_t * nv, uint32_t bank, uint32_t block, uint32_t offset, uint32_t count)
{
	bool ok = false;
	NandPartTable * ptab = NULL;
	const uint32_t base = (block * nv->geo.pages_per_block);
	uint32_t idx;

	if (NULL == (ptab = (NandPartTable*)alloc_mem(nv, sizeof(NandPartTable)))) {
		panic("alloc failed");
	} else {
		if (!copy_ptab(nv, ptab)) {
			log_printf(nv, LOG_LEVEL_SPEW, "unable to copy ptab\n");
		} else {
			for (idx = 0; idx < count; idx++) {
				const uint32_t page_in_block = offset + idx;
				const uint32_t page = base + page_in_block;
				log_printf(nv, LOG_LEVEL_SPEW,
					   "filling page %d:%d:%d with ptab\n",
					   bank, block, page_in_block);
				if (write_ptab(nv, bank, page, ptab)) {
					ok = true;
				}
			}
		}
		free_mem(nv, ptab, sizeof(NandPartTable));
	}

	return (ok);
}

static bool
beget_newborn(nand_nvram_t * nv)
{
	bool ok = false;
	bool is_open;
	uint32_t bank;
	uint32_t block;

	for (bank = 0; bank < nv->geo.number_of_banks; bank++) {
		is_open = false;
		for (block = 0; block < nv->geo.blocks_per_bank; block++) {
			if (!is_block_bad(nv, bank, block) && erase_block(nv, bank, block)) {
				if (!is_open) {
					nv->cursors[bank] = calc_base_page_from_block(nv, block);
					is_open = true;
					ok = true;
				} else {
					close_block(nv, bank, block, 0);
				}
			}
		}
	}

	if (ok) {
		nv->is_born = true;
		nv->is_dirty = true;
	}

	return (ok);
}

static void
migrate_forward(nand_nvram_t * nv)
{
	const uint8_t version_minor = get_blob_page_meta(nv, nv->shadow, 0)->version_minor;
	uint32_t bank;
	uint32_t block;
	uint32_t open_block;

	if (version_minor < kNAND_NVRAM_VERSION_MINOR) {
		for (bank = 0; bank < nv->geo.number_of_banks; bank++) {
			open_block = calc_block_from_page(nv, nv->cursors[bank]);
			for (block = 0; block < nv->geo.blocks_per_bank; block++) {
				if ((block != open_block) && !is_block_closed(nv, bank, block)) {
					if (!is_block_bad(nv, bank, block) && (erase_block(nv, bank, block))) {
						close_block(nv, bank, block, 0);
					}
				}
			}
		}
	}
}

static bool
find_youngest(nand_nvram_t * nv, uint32_t bank, uint32_t block, uint32_t* ancestor, uint32_t* youngest)
{
	/* Strategy:
	 *
	 * Do a reverse linear scan in order to find the youngest blob
	 * available in this block.  Because blobs must be committed
	 * to block in ascending order, the first good block from the
	 * end that can be read and that validates correctly will
	 * necessarily be the youngest generation blob in the block.
	 */
	bool ret = false;
	const uint32_t increment = calc_page_count_from_size(nv, nv->size);
	const uint32_t available = nv->geo.data_pages_per_block;
	const uint32_t slots = (available / increment);
	blob_t * scratch = NULL;
	uint32_t offset = (slots * increment);

	if (!alloc_blob(nv, &scratch, increment)) {
		nand_nvram_fail(nv, ret);
	} else {
		while (!ret && (offset != 0)) {
			const uint32_t base = calc_base_page_from_block(nv, block);
			offset -= increment;
			if (read_blob(nv, scratch, bank, block, base, offset)) {
				const uint32_t generation = get_blob_generation(nv, scratch);
				log_printf(nv, LOG_LEVEL_SPEW, 
					   "found generation %d at %dp0x%06X\n", 
					   generation, bank, base + offset);
				if (generation >= *ancestor) {
					*ancestor = generation;
					nv->cursors[bank] = base + offset + increment;
				}
				if (generation > *youngest) {
					*youngest = generation;
					free_blob(nv, &nv->shadow, increment);
					nv->shadow = scratch;
					scratch = NULL;
					if (!alloc_blob(nv, &scratch, increment)) {
						nand_nvram_fail(nv, ret);
					}
				}
				ret = true;
			} else {
				log_printf(nv, LOG_LEVEL_SPEW, 
					   "unable to read blob at %dp0x%06X\n", 
					   bank, base + offset);
			}
		} 
	}

	free_blob(nv, &scratch, increment);

	return (ret);
}

/* ========================================================================== */
