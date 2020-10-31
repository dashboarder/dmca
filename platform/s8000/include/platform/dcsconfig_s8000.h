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

#ifndef __DCSCONFIG_S8000_H
#define __DCSCONFIG_S8000_H

// Macros for tunables
#define BIT32_MASKFULL			(0xFFFFFFFF)
#define BIT32_MASK(_n)			(BIT32_MASKFULL >> (32 - (_n)))
#define BIT32_MASK_VAL(_h,_l,_d)	(((_d) & BIT32_MASK((_h) - (_l) + 1)) << (_l))
#define BIT32_MASK_HI_LO(_h,_l)		BIT32_MASK_VAL(_h,_l,BIT32_MASKFULL)
#define DCS_MASK_VAL(_h,_l,_d)		BIT32_MASK_HI_LO(_h,_l) , BIT32_MASK_VAL(_h,_l,_d)

// Maui C0 dcs param table
dcs_config_params_t dcs_params = {
	.supported_freqs		= DCS_FREQ(0) | DCS_FREQ(1) | DCS_FREQ(3),
	.num_freqchngctl_regs		= 5,
	.spllctrl_vco_1			= 0x00000076,
	.spllctrl_vco_2			= 0x00000078,
	.dllupdtctrl			= 0x50017350,
	.modereg			= 0x140a90b4,
	.autoref_params			= 0x0015005d,
	.pdn				= 0x62265295,
	.rnkcfg				= 0x00006061,
	.cackcswkds			= 0x000000db,
	.dqspdres			= 0x00000031,
	.casdllupdatectrl		= 0x0303030b,
	.dqsdllupdatectrl		= 0x0003000b,
	.rdsdllctrl_step2		= 0x00ff0004,
	.wrdqdqssdllctrl_step2		= 0xff000004,
	.cawrlvlsdllcode		= 0x00ff02ff,
	.dlllocktim			= 0x012c012c,
	.dficaltiming			= 0x04000410,
	.rdwrdqcaltiming_f0		= 0x00001426,
	.rdwrdqcalseglen_f0		= 0x00010002,
	.rdwrdqcaltiming_f1		= 0x00001422,
	.rdwrdqcalseglen_f1		= 0x00010002,
	.rdwrdqcaltimingctrl1		= 0x0000301e,
	.rdwrdqcaltimingctrl2		= 0x03111004,
	.rddqcalpatprbs4i		= 0x55555e26,
	.wrdqcalpatprbs4i		= 0x55555e26,
	.maxwrdqssdllmulfactor		= 0xa0a0080c,
	.dllupdtctrl1			= 0x50017550,
	.dllupdtintvl			= 0x10200020,
	.arefparam			= 0x08010019,
	.autoref_params2		= 0x0017005d,
	.odtszqc			= 0x00001000,
	.longsr				= 0x01012008,
	.freqchngctl_step3		= 0x00010000,
	.mr3cmd				= 0xf3,
	.mr13cmd			= 0x18,
	.addrcfg			= 0x00030201,
	.chnldec			= 0x00050220,
	.mr13cmd_step7			= 0x58,
	.mr3cmd_step7			= 0xf3,
	.mr22cmd			= 0x04,
	.mr11cmd			= 0x02,
	.mr13cmd_step9			= 0xd8,
	.freqchngctl_step9		= 0x00010000,
	.mr13cmd_step11			= 0x98,
	.mr2cmd_step11			= 0x24,
	.mr1cmd_step11			= 0xce,
	.mr3cmd_step11			= 0xb3,
	.mr22cmd_step11			= 0x04,
	.mr11cmd_step11			= 0x44,
	.mr12cmd_step11			= 0x11,
	.mr14cmd_step11			= 0x11,
	.mr13cmd_step13			= 0x18,
	.mr13cmd_step15			= 0x50,
	.freqchngctl_step15		= 0x00009999,
	.rdsdllctrl_step15		= 0x00150004,
	.odtszqc2			= 0xc0001000,
	.qbren_step16			= 0x0000000d,
	.qbrparam			= 0x000061a5,
	.qbren				= 0x0000000f,
	.mccgen				= 0x00000126,
	.caampclk			= 0x00000000,
	.dqampclk			= 0x00000000,
	.pwrmngten			= 0x00000133,
	.odtszqc3			= 0xc0003320,
	
	.freq			= {
		{	// 1200MHz
			.clk_period      = CLK_PERIOD_1200,
			.freqchngctl     = {0x18cd104d, 0x110c110e, 0xb303440b, 0xce012402, 0x00000416},
			.freqchngtim     = 0x000c1108,
			.lat             = 0x00103306,
			.phyrdwrtim      = 0x00050c0a,
			.caspch          = 0x531a060b,
			.act             = 0x18060d0b,
			.autoref         = 0x366c0078,
			.selfref         = 0x78071012,
			.derate	         = 0x38c486cc,
			.lat2		 = 0x00113306,
			.tat             = 0x01312222,
			.mifqmaxctrl             = 0x00000100,
			.nondqdspd       = 0x000b316b,
			.cavref          = 0x00000003,
			.odt_enable      = 0x00000001,
			.dqds            = 0x000c0b08,
			.dqdqsds         = 0x00030b08,
			.dqvref          = 0x00c000c0,
			.dqdficaltiming  = 0x04020402,
			.rddqcalwindow   = 0x00aa01d1,
			.wrdqcalwindow   = 0x00ca0160,
			.caoutdllscl     = 0x00000008,
			.dqsindllscl     = 0x00000008,
			.rdcapcfg        = 0x0100070d,
			.autoref2        = 0x366c006e,
		},
		{	// 800MHz
			.clk_period      = CLK_PERIOD_800,
			.freqchngctl     = {0x18cd104d, 0x110c110e, 0xf303020b, 0xae015202, 0x00000416},
			.freqchngtim     = 0x000c1108,
			.lat             = 0x00102206,
			.phyrdwrtim      = 0x00050b06,
			.caspch          = 0x42110408,
			.act             = 0x10040908,
			.autoref         = 0x24480050,
			.selfref         = 0x5004b012,
			.derate	         = 0x28835488,
			.lat2		 = 0x00112206,
			.tat             = 0x01312222,
			.nondqdspd       = 0x000b216b,
			.nondqds         = 0x00000000,
			.cavref          = 0x00000003,
			.odt_enable      = 0x00000000,
			.dqds            = 0x00080b00,
			.dqdqsds         = 0x00010b00,
			.dqvref          = 0x00df00df,
			.cadficaltiming  = 0x04000410,
			.dqdficaltiming  = 0x04020402,
			.rddqcalwindow   = 0x00aa01d1,
			.wrdqcalwindow   = 0x00ca0160,
#ifdef TARGET_DDR_800M			
			.caoutdllscl     = 0x0000000c,
			.dqsindllscl     = 0x0000000c,
#else			
			.caoutdllscl     = 0x0000000d,
			.dqsindllscl     = 0x0000000d,
#endif			
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
			.derate	         = 0x0c212142,
			.lat2		 = 0x001110c2,
			.tat             = 0x01212222,
			.nondqdspd       = 0x000b3d6b,
			.nondqds         = 0x00083d08,
			.cavref          = 0x00000003,
			.dqds            = 0x00080b00,
			.dqdqsds         = 0x00080b00,
			.dqvref          = 0x00000000,
			.cadficaltiming  = 0x04000410,
			.dqdficaltiming  = 0x04020402,
			.caoutdllscl     = 0x00000030,
			.dqsindllscl     = 0x00000030,
			.rdcapcfg        = 0x41000406,
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
			.derate	         = 0x10412082,
			.lat2		 = 0x001110c2,
			.tat             = 0x01212222,
			.mifqmaxctrl             = 0x00000001,
			.nondqdspd       = 0x000b3d6b,
			.nondqds         = 0x00083d08,
			.cavref          = 0x00000003,
			.odt_enable      = 0x00000000,
			.dqds            = 0x00080b00,
			.dqdqsds         = 0x00080b00,
			.dqvref          = 0x00000000,
			.cadficaltiming  = 0x04000410,
			.dqdficaltiming  = 0x04020402,
			.caoutdllscl     = 0x0000003f,
			.dqsindllscl     = 0x0000003f,
			.rdcapcfg        = 0x61000406,
			.autoref2         = 0x03050005,
		}
	}
};

