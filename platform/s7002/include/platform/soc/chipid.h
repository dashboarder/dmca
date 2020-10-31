/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_SOC_CHIPID_H
#define __PLATFORM_SOC_CHIPID_H

#include <platform/chipid.h>
#include <platform/soc/hwregbase.h>

#define	rCFG_FUSE0			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x00))
#define	rCFG_FUSE1			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x04))
#define	rCFG_FUSE2			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x08))
#define	rCFG_FUSE3			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x0C))
#define	rCFG_FUSE4			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x10))
#define	rCFG_FUSE5			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x14))

#define	rECIDLO				(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x80))
#define	rECIDHI				(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x84))

#define rCFG_FUSE0_RAW			(*(volatile uint32_t *)(CHIPID_BASE_ADDR + 0x100))

uint32_t chipid_get_total_rails_leakage();
uint32_t chipid_get_soc_voltage(uint32_t index);
uint32_t chipid_get_fuse_revision(void);

#endif /* __PLATFORM_SOC_CHIPID_H */
