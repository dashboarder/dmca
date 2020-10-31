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

#include <debug.h>
#include <platform.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>

#define MINIMUM_FUSE_REVISION	0x0

#define CHIPID_VOLTAGE_FIXED	0
#define CHIPID_MODE_NONE	0

struct chipid_voltage_config {
	uint32_t	safe_voltage;
	uint32_t	mode;
};

static struct chipid_voltage_config chipid_cpu_voltages[] = {
	[CHIPID_CPU_VOLTAGE_BYPASS]		= { 610, CHIPID_MODE_NONE},
	[CHIPID_CPU_VOLTAGE_SECUREROM]		= { 610, CHIPID_MODE_NONE},
	[CHIPID_CPU_VOLTAGE_396]		= { 610, CHIPID_MODE_NONE},
};

static struct chipid_voltage_config chipid_cpu_sram_voltages[] = {
	[CHIPID_CPU_VOLTAGE_BYPASS]		= { 800, CHIPID_MODE_NONE},
	[CHIPID_CPU_VOLTAGE_SECUREROM]		= { 800, CHIPID_MODE_NONE},
	[CHIPID_CPU_VOLTAGE_396]		= { 800, CHIPID_MODE_NONE},
};

static struct chipid_voltage_config chipid_soc_voltages[] = {
	[CHIPID_SOC_VOLTAGE_BYPASS]		= {725, CHIPID_MODE_NONE},
	[CHIPID_SOC_VOLTAGE_SECUREROM]		= {725, CHIPID_MODE_NONE},
	[CHIPID_SOC_VOLTAGE_VMIN]		= {725, CHIPID_MODE_NONE},
	[CHIPID_SOC_VOLTAGE_VNOM]		= {825, CHIPID_MODE_NONE},
};

static struct chipid_voltage_config chipid_gpu_voltages[] = {
	[CHIPID_GPU_VOLTAGE_OFF]		= {  0, CHIPID_MODE_NONE},
};

static struct chipid_voltage_config chipid_gpu_sram_voltages[] = {
	[CHIPID_GPU_VOLTAGE_OFF]		= {  0, CHIPID_MODE_NONE},
};

static struct chipid_voltage_config chipid_sram_voltages[] = {
	[CHIPID_VOLTAGE_FIXED]			= {850, CHIPID_MODE_NONE},
};

struct chipid_voltadj_entry {
	uint64_t voltage_index:8;
	uint64_t chipid:32;
	uint64_t chip_rev_min:8;
	uint64_t fuse_rev_min:8;
	struct chipid_vol_adj voltages;
};

#define CHIPID_ALL 0x0

// .voltages = {volAdj0, volAdj1, volAdj2, volAdj3, dvfmMaxAdj, dvmrAdj0, dvmrAdj1, dvmrAdj2}

static const struct chipid_voltadj_entry chipid_voltadj_entry[] = {
	{CHIPID_CPU_VOLTAGE_BYPASS,    CHIPID_ALL, CHIP_REVISION_A0, 0, .voltages = {0, 0, 0, 0, 0, 0, 0, 0}},
	{CHIPID_SOC_VOLTAGE_SECUREROM, CHIPID_ALL, CHIP_REVISION_A0, 0, .voltages = {0, 0, 0, 0, 0, 0, 0, 0}},
	{CHIPID_CPU_VOLTAGE_396,       0x8010,     CHIP_REVISION_A0, 0, .voltages = {0, 0, 0, 0, 0, 0, 0, 0}},
};

const struct chipid_vol_adj *chipid_get_vol_adj(enum chipid_voltage_index voltage_index)
{
	uint32_t chipid = chipid_get_chip_id();
	uint32_t chip_rev = chipid_get_chip_revision();
	uint32_t fuse_rev = chipid_get_fuse_revision();

	for (size_t i = 0; i < sizeof(chipid_voltadj_entry)/sizeof(chipid_voltadj_entry[0]); i++) {
		if (voltage_index != chipid_voltadj_entry[i].voltage_index) {
			continue;
		}
		if ((chipid != chipid_voltadj_entry[i].chipid) && (chipid_voltadj_entry[i].chipid != CHIPID_ALL)) {
			continue;
		}
		if ((chip_rev < chipid_voltadj_entry[i].chip_rev_min)) {
			continue;
		}
		if ((fuse_rev < chipid_voltadj_entry[i].fuse_rev_min)) {
			continue;
		}
		return &chipid_voltadj_entry[i].voltages;
	}
	return NULL;
}

static uint32_t chipid_get_base_voltage(void)
{
	uint32_t voltage = 0;

	// TODO

	return voltage;
}

static uint32_t chipid_get_cpu_binfuse_for_mode(uint32_t mode)
{
	uint32_t binfuse = 0;

	// TODO

	return binfuse;
}

static uint32_t chipid_get_cpu_sram_binfuse_for_mode(uint32_t mode)
{
	uint32_t binfuse = 0;

	// TODO

	return binfuse;
}

