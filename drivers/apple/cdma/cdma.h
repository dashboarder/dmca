/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __APPLE_CDMA_H
#define __APPLE_CDMA_H

#include <sys/types.h>
#include <platform/soc/hwregbase.h>

__BEGIN_DECLS

#define rCDMA_CHANNEL_CSR(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000000 + (n) * 0x1000))

#define CDMA_CSR_GO		(1<<0)
#define CDMA_CSR_ABT		(1<<1)
#define CDMA_CSR_PS		(1<<2)
#define CDMA_CSR_HIE		(1<<3)
#define CDMA_CSR_CIE		(1<<4)
#define CDMA_CSR_MTM		(1<<7)
#define CDMA_CSR_FC(_x)		(((_x) & 0xff) << 8)
#define CDMA_CSR_FC_NONE	0
#define CDMA_CSR_RUN(_x)	(((_x) >> 16) & 3)
#define CDMA_CSR_RUN_HALTED	0
#define CDMA_CSR_RUN_RUNNING	1
#define CDMA_CSR_RUN_STOPPED	2
#define CDMA_CSR_ERR		(1<<18)
#define CDMA_CSR_HIR		(1<<19)
#define CDMA_CSR_CIR		(1<<20)
#define CDMA_CSR_PSD		(1<<21)
#define CDMA_CSR_HIGH		(1<<22)
#define CDMA_CSR_SMALL		(1<<23)
#define CDMA_CSR_CCACHE(_x)	(((_x) & 0xf) << 24)

#define CDMA_CACHE_BUFFER	(1<<0)
#define CDMA_CACHE_CACHE	(1<<1)
#define CDMA_CACHE_RALLOC	(1<<2)
#define CDMA_CACHE_WALLOC	(1<<3)

#define rCDMA_CHANNEL_DCR(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000004 + (n) * 0x1000))

#define CDMA_DCR_DL		(1<<0)
#define CDMA_DCR_DTD_RX		(0<<1)
#define CDMA_DCR_DTD_TX		(1<<1)
#define CDMA_DCR_DW_1		(0<<2)
#define CDMA_DCR_DW_2		(1<<2)
#define CDMA_DCR_DW_4		(2<<2)
#define CDMA_DCR_DBS_1		(0<<4)
#define CDMA_DCR_DBS_2		(1<<4)
#define CDMA_DCR_DBS_4		(2<<4)
#define CDMA_DCR_DBS_8		(3<<4)
#define CDMA_DCR_DBS_16		(4<<4)
#define CDMA_DCR_DBS_32		(5<<4)
#define CDMA_DCR_ST		(1<<7)
#define CDMA_DCR_REQ(_x)	(((_x) & 0x3f) << 16)

#define rCDMA_CHANNEL_DAR(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000008 + (n) * 0x1000))
#define rCDMA_CHANNEL_DBR(n)			(*(volatile int32_t   *)(CDMA_BASE_ADDR + 0x00000C + (n) * 0x1000))
#define rCDMA_CHANNEL_MAR(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000010 + (n) * 0x1000))
#define rCDMA_CHANNEL_CAR(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000014 + (n) * 0x1000))
#define rCDMA_CHANNEL_ERR(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000018 + (n) * 0x1000))
#define rCDMA_CHANNEL_CMND0(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000020 + (n) * 0x1000))
#define rCDMA_CHANNEL_CMND1(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000024 + (n) * 0x1000))
#define rCDMA_CHANNEL_CMND2(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000028 + (n) * 0x1000))
#define rCDMA_CHANNEL_CMND3(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x00002C + (n) * 0x1000))
#define rCDMA_CHANNEL_CMND4(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000030 + (n) * 0x1000))
#define rCDMA_CHANNEL_CMND5(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000034 + (n) * 0x1000))
#define rCDMA_CHANNEL_CMND6(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000038 + (n) * 0x1000))
#define rCDMA_CHANNEL_CMND7(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x00003C + (n) * 0x1000))
#define rCDMA_CHANNEL_FIFOST(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000040 + (n) * 0x1000))
#define rCDMA_CHANNEL_FIFODT(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000400 + (n) * 0x1000))

