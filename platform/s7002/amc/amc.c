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
#include <drivers/amc/amc.h>
#include <drivers/amc/amc_phy.h>
#include <drivers/amc/amc_regs.h>
#include <drivers/amp/amp_v2.h>
#include <drivers/amp/amp_v2_calibration.h>
#include <drivers/dram.h>
#include <drivers/miu.h>
#include <sys.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/reconfig.h>

int mcu_initialize_dram (bool resume)
{
	return amc_init(resume);
}

void mcu_bypass_prep (int step)
{
	amc_phy_bypass_prep(step);
}

uint64_t mcu_get_memory_size (void)
{
	return amc_get_memory_size();
}

void amc_configure_address_decoding_and_mapping (void)
{
	rAMC_ADDRCFG = amc_params.addrcfg;

	rAMC_CHNLDEC = amc_params.chnldec;

	rAMC_ADDRMAP_MODE = 2;				// RIBI2

	rAMC_AIU_ADDRBANKHASH2 = amc_params.bankhash2;
	rAMC_AIU_ADDRBANKHASH1 = amc_params.bankhash1;
	rAMC_AIU_ADDRBANKHASH0 = amc_params.bankhash0;
}

void amc_enable_slow_boot (bool enable)
{
	if (enable) {
		// switch to slow clock
		clocks_set_performance(kPerformanceMemoryLow);
		spin(1);
	} else {
#if !SUPPORT_FPGA	
		// switch back to full-speed
		clocks_set_performance(kPerformanceMemoryFull);
		spin(1);
#endif
	}
}

// Some AMC features to be changed before calibration starts, and restored after calibration is complete
void amc_calibration_start(bool start)
{
	if (start)
		// Disable OdtsRdIntrvl
		rAMC_ODTS &= 0xFFFFFC00;
	else
		// Re-enable OdtsRdIntrvl
		rAMC_ODTS |= amc_params.odts;
}

void amc_enable_rddqcal(bool enable)
{
	if (enable)
		rAMC_READ_LEVELING |= 1;
	else
 		rAMC_READ_LEVELING &= ~1;
}

void amc_wrdqcal_start(bool start)
{
	if (start) {
		// Enable WriteMergeEn and WqInOrderEn
		rAMC_PSQWQCTL0 = (1 << 8) | (1 << 0);
		
		// Set SelfRefTmrVal to max
		rAMC_PWRMNGTPARAM |= (0xFFFF << 16);
		
		// Designer suggests 0xBB, so that writes do not age out
		rAMC_PSQWQCTL1 = 0xBB;
	} else {
		rAMC_PSQWQCTL0 = 0x00010100;
		rAMC_PWRMNGTPARAM = amc_params.pwrmngtparam_guided;
		// WqAgeOutVal has to be set to 3/4 of SelfRefTmrVal
		rAMC_PSQWQCTL1 = (3 * ((amc_params.pwrmngtparam_guided & 0xFFFF0000) >> 16)) >> 2;
		
		// Enabling AutoSR only after wrdqcal is done
		rAMC_PWRMNGTEN 	|= 0x00011011;
	}
}

void amc_finalize (bool resume)
{
	rAMC_AREFPARAM	|= 0x00001800;

	rAMC_QBREN	= 0x00110001;

	if (!resume) {
#if SUPPORT_FPGA
		rAMC_ODTS	= amc_params.odts;
#else
		rAMC_ODTS	= 0x00010000 | amc_params.odts;
#endif
		amc_enable_autorefresh();
	}

	rAMC_ZQC 		= 0x010c03ff;
#if !SUPPORT_FPGA	
	rAMC_AIUPRT_RD_CWF	= 0x00323200;
#endif	
	rAMC_QBRPARAM		= 0x00510000;
	rAMC_QBREN		= 0x00111001;

	rAMC_PSQWQCTL0		= 0x00010100;

#if SUPPORT_FPGA
	rAMC_PWRMNGTEN	= 0x00110010;
#else
	rAMC_PWRMNGTEN	= 0x00111011;
	rAMC_PWRMNGTPARAM = amc_params.pwrmngtparam_guided;
#endif
}

