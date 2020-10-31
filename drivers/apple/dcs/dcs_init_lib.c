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

/*
 * Init Sequence Library for use in:
 * Driver for the LPDDR4 compatible DRAM Control Subsystem (DCS) block
 */

/*
Summary of Steps from the Spec (Spreadsheet)

  + (Aa) Steps 0,1,2,3a     	   [need to identify parameterization]
  + (Ab) Step  3b		   [PMGR: set Freq to 50MHz & delay-wait]
  + (Ac) Steps 3c,4,5   	   [need to identify parameterization]
  + (B) Step   6		   [Change Clk Speed]
  + (C) Step   7		   [include delay-wait from early Step 8]
  + (D) Step   8a		   [Cal: CA  800MHz Vmin]
  + (E) Step   8b		   [Write Regs]
  + (F) Step   8c		   [Cal: CA  800MHz Vmin]   (...again? should it be CS?)
  + (G) Step   8d		   [Write Regs]
  + (H) Step   9		   [Write Regs]
  + (I) Step  10		   [Cal: WrLvl, RdDq, WrDq  800MHz Vmin]
  + (J) Step  11		   [Write Regs]
  + (K) Step  12		   [Cal: CA 1200MHz Vnom]
  + (L) Step  13		   [Write Regs]
  + (M) Step  14		   [Cal: WrLvl, RdDq, WrDq 1200MHz Vmin]
  + (N) Steps 15,16,17,18,19,20    [need to identify parameterization]

Between these steps there is possibly a need/desire for:
- Frequency Changes (PMGR)
- PMU save/restore
- debug messages
- register dumps (DV, SiVal)
- anything else!

CONFIG Table:
    Compile-Time Known:
	Type of Execution Environment	CoreOS, DV, SiVal
	Type of Target			SoC, FPGA, Palladium
	ChipID				0x8000, 0x8001, etc.
	Revision			A0, B0, B1, etc.
	Num DCS				4, 8, etc.			(Derived...!)
	Variant Flags			Work-arounds, RADAR Ref., et al.

	(Config Info know at Compile time can be subject to #if)

    Run-Time Known:
	Type of Init			ColdBoot, AOP_DDR, AOP_AWAKE
	Debug Flags			Low-Level, Milestone, et al.

RegValue Table
	Much like today's:  various values for various registers
				Sensitive to ChipID (also Target type?)
	(Internal static alloc (Complete Table Fill-in within Init function):
		Fill in Values (per ChipID/Rev & Target-type)
		return pointer to caller
			(Allows customization by caller before use)

TODO:
- Convert to using SPDS for register names, et al

*/

//================================================================================
// API Definitions
#include <drivers/dcs/dcs_init_lib.h>

dcs_config_t dcs_cfg;
extern dcs_config_params_t dcs_params;

#if izIBOOT
int debug_level_dcs = DEBUG_SPEW;
#endif

