/*
 * Copyright (C) 2009-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_SOC_AMG_H
#define __PLATFORM_SOC_AMG_H

#include <platform/soc/hwregbase.h>

/* Apple Memory Gasket */

#define AMG_INDEX_FOR_CHANNEL(_c)	(((_c) < 2) ? (_c) : ((_c) + 2))

#define rAMG_DLL_CONFIG(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x00))
#define rAMG_DLL_OVERRIDE(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x04))
#define rAMG_DLL_CONTROL(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x08))
#define rAMG_DLL_STATUS(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x0C))
#define rAMG_DLL_TIMER(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x1C))

#define rAMG_GATE_OFFSET(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x10))
#define rAMG_FINE_OFFSET(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x14))
#define rAMG_ADDR_OFFSET(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x18))

#define rAMG_RD_OFFSET(_c,_n)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x20 + ((_n)*4)))
#define rAMG_WR_OFFSET(_c,_n)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x30 + ((_n)*4)))

#define rAMG_CORE_CONFIG(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x40))
#define rAMG_INIT_CONTROL(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x44))
#define rAMG_DLL_FILTER(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x48))
#define rAMG_FREQ_SCALE(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x4C))

#define rAMG_ZQ_OVERRIDE(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x50))
#define rAMG_ZQ_STATUS(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x54))
#define rAMG_ZQ_CONTROL(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x58))

#define rAMG_READ_LATENCY(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x5C))

#define rAMG_DQCAL_CONTROL(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x60))
#define rAMG_DQCAL_STATUS(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x64))

#define rAMG_ZQ_CONF(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x70))
#define rAMG_CKSTOP_CONF(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x74))
#define rAMG_DLLTIMER_CONF(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x78))
#define rAMG_DLL_CONF(_c)	(*(volatile u_int32_t *)(AMG_BASE_ADDR + ((_c)*AMG_SPACING) + 0x7C))

struct amg_params {
	uint32_t flags;
	uint32_t core_config;
	uint32_t read_latency;
	uint32_t gate_offset;
	uint32_t freq_scale;
	uint32_t bit_time_ps;
};

#define FLAG_AMG_PARAM_CAL_STATIC	0x0001

#include <platform/amgconfig.h>

#endif /* ! __PLATFORM_SOC_AMG_H */
