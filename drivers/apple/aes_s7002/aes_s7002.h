/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __APPLE_AES_S7002_H
#define __APPLE_AES_S7002_H

#include <platform/soc/hwregbase.h>

/* AES-AP registers */

#define rAES_S7002_STS				(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x0000))
#define		STS_BUSY			(1U << 0)

#define rAES_S7002_TXT_IN_CTRL			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x0008))
#define		TXT_IN_CTRL_IV_CTX_SHIFT	(2)
#define		TXT_IN_CTRL_KEY_CTX_SHIFT	(1)

#define rAES_S7002_TXT_IN_STS			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x000C))
#define		TXT_IN_FIFO_SPACE_AVAIL		(1U << 0)

#define rAES_S7002_TXT_IN			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x0040))

#define rAES_S7002_TXT_OUT_STS			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x0050))
#define		TXT_OUT_STS_IV_CTX_SHIFT	(2)
#define		TXT_OUT_STS_KEY_CTX_SHIFT	(1)
#define		TXT_OUT_DATA_AVAIL		(1U << 0)

#define rAES_S7002_TXT_OUT			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x0080))

#define rAES_S7002_KEY_IN_CTRL			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x0090))
#define		KEY_IN_CTRL_MOD_CBC		(1U << 13)
#define		KEY_IN_CTRL_MOD_ECB		(0U << 13)
#define		KEY_IN_CTRL_DIR_ENC		(1U << 12)
#define		KEY_IN_CTRL_DIR_DEC		(0U << 12)
#define		KEY_IN_CTRL_LEN_256		(2U << 6)
#define		KEY_IN_CTRL_LEN_192		(1U << 6)
#define		KEY_IN_CTRL_LEN_128		(0U << 6)
#define		KEY_IN_CTRL_SEL_GID1		(3U << 4)
#define		KEY_IN_CTRL_SEL_GID0		(2U << 4)
#define		KEY_IN_CTRL_SEL_UID1		(1U << 4)
#define		KEY_IN_CTRL_SEL_SW		(0U << 4)
#define		KEY_IN_CTRL_CTX_SHIFT		(1)			
#define		KEY_IN_CTRL_VAL_SET		(1U << 0)

#define rAES_S7002_KEY_IN_STS			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x0094))
#define		KEY_IN_STS_RDY			(1U << 0)

#define rAES_S7002_KEY_IN0			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x00C0))
#define rAES_S7002_KEY_IN1			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x00C4))
#define rAES_S7002_KEY_IN2			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x00C8))
#define rAES_S7002_KEY_IN3			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x00CC))
#define rAES_S7002_KEY_IN4			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x00D0))
#define rAES_S7002_KEY_IN5			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x00D4))
#define rAES_S7002_KEY_IN6			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x00D8))
#define rAES_S7002_KEY_IN7			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x00DC))

#define rAES_S7002_IV_IN_CTRL			(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x00E0))
#define		IV_IN_CTRL_CTX_SHIFT		(1)
#define		IV_IN_CTRL_VAL_SET		(1U << 0)

#define rAES_S7002_IV_IN0				(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x0100))
#define rAES_S7002_IV_IN1				(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x0104))
#define rAES_S7002_IV_IN2				(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x0108))
#define rAES_S7002_IV_IN3				(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x010C))

/* rAES_S7002_IV_OUTxxxx to rAES_S7002_DSB_xxxx */

#define rAES_S7002_ERR				(*(volatile uint32_t *)(AES_S7002_BASE_ADDR + 0x01D0))
#define		AES_ERR_SET			(1U << 0)

#endif /* __APPLE_AES_S7002_H */
