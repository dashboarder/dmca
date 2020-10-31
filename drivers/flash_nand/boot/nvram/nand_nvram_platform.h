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

/* ========================================================================== */

/*! 
  @header nand_nvram_platform

  @abstract iBoot platform-specific macros needed by nand nvram

  @copyright Apple Inc.

  @updated 2011-05-18 
*/

/* ========================================================================== */

#ifndef _NAND_NVRAM_PLATFORM_H
#define _NAND_NVRAM_PLATFORM_H

#include <sys/types.h>

#include <debug.h>
#include <lib/heap.h>

__BEGIN_DECLS

/* ========================================================================== */

#define nand_nvram_alloc_mem(context, size) memalign((size), CPU_CACHELINE_SIZE)

#define nand_nvram_compare_mem(context, left, right, size) memcmp((left), (right), (size))

#define nand_nvram_copy_mem(context, dest, src, size) memcpy((dest), (src), (size))

#define nand_nvram_set_mem(context, mem, value, size) memset((mem), (value), (size))

#define nand_nvram_free_mem(context, mem, size) free((mem))

#define nand_nvram_printf(context, ...) dprintf(DEBUG_CRITICAL, __VA_ARGS__)

#define nand_nvram_is_block_bad(context, bank, block) flash_nand_nvram_is_block_bad((context), (bank), (block))

#define nand_nvram_read_page(context, bank, page, data, meta, is_clean) flash_nand_nvram_read_page((context), (bank), (page), (data), (meta), (is_clean))

#define nand_nvram_write_page(context, bank, page, data, meta) flash_nand_nvram_write_page((context), (bank), (page), (data), (meta))

#define nand_nvram_erase_block(context, bank, block) flash_nand_nvram_erase_block((context), (bank), (block))

#define nand_nvram_request_ptab_diff(context, data, size) flash_nand_nvram_request_ptab_diff((context), (data), (size))

#define nand_nvram_provide_ptab_diff(context, data, size) flash_nand_nvram_provide_ptab_diff((context), (data), (size))

#define nand_nvram_copy_ptab(context, buf) flash_nand_nvram_copy_ptab((context), (buf))

#define nand_nvram_read_ptab(context, bank, page, buf) flash_nand_nvram_read_ptab((context), (bank), (page), (buf))

#define nand_nvram_write_ptab(context, bank, page, buf) flash_nand_nvram_write_ptab((context), (bank), (page), (buf))

/* ========================================================================== */

bool flash_nand_nvram_is_block_bad(void * context, uint32_t bank, uint32_t block);
bool flash_nand_nvram_read_page(void * context, uint32_t bank, uint32_t page, void * data, void * meta, bool * is_clean);
bool flash_nand_nvram_write_page(void * context, uint32_t bank, uint32_t page, void * data, void * meta);
bool flash_nand_nvram_erase_block(void * context, uint32_t bank, uint32_t block);
bool flash_nand_nvram_request_ptab_diff(void * context, void * data, uint32_t size);
bool flash_nand_nvram_provide_ptab_diff(void * context, void * data, uint32_t size);
bool flash_nand_nvram_copy_ptab(void * context, void * buf);
bool flash_nand_nvram_read_ptab(void * context, uint32_t bank, uint32_t page, void * buf);
bool flash_nand_nvram_write_ptab(void * context, uint32_t bank, uint32_t page, void * buf);

/* ========================================================================== */

__END_DECLS

#endif /* _NAND_NVRAM_PLATFORM_H */

/* ========================================================================== */
