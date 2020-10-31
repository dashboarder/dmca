/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <drivers/power.h>
#include <platform/memmap.h>
#include <sys/boot.h>
#include <sys/security.h>

int32_t platform_restore_system(void)
{
	uint32_t *signature = (uint32_t *)(SLEEP_TOKEN_BUFFER_BASE + kSleepTokenKernelOffset);

	power_will_resume();

	dprintf(DEBUG_INFO, "restore_system: signature[0]: 0x%08x, signature[1]: 0x%08x\n",
		signature[0], signature[1]);

	if ((signature[0] != 'MOSX') || (signature[1] != 'SUSP')) return -1;

	signature[0] = 0;
	signature[1] = 0;

	if (!security_validate_sleep_token(SLEEP_TOKEN_BUFFER_BASE + kSleepTokeniBootOffset)) return -1;

	/* Jump to reset vector */
	dprintf(DEBUG_INFO, "restoring kernel\n");
	prepare_and_jump(BOOT_DARWIN_RESTORE, NULL, NULL);

	/* shouldn't get here */
	panic("returned from restore_system\n");
}
