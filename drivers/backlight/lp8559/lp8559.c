/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/iic.h>
#include <target/gpiodef.h>
#include <sys/task.h>
#include "lp8559.h"

static bool lp8559_probed;
static bool lp8559_present;

int beacon_reg_read(uint8_t reg_offset, uint8_t *value)
{
	int status;
	status = iic_read(BACKLIGHT_IIC_BUS, lp8559_IIC_ADDRESS, &reg_offset, 1, value, 1, IIC_NORMAL);
	return status;
}

int beacon_reg_write(uint8_t reg_offset, uint8_t value)
{
	int status;
	uint8_t data[2] = {reg_offset, value};
	status = iic_write(BACKLIGHT_IIC_BUS, lp8559_IIC_ADDRESS, data, 2);
	return status;
}

int lp8559_backlight_enable(uint32_t backlight_level)
{
	uint8_t cmd, epromid;

	if (!lp8559_probed) {
		// Probe for the lp8559 by reading a register
		if (beacon_reg_read(lp8559_COMMAND, &cmd) == 0) {
			// Mark the lp8559 as present
			lp8559_present = true;
		}

		// Mark the lp8559 as probed
		lp8559_probed = true;
	}

	// Return an error if the lp8559 is not present
	if (!lp8559_present) {
		dprintf(DEBUG_INFO, "failed to detect lp8559, bailing\n");
		return -1;
	}

	// Workaround for chips with EPROMID = 0x0
	// <rdar://problem/13700089> J85: Need to update register values in Beacon
	beacon_reg_read(lp8559_EPROMID, &epromid);
	if (epromid == 0x10) {
		// Unlock protected registers
		beacon_reg_write(lp8559_LOCK, 0xCF);

		// Set inductor current limit to 2.2A
		beacon_reg_write(lp8559_CONFIG, 0x62);

		// Set IMAX to 20mA:
		beacon_reg_write(lp8559_CURRENT, 0x02);

		// Set PWM cutover point to 1mA
		beacon_reg_write(lp8559_HYBRIDLO, 0x33);

		// Set SLOPE and VERT headroom control
		beacon_reg_write(lp8559_VHR1, 0x26);

		// Set PWM global offset
		beacon_reg_write(lp8559_TRIM9, 0x08);

		// Lock protected registers
		beacon_reg_write(lp8559_LOCK, 0x00);
	}

	beacon_reg_read(lp8559_COMMAND, &cmd);


	// <rdar://problem/20615779>
	// Dark boot may hand off to the OS with backlight disabled, but the OS backlight driver depends on iBoot
	// to configure the backlight even if it's left disabled.

	// read modify write
	// Bit 0  - on, Bit 3 - enable DWI
	cmd |= 0x9;
	if (backlight_level == 0) {
		cmd &= ~(0x9);
	}

	// Configure the backlight for defaults expect no ramp
	beacon_reg_write(lp8559_COMMAND, cmd);

	// IS this Required ?? 
	// <rdar://problem/11055346> Not seeing the battery trap icon
	// Delay to ensure that the lp8559 settings are processed before issuing dwi transaction
	task_sleep(600);

	extern int dwi_send_backlight_command(uint32_t backlight_command, uint32_t backlight_level);

	dwi_send_backlight_command(0xA, backlight_level);

	return 0;
}
