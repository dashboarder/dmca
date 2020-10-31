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

#include <debug.h>
#include <platform.h>
#include <platform/soc/hwregbase.h>
#include <platform/soc/reconfig.h>
#include <drivers/reconfig.h>
#include SUB_PLATFORM_SPDS_HEADER(aop_global)
#include <sys.h>

static const uint64_t *reconfig_bases;

static const uint64_t reconfig_bases_darwin[MAX_STAGE] = {
	[AWAKE_AOP_DDR_PRE] = AWAKE_AOP_DDR_PRE_BASE_ADDR,
	[AWAKE_AOP_DDR_POST] = AWAKE_AOP_DDR_POST_BASE_ADDR,
	[AOP_DDR_S2R_AOP_PRE] = AOP_DDR_S2R_AOP_PRE_BASE_ADDR,
	[AOP_DDR_S2R_AOP_POST] = AOP_DDR_S2R_AOP_POST_BASE_ADDR,
	[S2R_AOP_AOP_DDR_PRE] = S2R_AOP_AOP_DDR_PRE_BASE_ADDR,
	[S2R_AOP_AOP_DDR_POST] = S2R_AOP_AOP_DDR_POST_BASE_ADDR,
	[AOP_DDR_AWAKE_PRE] = AOP_DDR_AWAKE_PRE_BASE_ADDR,
	[AOP_DDR_AWAKE_POST] = AOP_DDR_AWAKE_POST_BASE_ADDR
};

static const uint64_t reconfig_bases_diags[MAX_STAGE] = {
	[AWAKE_AOP_DDR_PRE] = AWAKE_AOP_DDR_PRE_DIAGS_BASE_ADDR,
	[AWAKE_AOP_DDR_POST] = AWAKE_AOP_DDR_POST_DIAGS_BASE_ADDR,
	[AOP_DDR_S2R_AOP_PRE] = AOP_DDR_S2R_AOP_PRE_DIAGS_BASE_ADDR,
	[AOP_DDR_S2R_AOP_POST] = AOP_DDR_S2R_AOP_POST_DIAGS_BASE_ADDR,
	[S2R_AOP_AOP_DDR_PRE] = S2R_AOP_AOP_DDR_PRE_DIAGS_BASE_ADDR,
	[S2R_AOP_AOP_DDR_POST] = S2R_AOP_AOP_DDR_POST_DIAGS_BASE_ADDR,
	[AOP_DDR_AWAKE_PRE] = AOP_DDR_AWAKE_PRE_DIAGS_BASE_ADDR,
	[AOP_DDR_AWAKE_POST] = AOP_DDR_AWAKE_POST_DIAGS_BASE_ADDR
};

static const uint64_t reconfig_max_size[MAX_STAGE] = {
	[AWAKE_AOP_DDR_PRE] = AWAKE_AOP_DDR_PRE_MAX_SIZE,
	[AWAKE_AOP_DDR_POST] = AWAKE_AOP_DDR_POST_MAX_SIZE,
	[AOP_DDR_S2R_AOP_PRE] = AOP_DDR_S2R_AOP_PRE_MAX_SIZE,
	[AOP_DDR_S2R_AOP_POST] = AOP_DDR_S2R_AOP_POST_MAX_SIZE,
	[S2R_AOP_AOP_DDR_PRE] = S2R_AOP_AOP_DDR_PRE_MAX_SIZE,
	[S2R_AOP_AOP_DDR_POST] = S2R_AOP_AOP_DDR_POST_MAX_SIZE,
	[AOP_DDR_AWAKE_PRE] = AOP_DDR_AWAKE_PRE_MAX_SIZE,
	[AOP_DDR_AWAKE_POST] = AOP_DDR_AWAKE_POST_MAX_SIZE
};

