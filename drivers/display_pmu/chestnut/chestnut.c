/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
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
#include <drivers/display_pmu.h>
#include <lib/devicetree.h>
#include <target/gpiodef.h>
#include <target.h>

#define IIC_ADDRESS		(0x4E)

#define DEVICE_ID_0		(0x00)
#define DEVICE_ID_1		(0x01)
#define DEVICE_ID_2		(0x02)
#define REVISION		(0x03)
#define STATUS			(0x04)
#define ENABLE			(0x05)
#define VNEG_CONTROL		(0x0F)

#define TI_DEVICE_ID0		(0x007365)
#define TI_DEVICE_ID1		(0x00F365)
#define INTERSIL_DEVICE_ID	(0x0BA46E)

uint32_t	chestnut_device_id;
uint8_t 	chestnut_device_revision;
bool		chestnut_probed;
bool		chestnut_present;

static const char *chestnut_device_string(uint32_t device_id)
{
	switch (device_id) {
		case TI_DEVICE_ID0:
		case TI_DEVICE_ID1:
			return "TI";

		case INTERSIL_DEVICE_ID:
			return "Intersil";
	}

	return "Unknown";
}

static void chestnut_probe()
{
	if (!chestnut_probed) {
		// Probe by reading a register
		uint8_t data[4] = { 0 };

		data[0] = REVISION;
		if (iic_read(DISPLAY_PMU_IIC_BUS, IIC_ADDRESS, (const void *)data, 1, (void *)(data + 1), 1, IIC_NORMAL) == 0) {
			chestnut_device_revision = data[1];

			data[0] = DEVICE_ID_0;
			if (iic_read(DISPLAY_PMU_IIC_BUS, IIC_ADDRESS, (const void *)data, 1, (void *)(data + 1), 3, IIC_NORMAL) == 0)
				chestnut_device_id = (data[1] << 16) | (data[2] << 8) | data[3];

			chestnut_present = true;
		}

		chestnut_probed = true;
	}

	if (chestnut_present)
		dprintf(DEBUG_CRITICAL, "Display PMU found, 0x%08x, %s-%02x\n", chestnut_device_id, chestnut_device_string(chestnut_device_id), chestnut_device_revision);
	else
		dprintf(DEBUG_INFO, "Display PMU probe failed\n");
}

#if TARGET_CHESTNUT_USE_LOW_RIPPLE_MODE && !defined(TARGET_CHESTNUT_VNEG_CONTROL)
#define TARGET_CHESTNUT_VNEG_CONTROL 0x07
#endif

int display_pmu_init(void)
{
	chestnut_probe();

	if (chestnut_present) {
		uint8_t data[2];

#if defined(TARGET_CHESTNUT_VNEG_CONTROL)
		// Set low-ripple mode for VNEG
		// <rdar://problem/12983634> N5x | Set low-ripple mode in Chestnut before Chestnut rails are enabled
		data[0] = VNEG_CONTROL;
		data[1] = TARGET_CHESTNUT_VNEG_CONTROL;
		RELEASE_ASSERT(iic_write(DISPLAY_PMU_IIC_BUS, IIC_ADDRESS, data, 2) == 0);
#endif

		// Enable power to the panel
		data[0] = ENABLE;
		data[1] = 0x3;				// Enable Boost (VNEG and VPOS LDOs)
		data[1] |= target_get_lcm_ldos();
		RELEASE_ASSERT(iic_write(DISPLAY_PMU_IIC_BUS, IIC_ADDRESS, data, 2) == 0);

		return 0;
	}

	return -1;
}

int display_pmu_quiesce(void)
{
	if (chestnut_present) {
		uint8_t data[2];

		// Disable power to the panel
		data[0] = ENABLE;
		data[1] = 0;
		RELEASE_ASSERT(iic_write(DISPLAY_PMU_IIC_BUS, IIC_ADDRESS, data, 2) == 0);

		return 0;
	}

	return -1;
}

#if WITH_DEVICETREE
void display_pmu_update_device_tree(const char *node_string)
{
        uint32_t propSize;
        char *propName;
        void *propData;
	DTNode *node;

	if (!chestnut_present)
		return;

	// Find display-pmu node
	if (FindNode(0, node_string, &node)) {
		// Fill in the probed device info

		propName = "device-id";
		if (FindProperty(node, &propName, &propData, &propSize))
			if (propSize >= sizeof(chestnut_device_id))
				*(uint32_t *)propData = chestnut_device_id;

		propName = "device-revision";
		if (FindProperty(node, &propName, &propData, &propSize))
			if (propSize >= sizeof(chestnut_device_revision))
				*(uint32_t *)propData = chestnut_device_revision;

		propName = "device-string";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			bzero(propData, propSize);
			strlcpy(propData, chestnut_device_string(chestnut_device_id), propSize);
		}
	}
}
#endif
