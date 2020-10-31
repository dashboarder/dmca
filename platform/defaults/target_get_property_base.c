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
#include <lib/syscfg.h>
#include <target.h>

static const uint32_t prop_map[TARGET_PROPERTY_MAX] = {
	[TARGET_PROPERTY_WIFI_MACADDR]                  = 'WMac',
	[TARGET_PROPERTY_WIFI_CALIBRATION_TX_24]        = 'W24G',
	[TARGET_PROPERTY_WIFI_CALIBRATION_TX_50]        = 'W50G',
	[TARGET_PROPERTY_WIFI_CALIBRATION_RX_24]        = 'W24R',
	[TARGET_PROPERTY_WIFI_CALIBRATION_RX_50]        = 'W50R',
	[TARGET_PROPERTY_WIFI_CALIBRATION_RX_TEMP]      = 'WRxT',
	[TARGET_PROPERTY_WIFI_CALIBRATION_FREQ_GROUP_2G]= 'FG2G',
	[TARGET_PROPERTY_WIFI_WCAL]			= 'WCAL',
	[TARGET_PROPERTY_WIFI_BOARD_SNUM]               = 'WBSn',
	[TARGET_PROPERTY_BT_MACADDR]                    = 'BMac',
	[TARGET_PROPERTY_ETH_MACADDR]                   = 'EMac',
	[TARGET_PROPERTY_BB_REGION_SKU]                 = 'RSKU',
	[TARGET_PROPERTY_PINTO_MACADDR]                 = 'MMac',
	[TARGET_PROPERTY_WIFI1_MACADDR]                 = 'WMc2',
	[TARGET_PROPERTY_BT1_MACADDR]                   = 'BMc2',
	[TARGET_PROPERTY_ETH1_MACADDR]                  = 'EMc2',
	[TARGET_PROPERTY_BLUETOOTH_DEV_MACADDR0]        = 'RMac',
};

bool
target_get_property_base(enum target_property prop, void *data, int maxdata, int *retlen)
{
	int length = 0; 
	bool result = false;

	if ((prop < TARGET_PROPERTY_MAX) && (prop_map[prop] != 0)) {
		length = syscfgCopyDataForTag(prop_map[prop], data, maxdata);
		result = (length == maxdata);
	}

	if (retlen != NULL) *retlen = length;

	return result;
}
