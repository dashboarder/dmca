/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
	.flags      		= FLAG_AMC_PARAM_BUSTAT23 | FLAG_AMC_PARAM_ENABLE_AIU | FLAG_AMC_PARAM_LEGACY | FLAG_AMC_PARAM_ZQCL,

	.freqsel    		= 3,
	.tREFi      		= 0x5d,
	.longsrcnt  		= 0x2008,
	.srextrarefcnt		= (0x2 << 16),
	.rdlat      		= 4,
	.wrlat      		= 2,
	.phyrdlat   		= 8,
	.phywrlat   		= 1,
	.pdn        		= 0x22000303,
	.read       		= 0x00000102,
	.bustat     		= 0x00222209,
	.bustat2    		= 0x00222209,
	.derate     		= 0x1c620d46,
	.mcphyupdate		= 0x05030000,
	.mcphyupdate1		= 0x00000004,
	.autoref_params		= 0x1a1a1a1a,
	.chnldec		= 0x00050001,
	.qbrparam		= 0x00510000,
	.odts			= 0x190,
	.aref_freq0		= 0x00000110,
	.pwrmngtparam_small	= 0x00011000,
	.pwrmngtparam_guided	= 0x01803000,
	.schen_default		= 0x00000110,
	.pwrmngten_default	= 0x00100100,
	.addrcfg	= 0x00020201,
	.bankhash0	= 0x00006db6,
	.bankhash1	= 0x00005b6d,
	.bankhash2	= 0x000036db,
	.bootclkdivsr		= 1,
	.mr1			= 0xc4,
	.mr3			= 0x2,
	.mr2			= 0x6,
	.mr63			= 0xfc,
	.readleveling		= 0x00000300,

	.freq     		= {
		{	// 533MHz
			.cas         = 0x00000206,
			.pch         = 0x0002040c,
			.act         = 0x0c040706,
			.autoref     = 0x11240000,
			.selfref     = 0x00026040,
			.modereg     = 0x20190762,
			.arefparam   = 0x0c022131,
			.mifcassch   = 0x00000110,
			.trefbwbasecyc   = 0x0000002d,
		},
		{	// 266MHz
			.cas         = 0x00000004,
			.pch         = 0x00000007,
			.act         = 0x06020404,
			.autoref     = 0x09120000,
			.selfref     = 0x00014000,
			.modereg     = 0x000d0000,
			.arefparam   = 0x00000101,
			.mifcassch   = 0x00000000,
			.trefbwbasecyc   = 0x00000017,
		},
		{	// 133MHz
			.cas         = 0x00000002,
			.pch         = 0x00000004,
			.act         = 0x04010302,
			.autoref     = 0x050a0000,
			.selfref     = 0x0000a000,
			.modereg     = 0x00070000,
			.arefparam   = 0x00000101,
			.mifcassch   = 0x00000000,
			.trefbwbasecyc   = 0x0000000c,
		},
		{	// 24MHz
			.cas         = 0x00000002,
			.pch         = 0x00000002,
			.act         = 0x04010202,
			.autoref     = 0x02030000,
			.selfref     = 0x00003000,
			.modereg     = 0x00020000,
			.arefparam   = 0x00000101,
			.mifcassch   = 0x00000010,
		}
	}
};

static const struct amc_tunable amc_tunables[] = {
	{ &rAMC_AIUPRT_CFG, 0x00070000},
	{ &rAMC_AIUPRT_RD_GEN, 0x80000018},
	{ &rAMC_AIU_CHNLCRD, 0x00180018},
	{ &rAMC_AIU_CHNLTM, 0x40003101},
	{ &rAMC_AIU_RDBRSTLEN, 0x01010101},
	{ &rAMC_AIU_RDMAXCRD, 0x3012097f},
	{ &rAMC_AIU_RDREFL, 0x280f087f},
	{ &rAMC_AIU_WRBRSTLEN, 0x01010101},
	{ &rAMC_AIU_WRMAXCRD, 0x3012097f},
	{ &rAMC_AIU_WRREFL, 0x280f087f},
	{ &rAMC_AIU_CPUSPCREQ, 0x00000013},
	{ &rAMC_AIU_GFXSPCREQ, 0x00000019},
	{ &rAMC_AIU_NRTSPCREQ, 0x00000019},
	{ &rAMC_AIU_RTSPCREQ, 0x00000013},
	{ &rAMC_AIU_WPQSCH, 0x08000800},
	{ &rAMC_AIU_THOTLEN, 0x00000030},
	{ &rAMC_AIU_THOTLCPUPARAM, 0x00000084},
	{ &rAMC_AIU_THOTLRTPARAM, 0x00000084},
	{ &rAMC_AIU_THOTLNRTPARAM, 0x00000080},
	{ &rAMC_AIU_THOTLGFXPARAM, 0x00000080},
	{ &rAMC_AIU_PUSHEN, 0x00001100},
	{ &rAMC_AIU_THOTLCPUPARAM2, 0x00c00040},
	{ &rAMC_AIU_THOTLRTPARAM2, 0x00c00040},
	{ &rAMC_AIU_THOTLNRTPARAM2, 0x00000040},
	{ &rAMC_AIU_THOTLGFXPARAM2, 0x00000040},
	{ &rAMC_AIU_LLTSCHCTL, 0x00000001},
	{ &rAMC_OPIDLETMR, 0x00200000},
	{ &rAMC_OPTMRADJPARAM, 0x20400064},
	{ &rAMC_MIFACTSCH, 0x00000001},
	{ &rAMC_PSQRQCTL0, 0x01010000},
	{ &rAMC_PSQRQCTL1, 0x0000132a},
	{ &rAMC_PSQRQTIMER0, 0x00000018},
	{ &rAMC_PSQRQTIMER1, 0x00000006},
	{ &rAMC_PSQRQTIMER2, 0x00000020},
	{ &rAMC_PSQRQBRST, 0x040c040c},
	{ &rAMC_PSQRQSCHCRD, 0x002028f5},
	{ &rAMC_PSQWQCTL1, 0x00000180},
	{ &rAMC_PSQWQBRST, 0x02020208},
	{ &rAMC_PSQWQSCHCRD, 0x000405f5},
	{ &rAMC_MCUQOS, 0x00010f04},
	{ &rAMC_MCUQOSLLT, 0x00010108},

	{0, 0},
};
#endif
