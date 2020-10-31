/*
 * Copyright (C) 2007-2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DRIVERS_RADIO_H
#define __DRIVERS_RADIO_H

#include <sys/types.h>

#if WITH_DEVICETREE
#include <lib/devicetree.h>
#endif

#include <target.h>

__BEGIN_DECLS

bool radio_get_property(enum target_property prop, void *data, int maxdata, int *retlen);
void radio_init(void);
void radio_early_init(void);
void radio_late_init(void);
void radio_quiesce(void);
void radio_power_off(void);

void radio_init_pins( void );

void radio_pmu_power_on(void);
void radio_pmu_power_off(void);

bool radio_board_present(void);

#if WITH_DEVICETREE
int radio_update_device_tree(DTNode *radio_node);
#endif

__END_DECLS

#endif /* ! __DRIVERS_RADIO_H */
