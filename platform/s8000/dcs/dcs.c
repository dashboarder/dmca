/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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
#include <drivers/reconfig.h>
#include <sys.h>
#include <platform/soc/hwclocks.h>

int mcu_initialize_dram (bool resume)
{
	dcs_boottype_t boot_type;

	// Don't perform DRAM initializion if the memory controller is already
	// initialized (e.g., for SecureROM validation testing).
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	if (resume || DCS_REG_READ_CH(0, rDCS_MCU_AMCCTRL) == 0) {
#endif
		clock_gate(CLK_DCS0, true);
		clock_gate(CLK_DCS1, true);
		clock_gate(CLK_DCS2, true);
		clock_gate(CLK_DCS3, true);
#if SUB_PLATFORM_S8001 && DCS_NUM_CHANNELS > 4
		clock_gate(CLK_DCS4, true);
		clock_gate(CLK_DCS5, true);
		clock_gate(CLK_DCS6, true);
		clock_gate(CLK_DCS7, true);
#endif
		clock_gate(CLK_MCC, true);

		boot_type = (resume)? DCS_BOOT_RESUME:DCS_BOOT_COLDBOOT;
		// NOTE: Boot types AOP_DDR or AOP_AWAKE do not use this Code
		//       For those cases, the Init sequence is executed by the AOP

		// Perform the LPDDR4 Init for this type of Boot
		dcs_init(boot_type);

		// <rdar://problem/18602531> Maui A1: Samsung 25nm LP4 RBM failures - WA
		// Can't do this as part of dcs_dram_workarounds because reg value gets overwritten later in the sequence
		const struct dcs_memory_device_info *dev_info = dcs_get_memory_device_info();
		if(dev_info->vendor_id == JEDEC_LPDDR4_MANUF_ID_SAMSUNG)
			if(dev_info->rev_id == 0x3 && dev_info->rev_id2 == 0x0)
				for(uint32_t channel = 0; channel < DCS_NUM_CHANNELS; channel++)
					DCS_REG_ACCESS(rDCS_MCU_AREFEN_FREQ(0) + (channel * DCS_SPACING)) = 0x1010013f;
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	}
#endif

	return 0;
}

