/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/*
 * Init Sequence Library for use in:
 * Driver for the LPDDR4 compatible DRAM Control Subsystem (DCS) block
 */

/*
Summary of Steps from the Spec (Spreadsheet)

  + (Aa) Steps 0,1,2,3a     	   [need to identify parameterization]
  + (Ab) Step  3b		   [PMGR: set Freq to 50MHz & delay-wait]
  + (Ac) Steps 3c,4,5   	   [need to identify parameterization]
  + (B) Step   6		   [Change Clk Speed]
  + (C) Step   7		   [include delay-wait from early Step 8]
  + (D) Step   8a		   [Cal: CA  800MHz Vmin]
  + (E) Step   8b		   [Write Regs]
  + (F) Step   8c		   [Cal: CA  800MHz Vmin]   (...again? should it be CS?)
  + (G) Step   8d		   [Write Regs]
  + (H) Step   9		   [Write Regs]
  + (I) Step  10		   [Cal: WrLvl, RdDq, WrDq  800MHz Vmin]
  + (J) Step  11		   [Write Regs]
  + (K) Step  12		   [Cal: CA 1200MHz Vnom]
  + (L) Step  13		   [Write Regs]
  + (M) Step  14		   [Cal: WrLvl, RdDq, WrDq 1200MHz Vmin]
  + (N) Steps 15,16,17,18,19,20    [need to identify parameterization]

Between these steps there is possibly a need/desire for:
- Frequency Changes (PMGR)
- PMU save/restore
- debug messages
- register dumps (DV, SiVal)
- anything else!

CONFIG Table:
    Compile-Time Known:
	Type of Execution Environment	CoreOS, DV, SiVal
	Type of Target			SoC, FPGA, Palladium
	ChipID				0x8000, 0x8001, etc.
	Revision			A0, B0, B1, etc.
	Num DCS				4, 8, etc.			(Derived...!)
	Variant Flags			Work-arounds, RADAR Ref., et al.

	(Config Info know at Compile time can be subject to #if)

    Run-Time Known:
	Type of Init			ColdBoot, AOP_DDR, AOP_AWAKE
	Debug Flags			Low-Level, Milestone, et al.

RegValue Table
	Much like today's:  various values for various registers
				Sensitive to ChipID (also Target type?)
	(Internal static alloc (Complete Table Fill-in within Init function):
		Fill in Values (per ChipID/Rev & Target-type)
		return pointer to caller
			(Allows customization by caller before use)

TODO:
- Convert to using SPDS for register names, et al

*/

//================================================================================
// API Definitions
#include <drivers/dcs/dcs_init_lib.h>

extern dcs_config_t dcs_cfg;
extern dcs_config_params_t dcs_params;

#if izIBOOT
extern int debug_level_dcs;
#endif

// <rdar://problem/23244578>: To track throughout the sequence whether we should apply the Samsung DRAM workarounds
static bool apply_samsung_workaround = false;

void dcs_enable_autorefresh(void)
{
	uint32_t arefen_freq0 = 0x1011013f;
	uint32_t arefen_freq1 = 0x10010000;

	// Configure auto-refresh.  Freq0 has the auto-refresh enable, so do it last.
	dcs_reg_write_all_chan(rDCS_MCU_AREFEN_FREQ(3), 0x10100000);
	dcs_reg_write_all_chan(rDCS_MCU_AREFEN_FREQ(2), 0x10000000);
	dcs_reg_write_all_chan(rDCS_MCU_AREFEN_FREQ(1), arefen_freq1);
	dcs_reg_write_all_chan(rDCS_MCU_AREFEN_FREQ(0), arefen_freq0);
}

//================================================================================
// Implementing the Various Steps of the DCS Init, per Specification
// Based on AMC Init Spec (Spreadsheet)
// - Supporting Maui:  Spec (v??)
//================================================================================
// Step  0: AMC Prolog
//================================================================================
void dcs_init_Prolog(void)
{
	// NOTE: between the Compiler and the Optimization, this should vanish for non-FPGA builds
	if (izFPGA && izColdBoot) {
		dcs_reg_write_all_chan(rDCS_AMP_DRAM_RESETN(AMP_CA), 1);
		delay_for_us(200);
		dcs_reg_write_all_chan(rDCS_AMP_DRAM_RESETN(AMP_CA), 0);
		delay_for_us(200);
		dcs_reg_write_all_chan(rDCS_AMP_DRAM_RESETN(AMP_CA), 1);
		delay_for_us(200);
	}
}

