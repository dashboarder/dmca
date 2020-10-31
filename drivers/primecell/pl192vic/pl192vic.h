/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PRIMECELL_PL192VIC_H
#define __PRIMECELL_PL192VIC_H

#include <platform/soc/hwregbase.h>

#define rVICIRQSTATUS(v)		(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x000))
#define rVICFIQSTATUS(v)		(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x004))
#define rVICRAWINTR(v)			(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x008))
#define rVICINTSELECT(v)		(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x00C))
#define rVICINTENABLE(v)		(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x010))
#define rVICINTENCLEAR(v)		(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x014))
#define rVICSOFTINT(v)			(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x018))
#define rVICSOFTINTCLEAR(v)		(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x01C))
#define rVICPROTECTION(v)		(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x020))
#define rVICSWPRIORITYMASK(v)		(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x024))
#define rVICPRIORITYDAISY(v)		(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x028))
#define rVICVECTADDR(v, n)		(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x100 + (n) * 4))
#define rVICVECTPRIO(n)			(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x200 + (n) * 4))
#define rVICADDR(v)			(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0xF00))
#define rVICPERID0(v)			(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0xFE0))
#define rVICPERID1(v)			(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0xFE4))
#define rVICPERID2(v)			(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0xFE8))
#define rVICPERID3(v)			(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0xFEC))
#define rVICPCELLID0(v)			(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0xFF0))
#define rVICPCELLID1(v)			(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0xFF4))
#define rVICPCELLID2(v)			(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0xFF8))
#define rVICPCELLID3(v)			(*(volatile u_int32_t *)(VIC_BASE_ADDR + (v) * VIC_STRIDE + 0xFFC))

/*
 * If the platform has a VIC used for inter-processor communication,
 * we need to be able to touch its soft-int register.
 */
#ifdef IPC_VIC_BASE_ADDR
# define rVICIPCSOFTINT(v)		(*(volatile u_int32_t *)(IPC_VIC_BASE_ADDR + (v) * VIC_STRIDE + 0x018))
#endif

#endif /* __PRIMECELL_PL192VIC_H */
