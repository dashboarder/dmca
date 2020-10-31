/*
 * Copyright (C) 2007-2009 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <lib/libc.h>	/* for __unused */
#include <sys.h>
#include <platform/timer.h>
#include <drivers/power.h>

#if WITH_HW_TIMER
utime_t system_time(void)
{
	return timer_ticks_to_usecs(timer_get_ticks());
}

bool time_has_elapsed(utime_t start_time, utime_t timeout)
{
	return ((system_time() - start_time) >= timeout) ? true : false;
}

void spin(utime_t usecs)
{
	utime_t t;

	t = system_time();
	while (system_time() - t < usecs)
		;
}
#else
utime_t system_time(void)
{
	return 0;
}

bool time_has_elapsed(utime_t start_time __unused, utime_t timeout __unused)
{
	return true;
}

void spin(utime_t usecs __unused)
{
}

#endif

#if WITH_HW_POWER

utime_t calendar_time(void)
{
    return power_get_calendar_time();
}

#else

utime_t calendar_time(void)
{
    return 0;
}

#endif