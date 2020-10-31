/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#ifndef __SAMSUNG_SHA1_H
#define __SAMSUNG_SHA1_H

#include <platform/soc/hwregbase.h>

#define rSHA1_ADDR_CONF		(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x00))
#define rSHA1_ADDR_SWRESET	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x04))
#define rSHA1_ADDR_INT_SRC	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x08))
#define rSHA1_ADDR_INT_MASK	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x0C))
#define rSHA1_ADDR_ENDIAN	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x10))

#define rSHA1_ADDR_CODE0	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x20))
#define rSHA1_ADDR_CODE1	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x24))
#define rSHA1_ADDR_CODE2	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x28))
#define rSHA1_ADDR_CODE3	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x2C))
#define rSHA1_ADDR_CODE4	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x30))

#define rSHA1_ADDR_DATA0	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x40))
#define rSHA1_ADDR_DATA1	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x44))
#define rSHA1_ADDR_DATA2	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x48))
#define rSHA1_ADDR_DATA3	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x4C))
#define rSHA1_ADDR_DATA4	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x50))
#define rSHA1_ADDR_DATA5	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x54))
#define rSHA1_ADDR_DATA6	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x58))
#define rSHA1_ADDR_DATA7	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x5C))
#define rSHA1_ADDR_DATA8	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x60))
#define rSHA1_ADDR_DATA9	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x64))
#define rSHA1_ADDR_DATA10	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x68))
#define rSHA1_ADDR_DATA11	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x6C))
#define rSHA1_ADDR_DATA12	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x70))
#define rSHA1_ADDR_DATA13	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x74))
#define rSHA1_ADDR_DATA14	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x78))
#define rSHA1_ADDR_DATA15	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x7C))

#define rSHA1_MASTER_MODE	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x80))
#define rSHA1_MS_START_ADDR	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x84))
#define rSHA1_VERISON		(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x88))
#define rSHA1_MS_SIZE		(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x8C))
#define rSHA1_FIFO_PARAM	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x90))
#define rSHA1_FIFO_CMD		(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x94))
#define rSHA1_TX_FIFO_STAT	(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0x98))
#define rSHA1_TX_FIFO		(*(volatile u_int32_t *)(SHA1_BASE_ADDR + 0xA0))

struct sha1_regs {
	volatile u_int32_t *con;
	volatile u_int32_t *swreset;
	volatile u_int32_t *int_src;
	volatile u_int32_t *int_mask;
	volatile u_int32_t *twist;

	volatile u_int32_t *code0;
	volatile u_int32_t *code1;
	volatile u_int32_t *code2;
	volatile u_int32_t *code3;
	volatile u_int32_t *code4;

	volatile u_int32_t *data0;
	volatile u_int32_t *data1;
	volatile u_int32_t *data2;
	volatile u_int32_t *data3;
	volatile u_int32_t *data4;
	volatile u_int32_t *data5;
	volatile u_int32_t *data6;
	volatile u_int32_t *data7;
	volatile u_int32_t *data8;
	volatile u_int32_t *data9;
	volatile u_int32_t *data10;
	volatile u_int32_t *data11;
	volatile u_int32_t *data12;
	volatile u_int32_t *data13;
	volatile u_int32_t *data14;
	volatile u_int32_t *data15;

	volatile u_int32_t *mode;
	volatile u_int32_t *start_addr;
	volatile u_int32_t *ms_size;
};
typedef struct sha1_regs sha1_regs;

struct sha1_status {
	u_int32_t	offset;
	u_int32_t	cmd;
	u_int32_t	length;
};
typedef struct sha1_status sha1_status;

#endif /* __SAMSUNG_SHA1_H */