// DCS Tunables for s8000 C0
// Tunables Spec versions:
// MCU: https://seg-docs.ecs.apple.com/projects/maui//release/UserManual/tunables_b0/MCU.html dated 2015/06/13 10:26:02
// AMPSCA: https://seg-docs.ecs.apple.com/projects/maui//release/UserManual/tunables_b0/AMPSCA.html dated 2015/05/14 16:66:34
// Notes:
//	All 4 channels are tuned in exactly the same way
//	Registers with all fields' tunable values matching the reset value omitted
const dcs_tunable_t dcs_tunables[] = {
	// AMC Clk
	{ rDCS_MCU_AMCCLKPWRGATE,        DCS_MASK_VAL(12, 12,    0x0) },
	{ rDCS_MCU_AMCCLKPWRGATE,        DCS_MASK_VAL( 8,  8,    0x1) },

	// PSQ RQ (Part I)
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(31, 24,   0xFF) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(20, 20,    0x0) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(16, 16,    0x1) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(12, 12,    0x0) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL( 8,  8,    0x1) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL( 4,  4,    0x1) },

	// PSQ Freq 0
	{ rDCS_MCU_PSQCPU0LLT_FREQ(0),   DCS_MASK_VAL(29, 16, 0x3f4b) },
	{ rDCS_MCU_PSQCPU0LLT_FREQ(0),   DCS_MASK_VAL(12, 12,    0x1) },
	{ rDCS_MCU_PSQCPU0LLT_FREQ(0),   DCS_MASK_VAL( 8,  0,  0x0B4) },

	{ rDCS_MCU_PSQCPU1LLT_FREQ(0),   DCS_MASK_VAL(29, 16, 0x3f4b) },
	{ rDCS_MCU_PSQCPU1LLT_FREQ(0),   DCS_MASK_VAL(12, 12,    0x1) },
	{ rDCS_MCU_PSQCPU1LLT_FREQ(0),   DCS_MASK_VAL( 8,  0,  0x0B4) },

	{ rDCS_MCU_PSQSOCLLT_FREQ(0),    DCS_MASK_VAL(29, 16, 0x3f4b) },
	{ rDCS_MCU_PSQSOCLLT_FREQ(0),    DCS_MASK_VAL(12, 12,    0x1) },
	{ rDCS_MCU_PSQSOCLLT_FREQ(0),    DCS_MASK_VAL( 8,  0,  0x0B4) },

	// PSQ Freq 1
	{ rDCS_MCU_PSQCPU0LLT_FREQ(1),   DCS_MASK_VAL(29, 16, 0x3f87) },
	{ rDCS_MCU_PSQCPU0LLT_FREQ(1),   DCS_MASK_VAL(12, 12,    0x1) },
	{ rDCS_MCU_PSQCPU0LLT_FREQ(1),   DCS_MASK_VAL( 8,  0,  0x078) },

	{ rDCS_MCU_PSQCPU1LLT_FREQ(1),   DCS_MASK_VAL(29, 16, 0x3f87) },
	{ rDCS_MCU_PSQCPU1LLT_FREQ(1),   DCS_MASK_VAL(12, 12,    0x1) },
	{ rDCS_MCU_PSQCPU1LLT_FREQ(1),   DCS_MASK_VAL( 8,  0,  0x078) },

	{ rDCS_MCU_PSQSOCLLT_FREQ(1),    DCS_MASK_VAL(29, 16, 0x3f87) },
	{ rDCS_MCU_PSQSOCLLT_FREQ(1),    DCS_MASK_VAL(12, 12,    0x1) },
	{ rDCS_MCU_PSQSOCLLT_FREQ(1),    DCS_MASK_VAL( 8,  0,  0x078) },

	// PSQ RQ (Part II)
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

	// PSQ WQ
	{ rDCS_MCU_PSQWQ_CTL(0),         DCS_MASK_VAL(16, 16,    0x1) },
	{ rDCS_MCU_PSQWQ_CTL(0),         DCS_MASK_VAL( 8,  8,    0x1) },

	// part of init sequence, so have to apply tunables even if same as reset values
