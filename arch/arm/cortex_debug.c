/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch/arm/arm.h>
#include <lib/libc.h>
#include <sys.h>
#include <sys/menu.h>

#if defined(WITH_MENU) && WITH_MENU

extern char cortex_hang_start, cortex_hang_ffe, cortex_hang_end;
typedef uint32_t (*hang_func_t)(uint32_t);
static uint8_t *hang_buffer = NULL;
static hang_func_t cortex_hang;

static int do_hang(int argc, struct cmd_arg *args)
{
	u_int32_t count = 10000;
	utime_t before, after;
	int i, ret, always = 0;
	char *entry_point;

	if (argc > 1)
		count = args[1].u;
	if (!strcmp(args[0].str, "hangloop"))
		always = 1;

	if (hang_buffer == NULL) {
		hang_buffer = malloc(8192);
		entry_point = (char *)((((uint32_t)hang_buffer + 0xfff) & ~0xfff) -
				       (&cortex_hang_ffe + 2 - &cortex_hang_start));
		memcpy(entry_point, &cortex_hang_start, &cortex_hang_end - &cortex_hang_start);
		arm_clean_dcache();
		arm_invalidate_icache();
		cortex_hang = (hang_func_t)(entry_point+1); /* Thumb */
		printf("Created hang code buffer; base = %p, entry = %p\n", hang_buffer, cortex_hang);
	}

	for (i=0; (i<100) || always; i++) {
		printf("Try %3d: ", i+1);
		before = system_time();
		ret = cortex_hang(count);
		after = system_time();
		printf("complete; time = %6d\n", after-before);
		if (ret != 0) printf("   EARLY EXIT!\n");
	}
	return 0;
}

MENU_COMMAND_DEBUG(hang, do_hang, "try to hang cortex", NULL);
MENU_COMMAND_DEBUG(hangloop, do_hang, "try to hang cortex - run forever", NULL);

#endif
