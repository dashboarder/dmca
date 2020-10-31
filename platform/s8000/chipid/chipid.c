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

#include <debug.h>
#include <platform.h>
#include <platform/pmgr.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>


#if SUB_PLATFORM_S8000
#define MINIMUM_FUSE_REVISION	0x8
#elif SUB_PLATFORM_S8001
#define MINIMUM_FUSE_REVISION   0x6
#elif SUB_PLATFORM_S8003
#define MINIMUM_FUSE_REVISION   0x3
#else
#error "Unknown platform"
#endif

bool chipid_get_current_production_mode(void)
{
	return MINIPMGR_FUSE_CFG_FUSE0_PRODUCTION_MODE_XTRCT(rCFG_FUSE0) != 0;
}

bool chipid_get_raw_production_mode(void)
{
	return MINIPMGR_FUSE_CFG_FUSE0_PRODUCTION_MODE_XTRCT(rCFG_FUSE0_RAW) != 0;
}

void chipid_clear_production_mode(void)
{
	rCFG_FUSE0 &= ~MINIPMGR_FUSE_CFG_FUSE0_PRODUCTION_MODE_UMASK;
}

bool chipid_get_secure_mode(void)
{
	// demotion only applies to the SEP, so iBoot always reads
	// the raw value for secure mode (<rdar://problem/15182573>)
	return MINIPMGR_FUSE_CFG_FUSE0_SECURE_MODE_XTRCT(rCFG_FUSE0_RAW);
}

uint32_t chipid_get_security_domain(void)
{
	return MINIPMGR_FUSE_CFG_FUSE0_SECURITY_DOMAIN_XTRCT(rCFG_FUSE0);
}

uint32_t chipid_get_board_id(void)
{
	return MINIPMGR_FUSE_CFG_FUSE0_BID_XTRCT(rCFG_FUSE0);
}

uint32_t chipid_get_minimum_epoch(void)
{
	return MINIPMGR_FUSE_CFG_FUSE0_MINIMUM_EPOCH_XTRCT(rCFG_FUSE0);
}

uint32_t chipid_get_chip_id(void)
{
#if SUB_PLATFORM_S8000
        return 0x8000;
#elif SUB_PLATFORM_S8001
        return 0x8001;
#elif SUB_PLATFORM_S8003
        return 0x8003;
#else
#error "Unknown platform"
#endif
}

uint32_t chipid_get_chip_revision(void)
{
	// we use 4 bits for base layer and 4 bits for metal,
	// the fuses use 3 for each
#if defined(MINIPMGR_FUSE_CFG_FUSE4_DEV_VERSION_XTRCT)
	uint32_t fuse_val = MINIPMGR_FUSE_CFG_FUSE4_DEV_VERSION_XTRCT(rCFG_FUSE4);

	return (fuse_val & 0x7) | (((fuse_val >> 3) & 0x7) << 4);
#elif defined(MINIPMGR_FUSE_CFG_FUSE4_CHIP_REV_MAJOR_XTRCT)
	uint32_t fuse = rCFG_FUSE4;
	return ((MINIPMGR_FUSE_CFG_FUSE4_CHIP_REV_MAJOR_XTRCT(fuse) << 4) |
		(MINIPMGR_FUSE_CFG_FUSE4_CHIP_REV_MINOR_XTRCT(fuse)));
#else
#error "SPDS doesn't contain expected chip rev extraction defines"
#endif
}

uint32_t chipid_get_osc_frequency(void)
{
	return OSC_FREQ;
}

uint64_t chipid_get_ecid_id(void)
{
	return ((uint64_t)rECIDHI << 32) | rECIDLO;
}

uint64_t chipid_get_die_id(void)
{
	return ((uint64_t)rECIDHI << 32) | rECIDLO;
}

uint32_t chipid_get_cpu_voltage(uint32_t index)
{
	uint32_t voltage = pmgr_binning_get_mv(index, false, chipid_get_fuse_revision() >= MINIMUM_FUSE_REVISION);

	if (voltage == PMGR_BINNING_NOTFOUND)
		panic("Invalid CPU voltage index %d\n", index);

	return voltage;

}

uint32_t chipid_get_cpu_sram_voltage(uint32_t index)
{
	uint32_t voltage = pmgr_binning_get_mv(index, true, chipid_get_fuse_revision() >= MINIMUM_FUSE_REVISION);

	if (voltage == PMGR_BINNING_NOTFOUND)
		panic("Invalid CPU SRAM voltage index %d\n", index);

	return voltage;
}