static uint32_t chipid_get_soc_binfuse_for_mode(uint32_t mode)
{
	uint32_t binfuse = 0;

	// TODO

	return binfuse;
}

static uint32_t chipid_get_gpu_binfuse_for_mode(uint32_t mode)
{
	uint32_t binfuse = 0;

	// TODO

	return binfuse;
}

static uint32_t chipid_get_gpu_sram_binfuse_for_mode(uint32_t mode)
{
	uint32_t binfuse = 0;

	// TODO

	return binfuse;
}

static uint32_t chipid_get_sram_binfuse_for_mode(uint32_t mode)
{
	uint32_t binfuse = 0;

	// TODO

	return binfuse;
}

static uint32_t chipid_get_cpu_bin_voltage(uint32_t volt_index)
{
	uint32_t mode, binfuse;

	mode = chipid_cpu_voltages[volt_index].mode;

	// Return safe voltage if no binned voltage
	if (mode == CHIPID_MODE_NONE)
		return chipid_cpu_voltages[volt_index].safe_voltage;

	binfuse = chipid_get_cpu_binfuse_for_mode(mode);

	return (chipid_get_base_voltage() + binfuse * 5);
}

static uint32_t chipid_get_cpu_sram_bin_voltage(uint32_t volt_index)
{
	uint32_t mode, binfuse;

	mode = chipid_cpu_sram_voltages[volt_index].mode;

	// Return safe voltage if no binned voltage
	if (mode == CHIPID_MODE_NONE)
		return chipid_cpu_sram_voltages[volt_index].safe_voltage;

	binfuse = chipid_get_cpu_sram_binfuse_for_mode(mode);

	return (chipid_get_base_voltage() + binfuse * 5);
}

static uint32_t chipid_get_soc_bin_voltage(uint32_t volt_index)
{
	uint32_t mode, binfuse;

	mode = chipid_soc_voltages[volt_index].mode;

	// Return safe voltage if no binned voltage
	if (mode == CHIPID_MODE_NONE)
		return chipid_soc_voltages[volt_index].safe_voltage;

	binfuse = chipid_get_soc_binfuse_for_mode(mode);

	return (chipid_get_base_voltage() + binfuse * 5);
}

static uint32_t chipid_get_gpu_bin_voltage(uint32_t volt_index)
{
	uint32_t mode, binfuse;

	mode = chipid_gpu_voltages[volt_index].mode;

	// Return safe voltage if no binned voltage
	if (mode == CHIPID_MODE_NONE)
		return chipid_gpu_voltages[volt_index].safe_voltage;

	binfuse = chipid_get_gpu_binfuse_for_mode(mode);

	return (chipid_get_base_voltage() + binfuse * 5);
}

static uint32_t chipid_get_gpu_sram_bin_voltage(uint32_t volt_index)
{
	uint32_t mode, binfuse;

	mode = chipid_gpu_sram_voltages[volt_index].mode;

	// Return safe voltage if no binned voltage
	if (mode == CHIPID_MODE_NONE)
		return chipid_gpu_sram_voltages[volt_index].safe_voltage;

	binfuse = chipid_get_gpu_sram_binfuse_for_mode(mode);

	return (chipid_get_base_voltage() + binfuse * 5);
}

static uint32_t chipid_get_sram_bin_voltage(uint32_t volt_index)
{
	uint32_t mode, binfuse;

	mode = chipid_sram_voltages[volt_index].mode;

	// Return safe voltage if no binned voltage
	if (mode == CHIPID_MODE_NONE)
		return chipid_sram_voltages[volt_index].safe_voltage;

	binfuse = chipid_get_sram_binfuse_for_mode(mode);

	return (chipid_get_base_voltage() + binfuse * 5);
}

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
#if SUB_PLATFORM_T8010
        return 0x8010;
#else
#error "Unknown platform"
#endif
}

uint32_t chipid_get_chip_revision(void)
{
	uint32_t fuse_val = rCFG_FUSE4;

	return (MINIPMGR_FUSE_CFG_FUSE4_CHIP_REV_MAJOR_XTRCT(fuse_val) << 4) |
	       (MINIPMGR_FUSE_CFG_FUSE4_CHIP_REV_MINOR_XTRCT(fuse_val));
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
	uint32_t cpu_voltage;

	if (index >= sizeof(chipid_cpu_voltages)/sizeof(chipid_cpu_voltages[0]))
		panic("Invalid CPU voltage index %d\n", index);

	if (chipid_get_fuse_revision() < MINIMUM_FUSE_REVISION)
		cpu_voltage = chipid_cpu_voltages[index].safe_voltage;
	else
		cpu_voltage = chipid_get_cpu_bin_voltage(index);

	return cpu_voltage;
}

uint32_t chipid_get_cpu_sram_voltage(uint32_t index)
{
	uint32_t cpu_sram_voltage;

	if (index >= sizeof(chipid_cpu_sram_voltages)/sizeof(chipid_cpu_sram_voltages[0]))
		panic("Invalid CPU SRAM voltage index %d\n", index);

	if (chipid_get_fuse_revision() < MINIMUM_FUSE_REVISION)
		cpu_sram_voltage = chipid_cpu_sram_voltages[index].safe_voltage;
	else
		cpu_sram_voltage = chipid_get_cpu_sram_bin_voltage(index);

	return cpu_sram_voltage;
}