static const char *reconfig_stage_names[] = {
	[AWAKE_AOP_DDR_PRE] = "AWAKE_AOP_DDR_PRE",
	[AWAKE_AOP_DDR_POST] = "AWAKE_AOP_DDR_POST",
	[AOP_DDR_S2R_AOP_PRE] = "AOP_DDR_S2R_AOP_PRE",
	[AOP_DDR_S2R_AOP_POST] = "AOP_DDR_S2R_AOP_POST",
	[S2R_AOP_AOP_DDR_PRE] = "S2R_AOP_AOP_DDR_PRE",
	[S2R_AOP_AOP_DDR_POST] = "S2R_AOP_AOP_DDR_POST",
	[AOP_DDR_AWAKE_PRE] = "AOP_DDR_AWAKE_PRE",
	[AOP_DDR_AWAKE_POST] = "AOP_DDR_AWAKE_POST",
	[MAX_STAGE] = "MAX_STAGE"
};

static write_command_t write_commands[MAX_STAGE];
static uint32_t reconfig_count[MAX_STAGE] = {0};
static bool reconfig_committed[MAX_STAGE];
static bool reconfig_locked;

static void validate_stage(reconfig_stage_t stage)
{
	ASSERT(stage < MAX_STAGE);
	ASSERT(!reconfig_committed[stage]);
	ASSERT(!reconfig_locked);
}

static void bounds_check_stage(reconfig_stage_t stage)
{
	dprintf(DEBUG_SPEW, "%s: Stage %d, Count 0x%x, Max 0x%x\n", __FUNCTION__, stage, reconfig_count[stage]*4, (unsigned int)reconfig_max_size[stage]);

	if (reconfig_count[stage] * 4 >= reconfig_max_size[stage]) {
		panic("Writing past the reserved region for config sequence, Stage %d, Count 0x%x, Max 0x%x",
				stage, reconfig_count[stage]*4, reconfig_max_size[stage]);
	}
}

static void reconfig_write_pending(reconfig_stage_t stage)
{
	write_command_t *wc = &write_commands[stage];
	
	if (wc->in_progress) {
		commit_write_command(stage, wc);
	}
}

/* Write Command:
   Cmd, Offsets, Data
   Cmd 
   	31                       6 5       2  1   0
	| Base Address[35:10]     | Regs    | D | 1 |
   Offset0
   	| Offset i+3[9:2] | Offset i+2[9:2] | Offset i+1[9:2] | Offset i+0[9:2] |
   Offset1
   	| Offset i+3[9:2] | Offset i+2[9:2] | Offset i+1[9:2] | Offset i+0[9:2] |
   Offset2
   	| Offset i+3[9:2] | Offset i+2[9:2] | Offset i+1[9:2] | Offset i+0[9:2] |
   Offset3
   	| Offset i+3[9:2] | Offset i+2[9:2] | Offset i+1[9:2] | Offset i+0[9:2] |
   Data0
   .
   .
   .
   .
   Data15 (64 bit data requires 64 bit alignment)
*/ 
static void commit_write_command(reconfig_stage_t stage, write_command_t *wc)
{
	uint32_t cmd = WRITE_COMMAND;
	cmd |= ((wc->address >> 10) << 6);
	cmd |= ((wc->is_reg64 & 0x1)<<1);
	cmd |= RECONFIG_NUMBER_OF_REGS(wc->value_count);

	dprintf(DEBUG_SPEW, "%s: cmd 0x%x\n", __FUNCTION__, cmd);

	// Store the command
	rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = cmd;
	
	// Store the address offsets
	uint32_t ii = 0;
	while (ii <= (wc->value_count-1)/4 )
		rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = wc->offsets[ii++];

	// 8 byte alignment
	if (wc->is_reg64 && !ALIGNED_64(reconfig_count[stage])) {
		rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = ALIGNMENT_MARKER;
	}

	// Store the write values
	ii = 0;
	while (ii < wc->value_count) {
		rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = (uint32_t)wc->values[ii];
		if (wc->is_reg64) {
			rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = (uint32_t)(wc->values[ii] >> 32);
		}
		ii++;
	}
	bounds_check_stage(stage);

	// Clear out structure and use for next write command
	memset((void *)wc, 0, sizeof(write_command_t));
}

