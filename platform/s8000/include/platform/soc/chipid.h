/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
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
#include SUB_PLATFORM_SPDS_HEADER(minipmgr)

#define	rCFG_FUSE0			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE0_OFFSET))
#define	rCFG_FUSE1			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE1_OFFSET))
#define	rCFG_FUSE2			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE2_OFFSET))
#define	rCFG_FUSE3			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE3_OFFSET))
#define	rCFG_FUSE4			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE4_OFFSET))
#define	rCFG_FUSE5			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE5_OFFSET))
#define	rCFG_FUSE9			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE9_OFFSET))

#define CFG_FUSE9_PLL_VCO_RCTRL_SEL	(0x8)	// VCO_RCTRL_OW value is valid if set
#define CFG_FUSE9_PLL_VCO_RCTRL_OW_MASK	(0x7)	// PLL analog override value if valid

#define	rECIDLO				(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_ECID_FUSE0_OFFSET))
#define	rECIDHI				(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_ECID_FUSE1_OFFSET))

#define rSEP_SECURITY			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_SEP_SECURITY_OFFSET))

#define rCFG_FUSE0_RAW			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE0_RAW_OFFSET))

#define BOARD_ID_NONE		(0xff)
#define J105_AP_BOARD_ID	(0x04)
#define J105_DEV_BOARD_ID	(0x05)
#define J98A_AP_BOARD_ID	(0x10)
#define J98A_DEV_BOARD_ID	(0x11)
#define J99A_AP_BOARD_ID	(0x12)
#define J99A_DEV_BOARD_ID	(0x13)
#define J127_AP_BOARD_ID	(0x8)
#define J127_DEV_BOARD_ID	(0x9)
#define J128_AP_BOARD_ID	(0xA)
#define J128_DEV_BOARD_ID	(0xB)
#define J111_AP_BOARD_ID	(0x8)
#define J111_DEV_BOARD_ID	(0x9)
#define J112_AP_BOARD_ID	(0xA)
#define J112_DEV_BOARD_ID	(0xB)
#define N69_AP_BOARD_ID		(0x2)
#define N69_DEV_BOARD_ID	(0x3)
#define N69U_AP_BOARD_ID	(0x2)
#define N69U_DEV_BOARD_ID	(0x3)

enum chipid_voltage_index {
	CHIPID_SOC_VOLTAGE_BYPASS =	0,
	CHIPID_SOC_VOLTAGE_SECUREROM = 1,
	CHIPID_SOC_VOLTAGE_VMIN = 2,
	CHIPID_SOC_VOLTAGE_VNOM	= 3,

	CHIPID_CPU_VOLTAGE_BYPASS,
	CHIPID_CPU_VOLTAGE_SECUREROM,
	CHIPID_CPU_VOLTAGE_396,

	CHIPID_CPU_VOLTAGE_600,
	CHIPID_CPU_VOLTAGE_912,
	CHIPID_CPU_VOLTAGE_1200,
	CHIPID_CPU_VOLTAGE_1512,
	CHIPID_CPU_VOLTAGE_1800,
	CHIPID_CPU_VOLTAGE_1848,
	CHIPID_CPU_VOLTAGE_1896,
	CHIPID_CPU_VOLTAGE_1992,
	CHIPID_CPU_VOLTAGE_2112,

	CHIPID_CPU_VOLTAGE_396_WA,
	CHIPID_CPU_VOLTAGE_1200_WA,
	CHIPID_CPU_VOLTAGE_1512_WA,

	CHIPID_CPU_VOLTAGE_720,
	CHIPID_CPU_VOLTAGE_1080,
	CHIPID_CPU_VOLTAGE_1440,
	CHIPID_CPU_VOLTAGE_2160,
	CHIPID_CPU_VOLTAGE_2256_1core,
	CHIPID_CPU_VOLTAGE_2256,
	CHIPID_CPU_VOLTAGE_2352,
	CHIPID_CPU_VOLTAGE_2448,

	CHIPID_GPU_VOLTAGE_OFF,

	CHIPID_GPU_VOLTAGE_340,
	CHIPID_GPU_VOLTAGE_474,
	CHIPID_GPU_VOLTAGE_550,
	CHIPID_GPU_VOLTAGE_723,
	CHIPID_GPU_VOLTAGE_616,
	CHIPID_GPU_VOLTAGE_804,
	CHIPID_GPU_VOLTAGE_850,
	CHIPID_GPU_VOLTAGE_900,

	CHIPID_GPU_VOLTAGE_360,
	CHIPID_GPU_VOLTAGE_520,
	CHIPID_GPU_VOLTAGE_650,
	CHIPID_GPU_VOLTAGE_800,

	CHIPID_VOLTAGE_FIXED,

	CHIPID_ALL_VOLTAGE_LAST,
};

enum chipid_voltage_type {
	CHIPID_CPU_VOLTAGE,
	CHIPID_GPU_VOLTAGE,
	CHIPID_SOC_VOLTAGE,
};

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

#if SUB_PLATFORM_S8000
uint32_t chipid_get_pcie_txpll_vco_v2i_i_set(void);
uint32_t chipid_get_pcie_txpll_vco_v2i_pi_set(void);
uint32_t chipid_get_pcie_refpll_vco_v2i_i_set(void);
uint32_t chipid_get_pcie_refpll_vco_v2i_pi_set(void);
uint32_t chipid_get_pcie_rx_ldo(void);
#elif SUB_PLATFORM_S8001 || SUB_PLATFORM_S8003
uint32_t chipid_get_pcie_refpll_fcal_vco_digctrl(void);
#endif

#if SUPPORT_FPGA

#define FPGA_HAS_ALWAYS		(1 << 0)
#define FPGA_HAS_VXD		(1 << 1)
#define FPGA_HAS_AVE		(1 << 2)
#define FPGA_HAS_ISP		(1 << 3)
#define FPGA_HAS_DISP		(1 << 4)
#define FPGA_HAS_MSR		(1 << 6)
#define FPGA_HAS_JPEG		(1 << 7)
#define FPGA_HAS_GFX		(1 << 8)
#define FPGA_HAS_MEDIA		(1 << 9)

#define rECID_FUSE3		(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_ECID_FUSE3_OFFSET))

uint32_t chipid_get_fpga_block_instantiation(void);

#endif	// SUPPORT_FPGA

#endif /* __PLATFORM_SOC_CHIPID_H */
