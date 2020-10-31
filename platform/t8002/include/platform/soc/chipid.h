/*
 * Copyright (C) 2013 - 2014 Apple Inc. All rights reserved.
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
#include <platform/soc/pmgr.h>
#include <platform/soc/hwregbase.h>

#define	rCFG_FUSE0			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE0_OFFSET))
#define	rCFG_FUSE1			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE1_OFFSET))
#define	rCFG_FUSE2			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE2_OFFSET))
#define	rCFG_FUSE3			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE3_OFFSET))
#define	rCFG_FUSE4			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE4_OFFSET))
#define	rCFG_FUSE5			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE5_OFFSET))
#define rCFG_FUSE9			(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE9_OFFSET))

#define	rECIDLO				(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_ECID_FUSE0_OFFSET))
#define	rECIDHI				(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_ECID_FUSE1_OFFSET))

#define rSEP_SECURITY		(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_SEP_SECURITY_OFFSET))

#define rCFG_FUSE0_RAW		(*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_FUSE_CFG_FUSE0_RAW_OFFSET))

uint32_t chipid_get_total_rails_leakage();
uint32_t chipid_get_soc_voltage(uint32_t index);
uint32_t chipid_get_fuse_revision(void);

#endif /* __PLATFORM_SOC_CHIPID_H */
