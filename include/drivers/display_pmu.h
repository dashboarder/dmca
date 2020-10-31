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

#ifndef DRIVERS_DISPLAY_PMU_H
#define DRIVERS_DISPLAY_PMU_H	1

#if WITH_HW_CHESTNUT
#define DISPLAY_PMU_LDO(n)	(((n) + 1) << 2)
#endif

int display_pmu_init(void);
int display_pmu_quiesce(void);
void display_pmu_update_device_tree(const char *node);

#endif /* DRIVERS_DISPLAY_PMU_H */

