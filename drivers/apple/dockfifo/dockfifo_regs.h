/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __APPLE_DOCKFIFO_H
#define __APPLE_DOCKFIFO_H

#include <platform/soc/hwregbase.h>

#define DBGFIFO_W_SPACING		(0x1000)
#define DBGFIFO_CNFG_CG_ENA		(0x1)

#define rDBGFIFO_R_DATA(_f, _n)		(*(volatile uint32_t *)(DBGFIFO_0_BASE_ADDR + ((_f) * DBGFIFO_SPACING) + ((_n) * 4)))
#define rDBGFIFO_R_STAT(_f)		(*(volatile uint32_t *)(DBGFIFO_0_BASE_ADDR + ((_f) * DBGFIFO_SPACING) + 0x14))
#define rDBGFIFO_W_DATA(_f, _n)		(*(volatile uint32_t *)(DBGFIFO_0_BASE_ADDR + ((_f) * DBGFIFO_SPACING) + DBGFIFO_W_SPACING + ((_n) * 4)))
#define rDBGFIFO_W_STAT(_f)		(*(volatile uint32_t *)(DBGFIFO_0_BASE_ADDR + ((_f) * DBGFIFO_SPACING) + DBGFIFO_W_SPACING + 0x14))
#define rDBGFIFO_CNFG(_f)		(*(volatile uint32_t *)(DBGFIFO_0_BASE_ADDR + ((_f) * DBGFIFO_SPACING) + 0x2000))
#define rDBGFIFO_DRAIN(_f)		(*(volatile uint32_t *)(DBGFIFO_0_BASE_ADDR + ((_f) * DBGFIFO_SPACING) + 0x2004))
#define rDBGFIFO_INTMASK(_f)		(*(volatile uint32_t *)(DBGFIFO_0_BASE_ADDR + ((_f) * DBGFIFO_SPACING) + 0x2008))

#endif /* __APPLE_DOCKFIFO_H */
