/*
* Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <drivers/dcs/dcs_regs.h>
#include <drivers/dram.h>
#include <drivers/reconfig.h>
#include <platform.h>
#include <platform/chipid.h>
#include <platform/memmap.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <platform/soc/reconfig.h>

#if SUB_PLATFORM_S8000

# if TARGET_DDR_800M
#include <platform/soc/reconfig_sequences_1200_s8000.h>
#include <platform/soc/reconfig_sequences_1200_s8000_b0.h>
# else
#include <platform/soc/reconfig_sequences_1188_s8000.h>
#include <platform/soc/reconfig_sequences_1188_s8000_b0.h>
# endif // # if TARGET_DDR_800M

#elif SUB_PLATFORM_S8003

# if TARGET_DDR_800M
#include <platform/soc/reconfig_sequences_1200_s8003.h>
#include <platform/soc/reconfig_sequences_1200_s8003_a0.h>
# else
#include <platform/soc/reconfig_sequences_1188_s8003.h>
#include <platform/soc/reconfig_sequences_1188_s8003_a0.h>
# endif // # if TARGET_DDR_800M

#else
#include <platform/soc/reconfig_sequences_s8001.h>
#include <platform/soc/reconfig_sequences_s8001_a0.h>
#endif // #if SUB_PLATFORM_S8000

//
// For some documentation on how to update this and related files,
// see https://coreoswiki.apple.com/wiki/pages/n9b1J2R/Releasing_Reconfig_Engine_Changes.html
//

typedef void (inserter_callback_t)(enum boot_target target, reconfig_stage_t stage, int index);

static void add_breakpoint(reconfig_stage_t stage, int bitmask);

static void awake_to_aop_ddr_pre_sequence_insert(enum boot_target target, reconfig_stage_t stage, int index)
{
	if (index == 0)
		pmgr_awake_to_aop_ddr_pre_sequence_insert();
}

static void awake_to_aop_ddr_post_sequence_insert(enum boot_target target, reconfig_stage_t stage, int index)
{
	switch (index) {
	case A2D_POST_PWRGATE_BEFORE:
		// Nothing to do. AOP_DDR has the same MCC, DCS and ACS powergate tunables as AWAKE
		break;
	}
}

static void aop_ddr_to_s2r_aop_pre_sequence_insert(enum boot_target target, reconfig_stage_t stage, int index)
{
}

static void s2r_aop_to_aop_ddr_pre_sequence_insert(enum boot_target target, reconfig_stage_t stage, int index)
{
}

static void s2r_aop_to_aop_ddr_post_sequence_insert(enum boot_target target, reconfig_stage_t stage, int index)
{
	switch (index) {
	case S2D_POST_SECURITY_BEFORE:
#if SUB_PLATFORM_S8000
		if(platform_get_chip_revision() < CHIP_REVISION_C0)
			reconfig_command_raw(stage, s2r_aop_to_aop_ddr_b0_post_mcu_locked, sizeof(s2r_aop_to_aop_ddr_b0_post_mcu_locked)/sizeof(uint32_t));
		else
#elif SUB_PLATFORM_S8003
		if(platform_get_chip_revision() < CHIP_REVISION_A1)
			reconfig_command_raw(stage, s2r_aop_to_aop_ddr_a0_post_mcu_locked, sizeof(s2r_aop_to_aop_ddr_a0_post_mcu_locked)/sizeof(uint32_t));
		else
#endif			
		{
			reconfig_command_raw(stage, s2r_aop_to_aop_ddr_post_mcu_locked, sizeof(s2r_aop_to_aop_ddr_post_mcu_locked)/sizeof(uint32_t));
		}

		if (target != BOOT_DIAGS) {
			for (uint32_t i = 0; i < NUM_AMCCS; i++) {
				reconfig_command_write(stage, MCCLOCKREGION_TZ0BASEADDR(i), TZ0_BASE>>12, 0);
				reconfig_command_write(stage, MCCLOCKREGION_TZ0ENDADDR(i), (TZ0_BASE + TZ0_SIZE - 1)>>12, 0);
				reconfig_command_write(stage, MCCLOCKREGION_TZ0LOCK(i), 1, 0);
				reconfig_command_read(stage, MCCLOCKREGION_TZ0LOCK(i), 1, 1, 255, 0);
				reconfig_command_write(stage, MCCLOCKREGION_TZ1BASEADDR(i), TZ1_BASE>>12, 0);
				reconfig_command_write(stage, MCCLOCKREGION_TZ1ENDADDR(i), (TZ1_BASE + TZ1_SIZE - 1)>>12, 0);

				reconfig_command_write(stage, MCCLOCKREGION_TZ1LOCK(i), 1, 0);
				reconfig_command_read(stage, MCCLOCKREGION_TZ1LOCK(i), 1, 1, 255, 0);
				reconfig_command_write(stage, AMCC_MCCCFG_TAG_RD_DIS_ADDR(i), 1, 0);
			}
		}
		break;

	case S2D_POST_PWRGATE_BEFORE:
		pmgr_s2r_aop_to_aop_ddr_post_sequence_insert_pwrgate();
		break;

	case S2D_POST_MCU_DDR_BEFORE:
#if SUB_PLATFORM_S8000
		if(platform_get_chip_revision() < CHIP_REVISION_C0)
			reconfig_command_raw(stage, s2r_aop_to_aop_ddr_b0_post_mcu_ddr, sizeof(s2r_aop_to_aop_ddr_b0_post_mcu_ddr)/sizeof(uint32_t));
		else
#elif SUB_PLATFORM_S8003
		if(platform_get_chip_revision() < CHIP_REVISION_A1)
			reconfig_command_raw(stage, s2r_aop_to_aop_ddr_a0_post_mcu_ddr, sizeof(s2r_aop_to_aop_ddr_a0_post_mcu_ddr)/sizeof(uint32_t));
		else
#endif
		{
			reconfig_command_raw(stage, s2r_aop_to_aop_ddr_post_mcu_ddr, sizeof(s2r_aop_to_aop_ddr_post_mcu_ddr)/sizeof(uint32_t));
		}
		break;
	}
}

static void aop_ddr_to_awake_pre_sequence_insert(enum boot_target target, reconfig_stage_t stage, int index)
{
	if (target == BOOT_DIAGS && index == 0) {
		// As a first step towards supporting reconfig sequence programming for diags, we're
		// simply resetting the system once we get back to AWAKE. We'll eventually support
		// full-on S2R, but this is a first step to enable testing by the diags team
		uint32_t end_command = 0x00000000;
		reconfig_command_write(stage, 0x2102b001c, 0, false);
		reconfig_command_write(stage, 0x2102b0014, 1, false);
		reconfig_command_write(stage, 0x2102b0010, 0, false);
		reconfig_command_write(stage, 0x2102b001c, 4, false);
		reconfig_command_write(stage, 0x2102b0010, 0, false);
		reconfig_command_delay(stage, 0x3FFFFFF);
		reconfig_command_delay(stage, 0x3FFFFFF);
		reconfig_command_raw(stage, &end_command, 1);
	}
}

extern void dcs_aop_ddr_to_awake_post_tunables_insert(void);

static void aop_ddr_to_awake_post_sequence_insert(enum boot_target target, reconfig_stage_t stage, int index)
{
	uint32_t scratch0;

	switch (index) {
	case D2A_POST_MCU_AWAKE_BEFORE:
	// FPGA targets generally dont need the calibration since they run at 50 Mhz
#if !SUPPORT_FPGA

#if SUB_PLATFORM_S8000
		if(platform_get_chip_revision() < CHIP_REVISION_C0)
			reconfig_command_raw(stage, aop_ddr_to_awake_b0_post_mcu_awake_before_restoration, sizeof(aop_ddr_to_awake_b0_post_mcu_awake_before_restoration)/sizeof(uint32_t));
		else
#elif SUB_PLATFORM_S8003
		if(platform_get_chip_revision() < CHIP_REVISION_A1)
			reconfig_command_raw(stage, aop_ddr_to_awake_a0_post_mcu_awake_before_restoration, sizeof(aop_ddr_to_awake_a0_post_mcu_awake_before_restoration)/sizeof(uint32_t));
		else
#endif
		{
			reconfig_command_raw(stage, aop_ddr_to_awake_post_mcu_awake_before_restoration, sizeof(aop_ddr_to_awake_post_mcu_awake_before_restoration)/sizeof(uint32_t));
		}
		// Stitch the memory calibration values into the sequence with values that were saved to AOP_SRAM
		dcs_restore_calibration_results((volatile uint32_t *) MEMORY_CALIB_SAVE_BASE_ADDR);

		// XXX DV says to not use this sequence verbatim
#if SUB_PLATFORM_S8000
		if(platform_get_chip_revision() < CHIP_REVISION_C0)
			reconfig_command_raw(stage, aop_ddr_to_awake_b0_post_mcu_awake_after_restoration, sizeof(aop_ddr_to_awake_b0_post_mcu_awake_after_restoration)/sizeof(uint32_t));
		else
#elif SUB_PLATFORM_S8003
		if(platform_get_chip_revision() < CHIP_REVISION_A1)
			reconfig_command_raw(stage, aop_ddr_to_awake_a0_post_mcu_awake_after_restoration, sizeof(aop_ddr_to_awake_a0_post_mcu_awake_after_restoration)/sizeof(uint32_t));
		else
#endif
		{
			reconfig_command_raw(stage, aop_ddr_to_awake_post_mcu_awake_after_restoration, sizeof(aop_ddr_to_awake_post_mcu_awake_after_restoration)/sizeof(uint32_t));
		}

#endif // #if !SUPPORT_FPGA

		// <rdar://problem/18605066> AOP Boot: Maui A1: Samsung 25nm LP4 RBM failures - WA
		if(platform_get_memory_manufacturer_id() == JEDEC_LPDDR4_MANUF_ID_SAMSUNG) {
			uint8_t rev_id = 0, rev_id2 = 0;
			platform_get_memory_rev_ids(&rev_id, &rev_id2);
			if(rev_id == 3 && rev_id2 == 0)
				for(uint32_t channel = 0; channel < DCS_NUM_CHANNELS; channel++)
					reconfig_command_write(stage, (rDCS_MCU_AREFEN_FREQ(0) + (channel * DCS_SPACING)), 0x1010013f, false);
		}
		break;

	case D2A_POST_RESTORE_BEFORE:
		//
		// Enabling ASIO to allows setting its tunables.
		//
		reconfig_command_write(stage, (uintptr_t)&rPMGR_PS(SIO_BUSIF), 0xf, false);
		reconfig_command_read(stage, (uintptr_t)&rPMGR_PS(SIO_BUSIF), 0xf0, 0xf0, 255, false);
		reconfig_command_write(stage, (uintptr_t)&rPMGR_PS(SIO_P), 0xf, false);
		reconfig_command_read(stage, (uintptr_t)&rPMGR_PS(SIO_P), 0xf0, 0xf0, 255, false);
		reconfig_command_write(stage, (uintptr_t)&rPMGR_PS(SIO), 0xf, false);
		reconfig_command_read(stage, (uintptr_t)&rPMGR_PS(SIO), 0xf0, 0xf0, 255, false);

		//
		// Set all MINIPMGR/ACC/PMGR and static tunables.
		// Set PLL and PERF_SOC with MCU REF using bucket 1 (for DCS training)
		//
		pmgr_aop_ddr_to_awake_post_sequence_insert_pll();

		// memory tunables
		dcs_aop_ddr_to_awake_post_tunables_insert();

		// Restore board ID and memory info scratch registers to help out tools that rely on them
		scratch0 = rPMGR_SCRATCH0 & ~(kPlatformScratchFlagObjectManifestHashValid | kPlatformScratchFlagNonce);
		reconfig_command_write(stage, (uintptr_t)&rPMGR_SCRATCH0, scratch0, false);
		reconfig_command_write(stage, (uintptr_t)&rPMGR_SCRATCH13, rPMGR_SCRATCH13, false);

		// Need to enable UART0 for early resume serial logging
		reconfig_command_write(stage, (uintptr_t)&rPMGR_PS(SIO_P), 0xf, false);
		reconfig_command_read(stage, (uintptr_t)&rPMGR_PS(SIO_P), 0xf0, 0xf0, 255, false);
		reconfig_command_write(stage, (uintptr_t)&rPMGR_PS(UART0), 0xf, false);
		reconfig_command_read(stage, (uintptr_t)&rPMGR_PS(UART0), 0xf0, 0xf0, 255, false);

		// Set and lock IO RVBAR value (reset vector)
		if (target != BOOT_DIAGS) {
			reconfig_command_write(stage, CCC_CPU0_SYS_BASE_ADDR, TZ1_BASE | 1, 1); 
			reconfig_command_write(stage, CCC_CPU1_SYS_BASE_ADDR, TZ1_BASE | 1, 1);	
		} else { /* if (target != BOOT_DIAGS) */
			reconfig_command_write(stage, CCC_CPU0_SYS_BASE_ADDR, DEFAULT_KERNEL_ADDRESS, 1);
			reconfig_command_write(stage, CCC_CPU1_SYS_BASE_ADDR, DEFAULT_KERNEL_ADDRESS, 1);
		}

		//
		// Move to MCU to full performance, CPU to 396MHz, SoC to VMIN and restore full clock mesh
		//
		pmgr_aop_ddr_to_awake_post_sequence_insert();
		break;

	case D2A_POST_PREBOOT_BEFORE:
		// This stitch point is only for debug.

		// Add a breakpoint right before we wake the cores so that we can inspect the system
		// after the reconfig program has run
		add_breakpoint(stage, 1 << ((stage * 2) + 1));
		break;
	}
}

