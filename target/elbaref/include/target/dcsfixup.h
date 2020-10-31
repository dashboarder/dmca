/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

// <rdar://problem/19636728> Revert applicable Elba targets to 4 GB when kernel supports 4 GB
// will require removing chnldec from the table below, as well as
// setting DCS_NUM_CHANNELS to 8. In addition, the SDRAM_LEN and TEXT_BASE will have to be increased
static inline
void dcs_init_config_fixup_params(dcs_config_params_t *dcs_params, dcs_target_t targtype)
{
	dcs_params->chnldec              = 0x00050110;

#if DCS_DRAM_800MHZ
	dcs_params->freq[0].clk_period     = CLK_PERIOD_800;
	dcs_params->freq[0].freqchngctl[3] = 0xae015202;
	dcs_params->freq[1].freqchngctl[3] = 0xae011202;
	dcs_params->freq[0].lat            = 0x001031c6;
	dcs_params->freq[0].phyrdwrtim     = 0x00050c05;
	dcs_params->freq[0].caspch         = 0x43d10408;
	dcs_params->freq[0].act            = 0x10040908;
	dcs_params->freq[0].autoref        = 0x24480078;
	dcs_params->freq[0].selfref        = 0x5004b012;
	dcs_params->modereg                = 0x0e0c6084;
	dcs_params->pdn                    = 0x72273263;
	dcs_params->freq[0].derate         = 0x28835488;
	dcs_params->freq[0].lat2           = 0x001121c6;
	dcs_params->freq[0].tat            = 0x01312222;
	dcs_params->rdwrdqcaltiming_f0     = 0x01040401;
	dcs_params->rdwrdqcaltiming_f3     = 0x01040408;
	dcs_params->rdwrdqcaltimingctrl2   = 0x010f0a01;
	dcs_params->maxrddqssdllmulfactor  = 0x00a00b0b;
	dcs_params->maxwrdqssdllmulfactor  = 0xa0a00808;
	dcs_params->freq[0].rdcapcfg       = 0x0100070a;
	dcs_params->freq[0].autoref2       = 0x24480049;
	dcs_params->mr2cmd_step11          = 0x52;
	dcs_params->mr1cmd_step11          = 0xae;
	dcs_params->rdsdllctrl_step15      = 0x00280004;
	dcs_params->qbrparam               = 0x00000000;
#endif
};
