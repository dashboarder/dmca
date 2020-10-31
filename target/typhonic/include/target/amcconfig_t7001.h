/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
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

static const struct amc_param amc_params = {
	.lat			= 0x00060300,
	.phyrdwrtim		= 0x00020a04,
	.tREFi			= 0x5d,
	.pdn			= 0x22000403,
	.derate			= 0x24831359,
	.read			= 0x00000203,
	.bustat			= 0x00222209,
	.bustat2		= 0x00222209,
	.pwrmngten_default	= 0x00100100,
	.schen_default		= 0x00001110,
	.mcphyupdate		= 0x15030000,
	.mcphyupdate1		= 0x00000003,
	.arefparam		= 0x0ca2b532,
	.longsr			= 0x05022008,
	.mr1			= 0x44,
	.mr2			= 0x1a,
	.mr3			= 0x03,
	.addrcfg		= 0x00020201,
	.mccchnldec		= 0x00050020,
	.mcuchnhash		= 0x0ffffd54,
	.mcuchnhash2		= 0x0aaaaaa8,
	.mcschnldec		= 0x00020200,
	.qbrparam		= 0x00050200,
	.odts			= 0x00000320,
	.addrmapmode		= 0x00000501,
	
	.freq			= {
		{	// 800MHz
			.cas         = 0x00000308,
			.pch         = 0x00030612,
			.act         = 0x12040908,
			.autoref     = 0x19350000,
			.selfref     = 0x00039060,
			.modereg     = 0x20250782,
		},
		{	// 400MHz
			.cas         = 0x00000005,
			.pch         = 0x00000009,
			.act         = 0x09020505,
			.autoref     = 0x0d1b0000,
			.selfref     = 0x0001d000,
			.modereg     = 0x00130000,
		},
		{	// 200MHz
			.cas         = 0x00000003,
			.pch         = 0x00000005,
			.act         = 0x05010303,
			.autoref     = 0x070e0000,
			.selfref     = 0x0000f000,
			.modereg     = 0x000a0000,
		},
		{	// 50MHz
			.cas         = 0x00000002,
			.pch         = 0x00000002,
			.act         = 0x04010202,
			.autoref     = 0x02040000,
			.selfref     = 0x00004000,
			.modereg     = 0x00030000,
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
	{ &rAMC_PSQRQCTL1,		0x00001700 },
	{ &rAMC_PSQRQTIMER0,		0x00000040 },
	{ &rAMC_PSQRQTIMER1,		0x00000014 },
	{ &rAMC_PSQRQTIMER2,		0x00000040 },
	{ &rAMC_PSQRQBRST,		0x04100410 },
	{ &rAMC_PSQRQSCHCRD,		0x00101400 },
	{ &rAMC_PSQWQCTL0,		0x00010100 },
	{ &rAMC_PSQWQCTL1,		0x00000336 },
	{ &rAMC_PSQWQBRST,		0x04080410 },
	{ &rAMC_PSQWQSCHCRD,		0x00101400 },
	{ &rAMC_MCUQOSLLT,		0x0000011f },
	{ &rAMC_PWRMNGTPARAM,		0x03523000 },
};
