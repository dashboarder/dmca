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
#ifndef __DOCKFIFO_H
#define __DOCKFIFO_H

struct dockfifo_config {
	uint8_t		type;
	uint8_t		clk;
	uint8_t		irq;
	const char 	fifo_name[256];
};

#define DOCKFIFO_W	(1)
#define DOCKFIFO_R	(0)

#define DOCKFIFO_0		(0)
#define DOCKFIFO_1		(1)
#define DOCKFIFO_2		(2)
#define DOCKFIFO_3		(3)
#define DOCKFIFO_4		(4)
#define DOCKFIFO_5		(5)
#define DOCKFIFO_6		(6)
#define DOCKFIFO_7		(7)

// Maximum frame size that can be handled by dockfifo_bulk_read_frame.
#define DOCKFIFO_BULK_MRU	576

int32_t dockfifo_uart_init();
int32_t dockfifo_uart_putc(char c);
int32_t dockfifo_uart_getc(bool wait);

int32_t dockfifo_bulk_init();
int32_t dockfifo_bulk_quiesce();
int32_t dockfifo_bulk_read_frame(void *buf, size_t bytes, size_t *received);
int32_t dockfifo_bulk_write_frame(const void *buf, size_t bytes);

void	dockfifo_enable_clock_gating(int num);

#endif /* __DOCKFIFO_H */
