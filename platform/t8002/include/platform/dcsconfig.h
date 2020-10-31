/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __DCSCONFIG_H
#define __DCSCONFIG_H

// Needful defnitions for the initializers below
#include <drivers/dcs/dcs.h>
#include <drivers/dcs/dcs_regs.h>
#include <drivers/dcs/dcs_init_lib.h>
#include <drivers/dcs/dcs_calibration.h>

dcs_config_params_t dcs_params = {
	.supported_freqs		= DCS_FREQ(0) | DCS_FREQ(3),
	.num_freqchngctl_regs		= 5,
	.spllctrl_vco_1			= 0x00000018,
	.spllctrl_vco_2			= 0x00000015,
	.wrdqcalvrefcodecontrol		= 0x0d000019,
	.rddqcalvrefcodecontrol		= 0x0d000099,
	.dllupdtctrl			= 0x50017350,
	.modereg			= 0x120c80a4,
	.autoref_params			= 0x0015005d,
	.pdn				= 0x72274284,
	.rnkcfg				= 0x00006061,
	.casdllupdatectrl		= 0x03030305,
	.dqsdllupdatectrl		= 0x00030005,
	.rdsdllctrl_step2		= 0x00ff0002,
	.wrdqdqssdllctrl_step2		= 0xff000002,
	.cawrlvlsdllcode		= 0x00ff01ff,
	.dlllocktim			= 0x00c801f4,
	.rdwrdqcaltiming_f0		= 0x01040304,
	.rdwrdqcaltiming_f1		= 0x01040402,
	.rdwrdqcaltimingctrl1		= 0x0000381e,
	.rdwrdqcaltimingctrl2		= 0x01100e01,
	.rddqcalpatprbs4i		= 0x55553c5a,
	.wrdqcalpatprbs4i		= 0x00003c5a,
	.maxrddqssdllmulfactor		= 0x00a00c0c,
	.maxwrdqssdllmulfactor		= 0xa0a00c0d,
	.dllupdtctrl1			= 0x80017580,
	.dllupdtintvl			= 0x10200020,
	.dcccontrol			= 0x10050b27,
	.cbdrivestr			= 0x33838317,
	.cbioctl			= 0x00020021,
	.ckdrivestr			= 0x33838317,
	.ckioctl			= 0x00000021,
	.b0drivestr			= 0x33838317,
	.b0ioctl			= 0x71500021,
	.b0odt				= 0x01c00339,
	.b0odtctrl			= 0x00000007,
	.b1drivestr			= 0x33838317,
	.b1ioctl			= 0x71500021,
	.b1odt				= 0x01c00339,
	.b1odtctrl			= 0x00000007,
	.dqs0drivestr			= 0x33838317,
	.dqs0ioctl			= 0x71500021,
	.dqs0odt			= 0x01c00339,
	.dqs0zdetbiasen			= 0x00060928,
	.dqs0odtctrl			= 0x00000007,
	.dqs1drivestr			= 0x33838317,
	.dqs1ioctl			= 0x71500021,
	.dqs1odt			= 0x01c00339,
	.dqs1zdetbiasen			= 0x00060928,
	.dqs1odtctrl			= 0x00000007,
	.zcalfsm1			= 0x005b7f7f,
	.zcalfsm0			= 0x000f0333,
	.spare0				= 0x0000000b,
	.dficaltiming			= 0x05000504,
	.hwrddqcaltvref			= 0x08080808,
	.arefparam			= 0x0d012019,
	.autoref_params2		= 0x0015005d,
	.odtszqc			= 0x00003000,
	.longsr				= 0x01022008,
	.freqchngctl_step3		= 0x01010000,
	.mr3cmd				= 0xf2,
	.mr13cmd			= 0x18,
	.addrcfg			= 0x00030201,
	.aiuprtbaddr		= 0x00000010,
	.chnldec			= 0x00050101,
	// steps 7-10 are skipped for M8 since freq bucket 1 not used
	.mr13cmd_step11			= 0x58,
	.mr2cmd_step11			= 0x1b,
	.mr1cmd_step11			= 0xbe,
	.mr3cmd_step11			= 0xb2,
	.mr22cmd_step11			= 0x03,
	.mr11cmd_step11			= 0x33,
	.mr12cmd_step11			= 0x19,
	.mr14cmd_step11			= 0x19,
	.mr13cmd_step13			= 0x98,
	.mr13cmd_step15			= 0x90,
	.freqchngctl_step15		= 0x00009009,
	.rdsdllctrl_step15		= 0x001d0004,
	.odtszqc2			= 0xc0003000,
	.b0dyniselasrtime		= 0x00000014,
	.b0dynisel			= 0x00000001,
	.b1dynisel			= 0x00000009,
	.qbrparam			= 0x00000000,
	.qbren				= 0x00000003,
	.caampclk			= 0x11000000,
	.dqampclk			= 0x11000000,
	.pwrmngten			= 0x00000133,
	.odtszqc3			= 0xc0003320,
	
	.freq			= {
		{	// 1066MHz
			.clk_period      = CLK_PERIOD_1600,
			.freqchngctl     = {0x18cd104d, 0x190c190e, 0xb203330b, 0xbe011b02, 0x00000316},
			.freqchngtim     = 0x000c1108,
			.lat             = 0x00103285,
			.phyrdwrtim      = 0x00040c08,
			.caspch          = 0x43d7060a,
			.act             = 0x16060c0a,
			.autoref         = 0x30600078,
			.selfref         = 0x6b064012,
			.derate          = 0x34b3760b,
			.lat2		 = 0x00112285,
			.tat             = 0x01312222,
			.mifqmaxctrl             = 0x00000100,
			.cavref          = 0x00000003,
			.odt_enable      = 0x00000001,
			.dqvref          = 0x00cb00cb,
			.dqdficaltiming  = 0x04020402,
			.rddqcalwindow   = 0x00ff01d1,
			.wrdqcalwindow   = 0x00ff0160,
			.rdcapcfg        = 0x0100070c,
			.autoref2        = 0x30600070,
		},
		{	// 800MHz
			.freqchngctl     = {0x18cd104d, 0x110c110e, 0xd303220b, 0xae015202, 0x00000216},
			.freqchngtim     = 0x000c1108,
			.lat             = 0x00112206,
			.phyrdwrtim      = 0x00050b06,
			.caspch          = 0x42110408,
			.act             = 0x10040908,
			.autoref         = 0x24480050,
			.selfref         = 0x5004b012,
			.derate          = 0x28835488,
			.lat2		 = 0x00112206,
			.tat             = 0x01312222,
			.cavref          = 0x00000000,
			.odt_enable      = 0x00000001,
			.dqvref          = 0x00000000,
			.cadficaltiming  = 0x04000410,
			.dqdficaltiming  = 0x04020402,
			.rddqcalwindow   = 0x00ff01d1,
			.wrdqcalwindow   = 0x00ff0160,
			.rdcapcfg        = 0x2100060a,
			.autoref2         = 0x24480054,
		},
		{	// 200MHz
			.freqchngctl     = {0x18cd104d, 0x590c590e, 0xf303000b, 0x8e010002, 0x00000016},
			.freqchngtim     = 0x000c1108,
			.phyrdwrtim      = 0x00010b01,
			.caspch          = 0x40c50402,
			.act             = 0x04020302,
			.autoref         = 0x09120014,
			.selfref         = 0x28013012,
			.derate          = 0x0c212142,
			.lat2		 = 0x001110c2,
			.tat             = 0x01212222,
			.cavref          = 0x00000000,
			.dqvref          = 0x00000000,
			.cadficaltiming  = 0x04000410,
			.dqdficaltiming  = 0x04020402,
			.rdcapcfg        = 0x41000406,
			.autoref2         = 0x09120015,
		},
		{	// 50MHz
			.freqchngctl     = {0x18cd104d, 0x4d0c4d0e, 0xf203000b, 0x8e010002, 0x00000016},
			.freqchngtim     = 0x000c1108,
			.phyrdwrtim      = 0x00010901,
			.caspch          = 0x40c20402,
			.act             = 0x01020404,
			.autoref         = 0x03050005,
			.selfref         = 0x28005012,
			.derate          = 0x10412082,
			.lat2		 = 0x001110c2,
			.tat             = 0x01212222,
			.mifqmaxctrl             = 0x00000001,
			.cavref          = 0x00000003,
			.odt_enable      = 0x00000000,
			.dqvref          = 0x00000000,
			.cadficaltiming  = 0x04000410,
			.dqdficaltiming  = 0x04020402,			
			.rdcapcfg        = 0x61000406,
			.autoref2         = 0x03050005,
		}
	}
};

