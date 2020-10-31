/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __APPLE_AES_V2_H
#define __APPLE_AES_V2_H

#include <platform/soc/hwregbase.h>
#include SUB_PLATFORM_SPDS_HEADER(aes)

/* AES-v2 registers */

#define rAES_VERSION        	(*(volatile uint32_t *)(AES_AP_BASE_ADDR + AES_BLK_VERSION_OFFSET))

#define rAES_CONFIG         	(*(volatile uint32_t *)(AES_AP_BASE_ADDR + AES_BLK_CONFIG_OFFSET))

#define rAES_CONTROL        	(*(volatile uint32_t *)(AES_AP_BASE_ADDR + AES_BLK_CONTROL_OFFSET))

#define rAES_STATUS         	(*(volatile uint32_t *)(AES_AP_BASE_ADDR + AES_BLK_STATUS_OFFSET))

#define rAES_INT_STATUS     	(*(volatile uint32_t *)(AES_AP_BASE_ADDR + AES_BLK_INT_STATUS_OFFSET))

#define rAES_INT_ENABLE     	(*(volatile uint32_t *)(AES_AP_BASE_ADDR + AES_BLK_INT_ENABLE_OFFSET))

#define rAES_WATERMARKS     	(*(volatile uint32_t *)(AES_AP_BASE_ADDR + AES_BLK_WATERMARKS_OFFSET))

#define rAES_COMMAND_FIFO_STATUS (*(volatile uint32_t *)(AES_AP_BASE_ADDR + AES_BLK_COMMAND_FIFO_STATUS_OFFSET))

#define rAES_HISTORY_FIFO_STATUS (*(volatile uint32_t *)(AES_AP_BASE_ADDR + AES_BLK_HISTORY_FIFO_STATUS_OFFSET))

#define rAES_COMMAND_FIFO_COUNT (*(volatile uint32_t *)(AES_AP_BASE_ADDR + AES_BLK_COMMAND_FIFO_COUNT_OFFSET))

#define rAES_FLAG_COMMAND 	(*(volatile uint32_t *)(AES_AP_BASE_ADDR + AES_BLK_FLAG_COMMAND_OFFSET))

#define rAES_SKG_KEY 		(*(volatile uint32_t *)(AES_AP_BASE_ADDR + AES_BLK_SKG_KEY_OFFSET))

#define rAES_COMMAND_FIFO 	(*(volatile uint32_t *)(AES_AP_BASE_ADDR + AES_BLK_COMMAND_FIFO_OFFSET))

#endif /* __APPLE_AES_V2_H */
