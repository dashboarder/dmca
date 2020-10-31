/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __PLATFORM_SOC_SDO_H
#define __PLATFORM_SOC_SDO_H

#include <platform/soc/hwregbase.h>

#define	rDAC0_SCALE			(*(volatile u_int32_t *)(SDO_BASE_ADDR + 0x1C))
#define	rDAC1_SCALE			(*(volatile u_int32_t *)(SDO_BASE_ADDR + 0x20))
#define	rDAC2_SCALE			(*(volatile u_int32_t *)(SDO_BASE_ADDR + 0x24))

extern int sdo_init(void);

#endif /* ! __PLATFORM_SOC_SDO_H */