/* Queue up individual write commands into a data structure. When we get to 16 entries or 
   need to enter different type of command such as POLL, DELAY... commit the structure
   in the write command format required for reconfig engine into memory 
 
	- Check if valid stage
	- If we have a valid write data struct check if the new command fits into it
	- If it doesnt fit (i.e not in the 1 KB block), commit the previous sequence
	  to memory, and clean up the data structure
	- If it does fit continue
	- Check if the write command is going to be first one 
	- If it is setup the data structure
	- If not add the value to the data structure, and check if the struture is full
	  (i.e 16 values). If full commmit the sequence to memory, and clean up for 
	  next usage.
*/
void reconfig_command_write(reconfig_stage_t stage, uint64_t addr, uint64_t value, uint32_t is_reg64)
{
	ASSERT(is_reg64 || value <= UINT32_MAX);
	ASSERT((addr & 0x3) == 0);
	validate_stage(stage);
	write_command_t *wc = &write_commands[stage];

	dprintf(DEBUG_SPEW, "%s: Stage %s, Address 0x%llx, Value 0x%llx, reg64 %s\n", __FUNCTION__, reconfig_stage_names[stage], addr, value, is_reg64 ? "true":"false");

	if (wc->in_progress) {
		// if address and new address are not in a 1 KB range commit
		if ((wc->address & ALIGNMENT_MASK_1KB) != (addr & ALIGNMENT_MASK_1KB)) {
			commit_write_command(stage, wc);
		} else if (wc->is_reg64 != is_reg64) {
			// Also check if the sizes are the same
			commit_write_command(stage, wc);
		}
	}

	if (!wc->in_progress) {
		// setting up the first time
		wc->in_progress = true;
		wc->address = addr;
		wc->is_reg64 = is_reg64;
		wc->offsets[wc->value_count] = WRITE_OFFSETS(addr, wc->value_count);
		wc->values[wc->value_count] = value;
		wc->value_count++;
	} else {
		wc->values[wc->value_count] = value;
		wc->offsets[wc->value_count / 4] |= WRITE_OFFSETS(addr, wc->value_count);
		wc->value_count++;

		if (wc->value_count == MAX_WRITE_REGS) {
			commit_write_command(stage, wc);
		}
	}
}


/* Read Command:
   Cmd, Offset, Data Mask, Expected Data
   Cmd 
   	31                       6  5   4  3  2  1   0
	| Base Address[35:10]     | D | 0  0  0  1   0 |
   Offset0
   	31           17  16 15         8 7           0   
   	| XXXX   XXXX  | E | Re-try Cnt | Offset [9:2] |
   Data Mask (64 bit data requires 64 bit alignment)
   Expected Data (64 bit data requires 64 bit alignment)
*/ 
void reconfig_command_read(reconfig_stage_t stage, uint64_t addr, uint64_t value, uint64_t mask, uint32_t retry_cnt, uint32_t is_reg64)
{
	ASSERT(is_reg64 || value <= UINT32_MAX);
	ASSERT((retry_cnt & ~0xff) == 0);
	ASSERT((addr & 0x3) == 0);
 	validate_stage(stage);
	reconfig_write_pending(stage);

	dprintf(DEBUG_SPEW, "%s: Stage %s, Address 0x%llx, Value 0x%llx, Mask 0x%llx, Retry Cnt 0x%x, Retry Enable %s, is_reg64 %s\n", __FUNCTION__, reconfig_stage_names[stage], addr, value, mask, retry_cnt, retry_cnt ? "true":"false", is_reg64 ? "true":"false");

	uint32_t cmd = READ_COMMAND;
	uint32_t retry_enable = retry_cnt ? 1:0;
	cmd |= ((is_reg64 & 0x1) << 5);
	cmd |= ((addr >> 10) << 6);			// 1 KB Base address

	rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = cmd;
	rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = ((retry_enable & 0x1) << 16) | ((retry_cnt & 0xff) << 8) | ((addr & 0x3FF) >> 2);

	if (is_reg64) {
		if (!ALIGNED_64(reconfig_count[stage])) {
			rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = ALIGNMENT_MARKER;
		}
		rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = (uint32_t)(mask);
		rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = (uint32_t)(mask>32);
		rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = (uint32_t)(value);
		rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = (uint32_t)(value>>32);
	} else {
		rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = (uint32_t)mask;
		rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = (uint32_t)(value);
	}
	bounds_check_stage(stage);
}

