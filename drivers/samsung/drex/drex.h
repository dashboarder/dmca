/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __SAMSUNG_DREX_H
#define __SAMSUNG_DREX_H

#include <platform/soc/hwregbase.h>

#define rCONCONTROL			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x000))
#define rMEMCONTROL			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x004))
#define rMEMCONFIG			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x008))
#define rDIRECTCMD			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x010))
#define rPRECHCONFIG			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x014))
#define rPHYCONTROL0			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x018))
#define rPHYCONTROL1			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x01C))
#define rPHYCONTROL2			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x020))
#define rPHYCONTROL3			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x024))
#define rPWRDNCONFIG			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x028))
#define rTIMINGPZQ			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x02C))
#define rTIMINGAREF			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x030))
#define rTIMINGROW			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x034))
#define rTIMINGDATA			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x038))
#define rTIMINGPOWER			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x03C))
#define rPHYSTATUS			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x040))
#define rPHYZQCONTROL			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x044))
#define rCHIP0STATUS			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x048))
#define rCHIP1STATUS			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x04C))
#define rAREFSTATUS			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x050))
#define rMRSTATUS			(*(volatile u_int32_t *)(DREX_BASE_ADDR + 0x054))

#define rQOSTMDREX			(*(volatile u_int32_t *)(AXI_SPINE_BASE_ADDR + 0x400))
#define rQOSACDREX			(*(volatile u_int32_t *)(AXI_SPINE_BASE_ADDR + 0x404))

#endif /* ! __SAMSUNG_DREX_H */
