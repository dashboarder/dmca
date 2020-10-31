/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __TARGET_POWERCONFIG_H
#define __TARGET_POWERCONFIG_H

// Minimal definitions to make lib/power.c happy
#define TARGET_POWER_NO_BATTERY         (1)
#define PRECHARGE_BACKLIGHT_LEVEL	(824)
#define ALWAYS_BOOT_BATTERY_VOLTAGE	(5000)

// This fake SoC block takes the place of the external PMU in Fastsim
#define SWIFTER_PMU_BASE		(0x20F050000)

#endif
