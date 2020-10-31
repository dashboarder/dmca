/*
 * Copyright (C) 2007-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* Base address for the fake device */
#define HDC_BASE_ADDR	(0x70000000)

/* Registers */
#define rHDC_CMD	(*(volatile u_int32_t *)(HDC_BASE_ADDR + 0x000))
#define rHDC_FIRST	(*(volatile u_int32_t *)(HDC_BASE_ADDR + 0x010))
#define rHDC_STATUS	(*(volatile u_int32_t *)(HDC_BASE_ADDR + 0x030))
#define rHDC_DRV_SIZE	(*(volatile u_int32_t *)(HDC_BASE_ADDR + 0x080))
#define rHDC_DMA	(*(volatile u_int32_t *)(HDC_BASE_ADDR + 0x100))

/* Register operations */
#define HDC_SET_CMD_CMD(r,c)	((r) | (((c) & 0xff) << 24))
#define HDC_SET_CMD_DISK(r,c)	((r) | (((c) & 0xff) << 16))
#define HDC_SET_CMD_LEN(r,c)	((r) | (((c) & 0xff) <<  0))

#define HDC_GET_STATUS_ST(r)	(((r) >> 24) & 0xff)
#define HDC_GET_STATUS_DISK(r)	(((r) >> 16) & 0xff)
#define HDC_GET_STATUS_CNT(r)	(((r) >>  0) & 0xffff)

/* Constant values */
#define HDC_BLKSIZE	(512)
#define HDC_MAX_SIZE	(0x1000000000)

#define HDC_CMD_READ	(0x01)
#define HDC_CMD_WRITE	(0x02)
#define HDC_CMD_INT	(0x80)
#define HDC_CMD_MASK	(0X7F)

#define HDC_ST_READY	(0)
#define HDC_ST_BUSY	(1)
#define HDC_ST_DONE	(2)
#define HDC_ST_ERROR	(3)

