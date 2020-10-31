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

#include "iboot/amp_v3_shim.h"

///////////////////////////////////////////////////////////////////////////////
////// Global functions
///////////////////////////////////////////////////////////////////////////////

void amc_phy_preinit(void)
{
	uint32_t ch;
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		CSR_WRITE(rAMP_DLLUPDTCTRL(AMP_DQ0, ch), 0x00017307);
		CSR_WRITE(rAMP_DLLUPDTCTRL(AMP_DQ1, ch), 0x00017307);
		CSR_WRITE(rAMP_DLLUPDTCTRL(AMP_CA, ch), 0x00017307);
	}
}

void amc_phy_init(bool resume)
{
	uint32_t ch, f;
#if !SUPPORT_FPGA
	uint32_t rd, dq;
#endif

	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		CSR_WRITE(rAMP_AMPEN(AMP_CA, ch), 1);
		CSR_WRITE(rAMP_AMPEN(AMP_DQ0, ch), 1);
		CSR_WRITE(rAMP_AMPEN(AMP_DQ1, ch), 1);
		
#if !SUPPORT_FPGA
		CSR_WRITE(rAMP_DQDQSDS(AMP_DQ0, ch), amc_phy_params.dqdqsds);
		CSR_WRITE(rAMP_DQDQSDS(AMP_DQ1, ch), amc_phy_params.dqdqsds);
		CSR_WRITE(rAMP_NONDQDS(ch), amc_phy_params.nondqds);
		
		for (f = 0; f < AMP_FREQUENCY_SLOTS; f++) {
			CSR_WRITE(rAMP_DIFFMODE_FREQ(AMP_DQ0, ch, f), 0x00000121);
			CSR_WRITE(rAMP_DIFFMODE_FREQ(AMP_DQ1, ch, f), 0x00000121);
		}
		
		for (rd = 0; rd < AMP_MAX_RD; rd++) {
			for (dq = 0; dq < AMP_MAX_DQ; dq++) {
				CSR_WRITE(rAMP_RDDQDESKEW_CTRL(AMP_DQ0, ch, rd, dq), 0x00000006);
				CSR_WRITE(rAMP_RDDQDESKEW_CTRL(AMP_DQ1, ch, rd, dq), 0x00000006);
			}
		}
		
		CSR_WRITE(rAMP_DLLLOCKTIM(AMP_CA, ch), 0x000d0013);
		CSR_WRITE(rAMP_DLLLOCKTIM(AMP_DQ0, ch), 0x000d0013);
		CSR_WRITE(rAMP_DLLLOCKTIM(AMP_DQ1, ch), 0x000d0013);
#endif

		for (f = 0; f < AMP_FREQUENCY_SLOTS; f++) {
			CSR_WRITE(rAMP_CAOUTDLLSCL_FREQ(ch, f), amc_phy_params.freq[f].caoutdllscl);
			CSR_WRITE(rAMP_DQSINDLLSCL_FREQ(AMP_DQ0, ch, f), amc_phy_params.freq[f].dqsindllscl);
			CSR_WRITE(rAMP_DQSINDLLSCL_FREQ(AMP_DQ1, ch, f), amc_phy_params.freq[f].dqsindllscl);

			CSR_WRITE(rAMP_RDCAPCFG_FREQ(AMP_DQ0, ch, f), amc_phy_params.freq[f].rdcapcfg);
			CSR_WRITE(rAMP_RDCAPCFG_FREQ(AMP_DQ1, ch, f), amc_phy_params.freq[f].rdcapcfg);
			
			CSR_WRITE(rAMP_DQODTVREF_F(AMP_DQ0, ch, f), 0x00000080);
			CSR_WRITE(rAMP_DQODTVREF_F(AMP_DQ1, ch, f), 0x00000080);
		}

#if !SUPPORT_FPGA
		CSR_WRITE(rAMP_DLLUPDTCTRL(AMP_CA, ch), 0x00017507);
		CSR_WRITE(rAMP_DLLUPDTCTRL(AMP_DQ0, ch), 0x00017507);
		CSR_WRITE(rAMP_DLLUPDTCTRL(AMP_DQ1, ch), 0x00017507);
		
		CSR_WRITE(rAMP_IMPAUTOCAL(AMP_DQ0, ch), 0x00010000);
		CSR_WRITE(rAMP_IMPAUTOCAL(AMP_DQ1, ch), 0x00010000);
		CSR_WRITE(rAMP_IMPAUTOCAL(AMP_CA, ch), 0x00010000);

		CSR_WRITE(rAMP_DLLUPDTINTVL(AMP_CA, ch), 0x10200020);
		CSR_WRITE(rAMP_DLLUPDTINTVL(AMP_DQ0, ch), 0x10200020);
		CSR_WRITE(rAMP_DLLUPDTINTVL(AMP_DQ1, ch), 0x10200020);

		CSR_WRITE(rAMP_DLLEN(AMP_CA, ch), 0x00000100);
		CSR_WRITE(rAMP_DLLEN(AMP_DQ0, ch), 0x00000100);
		CSR_WRITE(rAMP_DLLEN(AMP_DQ1, ch), 0x00000100);
		
		CSR_WRITE(rAMP_DLLEN(AMP_CA, ch), 0x00000101);
		CSR_WRITE(rAMP_DLLEN(AMP_DQ0, ch), 0x00000101);
		CSR_WRITE(rAMP_DLLEN(AMP_DQ1, ch), 0x00000101);

		CSR_WRITE(rAMP_DLLEN(AMP_CA, ch), 0x00000100);
		CSR_WRITE(rAMP_DLLEN(AMP_DQ0, ch), 0x00000100);
		CSR_WRITE(rAMP_DLLEN(AMP_DQ1, ch), 0x00000100);
		
		amc_phy_run_dll_update(ch);
		
		CSR_WRITE(rAMP_IMPCALCMD(AMP_CA, ch), 0x00000101);
		CSR_WRITE(rAMP_IMPCALCMD(AMP_DQ0, ch), 0x00000101);
		CSR_WRITE(rAMP_IMPCALCMD(AMP_DQ1, ch), 0x00000101);

		while (CSR_READ(rAMP_IMPCALCMD(AMP_CA, ch)) & 0x1) {}
		while (CSR_READ(rAMP_IMPCALCMD(AMP_DQ0, ch)) & 0x1) {}
		while (CSR_READ(rAMP_IMPCALCMD(AMP_DQ1, ch)) & 0x1) {}
#endif
		
		CSR_WRITE(rAMP_AMPINIT(AMP_CA, ch), 0x00000001);
		CSR_WRITE(rAMP_AMPINIT(AMP_DQ0, ch), 0x00000001);
		CSR_WRITE(rAMP_AMPINIT(AMP_DQ1, ch), 0x00000001);
	}
}

