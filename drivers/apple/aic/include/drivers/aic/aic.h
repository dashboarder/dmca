/*
 * Copyright (C) 2009-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_SOC_AIC_H
#define __PLATFORM_SOC_AIC_H

#include <platform/soc/hwregbase.h>

// AIC_VERSION
// 0 - < H6
// 1 - H6-H8, M7
// 2 - M8
// 3 - H9 

#ifndef AIC_VERSION 
#error "AIC_VERSION is not defined in hwregbase.h"
#endif
#ifndef AIC_INT_COUNT
#error "AIC_INT_COUNT is not defined in hwregbase.h"
#endif

#define	rAIC_REV				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x0000))
#define	rAIC_CAP0				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x0004))
#define	rAIC_CAP1				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x0008))
#define	rAIC_RST				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x000C))

#define	rAIC_GLB_CFG				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x0010))
#define		AIC_GLBCFG_IEN		(1 << 0)
#if AIC_VERSION < 1
#define		AIC_GLBCFG_TEN		(1 << 1)
#define		AIC_GLBCFG_EWT(_t)	((_t) << 4)
#define		AIC_GLBCFG_IWT(_t)	((_t) << 8)
#define		AIC_GLBCFG_ACG		(1 << 16)
#define		AIC_GLBCFG_WT_MASK	(7)
#else
#define		AIC_GLBCFG_AEWT(_t)	((_t) << 4)
#define		AIC_GLBCFG_SEWT(_t)	((_t) << 8)
#define		AIC_GLBCFG_AIWT(_t)	((_t) << 12)
#define		AIC_GLBCFG_SIWT(_t)	((_t) << 16)
#define		AIC_GLBCFG_SYNC_ACG	(1 << 29)
#define		AIC_GLBCFG_EIR_ACG	(1 << 30)
#define		AIC_GLBCFG_REG_ACG	(1 << 31)
#define		AIC_GLBCFG_WT_MASK	(15)
#endif // AIC_VERSION < 1
#define		AIC_GLBCFG_WT_64MICRO	(7)

#if AIC_VERSION < 1
#define	rAIC_TIME_LO				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x0020))
#define	rAIC_TIME_HI				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x0028))
#elif AIC_VERSION < 3
#define	rAIC_TIME_LO				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x1020))
#define	rAIC_TIME_HI				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x1028))
#else // (AIC_VERSION >= 3)
#define	rAIC_TIME_LO				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x8020))
#define	rAIC_TIME_HI				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x8028))
#endif // AIC_VERSION

#define	rAIC_WHOAMI				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x2000))
#define	rAIC_IACK				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x2004))
#define	rAIC_IPI_SET				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x2008))
#define	rAIC_IPI_CLR				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x200C))
#define		AIC_IPI_NORMAL		(1 << 0)
#define		AIC_IPI_SELF		(1 << 31)
#define	rAIC_TMR_CFG				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x2010))
#define		AIC_TMRCFG_EN		(1 << 0)
#define		AIC_TMRCFG_IMD		(1 << 1)
#define		AIC_TMRCFG_EMD		(1 << 2)
#define		AIC_TMRCFG_SMD		(1 << 3)
#define		AIC_TMRCFG_FSL(_s)	((_s) << 4)
#define		kAIC_TMRCFGFSL_PVT	0
#define		kAIC_TMRCFGFSL_SGT	1
#define		kAIC_TMRCFGFSL_EXT	2
#define		kAIC_TMRCFG_FSL_MASK	(AIC_TMRCFG_FSL(3))
#if AIC_VERSION < 1
#define	rAIC_TMR_CNT				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x2014))
#define	rAIC_TMR_ISR				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x2018))
#define		AIC_TMRISR_PCT		(1 << 0)
#define		AIC_TMRISR_ETS		(1 << 1)
#define		AIC_TMRISR_STS		(1 << 2)
#define	rAIC_TMR_ST_SET				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x201C))
#define	rAIC_TMR_ST_CLR				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x2020))
#define		AIC_TMRST_SGT		(1 << 0)
#define		AIC_TMRST_TIM		(1 << 1)
#endif // AIC_VERSION < 1
#define	rAIC_IPI_MASK_SET			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x2024))
#define	rAIC_IPI_MASK_CLR			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x2028))
#if AIC_VERSION > 0
#define	rAIC_PVT_STAMP_CFG			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x2040))
#define		AIC_PVT_STAMP_CFG_EN	(1 << 31)
#define	rAIC_PVT_STAMP_LO			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x2048))
#define	rAIC_PVT_STAMP_HI			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x204C))
#endif // AIC_VERSION > 0

#define rAIC_EIR_DEST(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x3000 + ((_n) * 4)))
#define rAIC_EIR_SW_SET(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x4000 + ((_n) * 4)))
#define rAIC_EIR_SW_CLR(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x4080 + ((_n) * 4)))
#define rAIC_EIR_MASK_SET(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x4100 + ((_n) * 4)))
#define rAIC_EIR_MASK_CLR(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x4180 + ((_n) * 4)))
#define rAIC_EIR_INT_RO(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x4200 + ((_n) * 4)))

#define rAIC_WHOAMI_Pn(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x5000 + ((_n) * 0x80)))
#define rAIC_IACK_Pn(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x5004 + ((_n) * 0x80)))
#define rAIC_IPI_SET_Pn(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x5008 + ((_n) * 0x80)))
#define rAIC_IPI_CLR_Pn(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x500C + ((_n) * 0x80)))
#define rAIC_TMR_CFG_Pn(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x5010 + ((_n) * 0x80)))
#define rAIC_TMR_CNT_Pn(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x5014 + ((_n) * 0x80)))
#define rAIC_TMR_ISR_Pn(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x5018 + ((_n) * 0x80)))
#define rAIC_TMR_ST_SET_Pn(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x501C + ((_n) * 0x80)))
#define rAIC_TMR_ST_CLR_Pn(_n)			(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x5020 + ((_n) * 0x80)))
#define rAIC_IPI_MASK_SET_Pn(_n)		(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x5024 + ((_n) * 0x80)))
#define rAIC_IPI_MASK_CLR_Pn(_n)		(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x5028 + ((_n) * 0x80)))

#define kAIC_INT_SPURIOUS		(0x00000)
#define kAIC_INT_EXT			(0x10000)
#define kAIC_INT_IPI			(0x40000)
#define kAIC_INT_IPI_NORM		(0x40001)
#define kAIC_INT_IPI_SELF		(0x40002)
#define kAIC_INT_TMR			(0x70000)
#define kAIC_INT_PVT_TMR		(0x70001)
#define kAIC_INT_EXT_TMR		(0x70002)
#define kAIC_INT_SW_TMR			(0x70004)

#define AIC_INT_EXT(_v)			(((_v) & 0x70000) == kAIC_INT_EXT)
#define AIC_INT_IPI(_v)			(((_v) & 0x70000) == kAIC_INT_IPI)
#define AIC_INT_TMR(_v)			(((_v) & 0x70000) == kAIC_INT_TMR)

#define AIC_INT_EXTID(_v)		((_v) & 0x1FF)

#define AIC_SRC_TO_EIR(_s)		((_s) >> 5)
#define AIC_SRC_TO_MASK(_s)		(1 << ((_s) & 0x1F))

#define kAIC_MAX_EXTID			(AIC_INT_COUNT)
#define kAIC_VEC_IPI			(kAIC_MAX_EXTID)
#define kAIC_VEC_TMR			(kAIC_MAX_EXTID + 1)
#define kAIC_VEC_SW_TMR			(kAIC_VEC_TMR + 1)
#define kAIC_NUM_INTS			(kAIC_VEC_SW_TMR + 1)

#define kAIC_NUM_EIRS			AIC_SRC_TO_EIR(kAIC_MAX_EXTID)
#if AIC_VERSION < 1
#define kAIC_NUM_TMRS			(3)
#endif // AIC_VERSION < 1

#endif /* ! __PLATFORM_SOC_AIC_H */