/* Delay Command:
   Cmd
   	31                       6  5   4  3  2  1   0
	|      Count[25:0]        | 0   0  0  1  0   0 |
*/
void reconfig_command_delay(reconfig_stage_t stage, uint32_t delay)
{
	ASSERT((delay & ~0x3FFFFFF) == 0);
	validate_stage(stage);
	reconfig_write_pending(stage);

	dprintf(DEBUG_SPEW, "%s: Stage %s, Delay 0x%x cycles\n", __FUNCTION__, reconfig_stage_names[stage], delay);

	uint32_t cmd = DELAY_COMMAND;
	cmd |= ((delay & 0x3FFFFFF) << 6);

	rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = cmd;
	bounds_check_stage(stage);
}

/* "NOP" Command:
    Cmd: 0x44, same as delay cmd 
*/
void reconfig_command_nop(reconfig_stage_t stage) 
{
	validate_stage(stage);
	reconfig_write_pending(stage);

	dprintf(DEBUG_SPEW, "%s: Stage %s NOP command\n", __FUNCTION__, reconfig_stage_names[stage]);

	rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = 0x00000040 | NOP_COMMAND;
	bounds_check_stage(stage);
}

void reconfig_command_raw(reconfig_stage_t stage, const uint32_t *cmd, uint32_t cmdItems)
{
	validate_stage(stage);
	reconfig_write_pending(stage);

	// align to 64 bits. The config sequences script also makes an assumption
	// that it starts on a 64 bit aligned location
	if (reconfig_count[stage] % 2)
		reconfig_command_nop(stage);

	for (uint32_t jj = 0; jj < cmdItems; jj++) {
		rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = cmd[jj];
	}
	bounds_check_stage(stage);
	
	dprintf(DEBUG_SPEW, "%s: Committed raw command to memory\n", __FUNCTION__);
}

void reconfig_init(enum boot_target target)
{
	addr_t table_base;

	switch (target) {
		case BOOT_DARWIN:
			reconfig_bases = reconfig_bases_darwin;
			table_base = AOP_CFG_TABLE;
			break;
		case BOOT_DIAGS:
			reconfig_bases = reconfig_bases_diags;
			table_base = AOP_CFG_TABLE_DIAGS;
			break;
		default:
			panic("unknown target: %d", target);
	}

	// Set the CFG table pointer and CFG table entries
	rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_TABLE_BASE_OFFSET) = (uint32_t)(table_base & AOP_GLOBAL_SOC_CFG_TABLE_BASE_ADDR_UMASK);
	volatile uint32_t *aop_cfg_base = (volatile uint32_t *)table_base;
	*aop_cfg_base++ = (uint32_t)(reconfig_bases[AWAKE_AOP_DDR_PRE] >> 4);
	*aop_cfg_base++ = (uint32_t)(reconfig_bases[AWAKE_AOP_DDR_POST] >> 4);
	*aop_cfg_base++ = (uint32_t)(reconfig_bases[AOP_DDR_S2R_AOP_PRE] >> 4);
	*aop_cfg_base++ = (uint32_t)(reconfig_bases[AOP_DDR_S2R_AOP_POST] >> 4);
	*aop_cfg_base++ = (uint32_t)(reconfig_bases[S2R_AOP_AOP_DDR_PRE] >> 4);
	*aop_cfg_base++ = (uint32_t)(reconfig_bases[S2R_AOP_AOP_DDR_POST] >> 4);
	*aop_cfg_base++ = (uint32_t)(reconfig_bases[AOP_DDR_AWAKE_PRE] >> 4);
	*aop_cfg_base++ = (uint32_t)(reconfig_bases[AOP_DDR_AWAKE_POST] >> 4);

	dprintf(DEBUG_SPEW, "Initialize Config table and bases for reconfig stages\n");
}

