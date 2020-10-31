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
#include <drivers/display_pmu.h>
#include <lib/devicetree.h>
#include <sys/task.h>
#include <target/gpiodef.h>
#include <target.h>

#define IIC_ADDRESS		(0xEE)

#define ADDR_0x08		(0x08)
#define ADDR_0x15		(0x15)
#define ADDR_0x16		(0x16)
#define ADDR_0x17		(0x17)

static bool	beryllium_supported;
static uint8_t	beryllium_ldo_voltage;		// LDO voltage encoding read from Panel
static uint8_t	beryllium_boost_voltage;	// Boost voltage encoding read from Panel
static bool	beryllium_use_updated_voltages;	// Set if LDO and Boost voltage override exists 

int display_pmu_init(void)
{
	uint8_t data[2];

	extern bool ipod6_is_beryllium_supported();

	if((beryllium_supported = ipod6_is_beryllium_supported()) == false)
		goto exit;

#if WITH_HW_DISPLAY_SUMMIT
	extern int8_t summit_get_raw_panel_id(uint8_t *raw_panel_id, size_t raw_panel_id_length);
	uint8_t raw_panel_id[15];

	summit_get_raw_panel_id(raw_panel_id, sizeof(raw_panel_id));
	beryllium_ldo_voltage = raw_panel_id[4];
	beryllium_boost_voltage = raw_panel_id[5];
	beryllium_use_updated_voltages = true;
#endif

	// clear AUTO_RESTART bit
	// <rdar://problem/15928326> n27a: Beryllium OTP setting for AUTO_RESTART bit is set wrong
	data[0] = ADDR_0x15;
	RELEASE_ASSERT(iic_read(DISPLAY_PMU_IIC_BUS, IIC_ADDRESS, (const void *)data, 1, (void *)(data + 1), 1, IIC_NORMAL) == 0);
	data[1] &= ~(1 << 0);
	RELEASE_ASSERT(iic_write(DISPLAY_PMU_IIC_BUS, IIC_ADDRESS, data, 2) == 0);

	// clear SHDN bit, this will cause chip to enter IDLE mode after OTP read
	data[0] = ADDR_0x08;
	data[1] = 0;
	RELEASE_ASSERT(iic_write(DISPLAY_PMU_IIC_BUS, IIC_ADDRESS, data, 2) == 0);

	// wait 2ms
	task_sleep(2);

	if (beryllium_use_updated_voltages) {
		// set LDO voltage
		data[0] = ADDR_0x16;
		data[1] = beryllium_ldo_voltage;
		RELEASE_ASSERT(iic_write(DISPLAY_PMU_IIC_BUS, IIC_ADDRESS, data, 2) == 0);

		// set Boost voltage
		data[0] = ADDR_0x17;
		data[1] = beryllium_boost_voltage;
		RELEASE_ASSERT(iic_write(DISPLAY_PMU_IIC_BUS, IIC_ADDRESS, data, 2) == 0);
	}

exit:
	return 0;
}

int display_pmu_quiesce(void)
{
	uint8_t data[2];

	if (beryllium_supported == false)
		goto exit;

	// set SHDN bit, this will cause chip to go back into shutdown mode
	data[0] = ADDR_0x08;
	data[1] = 1;
	RELEASE_ASSERT(iic_write(DISPLAY_PMU_IIC_BUS, IIC_ADDRESS, data, 2) == 0);

exit:
	return 0;
}

#if WITH_DEVICETREE
void display_pmu_update_device_tree(const char *node_string)
{
        uint32_t propSize;
        char *propName;
        void *propData;
	DTNode *node;

	// Find display-pmu node
	if (FindNode(0, node_string, &node)) {
		if (beryllium_supported && beryllium_use_updated_voltages) { 
			propName = "ldo-voltage";
			if (FindProperty(node, &propName, &propData, &propSize))
				if (propSize >= sizeof(beryllium_ldo_voltage))
					*(uint32_t *)propData = beryllium_ldo_voltage;

			propName = "boost-voltage";
			if (FindProperty(node, &propName, &propData, &propSize))
				if (propSize >= sizeof(beryllium_boost_voltage))
					*(uint32_t *)propData = beryllium_boost_voltage;
		}
		else if (!beryllium_supported) {
			propName = "not-supported";
			if (FindProperty(node, &propName, &propData, &propSize))
				if (propSize >= sizeof(bool))
					*(bool *)propData = true;
		}
	}
}
#endif
