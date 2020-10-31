/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <platform.h>
#include <platform/clocks.h>
#include <platform/soc/hwclocks.h>

#include "dwi.h"

#ifndef TARGET_DWI_FREQUENCY
#error TARGET_DWI_FREQUENCY not defined by the target
#endif

#ifndef TARGET_DWI_TRANSFER_GAP_US
#error TARGET_DWI_TRANSFER_GAP_US not defined by the target
#endif

int dwi_init(void)
{
	u_int32_t dwi_divider, dwi_transfer_gap;

	clock_gate(CLK_DWI, true);

	dwi_divider = (clock_get_frequency(CLK_NCLK) + TARGET_DWI_FREQUENCY - 1) / TARGET_DWI_FREQUENCY;
	dwi_transfer_gap = (clock_get_frequency(CLK_NCLK) * TARGET_DWI_TRANSFER_GAP_US) / 1000000;

	rDWI_CLOCK_CONFIG = DWI_CLOCK_CONFIG_CLOCK_SCALER(dwi_divider) | DWI_CLOCK_CONFIG_TX_CPHA_1;
	rDWI_TRANSFER_GAP = dwi_transfer_gap;

	return 0;
}

int dwi_send_backlight_command(uint32_t backlight_command, uint32_t backlight_level)
{
	rDWI_ITR0_TX_DATA = (backlight_command << 28) | (backlight_level & 0x7FF);
	rDWI_ITR0_CONTROL = DWI_TR_CTRL_BYTE_SWAP | DWI_TR_CTRL_BYTE_CNT(4) | DWI_TR_CTRL_TRAN_EN | DWI_TR_CTRL_SLAVE_DWI;

	return 0;
}