void dcs_lock_down_mcc(void)
{
	// Don't allow MCC as RAM
	rAMCC_MCCGEN &= ~AMCC_MCCGEN_MCC_RAM_EN;
	rAMCC_MCCGEN |= AMCC_MCCGEN_MCC_RAM_EN_LOCK;
	// Don't allow tag RAM reads
	rAMCC_MCCCFG_TAG_RD_DIS |= 1;
#if NUM_AMCCS > 1
	rAMCC1_MCCGEN &= ~AMCC_MCCGEN_MCC_RAM_EN;
	rAMCC1_MCCGEN |= AMCC_MCCGEN_MCC_RAM_EN_LOCK;
	rAMCC1_MCCCFG_TAG_RD_DIS |= 1;
#endif
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
// https://seg-docs.ecs.apple.com/projects/malta//release/UserManual/Hx_LPDDR4_RevID_Workaround.pdf
void dcs_dram_workarounds(dcs_boottype_t boot_type)
{
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
	uint32_t autoref_freq1;
#endif
#if SUB_PLATFORM_S8000
	uint32_t dqvref_freq1;
#endif

	if(boot_type == DCS_BOOT_COLDBOOT) {

		const struct dcs_memory_device_info *dev_info = dcs_get_memory_device_info();

		switch (dev_info->vendor_id) {
			case JEDEC_LPDDR4_MANUF_ID_SAMSUNG:
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
				if(dev_info->rev_id == 5 && dev_info->rev_id2 == 1) {
					// <rdar://problem/21042636>  tRFCpbCyc_freq1 = 0x1c for Samsung
					autoref_freq1 = DCS_REG_READ_CH(0, rDCS_MCU_AUTOREF_FREQ(1));
					autoref_freq1 &= 0x00ffffff;
					autoref_freq1 |= (0x1c << 24);
					dcs_reg_write_all_chan(rDCS_MCU_AUTOREF_FREQ(1), autoref_freq1);
				}
#endif
#if SUB_PLATFORM_S8000
				if(dev_info->rev_id == 5) {
					dqvref_freq1 = 0x00ec00ec; // Maui
					dcs_reg_write_all_chan(rDCS_AMP_VREF_F(AMP_DQ0, 1), dqvref_freq1);
					dcs_reg_write_all_chan(rDCS_AMP_VREF_F(AMP_DQ1, 1), dqvref_freq1);
				}
#endif
				break;

			case JEDEC_LPDDR4_MANUF_ID_HYNIX:
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
				if(dev_info->rev_id == 3 && dev_info->rev_id2 == 2) {
					// <rdar://problem/21042636>  tRFCpbCyc_freq1 = 0x1c for Hynix
					autoref_freq1 = DCS_REG_READ_CH(0, rDCS_MCU_AUTOREF_FREQ(1));
					autoref_freq1 &= 0x00ffffff;
					autoref_freq1 |= (0x1c << 24);
					dcs_reg_write_all_chan(rDCS_MCU_AUTOREF_FREQ(1), autoref_freq1);
				}
#endif
#if SUB_PLATFORM_S8000
				if(dev_info->rev_id == 3) {
					dqvref_freq1 = 0x00df00df; // Maui
					dcs_reg_write_all_chan(rDCS_AMP_VREF_F(AMP_DQ0, 1), dqvref_freq1);
					dcs_reg_write_all_chan(rDCS_AMP_VREF_F(AMP_DQ1, 1), dqvref_freq1);
				}
#endif

				// <rdar://problem/18183851> Workarounds: Pull Up/Down shift for Maui Hynix 25nm
				if(dev_info->rev_id == 0x3) {
					// Test MRS Exit
					dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x0);

					// Test MRS Entry
					dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0xb0);
					dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0xe0);
					dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x90);

					if(dev_info->rev_id2 == 0x0) {
						// Rising Slew-rate up
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x87);

						// Self-refresh OSC Min
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0xd0);
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x90);
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x86);
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x81);
					}

					// For all SoCs that use this particular DRAM
					if(dev_info->rev_id2 <= 0x1) {
						// Pull Up RON shift
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0xc4);
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x90);
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x8e);
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x81);

						// Pull Down RON shift
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0xc4);
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x94);
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x92);
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x81);

						// ZQ calibration START MPC cmd
						dcs_mrcmd(MR_MPC, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x0, 0x4f);
						// wait 1us for tZQCAL
						dcs_spin(1, "Workarounds -- tZQCAL");

						// ZQ calibration LATCH MPC cmd
						dcs_mrcmd(MR_MPC, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x0, 0x51);
					}
				}
				break;

			case JEDEC_LPDDR4_MANUF_ID_MICRON:
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
				if(dev_info->rev_id == 1 && dev_info->rev_id2 == 2) {
					// <rdar://problem/21042636>  tRFCpbCyc_freq1 = 0x20 for Micron
					autoref_freq1 = DCS_REG_READ_CH(0, rDCS_MCU_AUTOREF_FREQ(1));
					autoref_freq1 &= 0x00ffffff;
					autoref_freq1 |= (0x20 << 24);
					dcs_reg_write_all_chan(rDCS_MCU_AUTOREF_FREQ(1), autoref_freq1);
				}
#endif
#if SUB_PLATFORM_S8000
				if(dev_info->rev_id == 1) {
					dqvref_freq1 = 0x00ea00ea; // Maui
					dcs_reg_write_all_chan(rDCS_AMP_VREF_F(AMP_DQ0, 1), dqvref_freq1);
					dcs_reg_write_all_chan(rDCS_AMP_VREF_F(AMP_DQ1, 1), dqvref_freq1);
				}
#endif

				if(dev_info->rev_id == 0x1) {
						// Test MRS Exit
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x0);

						// <rdar://problem/20303861> Test MRS: Disable VREFDQ TAP code local latch
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0xf1);
						dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x9, 0x40);
				}
				break;

			default:
				break;
		}
	}
}