//================================================================================
// Step  1: AMC Initial Configuration
//================================================================================
void dcs_init_AMC_Initial_Config(void)
{
	uint32_t i,n;

// rdar://problem/19137387: this applies to Maui A1 and B0, as well as other SoCs before Cayman

	dcs_reg_write(rAMCC_MCCCHNLDEC_ADDR, dcs_params.chnldec);
#if NUM_AMCCS == 2
	dcs_reg_write(rAMCC1_MCCCHNLDEC_ADDR, dcs_params.chnldec);
#endif

	dcs_reg_write_all_chan(rDCS_SPLLCTRL_CHARGEPUMP, 0x00000068);
	dcs_reg_write_all_chan(rDCS_SPLLCTRL_MODEREG, 0x1);
	dcs_reg_write_all_chan(rDCS_SPLLCTRL_VCO, dcs_params.spllctrl_vco_1);
	dcs_reg_write_all_chan(rDCS_SPLLCTRL_VCO, dcs_params.spllctrl_vco_2);
	dcs_reg_write_all_chan(rDCS_SPLLCTRL_LDO, 0x00000004);

	// This register write to be skipped for Maui A0/A1, but written to for B0 & later, and all other SoCs
	if((dcs_cfg.chipID != 0x8000) || (dcs_cfg.chipID == 0x8000 && dcs_cfg.chipRev >= CHIP_REVISION_B0))
		dcs_reg_write_all_chan(rDCS_SPLLCTRL_SPLLPWRDNCFG, 0x00000011);


	for(i = 0; i < DCS_FREQUENCY_SLOTS; i++) {
		for (n = 0; n < dcs_params.num_freqchngctl_regs; n++)
			dcs_reg_write_all_chan(rDCS_MCU_FREQCHNGCTL_FREQ(n, i), dcs_params.freq[i].freqchngctl[n]);

		// This register write to be skipped for Maui A0/A1, but written to for B0 & later, and all other SoCs
		if((dcs_cfg.chipID != 0x8000) || (dcs_cfg.chipID == 0x8000 && dcs_cfg.chipRev >= CHIP_REVISION_B0))
			dcs_reg_write_all_chan(rDCS_MCU_FREQCHNGTIM_FREQ(i), dcs_params.freq[i].freqchngtim);
	}

	dcs_reg_write_all_chan(rDCS_AMP_WRDQCALVREFCODECONTROL, dcs_params.wrdqcalvrefcodecontrol);
	dcs_reg_write_all_chan(rDCS_AMP_RDDQCALVREFCODECONTROL, dcs_params.rddqcalvrefcodecontrol);


	dcs_reg_write_all_chan(rDCS_AMP_HWRDDQCALVREFCONTROL,          0x01d0b060);
	dcs_reg_write_all_chan(rDCS_AMP_HWRDDQCALVREFOFFSETCONTROL(1), 0x06040200);
	dcs_reg_write_all_chan(rDCS_AMP_HWRDDQCALVREFOFFSETCONTROL(2), 0x00fafcfe);
	dcs_reg_write_all_chan(rDCS_AMP_HWWRDQCALVREFCONTROL,          0x02200060);
	dcs_reg_write_all_chan(rDCS_AMP_HWWRDQCALVREFOFFSETCONTROL(1), 0x06040200);
	dcs_reg_write_all_chan(rDCS_AMP_HWWRDQCALVREFOFFSETCONTROL(2), 0x00fafcfe);
	dcs_reg_write(rGLBTIMER_PERVREFCALCFG,                         0x00000003);
	dcs_reg_write(rGLBTIMER_VREFCNTRL,                             0x00060006);
	dcs_reg_write_all_chan(rDCS_MCU_MODEREG, 0x120c90b8);

	if (dcs_cfg.chipRev  < CHIP_REVISION_B0)
	{
		dcs_reg_write_all_chan(rDCS_MCU_RWCFG, dcs_params.rwcfg);
	}

	for(i = AMP_CA ; i < DCS_AMP_NUM_PHYS; i++)
		dcs_reg_write_all_chan(rDCS_AMP_DLLUPDTCTRL(i), dcs_params.dllupdtctrl);

	dcs_reg_write_all_chan(rDCS_MCU_LAT_FREQ(0), dcs_params.freq[0].lat);
	dcs_reg_write_all_chan(rDCS_MCU_LAT_FREQ(1), dcs_params.freq[1].lat);

	for(i = 0; i < DCS_FREQUENCY_SLOTS; i++)
		dcs_reg_write_all_chan(rDCS_MCU_PHYRDWRTIM_FREQ(i), dcs_params.freq[i].phyrdwrtim);
	for(i = 0; i < DCS_FREQUENCY_SLOTS; i++) {
		dcs_reg_write_all_chan(rDCS_MCU_CASPCH_FREQ(i),  dcs_params.freq[i].caspch);
		dcs_reg_write_all_chan(rDCS_MCU_ACT_FREQ(i),     dcs_params.freq[i].act);
		dcs_reg_write_all_chan(rDCS_MCU_AUTOREF_FREQ(i), dcs_params.freq[i].autoref);
		dcs_reg_write_all_chan(rDCS_MCU_SELFREF_FREQ(i), dcs_params.freq[i].selfref);

		if (i == 0) dcs_reg_write_all_chan(rDCS_MCU_MODEREG, dcs_params.modereg);

	}

	dcs_reg_write_all_chan(rDCS_MCU_AUTOREF_PARAMS, dcs_params.autoref_params);
	dcs_reg_write_all_chan(rDCS_MCU_PDN, dcs_params.pdn);
	dcs_reg_write_all_chan(rDCS_MCU_DERATE_FREQ(0), dcs_params.freq[0].derate);

	// These register writes to be skipped for Maui A0/A1, but written to for B0 & later, and all other SoCs
	if((dcs_cfg.chipID != 0x8000) || (dcs_cfg.chipID == 0x8000 && dcs_cfg.chipRev >= CHIP_REVISION_B0))
		for(i = 1; i < DCS_FREQUENCY_SLOTS; i++)
			dcs_reg_write_all_chan(rDCS_MCU_DERATE_FREQ(i), dcs_params.freq[i].derate);

	for(i = 0; i < DCS_FREQUENCY_SLOTS; i++)
		dcs_reg_write_all_chan(rDCS_MCU_LAT_FREQ(i), dcs_params.freq[i].lat2);
	for(i = 0; i < DCS_FREQUENCY_SLOTS; i++)
		dcs_reg_write_all_chan(rDCS_MCU_TAT_FREQ(i), dcs_params.freq[i].tat);
	dcs_reg_write_all_chan(rDCS_MCU_RNKCFG, dcs_params.rnkcfg);
	dcs_reg_write_all_chan(rDCS_MCU_MIFQMAXCTRL_FREQ(0), dcs_params.freq[0].mifqmaxctrl);
	dcs_reg_write_all_chan(rDCS_MCU_MIFQMAXCTRL_FREQ(3), dcs_params.freq[3].mifqmaxctrl);
	dcs_reg_write_all_chan(rDCS_MCU_PWRMNGTEN, 0);
	dcs_reg_write_all_chan(rDCS_MCU_ODTSZQC, 0x00002000);
	dcs_reg_write_all_chan(rDCS_MCU_AMCCTRL, 0x00000002);
	dcs_reg_write_all_chan(rDCS_MCU_MCPHYUPDTPARAM, 0x15030000);

}