#if DISPLAY_D620_TUNABLES
	{ rDCS_MCU_PSQWQ_CTL(1),         DCS_MASK_VAL(31, 16, 0x0944) },
#else
	{ rDCS_MCU_PSQWQ_CTL(1),         DCS_MASK_VAL(31, 16, 0x0494) },
#endif
	{ rDCS_MCU_PSQWQ_CTL(1),         DCS_MASK_VAL(15,  0, 0x0224) },

	{ rDCS_MCU_PSQWQ_CTL(2),         DCS_MASK_VAL(31, 16, 0x002B) },
	{ rDCS_MCU_PSQWQ_CTL(2),         DCS_MASK_VAL(15,  0, 0x0158) },

	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL(28, 24,   0x04) },
	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL(20, 16,   0x10) },
	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL(12,  8,   0x04) },
	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL( 4,  0,   0x10) },

	{ rDCS_MCU_PSQWQSCHCRD,          DCS_MASK_VAL(23, 16,   0x10) },
	{ rDCS_MCU_PSQWQSCHCRD,          DCS_MASK_VAL(15,  8,   0x14) },
	{ rDCS_MCU_PSQWQSCHCRD,          DCS_MASK_VAL( 7,  0,   0x00) },

	// QoS
	{ rDCS_MCU_QOSLLT,               DCS_MASK_VAL( 8,  8,    0x1) },
	{ rDCS_MCU_QOSLLT,               DCS_MASK_VAL( 4,  0,   0x1F) },

	{ rDCS_MCU_SBQOSUPGCTL,          DCS_MASK_VAL(31, 31,    0x1) },
	{ rDCS_MCU_SBQOSUPGCTL,          DCS_MASK_VAL( 7,  0,   0x6E) },

	// DRAM Cfg
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(0), DCS_MASK_VAL(31, 16, 0x0240) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(0), DCS_MASK_VAL( 7,  0,   0x00) },