#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
void dcs_dram_workarounds_step12 ( void )
{

	const struct dcs_memory_device_info *dev_info = dcs_get_memory_device_info();

	if ( (dev_info->vendor_id == JEDEC_LPDDR4_MANUF_ID_SAMSUNG) && (dev_info->rev_id == 5) && (dev_info->rev_id2 == 1) )
	{

		if((dcs_cfg.chipID == 0x8000 && dcs_cfg.chipRev >= CHIP_REVISION_C0) || (dcs_cfg.chipID == 0x8003 && dcs_cfg.chipRev >= CHIP_REVISION_A1) ||
			(dcs_cfg.chipID == 0x8001 && dcs_cfg.chipRev >= CHIP_REVISION_B0)  )
		{
			dcs_mrcmd(MR_WRITE, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x0b, 0x34);
			dcs_mrcmd(MR_MPC, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x00, 0x4F);
			dcs_spin(1, "Workaround-Samsung--step-12-tzQCAL");
			dcs_mrcmd(MR_MPC, DCS_NUM_CHANNELS, DCS_NUM_RANKS, 0x00, 0x51);
			dcs_spin(1, "Workaround-Samsung--step-12-tZQLAT");
			dcs_reg_write_all_chan(rDCS_MCU_FREQCHNGCTL_FREQ(2, 0), 0xb303340b);

		}

	}

}
#endif
//================================================================================
// Grab (Platform Specific) Tables with Definitions of Values for DCS Init & DCS Tunables
#include <platform/dcsconfig.h>

#if DCS_FIXUP_PARAMS
// Grab (Target Specific) Definition of Required Fixups to the Values for DCS Init
#include <target/dcsfixup.h>
#endif

// use_dcs_params is already the same as dcs_params, unless we want to override with older chip-rev or target specific values
void dcs_init_config_params(dcs_config_params_t *use_dcs_params, dcs_target_t targtype)
{
#if SUB_PLATFORM_S8000
	// Maui B0 DCS param table
	if(platform_get_chip_revision() < CHIP_REVISION_C0)
		memcpy(use_dcs_params, &dcs_params_b0, sizeof(dcs_config_params_t));
#endif

#if DCS_FIXUP_PARAMS
	// This gives the particular target a chance to make target-specific changes
	dcs_init_config_fixup_params(use_dcs_params, targtype);
#endif
}

const dcs_tunable_t *dcs_init_tunable_table(uint32_t *table_len)
{
	const dcs_tunable_t *tunables = dcs_tunables;
	uint32_t tunables_size = sizeof(dcs_tunables);

#if SUB_PLATFORM_S8000
	// Maui B0 DCS tunables
	if(platform_get_chip_revision() < CHIP_REVISION_C0) {
		tunables = dcs_tunables_b0;
		tunables_size = sizeof(dcs_tunables_b0);
	}
#endif

	if (table_len)
		*table_len = tunables_size / sizeof(tunables[0]);

	return tunables;
}

void dcs_aop_ddr_to_awake_post_tunables_insert(void)
{
	uint32_t i, num_tunables = 0;
	const dcs_tunable_t *tunables = NULL;
	uintptr_t curr_reg = 0;
	uint32_t reg_val = 0;

	tunables = dcs_init_tunable_table(&num_tunables);

	// At the time this function is called, tunables values are already expected to be programmed into the registers.
	// Expected format of the dcs_tunables table: Each field that has a tunable value is 1 entry in the tunables table
	for(i = 0; i < num_tunables; i++, tunables++) {

		// end of table
		if(tunables->reg == 0)
			break;

		// skip over subsequent tunables values for different fields in the same register, since we read out the entire register the first time
		if(tunables->reg == curr_reg)
			continue;

		curr_reg = tunables->reg;

		if(dcs_reg_is_outside_dcs_block(curr_reg)) {
			reg_val = DCS_REG_ACCESS(curr_reg);
			reconfig_command_write(AOP_DDR_AWAKE_POST, curr_reg, reg_val, false);
		} else {
			// if reg is within DCS hw block, the reg is duplicated for each instance of the hw block, just read out 1st instance
			// and write that value for all instances
			reg_val = DCS_REG_READ_CH(0, curr_reg);

			for(uint32_t channel = 0; channel < DCS_NUM_CHANNELS; channel++)
				reconfig_command_write(AOP_DDR_AWAKE_POST, (curr_reg + (channel * DCS_SPACING)), reg_val, false);
		}
	}
}