void reconfig_commit(reconfig_stage_t stage)
{
	validate_stage(stage);
	reconfig_write_pending(stage);

	uint32_t params = 0;

	switch (stage) {
		case AWAKE_AOP_DDR_PRE:		
			params = rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_AWAKE_TO_AOP_DDR_OFFSET);
			params &= (~AOP_GLOBAL_SOC_CFG_AWAKE_TO_AOP_DDR_PREAMBLE_ENABLE_UMASK);
			params &= (~AOP_GLOBAL_SOC_CFG_AWAKE_TO_AOP_DDR_PREAMBLE_UMASK);
			params |= AOP_GLOBAL_SOC_CFG_AWAKE_TO_AOP_DDR_PREAMBLE_ENABLE_INSRT(1);
			params |= AOP_GLOBAL_SOC_CFG_AWAKE_TO_AOP_DDR_PREAMBLE_INSRT(AWAKE_AOP_DDR_PRE);	
			rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_AWAKE_TO_AOP_DDR_OFFSET) = params;	
			break;
		case AWAKE_AOP_DDR_POST:
			params = rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_AWAKE_TO_AOP_DDR_OFFSET);
		        params &= (~AOP_GLOBAL_SOC_CFG_AWAKE_TO_AOP_DDR_POSTAMBLE_ENABLE_UMASK);	
			params &= (~AOP_GLOBAL_SOC_CFG_AWAKE_TO_AOP_DDR_POSTAMBLE_UMASK);
			params |= AOP_GLOBAL_SOC_CFG_AWAKE_TO_AOP_DDR_POSTAMBLE_ENABLE_INSRT(1);
			params |= AOP_GLOBAL_SOC_CFG_AWAKE_TO_AOP_DDR_POSTAMBLE_INSRT(AWAKE_AOP_DDR_POST);
			rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_AWAKE_TO_AOP_DDR_OFFSET) = params;
			break;
		case AOP_DDR_S2R_AOP_PRE:
			params = rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_S2R_AOP_OFFSET);
			params &= (~AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_S2R_AOP_PREAMBLE_ENABLE_UMASK);
			params &= (~AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_S2R_AOP_PREAMBLE_UMASK);
			params |= AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_S2R_AOP_PREAMBLE_ENABLE_INSRT(1);
			params |= AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_S2R_AOP_PREAMBLE_INSRT(AOP_DDR_S2R_AOP_PRE);
			rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_S2R_AOP_OFFSET) = params;
			break;
		case AOP_DDR_S2R_AOP_POST:					
			params = rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_S2R_AOP_OFFSET);
			params &= (~AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_S2R_AOP_POSTAMBLE_ENABLE_UMASK);
			params &= (~AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_S2R_AOP_POSTAMBLE_UMASK);
			params |= AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_S2R_AOP_POSTAMBLE_ENABLE_INSRT(1);
			params |= AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_S2R_AOP_POSTAMBLE_INSRT(AOP_DDR_S2R_AOP_POST);
			rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_S2R_AOP_OFFSET) = params;
			break;
		case S2R_AOP_AOP_DDR_PRE:		
			params = rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_S2R_AOP_TO_AOP_DDR_OFFSET);
			params &= (~AOP_GLOBAL_SOC_CFG_S2R_AOP_TO_AOP_DDR_PREAMBLE_ENABLE_UMASK);
			params &= (~AOP_GLOBAL_SOC_CFG_S2R_AOP_TO_AOP_DDR_PREAMBLE_UMASK);
			params |= AOP_GLOBAL_SOC_CFG_S2R_AOP_TO_AOP_DDR_PREAMBLE_ENABLE_INSRT(1);
			params |= AOP_GLOBAL_SOC_CFG_S2R_AOP_TO_AOP_DDR_PREAMBLE_INSRT(S2R_AOP_AOP_DDR_PRE);
			rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_S2R_AOP_TO_AOP_DDR_OFFSET) = params;
			break;
		case S2R_AOP_AOP_DDR_POST:					
			params = rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_S2R_AOP_TO_AOP_DDR_OFFSET); 
			params &= (~AOP_GLOBAL_SOC_CFG_S2R_AOP_TO_AOP_DDR_POSTAMBLE_ENABLE_UMASK);
			params &= (~AOP_GLOBAL_SOC_CFG_S2R_AOP_TO_AOP_DDR_POSTAMBLE_UMASK);
			params |= AOP_GLOBAL_SOC_CFG_S2R_AOP_TO_AOP_DDR_POSTAMBLE_ENABLE_INSRT(1);
			params |= AOP_GLOBAL_SOC_CFG_S2R_AOP_TO_AOP_DDR_POSTAMBLE_INSRT(S2R_AOP_AOP_DDR_POST);
			rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_S2R_AOP_TO_AOP_DDR_OFFSET) = params;
			break;
		case AOP_DDR_AWAKE_PRE: 
			params = rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_AWAKE_OFFSET);
			params &= (~AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_AWAKE_PREAMBLE_ENABLE_UMASK);
			params &= (~AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_AWAKE_PREAMBLE_UMASK);
			params |= AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_AWAKE_PREAMBLE_ENABLE_INSRT(1);
			params |= AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_AWAKE_PREAMBLE_INSRT(AOP_DDR_AWAKE_PRE);
			rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_AWAKE_OFFSET) = params;
			break;
		case AOP_DDR_AWAKE_POST:	
			params = rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_AWAKE_OFFSET);
			params &= (~AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_AWAKE_POSTAMBLE_ENABLE_UMASK);
			params &= (~AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_AWAKE_POSTAMBLE_UMASK);
			params |= AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_AWAKE_POSTAMBLE_ENABLE_INSRT(1);
			params |= AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_AWAKE_POSTAMBLE_INSRT(AOP_DDR_AWAKE_POST);
			rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_AOP_DDR_TO_AWAKE_OFFSET) = params;
			break;
		case MAX_STAGE:	panic("Unknown reconfig stage");
	}
	// End Command
	rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = 0;	
	rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = SEQUENCE_END;	
	rRECONFIG_RAM_BASE(reconfig_bases[stage], reconfig_count[stage]++) = SEQUENCE_END;	

	// XXX Lock entries
	// <rdar://problem/17754773>

	reconfig_committed[stage] = true;

	dprintf(DEBUG_SPEW, "%s: Enabled and committed stage %s, using 0x%x of 0x%llx bytes\n",
		__FUNCTION__, reconfig_stage_names[stage], reconfig_count[stage] * 4, reconfig_max_size[stage]);
}