void amc_dram_workarounds (bool resume)
{
	const struct amc_memory_device_info *dev_info;

	dev_info = amc_get_memory_device_info();

	switch (dev_info->vendor_id) {
	case JEDEC_MANUF_ID_HYNIX:
		// No work-arounds for Hynix ... yet!
		break;

	case JEDEC_MANUF_ID_ELPIDA:
		/* Filter only for Elpida 25nm to apply test MRS */
		if ((dev_info->rev_id == 0x2) && (dev_info->rev_id2 <= 0x2)) {  // MR6 = 2(25nm) and MR7 = 2(Rev2)
			if (!resume) {
				// Elpida has no TestMRS entry/exit cmd
				// <rdar://problem/15465954> Fiji: Need Duty Cycle Adjust work-around to improve Fiji+Elpida 25nm DDR eye margin
				amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0xbb);
				amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x9, 0x27);
			}
		}
		break;

	default:
		break;
	}

	return;
}

// Shift dq offset as needed.
// In the future, it's possible the shift value changes based on SOC or DRAM vendor.
// Thus, we need to have this function per SOC.
void amc_dram_shift_dq_offset (int8_t *dq_offset, uint8_t num_bytes) {
	int8_t shift_val;
	uint8_t i;
	const struct amc_memory_device_info *dev_info;
	
	dev_info = amc_get_memory_device_info();
	
	// No shifts defined by SEG yet for M7
	switch (dev_info->vendor_id) {			
		case JEDEC_MANUF_ID_HYNIX:
		case JEDEC_MANUF_ID_SAMSUNG:
		case JEDEC_MANUF_ID_ELPIDA:
		default:
			shift_val = 0;
			break;
	}
	
	if (shift_val)
		for (i = 0; i < num_bytes; i++)
			dq_offset[i] += shift_val;	
}
#if WITH_HW_RECONFIG

#define CALIB_TYPE_CAWRLVL	0
#define CALIB_TYPE_DQWRLVL	1

// <rdar://problem/16356260> M7 MEM Reconfig sequence: remove phyupdates
#if 0
static void amp_phy_update_reconfig(uint32_t update)
{
	// Release CKE and disable phyupdt to allow normal operation
	if (!update)
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_TESTMODE(AMP_CA,0), 0, 0);
	
	// issue phyupdt to block AMC traffic
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_CAPHYUPDTCRTL(0), update, 0);
	
	// wait for the phyupdt change to take effect. there is only 1 bit in the status reg: bit 0.
	reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t) &rAMP_CAPHYUPDTSTATUS(0)) | RECONFIG_RAM_CMD_READ, (update & 1), 1);
	
	// CKE must be low when updating dlysel to avoid glitches
	if (update)
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_TESTMODE(AMP_CA,0), TESTMODE_FORCECKELOW, 0);
}
#endif

static void amp_wrlvl_program_reconfig(uint32_t calib_type, uint32_t byte)
{
	addr_t codeaddr, dlyaddr;
	uint32_t code, dly;
	
	if (calib_type == CALIB_TYPE_CAWRLVL) {
		codeaddr = (addr_t) &rAMP_CAWRLVLSDLLCODE(0);
		code = rAMP_CAWRLVLSDLLCODE(0);
		dlyaddr = (addr_t) &rAMP_CAWRLVLCLKDLYSEL(0);
		dly = rAMP_CAWRLVLCLKDLYSEL(0);
	} else {
		codeaddr = (addr_t) &rAMP_DQWRLVLSDLLCODE(0, byte);
		dlyaddr = (addr_t) &rAMP_DQWRLVLDLYCHAINCTRL(0, byte);
		code = rAMP_DQWRLVLSDLLCODE(0, byte);
		dly = rAMP_DQWRLVLDLYCHAINCTRL(0, byte);
	}
	
	reconfig_append_command(RECONFIG_TYPE_MEM, codeaddr, code, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, dlyaddr, dly, 0);
}

