/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <sys/menu.h>
#include <platform/clocks.h>
#include <platform/soc/hwregbase.h>
#include <platform/soc/hwclocks.h>

#include "rosc.h"

static void rosc_ops(int location, int type, int period);

static void rosc_ops(int location, int type, int period)
{
	u_int32_t	coe = 0;

#if ROSC_WITH_CLOCK
	clock_gate(CLK_ROSC, true);
#endif
	coe = ROSC_PRESCALE * (clock_get_frequency(CLK_PCLK)/ROSC_INTERVAL);
	/* NOP */
	rROSC_CMD = ROSC_CMD_NOP;
	while ((rROSC_CMD & ROSC_CMD_STOP) != 0);
	/* Configure the ring osc */
	rROSC_CFG = ROSC_CFG_INT_DURATION | (1<<(ROSC_VTH - type + 3)) | (location<<0);
	if (!period) {
		u_int32_t	res = 0;
		/* one shot */
		rROSC_CUR_STAT = 0;
		rROSC_CMD = ROSC_CMD_RUN_ONCE;
		spin(1 * 1000);
		while ((rROSC_CMD & ROSC_CMD_STOP) != 0) {
		}
		res = rROSC_CUR_STAT;
		printf(" current: %dMHz\n", ((res + 2) * coe)/ROSC_1MHZ);
	} else {
		u_int32_t	res1 = 0;
		u_int32_t	res2 = 0;

		/* Free run */
		rROSC_MAX_STAT = 0;
		rROSC_MIN_STAT = 0xFFF;
		rROSC_CMD = ROSC_CMD_START;
		spin(period * 1000);
		rROSC_CMD = ROSC_CMD_STOP;

		while ((rROSC_CMD & ROSC_CMD_STOP) != 0) {
		}
		res1 = rROSC_MAX_STAT;
		res2 = rROSC_MIN_STAT;
		printf(" maximum: %dMHz, minimum %dMHz\n", ((res1 + 2) * coe)/ROSC_1MHZ, ((res2 + 2) * coe)/ROSC_1MHZ);

	}
#if ROSC_WITH_CLOCK
	clock_gate(CLK_ROSC, false);
#endif
	return;
}

#if defined(WITH_MENU) && WITH_MENU

static int do_rosc(int argc, struct cmd_arg *args)
{
	int period;

	if (!security_allow_modes(kSecurityModeHWAccess)) {
		printf("Permission Denied\n");
		return -1;
	}
	if (argc < 2)
		period = 0;
	else
		period = args[1].u;

	if (period) {
		printf("Ring OSC clock test in free run mode for %d mili seconds\n", period);
	} else {
		printf("Ring OSC clock test in one shot mode\n");
	}

	for (int i = 0; i <  ROSC_NUM; i++) {
		for (int j = 0; j < ROSC_VTH; j++) {
			if ((1<<(i + j * 5)) & ROSC_MASK) {
				switch (j) {
					case 0:
						printf("  HVT%d", i);
						break;
					case 1:
						printf("  RVT%d", i);
						break;
					case 2:
						printf("  LVT%d", i);
						break;
					default:
						break;
				}
				rosc_ops(i, j, period);
			}
		}
	}
	return 0;
}
MENU_COMMAND_DEBUG(rosc, do_rosc, "Ring OSC reading", NULL);

#endif
