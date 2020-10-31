/*
 * Copyright (C) 2010-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_SOC_CHIPID_H
#define __PLATFORM_SOC_CHIPID_H

#include <platform/chipid.h>
#include <platform/soc/hwregbase.h>

#define	rCFG_FUSE0			(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x00))
#define	rCFG_FUSE1			(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x04))
#define	rCFG_FUSE2			(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x08))
#define	rCFG_FUSE3			(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x0C))
#define	rCFG_FUSE4			(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x18))
#define	rCFG_FUSE5			(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x1C))
#define	rCFG_FUSE6			(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x20))
#define	rCFG_FUSE7			(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x24))

#define	rECIDLO				(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x10))
#define	rECIDHI				(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x14))

extern bool	chipid_get_ecid_image_personalization_required(void);

#define CHIPID_SOC_VOLTAGE_LOW		(0)
#define CHIPID_SOC_VOLTAGE_MED		(1)
#define CHIPID_SOC_VOLTAGE_HIGH		(2)
extern u_int32_t chipid_get_soc_voltage(u_int32_t index);

#define CHIPID_CPU_VOLTAGE_LOW		(0)
#define CHIPID_CPU_VOLTAGE_MED		(1)
#define CHIPID_CPU_VOLTAGE_HIGH		(2)
extern u_int32_t chipid_get_cpu_voltage(u_int32_t index);

extern u_int32_t chipid_get_fused_thermal_70C(void);
extern u_int32_t chipid_get_fused_thermal_25C(void);
extern u_int32_t chipid_get_fuse_revision(void);

#endif /* __PLATFORM_SOC_CHIPID_H */