void amc_phy_run_dll_update(uint8_t ch)
{
	CSR_WRITE(rAMP_DLLUPDTCMD(AMP_CA, ch), 0x00000001);
	CSR_WRITE(rAMP_DLLUPDTCMD(AMP_DQ0, ch), 0x00000001);
	CSR_WRITE(rAMP_DLLUPDTCMD(AMP_DQ1, ch), 0x00000001);
	
	while ((CSR_READ(rAMP_DLLUPDTCMD(AMP_CA, ch)) & 0x1)) ;
	while ((CSR_READ(rAMP_DLLUPDTCMD(AMP_DQ0, ch)) & 0x1)) ;
	while ((CSR_READ(rAMP_DLLUPDTCMD(AMP_DQ1, ch)) & 0x1)) ;
}

// <rdar://problem/15810647>: workaround to avoid memory read failure when switching from boot speed (f = 3) to normal speed (f = 0)
void amc_phy_pre_normal_speed_enable()
{
	uint8_t ch, f;
 	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		for (f = 1; f < AMP_FREQUENCY_SLOTS; f++) {
			CSR_WRITE(rAMP_CAOUTDLLSCL_FREQ(ch, f), amc_phy_params.freq[0].caoutdllscl);
			CSR_WRITE(rAMP_DQSINDLLSCL_FREQ(AMP_DQ0, ch, f), amc_phy_params.freq[0].dqsindllscl);
			CSR_WRITE(rAMP_DQSINDLLSCL_FREQ(AMP_DQ1, ch, f), amc_phy_params.freq[0].dqsindllscl);
		}

#if !SUPPORT_FPGA
		amc_phy_run_dll_update(ch);
#endif
	}
}
	
void amc_phy_finalize()
{
	uint8_t ch;
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++) {
		CSR_WRITE(rAMP_AMPCLK(AMP_CA, ch), amc_phy_params.ampclk);
		CSR_WRITE(rAMP_AMPCLK(AMP_DQ0, ch), amc_phy_params.ampclk);
		CSR_WRITE(rAMP_AMPCLK(AMP_DQ1, ch), amc_phy_params.ampclk);
	}
}

void amc_phy_bypass_prep(int step)
{
}

void amc_phy_restore_calibration_values(bool resume)
{
#if !SUPPORT_FPGA
#ifndef AMP_CALIBRATION_SKIP
	// Restore offsets from PMU
	if (resume)
		calibration_save_restore_regs(CALIB_RESTORE, AMC_NUM_CHANNELS);
#endif
#endif
}

// Perform CA, RDDQ, and WRLVL calibration
void amc_phy_calibration_ca_rddq_cal(bool resume)
{
#ifndef AMP_CALIBRATION_SKIP
	if (!resume)
		calibration_ca_rddq_wrlvl(resume);
#endif
}

void amc_phy_calibration_wrdq_cal(bool resume)
{
#ifndef AMP_CALIBRATION_SKIP
	if (!resume)
		calibration_wrdq_rddq(resume);
#endif
}
