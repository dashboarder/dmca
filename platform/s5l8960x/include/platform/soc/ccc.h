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

#ifndef __PLATFORM_SOC_CCC_H
#define __PLATFORM_SOC_CCC_H

#define rCCC_CPU0_IMPL_IORVBAR			(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + 0x000))
#define rCCC_CPU0_IMPL_CPU_IOACC_CTL_EL3	(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + 0x008))
#define rCCC_CPU0_IMPL_FED_ERR_STS		(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + 0x010))
#define rCCC_CPU0_IMPL_LSU_ERR_STS		(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + 0x018))
#define rCCC_CPU0_IMPL_MMU_ERR_STS		(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + 0x020))

#define CCC0_IMPL_BASE_ADDR			(CCC0_SYS_BASE_ADDR + 0x40000)
#define rCCC_CCC_IMPL_CPM_IOACC_CTL_EL3		(*(volatile u_int64_t *)(CCC0_IMPL_BASE_ADDR + 0x000))
#define rCCC_CCC_IMPL_L2C_ERR_STS		(*(volatile u_int64_t *)(CCC0_IMPL_BASE_ADDR + 0x008))
#define rCCC_CCC_IMPL_L2C_ERR_ADR		(*(volatile u_int64_t *)(CCC0_IMPL_BASE_ADDR + 0x010))
#define rCCC_CCC_IMPL_L2C_ERR_INF		(*(volatile u_int64_t *)(CCC0_IMPL_BASE_ADDR + 0x018))

#define rCCC_CPU1_IMPL_IORVBAR			(*(volatile u_int64_t *)(CCC_CPU1_SYS_BASE_ADDR + 0x000))
#define rCCC_CPU2_IMPL_IORVBAR			(*(volatile u_int64_t *)(CCC_CPU2_SYS_BASE_ADDR + 0x000))

#endif /* __PLATFORM_SOC_CCC_H */