static void add_breakpoint(reconfig_stage_t stage, int bitmask)
{
#if DEBUG_BUILD
	// Adds a spin on a MINIPMGR scratch register to allow pausing the sequence
	// at arbitrary points. To pause the sequence, set the appropriate bit in MINIPMGR_SCRATCH11
	// mem -memap 4 0x2102b802c <value>
	reconfig_command_read(stage, (uintptr_t)&rMINIPMGR_SCRATCH11, 0, bitmask, 0, 0);
#else
	// Add a dummy spin so that the release sequence and debug sequence don't differ
	reconfig_command_read(stage, (uintptr_t)&rMINIPMGR_SCRATCH11, 0, 0, 0, 0);
#endif
}

// Loops through the DV-provided sections of the sequence, alternating between inserting
// a DV-provided raw sequence into the overall sequence and using the provided callback
// to stitch in software-defined portions of the sequence
static void build_sequence(enum boot_target target, reconfig_stage_t stage, const reconfig_subsequence_t *subseqs, inserter_callback_t *inserter)
{
	int i;

	add_breakpoint(stage, 1 << ((stage * 2) + 0));

	for (i = 0; subseqs[i].sequence != NULL; i++) {
		if (inserter != NULL) {
			inserter(target, stage, i);
		}

		reconfig_command_raw(stage, subseqs[i].sequence, subseqs[i].elements);
	}

	if (inserter != NULL) {
		inserter(target, stage, i);
	}

	add_breakpoint(stage, 1 << ((stage * 2) + 1));

	reconfig_commit(stage);
}

