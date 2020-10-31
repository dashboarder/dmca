/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __APPLE_ROSC_H
#define __APPLE_ROSC_H

#include <platform/soc/hwregbase.h>

#define ROSC_INTERVAL		0x40000
#define ROSC_PRESCALE		0x100
#define ROSC_VTH		(3)
#define ROSC_NUM		(5)

#define rROSC_CMD		(*(volatile u_int32_t *)(ROSC_BASE_ADDR + 0x000000))
#define rROSC_CFG		(*(volatile u_int32_t *)(ROSC_BASE_ADDR + 0x000004))
#define rROSC_CUR_STAT		(*(volatile u_int32_t *)(ROSC_BASE_ADDR + 0x000008))
#define rROSC_MAX_STAT		(*(volatile u_int32_t *)(ROSC_BASE_ADDR + 0x00000c))
#define rROSC_MIN_STAT		(*(volatile u_int32_t *)(ROSC_BASE_ADDR + 0x000010))
#define rROSC_VER		(*(volatile u_int32_t *)(ROSC_BASE_ADDR + 0x0000A0))

#define ROSC_CMD_NOP		(0<<0)
#define ROSC_CMD_RUN_ONCE	(1<<0)
#define ROSC_CMD_START		(2<<0)
#define ROSC_CMD_STOP		(3<<0)

#define ROSC_CFG_INT_DURATION	((ROSC_INTERVAL-1)<<8)
#define ROSC_1MHZ		1000000
#endif /* ! __APPLE_ROSC_H */
