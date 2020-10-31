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
#ifndef __DRIVERS_THERMAL_H
#define __DRIVERS_THERMAL_H

#include <sys/types.h>

__BEGIN_DECLS

/*
 * Thermal management.
 *
 * Thermal status is reported via the following nodes in the MIB:
 *
 * Thermal.Temperature		unit current temperature in degrees Celsius.
 * Thermal.Fanspeed		current fan speed
 */

/*
 * thermal_init
 *
 * Initialise the thermal management system.
 *
 * If the default target is passed as 0, it is assumed the system already
 * knows the default.
 */
int		thermal_init(u_int32_t default_target);

/*
 * thermal_set_target_temperature
 *
 * Set the target temperature to be maintained by the thermal system in degrees Celsius.
 */
int		thermal_set_target_temperature(u_int32_t temperature);

/*
 * thermal_soft_regulation_fail
 *
 * Called when the system is no longer able to provide services that may be required
 * to perform soft thermal regulation, e.g. during a panic.  This allows a soft
 * regulation algorithm to e.g. set the cooling system to maximum, as long as care is
 * taken.
 */
void		thermal_soft_regulation_fail(void);

__END_DECLS

#endif /* __DRIVERS_THERMAL_H */
