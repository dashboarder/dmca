/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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

#define	rCFG_FUSE0			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x00))
#define	rCFG_FUSE1			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x04))
#define	rCFG_FUSE2			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x08))
#define	rCFG_FUSE3			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x0C))
#define	rCFG_FUSE4			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x10))
#define	rCFG_FUSE5			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x14))

#define	rECIDLO				(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x20))
#define	rECIDHI				(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x24))

#define rAP_SECURITY			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x40))
#define rSEP_SECURITY			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x44))

#define rCFG_FUSE0_RAW			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x100))

uint32_t chipid_get_soc_voltage(uint32_t index);
uint32_t chipid_get_gpu_voltage(uint32_t index);

#define CHIPID_CPU_VOLTAGE_BYPASS		(0)
#define CHIPID_CPU_VOLTAGE_SECUREROM	(1)
#define CHIPID_CPU_VOLTAGE_V0			(2)
#define CHIPID_CPU_VOLTAGE_V1			(3)
#define CHIPID_CPU_VOLTAGE_V2			(4)
#define CHIPID_CPU_VOLTAGE_V3			(5)
#define CHIPID_CPU_VOLTAGE_V4			(6)

// CPU Boot Voltage - Should be cleaned up since not used for H6
#if SUB_TARGET_N51 || SUB_TARGET_N53 || SUB_TARGET_J85 || SUB_TARGET_J86 || SUB_TARGET_J87 || SUB_TARGET_J85M || SUB_TARGET_J86M || SUB_TARGET_J87M
#define CHIPID_CPU_VOLTAGE_MED		(CHIPID_CPU_VOLTAGE_V2)
#else
#define CHIPID_CPU_VOLTAGE_MED		(CHIPID_CPU_VOLTAGE_V1)
#endif

uint32_t chipid_get_cpu_voltage(uint32_t index);

uint32_t chipid_get_ram_voltage(uint32_t index);

int32_t chipid_get_cpu_temp_offset(uint32_t cpu_number);

uint32_t chipid_get_fused_pmgr_thermal_sensor_cal_70C(uint32_t sensorID);
uint32_t chipid_get_fused_pmgr_thermal_sensor_cal_25C(uint32_t sensorID);
uint32_t chipid_get_fused_ccc_thermal_sensor_cal_70C(uint32_t sensorID);
uint32_t chipid_get_fused_ccc_thermal_sensor_cal_25C(uint32_t sensorID);
uint32_t chipid_get_fuse_revision(void);

uint32_t chipid_get_total_rails_leakage();

bool chipid_valid_thermal_sensor_cal_data_expected(void);	// <rdar://problem/13127987>

#endif /* __PLATFORM_SOC_CHIPID_H */
