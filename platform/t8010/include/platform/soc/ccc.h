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

#include <platform.h>
#include SUB_PLATFORM_SPDS_HEADER(acc)

// CPU0 Regsiters
#define rCCC_CPU0_IMPL_IORVBAR			(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + ACC_CPU0_IMPL_IO_RVBAR_OFFSET))
#define rCCC_CPU0_IMPL_MMU_ERR_STS		(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + ACC_CPU0_IMPL_MMU_ERR_STS_OFFSET))
#define rCCC_CPU0_IMPL_MIGSTS_EL1		(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + ACC_CPU0_IMPL_MIGSTS_EL1_OFFSET))

// CPU0 Hurricane Regsiters
#define rCCC_CPU0_IMPL_FED_ERR_STS		(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + ACC_CPU0_IMPL_FED_ERR_STS_OFFSET))
#define rCCC_CPU0_IMPL_LSU_ERR_STS		(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + ACC_CPU0_IMPL_LSU_ERR_STS_OFFSET))

// CPU0 Zephyr Regsiters
#define rCCC_CPU0_IMPL_E_FED_ERR_STS		(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + ACC_CPU0_IMPL_E_FED_ERR_STS_OFFSET))
#define rCCC_CPU0_IMPL_E_LSU_ERR_STS		(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + ACC_CPU0_IMPL_E_LSU_ERR_STS_OFFSET))

// CPU1 Registers
#define rCCC_CPU1_IMPL_IORVBAR			(*(volatile u_int64_t *)(CCC_CPU1_SYS_BASE_ADDR + ACC_CPU0_IMPL_IO_RVBAR_OFFSET))

// Registers shared between all CPUs
#define rCCC_CCC_IMPL_L2C_ERR_STS		(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + ACC_IMPL_L2C_ERR_STS_OFFSET))
#define rCCC_CCC_IMPL_L2C_ERR_ADR		(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + ACC_IMPL_L2C_ERR_ADR_OFFSET))
#define rCCC_CCC_IMPL_L2C_ERR_INF		(*(volatile u_int64_t *)(CCC_CPU0_SYS_BASE_ADDR + ACC_IMPL_L2C_ERR_INF_OFFSET))

#endif /* __PLATFORM_SOC_CCC_H */
