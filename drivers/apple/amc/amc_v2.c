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
#include <target.h>

// if there are separate ap/dev parameters, amc_init will copy
// the appropriate ones here
#if AMC_PARAMS_AP_DEV
static struct amc_param amc_params;
#endif
static bool amc_params_initialized;

static const struct amc_channel_addrs {
	volatile uint32_t	*rnkcfg;
	volatile uint32_t	*mrcmd;
	volatile uint32_t	*mrstatus;
	uint32_t		mrpoll;
	uint32_t		mrshift;
	volatile uint32_t	*initcmd;
	volatile uint32_t	*initstatus;
	uint32_t		mcuen;
} _amc_chregs[] = {
	{ &rAMC_CH0RNKCFG0, &rAMC_CH0MRCMD, &rAMC_CH01MRSTATUS, 0x01,  8, &rAMC_CH0INITCMD, &rAMC_CH01INITSTATUS, 0x0001 },
	{ &rAMC_CH1RNKCFG0, &rAMC_CH1MRCMD, &rAMC_CH01MRSTATUS, 0x10, 16, &rAMC_CH1INITCMD, &rAMC_CH01INITSTATUS, 0x0100 },
	{ &rAMC_CH2RNKCFG0, &rAMC_CH2MRCMD, &rAMC_CH23MRSTATUS, 0x01,  8, &rAMC_CH2INITCMD, &rAMC_CH23INITSTATUS, 0x0010 },
	{ &rAMC_CH3RNKCFG0, &rAMC_CH3MRCMD, &rAMC_CH23MRSTATUS, 0x10, 16, &rAMC_CH3INITCMD, &rAMC_CH23INITSTATUS, 0x1000 },
};

static struct amc_memory_device_info _amc_device_info;
static bool _amc_device_info_inited;

/*
 * Based on Fiji AMC Init.html (v48 - dated May 23, 2014). Tunables v7.1.0
 *          Capri AMC Init.html (v23 - dated May 23, 2014). Tunables v8.6.0
 *          j42d and j96 AMC Init.2GB.html (v48 - dated Dec 11, 2014)
 */

void amc_mrcmd(amc_mrcmd_op_t op, uint8_t channels, uint8_t ranks, int reg, uintptr_t val)
{
	uint8_t ch, r;
	
	for (ch = 0; ch < channels; ch++) {
		for (r = 0; r < ranks; r++) {
			amc_mrcmd_to_ch_rnk(op, ch, r, reg, val);
		}
	}
}

// Only send the cmd to specific channel and rank (used during calibration on H6 and later)
void amc_mrcmd_to_ch_rnk(amc_mrcmd_op_t op, uint8_t channel, uint8_t rank, int32_t reg, uintptr_t val)
{
	uint32_t cmd, regval;
	uint8_t *buffer = (uint8_t *) val;
	
	if (op == MR_READ)
		cmd = 0x00000011 | (reg << 8);
	else
		cmd = 0x00000001 | (reg << 8) | (((uint32_t)val) << 24);
	
	// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
	platform_memory_barrier();
	
	*_amc_chregs[channel].mrcmd = cmd | (rank << 16);
	while(((regval = *_amc_chregs[channel].mrstatus) & _amc_chregs[channel].mrpoll) != 0) ;
	if (op == MR_READ)
		*buffer++ = (regval >> _amc_chregs[channel].mrshift) & 0xff;
}

void amc_enable_autorefresh(void)
{
	// Configure auto-refresh.  Freq0 has the auto-refresh enable, so do it last.
	
	rAMC_AREFEN_FREQ(3) = 0x00000000;
	rAMC_AREFEN_FREQ(2) = 0x00000000;
	rAMC_AREFEN_FREQ(1) = 0x00000000;
	
	rAMC_AREFEN_FREQ(0) = 0x01011111;
}

