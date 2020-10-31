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

#ifndef __SAMSUNG_DWI_H
#define __SAMSUNG_DWI_H

#include <platform/soc/hwregbase.h>

#define rDWI_CLOCK_CONFIG		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0000))
#define  DWI_CLOCK_CONFIG_TX_CPHA_1	(1 << 0)
#define  DWI_CLOCK_CONFIG_TX_CPHA_2	(0 << 0)
#define  DWI_CLOCK_CONFIG_RX_CPHA_1	(1 << 1)
#define  DWI_CLOCK_CONFIG_RX_CPHA_2	(0 << 1)
#define  DWI_CLOCK_CONFIG_CPOL_LOW	(0 << 2)
#define  DWI_CLOCK_CONFIG_CPOL_HIGH	(1 << 2)
#define  DWI_CLOCK_CONFIG_CLOCK_SCALER(_x)	(((_x) - 1) << 16)
#define rDWI_TRANSFER_GAP		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0004))
#define rDWI_ITR0_CONTROL		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0010))
#define  DWI_TR_CTRL_TRAN_EN		(1 << 0)
#define  DWI_TR_CTRL_BYTE_CNT(_c)	(((_c) - 1) << 4)
#define  DWI_TR_CTRL_BYTE_SWAP		(1 << 6)
#define  DWI_TR_CTRL_BIT_ORDER		(1 << 7)
#define  DWI_TR_CTRL_INT_EN		(1 << 8)
#define rDWI_ITR0_TX_DATA		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0014))
#define rDWI_ITR0_RX_DATA		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0018))
#define rDWI_ITR0_INT_STATUS		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x001C))
#define rDWI_ITR1_CONTROL		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0020))
#define rDWI_ITR1_TX_DATA		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0024))
#define rDWI_ITR1_RX_DATA		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0028))
#define rDWI_ITR1_INT_STATUS		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x002C))
#define rDWI_ITR1_RAMP_COUNT		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0030))
#define rDWI_ITR1_DELTAV_ACTIVE		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0034))
#define rDWI_STR_CONTROL		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0040))
#define rDWI_STR_TX_DATA		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0044))
#define rDWI_STR_RX_DATA		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0048))
#define rDWI_STR_INT_STATUS		(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x004C))
#define rDWI_STR_DELAY			(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0050))
#define rDWI_VERSION			(*(volatile u_int32_t *)(DWI_BASE_ADDR + 0x0070))

#endif /* ! __SAMSUNG_DWI_H */
