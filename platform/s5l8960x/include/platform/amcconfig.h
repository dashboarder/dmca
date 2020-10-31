/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
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
	.flags     	 	= (FLAG_AMC_PARAM_BUSTAT23 | FLAG_AMC_PARAM_SLOW_BOOT | FLAG_AMC_PARAM_ZQCL),

	.tREFi      		= 0x5d,
	.longsrcnt  		= 0x2008,
	.srextrarefcnt		= 0x00020000,
	.rdlat      		= 6,
	.wrlat      		= 3,
	.phyrdlat   		= 10,
	.phywrlat   		= 2,
	.pdn        		= 0x22000403,
	.read       		= 0x00000203,
	.bustat     		= 0x00222209,
	.bustat2    		= 0x00222209,
	.derate     		= 0x2483135b,
	.mcphyupdate		= 0x05030000,
	.autoref_params		= 0x00000021,
	.pwrmngtparam_small	= 0x00011000,
	.pwrmngtparam_guided	= 0x01801000,
	.chnldec		= 0x00050000,
	.aref_freq0		= 0x00000110,
	.schen_default		= 0x00000110,
	.mr1			= 0x44,
	.mr2			= 0x1a,
	.mr3			= 0x02,
	.pwrmngten_default	= 0x00100100,
	.mcphyupdate1		= 0x00000001,
	.odts			= 0x190,
	.readleveling		= 0x00000300,
	.offset_shift_hynix	= -2,
	.offset_shift_elpida	= -1,
	
	.freq     		= {
		{	// 800MHz
			.cas         = 0x00000308,
			.pch         = 0x00030612,
			.act         = 0x12040908,
			.autoref     = 0x19350000,
			.selfref     = 0x00039060,
			.modereg     = 0x20250782,
			.mifcassch   = 0x00000110,
			.arefparam   = 0x0c022131,
		},
		{	// 400MHz
			.cas         = 0x00000005,
			.pch         = 0x00000009,
			.act         = 0x09020505,
			.autoref     = 0x0d1b0000,
			.selfref     = 0x0001d000,
			.modereg     = 0x00130000,
			.mifcassch   = 0x00000000,
			.arefparam   = 0x00000101,
		},
		{	// 200MHz
			.cas         = 0x00000003,
			.pch         = 0x00000005,
			.act         = 0x08020303,
			.autoref     = 0x070e0000,
			.selfref     = 0x0000f000,
			.modereg     = 0x000a0000,
			.mifcassch   = 0x00000000,
			.arefparam   = 0x00000101,
		},
		{	// 50MHz
			.cas         = 0x00000003,
			.pch         = 0x00000003,
			.act         = 0x08020303,
			.autoref     = 0x02040000,
			.selfref     = 0x00004000,
			.modereg     = 0x00030000,
			.mifcassch   = 0x00000000,
			.arefparam   = 0x00000101,
		}
	}
};

static const struct amc_tunable amc_tunables[] = {
	{ &rAMC_MCSARBCFG,		0x00000001 },
	{ &rAMC_OPIDLETMR,		0x00200000 },
	{ &rAMC_OPTMRADJPARAM,		0x20400064 },
	{ &rAMC_MIFACTSCH,		0x00000001 },
	{ &rAMC_MIFCASSCH_FREQ(0),	0x10000010 },
	{ &rAMC_PSQRQCTL0,		0xff010110 },
	{ &rAMC_PSQRQCTL1,		0x00001728 },
	{ &rAMC_PSQRQTIMER0,		0x00000040 },
	{ &rAMC_PSQRQTIMER1,		0x00000014 },
	{ &rAMC_PSQRQTIMER2,		0x00000040 },
	{ &rAMC_PSQRQBRST,		0x04100410 },
	{ &rAMC_PSQRQSCHCRD,		0x00101400 },
	{ &rAMC_PSQWQCTL0,		0x00010100 },
	{ &rAMC_PSQWQBRST,		0x04100410 },
	{ &rAMC_PSQWQSCHCRD,		0x00101400 },
	{ &rAMC_MCUQOS,			0x00001919 },
	{ &rAMC_MCUQOSLLT,		0x0000011f },
};
#endif
