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

/*
 * The amc_param struct is defined in drivers/apple/amc/amc.h, and this
 * file should only by included from amc.c.
 */

#define AMC_PARAMS_AP_DEV	(1)

static const struct amc_param amc_params_dev = {
	.flags     	 	= (FLAG_AMC_PARAM_BUSTAT23),

	.tREFi      		= 0x11,
	.longsrcnt  		= 0x2008,
	.srextrarefcnt		= 0x00020000,
	.rdlat      		= 4,
	.wrlat      		= 2,
	.phyrdlat   		= 10,
	.phywrlat   		= 1,
	.pdn        		= 0x22000203,
	.read       		= 0x00000101,
	.bustat     		= 0x0022221b,
	.bustat2    		= 0x0022221b,
	.derate     		= 0x04110111,
	.mcphyupdate		= 0x05030000,
	.autoref_params		= 0x00000021,
	.pwrmngtparam_small	= 0x00011000,
	.pwrmngtparam_guided	= 0x01801000,
	.chnldec		= 0x00050000,
	.arefparam		= 0x0c00a000,
	.aref_freq0		= 0x00000110,
	.schen_default		= 0x00000110,
	.mr1			= 0x44,
	.mr2			= 0x1a,
	.pwrmngten_default	= 0x00100000,
	
	.freq     		= {
		{	// 522MHz
			.cas         = 0x00000203,
			.pch         = 0x00020303,
			.act         = 0x08020303,
			.autoref     = 0x01020000,
			.selfref     = 0x00002030,
			.modereg     = 0x00010772,
			.mifcassch   = 0x001f0110,
		},
		{	// 400MHz
			.cas         = 0x00000003,
			.pch         = 0x00000003,
			.act         = 0x08020303,
			.autoref     = 0x01020000,
			.selfref     = 0x00002000,
			.modereg     = 0x00010000,
			.mifcassch   = 0x00000000,
		},
		{	// 200MHz
			.cas         = 0x00000003,
			.pch         = 0x00000003,
			.act         = 0x08020303,
			.autoref     = 0x01020000,
			.selfref     = 0x00002000,
			.modereg     = 0x00010000,
			.mifcassch   = 0x00000000,
		},
		{	// 50 & 24MHz
			.cas         = 0x00000003,
			.pch         = 0x00000003,
			.act         = 0x08020303,
			.autoref     = 0x01020000,
			.selfref     = 0x00002000,
			.modereg     = 0x00010000,
			.mifcassch   = 0x00000000,
		}
	}
};

static const struct amc_param amc_params_ap = {
	.flags     	 	= (FLAG_AMC_PARAM_BUSTAT23 | FLAG_AMC_PARAM_SLOW_BOOT),

	.tREFi      		= 0x5d,
	.longsrcnt  		= 0x2008,
	.srextrarefcnt		= 0x00020000,
	.rdlat      		= 6,
	.wrlat      		= 3,
	.phyrdlat   		= 10,
	.phywrlat   		= 2,
	.pdn        		= 0x22000403,
	.read       		= 0x00000203,
	.bustat     		= 0x0022221b,
	.bustat2    		= 0x0022221b,
	.derate     		= 0x2483135b,
	.mcphyupdate		= 0x05030000,
	.autoref_params		= 0x00000021,
	.pwrmngtparam_small	= 0x00011000,
	.pwrmngtparam_guided	= 0x01801000,
	.chnldec		= 0x00050000,
	.arefparam		= 0x0c00a000,
	.aref_freq0		= 0x00000110,
	.schen_default		= 0x00000110,
	.mr1			= 0x44,
	.mr2			= 0x1a,
	.pwrmngten_default	= 0x00100000,
	
	.freq     		= {
		{	// 522MHz
			.cas         = 0x00000308,
			.pch         = 0x00030612,
			.act         = 0x14040908,
			.autoref     = 0x19350000,
			.selfref     = 0x00039060,
			.modereg     = 0x40250772,
			.mifcassch   = 0x001f0110,
		},
		{	// 400MHz
			.cas         = 0x00000005,
			.pch         = 0x00000009,
			.act         = 0x0a020505,
			.autoref     = 0x0d1b0000,
			.selfref     = 0x0001d000,
			.modereg     = 0x00130000,
			.mifcassch   = 0x00000000,
		},
		{	// 200MHz
			.cas         = 0x00000003,
			.pch         = 0x00000005,
			.act         = 0x08020303,
			.autoref     = 0x070e0000,
			.selfref     = 0x0000f000,
			.modereg     = 0x000a0000,
			.mifcassch   = 0x00000000,
		},
		{	// 50
			.cas         = 0x00000003,
			.pch         = 0x00000003,
			.act         = 0x08020303,
			.autoref     = 0x02040000,
			.selfref     = 0x00004000,
			.modereg     = 0x00030000,
			.mifcassch   = 0x00000000,
		}
	}
};

static const struct amc_tunable amc_tunables[] = {
	{ 0, 0 }
};