void amc_configure_mem_reconfig_ram(void)
{
	uint32_t f, i;
#if !SUPPORT_FPGA
	uint32_t byte, bit;
#endif

	/* Step 1: AMC Initial Configuration */
	
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLSTS(AMP_DQ,0), 0x00017307, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLSTS(AMP_CA,0), 0x00017307, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_LAT, (amc_params.rdlat << 16) | (amc_params.wrlat << 8), 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_PHYRDWRTIM, (amc_params.phywrlat << 16) | (amc_params.phyrdlat << 8) | (amc_params.rdlat - 2), 0);
	
	for(f = 0; f < AMC_FREQUENCY_SLOTS; f++) {
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_CAS_FREQ(f), amc_params.freq[f].cas, 0);
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_PCH_FREQ(f), amc_params.freq[f].pch, 0);
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_ACT_FREQ(f), amc_params.freq[f].act, 0);
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AUTO_FREQ(f), amc_params.freq[f].autoref | ((f == 0) ? amc_params.tREFi : 0), 0);
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_SELF_FREQ(f), amc_params.freq[f].selfref, 0);
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_MODE_FREQ(f), amc_params.freq[f].modereg, 0);
	}
	
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AUTOREF_PARAMS, amc_params.autoref_params, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_PDN, amc_params.pdn, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_DERATE, amc_params.derate, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_RD, amc_params.read, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_BUSTAT, amc_params.bustat, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_BUSTAT_FREQ23, amc_params.bustat2, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_TREFBWBASECYC_FREQ(0), amc_params.freq[0].trefbwbasecyc, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_TREFBWBASECYC_FREQ(1), amc_params.freq[1].trefbwbasecyc, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_TREFBWBASECYC_FREQ(2), amc_params.freq[2].trefbwbasecyc, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_MIFCASSCH_FREQ(0), amc_params.freq[0].mifcassch, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_CH0RNKCFG0, 1, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_PWRMNGTEN, amc_params.pwrmngten_default, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_SCHEN, amc_params.schen_default, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_MCPHY_UPDTPARAM1, amc_params.mcphyupdate1, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_MCPHY_UPDTPARAM, amc_params.mcphyupdate, 0);
	
	// AMCEN
	{
		uint32_t amc_en = 0x00000001;

		if (amc_params.flags & FLAG_AMC_PARAM_LEGACY)
		{
			amc_en |= 0x80000000;
		}
		if (amc_params.flags & FLAG_AMC_PARAM_ENABLE_AIU)
		{
			amc_en |= 0x10000;
		}

		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AMCEN, amc_en, 0);
	}
	
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AIU_ADDRBANKHASH2, amc_params.bankhash2, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AIU_ADDRBANKHASH1, amc_params.bankhash1, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AIU_ADDRBANKHASH0, amc_params.bankhash0, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_ADDRMAP_MODE, 2, 0);
	
	/* Step 2: AMP Initial Configurations */
	
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_AMPEN(AMP_CA,0), 1, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_AMPEN(AMP_DQ,0), 1, 0);

#if !SUPPORT_FPGA
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DQDQSDS(0), amc_phy_params.drive_strength, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_NONDQDS(0), amc_phy_params.drive_strength, 0);
#endif
	// DIFFMODE_FREQ[n], n=0..3
	for(f = 0; f < AMP_FREQUENCY_SLOTS; f++)
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DIFFMODE_FREQ(0,f), 0x00000121, 0);
		
