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
  @header nand_nvram_core

  @abstract portable core of a nand-based NVRAM device implementation

  @discussion In order to boot from nand, the NVRAM device must have an
  implementation with a backing store on the nand flash device since
  the traditional nor-based backing store will no longer be available.
  This class implements such a device over a slice of the nand array,
  using a journalled commit approach with multiple redundant copies.

  @copyright Apple Inc.

  @updated 2011-05-18 
*/

/* ========================================================================== */

#ifndef _NAND_NVRAM_CORE_H
#define _NAND_NVRAM_CORE_H

__BEGIN_DECLS

/* ========================================================================== */

/*!
  @group public platform-independent interface function declarations
*/

/*!
  @function nand_nvram_open

  @abstract Open platform-independent ram-shadowed nand nvram device.

  @param _geometry Geometry of nand device array slice for backing store.

  @result Returns bool indicating whether function succeeded.
*/
bool nand_nvram_open(nand_nvram_t ** _nvram, void * context, nand_nvram_geometry_t * _geometry, uint32_t _size);

/*!
  @function nand_nvram_read

  @abstract Read data from the nvram device shadow.

  @param _offset Byte offset into nvram data blob.

  @param _buffer Pointer to memory buffer to read data from.

  @param _length Amount of data in bytes to read from blob.

  @result Returns bool indicating whether function succeeded.
*/
bool nand_nvram_read(nand_nvram_t * _nvram, uint32_t _offset, uint8_t * _buffer, uint32_t _length);

/*!
  @function nand_nvram_write

  @abstract Write data to the nvram device shadow.

  @param _offset Byte offset into nvram data blob.

  @param _buffer Pointer to memory buffer to write data into.

  @param _length Amount of data in bytes to write to blob.

  @result Returns bool indicating whether function succeeded.
*/
bool nand_nvram_write(nand_nvram_t * _nvram, uint32_t _offset, const uint8_t * _buffer, uint32_t _length);

/*!
  @function nand_nvram_sync

  @abstract Synchronize nvram device data to backing store.

  @result Returns bool indicating whether function succeeded.
*/
bool nand_nvram_sync(nand_nvram_t * _nvram);

/*!
  @function nand_nvram_get_geometry

  @abstract Provide access to nand nvram driver's geometry information.

  @result Returns pointer to struct defining nand nvram geometry.
*/
const nand_nvram_geometry_t * nand_nvram_get_geometry(nand_nvram_t * _nvram);

/*!
  @function nand_nvram_close

  @abstract Close platform-independent ram-shadowed nand nvram device.
*/
void nand_nvram_close(nand_nvram_t ** _nvram);

/* ========================================================================== */

__END_DECLS

#endif /* ! _NAND_NVRAM_CORE_H */

/* ========================================================================== */
