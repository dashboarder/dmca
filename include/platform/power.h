/*
 * Copyright (C) 2007-2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_POWER_H
#define __PLATFORM_POWER_H

#include <sys/types.h>

__BEGIN_DECLS

void platform_power_init(void);
void platform_power_spin(u_int32_t usecs);

#if WITH_HW_POWER_GATING

#include <platform/hwpower.h>

void platform_power_set_gate(uint32_t device, bool state);

#else /* ! WITH_HW_POWER_GATING */

#define PWRBIT_ALL		(0)
#define PWRBIT_OFF_DEFAULT	(0)
#define PWRBIT_OFF_DIAG		(0)

#define platform_power_set_gate(device, state)

#endif /* WITH_HW_POWER_GATING */

__END_DECLS

#endif /* ! __PLATFORM_POWER_H */