#define BIT32_MASKFULL			(0xFFFFFFFF)
#define BIT32_MASK(_n)			(BIT32_MASKFULL >> (32 - (_n)))
#define BIT32_MASK_VAL(_h,_l,_d)	(((_d) & BIT32_MASK((_h) - (_l) + 1)) << (_l))
#define BIT32_MASK_HI_LO(_h,_l)		BIT32_MASK_VAL(_h,_l,BIT32_MASKFULL)
#define DCS_MASK_VAL(_h,_l,_d)		BIT32_MASK_HI_LO(_h,_l) , BIT32_MASK_VAL(_h,_l,_d)

// M8 A0 Tunables Specs:
//	https://seg-docs.ecs.apple.com/projects/m8//release/UserManual/tunables/amc.html dated 1/16/2015 20:07:00
//	https://seg-docs.ecs.apple.com/projects/m8//release/UserManual/tunables/amcsys_aon.html dated 1/16/2015 20:06:27
//	https://seg-docs.ecs.apple.com/projects/m8//release/UserManual/tunables/amcx.html dated 1/16/2015 11:49:17
//	https://seg-docs.ecs.apple.com/projects/m8//release/UserManual/tunables/dcsh_aon.html dated 1/2/2015 00:30:12
//	https://seg-docs.ecs.apple.com/projects/m8//release/UserManual/tunables/ampsca.html dated 1/14/2015 19:42:28
//	https://seg-docs.ecs.apple.com/projects/m8//release/UserManual/tunables/AMPSDQ0.html dated 1/14/2015 19:44:46
//	https://seg-docs.ecs.apple.com/projects/m8//release/UserManual/tunables/AMPSDQ1.html dated 1/14/2015 19:44:46
const dcs_tunable_t dcs_tunables[] = {

	// AMCSYS
	{ rAMC_AIUPRT_RD_GEN,        DCS_MASK_VAL(3,   3,       0x1) },
	{ rAMC_AIUADDRBANKHASH(0),   DCS_MASK_VAL(14,  0,    0x6db6) },
	{ rAMC_AIUADDRBANKHASH(1),   DCS_MASK_VAL(14,  0,    0x5b6d) },
	{ rAMC_AIUADDRBANKHASH(2),   DCS_MASK_VAL(14,  0,    0x36db) },

	{ rAMC_AIUCHNLTM,            DCS_MASK_VAL(24, 20,      0x10) },
	{ rAMC_AIUCHNLTM,            DCS_MASK_VAL(16, 16,       0x1) },
	{ rAMC_AIUCHNLTM,            DCS_MASK_VAL(14, 14,       0x0) },
	{ rAMC_AIUCHNLTM,            DCS_MASK_VAL(13, 13,       0x1) },
	{ rAMC_AIUCHNLTM,            DCS_MASK_VAL(12, 12,       0x1) },
	{ rAMC_AIUCHNLTM,            DCS_MASK_VAL(8,   4,      0x10) },
	{ rAMC_AIUCHNLTM,            DCS_MASK_VAL(0,   0,       0x1) },

	{ rAMC_AIURDBRSTLEN,         DCS_MASK_VAL(30, 28,       0x1) },
	{ rAMC_AIURDBRSTLEN,         DCS_MASK_VAL(26, 24,       0x1) },
	{ rAMC_AIURDBRSTLEN,         DCS_MASK_VAL(18, 16,       0x1) },
	{ rAMC_AIURDBRSTLEN,         DCS_MASK_VAL(10,  8,       0x1) },
	{ rAMC_AIURDBRSTLEN,         DCS_MASK_VAL(2,   0,       0x1) },

	{ rAMC_AIURDMAXCRD,          DCS_MASK_VAL(30, 24,      0x30) },
	{ rAMC_AIURDMAXCRD,          DCS_MASK_VAL(22, 16,       0x8) },
	{ rAMC_AIURDMAXCRD,          DCS_MASK_VAL(14,  8,       0xc) },
	{ rAMC_AIURDMAXCRD,          DCS_MASK_VAL(6,   0,      0x7f) },

	{ rAMC_AIURDREFL,            DCS_MASK_VAL(30, 24,      0x28) },
	{ rAMC_AIURDREFL,            DCS_MASK_VAL(22, 16,       0x7) },
	{ rAMC_AIURDREFL,            DCS_MASK_VAL(14,  8,       0xa) },
	{ rAMC_AIURDREFL,            DCS_MASK_VAL(6,   0,      0x7f) },

	{ rAMC_AIUWRBRSTLEN,         DCS_MASK_VAL(30, 28,       0x1) },
	{ rAMC_AIUWRBRSTLEN,         DCS_MASK_VAL(26, 24,       0x1) },
	{ rAMC_AIUWRBRSTLEN,         DCS_MASK_VAL(18, 16,       0x1) },
	{ rAMC_AIUWRBRSTLEN,         DCS_MASK_VAL(10,  8,       0x1) },
	{ rAMC_AIUWRBRSTLEN,         DCS_MASK_VAL(2,   0,       0x1) },

	{ rAMC_AIUWRMAXCRD,          DCS_MASK_VAL(30, 24,      0x30) },
	{ rAMC_AIUWRMAXCRD,          DCS_MASK_VAL(22, 16,       0x8) },
	{ rAMC_AIUWRMAXCRD,          DCS_MASK_VAL(14,  8,       0xc) },
	{ rAMC_AIUWRMAXCRD,          DCS_MASK_VAL(6,   0,      0x7f) },

	{ rAMC_AIUWRREFL,            DCS_MASK_VAL(30, 24,      0x28) },
	{ rAMC_AIUWRREFL,            DCS_MASK_VAL(22, 16,       0x7) },
	{ rAMC_AIUWRREFL,            DCS_MASK_VAL(14,  8,       0xa) },
	{ rAMC_AIUWRREFL,            DCS_MASK_VAL(6,   0,      0x7f) },

	{ rAMC_AIUCPUSPCREQ,         DCS_MASK_VAL(9,   0,      0x13) },
	{ rAMC_AIUGRXSPCREQ,         DCS_MASK_VAL(9,   0,      0x19) },
	{ rAMC_AIUNRTSPCREQ,         DCS_MASK_VAL(9,   0,      0x19) },
	{ rAMC_AIURLTSPCREQ,         DCS_MASK_VAL(9,   0,      0x13) },

	{ rAMC_AIUWPQSCH,            DCS_MASK_VAL(30, 24,       0x8) },
	{ rAMC_AIUWPQSCH,            DCS_MASK_VAL(14,  8,       0x8) },

	{ rAMC_AIUTHOTLEN,           DCS_MASK_VAL(5,   5,       0x1) },
	{ rAMC_AIUTHOTLEN,           DCS_MASK_VAL(4,   4,       0x1) },

	{ rAMC_AIUTHOTLCPUPARAM,     DCS_MASK_VAL(10,  4,       0x8) },
	{ rAMC_AIUTHOTLCPUPARAM,     DCS_MASK_VAL(2,   2,       0x1) },

	{ rAMC_AIUTHOTLRLTPARAM,     DCS_MASK_VAL(10,  4,       0x8) },
	{ rAMC_AIUTHOTLRLTPARAM,     DCS_MASK_VAL(2,   2,       0x1) },

	{ rAMC_AIUTHOTLNRTPARAM,     DCS_MASK_VAL(10,  4,       0x8) },
	{ rAMC_AIUTHOTLGRXPARAM,     DCS_MASK_VAL(10,  4,       0x8) },

	{ rAMC_AIUPUSHEN,            DCS_MASK_VAL(12, 12,       0x1) },
	{ rAMC_AIUPUSHEN,            DCS_MASK_VAL(8,   8,       0x1) },

	{ rAMC_AIUTHOTLCPUPARAM2,    DCS_MASK_VAL(23, 23,       0x1) },
	{ rAMC_AIUTHOTLCPUPARAM2,    DCS_MASK_VAL(22, 22,       0x1) },
	{ rAMC_AIUTHOTLCPUPARAM2,    DCS_MASK_VAL(10,  4,       0x4) },

	{ rAMC_AIUTHOTLRLTPARAM2,    DCS_MASK_VAL(23, 23,       0x1) },
	{ rAMC_AIUTHOTLRLTPARAM2,    DCS_MASK_VAL(22, 22,       0x1) },
	{ rAMC_AIUTHOTLRLTPARAM2,    DCS_MASK_VAL(10,  4,       0x4) },

	{ rAMC_AIUTHOTLNRTPARAM2,    DCS_MASK_VAL(10,  4,       0x4) },
	{ rAMC_AIUTHOTLGRXPARAM2,    DCS_MASK_VAL(10,  4,       0x4) },

	{ rAMC_AIULLTSCHCTL,         DCS_MASK_VAL(0,   0,       0x1) },
	{ rAMC_AIURDMAXCRD2,         DCS_MASK_VAL(6,   0,      0x12) },
	{ rAMC_AIURDREFL2,           DCS_MASK_VAL(6,   0,       0xf) },
	{ rAMC_AIUWRMAXCRD2,         DCS_MASK_VAL(6,   0,      0x12) },
	{ rAMC_AIUWRMAXCRD2,         DCS_MASK_VAL(6,   0,      0x12) },
	{ rAMC_AIUWRREFL2,           DCS_MASK_VAL(6,   0,       0xf) },
	{ rAMC_AIUMEDSPCREQ,         DCS_MASK_VAL(9,   0,      0x19) },
	{ rAMC_AIUTHOTLMEDPARAM,     DCS_MASK_VAL(10,  4,       0x8) },
	{ rAMC_AIUTHOTLMEDPARAM2,    DCS_MASK_VAL(10,  4,       0x4) },
	{ rAMC_PSQRQCTL0,            DCS_MASK_VAL(31, 24,       0x1) },

	{ rAMC_PSQRQCTL1,            DCS_MASK_VAL(9,   9,        0x1) },
	{ rAMC_PSQRQCTL1,            DCS_MASK_VAL(5,   5,        0x1) },
	{ rAMC_PSQRQCTL1,            DCS_MASK_VAL(3,   3,        0x1) },
	{ rAMC_PSQRQCTL1,            DCS_MASK_VAL(1,   1,        0x1) },

	{ rAMC_PSQRQTIMER(0),        DCS_MASK_VAL(11,  0,        0xa) },
	{ rAMC_PSQRQTIMER(1),        DCS_MASK_VAL(11,  0,        0x5) },
	{ rAMC_PSQRQTIMER(2),        DCS_MASK_VAL(11,  0,       0x20) },

	{ rAMC_PSQRQBRST,            DCS_MASK_VAL(28, 24,        0x4) },
	{ rAMC_PSQRQBRST,            DCS_MASK_VAL(12,  8,        0x4) },

	{ rAMC_PSQRQSCHCRD,          DCS_MASK_VAL(23, 16,       0x20) },
	{ rAMC_PSQRQSCHCRD,          DCS_MASK_VAL(15,  8,       0x28) },

	{ rAMC_PSQWQCTL1,            DCS_MASK_VAL(31, 16,       0x24) },
	{ rAMC_PSQWQCTL1,            DCS_MASK_VAL(15,  0,       0xbb) },

	{ rAMC_PSQWQBRST,            DCS_MASK_VAL(28, 24,        0x2) },
	{ rAMC_PSQWQBRST,            DCS_MASK_VAL(20, 16,        0x2) },
	{ rAMC_PSQWQBRST,            DCS_MASK_VAL(12,  8,        0x2) },
	{ rAMC_PSQWQBRST,            DCS_MASK_VAL(4,   0,        0x8) },

	{ rAMC_PSQWQSCHCRD,          DCS_MASK_VAL(23, 16,        0x4) },
	{ rAMC_PSQWQSCHCRD,          DCS_MASK_VAL(15,  8,        0x5) },

	{ rAMC_MCUQOS,               DCS_MASK_VAL(16, 16,        0x1) },
	
	{ rAMC_MCUQOSLLT,            DCS_MASK_VAL(16, 16,        0x1) },
	{ rAMC_MCUQOSLLT,            DCS_MASK_VAL(4,   0,        0x8) },

	{ rAMC_ADDRMAPMODE,          DCS_MASK_VAL(2,   0,        0x2) },

	{ rAMC_PWRMNGTPARAM,         DCS_MASK_VAL(31, 16,       0x30) },
	{ rAMC_PWRMNGTPARAM,         DCS_MASK_VAL(15,  0,       0xfa) },

	{ rAMC_IDLE2BCG_CONTROL,     DCS_MASK_VAL(0,   0,        0x1) },

	// AMCX
	{ rDCS_MCU_AMCCLKPWRGATE,    DCS_MASK_VAL(12, 12,        0x1) },
	{ rDCS_MCU_AMCCLKPWRGATE,    DCS_MASK_VAL(8,   8,        0x1) },

	{ rDCS_MCU_IDLE_EXTEND_CONTROL, DCS_MASK_VAL(0,   0,        0x1) },

	{ rDCS_MCU_ROWHAM_CTL(0),       DCS_MASK_VAL(31, 16,     0x4002) },
	{ rDCS_MCU_ROWHAM_CTL(0),       DCS_MASK_VAL(0,   0,        0x1) },
	{ rDCS_MCU_ROWHAM_CTL(1),       DCS_MASK_VAL(31, 16,     0x4a38) },
	{ rDCS_MCU_ROWHAM_CTL(1),       DCS_MASK_VAL(15,  0,     0x22d4) },
	{ rDCS_MCU_ROWHAM_CTL(2),       DCS_MASK_VAL(15,  0,     0x4a33) },

	// DCSH_AON
	{ rDCS_AON_IDLE2BCG_CONTROL,    DCS_MASK_VAL(0,   0,        0x1) },

	// AMSPCA
	{ rDCS_AMP_AMPCLK(AMP_CA),              DCS_MASK_VAL(28, 28,        0x0) },
	{ rDCS_AMP_AMPCLK(AMP_CA),              DCS_MASK_VAL(24, 24,        0x0) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(0),        DCS_MASK_VAL(31, 16, 0x2) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(0),        DCS_MASK_VAL(15, 0, 0x3) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(1),        DCS_MASK_VAL(31, 16, 0x2) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(1),        DCS_MASK_VAL(15, 0, 0x3) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(2),        DCS_MASK_VAL(31, 16, 0x2) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(2),        DCS_MASK_VAL(15, 0, 0x3) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(3),        DCS_MASK_VAL(31, 16, 0x2) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(3),        DCS_MASK_VAL(15, 0, 0x3) },

	// AMPSDQ
	{ rDCS_AMP_AMPCLK(AMP_DQ0),              DCS_MASK_VAL(28, 28,        0x0) },
	{ rDCS_AMP_AMPCLK(AMP_DQ0),              DCS_MASK_VAL(24, 24,        0x0) },
	{ rDCS_AMP_AMPCLK(AMP_DQ1),              DCS_MASK_VAL(28, 28,        0x0) },
	{ rDCS_AMP_AMPCLK(AMP_DQ1),              DCS_MASK_VAL(24, 24,        0x0) },
};

#endif
