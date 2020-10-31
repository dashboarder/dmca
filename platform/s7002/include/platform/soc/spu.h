/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_SOC_SPU_H
#define __PLATFORM_SOC_SPU_H

#include <platform/soc/hwregbase.h>

#define rSPU_AKF_AXI_BASE              (*(volatile uint32_t *) (SPU_AKF_BASE_ADDR + 0x8))
#define rSPU_AKF_AXI_BASE_EXT          (*(volatile uint32_t *) (SPU_AKF_BASE_ADDR + 0xC))
#define rSPU_AKF_AXI_START             (*(volatile uint32_t *) (SPU_AKF_BASE_ADDR + 0x10))
#define rSPU_AKF_AXI_END               (*(volatile uint32_t *) (SPU_AKF_BASE_ADDR + 0x18))
#define rSPU_AKF_AXI_EN                (*(volatile uint32_t *) (SPU_AKF_BASE_ADDR + 0x20))
#define rSPU_AKF_AXI_IDLE_CTRL         (*(volatile uint32_t *) (SPU_AKF_BASE_ADDR + 0x24))
#define rSPU_AKF_AXI_CPU_CTRL          (*(volatile uint32_t *) (SPU_AKF_BASE_ADDR + 0x28))

#define SP_AKF_AXI_CPU_CTRL_RUN			(1 << 4)

#endif /* ! __PLATFORM_SOC_SPU_H */