#if !SUPPORT_FPGA
	// RDnDQm_DESKEW_CTRL, n=0..3, m=0..7
	for(byte = 0; byte < AMP_MAX_RD; byte++)
		for(bit = 0; bit < AMP_MAX_DQ; bit++)
			reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_RDDQDESKEW_CTRL(0, byte, bit), 0x00000006, 0);
			
	// DLLLOCKTIM
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLLOCKTIM(AMP_CA,0), 0x000d0013, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLLOCKTIM(AMP_DQ,0), 0x000d0013, 0);
#endif

	for(f = 0; f < AMP_FREQUENCY_SLOTS; f++) {
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_CAOUTDLLSCL_FREQ(0,f), amc_phy_params.freq[f].caoutdllscl, 0);
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DQSINDLLSCL_FREQ(0,f), amc_phy_params.freq[f].dqsindllscl, 0);
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_RDCAPCFG_FREQ(AMP_DQ,0,f), amc_phy_params.freq[f].rdcapcfg, 0);
	}
	
#if !SUPPORT_FPGA
	// DLLUPDTCTRL
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLUPDTCTRL(AMP_CA,0), 0x00017507, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLUPDTCTRL(AMP_DQ,0), 0x00017507, 0);
	// IMPAUTOCAL
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_IMPAUTOCAL(AMP_DQ,0), amc_phy_params.imp_auto_cal, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_IMPAUTOCAL(AMP_CA,0), amc_phy_params.imp_auto_cal, 0);
#endif
	
	// DLLUPDTINTVL
#if !SUPPORT_FPGA
	uint32_t dllupdtintvl = 0x10200000;
#else
	uint32_t dllupdtintvl = 0;
#endif
	
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLUPDTINTVL(AMP_CA,0), dllupdtintvl, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLUPDTINTVL(AMP_DQ,0), dllupdtintvl, 0);

	// DLLEN sequence
#if !SUPPORT_FPGA
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLEN(AMP_CA,0), 0x00000100, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLEN(AMP_DQ,0), 0x00000100, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLEN(AMP_CA,0), 0x00000101, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLEN(AMP_DQ,0), 0x00000101, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLEN(AMP_CA,0), 0x00000100, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLEN(AMP_DQ,0), 0x00000100, 0);
#endif

	// MDLLFREQBINDISABLE
#if !SUPPORT_FPGA
	uint32_t mdllfreqbindisable = 0x00000008;
#else
	uint32_t mdllfreqbindisable = 0x0000000f;
#endif
	
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_MDLLFREQBINDISABLE(AMP_CA,0), mdllfreqbindisable, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_MDLLFREQBINDISABLE(AMP_DQ,0), mdllfreqbindisable, 0);

	// DLLUPDTCMD
#if !SUPPORT_FPGA
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLUPDTCMD(AMP_CA,0), 0x00000001, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLUPDTCMD(AMP_DQ,0), 0x00000001, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t) &rAMP_DLLUPDTCMD(AMP_CA,0)) | RECONFIG_RAM_CMD_READ, 0, 1);
	reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t) &rAMP_DLLUPDTCMD(AMP_DQ,0)) | RECONFIG_RAM_CMD_READ, 0, 1);
#endif

	// AMPINIT
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_AMPINIT(AMP_CA,0), 0x00000001, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_AMPINIT(AMP_DQ,0), 0x00000001, 0);
	// IMPCALCMD sequence
#if !SUPPORT_FPGA
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_IMPCALCMD(AMP_CA,0), 0x00000101, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_IMPCALCMD(AMP_DQ,0), 0x00000101, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t) &rAMP_IMPCALCMD(AMP_CA,0)) | RECONFIG_RAM_CMD_READ, 0, 1);
	reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t) &rAMP_IMPCALCMD(AMP_DQ,0)) | RECONFIG_RAM_CMD_READ, 0, 1);
#endif
	
	/* Step 3: Self-Refresh Exit */

