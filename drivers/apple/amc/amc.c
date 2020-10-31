/*
 * Copyright (C) 2009-2014 Apple Inc. All rights reserved.
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

// Local static functions
static uint32_t amc_odd_parity(uint32_t input);

/*
 * Based on H4P/H4G AMC UM 0.63, H4 Tunables 0.36,
 *          H5P/Bali AMC UM 0.48, H5P/Bali Tunables 0.63
 *          Alcatraz AMC Init.html (A0: AMC: 3.53.5 AMP: 2.13.0 (v22). B0: 3.56.4 AMP: 2.13.1 (v76))
 *          Alcatraz A0 AMC Tunables dated Dec 8, 2012
 *          M7 AMC Init.html (AMC: 12.3.0 AMP: 5.7.0, dated Wed Mar 11 17:47:57 2014, Tunables as of Apr 9, 2014)
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
	
	*_amc_chregs[channel].mrcmd = cmd | (rank << 16);
	
// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
#if AMC_REG_VERSION > 2
	platform_memory_barrier();
#endif
	
	while(((regval = *_amc_chregs[channel].mrstatus) & _amc_chregs[channel].mrpoll) != 0) ;
	if (op == MR_READ)
		*buffer++ = (regval >> _amc_chregs[channel].mrshift) & 0xff;
}

void amc_enable_autorefresh(void)
{
	// Configure auto-refresh.  Freq0 has the auto-refresh enable, so do it last.
	
	rAMC_AREFEN_FREQ(1) = 0x00000000 | amc_params.aref_freq1;
	
	rAMC_AREFEN_FREQ(2) = 0x00000000;
	rAMC_AREFEN_FREQ(3) = 0x00000000;

	rAMC_AREFEN_FREQ(0) = 0x01011001 | amc_params.aref_freq0;
}

void amc_configure_default_address_decoding_and_mapping(void)
{
	rAMC_ADDRCFG      = 0x00020101;			// Default: 2Gb geometry: 9col, 14row, 8bank
	if (AMC_NUM_RANKS > 1)
		rAMC_ADDRCFG |= 0x01000000;		// 2rank
	rAMC_CHNLDEC  = 0x00040000 | amc_params.chnldec; // 2Gb/channel, interleaved
#if AMC_REG_VERSION < 3 && !SUB_PLATFORM_S7002
	if (amc_params.chnldec2)
		rAMC_CHNLDEC2 = amc_params.chnldec2;
	if (amc_params.chnldec4)
		rAMC_CHNLDEC4 = amc_params.chnldec4;
#endif

	rAMC_ADDRMAP_MODE = 2;				// RIBI2

#if AMC_REG_VERSION < 3 && !SUB_PLATFORM_S7002
	if (amc_params.flags & FLAG_AMC_PARAM_MRR_BIT_SWIZZLE)
		rAMC_PHYMRRDATADQBITMUX = 0x56714230;

	if (amc_params.flags & FLAG_AMC_PARAM_MRR_BYTE_SWIZZLE)
		rAMC_PHYMRRDATADQBYTEMUX = 0x2;
#endif
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

#if (AMC_REG_VERSION == 2 && SUB_PLATFORM_S7002)
	clocks_set_performance(kPerformanceMemoryLow);
#else
	// Make sure we're at full memory frequency
	clocks_set_performance(kPerformanceMemoryFull);
#endif

	// The clock_reset_device() call takes care of the requirements in <rdar://problem/7269959>
	clock_reset_device(CLK_MCU);

	// Keep track of 50us after MCU reset to ensure resume timing is in spec
	deadline = timer_get_ticks() + timer_usecs_to_ticks(50);

	//////////////////////////////////////////////////////////
	//
	// 1. AMC initial configuration
	//
	//////////////////////////////////////////////////////////

	amc_phy_enable_dqs_pulldown(false);

	tREFi = amc_params.tREFi;

	for (i = 0; i < AMC_FREQUENCY_SLOTS; i++) {
		rAMC_CAS_FREQ(i)         = amc_params.freq[i].cas;
		rAMC_PCH_FREQ(i)         = amc_params.freq[i].pch;
		rAMC_ACT_FREQ(i)         = amc_params.freq[i].act;
		rAMC_AUTO_FREQ(i)        = amc_params.freq[i].autoref | ((i == 0) ? tREFi : 0);
		rAMC_SELF_FREQ(i)        = amc_params.freq[i].selfref;
		rAMC_MODE_FREQ(i)        = amc_params.freq[i].modereg;
		rAMC_MIFCASSCH_FREQ(i)   = amc_params.freq[i].mifcassch;
	}

#if (AMC_REG_VERSION == 2 && SUB_PLATFORM_S7002)
	rAMC_TREFBWBASECYC_FREQ(0)	= amc_params.freq[0].trefbwbasecyc;
	rAMC_TREFBWBASECYC_FREQ(1)	= amc_params.freq[1].trefbwbasecyc;
	rAMC_TREFBWBASECYC_FREQ(2)	= amc_params.freq[2].trefbwbasecyc;
#endif		
	
	rAMC_LAT               = (amc_params.rdlat << 16) | (amc_params.wrlat << 8);
	rAMC_PDN               = amc_params.pdn;
	rAMC_DERATE            = amc_params.derate;
	rAMC_RD                = amc_params.read;

	if (amc_params.flags & FLAG_AMC_PARAM_BUSTAT23) {
		rAMC_BUSTAT_FREQ01     = amc_params.bustat;
		rAMC_BUSTAT_FREQ23     = amc_params.bustat2;
	} else {
		rAMC_BUSTAT            = amc_params.bustat;
	}

	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++)
		for (r = 0; r < AMC_NUM_RANKS; r++)
			*(_amc_chregs[ch].rnkcfg + AMC_RNKCFG_OFFSET(r)) = 1;

	rAMC_PHYRDWRTIM = (amc_params.phywrlat << 16) | (amc_params.phyrdlat << 8) | (amc_params.rdlat - 2);

	rAMC_PWRMNGTEN     = amc_params.pwrmngten_default;
	rAMC_SCHEN         = amc_params.schen_default;	// disable the scheduler

	rAMC_MCPHY_UPDTPARAM = amc_params.mcphyupdate;
#if (AMC_REG_VERSION == 3) || (AMC_REG_VERSION == 2 && SUB_PLATFORM_S7002)
	rAMC_MCPHY_UPDTPARAM1 = amc_params.mcphyupdate1;
#endif

	if (amc_params.autoref_params)
		rAMC_AUTOREF_PARAMS = amc_params.autoref_params;

	amc_configure_address_decoding_and_mapping();

	// Enable MCUs
	rAMC_AMCEN = (amc_params.flags & FLAG_AMC_PARAM_LEGACY) ? 0x80000000 : 0;
	
// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
#if AMC_REG_VERSION > 2
	platform_memory_barrier();
#endif
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++)
		rAMC_AMCEN |= _amc_chregs[ch].mcuen;
	// Enable AIU, if supported
	if (amc_params.flags & FLAG_AMC_PARAM_ENABLE_AIU)
		rAMC_AMCEN |= 0x10000;

	//////////////////////////////////////////////////////////
	//
	// 2. PHY Initial Configuartions
	//
	//////////////////////////////////////////////////////////
#ifdef AMP_SWIZZLE
#if (AMP_SWIZZLE == AMP_SWIZZLE_PER_J34M)	// per <rdar://15498696>
	// Set up the MRR Bit Mapping Muxen
	rAMC_PHYMRRDATADQBYTEMUX = 0x00000002;
	rAMC_PHYMRRDATADQBITMUX  = 0x76543210;
#endif
#endif
	amc_phy_init(resume);

	//////////////////////////////////////////////////////////
	//
	// 3. Self-Refresh and DRAM Reset
	//
	//////////////////////////////////////////////////////////
#if (AMC_REG_VERSION == 3) || ((AMC_REG_VERSION == 2) && SUB_PLATFORM_S7002)
	rAMC_AREFPARAM = amc_params.freq[0].arefparam;
	
	for (i = 1; i < AMC_FREQUENCY_SLOTS; i++) {
		rAMC_AREFPARAM_FREQ(i) = amc_params.freq[i].arefparam;
	}
#else
	rAMC_AREFPARAM    = 0x00060532 | amc_params.arefparam;
#endif
	
#if !((AMC_REG_VERSION == 2) && SUB_PLATFORM_S7002)
	amc_enable_slow_boot(true);
#endif

	// <rdar://problem/8082114> <rdar://problem/8222443>
	// <rdar://problem/8080928> <rdar://problem/8711948>
	rAMC_LONGSR       = 0x00000000;
	
#if (AMC_REG_VERSION == 3) || ((AMC_REG_VERSION == 2) && SUB_PLATFORM_S7002)
	rAMC_LONGSR    = 0x01000000 | amc_params.longsrcnt | amc_params.srextrarefcnt;
#endif

	if (resume)  {
		// Enable temperature based refresh rate modulation to compensate for missing refresh
		if (amc_params.odts) {
		#if SUPPORT_FPGA
			rAMC_ODTS	= amc_params.odts;
		#else
			rAMC_ODTS	= 0x00010000 | amc_params.odts;
		#endif
		}
        
		amc_enable_autorefresh();
		// Software must guarantee that at least 50 us have passed since the de-assertion
		// of AMC reset before self-refresh exit, in the resume-boot case.
		while (deadline > timer_get_ticks()) ;
	}
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++)
		*_amc_chregs[ch].initcmd = 0x00001000;	// start self-refresh exit seq on ch
	
// <rdar://problem/16239984> Alcatraz, Fiji, Capri: L2_TB: MCU PIO accesses to the same address and device stream getting reordered in CP
#if AMC_REG_VERSION > 2
	platform_memory_barrier();
#endif
	
	for (ch = 0; ch < AMC_NUM_CHANNELS; ch++)
		while (*_amc_chregs[ch].initcmd != 0) ;

#if AMC_REG_VERSION < 3 && !SUB_PLATFORM_S7002
	rAMC_AREFPARAM = 0x00020532 | amc_params.arefparam;
	rAMC_LONGSR    = 0x05000000 | amc_params.longsrcnt | amc_params.srextrarefcnt;
#endif

	//////////////////////////////////////////////////////////
	//
	// 4. DRAM Reset, ZQ Calibration & Configuration (cold boot only)
	//
	//////////////////////////////////////////////////////////
	if (!resume) {
		spin(200);

		// MR63 (DRAM reset and auto-refresh enable)
		amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x3f, amc_params.mr63);
		spin(12);

		// MR10 (ZQ initial calibration)
		amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0xa, 0xff);
		spin(1);

		// MR2 register (read/write latency, assumes legal combo in amc_params)
		amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x2, (amc_params.mr2 ? amc_params.mr2 : (amc_params.rdlat - 2)));
	}

	// Read device info: vendor, revision, and configuration info
	// Configuration info: device width, device type, device density
	// We are assuming all of our devices are identical
	amc_mrcmd(MR_READ, 1, 1, 0x5, (uintptr_t)&_amc_device_info.vendor_id);
	amc_mrcmd(MR_READ, 1, 1, 0x6, (uintptr_t)&_amc_device_info.rev_id);
	amc_mrcmd(MR_READ, 1, 1, 0x7, (uintptr_t)&_amc_device_info.rev_id2);
	amc_mrcmd(MR_READ, 1, 1, 0x8, (uintptr_t)&config_id);

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
		
	if ((_amc_device_info.density < JEDEC_DENSITY_1Gb) || (_amc_device_info.density > JEDEC_DENSITY_4Gb))
		panic("unsupported DRAM density: %dMbit", 64 << _amc_device_info.density);
	
	if (!resume) {
		// MR1 (nWR=pch_freq0.tWRCyc), WC=wrap, BT=Seq, BL=16)
		amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x1,
			  (amc_params.mr1 ? amc_params.mr1 :
						(((amc_params.freq[0].pch - 0x200) & 0xf00) >> 3) | 0x04));

		// MR3 register (output buffer drive strength 48-Ohm unless overridden)
		amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x3, (amc_params.mr3 ? amc_params.mr3 : 0x3));
	}

	// DRAM vendor-specific workarounds
	amc_dram_workarounds(resume);

	//////////////////////////////////////////////////////////
	//
	// 5. Topology-specific AMC re-configuration
	//
	//////////////////////////////////////////////////////////
	if (amc_params.flags & FLAG_AMC_PARAM_RECONFIG) {
		uint8_t num_banks;
		
		num_banks = 8;
		if (((_amc_device_info.type == JEDEC_TYPE_S4_SDRAM) && (_amc_device_info.density <= JEDEC_DENSITY_512Mb)) ||
		((_amc_device_info.type == JEDEC_TYPE_S2_SDRAM) && (_amc_device_info.density <= JEDEC_DENSITY_2Gb)))
			num_banks = 4;
		
		if (_amc_device_info.density == JEDEC_DENSITY_1Gb) {
			if (_amc_device_info.type == JEDEC_TYPE_S4_SDRAM)
				rAMC_AUTO_FREQ(0) = amc_params.freq[0].autoref | amc_params.tREFi_1Gb;

			rAMC_LONGSR = 0x05010000 | amc_params.longsrcnt_1Gb | amc_params.srextrarefcnt;

			if ((AMC_NUM_RANKS == 1) && (num_banks == 8)) {
				rAMC_CHNLDEC &= ~(7 << 16);
				rAMC_CHNLDEC |= 0x00030000;			// 1Gb/channel, interleaved
				rAMC_ADDRCFG = 0x00010101;
			}
		}
		else if (_amc_device_info.density == JEDEC_DENSITY_4Gb) {
			if ((AMC_NUM_RANKS == 1) && (num_banks == 8)) {
				rAMC_CHNLDEC &= ~(7 << 16);
				rAMC_CHNLDEC |= 0x00050000;			// 4Gb/channel, interleaved
				rAMC_ADDRCFG = 0x00020201;
			}
		}
		
		if (AMC_NUM_CHANNELS == 1) {
			switch(_amc_device_info.density) {
				case JEDEC_DENSITY_2Gb:
					rAMC_CHNLDEC |= 0x00000001;		// 2Gb/channel, stacked
					break;
					
				case JEDEC_DENSITY_4Gb:
					rAMC_CHNLDEC &= ~(7 << 16);
					rAMC_CHNLDEC |= 0x00050001;		// 4Gb/channel, stacked
					break;
				
				case JEDEC_DENSITY_1Gb:
					rAMC_CHNLDEC &= ~(7 << 16);
					rAMC_CHNLDEC |= 0x00030001;		// 1Gb/channel, stacked
					break;
				
			}
		}
	}
	
// For Alcatraz and later, we can configure the total memory size register dynamically
#if AMC_REG_VERSION > 2
	uint32_t device_size_Mbits = 64 << _amc_device_info.density;
	uint32_t total_size_Mbytes = (AMC_NUM_CHANNELS * AMC_NUM_RANKS * device_size_Mbits) >> 3;
	// Units of 128 MBs in the register
	rMCC_DRAMACCCTRL = (total_size_Mbytes >> 7) - 1;
	dprintf(DEBUG_INFO, "rMCC_DRAMACCCTRL: 0x%08x, memory_size: %u bytes\n", rMCC_DRAMACCCTRL, (total_size_Mbytes << 20));
#endif

	if (amc_params.flags & FLAG_AMC_PARAM_ZQCL) {
		if (resume) {
			uint8_t temp;

			rAMC_ZQC = 0x00090000;
			amc_mrcmd(MR_WRITE, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0xa, 0xab);
			spin(1);
			rAMC_ZQC = 0x00080000;

			// MR4 read to bring memory out of self-refresh
			amc_mrcmd(MR_READ, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x4, (uintptr_t)&temp);
		}
		
		// Wait 5 us before freq change to make sure all refreshes have been flushed
		spin(5);
	}

	//////////////////////////////////////////////////////////
	//
	// 6. Switch from boot-clock speed to normal speed
	//
	//////////////////////////////////////////////////////////
#if !SUPPORT_FPGA
	// Only enable AutoSR on pre-H6 devices
#if AMC_REG_VERSION < 3 && !SUB_PLATFORM_S7002
	rAMC_PWRMNGTEN |= 0x00000010;
#endif
#endif // !SUPPORT_FPGA

	rAMC_SCHEN = amc_params.schen_default | 0x1;				// enable the scheduler

	amc_enable_slow_boot(false);

	amc_phy_enable_dqs_pulldown(true);
	
	//////////////////////////////////////////////////////////
	//
	// 7. PHY DQ and address timing calibration
	//
	//////////////////////////////////////////////////////////
#if !SUPPORT_FPGA
	rAMC_READ_LEVELING = amc_params.readleveling;
#endif // !SUPPORT_FPGA
	amc_phy_calibration_ca_rddq_cal(resume);

	//////////////////////////////////////////////////////////
	//
	// 8. Enable power saving and other features, and turn on AMC scheduler
	//
	//////////////////////////////////////////////////////////
	amc_finalize(resume);

	amc_phy_finalize();

#if AMC_REG_VERSION < 3 && !SUB_PLATFORM_S7002
	// Program tunables
	for (i = 0; i < sizeof(amc_tunables) / sizeof(amc_tunables[0]); i++) {
		if (amc_tunables[i].reg == 0)
			break;
		*amc_tunables[i].reg = amc_tunables[i].value;
	}
#endif

	//////////////////////////////////////////////////////////
	//
	// 9. PHY write DQ calibration
	//
	//////////////////////////////////////////////////////////
	amc_phy_calibration_wrdq_cal(resume);

#if (AMC_REG_VERSION == 3) || ((AMC_REG_VERSION == 2) && SUB_PLATFORM_S7002)
	
	// <rdar://problem/13752969> requires moving MR4 from step 8 to after wrdqcal
	if (!resume) {
		// MR4 to bring memory out of self-refresh
		uint8_t odts;
		amc_mrcmd(MR_READ, AMC_NUM_CHANNELS, AMC_NUM_RANKS, 0x4, (uintptr_t)&odts);
	}
	
	// Program tunables after wrdqcal for H6
	for (i = 0; i < sizeof(amc_tunables) / sizeof(amc_tunables[0]); i++) {
		if (amc_tunables[i].reg == 0)
			break;
		*amc_tunables[i].reg = amc_tunables[i].value;
	}
#endif
	
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

void amc_scheduler_en_workaround(bool init)
{
	if (init) {
		uint8_t temp[AMC_NUM_CHANNELS];
		
		rAMC_PWRMNGTEN = 0x00000000;
		amc_mrcmd(MR_READ, AMC_NUM_CHANNELS, 1, 0x4, (uintptr_t)temp);
	} else {
		rAMC_PWRMNGTEN = 0x00000010;
	}
}

// H6 and M7 specific routine used during dynamic calibration
#if (AMC_REG_VERSION == 3) || ((AMC_REG_VERSION == 2) && SUB_PLATFORM_S7002)

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
addr_t amc_get_uncached_dram_virt_addr(uint32_t ch, uint32_t rnk, uint32_t bank, uint32_t row, uint32_t col)
{
	addr_t system_addr;
	uint32_t ch_dropped_addr, ch_inserted_val;
	uint32_t col_val;
	uint32_t rank_off, bank_off, row_off, col_off;
	uint32_t temp_bank_bit2, temp_bank_bit1, temp_bank_bit0;
	uint32_t rank_wid, bank_wid, row_wid, col_wid;
	uint32_t addrcfg = rAMC_ADDRCFG;
	uint32_t ch_insert_point, ch_insert_width, ch_insert_mask;
	uint32_t addrmapmode = rAMC_ADDRMAP_MODE;
#if AMC_REG_VERSION < 3	
	uint32_t addrbankhash0 = rAMC_AIU_ADDRBANKHASH0;
	uint32_t addrbankhash1 = rAMC_AIU_ADDRBANKHASH1;
	uint32_t addrbankhash2 = rAMC_AIU_ADDRBANKHASH2;
#else
	uint32_t addrbankhash0 = rAMC_MCSADDRBNKHASH(0);
	uint32_t addrbankhash1 = rAMC_MCSADDRBNKHASH(1);
	uint32_t addrbankhash2 = rAMC_MCSADDRBNKHASH(2);
#endif	
	uint32_t chnldec = rAMC_CHNLDEC;
	uint32_t amcen = rAMC_AMCEN;
#if AMC_REG_VERSION < 3
	uint32_t temp_ch_bit0, temp_ch_bit1;
	bool h4p_legacy_mode = amcen & 0x80000000;
#endif

	system_addr = SDRAM_BASE_UNCACHED;

	/*
	 DRAM Address Mapping Mode (H6 and M7 uses RIBI2)
	 RSBS:	0	Rank Stacked, Bank Stacked. {CS, BA, RA, CA}
	 RIBI1:	1	Rank Interleaved, Bank Interleaved, Option 1. {RA, CA-high, CS, BA, CA-low(128B)}
	 RIBI2:	2	Rank Interleaved, Bank Interleaved, Option 2. {RA, CS, BA, CA}
	 RSBI1:	3	Rank Stacked, Bank Interleaved, Option 1. {CS, RA, CA-high, BA, CA-low(128B)}
	 RIBI3:	4	Rank Interleaved, Bank Interleaved, Option 3. {RA, CS, CA-high, BA, CA-low(128B)}
	 */
	
	/*
	 BnkAddrWid
	 2-bits:	0
	 3-bits:	1 (H6)
	 ColAddrWid
	 8-bits:	0
	 9-bits:	1
	 10-bits:	2 (H6)
	 11-bits:	3
	 RowAddrWid
	 12-bits:	0
	 13-bits:	1
	 14-bits:	2 (H6)
	 15-bits:	3
	 CsWid
	 0-bits:	0 (N51)
	 1-bits:	1 (J71)
	 */
	
	// whether rank needs a bit depends on ADDRCFG.CSWID bit
	rank_wid = ((addrcfg >> 24) & 1);
	// rest of the bit widths also depend on ADDRCFG fields
	bank_wid = (addrcfg & 1) + 2;
	row_wid = ((addrcfg >> 16) & 0x3) + 12;
	col_wid = ((addrcfg >> 8) & 0x3) + 8;
	
	// column bits always start at bit 2 (each [row, col] specifies 4 bytes)
	col_off = 2;
	
	// these are used in addressing modes 1, 3, and 4
	uint32_t col_low_bits = 5;  // 128B - 2
	uint32_t col_low_mask = (1 << col_low_bits) - 1;
	
	switch(addrmapmode)
	{
		case 0:
			col_val = col;
			row_off = col_off + col_wid;
			bank_off = row_off + row_wid;
			rank_off = bank_off + bank_wid;
			break;
		case 1:
			col_val = ((col & ~col_low_mask) << (rank_wid + bank_wid)) | (col & col_low_mask);
			bank_off = col_off + col_low_bits;
			rank_off = bank_off + bank_wid;
			row_off = col_off + col_wid + bank_wid + rank_wid; // slightly different due to col split
			break;
		default:
		case 2:
			// RIBI2 (H6)
			col_val = col;
			bank_off = col_off + col_wid;
			rank_off = bank_off + bank_wid;
			row_off = rank_off + rank_wid;
			break;
		case 3:
			col_val = ((col & ~col_low_mask) << bank_wid) | (col & col_low_mask);
			bank_off = col_off + col_low_bits;
			row_off = col_off + col_wid + bank_wid; // slightly different due to col split
			rank_off = row_off + row_wid;
			break;
		case 4:
			col_val = ((col & ~col_low_mask) << bank_wid) | (col & col_low_mask);
			bank_off = col_off + col_low_bits;
			rank_off = col_off + col_wid + bank_wid; // slightly different due to col split
			row_off = rank_off + rank_wid;
			break;
	}
	
	// bank hashing
	//
	temp_bank_bit2 = amc_odd_parity(row & ~addrbankhash2);
	temp_bank_bit1 = amc_odd_parity(row & ~addrbankhash1);
	temp_bank_bit0 = amc_odd_parity(row & ~addrbankhash0);
	bank = bank ^ ((temp_bank_bit2 << 2) | (temp_bank_bit1 << 1) | temp_bank_bit0);
	
	// our address so far - only the channel num stuff is missing now
	//
	ch_dropped_addr = (rnk << rank_off) | (bank << bank_off) | (row << row_off) | (col_val << col_off);
	
	/*
	 ChSelTyp
	 Interleaving: 0 (H6)
	 Stacked:      1
	 */
	if (chnldec & 1) {
		// in stacked mode, ch will be inserted at bit 29 for H6
		ch_inserted_val = ch_dropped_addr | (ch << (((chnldec >> 16) & 0x7)  + 24));
	}
	else {   // interleaving

#if AMC_REG_VERSION < 3
		if (h4p_legacy_mode) {
			switch((chnldec >> 8) & 0xf) {
				case (1 << 0):
					ch_insert_point = 6;
					break;

				case (1 << 1):
					ch_insert_point = 8;
					break;

				case (1 << 2):
					ch_insert_point = 10;
					break;

				case (1 << 3):
					ch_insert_point = 12;
					break;

				default:
					panic("AMC CHNLDEC is not programmed correctly\n");
			}
		}
		else 
#endif
		{
			// ch is inserted at bit 6 for interleaving
			ch_insert_point = 6;
		}
		
		// ch insertion width ( 1 or 0 bits) depends on whether more than 1 channel is enabled
		ch_insert_width = (amcen >> 8) & 1;
		ch_insert_width += (amcen >> 12) & 1;
		
		// now fix the address so we can reconstruct the XOR
		ch_insert_mask = (1 << ch_insert_point) - 1;
		ch_dropped_addr = ((ch_dropped_addr & ~ch_insert_mask) << ch_insert_width) | (ch_dropped_addr & ch_insert_mask);
		
		// calculate the XOR value - XOR all the bits 

		ch_inserted_val = ch;
#if AMC_REG_VERSION < 3
		if (h4p_legacy_mode) {
			switch((chnldec >> 8) & 0xf) {
				case (1 << 0):
					ch_inserted_val ^= (ch_dropped_addr >> 6);
					break;

				case (1 << 1):
					ch_inserted_val ^= (ch_dropped_addr >> 8);
					break;

				case (1 << 2):
					ch_inserted_val ^= (ch_dropped_addr >> 10);
					break;

				case (1 << 3):
					ch_inserted_val ^= (ch_dropped_addr >> 12);
					break;
			}
		}
		else {
			temp_ch_bit0 = amc_odd_parity(ch_inserted_val & (rAMC_CHNLDEC2 & 0x3ffffff));
			temp_ch_bit1 = amc_odd_parity(ch_inserted_val & (rAMC_CHNLDEC4 & 0x3ffffff));
			ch_inserted_val = (temp_ch_bit1 << 1) | temp_ch_bit0;
		}
#else
		ch_inserted_val |= (ch_dropped_addr >> ch_insert_point);
		ch_inserted_val = amc_odd_parity(ch_inserted_val & rMCC_MCUCHNHASH);
#endif

		// finally, insert the XOR bits
		ch_insert_mask = (1 << ch_insert_width) - 1;  // reuse this var in a pseudo-related way
		ch_inserted_val = ch_dropped_addr | ((ch_inserted_val & ch_insert_mask) << ch_insert_point);
	}

	system_addr += (addr_t) ch_inserted_val;

	return system_addr;
}
#endif // AMC_REG_VERSION > 2