int32_t amc_init(bool resume)
{
	uint8_t config_id = 0;
	uint32_t tREFi;
	uint64_t deadline;
	uint32_t ch, r, i;

#if AMC_PARAMS_AP_DEV
	if (target_config_dev()) {
		memcpy(&amc_params, &amc_params_dev, sizeof(amc_params));
	} else {
		memcpy(&amc_params, &amc_params_ap, sizeof(amc_params));
	}
#endif
	amc_params_initialized = true;
	
	// Make sure we're at full memory frequency
	clocks_set_performance(kPerformanceMemoryFull);
	
	// The clock_reset_device() call takes care of the requirements in <rdar://problem/7269959>
	clock_reset_device(CLK_MCU);
	
	// Keep track of 50us after MCU reset to ensure resume timing is in spec
	deadline = timer_get_ticks() + timer_usecs_to_ticks(50);
	
	//////////////////////////////////////////////////////////
	//
	// 1. AMC initial configuration
	//
	//////////////////////////////////////////////////////////
	
	rAMC_PHYUPDATETIMERS = 0x50;
	amc_phy_preinit();
	
	rAMC_LAT = amc_params.lat;
	rAMC_PHYRDWRTIM = amc_params.phyrdwrtim;
	
	tREFi = amc_params.tREFi;
	
	for (i = 0; i < AMC_FREQUENCY_SLOTS; i++) {
		rAMC_CAS_FREQ(i)         = amc_params.freq[i].cas;
		rAMC_PCH_FREQ(i)         = amc_params.freq[i].pch;
		rAMC_ACT_FREQ(i)         = amc_params.freq[i].act;
		rAMC_AUTO_FREQ(i)        = amc_params.freq[i].autoref | ((i == 0) ? tREFi : 0);
		rAMC_SELF_FREQ(i)        = amc_params.freq[i].selfref;
		rAMC_MODE_FREQ(i)        = amc_params.freq[i].modereg;
	}
	
	rAMC_PDN               = amc_params.pdn;
	rAMC_DERATE            = amc_params.derate;
	rAMC_RD                = amc_params.read;
	
	rAMC_BUSTAT_FREQ01     = amc_params.bustat;
	rAMC_BUSTAT_FREQ23     = amc_params.bustat2;
	rAMC_MIFCASSCH_FREQ(0) = 0x110;
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++)
		for (r = 0; r < AMC_NUM_RANKS; r++)
			*(_amc_chregs[ch].rnkcfg + AMC_RNKCFG_OFFSET(r)) = 1;
	
	
	rAMC_PWRMNGTEN     = amc_params.pwrmngten_default;
	rAMC_SCHEN         = amc_params.schen_default;	// disable the scheduler
	
	rAMC_MCPHY_UPDTPARAM1 = amc_params.mcphyupdate1;
	rAMC_MCPHY_UPDTPARAM = amc_params.mcphyupdate;
	
	// Enable MCUs
	rAMC_AMCEN = 0;
	
	// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
	platform_memory_barrier();
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++)
		rAMC_AMCEN |= _amc_chregs[ch].mcuen;
	
	amc_configure_address_decoding_and_mapping();
		
	//////////////////////////////////////////////////////////
	//
	// 2. PHY Initial Configuartions
	//
	//////////////////////////////////////////////////////////
	amc_phy_init(resume);
	
	//////////////////////////////////////////////////////////
	//
	// 3. Self-Refresh and DRAM Reset
	//
	//////////////////////////////////////////////////////////

	// Wait 5 us after Impedence Calibration to avoid McPhyPending
	// preventing the SRFSM from exiting SR.
	spin(5);
	
	amc_phy_restore_calibration_values(resume);
	
	rAMC_AREFPARAM = amc_params.arefparam;
	
	amc_enable_slow_boot(true);
	
#if !SUPPORT_FPGA
	// Enable auto refresg derating by setting TempDrtEn
	rAMC_ODTS = 0x00010000;