#define rCDMA_CLOCK_ON(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000000 + (n) * 0x4))
#define rCDMA_CLOCK_OFF(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000008 + (n) * 0x4))
#define rCDMA_CLOCK_STATUS(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x000010 + (n) * 0x4))

#define rCDMA_CHANNEL_ERR_MASK	(0xFF)

// v4+
#define CDMA_CLOCK_AES_CORE	(30)
#define CDMA_CLOCK_AES_WRAP	(31)

#define rCDMA_FILTER_KEYDIS			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x800000))

#define rCDMA_FILTER_CSR(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x800000 + (n) * 0x1000))

#define CDMA_FCSR_LCK		(1<<0)
#define CDMA_FCSR_CHANNEL(_x)	(((_x) & 0xff) << 8)
#define CDMA_FCSR_ENC		(1<<16)
#define CDMA_FCSR_CBC		(1<<17)
#define CDMA_FCSR_KL_128	(0<<18)
#define CDMA_FCSR_KL_192	(1<<18)
#define CDMA_FCSR_KL_256	(2<<18)
#define CDMA_FCSR_KS_VARIABLE	(1<<20)
#define CDMA_FCSR_KEY(_x)	(((_x) & 0x0f) << 21)

#define rCDMA_FILTER_IVR0(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x800010 + (n) * 0x1000))
#define rCDMA_FILTER_IVR1(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x800014 + (n) * 0x1000))
#define rCDMA_FILTER_IVR2(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x800018 + (n) * 0x1000))
#define rCDMA_FILTER_IVR3(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x80001C + (n) * 0x1000))
#define rCDMA_FILTER_KBR0(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x800020 + (n) * 0x1000))
#define rCDMA_FILTER_KBR1(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x800024 + (n) * 0x1000))
#define rCDMA_FILTER_KBR2(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x800028 + (n) * 0x1000))
#define rCDMA_FILTER_KBR3(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x80002C + (n) * 0x1000))
#define rCDMA_FILTER_KBR4(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x800030 + (n) * 0x1000))
#define rCDMA_FILTER_KBR5(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x800034 + (n) * 0x1000))
#define rCDMA_FILTER_KBR6(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x800038 + (n) * 0x1000))
#define rCDMA_FILTER_KBR7(n)			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x80003C + (n) * 0x1000))

#define CDMA_FILTER_CONTEXTS	8
#define CDMA_FILTER_CONTEXT_NORMAL_M2M	1
#define CDMA_FILTER_CONTEXT_DESCRAMBLER	8

// v4+
#define rCDMA_FILTER_SKGCNTL			(*(volatile u_int32_t *)(CDMA_BASE_ADDR + 0x800428))

#define CDMA_FSKGCNTL_SKGDIS	(1 << 1)

struct cdma_command {
	u_int32_t nxt;
	u_int32_t ctrl;
#define CDMA_CTRL_CMD_HLT	0
#define CDMA_CTRL_CMD_NOP	1
#define CDMA_CTRL_CMD_LDIV	2
#define CDMA_CTRL_CMD_MEM	3
#define CDMA_CTRL_CMD_STP	4
#define CDMA_CTRL_CMD_MASK	0xf
#define CDMA_CTRL_BAR		(1 << 8)
#define CDMA_CTRL_CIR		(1 << 9)
#define CDMA_CTRL_FE		(1 << 16)
#define CDMA_CTRL_FR		(1 << 17)
#define CDMA_CTRL_QOS(_x)	(((_x) & 0xf) << 20)
#define CDMA_CTRL_CACHE(_x)	(((_x) & 0xf) << 24)
	u_int32_t addr;
	u_int32_t length;
	u_int32_t iv0;
	u_int32_t iv1;
	u_int32_t iv2;
	u_int32_t iv3;
};

bool	cdma_clock_enable(int channel, bool state);

__END_DECLS

#endif /* ! __APPLE_CDMA_H */