//================================================================================
// Configure the Library for use for the current Boot Type, Chip Type, etc.
//================================================================================
bool dcs_init_config(dcs_boottype_t boot_type, uint32_t chipID, uint32_t chipRev,
		     uint32_t num_dcs, uint32_t num_rank)
{
	dcs_cfg.valid = false;
	// TODO:  Get a little noisy about failures in this function

	// Validate that Build Environment is one that we support
	if (!(izIBOOT || izSIVAL || izDV || izPERTOS)) {
		return false;
	}
	// Validate that Target Type is one that we support
	if (!(izSOC || izFPGA || izPAL))
		return false;

	dcs_cfg.boot_type = boot_type;
	// Validate that the Boot Type is one that we support
	if (!(izColdBoot || izResume || izAOP_DDR || izAOP_Awake))
		return false;
	// Validate that we can support the requested number of channels/ranks
	if ((num_dcs > DCS_NUM_CHANNELS) || (num_rank > DCS_NUM_RANKS))
		return false;
#if izIBOOT
	// Validate the Chip ID provided against the actual (only for iBoot)
	if (chipID != platform_get_chip_id())
		return false;
#endif
	dcs_cfg.chipID    = chipID;
	dcs_cfg.chipRev   = chipRev;
	dcs_cfg.num_dcs   = num_dcs;
	dcs_cfg.num_rank  = num_rank;

	dcs_cfg.debugBitz   = DCS_DBG_DEFAULT;	// Default include Overview
	dcs_cfg.variantBitz = DCS_VARIANT_NONE;	// Default: No variant flags enabled

#if 1		// Very useful indicator that we have built this library correctly
	dbgprintf(DCS_DBG_OVERVIEW, "DCS Init Lib Built for: "
#if izIBOOT
		  "iBoot "
#endif
#if izSIVAL
		  "SiVal "
#endif
#if izDV
		  "DV "
#endif
#if izPERTOS
		  "PERTOS "
#endif
#if izSOC
		  "SOC"
#endif
#if izFPGA
		  "FPGA"
#endif
#if izPAL
		  "PAL"
#endif
		  "\n");
#endif
	dbgprintf(DCS_DBG_OVERVIEW,
		  "DCS Init [%s Build] for ChipID==0x%04x chipRev %c%c (#DCS==%d, #Rank==%d) [%s] using %s\n",
		  (izIBOOT)? "iBoot":(izSIVAL)? "SiVal":(izDV)? "DV":(izPERTOS)? "PERTOS":"??Build??",
		  chipID, ((chipRev >> 4) & 0xF) + 'A', ((chipRev >> 0) & 0xF) + '0',
		  dcs_cfg.num_dcs, dcs_cfg.num_rank,
		  (izSOC)? "SoC":(izFPGA)? "FPGA":(izPAL)? "Palladium":"??Target??",
		  (izColdBoot)? "Coldboot":(izResume)? "Resume":(izAOP_DDR)? "AOP_DDR":(izAOP_Awake)? "AOP_Awake":"??BootType??");

	// We're good to go!
	dcs_cfg.valid = true;
	return dcs_cfg.valid;
}

//================================================================================
// Alter the Debug or the Variant Bits, returning the value-before-modifcation
//================================================================================
uint32_t dcs_init_debug_enable(uint32_t bit)
{
	uint32_t ret = dcs_cfg.debugBitz;
	dcs_cfg.debugBitz    |=  bit;
	return ret;
}

uint32_t dcs_init_debug_disable(uint32_t bit)
{
	uint32_t ret = dcs_cfg.debugBitz;
	dcs_cfg.debugBitz    &= ~bit;
	return ret;
}

uint32_t dcs_init_variant_enable(uint32_t bit)
{
	uint32_t ret = dcs_cfg.variantBitz;
	dcs_cfg.variantBitz   |=  bit;
	return ret;
}

uint32_t dcs_init_variant_disable(uint32_t bit)
{
	uint32_t ret = dcs_cfg.variantBitz;
	dcs_cfg.variantBitz   &= ~bit;
	return ret;
}


//================================================================================
// Set up the Config Parameter Table (specifc to Platform, using target type)
//================================================================================
dcs_config_params_t *dcs_init_param_table(void)
{
	// Call (platform-specific) function to initialize values to be used during DCS Init
	dcs_init_config_params(&dcs_params, DCS_TARGET_TYPE);

	// Return pointer to the table
	// (allows caller to optionally modify values for possible experimentation)
	return &dcs_params;
}

//================================================================================
// Basic Register Write/Poll Functions, including MR Cmds
//================================================================================
void dcs_reg_write(uintptr_t reg_addr, uint32_t reg_value)
{
	dbgprintf(DCS_DBG_REG_ACCESS, "DCS Init     REG(%p) <- 0x%08X  (Single-Reg-Write)\n",
		  (void *)reg_addr, reg_value);
	DCS_REG_ACCESS(reg_addr) = reg_value;
}

void dcs_reg_poll(uintptr_t reg_addr, uint32_t mask_value, uint32_t required_value)
{
	dbgprintf(DCS_DBG_REG_ACCESS, "DCS Init     REG(%p) Poll: Mask (0x%08X) == 0x%08x (Single-Reg-Poll)\n",
		  (void *)reg_addr, mask_value, required_value);
	SPIN_W_TMO_WHILE((DCS_REG_ACCESS(reg_addr) & mask_value) != required_value);
}