//================================================================================
// Step  2: AMP Initial Configuration
//================================================================================
void dcs_init_AMP_Initial_Config(void)
{
	int i;

	if (dcs_cfg.chipRev < CHIP_REVISION_B0)
	{
		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPCK,	0x00003f3f);
		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPCS,	0x00003f3f);
		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPB0,	0x00003f3f);
		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPDQS0,	0x00003f3f);
		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPB1,	0x00003f3f);
		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPDQS1,	0x00003f3f);
		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPCA,	0x00003f3f);

		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPCK,	0x00013f3f);
		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPCS,	0x00013f3f);
		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPB0,	0x00013f3f);
		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPDQS0,	0x00013f3f);
		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPB1,	0x00013f3f);
		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPDQS1,	0x00013f3f);
		dcs_reg_write_all_chan(rDCS_AMPH_DCC_BYPCA,	0x00013f3f);
	}

	for(i = AMP_CA ; i < DCS_AMP_NUM_PHYS; i++)
		dcs_reg_write_all_chan(rDCS_AMP_AMPEN(i), 0x00000001);

	for(i = 0; i < DCS_FREQUENCY_SLOTS; i++)
		dcs_reg_write_all_chan(rDCS_AMP_VREF_F(AMP_CA, i), dcs_params.freq[i].cavref);

	dcs_reg_write_all_chan(rDCS_AMP_ODTENABLE_F(0), dcs_params.freq[0].odt_enable);
	dcs_reg_write_all_chan(rDCS_AMP_ODTENABLE_F(1), dcs_params.freq[1].odt_enable);
	dcs_reg_write_all_chan(rDCS_AMP_ODTENABLE_F(3), dcs_params.freq[3].odt_enable);

	for(i = 0; i < DCS_FREQUENCY_SLOTS; i++) {
		dcs_reg_write_all_chan(rDCS_AMP_VREF_F(AMP_DQ0, i), dcs_params.freq[i].dqvref);
		dcs_reg_write_all_chan(rDCS_AMP_VREF_F(AMP_DQ1, i), dcs_params.freq[i].dqvref);
	}

	dcs_reg_write_all_chan(rDCS_AMP_SDLL_UPDATE_CNTL(AMP_CA), dcs_params.casdllupdatectrl);
	dcs_reg_write_all_chan(rDCS_AMP_SDLL_UPDATE_CNTL(AMP_DQ0), dcs_params.dqsdllupdatectrl);
	dcs_reg_write_all_chan(rDCS_AMP_SDLL_UPDATE_CNTL(AMP_DQ1), dcs_params.dqsdllupdatectrl);

	dcs_reg_write_all_chan(rDCS_AMP_RDSDLLCTRL(AMP_DQ0), dcs_params.rdsdllctrl_step2);
	dcs_reg_write_all_chan(rDCS_AMP_RDSDLLCTRL(AMP_DQ1), dcs_params.rdsdllctrl_step2);
	dcs_reg_poll_all_chan(rDCS_AMP_RDSDLLCTRL(AMP_DQ0), dcs_params.rdsdllctrl_step2 & 0x7, 0x0);
	dcs_reg_poll_all_chan(rDCS_AMP_RDSDLLCTRL(AMP_DQ1), dcs_params.rdsdllctrl_step2 & 0x7, 0x0);

	dcs_reg_write_all_chan(rDCS_AMP_WRDQDQSSDLLCTRL(AMP_DQ0), dcs_params.wrdqdqssdllctrl_step2);
	dcs_reg_write_all_chan(rDCS_AMP_WRDQDQSSDLLCTRL(AMP_DQ1), dcs_params.wrdqdqssdllctrl_step2);
	dcs_reg_poll_all_chan(rDCS_AMP_WRDQDQSSDLLCTRL(AMP_DQ0), dcs_params.wrdqdqssdllctrl_step2 & 0x7, 0x0);
	dcs_reg_poll_all_chan(rDCS_AMP_WRDQDQSSDLLCTRL(AMP_DQ1), dcs_params.wrdqdqssdllctrl_step2 & 0x7, 0x0);

	dcs_reg_write_all_chan(rDCS_AMP_CAWRLVLSDLLCODE, dcs_params.cawrlvlsdllcode);
	dcs_reg_poll_all_chan(rDCS_AMP_CAWRLVLSDLLCODE, dcs_params.cawrlvlsdllcode & 0x300, 0x000);

	if (!izFPGA)
		for(i = AMP_CA ; i < DCS_AMP_NUM_PHYS; i++)
			dcs_reg_write_all_chan(rDCS_AMP_DLLLOCKTIM(i), dcs_params.dlllocktim);

	if(dcs_cfg.chipID == 0x8000 && dcs_cfg.chipRev >= CHIP_REVISION_B0) {
		dcs_reg_write_all_chan(rDCS_AMP_DQSPDENALWYSON(AMP_DQ0), 0x00000001);
		dcs_reg_write_all_chan(rDCS_AMP_DQSPDENALWYSON(AMP_DQ1), 0x00000001);
	}

	if((dcs_params.supported_freqs & DCS_FREQ(1)) != 0)
		dcs_reg_write_all_chan(rDCS_AMP_DFICALTIMING, dcs_params.dficaltiming);


	if (dcs_cfg.chipRev  < CHIP_REVISION_B0)
	{
		for(i = 1; i < DCS_FREQUENCY_SLOTS; i++)
			dcs_reg_write_all_chan(rDCS_AMPCA_DFICALTIMING_F(i), dcs_params.freq[i].cadficaltiming);
		for(i = 0; i < DCS_FREQUENCY_SLOTS; i++) {

		dcs_reg_write_all_chan(rDCS_AMPDQ_DFICALTIMING_F(AMP_DQ0, i), dcs_params.freq[i].dqdficaltiming);
		dcs_reg_write_all_chan(rDCS_AMPDQ_DFICALTIMING_F(AMP_DQ1, i), dcs_params.freq[i].dqdficaltiming);
	}
	}
	else
	{
		dcs_reg_write_all_chan(rDCS_AMPCA_DFICALTIMING_F(1), 0x04000710);
		dcs_reg_write_all_chan(rDCS_AMPCA_DFICALTIMING_F(2), 0x04000710);
		dcs_reg_write_all_chan(rDCS_AMPCA_DFICALTIMING_F(3), 0x04000510);

		dcs_reg_write_all_chan(rDCS_AMPDQ_DFICALTIMING_F(AMP_DQ0, 0), 0x06020704);
		dcs_reg_write_all_chan(rDCS_AMPDQ_DFICALTIMING_F(AMP_DQ0, 1), 0x04020702);
		dcs_reg_write_all_chan(rDCS_AMPDQ_DFICALTIMING_F(AMP_DQ0, 2), 0x04020702);
		dcs_reg_write_all_chan(rDCS_AMPDQ_DFICALTIMING_F(AMP_DQ0, 3), 0x04020504);

		dcs_reg_write_all_chan(rDCS_AMPDQ_DFICALTIMING_F(AMP_DQ1, 0), 0x06020704);
		dcs_reg_write_all_chan(rDCS_AMPDQ_DFICALTIMING_F(AMP_DQ1, 1), 0x04020702);
		dcs_reg_write_all_chan(rDCS_AMPDQ_DFICALTIMING_F(AMP_DQ1, 2), 0x04020702);
		dcs_reg_write_all_chan(rDCS_AMPDQ_DFICALTIMING_F(AMP_DQ1, 3), 0x04020504);
		dcs_reg_write_all_chan(rDCS_AMPCA_DFICALTIMING_RDDQCAL2, 0x00000019);
	}


	dcs_reg_write_all_chan(rDCS_AMP_MDLLCODE_CAP_CNTL(AMP_DQ0), 0x00000002);
	dcs_reg_write_all_chan(rDCS_AMP_MDLLCODE_CAP_CNTL(AMP_DQ1), 0x00000002);



	dcs_reg_write_all_chan(rDCS_AMP_RDWRDQCALTIMING_F(0), dcs_params.rdwrdqcaltiming_f0);

	if (dcs_cfg.chipRev < CHIP_REVISION_B0)
		dcs_reg_write_all_chan(rDCS_AMP_RDWRDQCALSEGLEN_F(0), dcs_params.rdwrdqcalseglen_f0);
	else
		dcs_reg_write_all_chan(rDCS_AMP_RDWRDQCALSEGLEN_F(0), 0x00040008);

	dcs_reg_write_all_chan(rDCS_AMP_RDWRDQCALTIMING_F(1), dcs_params.rdwrdqcaltiming_f1);
	dcs_reg_write_all_chan(rDCS_AMP_RDWRDQCALSEGLEN_F(1), dcs_params.rdwrdqcalseglen_f1);
	dcs_reg_write_all_chan(rDCS_AMP_RDWRDQCALTIMING_F(3), dcs_params.rdwrdqcaltiming_f3);
	dcs_reg_write_all_chan(rDCS_AMP_HWRDWRDQCALTIMINGCTRL(1), dcs_params.rdwrdqcaltimingctrl1);
	dcs_reg_write_all_chan(rDCS_AMP_HWRDWRDQCALTIMINGCTRL(2), dcs_params.rdwrdqcaltimingctrl2);


	dcs_reg_write_all_chan(rDCS_AMP_HWRDDQCALPATPRBS4I, dcs_params.rddqcalpatprbs4i);
	dcs_reg_write_all_chan(rDCS_AMP_HWWRDQCALPATPRBS4I, dcs_params.wrdqcalpatprbs4i);
	dcs_reg_write_all_chan(rDCS_AMP_HWWRDQCALPATPRBS7(0), 0x87654321);
	dcs_reg_write_all_chan(rDCS_AMP_HWWRDQCALPATPRBS7(1), 0xcdedcba9);
	dcs_reg_write_all_chan(rDCS_AMP_HWWRDQCALPATPRBS7(2), 0x456789ab);
	dcs_reg_write_all_chan(rDCS_AMP_HWWRDQCALPATPRBS7(3), 0x5fa63123);
	dcs_reg_write_all_chan(rDCS_AMP_HWWRDQCALPATINVERTMASK, 0x55550000);


	dcs_reg_write_all_chan(rDCS_AMP_RDDQCALWINDOW_F(0), dcs_params.freq[0].rddqcalwindow);
	dcs_reg_write_all_chan(rDCS_AMP_WRDQCALWINDOW_F(0), dcs_params.freq[0].wrdqcalwindow);
	dcs_reg_write_all_chan(rDCS_AMP_RDDQCALWINDOW_F(1), dcs_params.freq[1].rddqcalwindow);
	dcs_reg_write_all_chan(rDCS_AMP_WRDQCALWINDOW_F(1), dcs_params.freq[1].wrdqcalwindow);
	dcs_reg_write_all_chan(rDCS_AMP_MAXRDDQS_SDLL_MULFACTOR, dcs_params.maxrddqssdllmulfactor);
	dcs_reg_write_all_chan(rDCS_AMP_MAXWRDQS_SDLL_MULFACTOR, dcs_params.maxwrdqssdllmulfactor);

	// These register writes to be skipped for Maui A0/A1
	if((dcs_cfg.chipID != 0x8000) || (dcs_cfg.chipID == 0x8000 && dcs_cfg.chipRev >= CHIP_REVISION_B0)) {
		dcs_reg_write_all_chan(rDCS_AMPDQ_RDDQSMULFACTOR(AMP_DQ0), 0x20181000);
		dcs_reg_write_all_chan(rDCS_AMPDQ_RDDQSMULFACTOR(AMP_DQ1), 0x20181000);
	}

	for(i = 0; i < DCS_FREQUENCY_SLOTS; i++) {
		dcs_reg_write_all_chan(rDCS_AMP_RDCAPCFG_FREQ(AMP_DQ0, i), dcs_params.freq[i].rdcapcfg);
		dcs_reg_write_all_chan(rDCS_AMP_RDCAPCFG_FREQ(AMP_DQ1, i), dcs_params.freq[i].rdcapcfg);
	}

	if (!izFPGA) {
		for(i = AMP_CA ; i < DCS_AMP_NUM_PHYS; i++)
			dcs_reg_write_all_chan(rDCS_AMP_DLLUPDTCTRL(i), dcs_params.dllupdtctrl1);
	}
	dcs_reg_write_all_chan(rDCS_AMP_DLLUPDTINTVL(AMP_CA), dcs_params.dllupdtintvl);
	dcs_reg_write_all_chan(rDCS_AMP_DLLUPDTINTVL(AMP_DQ0), dcs_params.dllupdtintvl);
	dcs_reg_write_all_chan(rDCS_AMP_DLLUPDTINTVL(AMP_DQ1), dcs_params.dllupdtintvl);

	for(i = AMP_DQ0 ; i < DCS_AMP_NUM_PHYS; i++)
		dcs_reg_write_all_chan(rDCS_AMP_DLLEN(i), 0x00000100);

	dcs_reg_write_all_chan(rDCS_AMP_DCCCONTROL, dcs_params.dcccontrol);
	dcs_reg_write_all_chan(rDCS_AMP_DCCTIMER, 0x00000190);
	dcs_reg_write_all_chan(rDCS_AMP_HWWRDQCALDYNHALFCLKDLYCTRL, 0x00000001);