#if DISPLAY_D620_TUNABLES
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(1), DCS_MASK_VAL(31, 16, 0x0960) },
#else
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(1), DCS_MASK_VAL(31, 16, 0x04b0) },
#endif
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(1), DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_PWRMNGTPARAM_FREQ(2), DCS_MASK_VAL(31, 16, 0x0180) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(2), DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_PWRMNGTPARAM_FREQ(3), DCS_MASK_VAL(31, 16, 0x0030) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(3), DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_ROWHAM_CTL(0),        DCS_MASK_VAL(31, 16, 0x4002) },
	{ rDCS_MCU_ROWHAM_CTL(0),        DCS_MASK_VAL( 0,  0,    0x0) },

	{ rDCS_MCU_ROWHAM_CTL(1),        DCS_MASK_VAL(31, 16, 0x4a38) },
	{ rDCS_MCU_ROWHAM_CTL(1),        DCS_MASK_VAL(15,  0, 0x22d5) },

	{ rDCS_MCU_ROWHAM_CTL(2),        DCS_MASK_VAL(15, 0, 0x4a33) },

	// AMP
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(0),        DCS_MASK_VAL(31, 16, 0x2) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(0),        DCS_MASK_VAL(15, 0, 0x3) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(1),        DCS_MASK_VAL(31, 16, 0x1) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(1),        DCS_MASK_VAL(15, 0, 0x2) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(2),        DCS_MASK_VAL(31, 16, 0x2) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(2),        DCS_MASK_VAL(15, 0, 0x3) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(3),        DCS_MASK_VAL(31, 16, 0x2) },
	{ rDCS_AMP_RDWRDQCALSEGLEN_F(3),        DCS_MASK_VAL(15, 0, 0x3) },
	// Alternate End Marker
	// { 0, 0, 0 }
};

