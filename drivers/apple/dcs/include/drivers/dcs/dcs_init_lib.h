/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DCS_INIT_LIB_H
#define __DCS_INIT_LIB_H

/* DRAM Control Subsystem (DCS) Initialization Library API Definitions */

//================================================================================
// Build (Environment) Type: iBoot, SiVal, DV, PE-RTOS
//================================================================================
#define	DCS_BUILD_ENV_IBOOT		(0)
#define	DCS_BUILD_ENV_SIVAL		(1)
#define	DCS_BUILD_ENV_DV		(2)
#define	DCS_BUILD_ENV_PERTOS		(3)

#define izIBOOT			(DCS_BUILD_ENV_TYPE == DCS_BUILD_ENV_IBOOT)
#define izSIVAL			(DCS_BUILD_ENV_TYPE == DCS_BUILD_ENV_SIVAL)
#define izDV			(DCS_BUILD_ENV_TYPE == DCS_BUILD_ENV_DV)
#define izPERTOS		(DCS_BUILD_ENV_TYPE == DCS_BUILD_ENV_PERTOS)

//================================================================================
// Target Type: SoC, FPGA, Palladium
//================================================================================
#define	DCS_TARGET_SOC			(0)
#define	DCS_TARGET_FPGA			(1)
#define	DCS_TARGET_PALLADIUM		(2)

#define izSOC			(DCS_TARGET_TYPE == DCS_TARGET_SOC)
#define izFPGA			(DCS_TARGET_TYPE == DCS_TARGET_FPGA)
#define izPAL			(DCS_TARGET_TYPE == DCS_TARGET_PALLADIUM)

//================================================================================
// What Build Environment?
#ifndef DCS_BUILD_ENV_TYPE
#define DCS_BUILD_ENV_TYPE	DCS_BUILD_ENV_IBOOT
#endif

//================================================================================
#if izIBOOT
//================================================================================

#define AMP_H_BASE_ADDR AMPH_BASE_ADDR

#include <sys/types.h>
#include <debug.h>
#include <drivers/dcs/dcs_regs.h>
#include <drivers/dcs/dcs.h>
#include <drivers/dcs/dcs_calibration.h>
#include <drivers/dram.h>
#include <drivers/miu.h>
#include <lib/env.h>
#include <platform.h>
#include <platform/soc/hwclocks.h>
#include <platform/chipid.h>
#include <platform/memmap.h>
#include <platform/timer.h>
#include <sys.h>
#include <string.h>

#define delay_for_us(_us_delay)		spin(_us_delay)

extern int debug_level_dcs;
#define do_printf(x...)			dprintf(debug_level_dcs, x)

#define dcs_panic(x...)			\
	EXEC(		\
		debug_level_dcs    = DEBUG_CRITICAL;	\
		dcs_cfg.debugBitz |= DCS_DBG_CAL;	\
		calibration_dump_results(SELECT_CAL_ALL, true);	\
		panic(x);				\
	)

#if SUPPORT_FPGA
#define DCS_TARGET_TYPE		DCS_TARGET_FPGA
#endif

// iBoot gets these definitions via the make command line
// Maximum channels/ranks supported by the target
//#define DCS_NUM_CHANNELS		4
//#define DCS_NUM_RANKS			1

// iBoot gets this definition via the make command line
#ifdef AMP_CALIBRATION_SKIP
#define DCS_DO_CALIBRATION	0
#else
#define DCS_DO_CALIBRATION	1
#endif

//================================================================================
#elif izSIVAL
//================================================================================
#include "drivers/dcs/dcs_sival.h"

//================================================================================
#elif izDV
//================================================================================

//================================================================================
#elif izPERTOS
//================================================================================

//================================================================================
#else		// other than iBoot (examples: SiVal, DV, etc.)
//================================================================================
// iBoot has a special version of a tight SpinLoop with Timeout
// Other build environments may use the following simple definition (no Timeout),
// or else provide their own by modifying this definition
#define SPIN_W_TMO_WHILE(__expr)	while(__expr)

// iBoot defines this function to delay for N microseconds
// Other build environments may use the following degenerate definition (no delay),
// or else provide their own by modifying this definition
#define delay_for_us(_us_delay)		EXEC(;)

// iBoot defines this function to optionally print out debug messages
// Other build environments may use the following simple definition (printf),
// or else provide their own by modifying this definition
#define do_printf(x...)			printf(x)

// iBoot defines this function to halt execution and terminate because of a fatal error
// Other build environments may use the following similar definition (panic),
// or else provide their own by modifying this definition
#define dcs_panic(x...)			\
	EXEC(		\
		debug_level_dcs    = DEBUG_CRITICAL;	\
		dcs_cfg.debugBitz |= DCS_DBG_CAL;	\
		calibration_dump_results(SELECT_CAL_ALL, true);	\
		panic(x);				\
	)