void dcs_reg_write_all_chan(uintptr_t reg_addr, uint32_t reg_value)
{
	uint32_t channel;

	dbgprintf(DCS_DBG_REG_ACCESS, "DCS Init     REG(%p) <- 0x%08X (x%d)",
		  (void *)reg_addr, reg_value, dcs_cfg.num_dcs);
	for(channel = 0; channel < dcs_cfg.num_dcs; channel++) {
		DCS_REG_ACCESS(reg_addr + (channel * DCS_SPACING)) = reg_value;
		dbgprintf(DCS_DBG_CHAN_DETAIL, "%d ", channel);
	}
	dbgprintf(DCS_DBG_REG_ACCESS, "\n");
}

void dcs_reg_poll_all_chan(uintptr_t reg_addr, uint32_t mask_value, uint32_t required_value)
{
	uint32_t channel;

	dbgprintf(DCS_DBG_REG_ACCESS, "DCS Init     REG(%p) Poll: Mask (0x%08X) == 0x%08x ...",
		  (void *)reg_addr, mask_value, required_value);
	for(channel = 0; channel < dcs_cfg.num_dcs; channel++) {
		dbgprintf(DCS_DBG_CHAN_DETAIL, " %d", channel);
		SPIN_W_TMO_WHILE((DCS_REG_READ_CH(channel, reg_addr) & mask_value) != required_value);
		dbgprintf(DCS_DBG_CHAN_DETAIL, "!");
	}
	dbgprintf(DCS_DBG_REG_ACCESS, "\n");
}

// To determine whether to use dcs_reg_write_all_chan or dcs_reg_write,
// because regs within a dcs hw block will be duplicated per channel, while those outside the block will not
bool dcs_reg_is_outside_dcs_block(uintptr_t reg_addr)
{
	if((reg_addr >= DCS_BASE_ADDR) && (reg_addr < (DCS_BASE_ADDR + DCS_SPACING)))
		return false;

	return true;
}

// Only send the cmd to specific channel and rank (no poll afterward)
static void dcs_mrcmd_send_specific_ch_rnk(uint32_t cmd, uint8_t channel, uint8_t rank)
{
	cmd |= (rank << MRCMD_RANK_SHIFT);
	rDCS_MCU_MRINITCMD(channel) = cmd;
}

// Only poll the specific channel and rank (return cmd word)
static uint32_t dcs_mrcmd_poll_specific_ch_rnk(uint32_t cmd, uint8_t channel, uint8_t rank)
{
	SPIN_W_TMO_WHILE((cmd = rDCS_MCU_MRINITSTS(channel)) & MRCMD_POLL_BIT);
	return cmd;
}

// Only send the cmd to specific channel and rank
static uint32_t dcs_mrcmd_to_specific_ch_rnk(uint32_t cmd, uint8_t channel, uint8_t rank)
{
	dcs_mrcmd_send_specific_ch_rnk(cmd, channel, rank);
	return dcs_mrcmd_poll_specific_ch_rnk(cmd, channel, rank);
}

// Send the cmd to channel and rank as indicated (used during calibration on H6 and later)
static uint32_t dcs_form_mrcmd_word(dcs_mrcmd_op_t op, int reg, uintptr_t val)
{
	uint32_t cmd;

	if (op == MR_READ)
		cmd = MRCMD_TYPE_READ  | (reg << MRCMD_REG_SHIFT);
	else if (op == MR_WRITE)
		cmd = MRCMD_TYPE_WRITE | (reg << MRCMD_REG_SHIFT) | (((uint32_t)val) << MRCMD_DATA_SHIFT);
	// MPC
	else
		cmd = MRCMD_TYPE_MPC   | (reg << MRCMD_REG_SHIFT) | (((uint32_t)val) << MRCMD_DATA_SHIFT);

	return cmd;
}

// Send the cmd to channel and rank as indicated (used during calibration on H6 and later)
void dcs_mrcmd_to_ch_rnk(dcs_mrcmd_op_t op, uint8_t channel, uint8_t rank, int reg, uintptr_t val)
{
	uint32_t cmd, regval;
	uint8_t *buffer = (uint8_t *) val;

	cmd = dcs_form_mrcmd_word(op, reg, val);
	regval = dcs_mrcmd_to_specific_ch_rnk(cmd, channel, rank);
	if (op == MR_READ) {
		dbgprintf(DCS_DBG_MR_READ, " [data==0x%02x] ", (regval >> MRCMD_DATA_SHIFT) & 0xff);
		*buffer++ = (regval >> MRCMD_DATA_SHIFT) & 0xff;
	}
}