// Maui B0 dcs param table
dcs_config_params_t dcs_params_b0 = {
	.supported_freqs		= DCS_FREQ(0) | DCS_FREQ(1) | DCS_FREQ(3),
	.num_freqchngctl_regs		= 5,
	.spllctrl_vco_1			= 0x00000076,
	.spllctrl_vco_2			= 0x00000078,
	.rwcfg				= 0x000210ef,
	.dllupdtctrl			= 0x50017350,
	.modereg			= 0x140a90b4,
	.autoref_params			= 0x0015005d,
	.pdn				= 0x62265295,
	.rnkcfg				= 0x00006061,
	.cackcswkds			= 0x000000db,
	.dqspdres			= 0x00000031,
	.casdllupdatectrl		= 0x0303030b,
	.dqsdllupdatectrl		= 0x0003000b,
	.rdsdllctrl_step2		= 0x00ff0004,
	.wrdqdqssdllctrl_step2		= 0xff000004,
	.cawrlvlsdllcode		= 0x00ff02ff,
	.dlllocktim			= 0x012c012c,
	.dficaltiming			= 0x04000410,
	.rdwrdqcaltiming_f0		= 0x00001426,
	.rdwrdqcalseglen_f0		= 0x00010002,
	.rdwrdqcaltiming_f1		= 0x00001422,
	.rdwrdqcalseglen_f1		= 0x00010002,
	.rdwrdqcaltimingctrl1		= 0x0000301e,
	.rdwrdqcaltimingctrl2		= 0x03111004,
	.rddqcalpatprbs4i		= 0x55555e26,
	.wrdqcalpatprbs4i		= 0x55555e26,
	.maxwrdqssdllmulfactor		= 0xa0a0080c,
	.dllupdtctrl1			= 0x50017550,
	.dllupdtintvl			= 0x10200020,
	.arefparam			= 0x0d010019,
	.autoref_params2		= 0x0017005d,
	.odtszqc			= 0x00001000,
	.longsr				= 0x01022008,
	.freqchngctl_step3		= 0x00010000,
	.mr3cmd				= 0xf3,
	.mr13cmd			= 0x18,
	.addrcfg			= 0x00030201,
	.chnldec			= 0x00050220,
	.mr13cmd_step7			= 0x58,
	.mr3cmd_step7			= 0xf3,
	.mr22cmd			= 0x04,
	.mr11cmd			= 0x02,
	.mr13cmd_step9			= 0xd8,
	.freqchngctl_step9		= 0x00010000,
	.mr13cmd_step11			= 0x98,
	.mr2cmd_step11			= 0x24,
	.mr1cmd_step11			= 0xce,
	.mr3cmd_step11			= 0xb3,
	.mr22cmd_step11			= 0x04,
	.mr11cmd_step11			= 0x44,
	.mr12cmd_step11			= 0x11,
	.mr14cmd_step11			= 0x11,
	.mr13cmd_step13			= 0x18,
	.mr13cmd_step15			= 0x50,
	.freqchngctl_step15		= 0x00009999,
	.rdsdllctrl_step15		= 0x00150004,
	.odtszqc2			= 0xc0001000,
	.qbren_step16			= 0x0000000d,
	.qbrparam			= 0x000061a5,
	.qbren				= 0x0000000f,
	.mccgen				= 0x00000126,
	.caampclk			= 0x00000000,
	.dqampclk			= 0x00000000,
	.pwrmngten			= 0x00000133,
	.odtszqc3			= 0xc0003320,
	
	.freq			= {
		{	// 1200MHz
			.clk_period      = CLK_PERIOD_1200,
			.freqchngctl     = {0x18cd104d, 0x110c110e, 0xb303440b, 0xce012402, 0x00000416},
			.freqchngtim     = 0x000c1108,
			.lat             = 0x00103306,
			.phyrdwrtim      = 0x00050c0a,
			.caspch          = 0x531a060b,
			.act             = 0x18060d0b,
			.autoref         = 0x366c0078,
			.selfref         = 0x78071012,
			.derate	         = 0x38c486cc,
			.lat2		 = 0x00113306,
			.tat             = 0x01312222,
			.mifqmaxctrl             = 0x00000100,
			.nondqdspd       = 0x000b316b,
			.cavref          = 0x00000003,
			.odt_enable      = 0x00000001,
			.dqds            = 0x000c0b08,
			.dqdqsds         = 0x00030b08,
			.dqvref          = 0x00c000c0,
			.dqdficaltiming  = 0x04020402,
			.rddqcalwindow   = 0x00aa01d1,
			.wrdqcalwindow   = 0x00ca0160,
			.caoutdllscl     = 0x00000008,
			.dqsindllscl     = 0x00000008,
			.rdcapcfg        = 0x0100070d,
			.autoref2        = 0x366c006e,
		},
		{	// 800MHz
			.clk_period      = CLK_PERIOD_800,
			.freqchngctl     = {0x18cd104d, 0x110c110e, 0xf303020b, 0xae015202, 0x00000416},
			.freqchngtim     = 0x000c1108,
			.lat             = 0x00102206,
			.phyrdwrtim      = 0x00050b06,
			.caspch          = 0x42110408,
			.act             = 0x10040908,
			.autoref         = 0x24480050,
			.selfref         = 0x5004b012,
			.derate	         = 0x28835488,
			.lat2		 = 0x00112206,
			.tat             = 0x01312222,
			.nondqdspd       = 0x000b216b,
			.nondqds         = 0x00000000,
			.cavref          = 0x00000003,
			.odt_enable      = 0x00000000,
			.dqds            = 0x00080b00,
			.dqdqsds         = 0x00010b00,
			.dqvref          = 0x00df00df,
			.cadficaltiming  = 0x04000410,
			.dqdficaltiming  = 0x04020402,
			.rddqcalwindow   = 0x00aa01d1,
			.wrdqcalwindow   = 0x00ca0160,
#ifdef TARGET_DDR_800M			
			.caoutdllscl     = 0x0000000c,
			.dqsindllscl     = 0x0000000c,
#else			
			.caoutdllscl     = 0x0000000d,
			.dqsindllscl     = 0x0000000d,
#endif			
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
			.derate	         = 0x0c212142,
			.lat2		 = 0x001110c2,
			.tat             = 0x01212222,
			.nondqdspd       = 0x000b3d6b,
			.nondqds         = 0x00083d08,
			.cavref          = 0x00000003,
			.dqds            = 0x00080b00,
			.dqdqsds         = 0x00080b00,
			.dqvref          = 0x00000000,
			.cadficaltiming  = 0x04000410,
			.dqdficaltiming  = 0x04020402,
			.caoutdllscl     = 0x00000030,
			.dqsindllscl     = 0x00000030,
			.rdcapcfg        = 0x41000406,
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
			.derate	         = 0x10412082,
			.lat2		 = 0x001110c2,
			.tat             = 0x01212222,
			.mifqmaxctrl             = 0x00000001,
			.nondqdspd       = 0x000b3d6b,
			.nondqds         = 0x00083d08,
			.cavref          = 0x00000003,
			.odt_enable      = 0x00000000,
			.dqds            = 0x00080b00,
			.dqdqsds         = 0x00080b00,
			.dqvref          = 0x00000000,
			.cadficaltiming  = 0x04000410,
			.dqdficaltiming  = 0x04020402,
			.caoutdllscl     = 0x0000003f,
			.dqsindllscl     = 0x0000003f,
			.rdcapcfg        = 0x61000406,
			.autoref2         = 0x03050005,
		}
	}
};