// AMPH configuration

	dcs_reg_write_all_chan(rDCS_AMPH_CB_WKPUPD,       0);
	dcs_reg_write_all_chan(rDCS_AMPH_CB_DRIVESTR,     dcs_params.cbdrivestr);
	dcs_reg_write_all_chan(rDCS_AMPH_CB_IOCTL,        dcs_params.cbioctl);
	dcs_reg_write_all_chan(rDCS_AMPH_CK_WKPUPD,       0);
	dcs_reg_write_all_chan(rDCS_AMPH_CK_ZDETBIASEN,   0);
	dcs_reg_write_all_chan(rDCS_AMPH_CK_DRIVESTR,     dcs_params.ckdrivestr);
	dcs_reg_write_all_chan(rDCS_AMPH_CK_IOCTL,        dcs_params.ckioctl);
	dcs_reg_write_all_chan(rDCS_AMPH_B0_DRIVESTR,     dcs_params.b0drivestr);
	dcs_reg_write_all_chan(rDCS_AMPH_B0_WKPUPD,       0);
	dcs_reg_write_all_chan(rDCS_AMPH_B0_IOCTL,        dcs_params.b0ioctl);
	dcs_reg_write_all_chan(rDCS_AMPH_B0_ODT,          dcs_params.b0odt);
	dcs_reg_write_all_chan(rDCS_AMPH_B0_ODTCTRL,      dcs_params.b0odtctrl);
	dcs_reg_write_all_chan(rDCS_AMPH_B1_DRIVESTR,     dcs_params.b1drivestr);
	dcs_reg_write_all_chan(rDCS_AMPH_B1_ODTCTRL,      dcs_params.b1odtctrl);
	dcs_reg_write_all_chan(rDCS_AMPH_B1_WKPUPD,       0);
	dcs_reg_write_all_chan(rDCS_AMPH_B1_IOCTL,        dcs_params.b1ioctl);
	dcs_reg_write_all_chan(rDCS_AMPH_B1_ODT,          dcs_params.b1odt);
	dcs_reg_write_all_chan(rDCS_AMPH_DQS0_WKPUPD,	  0x00000782);
	dcs_reg_write_all_chan(rDCS_AMPH_DQS0_DRIVESTR,   dcs_params.dqs0drivestr);
	dcs_reg_write_all_chan(rDCS_AMPH_DQS0_IOCTL,      dcs_params.dqs0ioctl);
	dcs_reg_write_all_chan(rDCS_AMPH_DQS0_ODT,        dcs_params.dqs0odt);
	dcs_reg_write_all_chan(rDCS_AMPH_DQS0_ZDETBIASEN, dcs_params.dqs0zdetbiasen);
	dcs_reg_write_all_chan(rDCS_AMPH_DQS0_ODTCTRL,    dcs_params.dqs0odtctrl);
	dcs_reg_write_all_chan(rDCS_AMPH_DQS1_WKPUPD,	  0x00000782);
	dcs_reg_write_all_chan(rDCS_AMPH_DQS1_DRIVESTR,   dcs_params.dqs1drivestr);
	dcs_reg_write_all_chan(rDCS_AMPH_DQS1_IOCTL,      dcs_params.dqs1ioctl);
	dcs_reg_write_all_chan(rDCS_AMPH_DQS1_ODT,        dcs_params.dqs1odt);
	dcs_reg_write_all_chan(rDCS_AMPH_DQS1_ZDETBIASEN, dcs_params.dqs1zdetbiasen);
	dcs_reg_write_all_chan(rDCS_AMPH_DQS1_ODTCTRL,    dcs_params.dqs1odtctrl);

	dcs_reg_write_all_chan(rDCS_AMPH_DBG_REG0,        0x00000000);
	dcs_reg_write_all_chan(rDCS_AMPH_ZCAL_FSM(1),     dcs_params.zcalfsm1);
	dcs_reg_write_all_chan(rDCS_AMPH_ZCAL_FSM(0),     dcs_params.zcalfsm0);

	if (dcs_cfg.chipRev < CHIP_REVISION_B0)
		dcs_reg_write_all_chan(rDCS_AMPH_SPARE(0),        dcs_params.spare0);
	else
		dcs_reg_write_all_chan(rDCS_AMPH_SPARE(0),        0x00000816);

	for(i = AMP_CA ; i < DCS_AMP_NUM_PHYS; i++)
		dcs_reg_write_all_chan(rDCS_AMP_AMPINIT(i), 0x00000001);

	if (dcs_cfg.chipRev < CHIP_REVISION_B0)
		dcs_reg_write_all_chan(rDCS_AMP_DFICALTIMING, dcs_params.dficaltiming);
	else
	{
		dcs_reg_write_all_chan(rDCS_AMP_DFICALTIMING, 0x06001604);
		dcs_reg_write_all_chan(rDCS_AMPDQ_DFICALTIMING_F(AMP_DQ0, 0), 0x06021604);
		dcs_reg_write_all_chan(rDCS_AMPDQ_DFICALTIMING_F(AMP_DQ1, 0), 0x06021604);
	}

	dcs_reg_write_all_chan(rDCS_AMP_HWRDDQCALTVREF, dcs_params.hwrddqcaltvref);

}