void platform_reconfig_sequence_insert(enum boot_target target)
{
#if SUB_PLATFORM_S8000
	if(platform_get_chip_revision() < CHIP_REVISION_C0) {
		build_sequence(target, AWAKE_AOP_DDR_PRE, reconfig_awake_to_aop_ddr_b0_pre_seqs, awake_to_aop_ddr_pre_sequence_insert);
		build_sequence(target, AWAKE_AOP_DDR_POST, reconfig_awake_to_aop_ddr_b0_post_seqs, awake_to_aop_ddr_post_sequence_insert);
		build_sequence(target, AOP_DDR_S2R_AOP_PRE, reconfig_aop_ddr_to_s2r_aop_b0_pre_seqs, aop_ddr_to_s2r_aop_pre_sequence_insert);
		build_sequence(target, S2R_AOP_AOP_DDR_PRE, reconfig_s2r_aop_to_aop_ddr_b0_pre_seqs, s2r_aop_to_aop_ddr_pre_sequence_insert);
		build_sequence(target, S2R_AOP_AOP_DDR_POST, reconfig_s2r_aop_to_aop_ddr_b0_post_seqs, s2r_aop_to_aop_ddr_post_sequence_insert);
		build_sequence(target, AOP_DDR_AWAKE_PRE, reconfig_aop_ddr_to_awake_b0_pre_seqs, aop_ddr_to_awake_pre_sequence_insert);
		build_sequence(target, AOP_DDR_AWAKE_POST, reconfig_aop_ddr_to_awake_b0_post_seqs, aop_ddr_to_awake_post_sequence_insert);
	} else
#elif SUB_PLATFORM_S8003
	if(platform_get_chip_revision() < CHIP_REVISION_A1) {
		build_sequence(target, AWAKE_AOP_DDR_PRE, reconfig_awake_to_aop_ddr_a0_pre_seqs, awake_to_aop_ddr_pre_sequence_insert);
		build_sequence(target, AWAKE_AOP_DDR_POST, reconfig_awake_to_aop_ddr_a0_post_seqs, awake_to_aop_ddr_post_sequence_insert);
		build_sequence(target, AOP_DDR_S2R_AOP_PRE, reconfig_aop_ddr_to_s2r_aop_a0_pre_seqs, aop_ddr_to_s2r_aop_pre_sequence_insert);
		build_sequence(target, S2R_AOP_AOP_DDR_PRE, reconfig_s2r_aop_to_aop_ddr_a0_pre_seqs, s2r_aop_to_aop_ddr_pre_sequence_insert);
		build_sequence(target, S2R_AOP_AOP_DDR_POST, reconfig_s2r_aop_to_aop_ddr_a0_post_seqs, s2r_aop_to_aop_ddr_post_sequence_insert);
		build_sequence(target, AOP_DDR_AWAKE_PRE, reconfig_aop_ddr_to_awake_a0_pre_seqs, aop_ddr_to_awake_pre_sequence_insert);
		build_sequence(target, AOP_DDR_AWAKE_POST, reconfig_aop_ddr_to_awake_a0_post_seqs, aop_ddr_to_awake_post_sequence_insert);
	} else
#endif
	{
		build_sequence(target, AWAKE_AOP_DDR_PRE, reconfig_awake_to_aop_ddr_pre_seqs, awake_to_aop_ddr_pre_sequence_insert);
		build_sequence(target, AWAKE_AOP_DDR_POST, reconfig_awake_to_aop_ddr_post_seqs, awake_to_aop_ddr_post_sequence_insert);
		build_sequence(target, AOP_DDR_S2R_AOP_PRE, reconfig_aop_ddr_to_s2r_aop_pre_seqs, aop_ddr_to_s2r_aop_pre_sequence_insert);
		build_sequence(target, S2R_AOP_AOP_DDR_PRE, reconfig_s2r_aop_to_aop_ddr_pre_seqs, s2r_aop_to_aop_ddr_pre_sequence_insert);
		build_sequence(target, S2R_AOP_AOP_DDR_POST, reconfig_s2r_aop_to_aop_ddr_post_seqs, s2r_aop_to_aop_ddr_post_sequence_insert);
		build_sequence(target, AOP_DDR_AWAKE_PRE, reconfig_aop_ddr_to_awake_pre_seqs, aop_ddr_to_awake_pre_sequence_insert);
		build_sequence(target, AOP_DDR_AWAKE_POST, reconfig_aop_ddr_to_awake_post_seqs, aop_ddr_to_awake_post_sequence_insert);
	}
}