#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
//================================================================================
// Save off calibration results to AOP_SRAM region (later to be stitched into mcu_aop_awake_reconfig_restore sequence)
// Order of registers to be saved matches the order in the reconfig sequence
volatile uint32_t *dcs_save_calibration_results(volatile uint32_t *save_region, uint32_t freq)
{
	uint32_t ch, n;

	// Registers to be saved off after calibration in freq 1 has completed
	if (freq == DCS_FREQ(1)) {
		// CASDLLCTRL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_ACCESS(rAMP_CASDLLCTRL(ch));

		// CSSDLLCTRL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_ACCESS(rAMP_CSSDLLCTRL(ch));

		// CKSDLLCTRL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_ACCESS(rAMP_CKSDLLCTRL(ch));

		// AMPCAWRLVLSDLLCODE
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_ACCESS(rAMP_CAWRLVLSDLLCODE(ch));

		// CA_DESKEW_CTRL
		for(n = 0; n < CA_NUM_BITS; n++)
			for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
				*save_region++ = DCS_REG_ACCESS(rAMP_CADESKEW_CTRL(ch, n));

		// CS_DESKEW_CTRL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_ACCESS(rAMP_CSDESKEW_CTRL(ch));

		// CK_DESKEW_CTRL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_ACCESS(rAMP_CKDESKEW_CTRL(ch));

		// AMPWRLVLSDLLCODE0
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
			*save_region++ = DCS_REG_ACCESS(rAMP_DQWRLVLSDLLCODE(0, ch));
			*save_region++ = DCS_REG_ACCESS(rAMP_DQWRLVLSDLLCODE(1, ch));
		}

		// VREF_Fx
		for(n = 0; n < DCS_FREQUENCY_SLOTS; n++) {
			for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
				*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_VREF_F(AMP_DQ0, n));
				*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_VREF_F(AMP_DQ1, n));
			}
		}

		// AUTOREF_FREQ1
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_MCU_AUTOREF_FREQ(1));

		// CAMDLLVTSCALEREFCONTROL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
				*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_CAMDLL_VTSCALE_REFCNTL(AMP_DQ0));
				*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_CAMDLL_VTSCALE_REFCNTL(AMP_DQ1));
		}

		// RDMDLLVTSCALEREFCONTROL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_RDMDLL_VTSCALE_REFCNTL(AMP_DQ0));
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_RDMDLL_VTSCALE_REFCNTL(AMP_DQ1));
		}

		// WRMDLLVTSCALEREFCONTROL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_WRMDLL_VTSCALE_REFCNTL(AMP_DQ0));
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_WRMDLL_VTSCALE_REFCNTL(AMP_DQ1));
		}

		// FREQCHNGCTL1_FREQ1
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_MCU_FREQCHNGCTL_FREQ(1, 1));

	} else if (freq == DCS_FREQ(0)) {
		// Registers to be saved off after calibration in freq 0 has completed

		// VREF_F0
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_VREF_F(AMP_DQ0, 0));
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_VREF_F(AMP_DQ1, 0));
		}

		// FREQCHNGCTL1_FREQ0
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_MCU_FREQCHNGCTL_FREQ(1, 0));

		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_MCU_FREQCHNGCTL_FREQ(2, 0));
	} else if (freq == 0) {
		// <rdar://problem/23355941> Maui/Malta: Samsung DRAM workaround: disable hw zqcal
		// rGLBTIMER_ZQCTIMER
		*save_region++ = DCS_REG_ACCESS(rGLBTIMER_ZQCTIMER);
	}

	return save_region;
}

