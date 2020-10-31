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

#ifndef __DCSCONFIG_T8010_H
#define __DCSCONFIG_T8010_H

dcs_config_params_t dcs_params = {
	.supported_freqs		= DCS_FREQ(0) | DCS_FREQ(1) | DCS_FREQ(3),
	.num_freqchngctl_regs		= 5,
	.spllctrl_vco_1			= 0x00000015,
	.wrdqcalvrefcodecontrol	= 0x19191111,
	.rddqcalvrefcodecontrol	= 0x19199191,
	.pdn1				= 0x0,
	.modereg			= 0x1a0c80f4,
	.modereg1			= 0x0000000c,
	.autoref_params			= 0x0015005d,
	.pdn				= 0x722762c6,
	.rnkcfg				= 0x00006061,
	.casdllupdatectrl		= 0x03030305,
	.dqsdllupdatectrl		= 0x00030005,
	.rdsdllctrl_step2		= 0x00ff0002,
	.wrdqdqssdllctrl_step2		= 0xff000002,
	.cawrlvlsdllcode		= 0x00ff01ff,
	.dlllocktim			= 0x00c801f4,
	.rdwrdqcaltiming_f0		= 0x01060508,
	.rdwrdqcalseglen_f0		= 0x0d0b1313,
	.rdwrdqcaltiming_f1		= 0x01060305,
	.rdwrdqcalseglen_f1		= 0x09070d0d,
	.rdwrdqcaltiming_f3		= 0x01060308,
	.rddqcalpatprbs4i		= 0x55553c5a,
	.wrdqcalpatprbs4i		= 0x00003c5a,
	.maxrddqssdllmulfactor		= 0x00a00d14,
	.maxwrdqssdllmulfactor		= 0xa0a00d0d,
	.dllupdtctrl1			= 0x80000080,
	.dllupdtintvl			= 0x10200020,
	.dcccontrol			= 0x10050b27,
	.cbdrivestr			= 0x77838317,
	.cbioctl			= 0x0002000b,
	.ckdrivestr			= 0x77838317,
	.ckioctl			= 0x0000000f,
	.b0drivestr			= 0x77838317,
	.b0ioctl			= 0x7160000f,
	.b0odt				= 0x01c00333,
	.b0odtctrl			= 0x00000007,
	.b1drivestr			= 0x77838317,
	.b1odtctrl			= 0x00000007,	
	.b1ioctl			= 0x7160000f,
	.b1odt				= 0x01c00333,
	.dqs0drivestr			= 0x77838317,
	.dqs0ioctl			= 0x7160000f,
	.dqs0odt			= 0x01c00333,
	.dqs0zdetbiasen			= 0x0007082a,
	.dqs0odtctrl			= 0x00000007,
	.dqs1drivestr			= 0x77838317,
	.dqs1ioctl			= 0x7160000f,
	.dqs1odt			= 0x01c00333,
	.dqs1zdetbiasen			= 0x0007082a,
	.dqs1odtctrl			= 0x00000007,
	.zcalfsm1			= 0x00887f7f,
	.zcalfsm0			= 0x000f031b,
	.spare0				= 0x00000016,
	.hwrddqcaltvref			= 0x08080808,
	.hwwrdqcaltvref			= 0x323286c8,
	.arefparam			= 0x08012019,
	.autoref_params2		= 0x0017005d,
	.odtszqc			= 0x00001000,
	.longsr				= 0x01012008,
	.freqchngctl_step3		= 0x00010000,
	.mr3cmd				= 0xf3,
	.mr13cmd			= 0x18,
	.addrcfg			= 0x00030201,
	.chnldec			= 0x00050220,
// steps 7-14 are skipped for FPGA since only freq used is 50 MHz
// <rdar://problem/19339246> Cayman: Implement stesp 7-14 of the memory init sequence (only used on silicon)
//	.mr3cmd_step11			= 0xb2,
	.mr13cmd_step15			= 0x50,
	.freqchngctl_step15		= 0x00009999,
	.rdsdllctrl_step15		= 0x00140004,
	.odtszqc2			= 0xc0001000,
	.qbren_step16			= 0x0000000f,
	.b0dyniselasrtime		= 0x00001c25,
	.b0dynisel			= 0x00000003,
	.b1dynisel			= 0x00000003,
	.qbrparam			= 0x00001070,
	.qbren				= 0x0000000f,
	.mccgen				= 0x00100124,
	.caampclk			= 0x8a000000,
	.dqampclk			= 0x80000000,
	.pwrmngten			= 0x00000033,
	.odtszqc3			= 0xc0003320,
	
	.freq			= {
		{	// 1600MHz
			.clk_period      = CLK_PERIOD_1600,
			.freqchngctl     = {0x18cd104d, 0x110c110e, 0xa303440b, 0xde012d02, 0x00000416},
			.freqchngtim     = 0x000c1108,
			.lat             = 0x00113387,
			.phyrdwrtim      = 0x00060d0c,
			.caspch          = 0x63e2080f,
			.act             = 0x2008110f,
			.autoref         = 0x489000a0,
			.selfref         = 0xa0096012,
			.pdn             = 0x00000076,
			.derate          = 0x4d05a910,
			.lat2		 = 0x00123387,
			.tat             = 0x01412222,
			.mifqmaxctrl             = 0x00000100,
			.cavref          = 0x00000003,
			.dqvref          = 0x00c00000,
			.dqdficaltiming  = 0x04060604,
			.rddqcalwindow   = 0x00f801d1,
			.wrdqcalwindow   = 0x012f0360,
			.rdcapcfg        = 0x00000810,
			.autoref2        = 0x48900092,
		},
		{	// 1066MHz
			.freqchngctl     = {0x18cd104d, 0x110c110e, 0xd303220b, 0xbe011b02, 0x00000216},
			.freqchngtim     = 0x000c1108,
			.lat             = 0x001022c5,
			.phyrdwrtim      = 0x00040c09,
			.caspch          = 0x4297060a,
			.act             = 0x16060c0a,
			.autoref         = 0x3060006b,
			.selfref         = 0x6b064012,
			.pdn             = 0x00000054,
			.derate          = 0x34b3760b,
			.lat2		 = 0x001122c5,
			.tat             = 0x01312222,
			.cavref          = 0x00000003,
			.dqvref          = 0x00c00000,
			.cadficaltiming  = 0x04060604,
			.dqdficaltiming  = 0x04060604,
			.rddqcalwindow   = 0x00f801d1,
			.wrdqcalwindow   = 0x012f0360,
			.rdcapcfg        = 0x2000070c,
			.autoref2         = 0x30600061,
		},
		{	// 200MHz
			.freqchngctl     = {0x18cd104d, 0x590c590e, 0xf303000b, 0x8e010002, 0x00000016},
			.freqchngtim     = 0x000c1108,
			.phyrdwrtim      = 0x00010b01,
			.caspch          = 0x40c50402,
			.act             = 0x04020302,
			.autoref         = 0x09120014,
			.selfref         = 0x28013012,
			.pdn             = 0x00000033,
			.derate          = 0x0c212142,
			.lat2		 = 0x001110c2,
			.tat             = 0x01212222,
			.cavref          = 0x00000003,
			.dqvref          = 0x00000000,
			.cadficaltiming  = 0x04060604,
			.dqdficaltiming  = 0x04060604,
			.rdcapcfg        = 0x40000406,
			.autoref2         = 0x09120013,
		},
		{	// 50MHz
			.freqchngctl     = {0x18cd104d, 0x590c590e, 0xf303000b, 0x8e010002, 0x00000016},
			.freqchngtim     = 0x000c1108,
			.phyrdwrtim      = 0x00010901,
			.caspch          = 0x40c20402,
			.act             = 0x02020404,
			.autoref         = 0x03050005,
			.selfref         = 0x28006012,
			.pdn             = 0x00000033,
			.derate          = 0x10412082,
			.lat2		 = 0x001110c2,
			.tat             = 0x01212222,
			.mifqmaxctrl             = 0x00000001,
			.cavref          = 0x00000003,
			.dqvref          = 0x00000000,
			.cadficaltiming  = 0x02060602,
			.dqdficaltiming  = 0x02060602,
			.rdcapcfg        = 0x60000406,
			.autoref2         = 0x03050005,
		}
	}
};