#endif
	
	rAMC_LONGSR = amc_params.longsr;
	
	if (resume)  {
		amc_enable_autorefresh();
		// Software must guarantee that at least 50 us have passed since the de-assertion
		// of AMC reset before self-refresh exit, in the resume-boot case.
		while (deadline > timer_get_ticks()) ;
	}
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++)
		*_amc_chregs[ch].initcmd = 0x00001000;	// start self-refresh exit seq on ch
	
	// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
	platform_memory_barrier();
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++)
		while (*_amc_chregs[ch].initcmd != 0) ;
		
	//////////////////////////////////////////////////////////
	//
	// 4. DRAM Reset, ZQ Calibration & Configuration (cold boot only)
	//
	//////////////////////////////////////////////////////////
	if (!resume) {
		spin(200);
		
		// MR63 (DRAM reset and auto-refresh enable)
		amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x3f, 0xfc);
		spin(12);
		
		// MR10 (ZQ initial calibration)
		amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0xa, 0xff);
		spin(1);
		
		// MR2 register (read/write latency, assumes legal combo in amc_params)
		amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x2, amc_params.mr2);

		// MR1
		amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x1, amc_params.mr1);
		
		// MR3 register (output buffer drive strength 48-Ohm unless overridden)
		amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x3, (amc_params.mr3 ? amc_params.mr3 : 0x3));
	}
	
	//////////////////////////////////////////////////////////
	//
	// 5. Topology-specific AMC re-configuration
	//
	//////////////////////////////////////////////////////////

	if (resume) {
		rAMC_ZQC = 0x00090000;
		amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0xa, 0xab);
		spin(1);
		rAMC_ZQC = 0x00080000;
	}
	
	rAMC_ADDRCFG = amc_params.addrcfg;
	
	// Read device info: vendor, revision, and configuration info
	// Configuration info: device width, device type, device density
	// We are assuming all of our devices are identical
	amc_mrcmd(MR_READ, 1, 1, 0x5, (uintptr_t)&_amc_device_info.vendor_id);
	amc_mrcmd(MR_READ, 1, 1, 0x6, (uintptr_t)&_amc_device_info.rev_id);
	amc_mrcmd(MR_READ, 1, 1, 0x7, (uintptr_t)&_amc_device_info.rev_id2);
	amc_mrcmd(MR_READ, 1, 1, 0x8, (uintptr_t)&config_id);
	
#if SUPPORT_FPGA
	if ((config_id == 0xff) || (config_id == 0)) {
		// Fake the MR_READ results to something accurate but innocuous
		config_id = (0 << JEDEC_MR8_WIDTH_SHIFT) | (5 << JEDEC_MR8_DENSITY_SHIFT) | (0 << JEDEC_MR8_TYPE_SHIFT);
		_amc_device_info.vendor_id = JEDEC_MANUF_ID_RSVD2;
	}
#endif
	
	if ((_amc_device_info.vendor_id == 0) || (config_id == 0))
		panic("failed to read vendor-id/config-id, vid:%08x, config:%08x, rev:%08x\n", _amc_device_info.vendor_id,
		      config_id, _amc_device_info.rev_id);
	
	_amc_device_info.width = (32 >> ((config_id >> JEDEC_MR8_WIDTH_SHIFT) & JEDEC_MR8_WIDTH_MASK)) >> 3;
	_amc_device_info.density = ((config_id >> JEDEC_MR8_DENSITY_SHIFT) & JEDEC_MR8_DENSITY_MASK);
	_amc_device_info.type =  ((config_id >> JEDEC_MR8_TYPE_SHIFT) & JEDEC_MR8_TYPE_MASK);
	
	_amc_device_info_inited = true;
	
	dprintf(DEBUG_INFO, "sdram vendor id:0x%02x rev id:0x%02x rev id2:%02x\n",
		_amc_device_info.vendor_id, _amc_device_info.rev_id, _amc_device_info.rev_id2);
	dprintf(DEBUG_INFO, "sdram config: width %d/%d Mbit/type %d\n", _amc_device_info.width << 3,
		64 << _amc_device_info.density,  _amc_device_info.type);
	
	if ((_amc_device_info.density < JEDEC_DENSITY_1Gb) || (_amc_device_info.density > JEDEC_DENSITY_32Gb))
		panic("unsupported DRAM density: %dMbit", 64 << _amc_device_info.density);
	
	uint32_t device_size_Mbits = 64 << _amc_device_info.density;
	uint32_t total_size_Mbytes = (AMC_NUM_CHANNELS * AMC_NUM_RANKS * device_size_Mbits) >> 3;
	// Units of 128 MBs in the register
	rMCC_DRAMACCCTRL = (total_size_Mbytes >> 7) - 1;
	dprintf(DEBUG_INFO, "rMCC_DRAMACCCTRL: 0x%08x, memory_size: %u bytes\n", rMCC_DRAMACCCTRL, (total_size_Mbytes << 20));
	
	rMCC_CHNLDEC = amc_params.mccchnldec;
	rAMC_MCSCHNLDEC = amc_params.mcschnldec;
	rMCC_MCUCHNHASH = amc_params.mcuchnhash;
	if (amc_params.mcuchnhash2)
		rMCC_MCUCHNHASH2 = amc_params.mcuchnhash2;
    
   	// DRAM vendor-specific workarounds
	amc_dram_workarounds(resume);
    
	//////////////////////////////////////////////////////////
	//
	// 6. Switch from boot-clock speed to normal speed
	//
	//////////////////////////////////////////////////////////
	
	// Wait 5 us before freq change to make sure all refreshes have been flushed
	spin(5);
	
	rAMC_SCHEN = amc_params.schen_default | 0x1;				// enable the scheduler
	
	amc_phy_pre_normal_speed_enable();
	amc_enable_slow_boot(false);
	
	//////////////////////////////////////////////////////////
	//
	// 7. PHY DQ and address timing calibration
	//
	//////////////////////////////////////////////////////////
