/*
 * Copyright (C) 2009-2012 Apple Inc. All rights reserved.
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
	.flags                  = FLAG_AMC_PARAM_ENABLE_AIU | FLAG_AMC_PARAM_SLOW_BOOT,

	.freqsel                = 0,
	.tREFi                  = 0x5d,
	.longsrcnt              = 0x0802,
	.srextrarefcnt		= 0x00010000,
	.tREFi_1Gb              = 0xbb,
	.longsrcnt_1Gb          = 0x0401,
	.rdlat                  = 6,
	.wrlat                  = 3,
	.phyrdlat               = 6,				// rdlat
	.phywrlat   		= 3,
	.pdn                    = 0x33000303,
	.read                   = 0x00000103,
	.bustat                 = 0x00020002,
	.derate                 = 0x28831258,
	.mcphyupdate            = 0x00000009,			// <rdar://problem/8199905> <rdar://problem/8256572>
	.pwrmngtparam_small     = 0x00013000,
	.pwrmngtparam_guided    = 0x00fa3000,
	.bootclkdivsr           = 8,
	.aref_freq0		= 0x00100100,
	.aref_freq1		= 0x00100000,
	.chnldec		= 0x00000F00,
	.arefparam              = 0x06000000,
	.readleveling           = 0x02000301,

	.freq                   = {
		{	// 400MHz
			.cas         = 0x00000308,
			.pch         = 0x00030611,
			.act         = 0x14040908,
			.autoref     = 0x1834a000,
			.selfref     = 0x00038060,
			.modereg     = 0x00240502,
			.mifcassch   = 0x001f0111,
			.mifqmaxctrl = 0x00000000,
		},
		{	// 200MHz
			.cas         = 0x00000004,
			.pch         = 0x00000009,
			.act         = 0x0a020504,
			.autoref     = 0x0c1a0000,
			.selfref     = 0x0001c000,
			.modereg     = 0x00120000,
			.mifcassch   = 0x00000000,
			.mifqmaxctrl = 0x00000000,
		},
		{	// 100MHz
			.cas         = 0x00000003,
			.pch         = 0x00000005,
			.act         = 0x08020303,
			.autoref     = 0x060d0000,
			.selfref     = 0x0000e000,
			.modereg     = 0x00090000,
			.mifcassch   = 0x00000000,
			.mifqmaxctrl = 0x00000000,
		},
		{	// 50 & 24MHz
			.cas         = 0x00000003,
			.pch         = 0x00000003,
			.act         = 0x08020303,
			.autoref     = 0x03070000,
			.selfref     = 0x00007000,
			.modereg     = 0x00060000,
			.mifcassch   = 0x00000010,
			.mifqmaxctrl = 0x00000001,
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

	{ &rAMC_AIU_RDBRSTLEN,		0x01010101 },		// burst len is 64B
	{ &rAMC_AIU_RDMINCRD,		0x7B7B7C7B },		// RT:NRT:CPU:GFX = 40:40:5:10
	{ &rAMC_AIU_WRBRSTLEN,		0x01010101 },		// burst len is 64B
	{ &rAMC_AIU_WRMINCRD,		0x7B7B7C7B },		// RT:NRT:CPU:GFX = 40:40:5:10

	{ &rAMC_AIU_CPUSPCREQ,		0x00000019 },		// CPU ageout timer
	{ &rAMC_AIU_GFXSPCREQ,		0x00000032 },		// GFX ageout timer
	{ &rAMC_AIU_NRTSPCREQ,		0x00000032 },		// Hperf-NRT ageout timer
	{ &rAMC_AIU_RTSPCREQ,		0x00000019 },		// Hperf-RT ageout timer
	{ &rAMC_AIU_WPQSCH,		0x10001000 },		// only supported for Hperf-RT
	{ &rAMC_AIU_THOTLCPUPARAM,	0x0A4640C4 },
	{ &rAMC_AIU_THOTLRTPARAM,	0x0A4640C4 },
	{ &rAMC_AIU_THOTLNRTPARAM,	0x00000000 },
	{ &rAMC_AIU_THOTLGFXPARAM,	0x00000000 },
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

	// Platform specific
	
	{ &rAMC_AIU_RDMAXCRD,		0x2828050a },		// RT:NRT:CPU:GFX = 40:40:5:10
	{ &rAMC_AIU_RDREFL,		0x21210408 },		// 5/6 of AIURDMAXCRD fields
	{ &rAMC_AIU_WRMAXCRD,		0x2828050a }, 		// RT:NRT:CPU:GFX = 40:40:5:10
	{ &rAMC_AIU_WRREFL,		0x21210408 },		// 5/6 of AIUWRMAXCRD fields
	{ &rAMC_AIU_THOTLEN,		0x00001111 },		// enable throttling
	{ &rAMC_PSQWQCTL1,		0x000000C0 },		// ages write out faster than default (1/2 SelfRef)
	{ &rAMC_MCUQOSLLT,		0x00010104 },
	{ &rAMC_PWRMNGTPARAM,		0x01801000 },		// SelfRef timer = 384 cycles

	// There is a bug around write merge, which we believe is avoided 
	// in current drivers: <rdar://problem/9076703>
	// Power and performance measurements of disabling write merging were
	// done: <rdar://problem/9092558> <rdar://problem/9087726>
	{ &rAMC_PSQWQCTL0,		0x00010100 },		// Write merge enable
};
#endif
