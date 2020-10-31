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
#include <target.h>
#include <platform/memmap.h>
#include <platform/gpiodef.h>
#include <target/gpiodef.h>
#include <drivers/power.h>
#include <stdlib.h>
#include <lib/env.h>
#include <lib/syscfg.h>
#include <platform/dmin.h>

enum {
	kRTKitArgBlockVersion1 = 'RTK1',
	kDaliFWArgVersion1 = 'DAL1',
	kDaliFWArgVersion2 = 'DAL2'
};


enum
{
  kMode24hEnabledOffset = 0,
  kRotateScreenOffset = 1,
  kDaliDarkWakeOffset = 2, /* 0 = normal wake, 1 = scheduled thermal wake, don't paint screen */
  kPmuDebugOffset = 3
};

typedef struct dali_payload_t {
  uint32_t	magic; /* = kDaliFWArgVersion2 */
  uint32_t	reserved; 
  uint8_t	charFlags[8];
  int64_t	utcOffset;

  DMin_t	dmin;
} dali_payload_t;

typedef struct dali_args_t {
	uint32_t	magic; /* = kRTKitArgBlockVersion1 */
	uint32_t	size;  /* size of pending payload */
	dali_payload_t	payload;
} dali_args_t;

void*
target_prepare_dali(void)
{
#if defined(PMU_LDO_OPAL) && WITH_HW_POWER
	// <rdar://problem/17768885> Interim Solution: Turn off LDO8 in Dali mode/iBoot
	// Opal uses a lot of power when held in reset. For sure we don't need it in Dali.
	power_enable_ldo(PMU_LDO_OPAL, false);
#endif
#ifdef GPIO_SPU_TO_OPAL_CS_L
	gpio_configure(GPIO_SPU_TO_OPAL_CS_L, GPIO_CFG_IN);
#endif
	dali_args_t* args = (dali_args_t *)PANIC_BASE;
	bzero(args, sizeof(dali_args_t));

	args->magic = kRTKitArgBlockVersion1;
	args->size = sizeof(dali_payload_t);

	args->payload.magic = kDaliFWArgVersion2;
	args->payload.charFlags[kMode24hEnabledOffset] = (env_get_uint("dali-24h-mode", 0) == 1);
	args->payload.charFlags[kRotateScreenOffset] = (env_get_uint("display-rotation", 0) == 1);
	args->payload.charFlags[kPmuDebugOffset] = (env_get_uint("dali-pmu-debug", 0) == 1);

	syscfgCopyDataForTag('DMin', (uint8_t*)&(args->payload.dmin), sizeof(DMin_t));

	const char* utcOffsetStr = env_get("utc-offset");
	if (utcOffsetStr) {
		args->payload.utcOffset = atoi(utcOffsetStr);
	}

	return args;
}
