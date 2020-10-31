/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/dcs/dcs.h>
#include <drivers/dcs/dcs_regs.h>
#include <drivers/dcs/dcs_init_lib.h>
#include <drivers/dram.h>
#include <drivers/miu.h>
#include <sys.h>
#include <platform/soc/hwclocks.h>

int mcu_initialize_dram (bool resume)
{
	dcs_boottype_t boot_type;

	// Don't perform DRAM initializion if the memory controller is already
	// initialized (e.g., for SecureROM validation testing).
	if (resume || DCS_REG_READ_CH(0, rDCS_MCU_AMCCTRL) == 0) {
		clock_gate(CLK_AMC, true);

		boot_type = (resume)? DCS_BOOT_RESUME:DCS_BOOT_COLDBOOT;
		// NOTE: Boot types AOP_DDR or AOP_AWAKE do not use this Code
		//       For those cases, the Init sequence is executed by the AOP

		// Perform the LPDDR4 Init for this type of Boot
		dcs_init(boot_type);
	}

	return 0;
}

void mcu_bypass_prep (int step)
{
}

uint64_t mcu_get_memory_size (void)
{
	return dcs_get_memory_size();
}

//================================================================================
void dcs_enable_slow_boot (bool enable)
{
	if (enable) {
		// switch to slow clock
		clocks_set_performance(kPerformanceMemoryLow);
		spin(1);
	}
	else {
#if !SUPPORT_FPGA
		// switch back to full-speed
		clocks_set_performance(kPerformanceMemoryFull);
		spin(1);
#endif		
	}
}

//================================================================================
void dcs_dram_workarounds(dcs_boottype_t boot_type)
{
	/* No workarounds yet */
}

//================================================================================
// Grab (Platform Specific) Tables with Definitions of Values for DCS Init & DCS Tunables
#include <platform/dcsconfig.h>

#if DCS_FIXUP_PARAMS
// Grab (Target Specific) Definition of Required Fixups to the Values for DCS Init
#include <target/dcsfixup.h>
#endif
//================================================================================
void dcs_init_config_params(dcs_config_params_t *use_dcs_params, dcs_target_t targtype)
{
	memcpy(use_dcs_params, &dcs_params, sizeof(dcs_config_params_t));

#if DCS_FIXUP_PARAMS
	// This gives the particular target a chance to make target-specific changes
	dcs_init_config_fixup_params(use_dcs_params, targtype);
#endif
}

//================================================================================
const dcs_tunable_t *dcs_init_tunable_table(uint32_t *table_len)
{
	if (table_len)
		*table_len = sizeof(dcs_tunables) / sizeof(dcs_tunables[0]);

	return dcs_tunables;
}

//================================================================================
// Save off calibration results to AOP_SRAM region (later to be stitched into reconfig sequence)
// Order of registers to be saved must match the order to be restored
volatile uint32_t *dcs_save_calibration_results(volatile uint32_t *save_region, uint32_t freq)
{
	/* TBD */
	return NULL;
}

//================================================================================
// Stitch the saved calibration results into the reconfig sequence
// To be called immediately after stitching mcu_aop_awake_reconfig_pre_restore
// Order of registers to be restored matches the order in the reconfig sequence
void dcs_restore_calibration_results(volatile uint32_t *save_region)
{
	/* TBD */
}


//================================================================================
