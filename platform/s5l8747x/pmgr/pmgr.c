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
#include <arch.h>
#include <platform.h>
#include <platform/timer.h>
#include <platform/power.h>
#include <platform/soc/power.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>


void platform_power_init(void)
{
	// configure the AAM clock
	rPWRCONF = (0 << 25) | (1<<24) | (0<<20) |		// AAM clock 12MHz, prescalar on /1
		   (2<<16) |					// enable 2 synch flip flops
		   (1<<14) |					// enable all OSCs in normal mode
		   (1<<13) |					// enable all OSCs in normal mode
		   (1<<12) |					// enable all OSCs in normal mode
		   (1<<1);

	rTMCMD = 0;    	       	  				// Stop the AAM timer
	rTMPRE = platform_get_osc_frequency() / 1000000;	// Set timer prescale for 1 MHz
}

void platform_power_spin(u_int32_t usecs)
{
	rTMDATA = usecs;
	rTMCMD = (1 << 1) | (1 << 0);				// Clear and start the AAM timer

	while ((rTMALARM & 1) == 0);				// Wait for the timer to fire

	rTMCMD = (1 << 1) | (0 << 0);				// Clear and stop the AAM timer
}

void platform_disable_keys(u_int32_t gid, u_int32_t uid)
{
	if (gid) rSECCONF = 1 << 1;
	if (uid) rSECCONF = 1 << 0;
}


