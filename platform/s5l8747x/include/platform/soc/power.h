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

#ifndef __PLATFORM_SOC_POWER_H
#define __PLATFORM_SOC_POWER_H

#include <platform/soc/hwregbase.h>

#define rPWRCONF			(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x000))
#define rPWRCMD				(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x004))
#define rOSCCNT				(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x038))
#define rPLLCNT				(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x03C))
#define rSTATESAVE0			(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x044))
#define rSTATESAVE1			(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x048))
#define rSTATESAVE2			(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x04C))
#define rSTATESAVE3			(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x050))
#define rTMCMD				(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x06C))
#define rTMDATA				(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x070))
#define rTMPRE				(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x074))
#define rTMCNT				(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x078))
#define rTMALARM			(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x07C))
#define rSECCONF			(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x104))
#define rVERSION			(*(volatile u_int32_t *)(AAM_BASE_ADDR + 0x108))

#endif /* ! __PLATFORM_SOC_POWER_H */
