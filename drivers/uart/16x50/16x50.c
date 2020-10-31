/* -*- tab-width: 8; Mode: C; c-basic-offset: 8; indent-tabs-mode: t -*-
 *
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <drivers/uart.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwregbase.h>

#include "16x50.h"

static const uint32_t uart_ports[] = {
#ifdef  UART0_BASE_ADDR
	UART0_BASE_ADDR,
# ifdef UART1_BASE_ADDR
	UART1_BASE_ADDR,
# endif
#else
# error No UARTs configured
#endif
};

/*
 * uart_hw_init
 *
 * Initialise the port for the requested baudrate.
 */
int
uart_hw_init(uint32_t port, uint32_t baudrate)
{
	return(uart_hw_init_extended(port, baudrate, 8, PARITY_NONE, 1));
}

int
uart_hw_init_extended(uint32_t port,
		      uint32_t baudrate,
		      uint32_t databits,
		      enum uart_parity parity,
		      uint32_t stopbits)
{
	uint32_t	divisor;
	uint8_t		rv;

	/* wait for the UART to be idle before talking to it */
	while (!(rLSR(port) & LSR_THRE))
		;

	/* this can happen if the plaform defines more ports than the target */
	if (port >= sizeof(uart_ports) / sizeof(uart_ports[0]))
		return(-1);

	/* disable interrupts, we poll */
	rIER(port) = 0;	

	/* compute LCR */
	rv = LCR_WLS_8;
	switch(parity) {
	case PARITY_EVEN:
		rv |= LCR_PEN | LCR_EPS;
		break;
	case PARITY_ODD:
		rv |= LCR_PEN;
		break;
	default:
		break;
	}
	switch(stopbits) {
	default:
	case 1:
		rv |= LCR_1_STB;
		break;
	case 2:
		rv |= LCR_2_STB;
		break;
	}

	divisor = clock_get_frequency(CLK_UART) / (baudrate * 16);
	rLCR(port) = rv | LCR_DLAB;	/* enable the divisor latch */	
	rDLL(port) = divisor & 0xff;
	rDLM(port) = (divisor >> 8) & 0xff;
	rLCR(port) = rv;		/* disable the divisor latch */
	
	/* reset & re-enble FIFO with trigger at 1 byte */
	rFCR(port) = FCR_FIFO_EN | FCR_RXSR | FCR_TXSR | FCR_TL1;
	
	return(0);
}

int
uart_hw_set_rx_buf(uint32_t port, bool interrupt, size_t buffer_len)
{
	/* optional interface not implemented */
	return -1;
}

/*
 * uart_hw_getc
 *
 * Get a byte from the port.  Returns -1 if there isn't one ready.
 */
int
uart_hw_getc(uint32_t port)
{
	ASSERT((unsigned)port < (sizeof(uart_ports) / sizeof(uart_ports[0])));

	/* if data ready, return it */
	if (rLSR(port) & LSR_DR)
		return(rRBR(port));
	return(-1);
}

/*
 * uart_hw_putc
 *
 * Write a byte to the port.
 */
int
uart_hw_putc(uint32_t port, char c)
{

	ASSERT((unsigned)port < (sizeof(uart_ports) / sizeof(uart_ports[0])));

	/* if we can send the byte, do it */
	if (rLSR(port) & LSR_THRE) {
		rTHR(port) = c;
		return(0);
	}

	/* can't send now, caller will yield and retry */
	return(-1);
}
