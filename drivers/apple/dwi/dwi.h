/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __APPLE_DWI_H
#define __APPLE_DWI_H

#include <platform/soc/hwregbase.h>
#include SUB_PLATFORM_SPDS_HEADER(dwi)

#define rDWI_CLOCK_CONFIG		(*(volatile u_int32_t *)(DWI_BASE_ADDR + DWI_BLK_CLOCK_CONFIG_OFFSET))
#define  DWI_CLOCK_CONFIG_TX_CPHA_1	(DWI_BLK_CLOCK_CONFIG_TX_CPHA_INSRT(1) << 0)
#define  DWI_CLOCK_CONFIG_TX_CPHA_2	(DWI_BLK_CLOCK_CONFIG_TX_CPHA_INSRT(0) << 0)
#define  DWI_CLOCK_CONFIG_RX_CPHA_1	(DWI_BLK_CLOCK_CONFIG_TX_CPHA_INSRT(1) << 1)
#define  DWI_CLOCK_CONFIG_RX_CPHA_2	(DWI_BLK_CLOCK_CONFIG_TX_CPHA_INSRT(0) << 1)
#define  DWI_CLOCK_CONFIG_CPOL_LOW	(DWI_BLK_CLOCK_CONFIG_CPOL_INSRT(0) << 2)
#define  DWI_CLOCK_CONFIG_CPOL_HIGH	(DWI_BLK_CLOCK_CONFIG_CPOL_INSRT(1) << 2)
#define  DWI_CLOCK_CONFIG_CLOCK_SCALER(_x)	(DWI_BLK_CLOCK_CONFIG_CLOCK_SCALER_INSRT(_x - 1))
#define rDWI_TRANSFER_GAP		(*(volatile u_int32_t *)(DWI_BASE_ADDR + DWI_BLK_TRANSFER_GAP_OFFSET))
#define rDWI_ITR0_CONTROL		(*(volatile u_int32_t *)(DWI_BASE_ADDR + DWI_BLK_ITR0_CONTROL_OFFSET))
#define  DWI_TR_CTRL_TRAN_EN		(1 << DWI_BLK_ITR0_CONTROL_ITR0_TRAN_EN_SHIFT)
#define  DWI_TR_CTRL_BYTE_CNT(_c)	(DWI_BLK_ITR0_CONTROL_ITR0_BYTE_COUNT_INSRT(_c - 1))
#define  DWI_TR_CTRL_BYTE_SWAP		(1 << DWI_BLK_ITR0_CONTROL_ITR0_BYTE_SWAP_SHIFT)
#define  DWI_TR_CTRL_BIT_ORDER		(1 << DWI_BLK_ITR0_CONTROL_ITR0_BIT_ORDER_SHIFT)
#define  DWI_TR_CTRL_SLAVE_PSPI		(DWI_BLK_ITR0_CONTROL_SLAVE_SEL_INSRT(0))
#define  DWI_TR_CTRL_SLAVE_DWI		(DWI_BLK_ITR0_CONTROL_SLAVE_SEL_INSRT(1))
#define rDWI_ITR0_TX_DATA		(*(volatile u_int32_t *)(DWI_BASE_ADDR + DWI_BLK_ITR0_TX_DATA_OFFSET))

#endif /* ! __APPLE_DWI_H */
