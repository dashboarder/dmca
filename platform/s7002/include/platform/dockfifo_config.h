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
#ifndef __DOCKFIFO_CONFIG_H
#define __DOCKFIFO_CONFIG_H

#include <drivers/dockfifo/dockfifo.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>

// Fifo enumeration
#define DOCKFIFO_UART_WRITE	(DOCKFIFO_0)
#define DOCKFIFO_UART_READ	(DOCKFIFO_1)
#define DOCKFIFO_BULK_WRITE	(DOCKFIFO_2)
#define DOCKFIFO_BULK_READ	(DOCKFIFO_3)

static const struct dockfifo_config dockfifo_configs[] = {
	/* 	read/write	clock			interrupt	fifo name	*/  
	{	DOCKFIFO_W, 	CLK_DOCKFIFO, 	DBGFIFO_0_NOT_EMPTY, 	"uart write"	},
	{ 	DOCKFIFO_R, 	CLK_DOCKFIFO, 	DBGFIFO_1_NOT_EMPTY, 	"uart read"	},
	{ 	DOCKFIFO_W, 	CLK_DOCKFIFO, 	DBGFIFO_2_NOT_EMPTY, 	"bulk write"	},
	{ 	DOCKFIFO_R, 	CLK_DOCKFIFO, 	DBGFIFO_3_NOT_EMPTY, 	"bulk read"	},
};

#endif /* __DOCKFIFO_CONFIG_H */
