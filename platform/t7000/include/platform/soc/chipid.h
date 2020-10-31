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

#define	rCFG_FUSE0			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x00))
#define	rCFG_FUSE1			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x04))
#define	rCFG_FUSE2			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x08))
#define	rCFG_FUSE3			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x0C))
#define	rCFG_FUSE4			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x10))
#define	rCFG_FUSE5			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x14))
#define	rCFG_FUSE6			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x18))
#define	rCFG_FUSE7			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x1C))
#define	rCFG_FUSE8			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x20))
#define	rCFG_FUSE9			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x24))
#define rCFG_FUSE15			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x3C))

#if SUB_PLATFORM_T7001
#define CFG_FUSE8_PCIE_REF_PLL_VREG_ADJ_SHIFT	(24)
#define CFG_FUSE8_PCIE_REF_PLL_VREG_ADJ_MASK	(0x7)
#endif

#if SUB_PLATFORM_T7000
#define MINIMUM_FUSE_REVISION	0x9
#elif SUB_PLATFORM_T7001
// Fuse Rev 0x2 means Capri A1 or later.
// So all binning are against Capri A1 in "Capri Test Plan Version 1.1"
#define MINIMUM_FUSE_REVISION	0x2
// Fuse Rev 0x41 means Capri with updated binning for J99 P5 and P6.
#define MINIMUM_BINNED_GPU_P6_REVISION	0x41
#define P6_VOL_OFFSET_FROM_P4_BINNING 143
#endif

#define CFG_FUSE9_PLL_VCO_RCTRL_SEL	(0x8)	// VCO_RCTRL_OW value is valid if set
#define CFG_FUSE9_PLL_VCO_RCTRL_OW_MASK	(0x7)	// PLL analog override value if valid

#define CFG_FUSE9_PLL_0_SHIFT		(0)
#define CFG_FUSE9_PLL_1_SHIFT		(4)
#define CFG_FUSE9_PLL_2_SHIFT		(8)
#define CFG_FUSE9_PLL_3_SHIFT		(12)
#define CFG_FUSE9_PLL_4_SHIFT		(16)
#define CFG_FUSE9_PLL_5_SHIFT		(20)
#define CFG_FUSE9_PLL_CPU_SHIFT		(24)
#define CFG_FUSE9_PLL_LPDP_SHIFT	(28)

#define  CFG_FUSE2_THERMAL_SEN_MASK		(0x1f)
#define  CFG_FUSE2_THERMAL_SEN_SHIFT(_idx)	(5 * (_idx))
#define  CFG_FUSE2_THERMAL_SEN(_idx)		((rCFG_FUSE2 >> CFG_FUSE2_THERMAL_SEN_SHIFT(_idx)) & CFG_FUSE2_THERMAL_SEN_MASK)

#define	rECIDLO				(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x80))
#define	rECIDHI				(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x84))

#define rSEP_SECURITY			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x104))

#define rCFG_FUSE0_RAW			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x200))

#define CHIPID_SOC_VOLTAGE_BYPASS		(0)
#define CHIPID_SOC_VOLTAGE_SECUREROM		(1)
#define CHIPID_SOC_VOLTAGE_IBOOT_MEM_LOW_PERF	(2)
#define CHIPID_SOC_VOLTAGE_VMIN			(3)
#define CHIPID_SOC_VOLTAGE_VNOM			(4)

uint32_t chipid_get_soc_voltage(uint32_t index);


/* Wahtever is the platform CHIPID_GPU_VOLTAGE_V0_DIDT index
 * will always follows immediatly the last CHIPID_GPU_VOLTAGE_V[0-9]
 * So, CHIPID_GPU_VOLTAGE_V0_DIDT will also be the number of regular voltages,
 * i.e. all voltages excepts DIDT ones.*/
