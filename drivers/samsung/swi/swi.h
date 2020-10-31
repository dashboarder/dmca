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

#ifndef __SAMSUNG_SWI_H
#define __SAMSUNG_SWI_H

#include <platform/soc/hwregbase.h>

#define rSWI_CON			(*(volatile u_int32_t *)(SWI_BASE_ADDR + 0x00))
#define  SWI_CON_SWI_CLK_DIV(n)		(((n) & 0xFF) << 8)
#define  SWI_CON_SWI_CLK_SEL_NCLK	(0 << 2)
#define  SWI_CON_SWI_CLK_SEL_PCLK	(1 << 2)
#define  SWI_CON_SWI_OFF_STATE_LOW	(0 << 1)
#define  SWI_CON_SWI_OFF_STATE_HIGH	(1 << 1)
#define  SWI_CON_SWI_EN			(1 << 0)
#define rSWI_SWRESET			(*(volatile u_int32_t *)(SWI_BASE_ADDR + 0x04))
#define rSWI_TIMING			(*(volatile u_int32_t *)(SWI_BASE_ADDR + 0x08))
#define rSWI_INT_FLAG			(*(volatile u_int32_t *)(SWI_BASE_ADDR + 0x0C))
#define rSWI_INT_MASK			(*(volatile u_int32_t *)(SWI_BASE_ADDR + 0x10))
#define rSWI_ITR_COM			(*(volatile u_int32_t *)(SWI_BASE_ADDR + 0x14))
#define  SWI_ITR_COM_SWI_ITR_MODE_8BIT	(0 << 1)
#define  SWI_ITR_COM_SWI_ITR_MODE_16BIT	(1 << 1)
#define  SWI_ITR_COM_SWI_ITR_SET	(1 << 0)
#define rSWI_ITR_DATA			(*(volatile u_int32_t *)(SWI_BASE_ADDR + 0x18))
#define rSWI_STR_COM			(*(volatile u_int32_t *)(SWI_BASE_ADDR + 0x1C))
#define rSWI_STR_DATA			(*(volatile u_int32_t *)(SWI_BASE_ADDR + 0x20))
#define rSWI_STR_DLY			(*(volatile u_int32_t *)(SWI_BASE_ADDR + 0x24))
#define rSWI_HTR_COM			(*(volatile u_int32_t *)(SWI_BASE_ADDR + 0x28))
#define rSWI_HTR_DATA			(*(volatile u_int32_t *)(SWI_BASE_ADDR + 0x2C))
#define rSWI_VERISON			(*(volatile u_int32_t *)(SWI_BASE_ADDR + 0x30))

#endif /* ! __SAMSUNG_SWI_H */