uint32_t chipid_get_soc_voltage(uint32_t index)
{
	uint32_t soc_voltage;

	if (index >= sizeof(chipid_soc_voltages)/sizeof(chipid_soc_voltages[0]))
		panic("Invalid SOC voltage index %d\n", index);

	if (chipid_get_fuse_revision() < MINIMUM_FUSE_REVISION)
		soc_voltage = chipid_soc_voltages[index].safe_voltage;
	else
		soc_voltage = chipid_get_soc_bin_voltage(index);

	return soc_voltage;
}

uint32_t chipid_get_gpu_voltage(uint32_t index)
{
	uint32_t gpu_voltage;

	if (index >= sizeof(chipid_gpu_voltages)/sizeof(chipid_gpu_voltages[0]))
		panic("Invalid GPU voltage index %d\n", index);

	if (chipid_get_fuse_revision() < MINIMUM_FUSE_REVISION)
		gpu_voltage = chipid_gpu_voltages[index].safe_voltage;
	else
		gpu_voltage = chipid_get_gpu_bin_voltage(index);

	return gpu_voltage;
}

uint32_t chipid_get_gpu_sram_voltage(uint32_t index)
{
	uint32_t gpu_sram_voltage;

	if (index >= sizeof(chipid_gpu_sram_voltages)/sizeof(chipid_gpu_sram_voltages[0]))
		panic("Invalid GPU SRAM voltage index %d\n", index);

	if (chipid_get_fuse_revision() < MINIMUM_FUSE_REVISION)
		gpu_sram_voltage = chipid_gpu_sram_voltages[index].safe_voltage;
	else
		gpu_sram_voltage = chipid_get_gpu_sram_bin_voltage(index);

	return gpu_sram_voltage;
}

uint32_t chipid_get_sram_voltage(uint32_t index)
{
	uint32_t sram_voltage;

	index = CHIPID_VOLTAGE_FIXED;

	if (index >= sizeof(chipid_sram_voltages)/sizeof(chipid_sram_voltages[0]))
		panic("Invalid SRAM voltage index %d\n", index);

	if (chipid_get_fuse_revision() < MINIMUM_FUSE_REVISION)
		sram_voltage = chipid_sram_voltages[index].safe_voltage;
	else
		sram_voltage = chipid_get_sram_bin_voltage(index);

	return sram_voltage;
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
	// <rdar://problem/18460311> Workaround for Maui LPO issue
	// return MINIPMGR_FUSE_CFG_FUSE2_LPO_TRIM_XTRCT(rCFG_FUSE2);
	return 0x20;
}

uint32_t chipid_get_pcie_refpll_fcal_vco_digctrl(void)
{
	return MINIPMGR_FUSE_CFG_FUSE4_PCIE_REFPLL_FCAL_VCO_DIGCTRL_XTRCT(rCFG_FUSE4);
}

uint32_t chipid_get_soc_temp_sensor_trim(uint32_t sensor_index)
{
	uint32_t sensor_trim;

	switch (sensor_index) {
		case 0:
			sensor_trim = rCFG_FUSE2;
			sensor_trim &=  MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN0_TRIMG_UMASK | MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN0_TRIMO_UMASK;
			sensor_trim >>= MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN0_TRIMG_SHIFT;
			break;
		case 1:
			sensor_trim = rCFG_FUSE2;
			sensor_trim &=  MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN1_TRIMG_UMASK | MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN1_TRIMO_UMASK;
			sensor_trim >>= MINIPMGR_FUSE_CFG_FUSE2_THERMAL_SEN1_TRIMG_SHIFT;
			break;
		case 2:
			sensor_trim = rCFG_FUSE3;
			sensor_trim &=  MINIPMGR_FUSE_CFG_FUSE3_THERMAL_SEN2_TRIMG_UMASK | MINIPMGR_FUSE_CFG_FUSE3_THERMAL_SEN2_TRIMO_UMASK;
			sensor_trim >>= MINIPMGR_FUSE_CFG_FUSE3_THERMAL_SEN2_TRIMG_SHIFT;
			break;
		default:
			panic("invalid thermal sensor %u", sensor_index);
	}
	return sensor_trim;
}

uint32_t chipid_get_fuse_revision(void)
{
	return MINIPMGR_FUSE_CFG_FUSE4_REV_XTRCT(rCFG_FUSE4);
}

uint32_t chipid_get_total_rails_leakage()
{
	// FIXME
	return 0;
}

#if SUPPORT_FPGA

#define FPGA_HAS_INT3	(FPGA_HAS_MEDIA | FPGA_HAS_MSR | FPGA_HAS_JPEG | FPGA_HAS_VXD | FPGA_HAS_DISP)

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