#if SUB_TARGET_N56 || SUB_TARGET_J42 || SUB_TARGET_J42D
#define CHIPID_GPU_VOLTAGE_OFF			(0)
#define CHIPID_GPU_VOLTAGE_V0			(1)
#define CHIPID_GPU_VOLTAGE_V1			(2)
#define CHIPID_GPU_VOLTAGE_V2			(3)
#define CHIPID_GPU_VOLTAGE_V3			(4)
#define CHIPID_GPU_VOLTAGE_V0_DIDT		(5)
#define CHIPID_GPU_VOLTAGE_V1_DIDT		(6)
#define CHIPID_GPU_VOLTAGE_V2_DIDT		(7)
#define CHIPID_GPU_VOLTAGE_V3_DIDT		(8)
#elif SUB_PLATFORM_T7001 || SUB_TARGET_J96 || SUB_TARGET_J97
#define CHIPID_GPU_VOLTAGE_OFF			(0)
#define CHIPID_GPU_VOLTAGE_V0			(1)
#define CHIPID_GPU_VOLTAGE_V1			(2)
#define CHIPID_GPU_VOLTAGE_V2			(3)
#define CHIPID_GPU_VOLTAGE_V3			(4)
#define CHIPID_GPU_VOLTAGE_V4			(5)
#define CHIPID_GPU_VOLTAGE_V5			(6)
#define CHIPID_GPU_VOLTAGE_V0_DIDT		(7)
#define CHIPID_GPU_VOLTAGE_V1_DIDT		(8)
#define CHIPID_GPU_VOLTAGE_V2_DIDT		(9)
#define CHIPID_GPU_VOLTAGE_V3_DIDT		(10)
#define CHIPID_GPU_VOLTAGE_V4_DIDT		(11)
#define CHIPID_GPU_VOLTAGE_V5_DIDT		(12)
#define CHIPID_GPU_VOLTAGE_V_UVD		CHIPID_GPU_VOLTAGE_V5
#else
#define CHIPID_GPU_VOLTAGE_OFF			(0)
#define CHIPID_GPU_VOLTAGE_V0			(1)
#define CHIPID_GPU_VOLTAGE_V1			(2)
#define CHIPID_GPU_VOLTAGE_V2			(3)
#define CHIPID_GPU_VOLTAGE_V0_DIDT		(4)
#define CHIPID_GPU_VOLTAGE_V1_DIDT		(5)
#define CHIPID_GPU_VOLTAGE_V2_DIDT		(6)
#endif

uint32_t chipid_get_gpu_voltage(uint32_t index);

#define CHIPID_CPU_VOLTAGE_BYPASS		(0)
#define CHIPID_CPU_VOLTAGE_SECUREROM		(1)
#define CHIPID_CPU_VOLTAGE_V0			(2)
#define CHIPID_CPU_VOLTAGE_V1			(3)
#define CHIPID_CPU_VOLTAGE_V2			(4)
#define CHIPID_CPU_VOLTAGE_V3			(5)
#define CHIPID_CPU_VOLTAGE_V4			(6)
#define CHIPID_CPU_VOLTAGE_V5			(7)
// Currently pmgr expect any CHIPID_CPU_VOLTAGE_V* to be
// an index in the DVFM table. So, between 0 and 7.
// It will no more be the case for 1.6GHz
// who would be CHIPID_CPU_VOLTAGE_V6 or CHIPID_CPU_VOLTAGE_V6_UNBINNED.
#define CHIPID_CPU_VOLTAGE_V6			(8)
#define CHIPID_CPU_VOLTAGE_V6_UNBINNED	(9)


uint32_t chipid_get_cpu_voltage(uint32_t index);

uint32_t chipid_get_ram_voltage(uint32_t index);

uint32_t chipid_get_gpu_ram_voltage(uint32_t index);

int32_t chipid_get_cpu_temp_offset(uint32_t cpu_number);

uint32_t chipid_get_soc_temp_sensor_trim(uint32_t sensor_index);

uint32_t chipid_get_fuse_revision(void);

uint32_t chipid_get_total_rails_leakage(void);

enum chipid_vol_adj_type {
	CHIPID_VOL_ADJ_CPU,
	CHIPID_VOL_ADJ_SOC,	
};

struct chipid_vol_adj {
	uint32_t region_uV[4];
};
void chipid_get_vol_adj(enum chipid_vol_adj_type vol_adj_type, uint32_t index, struct chipid_vol_adj * vol_adj);

#if SUPPORT_FPGA

#define FPGA_HAS_ALWAYS		(1 << 0)
#define FPGA_HAS_VXD		(1 << 1)
#define FPGA_HAS_AVE		(1 << 2)
#define FPGA_HAS_ISP		(1 << 3)
#define FPGA_HAS_DISP0		(1 << 4)
#define FPGA_HAS_DISP1		(1 << 5)
#define FPGA_HAS_MSR		(1 << 6)
#define FPGA_HAS_JPEG		(1 << 7)
#define FPGA_HAS_GFX		(1 << 8)
#define FPGA_HAS_MEDIA		(1 << 9)
#define FPGA_HAS_PCIE		(1 << 10)

#define rECID_FUSE7		(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x9c))

uint32_t chipid_get_fpga_block_instantiation(void);

#endif	// SUPPORT_FPGA

#endif /* __PLATFORM_SOC_CHIPID_H */