uint32_t chipid_get_soc_voltage(uint32_t index)
{
	uint32_t voltage;

#if SUB_PLATFORM_S8001
#if SUB_TARGET_J99A | SUB_TARGET_J98A
	index = CHIPID_SOC_VOLTAGE_VMIN;
#else
	index = CHIPID_SOC_VOLTAGE_VNOM;
#endif
#endif
	voltage = pmgr_binning_get_mv(index, false, chipid_get_fuse_revision() >= MINIMUM_FUSE_REVISION);

	if (voltage == PMGR_BINNING_NOTFOUND)
		panic("Invalid SOC voltage index %d\n", index);

	return voltage;
}

uint32_t chipid_get_gpu_voltage(uint32_t index)
{
	uint32_t voltage = pmgr_binning_get_mv(index, false, chipid_get_fuse_revision() >= MINIMUM_FUSE_REVISION);

	if (voltage == PMGR_BINNING_NOTFOUND)
		panic("Invalid GPU voltage index %d\n", index);

	return voltage;

}

uint32_t chipid_get_gpu_sram_voltage(uint32_t index)
{
	uint32_t voltage = pmgr_binning_get_mv(index, true, chipid_get_fuse_revision() >= MINIMUM_FUSE_REVISION);

	if (voltage == PMGR_BINNING_NOTFOUND)
		panic("Invalid GPU SRAM voltage index %d\n", index);

	return voltage;
}

uint32_t chipid_get_sram_voltage(uint32_t index)
{
	uint32_t voltage = pmgr_binning_get_mv(CHIPID_VOLTAGE_FIXED, true, chipid_get_fuse_revision() >= MINIMUM_FUSE_REVISION);

	if (voltage == PMGR_BINNING_NOTFOUND)
		panic("Invalid SRAM voltage index %d\n", index);

	return voltage;
}

bool chipid_get_fuse_lock(void)
{
	return MINIPMGR_FUSE_CFG_FUSE1_AP_LOCK_XTRCT(rCFG_FUSE1) != 0;
}

void chipid_set_fuse_lock(bool locked)
{
	if (locked) {
		rCFG_FUSE1 |= MINIPMGR_FUSE_CFG_FUSE1_AP_LOCK_INSRT(1);
		asm("dsb sy");
		if (!chipid_get_fuse_lock()) {
			panic("Failed to lock fuses\n");
		}
	}
}

bool chipid_get_fuse_seal(void)
{
	return MINIPMGR_FUSE_CFG_FUSE1_SEAL_FUSES_XTRCT(rCFG_FUSE1) != 0;
}

uint32_t chipid_get_lpo_trim(void)
{
#if SUB_PLATFORM_S8000
	// <rdar://problem/18530570> LPO clock doesn't lock with fused TRIM value but does with 0 trim
	if (chipid_get_fuse_revision() < 3)
		return 0x20;
	else
#endif
		return MINIPMGR_FUSE_CFG_FUSE2_LPO_TRIM_XTRCT(rCFG_FUSE2);
}

#if SUB_PLATFORM_S8000
uint32_t chipid_get_pcie_txpll_vco_v2i_i_set(void)
{
	return MINIPMGR_FUSE_CFG_FUSE3_PCIE_TXPLL_VCO_V2I_I_SET_XTRCT(rCFG_FUSE3);
}

uint32_t chipid_get_pcie_txpll_vco_v2i_pi_set(void)
{
	return MINIPMGR_FUSE_CFG_FUSE3_PCIE_TXPLL_VCO_V2I_PI_SET_XTRCT(rCFG_FUSE3);
}

uint32_t chipid_get_pcie_refpll_vco_v2i_i_set(void)
{
	return MINIPMGR_FUSE_CFG_FUSE4_PCIE_REFPLL_VCO_V2I_I_SET_XTRCT(rCFG_FUSE4);
}

uint32_t chipid_get_pcie_refpll_vco_v2i_pi_set(void)
{
	return MINIPMGR_FUSE_CFG_FUSE4_PCIE_REFPLL_VCO_V2I_PI_SET_XTRCT(rCFG_FUSE4);
}