//================================================================================
// Step  3: Self-Refresh Exit
//================================================================================
void dcs_init_Self_Refresh_Exit_a(void)
{
	// Wait 5 us after Impedence Calibration to avoid McPhyPending
	// preventing the SRFSM from exiting SR.
	dcs_spin(5, "after Imp. Cal.");

	dcs_reg_write_all_chan(rDCS_MCU_AREFPARAM, dcs_params.arefparam);

	for(int i = 0; i < DCS_FREQUENCY_SLOTS; i++)
		dcs_reg_write_all_chan(rDCS_MCU_AUTOREF_FREQ(i), dcs_params.freq[i].autoref2);

	dcs_reg_write_all_chan(rDCS_MCU_AUTOREF_PARAMS, dcs_params.autoref_params2);

	dcs_reg_write_all_chan(rDCS_MCU_ODTSZQC, dcs_params.odtszqc);
	// M8 Bug radar #20346417, two writes to ensure propogation
	dcs_reg_write_all_chan(rDCS_MCU_LONGSR, dcs_params.longsr);
	dcs_reg_write_all_chan(rDCS_MCU_LONGSR, dcs_params.longsr);
	dcs_reg_write_all_chan(rDCS_MCU_MCPHYUPDTPARAM, 0x15030004);

}

//========================================
// Caller Code should:
// - Change Frequency to 50MHz
//========================================

void dcs_init_Self_Refresh_Exit_b(void)
{
	dcs_reg_write_all_chan(rDCS_MCU_AMCCTRL, 0x00000003);


	if (!izFPGA) {
		dcs_reg_write_all_chan(rDCS_AMP_IMPCALCMD, 0x00000001);
		dcs_reg_poll_all_chan(rDCS_AMP_IMPCALCMD, 0x01, 0x00);
	}

	if (izResume)
		dcs_enable_autorefresh();

	dcs_spin(2200, "After Enable Auto-Refresh");

	if((dcs_cfg.chipID != 0x8000) || (dcs_cfg.chipID == 0x8000 && dcs_cfg.chipRev >= CHIP_REVISION_B0)) {
		dcs_reg_write_all_chan(rDCS_MCU_FREQCHNGCTL, dcs_params.freqchngctl_step3);
		dcs_reg_poll_all_chan(rDCS_MCU_FREQCHNGCTL, 0x10000, 0x00000);
		dcs_spin(2, "After SoC Update");
	}

	// Resume-Boot and Cold-Boot use different Exit commands for Self-Refresh
	// Issue self-refresh exit cmd for resume boot with MPC
	int n = (izColdBoot)? 0x00000000:0x00004000;
	// Issue self-refresh exit cmd for resume boot with MPC
	dcs_reg_write_all_chan(rDCS_MCU_MRINITCMD_0, n);
	dcs_reg_write_all_chan(rDCS_MCU_MRINITCMD_0, n | 0x1);
	dcs_reg_poll_all_chan(rDCS_MCU_MRINITCMD_0, 0x01, 0x00);

	dcs_spin(2, NULL);

	dcs_reg_write(rGLBTIMER_CHEN, (1 << dcs_cfg.num_dcs) - 1);
}

//================================================================================
// Step  4: DRAM Reset, ZQ Calibration & Configuration (cold boot only)
//================================================================================
void dcs_init_ZQ_Cal_Cfg(void)
{
	// ZQ calibration START MPC cmd
	dcs_mrcmd(MR_MPC, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x0, 0x4f);
	// wait 1us for tZQCAL
	dcs_spin(1, "tZQCAL");

	// ZQ calibration LATCH MPC cmd
	dcs_mrcmd(MR_MPC, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x0, 0x51);
	// wait 10ns for tZQLAT
	dcs_spin(1, "tZQLAT");

	if (izColdBoot) {
		// MR2 register (read/write latency)
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x2, 0x0);
		// MR1 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x1, 0x8e);
		// MR3 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x3, dcs_params.mr3cmd);
		// MR22 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x16, 0x0);
		// MR11 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0xb, 0x0);
		// MR13 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0xd, dcs_params.mr13cmd);
		// MR12 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0xc, 0x59);
		// MR14 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0xe, 0x59);

		// MR23 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x17, 0x80);

		uint32_t patinvertmask = (DCS_REG_READ_CH(0, rDCS_AMP_HWRDDQCALPATPRBS4I) >> 16) & 0xffff;
		uint32_t prbs = DCS_REG_READ_CH(0, rDCS_AMP_HWWRDQCALPATPRBS4I);

		// MR15 register
		dcs_mrcmd(MR_MPC, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0xf, patinvertmask & 0xff);
		// MR20 register
		dcs_mrcmd(MR_MPC, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x14, (patinvertmask >> 8) & 0xff);
		// MR32 register
		dcs_mrcmd(MR_MPC, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x20, prbs & 0xff);
		// MR40 register
		dcs_mrcmd(MR_MPC, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x28, (prbs >> 8) & 0xff);
	}
}

