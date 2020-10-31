/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* This file contains Tunables for s8001 ipad6b */

#include <platform/tunables.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwregbase.h>
#include <soc/module/address_map.h>

/**************************************** BEGIN CHIP_REVISION_ALL ****************************************/
// <rdar://problem/22709991> Update iBoot/PMGR driver settings for enabling UV_WARN for J127/J105
#define SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8001_IPAD6B_EXTRA \
{  /*! GFX_THROTTLE_DITHER0_INT  */          0x7403c,        sizeof(uint32_t), 0xffffffffu,    0x480000u}, \
{  /*! GFX_THROTTLE_DITHER0      */          0x74040,        sizeof(uint32_t), 0xffffffffu,    0x10555u}, \
{  /*! GFX_THROTTLE_DITHER1      */          0x74044,        sizeof(uint32_t), 0xffffffffu,    0x106dbu}, \
{  /*! GFX_THROTTLE_DITHER2      */          0x74048,        sizeof(uint32_t), 0xffffffffu,    0x10fffu}, \
{  /*! GFX_THROTTLE_DITHER3      */          0x7404c,        sizeof(uint32_t), 0xffffffffu,    0x00fffu}, \
{  /*! GFX_THROTTLE_DITHER4      */          0x74050,        sizeof(uint32_t), 0xffffffffu,    0x00fffu}, \
{  /*! GFX_THROTTLE_DITHER5      */          0x74054,        sizeof(uint32_t), 0xffffffffu,    0x00fffu}, \
{  /*! GFX_THROTTLE_SRC2_CTL     */          0x74078,        sizeof(uint32_t), 0xffffffffu,    0x00003u}, \
{  /*! GFX_THROTTLE_SRC2_ASSERT_COUNT     */ 0x7407c,        sizeof(uint32_t), 0xffffffffu,    0x00000u}, \
{  /*! GFX_THROTTLE_SRC2_DEASSERT_COUNT   */ 0x74080,        sizeof(uint32_t), 0xffffffffu,    0x00000u}, \
{  /*! GFX_THROTTLE_SRC2_MIN_ACTIVE_COUNT */ 0x74084,        sizeof(uint32_t), 0xffffffffu,    0x00000u}, \
{  /*! GFX_PERF_CFG4    */                   0xfc140,        sizeof(uint32_t), 0xffffffffu,    0x10003u}, \
{  /*! GFX_PERF_CFG5    */                   0xfc150,        sizeof(uint32_t), 0xffffffffu,    0x10001u}, \
{  /*! GFX_PERF_CTL    */                    0xfc000,        sizeof(uint32_t), 0xffffffffu,    0x00003u}, \
{                                         -1,             -1,               -1,          -1            }
/**************************************** END CHIP_REVISION_ALL ****************************************/

/**************************************** BEGIN CHIP_REVISION_B0 ****************************************/
#include <soc/s8001/b0/tunable/pmgr.h>

static const struct tunable_struct pmgr_tunable_b0[] = {
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8001_IPAD6B_EXTRA
};

/***************************************** END CHIP_REVISION_B0 *****************************************/

/**************************************** BEGIN CHIP_REVISION_A0 ****************************************/
#include <soc/s8001/a0/tunable/pmgr.h>

static const struct tunable_struct pmgr_tunable_a0[] = {
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8001_IPAD6B_EXTRA
};

/***************************************** END CHIP_REVISION_A0 *****************************************/

// For each chip, highest revision must come first.
// Notice reconfig == false for minipmgr, since it's an AOP.
const struct tunable_chip_struct tunables_pmgr_product[] = {
	// B0
	{CHIP_REVISION_B0, SOUTH_BRIDGE_PMS_PMGR_PADDR_S8001_B0,                pmgr_tunable_b0,         NULL,           true},

	// A0
	{CHIP_REVISION_A0, SOUTH_BRIDGE_PMS_PMGR_PADDR_S8001_A0,                pmgr_tunable_a0,         NULL,           true},

	{0, 0, NULL, NULL, 0}
};