void dcs_mrcmd(dcs_mrcmd_op_t op, uint8_t channels, uint8_t ranks, int reg, uintptr_t val)
{
	uint8_t ch, r;
	uint32_t cmd, regval;
	uint8_t *buffer = (uint8_t *) val;

	cmd = dcs_form_mrcmd_word(op, reg, val);
	dbgprintf(DCS_DBG_REG_ACCESS, "DCS Init     MRCMD(0x%08X)[%dx chan][%dx rank]",
		  cmd, channels, ranks);
	dbgprintf(DCS_DBG_CHAN_DETAIL, " SEND: ");
	for (ch = 0; ch < channels; ch++) {
		for (r = 0; r < ranks; r++) {
			dbgprintf(DCS_DBG_CHAN_DETAIL, "->%d%d ", ch, r);
			dcs_mrcmd_send_specific_ch_rnk(cmd, ch, r);
		}
	}
	dbgprintf(DCS_DBG_CHAN_DETAIL, " POLL: ");
	for (ch = 0; ch < channels; ch++) {
		for (r = 0; r < ranks; r++) {
			regval = dcs_mrcmd_poll_specific_ch_rnk(cmd, ch, r);
			dbgprintf(DCS_DBG_CHAN_DETAIL, "%d%d! ", ch, r);
			if (op == MR_READ) {
				dbgprintf(DCS_DBG_MR_READ, " [data==0x%02x] ", (regval >> MRCMD_DATA_SHIFT) & 0xff);
				*buffer++ = (regval >> MRCMD_DATA_SHIFT) & 0xff;
			}
		}
	}
	dbgprintf(DCS_DBG_REG_ACCESS, "\n");
}

//================================================================================
// Utility Functions
//================================================================================
void dcs_spin(uint32_t duration_us, char *reason)
{
	if (reason == NULL) reason = "";
	dbgprintf(DCS_DBG_SPIN, "DCS Init          Spin %d us %s\n", duration_us, reason);
	delay_for_us(duration_us);
}



//================================================================================
// Apply Tunables (uses callout that is specifc to Platform)
//================================================================================
void dcs_init_apply_tunables(void)
{
	volatile uintptr_t regptr = 0;
	uint32_t regval;
	uint32_t i,numtune = 0;
	const struct dcs_tunable *dcs_tune;

	dcs_tune = dcs_init_tunable_table(&numtune);
	// Check that we have a table
	if ((dcs_tune == NULL) || (numtune == 0))
		return;

	for (i = 0; i < numtune; i++, dcs_tune++) {
		// end of list?
		if (dcs_tune->reg == 0)
			break;
		// Check if we are now looking at a new register
		if (dcs_tune->reg != regptr) {
			// Write out regval to previous register (if there was one)
			if (regptr) {
				if(dcs_reg_is_outside_dcs_block(regptr))
					dcs_reg_write(regptr, regval);
				else
					dcs_reg_write_all_chan(regptr, regval);
			}
			// read in the current register value (READ from CHAN zero, write to ALL CHAN)
			regptr  =  dcs_tune->reg;
			regval  = *((uint32_t *)regptr);
		}
		// Modify the regval for the Bits indicated in the Mask
		regval &= ~dcs_tune->mask;
		regval |=  dcs_tune->value;
	}
	// (Final) Write out regval to previous register (if there was one)
	if (regptr) {
		if(dcs_reg_is_outside_dcs_block(regptr))
			dcs_reg_write(regptr, regval);
		else
			dcs_reg_write_all_chan(regptr, regval);
	}
#if SUB_PLATFORM_S8001
	if (dcs_cfg.chipRev < CHIP_REVISION_B0)
	{
		dcs_reg_write_all_chan(rDCS_MCU_ROWHAM_CTL(1), 0x4A3822D4);
	}
#endif
}




