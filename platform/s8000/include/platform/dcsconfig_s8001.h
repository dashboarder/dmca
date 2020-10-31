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

#ifndef __DCSCONFIG_S8001_H
#define __DCSCONFIG_S8001_H

dcs_config_params_t dcs_params = {
	.supported_freqs		= DCS_FREQ(0) | DCS_FREQ(3),
	.num_freqchngctl_regs		= 5,
	.spllctrl_vco_1			= 0x00000016,
	.spllctrl_vco_2			= 0x00000015,
	.wrdqcalvrefcodecontrol		= 0x19191111,
	.rddqcalvrefcodecontrol		= 0x19191191,
	.rwcfg				= 0x000210ef,
	.dllupdtctrl			= 0x50017350,
	.modereg			= 0x1a0cc0f4,
	.autoref_params			= 0x0015005d,
	.pdn				= 0x722762c6,
	.rnkcfg				= 0x00006061,
	.casdllupdatectrl		= 0x03030305,
	.dqsdllupdatectrl		= 0x00030005,
	.rdsdllctrl_step2		= 0x00ff0002,
	.wrdqdqssdllctrl_step2		= 0xff000002,
	.cawrlvlsdllcode		= 0x00ff01ff,
	.dlllocktim			= 0x00c801f4,
	.rdwrdqcaltiming_f0		= 0x01040508,
	.rdwrdqcalseglen_f0		= 0x000c0012,
	.rdwrdqcaltiming_f1		= 0x01040402,
	.rdwrdqcalseglen_f1		= 0x000c0012,
	.rdwrdqcaltiming_f3		= 0x01040508,
	.rdwrdqcaltimingctrl1		= 0x0000381e,
	.rdwrdqcaltimingctrl2		= 0x01141101,
	.rddqcalpatprbs4i		= 0x55553c5a,
	.wrdqcalpatprbs4i		= 0x00003c5a,
	.maxrddqssdllmulfactor		= 0x00a01414,
	.maxwrdqssdllmulfactor		= 0xa0a00c0d,
	.dllupdtctrl1			= 0x80017580,
	.dllupdtintvl			= 0x10200020,
	.dcccontrol			= 0x10050b27,
	.dcccal				= 0x40401320,
	.cbdrivestr			= 0x33838317,
	.cbioctl			= 0x00020023,
	.ckdrivestr			= 0x33838317,
	.ckioctl			= 0x00000027,
	.b0drivestr			= 0x33838317,
	.b0ioctl			= 0x71500027,
	.b0odt				= 0x01c00333,
	.b0odtctrl			= 0x00000007,
	.b1drivestr			= 0x33838317,
	.b1ioctl			= 0x71500027,
	.b1odt				= 0x01c00333,
	.b1odtctrl			= 0x00000007,
	.dqs0drivestr			= 0x33838317,
	.dqs0ioctl			= 0x71500007,
	.dqs0odt			= 0x01c00336,
	.dqs0zdetbiasen			= 0x00060028,
	.dqs0odtctrl			= 0x00000007,
	.dqs1drivestr			= 0x33838317,
	.dqs1ioctl			= 0x71500007,
	.dqs1odt			= 0x01c00336,
	.dqs1zdetbiasen			= 0x00060028,
	.dqs1odtctrl			= 0x00000007,
	.zcalfsm1			= 0x00887f7f,
	.zcalfsm0			= 0x000f031b,
	.spare0				= 0x00000016,
	.dficaltiming			= 0x06000504,
	.hwrddqcaltvref			= 0x08080808,
	.arefparam			= 0x0d012019,
	.autoref_params2		= 0x0017005d,
	.odtszqc			= 0x00001000,
	.longsr				= 0x01022008,
	.freqchngctl_step3		= 0x01010000,
	.mr3cmd				= 0xf3,
	.mr13cmd			= 0x18,
	.addrcfg			= 0x00030201,
	.chnldec			= 0x00050120,
	// steps 7-10 are skipped for Elba since freq bucket 1 not used
	.mr13cmd_step11			= 0x58,
	.mr2cmd_step11			= 0x2d,
	.mr1cmd_step11			= 0xde,
	.mr3cmd_step11			= 0xb3,
	.mr22cmd_step11			= 0x04,
	.mr11cmd_step11			= 0x44,
	.mr12cmd_step11			= 0x11,
	.mr14cmd_step11			= 0x11,
	.mr13cmd_step13			= 0x98,
	.dqs0wkpupd			= 0x00010782,
	.dqs1wkpupd			= 0x00010782,
	.mr13cmd_step15			= 0x90,
	.freqchngctl_step15		= 0x00009999,
	.rdsdllctrl_step15		= 0x00140004,
	.odtszqc2			= 0xc0001000,
	.qbren_step16			= 0x0000000d,
	.b0dyniselasrtime		= 0x00000019,
	.b0dynisel			= 0x00000001,
	.b1dynisel			= 0x00000000,
	.qbrparam			= 0x000000a8,
	.qbren				= 0x0000000f,
	.mccgen				= 0x00000126,
	.caampclk			= 0x00000000,
	.dqampclk			= 0x00000000,
	.pwrmngten			= 0x00000133,
	.odtszqc3			= 0xc0003320,
	
	.freq			= {
		{	// 1600MHz
			.clk_period      = CLK_PERIOD_1600,
			.freqchngctl     = {0x18cd104d, 0x110c110e, 0xb303440b, 0xde012d02, 0x00000416},
			.freqchngtim     = 0x000c1108,
			.lat             = 0x00103387,
			.phyrdwrtim      = 0x00060d0c,
			.caspch          = 0x63e2080f,
			.act             = 0x2008110f,
			.autoref         = 0x48900078,
			.selfref         = 0xa0096012,
			.derate          = 0x4d05a910,
			.lat2		 = 0x00123387,
			.tat             = 0x01412222,
			.mifqmaxctrl             = 0x00000100,
			.cavref          = 0x00000003,
			.odt_enable      = 0x00000001,
			.dqvref          = 0x00c000c0,
			.dqdficaltiming  = 0x06020404,
			.rddqcalwindow   = 0x00ff01d1,
			.wrdqcalwindow   = 0x00ff0160,
			.rdcapcfg        = 0x01000810,
			.autoref2        = 0x48900092,
		},
		{	// 800MHz
			.freqchngctl     = {0x18cd104d, 0x110c110e, 0xd303220b, 0xae015202, 0x00000216},
			.freqchngtim     = 0x000c1108,
			.lat             = 0x00102206,
			.phyrdwrtim      = 0x00050b06,
			.caspch          = 0x42110408,
			.act             = 0x10040908,
			.autoref         = 0x24480050,
			.selfref         = 0x5004b012,
			.derate          = 0x28835488,
			.lat2		 = 0x00112206,
			.tat             = 0x01312222,
			.cavref          = 0x00000003,
			.odt_enable      = 0x00000001,
			.dqvref          = 0x00c000c0,
			.cadficaltiming  = 0x04000410,
			.dqdficaltiming  = 0x04020402,
			.rddqcalwindow   = 0x00ff01d1,
			.wrdqcalwindow   = 0x00ff0160,
			.rdcapcfg        = 0x2100060a,
			.autoref2         = 0x24480049,
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
			.cavref          = 0x00000003,
			.dqvref          = 0x00000000,
			.cadficaltiming  = 0x04000410,
			.dqdficaltiming  = 0x04020402,
			.rdcapcfg        = 0x41000408,
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
			.derate          = 0x10412082,
			.lat2		 = 0x001110c2,
			.tat             = 0x01212222,
			.mifqmaxctrl             = 0x00000001,
			.cavref          = 0x00000003,
			.odt_enable      = 0x00000000,
			.dqvref          = 0x00000000,
			.cadficaltiming  = 0x04000410,
			.dqdficaltiming  = 0x04020404,
			.rdcapcfg        = 0x61000408,
			.autoref2         = 0x03050005,
		}
	}
};