//================================================================================
// Step  5a: AddrConfig Configuration
//================================================================================
void dcs_init_AddrCfg_Cfg(void)
{

	dcs_reg_write(rAMCC_ADDRCFG_ADDR, dcs_params.addrcfg);
#if NUM_AMCCS == 2
	dcs_reg_write(rAMCC1_ADDRCFG_ADDR, dcs_params.addrcfg);
#endif
}

//========================================
// Caller Code should:
// - Use MR Cmds to Query the Device
//========================================

//================================================================================
// Step  5b: Total Mem Size Config
//================================================================================
void dcs_init_TotalSize_Cfg(uint32_t total_size_Mbytes)
{
	dbgprintf(DCS_DBG_ASSESS_CONFIG, "Memory_size: %llu bytes\n",
					(((uint64_t)total_size_Mbytes) * MEG * NUM_AMCCS));
	// (Units of 128 MBs in the register)
	dcs_reg_write(rAMCC_DRAMACCCTRL_ADDR, ((total_size_Mbytes / NUM_AMCCS) / DCS_NUM_MEG_IN_DRAM_BLK) - 1);
	dcs_reg_write(rAMCC_MCCCHNLDEC_ADDR, dcs_params.chnldec);

#if NUM_AMCCS == 2
	dcs_reg_write(rAMCC1_DRAMACCCTRL_ADDR, ((total_size_Mbytes / NUM_AMCCS) / DCS_NUM_MEG_IN_DRAM_BLK) - 1);
	dcs_reg_write(rAMCC1_MCCCHNLDEC_ADDR, dcs_params.chnldec);
#endif

	dbgprintf(DCS_DBG_ASSESS_CONFIG, "rAMCC_DRAMACCCTRL: 0x%08x\n", rAMCC_DRAMACCCTRL);

	const struct dcs_memory_device_info *dev_info = dcs_get_memory_device_info();
	if((dev_info->vendor_id == JEDEC_LPDDR4_MANUF_ID_SAMSUNG) && (dev_info->rev_id == 5) && (dev_info->rev_id2 <= 1))
		apply_samsung_workaround = true;

	if(apply_samsung_workaround) {
		dcs_reg_write_all_chan(rDCS_AMP_VREF_F(AMP_DQ0, 0), 0x00d800d8);
		dcs_reg_write_all_chan(rDCS_AMP_VREF_F(AMP_DQ1, 0), 0x00d800d8);

		dcs_reg_write_all_chan(rDCS_AMP_VREF_F(AMP_DQ0, 1), 0x00d800d8);
		dcs_reg_write_all_chan(rDCS_AMP_VREF_F(AMP_DQ1, 1), 0x00d800d8);

		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x3, 0xf2);
		dcs_mrcmd(MR_MPC, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x0, 0x4f);

		dcs_spin(1, " For tZQCAL");
		dcs_mrcmd(MR_MPC, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x0, 0x51);


		dcs_reg_write_all_chan(rDCS_MCU_FREQCHNGCTL_FREQ(2, 0), 0xb203330b);
		dcs_reg_write_all_chan(rDCS_MCU_FREQCHNGCTL_FREQ(4, 0), 0x00000316);
		dcs_reg_write_all_chan(rDCS_MCU_FREQCHNGCTL_FREQ(2, 3), 0xf203000b);

		dcs_reg_write_all_chan(rDCS_AMPH_CB_IMPCTL, 0x01001c2b);
		dcs_reg_write_all_chan(rDCS_AMPH_B0_ODT, 0x01c00339);
		dcs_reg_write_all_chan(rDCS_AMPH_B1_ODT, 0x01c00339);
		dcs_reg_write_all_chan(rDCS_AMPH_DQS0_ODT, 0x01c00339);
		dcs_reg_write_all_chan(rDCS_AMPH_DQS1_ODT, 0x01c00339);
	}
}

//================================================================================
// Step  6: Switch from boot-clock speed to normal operation speed
//================================================================================
void dcs_init_Reenable_AMC_Scheduler(void)
{
	// Wait 5 us before freq change to make sure all refreshes have been flushed
	dcs_spin(5, "After Refresh");

	// Enable (again) the AMC Scheduler
	dcs_reg_write_all_chan(rDCS_MCU_AMCCTRL, 0x00000003);
}

//================================================================================
// Step  7: Setup registers for CA calibration for bucket 1
//================================================================================
void dcs_init_CA_Cal_Setup_Freq1(void)
{
	// MR13 register
	dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0xd, dcs_params.mr13cmd_step7);

	if (izColdBoot) {
		if (izFPGA) {
			// MR11 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0xb, 0x00);
		} else {
			// MR2 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x2, 0x52);
			// MR1 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x1, 0xae);
			// MR3 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x3, dcs_params.mr3cmd_step7);
			// MR22 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x16, dcs_params.mr22cmd);
			// MR11 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0xb, dcs_params.mr11cmd);
			// MR12 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0xc, 0x11);
			// MR14 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0xe, 0x11);
		}
		dcs_reg_write_all_chan(rDCS_AMP_CAWRLVLSDLLCODE, 0x00000200);
		dcs_reg_poll_all_chan(rDCS_AMP_CAWRLVLSDLLCODE, 0x0200, 0x0000);
	}
}

//================================================================================
// Step  8: AMP Dynamic CA Vref Timing Calibration at Vmin 800MHz
//================================================================================
void dcs_init_CA_Cal_Freq1(void)
{
	// Nothing to be done at this step for cold boot
}

//================================================================================
// Step  9: Setup registers for DQ calibration for bucket 1
//================================================================================
void dcs_init_DQ_Cal_Setup_Freq1_a(void)
{
	int i;

	if (izColdBoot) {
		// MR13 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0xd, dcs_params.mr13cmd_step9);
	}

	dcs_spin(1, NULL);

	if(izResume) {
		dcs_reg_write_all_chan(rDCS_MCU_PWRMNGTEN, 0x00000002);
		dcs_reg_write_all_chan(rDCS_MCU_FREQCHNGCTL, 0x01008888);

		for(i = AMP_DQ0 ; i < DCS_AMP_NUM_PHYS; i++)
			dcs_reg_write_all_chan(rDCS_AMP_WRDQDQSSDLLCTRL(i), 0xFF00000C);
		for(i = AMP_CA ; i < DCS_AMP_NUM_PHYS; i++)
			dcs_reg_write_all_chan(rDCS_AMP_SDLL_UPDATE_DEFER_EN(i), 0x00000000);
		for(i = AMP_DQ0; i < DCS_AMP_NUM_PHYS; i++)
			dcs_reg_write_all_chan(rDCS_AMP_MDLLOVRRD(i),  0x00000000);
	}
}