// iBoot derives this definition from other definitions provided via the make command line
// Other build environments may explicitly define the target type here,
// or use any other desired method
//#define DCS_TARGET_TYPE		DCS_TARGET_SOC
//#define DCS_TARGET_TYPE		DCS_TARGET_FPGA
//#define DCS_TARGET_TYPE		DCS_TARGET_PALLADIUM

// Maximum channels/ranks supported by the target
#define DCS_NUM_CHANNELS		4
#define DCS_NUM_RANKS			1

#define DCS_DO_CALIBRATION		1	// ZERO to Skip Calibration

//================================================================================
#endif
//================================================================================
#ifndef DCS_TARGET_TYPE
#define DCS_TARGET_TYPE			DCS_TARGET_SOC
#endif

typedef uint32_t dcs_buildtype_t;
typedef uint32_t dcs_target_t;

#define CACAL_MAX_SWLOOPS		1

//================================================================================
// Config Sturcture
//================================================================================
typedef struct {
	bool		valid;
	dcs_boottype_t	boot_type;
	uint32_t	chipID;		// Supported ChipID values listed below
	uint32_t	chipRev;	// A0, B0, B1, etc.
	uint32_t	num_dcs;	// Number of DCS Blocks for this Chip
	uint32_t	num_rank;	// Number of Ranks for the DRAM
	uint32_t	debugBitz;	// Bit Field to control various debug behaviors
	uint32_t	variantBitz;	// Bit Field to control variant behavior (workaround, per RADAR)
} dcs_config_t;

// Supported Chip Revision Codes
#define DCS_CHIP_REV_A0		0x00
#define DCS_CHIP_REV_A1		0x01
#define DCS_CHIP_REV_A2		0x02
#define DCS_CHIP_REV_B0		0x10
#define DCS_CHIP_REV_B1		0x11
#define DCS_CHIP_REV_B2		0x12
#define DCS_CHIP_REV_C0		0x20
#define DCS_CHIP_REV_C1		0x21
#define DCS_CHIP_REV_C2		0x22

//================================================================================
#define BIT_N(_N)			(1 << (_N))

// Defined debug Bitz
#define DCS_DBG_NONE			0x00000000
#define DCS_DBG_REG_ACCESS		BIT_N(0)
#define DCS_DBG_CHAN_DETAIL		BIT_N(1)
#define DCS_DBG_SPIN			BIT_N(2)
#define DCS_DBG_MR_READ			BIT_N(3)
#define DCS_DBG_MILESTONE		BIT_N(4)
#define DCS_DBG_CAL			BIT_N(5)
#define DCS_DBG_CAL_MILESTONE		BIT_N(6)
#define DCS_DBG_FREQ			BIT_N(7)

#define DCS_DBG_OVERVIEW		BIT_N(8)
#define DCS_DBG_ASSESS_CONFIG		BIT_N(9)

#define DCS_DBG_DEFAULT			(DCS_DBG_OVERVIEW | DCS_DBG_ASSESS_CONFIG)

// Defined variant Bitz
#define DCS_VARIANT_NONE		0x00000000
#define DCS_VARIANT_SKIP_MCC		BIT_N(0)
#define DCS_VARIANT_VREF_USES_WIDEST	BIT_N(1) // Use Widest-Eye Algorithm for Optimal Vref

#define DCS_NONVAL			(0xF82BDEAD)

//================================================================================
extern dcs_config_t dcs_cfg;
// extern dcs_config_params_t dcs_params;

#define izColdBoot	(dcs_cfg.boot_type == DCS_BOOT_COLDBOOT)
#define izResume	(dcs_cfg.boot_type == DCS_BOOT_RESUME)
#define izAOP_DDR	(dcs_cfg.boot_type == DCS_BOOT_AOP_DDR)
#define izAOP_Awake	(dcs_cfg.boot_type == DCS_BOOT_AOP_AWAKE)

//================================================================================
#define ifbit_printf(_b, _f, x...)	EXEC(if ((_b) & (_f)) do_printf(x))
#define dbgprintf(_b, x...)		ifbit_printf(_b, dcs_cfg.debugBitz, x)

//================================================================================
// These are used by the Calibration Functions ... provided by dcs_init_lib

// Save SRAM space in iBoot by removing calibration debug printfs
#if izIBOOT
#define shim_printf(x...)		do { } while(0)
#else
#define shim_printf(x...)		dbgprintf(DCS_DBG_CAL, x)
#endif

#define shim_panic(x...)               dcs_panic(x)
#define shim_mrcmd_to_ch_rnk(x...)	dcs_mrcmd_to_ch_rnk(x)

