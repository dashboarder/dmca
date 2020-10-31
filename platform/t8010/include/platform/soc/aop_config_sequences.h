/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __AOP_CONFIG_SEQUENCES_H__
#define __AOP_CONFIG_SEQUENCES_H__

// These structures are valid reconfig commands. They are stitched together in 
// the platform init.c. For example look at the platform_s2r_aop_to_aop_ddr_post_sequence_insert()
// to figure out how the reconfig sequences are stitched together for the transition
// from s2r_aop to aop_ddr postamble

uint32_t s2r_aop_to_aop_ddr_post[] = {
	/* Comment: !!!FIXME!!! */
	0x00000000, /*Cfg End */
	0x00000000 /* END COMMAND */
};

uint32_t s2r_aop_to_aop_ddr_post_2[] = {
	/* Comment: !!!FIXME!!! */
	0x00000000, /*Cfg End */
	0x00000000 /* END COMMAND */
};

uint32_t s2r_aop_to_aop_ddr_post_3[] = {
	/* Comment: !!!FIXME!!! */
	0x00000000, /*Cfg End */
	0x00000000 /* END COMMAND */
};

/* Comment: // ****************************************************************************************** */
/* Comment: // ** AOP_DDR->S2R_AOP PREAMBLE */
/* Comment: // ****************************************************************************************** */
uint32_t aop_ddr_to_s2r_aop_pre[] = {
	/* Comment: !!!FIXME!!! */
	0x00000000, /*Cfg End */
	0x00000000 /* END COMMAND */
};

/* Comment: // ****************************************************************************************** */
/* Comment: // ** AWAKE->AOP_DDR PREAMBLE */
/* Comment: // ****************************************************************************************** */
uint32_t awake_to_aop_ddr_pre[] = {
	/* Comment: !!!FIXME!!! */
	0x00000000, /*Cfg End */
	0x00000000 /* END COMMAND */
};

/* Comment: // ****************************************************************************************** */
/* Comment: // ** AWAKE->AOP_DDR POSTAMBLE */
/* Comment: // ** */
/* Comment: // ** Note: */
/* Comment: // ** According to the PMGR spec section 11.14.4 (Software Teardown for Awake Exit), either */
/* Comment: // ** SW or the config engine may turn off SIO/PCIE.  For DV simulations, we chose to power */
/* Comment: // ** power down ALL of the unneeded blocks in the config engine rather than from the AP. This */
/* Comment: // ** provided functionally equivalent behavior and allowed the AWAKE->AOP_DDR and */
/* Comment: // ** S2R_AOP->AOP_DDR to appear symmetric.  SiVal/SW may move parts of this sequence */
/* Comment: // ** to AP software if desired. */
/* Comment: // **  */
/* Comment: // ****************************************************************************************** */
uint32_t awake_to_aop_ddr_post[] = {
	/* Comment: !!!FIXME!!! */
	0x00000000, /*Cfg End */
	0x00000000 /* END COMMAND */
};

/* Comment: // ****************************************************************************************** */
/* Comment: // ** AOP_DDR->AWAKE PREAMBLE */
/* Comment: // ****************************************************************************************** */
uint32_t aop_ddr_to_awake_pre[] = {
	/* Comment: !!!FIXME!!! */
	0x00000000, /*Cfg End */
	0x00000000 /* END COMMAND */
};

/* Comment: // ****************************************************************************************** */
/* Comment: // ** AOP_DDR->AWAKE POSTAMBLE */
/* Comment: // ** */
/* Comment: // ** Note: */
/* Comment: // ** According to the PMGR spec section 11.14.3 (Software Init for AOP_DDR to AWAKE), the */
/* Comment: // ** config engine restores the state of the chip in this step.  The specific use cases where */
/* Comment: // ** the config engine is the owner of state restoration are not yet defined. */
/* Comment: // ** */
/* Comment: // ** The sequence below contains the minimum set of commands to re-boot the AP and restore */
/* Comment: // ** DRAM operation at bucket 1.  AP may bring DRAM up to bucket 0. */
/* Comment: // ** */
/* Comment: // ** It is assumed that SW will add extensively to this sequence for state restoration based */
/* Comment: // ** on future use cases. */
/* Comment: // **  */
/* Comment: // ****************************************************************************************** */
uint32_t aop_ddr_to_awake_post[] = {
	/* Comment: !!!FIXME!!! */
	0x00000000, /*Cfg End */
	0x00000000 /* END COMMAND */
};

uint32_t aop_ddr_to_awake_post_2[] = {
	/* Comment: !!!FIXME!!! */
	0x00000000, /*Cfg End */
	0x00000000 /* END COMMAND */
};

uint32_t mcu_aop_ddr_reconfig[] = {
	/* Comment: !!!FIXME!!! */
	0x00000000, /*Cfg End */
	0x00000000 /* END COMMAND */
};

uint32_t mcu_aop_awake_reconfig_pre_restore[] = {
	/* Comment: !!!FIXME!!! */
	0x00000000, /*Cfg End */
	0x00000000 /* END COMMAND */
};

// Calibrated values will be added to this sequence, thus needs to be aligned.
// The order of registers in this sequence impacts dcs_save_calibration_results and dcs_restore_calibration_results
uint32_t mcu_aop_awake_reconfig_restore[]__aligned(32) = {
	/* Comment: !!!FIXME!!! */
	0x00000000, /*Cfg End */
	0x00000000 /* END COMMAND */
};

uint32_t mcu_aop_awake_reconfig_post_restore[] = {
	/* Comment: !!!FIXME!!! */
	0x00000000, /*Cfg End */
	0x00000000 /* END COMMAND */
};

#endif // __AOP_CONFIG_SEQUENCES_H__