// Caller code should change frequency to bucket 1
void dcs_init_DQ_Cal_Setup_Freq1_b(void)
{
	if (izColdBoot) {
		if((dcs_cfg.chipID != 0x8000) || (dcs_cfg.chipID == 0x8000 && dcs_cfg.chipRev >= CHIP_REVISION_B0)) {
			dcs_reg_write_all_chan(rDCS_MCU_FREQCHNGCTL, dcs_params.freqchngctl_step9);
			dcs_reg_poll_all_chan(rDCS_MCU_FREQCHNGCTL, 0x10000, 0x00000);

			// Wait 2us for the soc update to finish
			dcs_spin(2, NULL);
		}
	}
}

//================================================================================
// Step 10: PHY write DQ calibration
//================================================================================
void dcs_init_wrdq_skew(void)
{
	dcs_reg_write_all_chan(rDCS_AMP_DQSDQ_SKEWCTL(AMP_DQ0), 0x06000000);
	dcs_reg_write_all_chan(rDCS_AMP_DQSDQ_SKEWCTL(AMP_DQ1), 0x06000000);
}



//================================================================================
// Step 11: Setup registers for CA calibration for bucket 0
//================================================================================
void dcs_init_CA_Cal_Setup_Freq0(void)
{
	if (izColdBoot) {

		if(dcs_cfg.chipID == 0x8000 && dcs_cfg.chipRev >= CHIP_REVISION_B0) {
			dcs_reg_write_all_chan(rDCS_AMP_DQFLTCTRL(AMP_DQ0), 0x00000000);
			dcs_reg_write_all_chan(rDCS_AMP_DQFLTCTRL(AMP_DQ1), 0x00000000);
		}

		// MR13 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x0d, dcs_params.mr13cmd_step11);
		if (!izFPGA) {
			// MR2 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x02, dcs_params.mr2cmd_step11);
			// MR1 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x01, dcs_params.mr1cmd_step11);
			// MR3 register
			if(apply_samsung_workaround)
				dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x03, 0xb2);
			else
				dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x03, dcs_params.mr3cmd_step11);
		}

		if(apply_samsung_workaround) {
			// MR22 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x16, 0x03);
			// MR11 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x0b, 0x33);
		} else {
			// MR22 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x16, dcs_params.mr22cmd_step11);
			// MR11 register
			dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x0b, (izFPGA) ? 0x00 : dcs_params.mr11cmd_step11);
		}

		// MR12 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x0c, dcs_params.mr12cmd_step11);
		// MR14 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x0e, dcs_params.mr14cmd_step11);

		if((dcs_params.supported_freqs & DCS_FREQ(1)) == 0) {
			dcs_reg_write_all_chan(rDCS_AMP_CAWRLVLSDLLCODE, 0x00000200);
			dcs_reg_poll_all_chan(rDCS_AMP_CAWRLVLSDLLCODE, 0x200, 0x000);
		}
	}
}

//================================================================================
// Step 12: AMP Dynamic CA Timing Calibration
//================================================================================

//================================================================================
// Step 13: Setup registers for DQ calibration for bucket 0
//================================================================================
void dcs_init_DQ_Cal_Setup_Freq0_a(void)
{
	if (izColdBoot) {
		// MR13 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x0d, dcs_params.mr13cmd_step13);

		// Wait 1 us
		dcs_spin(1, NULL);
	}
}

void dcs_init_DQ_Cal_Setup_Freq0_b(void)
{
	if (izColdBoot) {
		if((dcs_cfg.chipID != 0x8000) || (dcs_cfg.chipID == 0x8000 && dcs_cfg.chipRev >= CHIP_REVISION_B0)) {
			dcs_reg_write_all_chan(rDCS_MCU_FREQCHNGCTL, 0x00010000);
			dcs_reg_poll_all_chan(rDCS_MCU_FREQCHNGCTL, 0x10000, 0x00000);

			// Wait 2us for the soc update to finish
			dcs_spin(2, NULL);
		}
	}
}

//================================================================================
// Step 14: AMP Dynamic DQ Calibration
//================================================================================
void dcs_init_post_wrlvl(void)
{
	if(dcs_cfg.chipID == 0x8000 && dcs_cfg.chipRev >= CHIP_REVISION_B0) {
		dcs_reg_write_all_chan(rDCS_AMP_DQFLTCTRL(AMP_DQ0), 0x00000010);
		dcs_reg_write_all_chan(rDCS_AMP_DQFLTCTRL(AMP_DQ1), 0x00000010);
	}
	if (!izFPGA)
	{

		dcs_reg_write_all_chan(rDCS_AMPH_DQS0_WKPUPD, dcs_params.dqs0wkpupd);
		dcs_reg_write_all_chan(rDCS_AMPH_DQS1_WKPUPD, dcs_params.dqs1wkpupd);
	}
}
//================================================================================
// Step 15: Setup Registers for Boot
//================================================================================
void dcs_init_Reg_for_Boot(void)
{
	if (dcs_cfg.chipRev >= CHIP_REVISION_B0)
		dcs_reg_write_all_chan(rDCS_AMPH_SPARE(0),        0xc16);

#ifndef DCS_RUN_AT_50MHZ
	if (izColdBoot) {
		// MR13 register
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x0d, dcs_params.mr13cmd_step15);
	}
#endif

	if(izFPGA && !(dcs_params.supported_freqs & DCS_FREQ(1)))
		dcs_mrcmd(MR_WRITE, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x0d, 0x50);

	// Wait 1 us
	dcs_spin(1, NULL);

	if (izColdBoot) {
		dcs_reg_write_all_chan(rDCS_MCU_FREQCHNGCTL, dcs_params.freqchngctl_step15);
		dcs_reg_write_all_chan(rDCS_AMP_WRDQDQSSDLLCTRL(AMP_DQ0), 0xff000008);
		dcs_reg_write_all_chan(rDCS_AMP_WRDQDQSSDLLCTRL(AMP_DQ1), 0xff000008);


#if !DCS_DO_CALIBRATION
		// skip these for all silicon targets, where calibration is performed, also skip for any Maui A1 target
		if((dcs_cfg.chipID != 0x8000) || (dcs_cfg.chipID == 0x8000 && dcs_cfg.chipRev >= CHIP_REVISION_B0)) {

			dcs_reg_write_all_chan(rDCS_AMP_RDDQSDLL_DLYSEL(AMP_DQ0), 0x00000000);
			dcs_reg_write_all_chan(rDCS_AMP_RDDQSDLL_DLYSEL(AMP_DQ1), 0x00000000);
			dcs_reg_write_all_chan(rDCS_AMP_RDSDLLCTRL(AMP_DQ0), dcs_params.rdsdllctrl_step15);
			dcs_reg_write_all_chan(rDCS_AMP_RDSDLLCTRL(AMP_DQ1), dcs_params.rdsdllctrl_step15);
			dcs_reg_poll_all_chan(rDCS_AMP_RDSDLLCTRL(AMP_DQ0), dcs_params.rdsdllctrl_step15 & 0x7, 0x0);
			dcs_reg_poll_all_chan(rDCS_AMP_RDSDLLCTRL(AMP_DQ1), dcs_params.rdsdllctrl_step15 & 0x7, 0x0);
		}
#endif
	}


	if(!izFPGA)
	{
		dcs_reg_write_all_chan(rDCS_AMP_HWRDWRDQCALFULLSCANEN, 0x00000000);
		dcs_reg_write_all_chan(rDCS_AMP_HWRDWRDQCALFULLSCANEN, 0x00000003);
	}

	dcs_reg_write_all_chan(rDCS_AMP_HWRDDQCALVREFCONTROL, 0x01D0B061);
	dcs_reg_write_all_chan(rDCS_AMP_HWWRDQCALVREFCONTROL, 0x02200061);

}