uint32_t chipid_get_pcie_rx_ldo(void)
{
	return MINIPMGR_FUSE_CFG_FUSE3_PCIE_RX_LDO_XTRCT(rCFG_FUSE3);
}
#elif SUB_PLATFORM_S8001 || SUB_PLATFORM_S8003
uint32_t chipid_get_pcie_refpll_fcal_vco_digctrl(void)
{
	return MINIPMGR_FUSE_CFG_FUSE4_PCIE_REFPLL_FCAL_VCO_DIGCTRL_XTRCT(rCFG_FUSE4);
}
#endif

uint32_t chipid_get_soc_temp_sensor_trim(uint32_t sensor_index)
{
	uint32_t sensor_trim;

#if SUB_PLATFORM_S8000 && defined(MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN0_TRIMG_UMASK)

	// S8000 A1
	if (chipid_get_chip_revision() == CHIP_REVISION_A1) {
		switch (sensor_index) {
			case 0:
				return MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN0_XTRCT_V1(rCFG_FUSE2);
			case 1:
				return MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN1_XTRCT_V1(rCFG_FUSE2);
			case 2:
				return MINIPMGR_FUSE_CFG_FUSE3_THERMAL_SEN2_XTRCT_V1(rCFG_FUSE3);
			default:
				panic("invalid thermal sensor %u", sensor_index);
		}
	}
#endif

	// S8000 B0/C0, S8001 A0/B0, S8003 A0/A1
	switch (sensor_index) {
		case 0:
			sensor_trim = rCFG_FUSE2;
			sensor_trim &=  MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN0_TRIMG_UMASK | MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN0_TRIMO_UMASK;
			sensor_trim >>= MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN0_TRIMG_SHIFT;
			return sensor_trim;
		case 1:
			sensor_trim = rCFG_FUSE2;
			sensor_trim &=  MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN1_TRIMG_UMASK | MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN1_TRIMO_UMASK;
			sensor_trim >>= MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN1_TRIMG_SHIFT;
			return sensor_trim;
		case 2:
			sensor_trim = rCFG_FUSE3;
			sensor_trim &=  MINIPMGR_FUSE_CFG_FUSE3_THERMAL_SEN2_TRIMG_UMASK | MINIPMGR_FUSE_CFG_FUSE3_THERMAL_SEN2_TRIMO_UMASK;
			sensor_trim >>= MINIPMGR_FUSE_CFG_FUSE3_THERMAL_SEN2_TRIMG_SHIFT;
			return sensor_trim;

#if SUB_PLATFORM_S8001
		case 3:
			sensor_trim = rCFG_FUSE3;
			sensor_trim &=  MINIPMGR_FUSE_CFG_FUSE3_THERMAL_SEN3_TRIMG_UMASK | MINIPMGR_FUSE_CFG_FUSE3_THERMAL_SEN3_TRIMO_UMASK;
			sensor_trim >>= MINIPMGR_FUSE_CFG_FUSE3_THERMAL_SEN3_TRIMG_SHIFT;
			return sensor_trim;
#endif

		default:
			panic("invalid thermal sensor %u", sensor_index);
	}
}

uint32_t chipid_get_fuse_revision(void)
{
	uint32_t version = pmgr_binning_get_revision();
	if (version == PMGR_BINNING_NOTFOUND) {
		return 0;
	}
	return version;
}

uint32_t chipid_get_total_rails_leakage()
{
	// FIXME
	return 0;
}

#if SUPPORT_FPGA

#define FPGA_HAS_INT3	(FPGA_HAS_DISP | FPGA_HAS_MEDIA | FPGA_HAS_JPEG | FPGA_HAS_MSR | FPGA_HAS_VXD)

uint32_t chipid_get_fpga_block_instantiation(void)
{
	// Hardware blocks instantiated.
	uint32_t	blocks = (rECID_FUSE3 >> 18) & 0xF;
	uint32_t	mask = FPGA_HAS_ALWAYS;

	switch (blocks) {
		// INT2 := ACC + AF + AMC + SouthBridge + PCIE
		case 0x1:
			break;
			
		// INT2GFX := INT2 + GFX
		case 0x2:
			mask |= FPGA_HAS_GFX;
			break;
			
		// INT3 := INT2 + DISP + MEDIA + JPEG + MSR + VXD
		case 0x8:
			mask |= FPGA_HAS_INT3;
			break;
			
		// INT3GFX := INT3 + GFX
		case 0xA:
			mask |= (FPGA_HAS_INT3 | FPGA_HAS_GFX);
			break;
			
		default:
			panic("Unknown hardware block instantiation: 0x%x", blocks);
	}
	
	return mask;
}
#endif