#define BIT32_MASKFULL			(0xFFFFFFFF)
#define BIT32_MASK(_n)			(BIT32_MASKFULL >> (32 - (_n)))
#define BIT32_MASK_VAL(_h,_l,_d)	(((_d) & BIT32_MASK((_h) - (_l) + 1)) << (_l))
#define BIT32_MASK_HI_LO(_h,_l)		BIT32_MASK_VAL(_h,_l,BIT32_MASKFULL)
#define DCS_MASK_VAL(_h,_l,_d)		BIT32_MASK_HI_LO(_h,_l) , BIT32_MASK_VAL(_h,_l,_d)

// DCS Tunables for s8001 B0
// Tunables Spec versions:
// MCU: https://seg-docs.ecs.apple.com/projects/elba//release/UserManual/tunables_a0/MCU.html v14.2.0
// AMPSCA: https://seg-docs.ecs.apple.com/projects/elba//release/UserManual/tunables_a0/AMPSCA.html dated 2014/11/10 13:03:48
// Notes:
//	All channels are tuned in exactly the same way
//	Registers with all fields' tunable values matching the reset value omitted
const dcs_tunable_t dcs_tunables[] = {
	{ rDCS_MCU_AMCCLKPWRGATE,        DCS_MASK_VAL(12, 12,    0x0) },
	{ rDCS_MCU_AMCCLKPWRGATE,        DCS_MASK_VAL( 8,  8,    0x1) },

	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(31, 24,   0xFF) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(20, 20,    0x0) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(16, 16,    0x1) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(12, 12,    0x0) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL( 8,  8,    0x1) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL( 4,  4,    0x1) },

	{ rDCS_MCU_PSQRQTIMER0,          DCS_MASK_VAL(27, 16,  0x014) },
	{ rDCS_MCU_PSQRQTIMER0,          DCS_MASK_VAL(11,  0,  0x040) },

	{ rDCS_MCU_PSQRQTIMER1,          DCS_MASK_VAL(27, 16,  0x00F) },
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
	{ rDCS_MCU_PSQWQ_CTL(1),         DCS_MASK_VAL(31, 16, 0x0164) },
	{ rDCS_MCU_PSQWQ_CTL(1),         DCS_MASK_VAL(15,  0, 0x02DA) },

	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL(28, 24,   0x04) },
	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL(20, 16,   0x08) },
	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL(12,  8,   0x04) },
	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL( 4,  0,   0x10) },

	{ rDCS_MCU_PSQWQSCHCRD,          DCS_MASK_VAL(23, 16,   0x10) },
	{ rDCS_MCU_PSQWQSCHCRD,          DCS_MASK_VAL(15,  8,   0x14) },
	{ rDCS_MCU_PSQWQSCHCRD,          DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_QOSLLT,               DCS_MASK_VAL( 8,  8,    0x1) },
	{ rDCS_MCU_QOSLLT,               DCS_MASK_VAL( 4,  0,   0x1F) },

	{ rDCS_MCU_SBQOSUPGCTL,          DCS_MASK_VAL(31, 31,    0x1) },
	{ rDCS_MCU_SBQOSUPGCTL,          DCS_MASK_VAL(15,  8,   0x6F) },
	{ rDCS_MCU_SBQOSUPGCTL,          DCS_MASK_VAL( 7,  0,   0x6E) },

	// DRAM Cfg
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(0), DCS_MASK_VAL(31, 16, 0x0300) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(0), DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_PWRMNGTPARAM_FREQ(1), DCS_MASK_VAL(31, 16, 0x0180) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(1), DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_PWRMNGTPARAM_FREQ(2), DCS_MASK_VAL(31, 16, 0x0180) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(2), DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_PWRMNGTPARAM_FREQ(3), DCS_MASK_VAL(31, 16, 0x0030) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(3), DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_ROWHAM_CTL(0),        DCS_MASK_VAL(31, 16, 0x4002) },
	{ rDCS_MCU_ROWHAM_CTL(0),        DCS_MASK_VAL( 0,  0,    0x0) },

	{ rDCS_MCU_ROWHAM_CTL(1),        DCS_MASK_VAL(31, 16, 0x4A38) },
	{ rDCS_MCU_ROWHAM_CTL(1),        DCS_MASK_VAL(15, 0, 0x22D5) },

	{ rDCS_MCU_ROWHAM_CTL(2),        DCS_MASK_VAL(15, 0, 0x4A33) },

	// AMP
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(0),        DCS_MASK_VAL(31, 16, 0x2) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(0),        DCS_MASK_VAL(15, 0, 0x3) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(1),        DCS_MASK_VAL(31, 16, 0x2) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(1),        DCS_MASK_VAL(15, 0, 0x3) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(2),        DCS_MASK_VAL(31, 16, 0x2) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(2),        DCS_MASK_VAL(15, 0, 0x3) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(3),        DCS_MASK_VAL(31, 16, 0x2) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(3),        DCS_MASK_VAL(15, 0, 0x3) },
	// Alternate End Marker
	// { 0, 0, 0 }
};



#endif
