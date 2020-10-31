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
#ifndef __PLATFORM_CLOCKS_H
#define __PLATFORM_CLOCKS_H

#include <sys/types.h>

__BEGIN_DECLS

/* generic clock descriptors */
#define CLK_CPU 0
#define CLK_PERIPH 1
#define CLK_MEM 2
#define CLK_BUS 3
#define CLK_FIXED 4
#define CLK_TIMEBASE 5
#define HWCLOCK_BASE 6

void clock_get_frequencies(u_int32_t *clocks, u_int32_t count);
u_int32_t clock_get_frequency(int clock);
void clock_set_frequency(int clock, u_int32_t divider, u_int32_t pll_p, u_int32_t pll_m, u_int32_t pll_s, u_int32_t pll_t);
u_int32_t clocks_get_performance_divider(void);
void clock_gate(int device, bool enable);
void power_on(int device);
void clock_reset_device(int device);
void clock_set_device_reset(int device, bool set);
bool clock_get_pcie_refclk_good(void);

void platform_diag_gate_clocks(void);

int clocks_init(void);
int clocks_set_default(void);
void clocks_quiesce(void);
u_int32_t clocks_set_performance(u_int32_t performance_level);

__END_DECLS

#endif /* ! __PLATFORM_CLOCKS_H */