#if !SUPPORT_FPGA
	rAMC_READ_LEVELING = amc_params.readleveling;
	amc_phy_calibration_ca_rddq_cal(resume);
#endif // !SUPPORT_FPGA
	
	//////////////////////////////////////////////////////////
	//
	// 8. Enable other features
	//
	//////////////////////////////////////////////////////////

	rAMC_ZQC = 0x010c03ff;
	rAMC_QBREN = 0x00110001;
	
	if (!resume)
		amc_enable_autorefresh();
	
	//////////////////////////////////////////////////////////
	//
	// 9. Enable the Fast Critical Word Forwarding feature
	//
	//////////////////////////////////////////////////////////
	
	rAMC_QBRPARAM = amc_params.qbrparam;
	rAMC_QBREN = 0x00111001;
	rMCC_MCCGEN = 0x00000126;
	
	//////////////////////////////////////////////////////////
	//
	// 10. PHY write DQ calibration
	//
	//////////////////////////////////////////////////////////
#if !SUPPORT_FPGA
	amc_phy_calibration_wrdq_cal(resume);
#endif
	rAMC_PHYUPDATETIMERS = 0x00000f50;
		
	//////////////////////////////////////////////////////////
	//
	// 11. Enable Power & ClockGating features
	//
	//////////////////////////////////////////////////////////
	amc_phy_finalize();
	amc_finalize(resume);	

	//////////////////////////////////////////////////////////
	//
	// 12. ODTS read and bring memory out of self-refresh
	//
	//////////////////////////////////////////////////////////
	
	// MR4 read to bring memory out of self-refresh
	uint8_t temp;
	amc_mrcmd(MR_READ, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x4, (uintptr_t)&temp);
	
	rAMC_ODTS |= amc_params.odts;
	
	// Program tunables at the end
	for (i = 0; i < sizeof(amc_tunables) / sizeof(amc_tunables[0]); i++) {
		if (amc_tunables[i].reg == 0)
			break;
		*amc_tunables[i].reg = amc_tunables[i].value;
	}
	
	// cache memory info for later
	platform_set_memory_info(_amc_device_info.vendor_id, amc_get_memory_size());
	
	return 0;
}

void mcu_late_init(void)
{
}

uint64_t amc_get_memory_size(void)
{
	// if memory is not inited, density is unknown, spin here ...
	if (false == _amc_device_info_inited)
		for(;;) ;
	
	// device density (in MBytes) * num of channels * num of ranks
	return ((8 << _amc_device_info.density) * AMC_NUM_CHANNELS * AMC_NUM_RANKS);
}

const struct amc_memory_device_info *amc_get_memory_device_info(void)
{
	// if memory is not inited, device info is unknown, spin here ...
	if (false == _amc_device_info_inited)
		for(;;) ;
	
	return ((const struct amc_memory_device_info *)&_amc_device_info);
}

const struct amc_param *amc_get_params(void)
{
	ASSERT(amc_params_initialized);
	return &amc_params;
}

/*
 * Some routines used during dynamic calibration
 */

// Some AMC features to be changed before calibration starts, and restored after calibration is complete
void amc_calibration_start(bool start)
{
	// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
	platform_memory_barrier();
	
	if (start) {
		// Disable OdtsRdIntrvl
		rAMC_ODTS &= 0xFFFFFC00;
	} else {
		// Re-enable OdtsRdIntrvl
		rAMC_ODTS |= amc_params.odts;
	}
}

void amc_enable_rddqcal(bool enable)
{
	// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
	platform_memory_barrier();
	
	if (enable) {
		rAMC_READ_LEVELING |= 1;
	} else {
 		rAMC_READ_LEVELING &= ~1;
	}
}

void amc_wrdqcal_start(bool start)
{
	if (start) {
		
		// Enable WriteMergeEn and WqInOrderEn
		rAMC_PSQWQCTL0 = (1 << 8) | (1 << 0);
		
		// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
		platform_memory_barrier();
		
		// Set SelfRefTmrVal to max
		rAMC_PWRMNGTPARAM |= (0xFFFF << 16);
		
		// Designer suggests 0xBB, so that writes do not age out
		rAMC_PSQWQCTL1 = 0xBB;
		
	} else {
		rAMC_PSQWQCTL0 = 0;
	}
}

