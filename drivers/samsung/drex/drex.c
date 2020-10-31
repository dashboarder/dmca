/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/miu.h>
#include <platform.h>
#include <platform/chipid.h>
#include <platform/clocks.h>
#include <platform/gpio.h>
#include <platform/soc/hwclocks.h>

#include "drex.h"

#if (TARGET_DREX_CLK_RATIO != 1) && (TARGET_DREX_CLK_RATIO != 2)
#error "Invalid TARGET_DREX_CLK_RATIO"
#endif

static u_int32_t calc_sdram_refresh(u_int32_t tREFI, int clock);

static uint8_t drex_mrr(uint8_t address)
{
	uint32_t cmd_type = 9;
	uint32_t cmd_addr = ((address >> 6) & 0x3) | ((address & 0x7) << 10);
	uint32_t cmd_bank = (address >> 3) & 0x7;

	rDIRECTCMD = (cmd_type << 24) | (cmd_bank << 16) | (cmd_addr);
	spin(2);

	return rMRSTATUS;
}

static void drex_mrw(uint8_t address, uint8_t data)
{
	uint32_t cmd_type = 0;
	uint32_t cmd_addr = (address >> 6) | (data << 2) | ((address & 0x7) << 10);
	uint32_t cmd_bank = (address >> 3) & 0x7;

	rDIRECTCMD = (cmd_type << 24) | (cmd_bank << 16) | (cmd_addr);
	spin(2);
}

int  mcu_initialize_dram(bool resume)
{
#if !SUPPORT_FPGA
	// Change PhyZQControl.ctrl_zq_mode_dds to 4 for DDR2 400MHz. The default value, 7, is for DDR3 667MHz
	rPHYZQCONTROL = 0xE3855431;
	// Set ctrl_start_point and ctrl_inc, enable differential DQS.
	// Set ctrl_dll_on to activate the PHY DLL
	rPHYCONTROL0 = (0x10 << 16) | (0x10 << 8) | (0x1 << 3) | (0x1 << 1);
	// Set ctrl_shiftc = 5 (90 degree shift), ctrl_ref = 8
	rPHYCONTROL1 = (0x00 << 8) | (0x8 << 4) | 0x5;
	// Set ctrl_start
	rPHYCONTROL0 = (0x10 << 16) | (0x10 << 8) | (0x1 << 3) | (0x1 << 1) | 0x1;
#endif
	rCONCONTROL = 0x0FFF30C0 | ((TARGET_DREX_CLK_RATIO - 1) << 1);
	rMEMCONTROL = 0x00202500;
	rMEMCONFIG = 0x00F01223;

	// Open page policy for all banks, maximum timeout before closing page
	rPRECHCONFIG = 0xFF000000;
	// Maximum timeout for going into self-refresh and dynamic power-down
	rPWRDNCONFIG = 0xFFFF00FF;

	rTIMINGAREF = TARGET_DREX_TIMING_AREF;
	rTIMINGROW = TARGET_DREX_TIMING_ROW;
	rTIMINGDATA = TARGET_DREX_TIMING_DATA;
	rTIMINGPOWER = TARGET_DREX_TIMING_POWER;

#if !SUPPORT_FPGA
	// Wait for ctrl_locked to be set
	while ((rPHYSTATUS & (1 << 2)) == 0);
	// Prepare for the Force DLL resynchronization
	rPHYCONTROL1 = (0x00 << 8) | (0x8 << 4) | (0x0 << 3) | 0x5;
	// Force DLL resynchronization
	rPHYCONTROL1 = (0x00 << 8) | (0x8 << 4) | (0x1 << 3) | 0x5;

	// ZQ calibration
	// zq_start
	rPHYZQCONTROL |= (1 << 1);
	// wait for ctrl_zq_end
	while ((rPHYSTATUS & (1 << 16)) == 0);
	// fp_resync for ZQ result update
	rPHYCONTROL1 &= ~(1 << 3);
	rPHYCONTROL1 |= 1 << 3;
	rPHYCONTROL1 &= ~(1 << 3);
	// zq_stop
	rPHYZQCONTROL &= ~(1 << 1);
#else
	spin(100000);
#endif

	// NOP: exit from active/precharge power down or deep power down
	rDIRECTCMD = 0x07000000;
	spin(250);

	// MRW63: reset memory devices and do device auto-initialization
	drex_mrw(63, 0);

	spin(10);

	// RL = 6, WL = 3
	drex_mrw(2, 4);

	// Drive strength 48-ohm typical
	drex_mrw(3, 3);

	// Turn on auto-refresh counter
	rCONCONTROL |= (1 << 5);

	// Tunables for QoS, see radar 11065185
	rQOSTMDREX = TARGET_DREX_QOS_TIDEMARK;
	rQOSACDREX = TARGET_DREX_QOS_ACCESS;

	return 0;
}

void mcu_adjust_performance(void)
{
}

uint64_t mcu_get_memory_size(void)
{
	return chipid_get_memory_density() * chipid_get_memory_ranks() * (chipid_get_memory_width() / 32);
}