// DCS Tunables for s8000 B0
// Tunables Spec versions:
// MCU: https://seg-docs.ecs.apple.com/projects/maui//release/UserManual/tunables_b0/MCU.html dated 2015/06/09 22:44:29
// AMPSCA: https://seg-docs.ecs.apple.com/projects/maui//release/UserManual/tunables_b0/AMPSCA.html dated 2014/11/07 14:45:21
// Notes:
//	All 4 channels are tuned in exactly the same way
//	Registers with all fields' tunable values matching the reset value omitted
const dcs_tunable_t dcs_tunables_b0[] = {
	// AMC Clk
	{ rDCS_MCU_AMCCLKPWRGATE,        DCS_MASK_VAL(12, 12,    0x0) },
	{ rDCS_MCU_AMCCLKPWRGATE,        DCS_MASK_VAL( 8,  8,    0x1) },

	// PSQ RQ (Part I)
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(31, 24,   0xFF) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(20, 20,    0x0) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(16, 16,    0x1) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL(12, 12,    0x0) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL( 8,  8,    0x0) },
	{ rDCS_MCU_PSQRQCTL0,            DCS_MASK_VAL( 4,  4,    0x0) },

	// PSQ Freq 0
	{ rDCS_MCU_PSQCPU0LLT_FREQ(0),   DCS_MASK_VAL(29, 16,    0x0) },
	{ rDCS_MCU_PSQCPU0LLT_FREQ(0),   DCS_MASK_VAL(12, 12,    0x1) },
	{ rDCS_MCU_PSQCPU0LLT_FREQ(0),   DCS_MASK_VAL( 8,  0,  0x0B4) },

	{ rDCS_MCU_PSQCPU1LLT_FREQ(0),   DCS_MASK_VAL(29, 16,    0x0) },
	{ rDCS_MCU_PSQCPU1LLT_FREQ(0),   DCS_MASK_VAL(12, 12,    0x1) },
	{ rDCS_MCU_PSQCPU1LLT_FREQ(0),   DCS_MASK_VAL( 8,  0,  0x0B4) },

	{ rDCS_MCU_PSQSOCLLT_FREQ(0),    DCS_MASK_VAL(29, 16,    0x0) },
	{ rDCS_MCU_PSQSOCLLT_FREQ(0),    DCS_MASK_VAL(12, 12,    0x1) },
	{ rDCS_MCU_PSQSOCLLT_FREQ(0),    DCS_MASK_VAL( 8,  0,  0x0B4) },

	// PSQ Freq 1
	{ rDCS_MCU_PSQCPU0LLT_FREQ(1),   DCS_MASK_VAL(29, 16,    0x0) },
	{ rDCS_MCU_PSQCPU0LLT_FREQ(1),   DCS_MASK_VAL(12, 12,    0x1) },
	{ rDCS_MCU_PSQCPU0LLT_FREQ(1),   DCS_MASK_VAL( 8,  0,  0x078) },

	{ rDCS_MCU_PSQCPU1LLT_FREQ(1),   DCS_MASK_VAL(29, 16,    0x0) },
	{ rDCS_MCU_PSQCPU1LLT_FREQ(1),   DCS_MASK_VAL(12, 12,    0x1) },
	{ rDCS_MCU_PSQCPU1LLT_FREQ(1),   DCS_MASK_VAL( 8,  0,  0x078) },

	{ rDCS_MCU_PSQSOCLLT_FREQ(1),    DCS_MASK_VAL(29, 16,    0x0) },
	{ rDCS_MCU_PSQSOCLLT_FREQ(1),    DCS_MASK_VAL(12, 12,    0x1) },
	{ rDCS_MCU_PSQSOCLLT_FREQ(1),    DCS_MASK_VAL( 8,  0,  0x078) },

	// PSQ RQ (Part II)
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

	// PSQ WQ
	{ rDCS_MCU_PSQWQ_CTL(0),         DCS_MASK_VAL(16, 16,    0x1) },
	{ rDCS_MCU_PSQWQ_CTL(0),         DCS_MASK_VAL( 8,  8,    0x1) },

	// part of init sequence, so have to apply tunables even if same as reset values