//================================================================================
// Callouts Required by this Library.
// Must be Provided externally (build environment, platform, target, etc.)
//================================================================================
void dcs_dram_workarounds(dcs_boottype_t boot_type);
void dcs_dram_workarounds_step12(void);
void dcs_init_config_params(dcs_config_params_t *use_dcs_params, dcs_target_t targtype);
const dcs_tunable_t *dcs_init_tunable_table(uint32_t *table_len);
void dcs_change_freq(uint32_t freq_bin);
void dcs_store_memory_calibration(void *cal_values, uint32_t cal_size);
void dcs_load_memory_calibration(void *cal_values, uint32_t cal_size);

//================================================================================
// These are used by the Calibration Functions ... must be provided by build environment
#define shim_change_freq(_bin)		dcs_change_freq(_bin)
#define shim_store_memory_calibration   dcs_store_memory_calibration
#define shim_load_memory_calibration    dcs_load_memory_calibration

//================================================================================
// API Functions provided by this Library and available for use
//================================================================================
bool dcs_init_config(dcs_boottype_t boot_type, uint32_t chipID, uint32_t chipRev,
		     uint32_t num_dcs, uint32_t num_rank);
uint32_t dcs_init_debug_enable(uint32_t bit);
uint32_t dcs_init_debug_disable(uint32_t bit);
uint32_t dcs_init_variant_enable(uint32_t bit);
uint32_t dcs_init_variant_disable(uint32_t bit);
dcs_config_params_t *dcs_init_param_table(void);

void dcs_mrcmd_to_ch_rnk(dcs_mrcmd_op_t op, uint8_t channel, uint8_t rank, int reg, uintptr_t val);
void dcs_mrcmd(dcs_mrcmd_op_t op, uint8_t channels, uint8_t ranks, int reg, uintptr_t val);

void dcs_spin(uint32_t duration_us, char *reason);
void dcs_enable_autorefresh(void);
void dcs_init_apply_tunables(void);

// Step  0: AMC Prolog
void dcs_init_Prolog(void);
// Step  1: AMC Initial Configuration
void dcs_init_AMC_Initial_Config(void);
// Step  2: AMP Initial Configuration
void dcs_init_AMP_Initial_Config(void);
// Step  3: Self-Refresh Exit
void dcs_init_Self_Refresh_Exit_a(void);
void dcs_init_Self_Refresh_Exit_b(void);
// Step  4: DRAM Reset, ZQ Calibration & Configuration (cold boot only)
void dcs_init_ZQ_Cal_Cfg(void);
// Step  5a: AddrConfig Configuration
void dcs_init_AddrCfg_Cfg(void);
void dcs_init_TotalSize_Cfg(uint32_t total_size_Mbytes);
// Step  6: Switch from boot-clock speed to normal operation speed
void dcs_init_Reenable_AMC_Scheduler(void);
// Step  7: Setup registers for CA calibration for bucket 1
void dcs_init_CA_Cal_Setup_Freq1(void);
// Step  8: AMP Dynamic CA Vref Timing Calibration at Vmin 800MHz
void dcs_init_CA_Cal_Freq1(void);
// Step  9: Setup registers for DQ calibration for bucket 1
void dcs_init_DQ_Cal_Setup_Freq1_a(void);
void dcs_init_DQ_Cal_Setup_Freq1_b(void);
// Step 10: PHY write DQ calibration
void dcs_init_wrdq_skew(void);
void dcs_init_post_wrlvl(void);
// Step 11: Setup registers for CA calibration for bucket 0
void dcs_init_CA_Cal_Setup_Freq0(void);
// Step 12: AMP Dynamic CA Timing Calibration
//	NO PROVIDED FUNCTION
// Step 13: Setup registers for DQ calibration for bucket 0
void dcs_init_DQ_Cal_Setup_Freq0_a(void);
void dcs_init_DQ_Cal_Setup_Freq0_b(void);
// Step 14: AMP Dynamic DQ Calibration
//	NO PROVIDED FUNCTION
// Step 15: Setup Registers for Boot
void dcs_init_Reg_for_Boot(void);
// Step 16: Enable Other Features
void dcs_init_MoreFeatures(void);
// Step 17: Enable Fast Critical Word Forwarding Feature
void dcs_init_Fast_Critical_Word_Forwarding(void);
// Step 18: Enable Power- and Clock-Gating Features; Config MCC and Global Timers
void dcs_init_Gating_Global_Timers(void);
// Step 19: ODTS read & set interval for periodic MR4 on-die Temp sensor reading
void dcs_init_ODTS(void);

// <rdar://problem/23244578> Elba: Samsung DRAM workaround: disable hw zqcal
bool shim_apply_samsung_workaround(void);

void dcs_reg_write(uintptr_t reg_addr, uint32_t reg_value);
void dcs_reg_poll(uintptr_t reg_addr, uint32_t mask_value, uint32_t required_value);
void dcs_reg_write_all_chan(uintptr_t reg_addr, uint32_t reg_value);
void dcs_reg_poll_all_chan(uintptr_t reg_addr, uint32_t mask_value, uint32_t required_value);

#endif /* ! __DCS_INIT_LIB_H */