//================================================================================
// Stitch the saved calibration results into the reconfig sequence
// To be called immediately after stitching mcu_aop_awake_reconfig_pre_restore
// Uses mcu_aop_awake_reconfig_restore as reference for register order and fields
void dcs_restore_calibration_results(volatile uint32_t *save_region)
{
	uint32_t ch, n, f;
	uint32_t reg_val;

	// CASDLLCTRL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = ((*save_region) & SDLLOVRVAL_MASK) | RUNSDLLUPDWRRESULT;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CASDLLCTRL(ch), reg_val, false);
		save_region++;
	}

	// CSSDLLCTRL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = ((*save_region) & SDLLOVRVAL_MASK) | RUNSDLLUPDWRRESULT;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CSSDLLCTRL(ch), reg_val, false);
		save_region++;
	}

	// CKSDLLCTRL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = ((*save_region) & SDLLOVRVAL_MASK) | RUNSDLLUPDWRRESULT;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CKSDLLCTRL(ch), reg_val, false);
		save_region++;
	}

	// AMPCAWRLVLSDLLCODE
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = ((*save_region) & WRLVLSDLLCODE_MASK) | WRLVLRUNUPDWRRESULT;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CAWRLVLSDLLCODE(ch), reg_val, false);
		save_region++;
	}

	// Poll for AMPCAWRLVLSDLLCODE.WRLVLRUNUPDWRRESULT to clear
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
		reconfig_command_read(AOP_DDR_AWAKE_POST, rAMP_CAWRLVLSDLLCODE(ch), 0, WRLVLRUNUPDWRRESULT, 0, false);

	// CA_DESKEW_CTRL
	for(n = 0; n < CA_NUM_BITS; n++) {
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
			reg_val = ((*save_region) & DESKEWCODE_MASK) | RUNDESKEWUPD;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CADESKEW_CTRL(ch, n), reg_val, false);
			save_region++;
		}
	}

	// CS_DESKEW_CTRL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
			reg_val = ((*save_region) & DESKEWCODE_MASK) | RUNDESKEWUPD;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CSDESKEW_CTRL(ch), reg_val, false);
			save_region++;
	}

	// CK_DESKEW_CTRL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
			reg_val = ((*save_region) & DESKEWCODE_MASK) | RUNDESKEWUPD;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CKDESKEW_CTRL(ch), reg_val, false);
			save_region++;
	}

	// AMPWRLVLSDLLCODE0
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		// for DQ0 and DQ1
		for(n = 0; n < 2; n++) {
			reg_val = ((*save_region) & WRLVLSDLLCODE_MASK) | WRLVLRUNUPDWRRESULT;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_DQWRLVLSDLLCODE(n, ch), reg_val, false);
			save_region++;
		}
	}

	// VREF_Fx
	for(f = 0; f < DCS_FREQUENCY_SLOTS; f++) {
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
			// for DQ0 and DQ1
			for(n = AMP_DQ0; n < DCS_AMP_NUM_PHYS; n++) {
				reg_val = (*save_region) & (VREFSEL_MASK | DQSVREFSEL_MASK);
				reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMP_VREF_F(n, f) + (ch * DCS_SPACING), reg_val, false);
				save_region++;
			}
		}
	}

	// AUTOREF_FREQ1
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = *save_region;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_MCU_AUTOREF_FREQ(1) + (ch * DCS_SPACING), reg_val, false);
		save_region++;
	}

	// CAMDLLVTSCALEREFCONTROL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		// for DQ0 and DQ1: shift the status field value into the override field
		for(n = AMP_DQ0; n < DCS_AMP_NUM_PHYS; n++) {
			reg_val = (((*save_region) & VTSCALEREFSTATUS_MASK) << VTSCALEREFOVERRIDEVAL_SHIFT) | VTSCALEREFUPDATE;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMP_CAMDLL_VTSCALE_REFCNTL(n) + (ch * DCS_SPACING), reg_val, false);
			save_region++;
		}
	}

	// RDMDLLVTSCALEREFCONTROL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		// for DQ0 and DQ1: shift the status field value into the override field
		for(n = AMP_DQ0; n < DCS_AMP_NUM_PHYS; n++) {
			reg_val = (((*save_region) & VTSCALEREFSTATUS_MASK) << VTSCALEREFOVERRIDEVAL_SHIFT) | VTSCALEREFUPDATE;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMP_RDMDLL_VTSCALE_REFCNTL(n) + (ch * DCS_SPACING), reg_val, false);
			save_region++;
		}
	}

	// WRMDLLVTSCALEREFCONTROL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		// for DQ0 and DQ1: shift the status field value into the override field
		for(n = AMP_DQ0; n < DCS_AMP_NUM_PHYS; n++) {
			reg_val = (((*save_region) & VTSCALEREFSTATUS_MASK) << VTSCALEREFOVERRIDEVAL_SHIFT) | VTSCALEREFUPDATE;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMP_WRMDLL_VTSCALE_REFCNTL(n) + (ch * DCS_SPACING), reg_val, false);
			save_region++;
		}
	}

	// FREQCHNGCTL1_FREQ1
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = *save_region;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_MCU_FREQCHNGCTL_FREQ(1, 1) + (ch * DCS_SPACING), reg_val, false);
		save_region++;
	}

	// The next set of registers were saved off after freq0 calibration completed

	// VREF_F0
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = *save_region;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMP_VREF_F(AMP_DQ0, 0) + (ch * DCS_SPACING), reg_val, false);
		save_region++;

		reg_val = *save_region;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMP_VREF_F(AMP_DQ1, 0) + (ch * DCS_SPACING), reg_val, false);
		save_region++;
	}

	// FREQCHNGCTL1_FREQ0
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = *save_region;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_MCU_FREQCHNGCTL_FREQ(1, 0) + (ch * DCS_SPACING), reg_val, false);
		save_region++;
	}

	// FREQCHNGCTL2_FREQ0
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = *save_region;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_MCU_FREQCHNGCTL_FREQ(2, 0) + (ch * DCS_SPACING), reg_val, false);
		save_region++;
	}

	// <rdar://problem/23355941> Maui/Malta: Samsung DRAM workaround: disable hw zqcal
	// rGLBTIMER_ZQCTIMER
	reg_val = *save_region;
	reconfig_command_write(AOP_DDR_AWAKE_POST, rGLBTIMER_ZQCTIMER, reg_val, false);
	save_region++;
}
#endif // #if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003