#define BIT32_MASKFULL			(0xFFFFFFFF)
#define BIT32_MASK(_n)			(BIT32_MASKFULL >> (32 - (_n)))
#define BIT32_MASK_VAL(_h,_l,_d)	(((_d) & BIT32_MASK((_h) - (_l) + 1)) << (_l))
#define BIT32_MASK_HI_LO(_h,_l)		BIT32_MASK_VAL(_h,_l,BIT32_MASKFULL)
#define DCS_MASK_VAL(_h,_l,_d)		BIT32_MASK_HI_LO(_h,_l) , BIT32_MASK_VAL(_h,_l,_d)

// DCS Tunables for t8010
// Tunables Spec versions:
// MCU: https://seg-docs.ecs.apple.com/projects/cayman//release/UserManual/tunables/MCU.html dated 2015/02/09
// Notes:
//	All channels are tuned in exactly the same way
//	Registers with all fields' tunable values matching the reset value omitted
const dcs_tunable_t dcs_tunables[] = {

	{ rDCS_MCU_CLKGATE,              DCS_MASK_VAL(19, 16,    0x1) },
	{ rDCS_MCU_CLKGATE,              DCS_MASK_VAL(15, 12,    0x3) },
	{ rDCS_MCU_CLKGATE,              DCS_MASK_VAL( 8,  7,    0x2) },
	{ rDCS_MCU_CLKGATE,              DCS_MASK_VAL( 6,  4,    0x1) },

	{ rDCS_MCU_AMCCLKPWRGATE,        DCS_MASK_VAL(12, 12,    0x1) },
	{ rDCS_MCU_AMCCLKPWRGATE,        DCS_MASK_VAL( 8,  8,    0x1) },

	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(31, 24,   0xff) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(20, 20,    0x0) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(16, 16,    0x1) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(12, 12,    0x0) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL( 8,  8,    0x1) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL( 4,  4,    0x1) },

	{ rDCS_MCU_PSQRQTIMER0,          DCS_MASK_VAL(27, 16,  0x014) },
	{ rDCS_MCU_PSQRQTIMER0,          DCS_MASK_VAL(11,  0,  0x040) },

	{ rDCS_MCU_PSQRQTIMER1,          DCS_MASK_VAL(27, 16,  0x00f) },
	{ rDCS_MCU_PSQRQTIMER1,          DCS_MASK_VAL(11,  0,  0x040) },

	{ rDCS_MCU_PSQRQBRST,            DCS_MASK_VAL(28, 24,   0x04) },
	{ rDCS_MCU_PSQRQBRST,            DCS_MASK_VAL(20, 16,   0x10) },
	{ rDCS_MCU_PSQRQBRST,            DCS_MASK_VAL(12,  8,   0x04) },
	{ rDCS_MCU_PSQRQBRST,            DCS_MASK_VAL( 4,  0,   0x10) },

	{ rDCS_MCU_PSQRQSCHCRD,          DCS_MASK_VAL(23, 16,   0x10) },
	{ rDCS_MCU_PSQRQSCHCRD,          DCS_MASK_VAL(15,  8,   0x14) },
	{ rDCS_MCU_PSQRQSCHCRD,          DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_PSQWQ_CTL(0),         DCS_MASK_VAL(16, 16,    0x1) },
	{ rDCS_MCU_PSQWQ_CTL(0),         DCS_MASK_VAL( 8,  8,    0x1) },

	// part of init sequence, so have to apply tunables even if same as reset values
	{ rDCS_MCU_PSQWQ_CTL(1),         DCS_MASK_VAL(31, 16, 0x01e6) },
	{ rDCS_MCU_PSQWQ_CTL(1),         DCS_MASK_VAL(15,  0, 0x02da) },

	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL(28, 24,   0x04) },
	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL(20, 16,   0x08) },
	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL(12,  8,   0x04) },
	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL( 4,  0,   0x10) },

	{ rDCS_MCU_PSQWQSCHCRD,          DCS_MASK_VAL(23, 16,   0x10) },
	{ rDCS_MCU_PSQWQSCHCRD,          DCS_MASK_VAL(15,  8,   0x14) },
	{ rDCS_MCU_PSQWQSCHCRD,          DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_QOSLLT,               DCS_MASK_VAL( 8,  8,    0x1) },
	{ rDCS_MCU_QOSLLT,               DCS_MASK_VAL( 4,  0,   0x1f) },

	{ rDCS_MCU_SBQOSUPGCTL,          DCS_MASK_VAL(31, 31,    0x1) },
	{ rDCS_MCU_SBQOSUPGCTL,          DCS_MASK_VAL(15,  8,   0x6f) },
	{ rDCS_MCU_SBQOSUPGCTL,          DCS_MASK_VAL( 7,  0,   0x6e) },

	// DRAM Cfg
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(0), DCS_MASK_VAL(31, 16, 0x0300) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(0), DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_PWRMNGTPARAM_FREQ(1), DCS_MASK_VAL(31, 16, 0x0200) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(1), DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_PWRMNGTPARAM_FREQ(2), DCS_MASK_VAL(31, 16, 0x0180) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(2), DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_PWRMNGTPARAM_FREQ(3), DCS_MASK_VAL(31, 16, 0x0017) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(3), DCS_MASK_VAL( 7,  0,   0x00) },

	// Alternate End Marker
	// { 0, 0, 0 }
};

#endif
