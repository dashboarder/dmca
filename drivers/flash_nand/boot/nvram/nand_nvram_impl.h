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

/* ========================================================================== */

/*! 
  @header nand_nvram_impl

  @abstract portable header info for nand nvram implementation use only

  @discussion These declarations have been pulled out into an
  independent file so that diagnostic utilities and white box unit
  tests might take advantage of knowing implementation-specific
  details.  Nevertheless, no client of the nand nvram driver nor its
  system-specific wrapper should ever need to reference this header.
  Any declarations that need that level of visibility are intended to
  be placed in "nand_nvram_core.h".

  @copyright Apple Inc.

  @updated 2012-02-21 
*/

/* ========================================================================== */

#ifndef _NAND_NVRAM_IMPL_H
#define _NAND_NVRAM_IMPL_H

__BEGIN_DECLS

/* ========================================================================== */

/*!
  @group public platform-independent type declarations
*/

/*!
  @typedef nand_nvram_t

  @abstract Opaque reference to structure needed by nand nvram implementation.
*/
typedef struct nand_nvram nand_nvram_t;

/*!
  @typedef nand_nvram_geometry_t

  @abstract
*/
typedef struct nand_nvram_geometry nand_nvram_geometry_t;

/* ========================================================================== */

/*!
  @group private platform-independent type declarations
*/

typedef struct _meta meta_t;
typedef struct _blob blob_t;

/* ========================================================================== */

/*!
  @group public platform-independent constant definitions
*/

/*!
  @const kNAND_NVRAM_SIZE
  
  @abstract Standard size of nvram presented by the driver.  Note
  that, if this ends up changing in the future, this value should
  start being defined on a platform/product specific basis.
*/
#define kNAND_NVRAM_SIZE 8192

/*!
  @const kNAND_NVRAM_META_SIZE
  
  @abstract Amount of bytes required by meta data for each nvram
  backing store page.
*/
#define kNAND_NVRAM_META_SIZE 10

/*!
  @const kNAND_NVRAM_PAGE_RESERVED_IN_BLOCK

  @abstract Number of pages reserved for partition table in each
  NAND block.
*/
#define kNAND_NVRAM_PAGE_RESERVED_IN_BLOCK 2

/* ========================================================================== */

/*!
  @group private platform-independent constant definitions
*/

/*!
  @const NULL

  @abstract It really sucks that some of our systems don't define NULL.
*/
#ifndef NULL
#define NULL ((void *)0)
#endif

/*!
  @const kNAND_NVRAM_MAX_SIZE

  @abstract Maximum amount of storage; should fit into a standard 2K page.
*/
#define kNAND_NVRAM_MAX_SIZE (128 * 2048)

/*!
  @const kNAND_NVRAM_SIGNATURE

  @abstract Constant value used to identify nand nvram meta data.
*/
#define kNAND_NVRAM_SIGNATURE ((uint16_t) 0xBEAD)

/*!
  @const kNAND_NVRAM_VERSION_MAJOR

  @abstract Major version of the driver that wrote the nvram content.
  Major version differences are considered to be incompatible.	Even
  major version numbers are reserved for development purposes; odd
  version numbers should be used for when development is frozen,
  stabilized, and therefore only accepting minor revision level changes.
*/
#define kNAND_NVRAM_FEATURE_EXTRA_AREA_VERSION_MAJOR ((uint8_t) 0x0)
#define kNAND_NVRAM_VERSION_MAJOR kNAND_NVRAM_FEATURE_EXTRA_AREA_VERSION_MAJOR

/*!
  @const kNAND_NVRAM_VERSION_MINOR

  @abstract Minor version of the driver that wrote the nvram content.
  Minor versions should be fully compatible within a major version and
  are intended to be incremented when the driver has changed between
  public releases.
*/
#define kNAND_NVRAM_FEATURE_EXTRA_AREA_VERSION_MINOR ((uint8_t) 0x3)
#define kNAND_NVRAM_VERSION_MINOR kNAND_NVRAM_FEATURE_EXTRA_AREA_VERSION_MINOR

