/* -*- tab-width: 8; Mode: C; c-basic-offset: 8; indent-tabs-mode: t -*-
 *
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

/*
 * Register layout for a memory-mapped 16x50-style UART.
 *
 * Register and bit names/definitions from the National Semiconductor PC16550D datasheet.
 *
 * Note that the register set described here is a subset containing just those registers
 * useful for the iBoot environment.  In particular, no modem support registers/definitions
 * are included.
 */

#ifndef __UART_16x50_H
#define __UART_16x50_H

/* allow an override of the spacing between registers */
#ifndef UART16x50_REGISTER_STRIDE
# define UART16x50_REGISTER_STRIDE	4
#endif

/* allow an override of the datatype used for reading/writing the registers */
#ifndef UART16x50_REGISTER_TYPE
# define UART16x50_REGISTER_TYPE	uint32_t
#endif

/* produce a pointer to a UART register, assuming base registers in uart_ports */
#define UART_REG(_port, _offset) \
	(*(volatile UART16x50_REGISTER_TYPE *)((uint8_t *)uart_ports[(_port)] + ((_offset) * UART16x50_REGISTER_STRIDE)))

//
// UART Registers for UART _port
//
#define rRBR(_port)	UART_REG(_port, 0)	// receiver buffer register (RO)
#define rTHR(_port)	UART_REG(_port, 0)	// transmitter holding register (WO)
#define rIER(_port)	UART_REG(_port, 1)	// interrupt enable register
#define rIIR(_port)	UART_REG(_port, 2)	// interrupt ident. register (RO)
#define rFCR(_port)	UART_REG(_port, 2)	// FIFO control register (WO)
#define rLCR(_port)	UART_REG(_port, 3)	// line control register 
#define rLSR(_port)	UART_REG(_port, 5)	// line status register
#define rDLL(_port)	UART_REG(_port, 0)	// divisor latch (LS)
#define rDLM(_port)	UART_REG(_port, 1)	// divisor latch (MS)

// IER
#define IER_ERBFI	(1<<0)			// enable received data available interrupt
#define IER_ETBEI	(1<<1)			// enable transmit holding register empty interrupt
#define IER_ELSI	(1<<2)			// enable receiver line status interrupt

// IIR
#define IIR_NOTPEND	(1<<0)			// interrupt not pending
#define IIR_IID_MASK	0xe			// interrupt pending/ID mask
#define IIR_IID_THRE	0x2			// transmitter holding register empty
#define IIR_IID_RBF	0x4			// received data available
#define IIR_IID_LSR	0x6			// receiver line status
#define IIR_IID_CTI	0xc			// character timeout indication

// FCR
#define FCR_FIFO_EN	(1<<0)			// FIFO enable
#define FCR_RXSR	(1<<1)			// reciever fifo reset
#define FCR_TXSR	(1<<2)			// transmitter fifo reset
#define FCR_TL1		(0<<6)			// RCVR FIFO trigger level - 1 byte
#define FCR_TL4		(1<<6)			// 4 bytes
#define FCR_TL8		(2<<6)			// 8 bytes
#define FCR_TL14	(3<<6)			// 14 bytes

// LCR
#define LCR_WLS_8	(3<<0)			// 8 bit data (seriously, who uses anything else?)
#define LCR_1_STB	(0<<2)			// 1 stop bit
#define LCR_2_STB	(1<<2)			// 2 stop bits
#define LCR_PEN		(1<<3)			// parity enable
#define LCR_EPS		(1<<4)			// even parity select
#define LCR_STICK	(1<<5)			// stick parity
#define LCR_SBRK	(1<<6)			// set break
#define LCR_DLAB	(1<<7)			// divisor latch access bit

// LSR
#define LSR_DR		(1<<0)			// data ready 
#define LSR_OE		(1<<1)			// overrun 
#define LSR_PE		(1<<2)			// parity error 
#define LSR_FE		(1<<3)			// framing error 
#define LSR_BI		(1<<4)			// break 
#define LSR_THRE	(1<<5)			// transmit holding register empty 
#define LSR_TEMT	(1<<6)			// transmitter empty 
#define LSR_RFERR	(1<<7)			// error in receiver FIFO

#endif /* __UART_16x50_H */
