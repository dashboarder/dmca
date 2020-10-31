/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
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
#include <platform/soc/pmgr.h>
#include <platform/soc/hwregbase.h>
#include <soc/t8010/a0/minipmgr.h>

#define	rCFG_FUSE0			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE0_OFFSET))
#define	rCFG_FUSE1			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE1_OFFSET))
#define	rCFG_FUSE2			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE2_OFFSET))
#define	rCFG_FUSE3			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE3_OFFSET))
#define	rCFG_FUSE4			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE4_OFFSET))
#define	rCFG_FUSE5			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE5_OFFSET))
#define	rCFG_FUSE9			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE9_OFFSET))

#define	rECIDLO				(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_ECID_FUSE0_OFFSET))
#define	rECIDHI				(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_ECID_FUSE1_OFFSET))

#define rSEP_SECURITY			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_SEP_SECURITY_OFFSET))

#define rCFG_FUSE0_RAW			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE0_RAW_OFFSET))

 enum chipid_voltage_index {
	CHIPID_CPU_VOLTAGE_BYPASS,
	CHIPID_CPU_VOLTAGE_SECUREROM,
	CHIPID_CPU_VOLTAGE_396,

	CHIPID_GPU_VOLTAGE_OFF,

	CHIPID_ALL_VOLTAGE_LAST,
};

enum chipid_voltage_type {
	CHIPID_CPU_VOLTAGE,
	CHIPID_GPU_VOLTAGE,
};

struct chipid_vol_adj {
	uint64_t volAdj0:20;
	uint64_t volAdj1:20;
	uint64_t volAdj2:20;
	uint64_t volAdj3:20;
	uint64_t dvfmMaxAdj:20;
	uint64_t dvmrAdj0:20;
	uint64_t dvmrAdj1:20;
	uint64_t dvmrAdj2:20;
};

#define CHIPID_SOC_VOLTAGE_BYPASS		(0)
#define CHIPID_SOC_VOLTAGE_SECUREROM	(1)
#define CHIPID_SOC_VOLTAGE_VMIN			(2)
#define CHIPID_SOC_VOLTAGE_VNOM			(3)

extern const struct chipid_vol_adj *chipid_get_vol_adj(enum chipid_voltage_index voltage_index);

uint32_t chipid_get_cpu_voltage(uint32_t index);

uint32_t chipid_get_cpu_sram_voltage(uint32_t index);

uint32_t chipid_get_soc_voltage(uint32_t index);

uint32_t chipid_get_gpu_voltage(uint32_t index);

uint32_t chipid_get_gpu_sram_voltage(uint32_t index);

uint32_t chipid_get_sram_voltage(uint32_t index);

int32_t chipid_get_cpu_temp_offset(uint32_t cpu_number);

uint32_t chipid_get_soc_temp_sensor_trim(uint32_t sensor_index);

uint32_t chipid_get_fuse_revision(void);

uint32_t chipid_get_total_rails_leakage();

uint32_t chipid_get_lpo_trim(void);

uint32_t chipid_get_pcie_refpll_fcal_vco_digctrl(void);

#if SUPPORT_FPGA

#define FPGA_HAS_ALWAYS		(1 << 31)
#define FPGA_HAS_MEDIA		(1 << 0)
#define FPGA_HAS_MSR		(1 << 1)
#define FPGA_HAS_JPEG		(1 << 2)
#define FPGA_HAS_AVE		(1 << 3)
#define FPGA_HAS_VXD		(1 << 4)
#define FPGA_HAS_ISP		(1 << 5)
#define FPGA_HAS_RTBUSMUX	(1 << 6)
#define FPGA_HAS_DISP		(1 << 7)
#define FPGA_HAS_GFX		(1 << 8)

#define rECID_FUSE3		(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_ECID_FUSE3_OFFSET))

uint32_t chipid_get_fpga_block_instantiation(void);

#endif	// SUPPORT_FPGA

#endif /* __PLATFORM_SOC_CHIPID_H */