/*!
  @const kNAND_NVRAM_GENERATION_ERASED

  @abstract A generation number of all ones is reserved since it also
  corresponds to the erased state of nand bits.
*/
#define kNAND_NVRAM_GENERATION_ERASED ((uint32_t) 0xFFFFFFFF)

/*!
  @const kNAND_NVRAM_GENERATION_UNKNOWN

  @abstract A generation number of all zeros is reserved since it
  corresponds to the state of an uninitialized meta struct.
*/
#define kNAND_NVRAM_GENERATION_UNKNOWN ((uint32_t) 0x00000000)

/* ========================================================================== */

/*!
  @group public platform-independent structure declarations
*/

/*!
  @struct nand_nvram_geometry

  @abstract Geometry of nand device array slice.

  @field number_of_banks Number of logical banks.

  @field blocks_per_bank Number of blocks in each bank.

  @field pages_per_block Number of pages per block.

  @field data_pages_per_block Number of pages per block available for data storage.

  @field bytes_per_page Size in bytes of page.

  @field bytes_per_meta_buffer Size in bytes of buffer for in-memory per-page meta info.

  @field bytes_per_meta_actual Size in bytes of in-storage per-page meta info (i.e. actual size).
*/
struct nand_nvram_geometry {

	uint32_t number_of_banks;
	uint32_t blocks_per_bank;
	uint32_t pages_per_block;
	uint32_t data_pages_per_block;
	uint32_t bytes_per_page;
	uint32_t bytes_per_meta_buffer;
	uint32_t bytes_per_meta_actual;
};

/* ========================================================================== */

/*!
  @group private platform-independent structure declarations
*/

/*!
  @struct _meta
  
  @abstract Structure defining per-page meta layout.

  @field generation Monotonically increasing generation of blob;
  kNAND_NVRAM_GENERATION_UNKNOWN and kNAND_NVRAM_GENERATION_ERASED are
  reserved values that should never appear on a nand device.

  @field page_max Maximum page index for blob (i.e. page count - 1).
  
  @field page_idx Page index of this meta within blob.

  @field version_major Major version number of nand nvram driver.
  
  @field version_minor Minor version number of nand nvram driver.
  
  @field signature Constant value of kNAND_NVRAM_SIGNATURE marks valid
  nvram meta.
*/
struct _meta {
	
	uint32_t  generation;
	uint8_t   page_max;
	uint8_t   page_idx;
	uint8_t   version_major;
	uint8_t   version_minor;
	uint16_t  signature;

} __attribute__((__packed__));

/*!
  @struct _blob

  @abstract Container for N pages of data and meta information
  for convenience of writing to/from nand devices.

  @field pageCount Number of pages currently allocated to
  shadow the blob.

  @field metaBufs Buffer containing pages' meta structs.

  @field dataBufs Buffer containing pages' data.
*/
struct _blob {

	void * meta_bufs;
	void * data_bufs;
};

/*!
  @struct nand_nvram

  @abstract NAND-based nvram driver context information.

  @field geometry Geometry of nand device array slice used for
  backing store.

  @field is_dirty Flag indicating whether shadow has been
  modified since last restore or sync.

  @field shadow Blob used as container for pages of data and meta
  information being shadowed from nand.

  @field cursors Array of page numbers of next page in each
  bank available for commit.
*/
struct nand_nvram {

	nand_nvram_geometry_t geo;
	void * context;
	uint32_t size;
	bool is_born;
	bool is_dirty;
	uint32_t * watermarks;
	uint32_t * cursors;
	bool * copies;
	blob_t * shadow;
};

/* ========================================================================== */

__END_DECLS

#endif /* ! _NAND_NVRAM_IMPL_H */

/* ========================================================================== */