#if DISPLAY_D620_TUNABLES
	{ rDCS_MCU_PSQWQ_CTL(1),         DCS_MASK_VAL(31, 16, 0x0944) },
#else
	{ rDCS_MCU_PSQWQ_CTL(1),         DCS_MASK_VAL(31, 16, 0x0494) },
#endif
	{ rDCS_MCU_PSQWQ_CTL(1),         DCS_MASK_VAL(15,  0, 0x0224) },

	{ rDCS_MCU_PSQWQ_CTL(2),         DCS_MASK_VAL(31, 16, 0x002B) },
	{ rDCS_MCU_PSQWQ_CTL(2),         DCS_MASK_VAL(15,  0, 0x0158) },

	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL(28, 24,   0x04) },
	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL(20, 16,   0x10) },
	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL(12,  8,   0x04) },
	{ rDCS_MCU_PSQWQBRST,            DCS_MASK_VAL( 4,  0,   0x10) },

	{ rDCS_MCU_PSQWQSCHCRD,          DCS_MASK_VAL(23, 16,   0x10) },
	{ rDCS_MCU_PSQWQSCHCRD,          DCS_MASK_VAL(15,  8,   0x14) },
	{ rDCS_MCU_PSQWQSCHCRD,          DCS_MASK_VAL( 7,  0,   0x00) },

	// QoS
	{ rDCS_MCU_QOSLLT,               DCS_MASK_VAL( 8,  8,    0x1) },
	{ rDCS_MCU_QOSLLT,               DCS_MASK_VAL( 4,  0,   0x1F) },

	{ rDCS_MCU_SBQOSUPGCTL,          DCS_MASK_VAL(31, 31,    0x1) },
	{ rDCS_MCU_SBQOSUPGCTL,          DCS_MASK_VAL( 7,  0,   0x6E) },

	// DRAM Cfg
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(0), DCS_MASK_VAL(31, 16, 0x0240) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(0), DCS_MASK_VAL( 7,  0,   0x00) },

#if DISPLAY_D620_TUNABLES
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(1), DCS_MASK_VAL(31, 16, 0x0960) },
#else
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(1), DCS_MASK_VAL(31, 16, 0x04b0) },
#endif
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(1), DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_PWRMNGTPARAM_FREQ(2), DCS_MASK_VAL(31, 16, 0x0180) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(2), DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_PWRMNGTPARAM_FREQ(3), DCS_MASK_VAL(31, 16, 0x0030) },
	{ rDCS_MCU_PWRMNGTPARAM_FREQ(3), DCS_MASK_VAL( 7,  0,   0x00) },

	{ rDCS_MCU_ROWHAM_CTL(0),        DCS_MASK_VAL(31, 16, 0x4002) },
	{ rDCS_MCU_ROWHAM_CTL(0),        DCS_MASK_VAL( 0,  0,    0x0) },

	{ rDCS_MCU_ROWHAM_CTL(1),        DCS_MASK_VAL(31, 16, 0x4a38) },
	{ rDCS_MCU_ROWHAM_CTL(1),        DCS_MASK_VAL(15, 0, 0x22d4) },

	{ rDCS_MCU_ROWHAM_CTL(2),        DCS_MASK_VAL(15, 0, 0x4a33) },
	
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
