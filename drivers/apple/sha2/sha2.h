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

#ifndef __APPLE_SHA2_H
#define __APPLE_SHA2_H

#include <platform/soc/hwregbase.h>

#define rSHA2_VERSION			(*(volatile u_int32_t *)(SHA2_BASE_ADDR + 0x04))
#define rSHA2_CONFIG			(*(volatile u_int32_t *)(SHA2_BASE_ADDR + 0x08))

#define SHA2_PIO_MODE				(0 << 0)
#define SHA2_DMA_MODE				(1 << 0)
#define SHA2_CONFIG_TYPE(_t)			(((_t) & 0x7) << 4)

#define SHA_TYPE_160				(1)
#define SHA_TYPE_224				(2)
#define SHA_TYPE_256				(3)
#define SHA_TYPE_384				(4)
#define SHA_TYPE_512				(5)

#define rSHA2_MSGCTL			(*(volatile u_int32_t *)(SHA2_BASE_ADDR + 0x0C))

#define SHA2_MSGCTL_RDY				(1 << 0)
#define SHA2_MSGCTL_FIRST			(1 << 1)
#define SHA2_MSGCTL_LAST			(1 << 2)

#define rSHA2_STATUS			(*(volatile u_int32_t *)(SHA2_BASE_ADDR + 0x10))

#define SHA2_HASH_BUSY				(1 << 0)

#define rSHA2_IRQ			(*(volatile u_int32_t *)(SHA2_BASE_ADDR + 0x14))

#define SHA2_IRQ_DONE				(1 << 0)
#define SHA2_IRQ_LAST				(1 << 1)

#define rSHA2_IRQEN			(*(volatile u_int32_t *)(SHA2_BASE_ADDR + 0x18))

#define SHA2_IRQEN_DONE				(1 << 0)
#define SHA2_IRQEN_LAST				(1 << 1)

#define rSHA2_DMACTL			(*(volatile u_int32_t *)(SHA2_BASE_ADDR + 0x1C))

#define SHA2_DMACTL_START			(1 << 0)
#define SHA2_DMACTL_BURSTSIZE(_s)		(((_s) & 0x7) << 4)

#define rSHA2_DMATOP			(*(volatile u_int32_t *)(SHA2_BASE_ADDR + 0x20))
#define rSHA2_HASH(_h)			(*(volatile u_int32_t *)(SHA2_BASE_ADDR + 0x40 + ((_h) * 4)))
#define rSHA2_MSGBLK(_m)		(*(volatile u_int32_t *)(SHA2_BASE_ADDR + 0x80 + ((_m) * 4)))

#endif /* ! __APPLE_SHA2_H */