void reconfig_lock(enum boot_target target)
{
	uint32_t lock_reg;
	addr_t lock_base;
	addr_t lock_limit;

	// Can't lock twice
	ASSERT(!reconfig_locked);
	ASSERT(rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_WRITE_LOCK_OFFSET) == 0);

	// It's possible to lock down a sub-region of the reconfig program. Since we aren't
	// currently modifying any of the sequences after iBoot finishes, we're locking down
	// the entire portion of SRAM used for reconfig engine tables and programs.
	// Details on locking down a portion of the program are in section 2.1.8 of
	// https://seg-docs.ecs.apple.com/projects/maui//release/specs/Apple/Top/Maui_boot_sequences.pdf
	switch (target) {
		case BOOT_DARWIN:
			lock_base = AOP_RECONFIG_REGION_BASE_ADDR;
			lock_limit = lock_base + AOP_RECONFIG_REGION_USED - 1;
			break;
		case BOOT_DIAGS:
			lock_base = AOP_RECONFIG_REGION_DIAGS_BASE_ADDR;
			lock_limit = lock_base + AOP_RECONFIG_REGION_DIAGS_USED - 1;
			break;
		default:
			panic("unknown target %d", target);
	}

	lock_reg  = AOP_GLOBAL_SOC_CFG_LOCKABLE_SRAM_BASE_INSRT(lock_base >> 6);
	lock_reg |= AOP_GLOBAL_SOC_CFG_LOCKABLE_SRAM_LIMIT_INSRT(lock_limit >> 6);
	rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_LOCKABLE_SRAM_OFFSET) = lock_reg;

	// Disable writes to the lock region register
	rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_WRITE_LOCK_OFFSET) = 1;
	RELEASE_ASSERT(rAOP_GLOBAL(AOP_GLOBAL_SOC_CFG_WRITE_LOCK_OFFSET) == 1);
}