#if SUB_PLATFORM_S8001
//================================================================================
// Save off calibration results to AOP_SRAM region (later to be stitched into mcu_aop_awake_reconfig_restore sequence)
// Order of registers to be saved matches the order in the reconfig sequence
volatile uint32_t *dcs_save_calibration_results(volatile uint32_t *save_region, uint32_t freq)
{
	uint32_t ch, n;

	if(freq == DCS_FREQ(0)) {
		// CASDLLCTRL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_ACCESS(rAMP_CASDLLCTRL(ch));

		// CSSDLLCTRL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_ACCESS(rAMP_CSSDLLCTRL(ch));

		// CKSDLLCTRL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_ACCESS(rAMP_CKSDLLCTRL(ch));

		// AMPCAWRLVLSDLLCODE
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_ACCESS(rAMP_CAWRLVLSDLLCODE(ch));

		// CA_DESKEW_CTRL
		for(n = 0; n < CA_NUM_BITS; n++)
			for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
				*save_region++ = DCS_REG_ACCESS(rAMP_CADESKEW_CTRL(ch, n));

		// CS_DESKEW_CTRL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_ACCESS(rAMP_CSDESKEW_CTRL(ch));

		// CK_DESKEW_CTRL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_ACCESS(rAMP_CKDESKEW_CTRL(ch));

		// AMPWRLVLSDLLCODE0
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
			*save_region++ = DCS_REG_ACCESS(rAMP_DQWRLVLSDLLCODE(0, ch));
			*save_region++ = DCS_REG_ACCESS(rAMP_DQWRLVLSDLLCODE(1, ch));
		}

		// VREF_Fx
		for(n = 0; n < DCS_FREQUENCY_SLOTS; n++) {
			for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
				*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_VREF_F(AMP_DQ0, n));
				*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_VREF_F(AMP_DQ1, n));
			}
		}

		// WRDQCALVREFCODESTATUS
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_WRDQCALVREFCODESTATUS);

		// RDDQCALVREFCODESTATUS
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_RDDQCALVREFCODESTATUS);

		// CAMDLLVTSCALEREFCONTROL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
				*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_CAMDLL_VTSCALE_REFCNTL(AMP_DQ0));
				*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_CAMDLL_VTSCALE_REFCNTL(AMP_DQ1));
		}

		// RDMDLLVTSCALEREFCONTROL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
				*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_RDMDLL_VTSCALE_REFCNTL(AMP_DQ0));
				*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_RDMDLL_VTSCALE_REFCNTL(AMP_DQ1));
		}

		// WRMDLLVTSCALEREFCONTROL
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
				*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_WRMDLL_VTSCALE_REFCNTL(AMP_DQ0));
				*save_region++ = DCS_REG_READ_CH(ch, rDCS_AMP_WRMDLL_VTSCALE_REFCNTL(AMP_DQ1));
		}

		// FREQCHNGCTL1_FREQ0
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_MCU_FREQCHNGCTL_FREQ(1, 0));

		// FREQCHNGCTL2_FREQ0
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_MCU_FREQCHNGCTL_FREQ(2, 0));

		// FREQCHNGCTL2_FREQ3
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_MCU_FREQCHNGCTL_FREQ(2, 3));

		// FREQCHNGCTL4_FREQ0
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
			*save_region++ = DCS_REG_READ_CH(ch, rDCS_MCU_FREQCHNGCTL_FREQ(4, 0));

		// CB_IMPCTL
		*save_region++ = DCS_REG_READ_CH(0, rDCS_AMPH_CB_IMPCTL);

		// B0_ODT
		*save_region++ = DCS_REG_READ_CH(0, rDCS_AMPH_B0_ODT);

		// B1_ODT
		*save_region++ = DCS_REG_READ_CH(0, rDCS_AMPH_B1_ODT);

		// DQS0_ODT
		*save_region++ = DCS_REG_READ_CH(0, rDCS_AMPH_DQS0_ODT);

		// DQS1_ODT
		*save_region++ = DCS_REG_READ_CH(0, rDCS_AMPH_DQS1_ODT);
	} else if(freq == 0) {
		// <rdar://problem/23244578> Elba: Samsung DRAM workaround: disable hw zqcal
		// rGLBTIMER_ZQCTIMER
		*save_region++ = DCS_REG_ACCESS(rGLBTIMER_ZQCTIMER);
	}

	return save_region;
}