// returns 1 if number of 1s in input is odd, otherwise returns 0
static uint32_t amc_odd_parity(uint32_t input)
{
	uint32_t output = input;
	
	output = (output & 0x0000ffff) ^ (output >> 16);
	output = (output & 0x000000ff) ^ (output >>  8);
	output = (output & 0x0000000f) ^ (output >>  4);
	output = (output & 0x00000003) ^ (output >>  2);
	output = (output & 0x00000001) ^ (output >>  1);
	
	return output;
}

// Given ch, rnk, bank, row, and col, compute the Apple Fabric address that CPU can use to do reads/writes
// For details of addr mapping, see "Address_Unamp.xls" at
// https://seg-fijipublic.ecs.apple.com/doc/release/index.php?dir=/reference/org-seg-services-vhost-seg--fijipublic.ecs.apple.com/content/doc/release/specs/Apple_IP/AMC
uint64_t amc_get_uncached_dram_virt_addr(uint32_t ch, uint32_t rnk, uint32_t bank, uint32_t row, uint32_t col)
{
	uint64_t system_addr;
	uint32_t ch_dropped_addr, ch_inserted_val;
	uint32_t col_val;
	uint32_t bank_addr_pos, col_low_bits, col_low_mask;
	uint32_t rank_off, bank_off, row_off, col_off;
	uint32_t temp_bank_bit2, temp_bank_bit1, temp_bank_bit0;
	uint32_t rank_wid, bank_wid, row_wid, col_wid;
	uint32_t addrcfg = rAMC_ADDRCFG;
	uint32_t ch_insert_point, ch_insert_width, ch_insert_mask;
	uint32_t addrmapmode = rAMC_ADDRMAP_MODE;
	uint32_t mcsaddrbankhash0 = rAMC_MCSADDRBNKHASH(0);
	uint32_t mcsaddrbankhash1 = rAMC_MCSADDRBNKHASH(1);
	uint32_t mcsaddrbankhash2 = rAMC_MCSADDRBNKHASH(2);
	uint32_t chnldec = rMCC_CHNLDEC;
	uint32_t mcschnldec = rAMC_MCSCHNLDEC;
	
	system_addr = SDRAM_BASE_UNCACHED;
	
	/*
	 DRAM Address Mapping Mode (H7 uses RIBI1)
	 RSBS:	0	Rank Stacked, Bank Stacked. {CS, BA, RA, CA}
	 RIBI1:	1	Rank Interleaved, Bank Interleaved, Option 1. {RA, CA-high, CS, BA, CA-low(128B)}
	 RIBI2:	2	Rank Interleaved, Bank Interleaved, Option 2. {RA, CS, BA, CA}
	 RSBI1:	3	Rank Stacked, Bank Interleaved, Option 1. {CS, RA, CA-high, BA, CA-low(128B)}
	 RIBI3:	4	Rank Interleaved, Bank Interleaved, Option 3. {RA, CS, CA-high, BA, CA-low(128B)}
	 */
	
	/*
	 BnkAddrWid
	 2-bits:	0
	 3-bits:	1 (H7)
	 ColAddrWid
	 8-bits:	0
	 9-bits:	1
	 10-bits:	2 (H7)
	 11-bits:	3
	 RowAddrWid
	 12-bits:	0
	 13-bits:	1
	 14-bits:	2 (H7)
	 15-bits:	3
	 CsWid
	 0-bits:	0
	 1-bits:	1 
	 */
	
	// whether rank needs a bit depends on ADDRCFG.CSWID bit
	rank_wid = ((addrcfg >> 24) & 1);
	// rest of the bit widths also depend on ADDRCFG fields
	bank_wid = (addrcfg & 1) + 2;
	row_wid = ((addrcfg >> 16) & 0x3) + 12;
	col_wid = ((addrcfg >> 8) & 0x3) + 8;
	
	// column bits always start at bit 2 (each [row, col] specifies 4 bytes)
	col_off = 2;
	
	bank_addr_pos = 6 + ((addrmapmode & 0x700) >> 8);
	
	switch(addrmapmode & 1)
	{
		case 0:
			col_val = col;
			row_off = col_off + col_wid;
			bank_off = row_off + row_wid;
			rank_off = bank_off + bank_wid;
			break;
			
		default:
		case 1:
			// RIBI1 (H7)
			if (bank_addr_pos == (col_wid + col_off)) {
				col_val = col;
				bank_off = col_off + col_wid;
				rank_off = bank_off + bank_wid;
				row_off = rank_off + rank_wid;
			} else {
				// bank bits interleaved with column bits (not POR on H7)
				col_low_bits = bank_addr_pos - col_off;
				col_low_mask = (1 << col_low_bits) - 1;
				
				col_val = ((col & ~col_low_mask) << bank_wid) | (col & col_low_mask);
				bank_off = bank_addr_pos;
				rank_off = col_off + col_wid + bank_wid;
				row_off = rank_off + rank_wid;
			}
			break;
	}
	
	// bank hashing
	//
	temp_bank_bit2 = amc_odd_parity(row & ~mcsaddrbankhash2);
	temp_bank_bit1 = amc_odd_parity(row & ~mcsaddrbankhash1);
	temp_bank_bit0 = amc_odd_parity(row & ~mcsaddrbankhash0);
	bank = bank ^ ((temp_bank_bit2 << 2) | (temp_bank_bit1 << 1) | temp_bank_bit0);
	
	// our address so far - only the channel num stuff is missing now
	//
	ch_dropped_addr = (rnk << rank_off) | (bank << bank_off) | (row << row_off) | (col_val << col_off);
	
	/*
	 ChSelTyp
	 Interleaving: 0 (H7)
	 Stacked:      1
	 */
	if (chnldec & 1) {
		// in stacked mode, ch will be inserted at bit 29 for H7
		ch_inserted_val = ch_dropped_addr | (ch << (((chnldec >> 16) & 0xF)  + 24));
	}
	else {   // interleaving
		
		// for H7, ch is inserted for interleaving at bit specified by rAMC_MCSCHNLDEC.ChnlStartBit
		ch_insert_point = 6 + ((mcschnldec & 0x300) >> 8);
		
		// ch insertion width (0, 1, 2, or 3 bits) depends on how many channels are enabled
		ch_insert_width = (mcschnldec & 0x30000) >> 16;
		
		// now fix the address so we can reconstruct the XOR
		ch_insert_mask = (1 << ch_insert_point) - 1;
		ch_dropped_addr = ((ch_dropped_addr & ~ch_insert_mask) << ch_insert_width) | (ch_dropped_addr & ch_insert_mask);
				
		if (ch_insert_width <= 1) {
			// calculate the XOR value - XOR all the bits (only the bits that can participate -- determined via rMCC_MCUCHNHASH)
			ch_inserted_val = ch | (ch_dropped_addr >> ch_insert_point);
			// now shift up the ch_inserted_val to align with rMCC_MCUCHNHASH, whose bit 0 maps to PA[6] of AF address
			ch_inserted_val = amc_odd_parity((ch_inserted_val << (ch_insert_point - 6)) & rMCC_MCUCHNHASH);
		} else {
			// This code only handles 2 bits for channel number
			if (ch_insert_width > 2)
				panic("Unable to handle addressing for more than 4 channels");
			
			// handle 2 bit channel value differently (for Capri)
			uint32_t chnhash_addr = (ch << (ch_insert_point - 6) | (ch_dropped_addr >> 6));
			ch_inserted_val = amc_odd_parity(chnhash_addr & rMCC_MCUCHNHASH);
			
			ch_inserted_val |= (amc_odd_parity(chnhash_addr & rMCC_MCUCHNHASH2)) << 1;
		}
		
		// finally, insert the XOR bits
		ch_insert_mask = (1 << ch_insert_width) - 1;  // reuse this var in a pseudo-related way
		ch_inserted_val = ch_dropped_addr | ((ch_inserted_val & ch_insert_mask) << ch_insert_point);
	}
	
	system_addr += (uint64_t) ch_inserted_val;
	return system_addr;
}

// Returns number of consecutive bytes given channel and rank before channel interleaving
uint32_t amc_get_consecutive_bytes_perchnrnk(void)
{
	// FIXME: DV says assume this is 0 for now (resulting in 64 consecutive bytes)
	// uint32_t ch_start_bit = (rAMC_MCSCHNLDEC & 0x300) >> 8;
	uint32_t ch_start_bit = 0;
	
	// given ChnlStartBit, consecutive bytes is simply 1 << (6 + ChnlStartBit)
	return (1 << (6 + ch_start_bit));
}
