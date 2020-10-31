/*
 * Copyright (C) 2011-2015 Apple Inc. All rights reserved.
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
#include <lib/mib.h>
#include <sys.h>
#include <target.h>

#define LM3534_GPR		(0x10)
#define LM3534_DFS		(0x11)
#define LM3534_RRR		(0x12)
#define LM3534_BRL		(0x13)
#define LM3534_BRH		(0x14)
#define LM3534_AFT		(0x32)

static bool lm3534_probed;
static uint32_t lm3534_address[2];
static uint32_t lm3534_bus[2];
static bool lm3534_present[2];

int lm3534_backlight_probe(void)
{
	uint8_t data[1];

	if (lm3534_probed)
		return lm3534_present[0] ? 0 : -1;
	lm3534_probed = true;

	lm3534_bus[0] = mib_get_u32(kMIBTargetBacklight0I2CBus);
	lm3534_bus[1] = mib_get_u32_opt(kMIBTargetBacklight1I2CBus, (uint32_t)-1);
	lm3534_address[0] = mib_get_u32(kMIBTargetBacklight0I2CAddress);
	lm3534_address[1] = mib_get_u32_opt(kMIBTargetBacklight1I2CAddress, (uint32_t)-1);

	for (unsigned i = 0; i < 2; i++) {
		if (lm3534_bus[i] == (uint32_t)-1) {
			break;
		}
		
		dprintf(DEBUG_SPEW, "lm3534: probing backlight controller %u at i2c%u address 0x%x\n", i, lm3534_bus[i], lm3534_address[i]);

		// Probe for the LM3534 by reading a register
		data[0] = LM3534_GPR;
		if (iic_read(lm3534_bus[i], lm3534_address[i], data, 1, data, 1, IIC_NORMAL) == 0) {
			// Mark the LM3534 as present
			lm3534_present[i] = true;
		}

		if (!lm3534_present[i]) {
			if (i == 0) {
				// Return an error if the first LM3534 is not present
				dprintf(DEBUG_INFO, "lm3534: failed to detect backlight controller, bailing\n");
				return -1;
			} else {
				// Depending on the display card attached, the second backlight
				// controller may or may not exist. Its absense is not fatal.
				dprintf(DEBUG_INFO, "lm3534: failed to detect second backlight controller, ignoring\n");
			}
		} else {
			dprintf(DEBUG_SPEW, "lm3534: successfully probed backlight controller %u with value 0x%x\n", i, data[0]);
		}
	}

	return 0;
}

int lm3534_backlight_enable(uint32_t backlight_level)
{
	uint8_t data[2];

	if (!lm3534_probed) {
		lm3534_backlight_probe();
	}

	if (!lm3534_present[0])
		return -1;

	for (unsigned i = 0; i < 2; i++) {
		if (!lm3534_present[i])
			break;

		// Configure the backlight for defaults expect no ramp
		dprintf(DEBUG_SPEW, "lm3534: setting backlight controller %u to level %u\n", i, backlight_level);
		data[0] = LM3534_GPR;
		data[1] = target_lm3534_gpr(i);

		// <rdar://problem/20615779>
		// Dark boot may hand off to the OS with backlight disabled, but the OS backlight driver depends on iBoot
		// to configure the backlight even if it's left disabled.
		if (backlight_level == 0)
			data[1] &= ~0x1; // bit 0: chip enable

		iic_write(lm3534_bus[i], lm3534_address[i], data, 2);

		uint32_t thresholdValue = 0;
		if(mib_exists(kMIBTargetBacklightAutoFreqThresh, &thresholdValue)) {
			dprintf(DEBUG_SPEW, "lm3534: Modify Auto-Frequency Threshold (0x%x)\n", thresholdValue);
			data[0] = LM3534_AFT;
			data[1] = thresholdValue;
			iic_write(lm3534_bus[i], lm3534_address[i], data, 2);
		}
	}

	// <rdar://problem/11055346> Not seeing the battery trap icon
	// Delay to ensure that the LM3534 settings are processed before issuing dwi transaction
	spin(600);

	extern int dwi_send_backlight_command(uint32_t backlight_command, uint32_t backlight_level);

	dwi_send_backlight_command(0xA, backlight_level);

	return 0;
}