#if !SUPPORT_FPGA
	/* CA calibration offsets */
	
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_CASDLLCTRL(0), (1 << 24) | (rAMP_CASDLLCTRL(0)), 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t) &rAMP_CASDLLCTRL(0)) | RECONFIG_RAM_CMD_READ, 0, (1 << 24));

	for (bit = 0; bit < CA_NUM_BITS; bit++)
			reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_CADESKEW_CTRL(0,bit), rAMP_CADESKEW_CTRL(0,bit), 0);

	// ck: skip if 0, since that is reset value
	uint32_t caclk = rAMP_CKDESKEW_CTRL(0);
	if (caclk) {
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_TESTMODE(AMP_CA,0), TESTMODE_FORCECKELOW, 0);
		
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DRAMSIGDLY(AMP_CA, 0, 0), rAMP_DRAMSIGDLY(AMP_CA, 0, 0), 0);
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_CSDESKEW_CTRL(0), caclk, 0);
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_CKDESKEW_CTRL(0), caclk, 0);
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_CKEDESKEW_CTRL(0), caclk, 0);
		
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_TESTMODE(AMP_CA,0), 0, 0);
	}
	
	/* WrLvl calibration offsets */

	amp_wrlvl_program_reconfig(CALIB_TYPE_CAWRLVL, 0);
	for (byte = 0; byte < DQ_NUM_BYTES; byte++)
		amp_wrlvl_program_reconfig(CALIB_TYPE_DQWRLVL, byte);
#endif
	
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AREFPARAM, amc_params.freq[0].arefparam, 0);
	for (f = 1; f < AMC_FREQUENCY_SLOTS; f++)
			reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AREFPARAM_FREQ(f), amc_params.freq[f].arefparam, 0);
		
#if !SUPPORT_FPGA
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_ODTS, amc_params.odts | 0x00010000, 0);
#else
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_ODTS, amc_params.odts, 0);
#endif
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_LONGSR, 0x01000000 | amc_params.longsrcnt | amc_params.srextrarefcnt, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AREFEN_FREQ(3), 0, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AREFEN_FREQ(2), 0, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AREFEN_FREQ(1), amc_params.aref_freq1, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AREFEN_FREQ(0), 0x01011001 | amc_params.aref_freq0, 0);
	// INITCMDCH0
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_CH0INITCMD, 0x1000, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t) &rAMC_CH0INITCMD) | RECONFIG_RAM_CMD_READ, 0, 0x1000);
	
	/* Step 4: DRAM Reset, skipped for resume boot */
	
	/* Step 5: Topology-specific configuration */
	
	// ZQC
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_ZQC, 0x00090000, 0);
	// MR10: ZQCL
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_CH0MRCMD, 0xab000a01, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t) &rAMC_CH01MRSTATUS) | RECONFIG_RAM_CMD_READ, 0, 1);
	
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_ZQC, 0x00080000, 0);
	// ADDRCFG
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_ADDRCFG, amc_params.addrcfg, 0);
	// CHNLDEC
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AIU_CHNLDEC, amc_params.chnldec, 0);
	
	/* Step 6: Switch from boot-clock speed to normal operation speed */
	
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_SCHEN, amc_params.schen_default | 0x1, 0);
	
	/* Step 7: CA and Rddq calibration, skipped for resume boot */
	
	/* Step 8: Enable other features */
		
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AREFPARAM, amc_params.freq[0].arefparam | 0x00001800, 0);
	// ZQC
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_ZQC, 0x010c03ff, 0);
	// QBREN
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_QBREN, 0x00110001, 0);
	// PSQWQCTRL0
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_PSQWQCTL0, 0x00010100, 0);
	
	/* Step 9: Enable the Fast Critical Word Forwarding feature */
	
	// RD_CWF
#if !SUPPORT_FPGA
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AIUPRT_RD_CWF, 0x00323200, 0);
#else
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_AIUPRT_RD_CWF, 0, 0);
#endif
	// QBRPARAM
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_QBRPARAM, 0x00510000, 0);
	// QBREN
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_QBREN, 0x00111001, 0);

	/* Step 10: Rddq and Wrdq offsets, restore dllupdtintvl POR value */