void dump_reconfig(reconfig_dump_t option)
{
	if ((option == REGION_ONLY) || (option == REGION_AND_COMMAND)) {
		dprintf(DEBUG_INFO, "\nReconfig region bases\n");
		dprintf(DEBUG_INFO, "~~~~~~~~~~~~~~~~~~~~~~\n");
		dprintf(DEBUG_INFO, "AOP_CFG_TABLE                      0x%9llx\n", AOP_CFG_TABLE);
		dprintf(DEBUG_INFO, "AWAKE_AOP_DDR_PRE_BASE_ADDR        0x%9llx\n", reconfig_bases[AWAKE_AOP_DDR_PRE]);
		dprintf(DEBUG_INFO, "AWAKE_AOP_DDR_POST_BASE_ADDR       0x%9llx\n", reconfig_bases[AWAKE_AOP_DDR_POST]);
		dprintf(DEBUG_INFO, "AOP_DDR_S2R_AOP_PRE_BASE_ADDR      0x%9llx\n", reconfig_bases[AOP_DDR_S2R_AOP_PRE]);
		dprintf(DEBUG_INFO, "AOP_DDR_S2R_AOP_POST_BASE_ADDR     0x%9llx\n", reconfig_bases[AOP_DDR_S2R_AOP_POST]);
		dprintf(DEBUG_INFO, "S2R_AOP_AOP_DDR_PRE_BASE_ADDR      0x%9llx\n", reconfig_bases[S2R_AOP_AOP_DDR_PRE]);
		dprintf(DEBUG_INFO, "S2R_AOP_AOP_DDR_POST_BASE_ADDR     0x%9llx\n", reconfig_bases[S2R_AOP_AOP_DDR_POST]);
		dprintf(DEBUG_INFO, "AOP_DDR_AWAKE_PRE_BASE_ADDR        0x%9llx\n", reconfig_bases[AOP_DDR_AWAKE_PRE]);
		dprintf(DEBUG_INFO, "AOP_DDR_AWAKE_POST_BASE_ADDR       0x%9llx\n", reconfig_bases[AOP_DDR_AWAKE_POST]);
	}

	if ((option == COMMAND_ONLY) || (option == REGION_AND_COMMAND)) {
		for (uint32_t ii = 0; ii < MAX_STAGE; ii++) {
			dprintf(DEBUG_INFO, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
			dprintf(DEBUG_INFO, "\n%s\n", reconfig_stage_names[ii]);
			for (uint32_t jj = 0; jj < reconfig_count[ii]; jj++) {
				dprintf(DEBUG_INFO, "0x%08x ", rRECONFIG_RAM_BASE(reconfig_bases[ii], jj++));
				dprintf(DEBUG_INFO, "0x%08x ", rRECONFIG_RAM_BASE(reconfig_bases[ii], jj++));
				dprintf(DEBUG_INFO, "0x%08x ", rRECONFIG_RAM_BASE(reconfig_bases[ii], jj++));
				dprintf(DEBUG_INFO, "0x%08x\n", rRECONFIG_RAM_BASE(reconfig_bases[ii], jj++));
			}
			dprintf(DEBUG_INFO, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		}
	}
}
