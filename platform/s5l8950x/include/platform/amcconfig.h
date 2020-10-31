/*
 * Copyright (C) 2010-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#if WITH_TARGET_AMC_PARAMS
#include <target/amcconfig.h>
#else
static const struct amc_param amc_params = {
	.flags     	 	= (FLAG_AMC_PARAM_BUSTAT23 | FLAG_AMC_PARAM_SLOW_BOOT | FLAG_AMC_PARAM_RECONFIG | FLAG_AMC_PARAM_ENABLE_AIU),

	.tREFi      		= 0x5d,
	.longsrcnt  		= 0x2008,
	.tREFi_1Gb		= 0x5d,
	.longsrcnt_1Gb		= 0x2008,
	.rdlat      		= 8,
	.wrlat      		= 4,
	.phyrdlat   		= 8,
	.phywrlat   		= 4,
	.pdn        		= 0x33000503,
	.read       		= 0x00000103,
	.bustat     		= 0x00222212,
	.bustat2    		= 0x00222212,
	.derate     		= 0x30b3177b,
	.mcphyupdate		= 0x05030000,
	.autoref_params		= 0x22,		// <rdar://problem/9133484> MIF: tREFBW violation due to freq change
	.pwrmngtparam_small	= 0x00011000,
	.pwrmngtparam_guided	= 0x01801000,
	.chnldec		= 0x00000100,
	.chnldec2		= 0x003ffffd,
	.arefparam		= 0x0600a000,
	.bootclkdivsr		= 10,
	.aref_freq0		= 0x00100100,
	.aref_freq1		= 0x00100100,
	.srextrarefcnt		= 0x00010000,
	.readleveling           = 0x01000301,

	.freq     		= {
		{	// 522MHz
			.cas         = 0x0000040a,
			.pch         = 0x00040816,
			.act         = 0x1a060b0a,
			.autoref     = 0x1f439000,
			.selfref     = 0x00048080,
			.modereg     = 0x002f0502,
			.mifcassch   = 0x001f0111,
		},
		{	// 400MHz
			.cas         = 0x00000008,
			.pch         = 0x00000012,
			.act         = 0x14050a08,
			.autoref     = 0x18340000,
			.selfref     = 0x00038000,
			.modereg     = 0x00240000,
			.mifcassch   = 0x00000010,
		},
		{	// 200MHz
			.cas         = 0x00000004,
			.pch         = 0x00000009,
			.act         = 0x0a030504,
			.autoref     = 0x0c1a0000,
			.selfref     = 0x0001c000,
			.modereg     = 0x00120000,
			.mifcassch   = 0x00000000,
		},
		{	// 50 & 24MHz
			.cas         = 0x00000003,
			.pch         = 0x00000003,
			.act         = 0x08020303,
			.autoref     = 0x03070000,
			.selfref     = 0x00007000,
			.modereg     = 0x00060000,
			.mifcassch   = 0x00000010,
		}
	}
};

static const struct amc_tunable amc_tunables[] = {
	// These settings are common across all H4x and H5P.  Channel decode and hashing
	// is done in mainline init code.  Any tunables that match hardware defaults are
	// omitted here.

	{ &rAMC_AIUPRT_CFG,		0x00070000 },		// address error enable
	{ &rAMC_AIUPRT_RD_GEN,		0x00000018 },		// fence read detect on GFX
	{ &rAMC_AIUPRT_WR,		0x00000001 },		// out-of-order write, ignore B-bit

	{ &rAMC_AIU_CHNLDEC2,		0x003FFFFD },
	
	{ &rAMC_AIU_RDBRSTLEN,		0x01010101 },		// burst len is 64B
	{ &rAMC_AIU_WRBRSTLEN,		0x01010101 },		// burst len is 64B

	{ &rAMC_AIU_CPUSPCREQ,		0x00000019 },		// CPU ageout timer
	{ &rAMC_AIU_GFXSPCREQ,		0x00000032 },		// GFX ageout timer
	{ &rAMC_AIU_NRTSPCREQ,		0x00000032 },		// Hperf-NRT ageout timer
	{ &rAMC_AIU_RTSPCREQ,		0x00000019 },		// Hperf-RT ageout timer
	{ &rAMC_AIU_PUSHEN,		0x00001100 },		// only supported for Hperf-RT

	{ &rAMC_OPIDLETMR,		0x00200000 },
	{ &rAMC_OPTMRADJEN,		0x00000001 },
	{ &rAMC_OPTMRADJPARAM,		0x20400064 },

	{ &rAMC_MIFACTSCH,		0x00000001 },

	{ &rAMC_PSQRQTIMER0,		0x00000040 },		// RTG bypass up to 64 transactions
	{ &rAMC_PSQRQTIMER1,		0x00000018 },		// RTY bypass up to 24 transactions
	{ &rAMC_PSQRQTIMER2,		0x00000020 },		// RTG bypass up to 32 transactions
	{ &rAMC_PSQRQBRST,		0x040c040c },		// RTR transactions don't affect read burst size
	{ &rAMC_PSQRQSCHCRD,		0x002028f5 },		// read credits = 2 * write credits
	{ &rAMC_PSQWQBRST,		0x0208040c },		// RTR reads cause reduced write burst size
	{ &rAMC_PSQWQSCHCRD,		0x001014f5 },		// write credits = 1/2 * read credits
	{ &rAMC_MCUQOS,			0x00010f04 },		// RTG=4, RTY=15, enable pick-oldest-RTY
	{ &rAMC_PWRMNGTPARAM,		0x03001000 },		// SelfRef timer = 384 cycles

	// Platform specific
	
	{ &rAMC_AIU_RDMAXCRD,		0x3012097f },		// RT:NRT:CPU:GFX = 48:18:09:MAX
	{ &rAMC_AIU_RDREFL,		0x280f087f },		// 5/6 of AIURDMAXCRD fields
	{ &rAMC_AIU_WRMAXCRD,		0x3012097f },		// RT:NRT:CPU:GFX = 48:18:09:MAX
	{ &rAMC_AIU_WRREFL,		0x280f087f },		// 5/6 of AIUWRMAXCRD fields

	{ &rAMC_AIU_ADDRBANKHASH0,	0x00006db6 },		// Enable bank hashing, bank[0] derived from bits 0,3,6,..
	{ &rAMC_AIU_ADDRBANKHASH1,	0x00005b6d },		// Enable bank hashing, bank[1] derived from bits 1,4,7,..
	{ &rAMC_AIU_ADDRBANKHASH2,	0x000036db },		// Enable bank hashing, bank[2] derived from bits 2,5,8,..
	{ &rAMC_AIU_CHNLCRD,		0x00180018 },
	{ &rAMC_AIU_LLTSCHCTL,		0x00000001 },		// Ensure LLT traffic from all ports is treated as medium priority request in the AIU arbitration
	{ &rAMC_MCUQOSLLT,		0x00010108 },

	{ &rAMC_AIU_CHNLTM,		0x40006101 },
	{ &rAMC_AIU_WPQSCH,		0x08000800 },		// set starvation counts to 8
	{ &rAMC_PSQRQCTL1,		0x0000132a },
	{ &rAMC_PSQWQCTL0,		0x00010100 },		// enable write merging
	{ &rAMC_PSQWQCTL1,		0x00000180 },		// ages write out faster than default (1/2 SelfRef)
};
#endif