//================================================================================
// Step 16: Enable Other Features
//================================================================================
void dcs_init_MoreFeatures(void)
{
	dcs_reg_write_all_chan(rDCS_MCU_AREFPARAM, dcs_params.arefparam);
	dcs_reg_write_all_chan(rDCS_MCU_ODTSZQC, dcs_params.odtszqc2);
	dcs_reg_write_all_chan(rDCS_MCU_QBREN, dcs_params.qbren_step16);
	if (izColdBoot)
		dcs_enable_autorefresh();
	dcs_reg_write_all_chan(rDCS_AMPH_B0_DYN_ISEL_ASRTIME, dcs_params.b0dyniselasrtime);
	dcs_reg_write_all_chan(rDCS_AMPH_B0_DYN_ISEL, dcs_params.b0dynisel);
	dcs_reg_write_all_chan(rDCS_AMPH_B1_DYN_ISEL, dcs_params.b1dynisel);
}

//================================================================================
// Step 17: Enable Fast Critical Word Forwarding Feature
//================================================================================
void dcs_init_Fast_Critical_Word_Forwarding(void)
{
	dcs_reg_write_all_chan(rDCS_MCU_QBRPARAM, dcs_params.qbrparam);
	dcs_reg_write_all_chan(rDCS_MCU_QBREN, dcs_params.qbren);
	dcs_reg_write(rAMCC_MCCGEN_ADDR, dcs_params.mccgen);
	dcs_reg_write(rAMCC_MCC0QPROPCTRL_ADDR, 0x300011a2);
	dcs_reg_write(rAMCC_MCC1QPROPCTRL_ADDR, 0x300011a2);

#if NUM_AMCCS == 2
	dcs_reg_write(rAMCC1_MCCGEN_ADDR, dcs_params.mccgen);
	dcs_reg_write(rAMCC1_MCC0QPROPCTRL_ADDR, 0x300011a2);
	dcs_reg_write(rAMCC1_MCC1QPROPCTRL_ADDR, 0x300011a2);
#endif // #if NUM_AMCCS == 2

}

//================================================================================
// Step 18: Enable Power- and Clock-Gating Features; Config MCC and Global Timers
//================================================================================
void dcs_init_Gating_Global_Timers(void)
{
	dcs_reg_write_all_chan(rDCS_AMP_AMPCLK(AMP_CA), dcs_params.caampclk);
	dcs_reg_write_all_chan(rDCS_AMP_AMPCLK(AMP_DQ0), dcs_params.dqampclk);
	dcs_reg_write_all_chan(rDCS_AMP_AMPCLK(AMP_DQ1), dcs_params.dqampclk);
	dcs_reg_write_all_chan(rDCS_SPLLCTRL_MDLLPWRDNCFG(0), 0x00000001);
	dcs_reg_write_all_chan(rDCS_MCU_PWRMNGTEN, dcs_params.pwrmngten);

	// Global Timer Setup

	dcs_reg_write(rGLBTIMER_PMGRWAKEUPCFG,        0x0001ff);
	dcs_reg_write(rGLBTIMER_PREFREQ2ALLBANKDLY(0),          0x00f000f0);
	dcs_reg_write(rGLBTIMER_PREFREQ2ALLBANKDLY(1),          0x00f000f0);
	dcs_reg_write(rGLBTIMER_PREFREQCHNG2FREQCHNGDLY(0),     0x02400240);
	dcs_reg_write(rGLBTIMER_PREFREQCHNG2FREQCHNGDLY(1),     0x02400240);

	dcs_reg_write(rGLBTIMER_CALSEG2ALLBANK(0),     0x00fa0120);
	dcs_reg_write(rGLBTIMER_ALLBANK2CALSEG(0),     0x00fa0120);

	dcs_reg_write(rGLBTIMER_MDLLTIMER,            0x000bb8);
	dcs_reg_write(rGLBTIMER_DCCTIMER,             0x000bb8);
	dcs_reg_write(rGLBTIMER_MDLLVOLTRAMPTIMER,    0x00004b);
	dcs_reg_write(rGLBTIMER_CTRLUPDMASKTIMER,     0x00000f);
	// <rdar://problem/19573098> Elba A0: Samsung 20nm Rev0 LPDDR4 work-around(disable IDT cal)
	//dcs_reg_write(rGLBTIMER_IDTTIMER,             0x007530);	// 30000
	dcs_reg_write(rGLBTIMER_RDCALTIMER,         0x002dc6bb);
	dcs_reg_write(rGLBTIMER_WRCALTIMER,         0x002dc6c0);

	// <rdar://problem/23244578> Elba: Samsung DRAM workaround: disable hw zqcal
	if(!apply_samsung_workaround)
		dcs_reg_write(rGLBTIMER_ZQCTIMER,             0x3d0900);

	dcs_reg_write(rGLBTIMER_PERCAL_FREQCHNGTIMER, 0x000001);
	dcs_reg_write(rGLBTIMER_IMPCALTIMER,          0x002ee0);

	// Dynamic Clk & Power Gating
	dcs_reg_write_all_chan(rDCS_MCU_AMCCLKPWRGATE, 0x050a0000);

}

//================================================================================
// Step 19: ODTS read & set interval for periodic MR4 on-die Temp sensor reading
//================================================================================
void dcs_init_ODTS(void)
{
	uint8_t temp[16*8];	// Far more Channels/Ranks than we are likely ever to have/need
	// INSTEAD OF: uint8_t temp[dcs_cfg.num_dcs * dcs_cfg.num_rank];

	// MR4 read to bring memory out of self-refresh
	dcs_mrcmd(MR_READ, dcs_cfg.num_dcs, dcs_cfg.num_rank, 0x4, (uintptr_t)temp);
	// M8 Bug radar #20346417, two writes to ensure propogation
	dcs_reg_write_all_chan(rDCS_MCU_ODTSZQC, dcs_params.odtszqc3);
	dcs_reg_write_all_chan(rDCS_MCU_ODTSZQC, dcs_params.odtszqc3);

}

// <rdar://problem/23244578>: This function used in calibration code for Elba
bool shim_apply_samsung_workaround(void)
{
	return apply_samsung_workaround;
}

//================================================================================