//================================================================================
// Stitch the saved calibration results into the reconfig sequence
// To be called immediately after stitching mcu_aop_awake_reconfig_pre_restore
// Uses mcu_aop_awake_reconfig_restore as reference for register order and fields
void dcs_restore_calibration_results(volatile uint32_t *save_region)
{
	uint32_t ch, n, f;
	uint32_t reg_val;

	// CASDLLCTRL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = ((*save_region) & SDLLOVRVAL_MASK) | RUNSDLLUPDWRRESULT;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CASDLLCTRL(ch), reg_val, false);
		save_region++;
	}

	// CSSDLLCTRL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = ((*save_region) & SDLLOVRVAL_MASK) | RUNSDLLUPDWRRESULT;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CSSDLLCTRL(ch), reg_val, false);
		save_region++;
	}

	// CKSDLLCTRL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = ((*save_region) & SDLLOVRVAL_MASK) | RUNSDLLUPDWRRESULT;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CKSDLLCTRL(ch), reg_val, false);
		save_region++;
	}

	// AMPCAWRLVLSDLLCODE
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = ((*save_region) & WRLVLSDLLCODE_MASK) | WRLVLRUNUPDWRRESULT;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CAWRLVLSDLLCODE(ch), reg_val, false);
		save_region++;
	}

	// Poll for AMPCAWRLVLSDLLCODE.WRLVLRUNUPDWRRESULT to clear
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
		reconfig_command_read(AOP_DDR_AWAKE_POST, rAMP_CAWRLVLSDLLCODE(ch), 0, WRLVLRUNUPDWRRESULT, 0, false);

	// CA_DESKEW_CTRL
	for(n = 0; n < CA_NUM_BITS; n++) {
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
			reg_val = ((*save_region) & DESKEWCODE_MASK) | RUNDESKEWUPD;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CADESKEW_CTRL(ch, n), reg_val, false);
			save_region++;
		}
	}

	// CS_DESKEW_CTRL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
			reg_val = ((*save_region) & DESKEWCODE_MASK) | RUNDESKEWUPD;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CSDESKEW_CTRL(ch), reg_val, false);
			save_region++;
	}

	// CK_DESKEW_CTRL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
			reg_val = ((*save_region) & DESKEWCODE_MASK) | RUNDESKEWUPD;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_CKDESKEW_CTRL(ch), reg_val, false);
			save_region++;
	}

	// AMPWRLVLSDLLCODE0
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		// for DQ0 and DQ1
		for(n = 0; n < 2; n++) {
			reg_val = ((*save_region) & WRLVLSDLLCODE_MASK) | WRLVLRUNUPDWRRESULT;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rAMP_DQWRLVLSDLLCODE(n, ch), reg_val, false);
			save_region++;
		}
	}

	// VREF_Fx
	for(f = 0; f < DCS_FREQUENCY_SLOTS; f++) {
		for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
			// for DQ0 and DQ1
			for(n = AMP_DQ0; n < DCS_AMP_NUM_PHYS; n++) {
				reg_val = (*save_region) & (VREFSEL_MASK | DQSVREFSEL_MASK);
				reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMP_VREF_F(n, f) + (ch * DCS_SPACING), reg_val, false);
				save_region++;
			}
		}
	}

	// WRDQCALVREFCODECONTROL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = (*save_region);
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMP_WRDQCALVREFCODECONTROL + (ch * DCS_SPACING), reg_val, false);
		save_region++;
	}

	// RDDQCALVREFCODECONTROL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = (*save_region);
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMP_RDDQCALVREFCODECONTROL + (ch * DCS_SPACING), reg_val, false);
		save_region++;
	}

	// CAMDLLVTSCALEREFCONTROL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		// for DQ0 and DQ1: shift the status field value into the override field
		for(n = AMP_DQ0; n < DCS_AMP_NUM_PHYS; n++) {
			reg_val = (((*save_region) & VTSCALEREFSTATUS_MASK) << VTSCALEREFOVERRIDEVAL_SHIFT) | VTSCALEREFUPDATE;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMP_CAMDLL_VTSCALE_REFCNTL(n) + (ch * DCS_SPACING), reg_val, false);
			save_region++;
		}
	}

	// RDMDLLVTSCALEREFCONTROL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		// for DQ0 and DQ1: shift the status field value into the override field
		for(n = AMP_DQ0; n < DCS_AMP_NUM_PHYS; n++) {
			reg_val = (((*save_region) & VTSCALEREFSTATUS_MASK) << VTSCALEREFOVERRIDEVAL_SHIFT) | VTSCALEREFUPDATE;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMP_RDMDLL_VTSCALE_REFCNTL(n) + (ch * DCS_SPACING), reg_val, false);
			save_region++;
		}
	}

	// WRMDLLVTSCALEREFCONTROL
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		// for DQ0 and DQ1: shift the status field value into the override field
		for(n = AMP_DQ0; n < DCS_AMP_NUM_PHYS; n++) {
			reg_val = (((*save_region) & VTSCALEREFSTATUS_MASK) << VTSCALEREFOVERRIDEVAL_SHIFT) | VTSCALEREFUPDATE;
			reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMP_WRMDLL_VTSCALE_REFCNTL(n) + (ch * DCS_SPACING), reg_val, false);
			save_region++;
		}
	}

	// FREQCHNGCTL1_FREQ0
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = *save_region;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_MCU_FREQCHNGCTL_FREQ(1, 0) + (ch * DCS_SPACING), reg_val, false);
		save_region++;
	}

	// FREQCHNGCTL2_FREQ0
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = *save_region;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_MCU_FREQCHNGCTL_FREQ(2, 0) + (ch * DCS_SPACING), reg_val, false);
		save_region++;
	}

	// FREQCHNGCTL2_FREQ3
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = *save_region;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_MCU_FREQCHNGCTL_FREQ(2, 3) + (ch * DCS_SPACING), reg_val, false);
		save_region++;
	}

	// <rdar://problem/23244578> Elba: Samsung DRAM workaround: disable hw zqcal
	// FREQCHNGCTL4_FREQ0
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++) {
		reg_val = *save_region;
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_MCU_FREQCHNGCTL_FREQ(4, 0) + (ch * DCS_SPACING), reg_val, false);
		save_region++;
	}

	// CB_IMPCTL
	reg_val = *save_region;
	save_region++;
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMPH_CB_IMPCTL + (ch * DCS_SPACING), reg_val, false);

	// B0_ODT
	reg_val = *save_region;
	save_region++;
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMPH_B0_ODT + (ch * DCS_SPACING), reg_val, false);

	// B1_ODT
	reg_val = *save_region;
	save_region++;
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMPH_B1_ODT + (ch * DCS_SPACING), reg_val, false);

	// DQS0_ODT
	reg_val = *save_region;
	save_region++;
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMPH_DQS0_ODT + (ch * DCS_SPACING), reg_val, false);

	// DQS1_ODT
	reg_val = *save_region;
	save_region++;
	for(ch = 0; ch < DCS_NUM_CHANNELS; ch++)
		reconfig_command_write(AOP_DDR_AWAKE_POST, rDCS_AMPH_DQS1_ODT + (ch * DCS_SPACING), reg_val, false);

	// rGLBTIMER_ZQCTIMER
	reg_val = *save_region;
	reconfig_command_write(AOP_DDR_AWAKE_POST, rGLBTIMER_ZQCTIMER, reg_val, false);
	save_region++;
}
#endif // #if SUB_PLATFORM_S8001

