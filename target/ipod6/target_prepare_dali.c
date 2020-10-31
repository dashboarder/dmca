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
#include <debug.h>
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
	kDaliFWArgVersion2 = 'DAL2',
	kDaliFWArgVersion3 = 'DAL3',
	kDaliFWArgVersion4 = 'DAL4'
};


enum
{
  kMode24hEnabledOffset = 0,
  kRotateScreenOffset = 1,
  kDaliDarkWakeOffset = 2, /* 0 = normal wake, 1 = scheduled thermal wake, don't paint screen */
  kPmuDebugOffset = 3,
  kIsChargerConnectedOffset = 4,
  kPrechargeNeededOffset = 5
};

#define kUTCOffsetUnsetValue 0x7fffffffffffffffLL /* INT64_MAX. There's only 86400 seconds in a day */

typedef struct dali_payload_t {
  uint32_t	magic; /* = kDaliFWArgVersion3 */
  uint32_t	reserved; 
  uint8_t	charFlags[8];
  int64_t	utcOffset; /* Set to either a valid offset, or kUTCOffsetUnsetValue if unset */

  DMin_t	dmin;
} dali_payload_t;

typedef struct dali_args_t {
	uint32_t	magic; /* = kRTKitArgBlockVersion1 */
	uint32_t	size;  /* size of pending payload */
	dali_payload_t	payload;
} dali_args_t;
 
#define kDMinNVRAMKey "device-material"

static void save_dmin_to_nvram(const DMin_t* dmin)
{
	#if WITH_LLB_NVRAM
	if (target_needs_chargetrap())
		panic("LLB NVRAM is read-only, it's a logic error to cache NVRAM here");
	#endif
	char dmin_ascii[2*sizeof(DMin_t) + 1];
	const char* dmin_buffer = (const char*)(dmin);

	for(unsigned long i = 0; i < sizeof(DMin_t); i++)
	{
		snprintf(dmin_ascii + 2*i, sizeof(dmin_ascii) - 2*i, "%02X", dmin_buffer[i]);
	}
	dmin_ascii[2*sizeof(DMin_t)] = '\0';
	env_set(kDMinNVRAMKey, dmin_ascii, ENV_PERSISTENT);
	// dprintf(DEBUG_INFO, "save_dmin_to_nvram: Set kDMinNVRAMKey = %s\n", dmin_ascii);
}

static int get_dmin_from_nvram(DMin_t* dest_dmin)
{
	char* dest_buf = (char*)(dest_dmin);
	const char* dmin_val = env_get(kDMinNVRAMKey);

	if (dmin_val == NULL) {
		// dprintf(DEBUG_INFO, "No DMin cached in NVRAM yet\n");
		return -1;
	}
	
	unsigned int dmin_len = (unsigned int)strlen(dmin_val);

	if (dmin_len > 2*sizeof(DMin_t) || (dmin_len & 0x1)) {
		dprintf(DEBUG_CRITICAL, "Unexpected DMin key length %u, expected even and <= %zu\n", dmin_len, 2*sizeof(DMin_t));
		return -2;
	}
	
	unsigned long i = 0;
	unsigned long p = 0;
	while ( i < dmin_len ) {
		unsigned char byte = 0;
		for (int j = 0; j < 2; j++) {
			int shift_factor = 4*(1-j);

			switch(dmin_val[i]) {
			case '0' ... '9':
				byte += (dmin_val[i] - '0') << shift_factor;
				break;
			case 'A' ... 'F':
				byte += ((dmin_val[i] - 'A') + 10) << shift_factor;
				break;
			default:
				dprintf(DEBUG_CRITICAL, "Non-hex digit in DMin NVRAM key: 0x%x\n", dmin_val[i]);
				return -3;
			}
			i++;
		}
		dest_buf[p++] = byte;
	}
	return 0;
}

void
target_init_fast_dali()
{
	DMin_t dmin;
	bzero(&dmin, sizeof(DMin_t));
	syscfgCopyDataForTag('DMin', (uint8_t*)&(dmin), sizeof(DMin_t));
	save_dmin_to_nvram(&dmin);
}



static bool is_dark_wake()
{
	bool powersupply_change_event, button_event, other_wake_event;
	pmu_check_events(&powersupply_change_event, &button_event, &other_wake_event);
	dprintf(DEBUG_INFO, "is_dark_wake: ps_changed = %d, button = %d, other = %d\n", powersupply_change_event, button_event, other_wake_event);
	return !powersupply_change_event && !button_event;
}

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

	args->payload.magic = kDaliFWArgVersion3;
	args->payload.charFlags[kMode24hEnabledOffset] = (env_get_uint("dali-24h-mode", 0) == 1);
	args->payload.charFlags[kRotateScreenOffset] = (env_get_uint("display-rotation", 0) == 1);
	args->payload.charFlags[kPmuDebugOffset] = (env_get_uint("dali-pmu-debug", 0) == 1);
#if WITH_HW_POWER
	args->payload.charFlags[kDaliDarkWakeOffset] = (is_dark_wake());
	args->payload.charFlags[kIsChargerConnectedOffset] = (power_has_usb());
	args->payload.charFlags[kPrechargeNeededOffset] = (power_needs_precharge());
	dprintf(DEBUG_INFO, "Charger Connected: %d, Precharge needed: %d\n", (int)args->payload.charFlags[kIsChargerConnectedOffset], (int)args->payload.charFlags[kPrechargeNeededOffset]);
#endif

	if (get_dmin_from_nvram(&(args->payload.dmin)) < 0) {
		dprintf(DEBUG_CRITICAL, "Unable to find DMin in NVRAM; booting into Dali mode with potentially incorrect DMin information!\n");
	}

	args->payload.utcOffset = kUTCOffsetUnsetValue;
	const char* utcOffsetStr = env_get("utc-offset");
	if (utcOffsetStr) {
		args->payload.utcOffset = atoi(utcOffsetStr);
	}

	return args;
}