#if !SUPPORT_FPGA
	/* Rddq calibration offsets */
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DQSDLLCTRL_RD(0, byte),  (1 << 24) | rAMP_DQSDLLCTRL_RD(0, byte), 0);
		reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t) &rAMP_DQSDLLCTRL_RD(0, byte)) | RECONFIG_RAM_CMD_READ, 0, (1 << 24));
		
		for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++)
			reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_RDDQDESKEW_CTRL(0, byte, bit), rAMP_RDDQDESKEW_CTRL(0, byte, bit), 0);
	}
	
	/* Wrdq calibration offsets */
	
	for (byte = 0; byte < DQ_NUM_BYTES; byte++) {
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DQSDLLCTRL_WR(0, byte),  (1 << 24) | rAMP_DQSDLLCTRL_WR(0, byte), 0);
		reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t) &rAMP_DQSDLLCTRL_WR(0, byte)) | RECONFIG_RAM_CMD_READ, 0, (1 << 24));
		
		// rAMP_DQWRLVLDLYCHAINCTRL.WrLvlClk90Dly already restored as part of wrlvl offset restorations
		
		for (bit = 0; bit < DQ_NUM_BITS_PER_BYTE; bit++)
			reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_WRDQDESKEW_CTRL(0, byte, bit), rAMP_WRDQDESKEW_CTRL(0, byte, bit), 0);
		
		// DM controlled by DQS register
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_WRDQSDESKEW_CTRL(0, byte), rAMP_WRDQSDESKEW_CTRL(0, byte), 0);
		
		// DQS controlled by DM register
		uint32_t dqs_deskew = rAMP_WRDMDESKEW_CTRL(0, byte);
		// skip if 0, since that is reset value
		if (dqs_deskew)
			reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_WRDMDESKEW_CTRL(0, byte), dqs_deskew, 0);
	}

	// DLLUPDTCMD
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLUPDTCMD(AMP_CA,0), 0x00000001, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLUPDTCMD(AMP_DQ,0), 0x00000001, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t) &rAMP_DLLUPDTCMD(AMP_CA,0)) | RECONFIG_RAM_CMD_READ, 0, 1);
	reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t) &rAMP_DLLUPDTCMD(AMP_DQ,0)) | RECONFIG_RAM_CMD_READ, 0, 1);
#endif

	// DLLUPDTINTVL
#if !SUPPORT_FPGA
	dllupdtintvl = 0x1020005a;
#else
	dllupdtintvl = 0;
#endif
	
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLUPDTINTVL(AMP_CA,0), dllupdtintvl, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_DLLUPDTINTVL(AMP_DQ,0), dllupdtintvl, 0);
	
	/* Step 11: Enable Power & ClockGating features */
	
	// AMPCLK
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_AMPCLK(AMP_CA,0), 0x00010000, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMP_AMPCLK(AMP_DQ,0), 0x00010000, 0);
#if SUPPORT_FPGA
	// PWRMNGTEN
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_PWRMNGTEN, 0x00110010, 0);
#else
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_PWRMNGTEN, 0x00111011, 0);
#endif
#if !SUPPORT_FPGA
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_PWRMNGTPARAM, amc_params.pwrmngtparam_guided, 0);
	// PSQWQCTL1
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_PSQWQCTL1, 0x00000120, 0);
#endif
	
	/* Step 12: Do a ODTS Read */
	
	// MR4
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) &rAMC_CH0MRCMD, 0x00000411, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t) &rAMC_CH01MRSTATUS) | RECONFIG_RAM_CMD_READ, 0, 1);
    
    /* AMC Tunables */
    for (i = 0; i < sizeof(amc_tunables) / sizeof(amc_tunables[0]); i++) {
        if (amc_tunables[i].reg == 0)
        	break;
		reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t) amc_tunables[i].reg, amc_tunables[i].value, 0);
    }
    
}

#endif // WITH_HW_RECONFIG
