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

static inline
void dcs_init_config_fixup_params(dcs_config_params_t *dcs_params, dcs_target_t targtype)
{
	dcs_params->freq[0].freqchngctl[2]          = 0xa303000b;
	dcs_params->freq[1].freqchngctl[2]          = 0xd303000b;
	dcs_params->freq[0].lat          = 0x001130c2;
	dcs_params->freq[1].lat          = 0x001020c2;
	dcs_params->freq[0].phyrdwrtim   = 0x00010d01;
	dcs_params->freq[1].phyrdwrtim   = 0x00010c01;
	dcs_params->freq[3].phyrdwrtim   = 0x00010b01;
	dcs_params->freq[0].caspch       = 0x40c20402;
	dcs_params->freq[0].act  	 = 0x01020202;
	dcs_params->freq[0].autoref      = 0x010100a0;
	dcs_params->freq[0].selfref      = 0x28002012;
	dcs_params->freq[0].pdn      = 0x00000033;
	dcs_params->modereg              = 0x060c5024;
	dcs_params->modereg1             = 0x00000005;
	dcs_params->freq[1].caspch       = 0x40c20402;
	dcs_params->freq[1].act          = 0x01020202;
	dcs_params->freq[1].autoref      = 0x0101006b;
	dcs_params->freq[1].selfref      = 0x28002012;
	dcs_params->freq[1].pdn      = 0x00000033;
	dcs_params->freq[2].caspch       = 0x40c20402;
	dcs_params->freq[2].act          = 0x01020202;
	dcs_params->freq[2].autoref      = 0x01010014;
	dcs_params->freq[2].selfref      = 0x28002012;
	dcs_params->freq[3].act          = 0x01020404;
	dcs_params->freq[3].autoref      = 0x01010005;
	dcs_params->freq[3].selfref      = 0x28002012;
	dcs_params->autoref_params       = 0x00150013;
	dcs_params->pdn                  = 0x31276222;
	dcs_params->freq[0].derate       = 0x08212082;
	dcs_params->freq[1].derate       = 0x08212082;
	dcs_params->freq[2].derate       = 0x08212082;
	dcs_params->freq[0].lat2         = 0x001110c2;
	dcs_params->freq[1].lat2         = 0x001110c2;
	dcs_params->freq[0].tat          = 0x01212222;
	dcs_params->freq[1].tat          = 0x01212222;
	dcs_params->freq[3].mifqmaxctrl          = 0x00000003;
	dcs_params->freq[0].rdcapcfg     = 0x00000606;
	dcs_params->freq[1].rdcapcfg     = 0x20000606;
	dcs_params->freq[2].rdcapcfg     = 0x40000606;
	dcs_params->freq[3].rdcapcfg     = 0x60000606;
	dcs_params->b0odtctrl            = 0x00000000;
	dcs_params->b1odtctrl            = 0x00000000;
	dcs_params->dqs0odtctrl          = 0x00000000;
	dcs_params->dqs1odtctrl          = 0x00000000;
	dcs_params->freq[0].autoref2      = 0x01010001;
	dcs_params->freq[1].autoref2      = 0x01010001;
	dcs_params->freq[2].autoref2      = 0x01010001;
	dcs_params->freq[3].autoref2      = 0x01010001;
	dcs_params->autoref_params2       = 0x00170013;
	dcs_params->odtszqc              = 0x00000000;
	dcs_params->odtszqc2             = 0xc0000000;
	dcs_params->qbrparam              = 0x00001000;
	dcs_params->odtszqc3             = 0xc0002320;
};
