/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __DRIVERS_HDC_H
#define __DRIVERS_HDC_H

#include <sys/types.h>
#include <lib/blockdev.h>

__BEGIN_DECLS

/* Generic Device defines */
#define HDC_BLOCK_SIZE		512

#define HDC_CMD_READ		0x01
#define HDC_CMD_WRITE		0x02
#define HDC_CMD_INT		0x80
#define HDC_CMD_MASK		0x7F

#define HDC_ST_READY		0x0
#define HDC_ST_BUSY		0x1
#define HDC_ST_DONE		0x2
#define HDC_ST_ERROR		0x3

#define HDC_SET_CMD_CMD(r,c)	((r) | (((c) & 0xff) << 24))
#define HDC_SET_CMD_DISK(r,c)	((r) | (((c) & 0xff) << 16))
#define HDC_SET_CMD_LEN(r,c)	((r) | (((c) & 0xff) <<  0))

#define HDC_GET_STATUS_ST(r)	(((r) >> 24) & 0xff)
#define HDC_GET_STATUS_DISK(r)	(((r) >> 16) & 0xff)
#define HDC_GET_STATUS_CNT(r)	(((r) >>  0) & 0xffff)


int hdc_init(void);

__END_DECLS

#endif /* __DRIVERS_HDC_H */
