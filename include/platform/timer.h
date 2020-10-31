/*
 * Copyright (C) 2007-2010 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_TIMER_H
#define __PLATFORM_TIMER_H

#include <sys/types.h>

__BEGIN_DECLS

/*
 * One-time timer initialisation.
 */
int timer_init(u_int32_t timer);

/*
 * Returns the current time in ticks.
 */
u_int64_t timer_get_ticks(void);

/*
 * Returns the tick rate per second.
 */
u_int32_t timer_get_tick_rate(void);

/*
 * Returns the number of microseconds that are equivalent to number of ticks passed in.
 */
utime_t timer_ticks_to_usecs(u_int64_t ticks);

/*
 * Returns the number of ticks that are equivalent to number of microseconds passed in.
 */
u_int64_t timer_usecs_to_ticks(utime_t usecs);

/*
 * Registers a single callback at (at_ticks).
 *
 * Passing NULL will delete the current callback.
 */
void timer_deadline_enter(u_int64_t at_ticks, void (* callback)(void));

/*
 * Stop all timers; used when shutting down or handing off to another image.
 */
void timer_stop_all(void);

/*
 * Return a word containing entropy.
 */
u_int32_t timer_get_entropy(void);

void wdt_enable(void);
void wdt_system_reset(void);
void wdt_chip_reset(void);


/******************************************
 Wrapper MACRO for a Register Spin-Loop with Time-Out
 */
#define SPIN_TIMEOUT_WHILE(__expr,__max)				\
  do {									\
    bool __tmo = false;							\
    uint64_t __start = timer_get_ticks();				\
    for (;;) {								\
      if ((timer_get_ticks() - __start) >= __max)			\
        __tmo = true;							\
      if (!(__expr))							\
        break;								\
      if (USE_TMO_IN_REGSPIN_LOOP && __tmo)				\
        panic("%s:%d spin loop has timed out",				\
	      __FILE__, __LINE__);					\
    }									\
  } while (0)

// If this is ZERO, there will be no Time-Out nor panic
#define USE_TMO_IN_REGSPIN_LOOP		1

// Standard Timeout is 1 second
#define SPIN_W_TMO_WHILE(__expr)	SPIN_TIMEOUT_WHILE(__expr, 1000000)
#define SPIN_W_TMO_UNTIL(__expr)	SPIN_W_TMO_WHILE(!(__expr))

__END_DECLS

#endif

