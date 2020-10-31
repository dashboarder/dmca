/*
 * Copyright (C) 2007-2009 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
//#include <string.h>
#include <arch.h>
#include <debug.h>
#include <cbuffer.h>
#include <drivers/power.h>
#include <drivers/uart.h>
#include <platform.h>
#include <platform/int.h>
#include <platform/clocks.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <sys.h>
#include <sys/task.h>
#if WITH_TARGET_CONFIG
#include <target/uartconfig.h>
#elif WITH_PLATFORM_UARTCONFIG
#include <platform/uartconfig.h>
#endif

#include "uart.h"

#ifndef UARTS_MASK
#define UARTS_MASK		((1 << UARTS_COUNT) - 1)
#endif

#ifndef UART_FLOWCONTROL_MASK
#define UART_FLOWCONTROL_MASK		(0x0E)
#endif


#define UART_RX_LEN	16

// defined in sys/debug.c
extern int DebugUartReady;

static void uart_interrupt(void *);
static int tx_fifo_count(int port);
static int rx_fifo_count(int port);
static void uart_dump_config(int port);
static void uart_clear_rx(int port);
static int uart_rx_complete(int port);
static void uart_clear_tx(int port);
static int uart_tx_complete(int port);
static void drain_rx_fifo(int port);

typedef struct {
	volatile u_int32_t *ulcon;
	volatile u_int32_t *ucon;
	volatile u_int32_t *ufcon;
	volatile u_int32_t *umcon;
	volatile u_int32_t *utrstat;
	volatile u_int32_t *uerstat;
	volatile u_int32_t *ufstat;
	volatile u_int32_t *umstat;
	volatile u_int32_t *utxh;
	volatile u_int32_t *urxh;
	volatile u_int32_t *ubrdiv;
	volatile u_int32_t *uabrcnt;
	int		    clock;
	u_int32_t	    irq;
} uart_regs_t;

static const uart_regs_t uart_regs[] = {
#if UARTS_COUNT > 0
	{ &rULCON0, &rUCON0, &rUFCON0, &rUMCON0, &rUTRSTAT0, &rUERSTAT0, &rUFSTAT0, &rUMSTAT0, &rUTXH0, &rURXH0, &rUBRDIV0, &rUABRCNT0, CLK_UART0, INT_UART0 },
#endif
#if UARTS_COUNT > 1
	{ &rULCON1, &rUCON1, &rUFCON1, &rUMCON1, &rUTRSTAT1, &rUERSTAT1, &rUFSTAT1, &rUMSTAT1, &rUTXH1, &rURXH1, &rUBRDIV1, &rUABRCNT1, CLK_UART1, INT_UART1 },
#endif
#if UARTS_COUNT > 2
	{ &rULCON2, &rUCON2, &rUFCON2, &rUMCON2, &rUTRSTAT2, &rUERSTAT2, &rUFSTAT2, &rUMSTAT2, &rUTXH2, &rURXH2, &rUBRDIV2, &rUABRCNT2, CLK_UART2, INT_UART2 },
#endif
#if UARTS_COUNT > 3
	{ &rULCON3, &rUCON3, &rUFCON3, &rUMCON3, &rUTRSTAT3, &rUERSTAT3, &rUFSTAT3, &rUMSTAT3, &rUTXH3, &rURXH3, &rUBRDIV3, &rUABRCNT3, CLK_UART3, INT_UART3 },
#endif
#if UARTS_COUNT > 4
	{ &rULCON4, &rUCON4, &rUFCON4, &rUMCON4, &rUTRSTAT4, &rUERSTAT4, &rUFSTAT4, &rUMSTAT4, &rUTXH4, &rURXH4, &rUBRDIV4, &rUABRCNT4, CLK_UART4, INT_UART4 },
#endif
#if UARTS_COUNT > 5
	{ &rULCON5, &rUCON5, &rUFCON5, &rUMCON5, &rUTRSTAT5, &rUERSTAT5, &rUFSTAT5, &rUMSTAT5, &rUTXH5, &rURXH5, &rUBRDIV5, &rUABRCNT5, CLK_UART5, INT_UART5 },
#endif
#if UARTS_COUNT > 6
	{ &rULCON6, &rUCON6, &rUFCON6, &rUMCON6, &rUTRSTAT6, &rUERSTAT6, &rUFSTAT6, &rUMSTAT6, &rUTXH6, &rURXH6, &rUBRDIV6, &rUABRCNT6, CLK_UART6, INT_UART6 },
#endif
#if UARTS_COUNT > 7
	{ &rULCON7, &rUCON7, &rUFCON7, &rUMCON7, &rUTRSTAT7, &rUERSTAT7, &rUFSTAT7, &rUMSTAT7, &rUTXH7, &rURXH7, &rUBRDIV7, &rUABRCNT7, CLK_UART7, INT_UART7 },
#endif
#if UARTS_COUNT > 8
	{ &rULCON8, &rUCON8, &rUFCON8, &rUMCON8, &rUTRSTAT8, &rUERSTAT8, &rUFSTAT8, &rUMSTAT8, &rUTXH8, &rURXH8, &rUBRDIV8, &rUABRCNT8, CLK_UART8, INT_UART8 },
#endif
#if UARTS_COUNT > 9
#error "Need to add more entries to uart_regs"
#endif
};

enum uart_mode {
	MODE_POLL,
	MODE_INT,
};

enum uart_clk {
	PCLK,
	NCLK,
	SLOWCLK,
};

static struct uart_status {
	int port;
	int baud;
	int sample_rate;
	bool flow_en;
	bool fifo_en;
	enum uart_mode mode;
	bool nclk; // false is pclk

	/* Interrupt mode */
	CBUFFER rx_cbuf;

	struct task_event rx_event;

	/* completion flags */
	volatile int tx_complete;
	volatile int rx_complete;
} uart_status[UARTS_COUNT];

static void uart_set_sample_rate(int port, int rate);
static void uart_set_baud_rate(int port, u_int32_t baud_rate);
static void uart_set_flow_control(int port, bool flow);
static void uart_set_fifo_enable(int port, bool enable);
static void uart_set_mode(int port, enum uart_mode mode);
static void uart_set_clk(int port, enum uart_clk clk);

static int uart_read(int port, void *_buf, size_t len, bool wait);
static int uart_write(int port, const void *_buf, size_t len, bool wait);

static const uart_regs_t *uregs(int port)
{	
	if (port < 0 || port >= UARTS_COUNT || !(UARTS_MASK & (1 << port)))
		panic("invalid uart\n");

	return &uart_regs[port];
}

static void uart_set_sample_rate(int port, int rate)
{
	const uart_regs_t *regs = uregs(port);

#if UART_VERSION == 0
	switch (rate) {
		case 4:
			*regs->ubrdiv = (*regs->ubrdiv & ~(3<<16)) | (2<<16);
			uart_status[port].sample_rate = 4;
			break;
		case 8:
			*regs->ubrdiv = (*regs->ubrdiv & ~(3<<16)) | (1<<16);
			uart_status[port].sample_rate = 8;
			break;
		case 16:
			*regs->ubrdiv = (*regs->ubrdiv & ~(3<<16)) | (0<<16);
			uart_status[port].sample_rate = 16;
			break;
		default:
			printf("uart_set_sample_rate: invalid rate %d\n", rate);
			return;
	}
#else
	if ((rate < 4) || (rate > 16)) {
		printf("uart_set_sample_rate: invalid rate %d\n", rate);
		return;
	}
	*regs->ubrdiv = (*regs->ubrdiv & ~(0xF<<16)) | ((16 - rate)<<16);
	uart_status[port].sample_rate = rate;
#endif

	/* based on the new info, set the baud rate */
	uart_set_baud_rate(port, uart_status[port].baud);
}

static void uart_set_clk(int port, enum uart_clk clk)
{
	const uart_regs_t *regs = uregs(port);

	switch (clk) {
		case PCLK:
			uart_status[port].nclk = false;
			*regs->ucon &= ~(1<<10);
			break;
		case NCLK:
			uart_status[port].nclk = true;
			*regs->ucon |= (1<<10);
			break;
		default:
			panic("uart_set_clk: invalid clock\n");
	}

	/* based on the new info, set the baud rate */
	uart_set_baud_rate(port, uart_status[port].baud);
}

static void uart_set_baud_rate(int port, u_int32_t baud_rate)
{
	const uart_regs_t *regs = uregs(port);
	u_int32_t baud_low, actual_baud;
	int div;
	u_int32_t clk;

	if (uart_status[port].nclk)
		clk = clock_get_frequency(CLK_NCLK);
	else
		clk = clock_get_frequency(CLK_PCLK);
#if UART_CLOCK_SOURCE_OVERRIDE
	clk = UART_CLOCK_SOURCE_OVERRIDE;
#endif
	if (uart_status[port].sample_rate == 0) {
		// sample_rate isn't set yet, can't calculate baud.
		return;
	}
	div = clk / (baud_rate * uart_status[port].sample_rate);

	actual_baud = clk / ((div + 0) * uart_status[port].sample_rate);
	baud_low    = clk / ((div + 1) * uart_status[port].sample_rate);

	if ((baud_rate - baud_low) < (actual_baud - baud_rate)) {
		div++;
		actual_baud = baud_low;
	}

	*regs->ubrdiv = (*regs->ubrdiv & ~0xFFFF) | (div - 1);

	printf("uart_set_baud_rate: port %d, baud %d, sample %d, divider %d, actual baud %d\n", 
			port, baud_rate, uart_status[port].sample_rate, div - 1, actual_baud);

	uart_status[port].baud = baud_rate;
}   

static void uart_set_flow_control(int port, bool flow)
{
	const uart_regs_t *regs = uregs(port);

	if (port == 0) {
		printf("%s: uart 0 does not support flow control\n", __FUNCTION__);
		return;
	}

	if (flow) {
		*regs->umcon = 0x10; // set auto flow control
	} else {
		*regs->umcon = 0x01; // assert RTS
	}
	uart_status[port].flow_en = flow;
}

static void uart_set_fifo_enable(int port, bool enable)
{
	const uart_regs_t *regs = uregs(port);

	if (enable) {
		*regs->ufcon = 0x6; // reset both fifos
		*regs->ufcon = (0<<6)|(0<<4)|0x1; // enable fifo, tx trigger level 0, rx trigger level 4
	} else {
		*regs->ufcon = 0; // disable all fifos
	}
	uart_status[port].fifo_en = enable;
}

static void uart_set_mode(int port, enum uart_mode mode)
{
	const uart_regs_t *regs = uregs(port);
	int old_mode;

	printf("uart_set_mode: port %d, mode %d\n", port, mode);
	
	old_mode = uart_status[port].mode;
	uart_status[port].mode = mode;

	switch (mode) {
		case MODE_POLL: {
			// no interrupts in polled mode
			*regs->ucon &= ~((0xf<<11)|(1<<7));
			*regs->ucon = (*regs->ucon & ~0xf) | 0x5; // tx/rx polled/int mode
			break;
		}
		case MODE_INT: {
			// interrupt mode
			*regs->ucon = (*regs->ucon & ~(0xf<<11)) | ((0xb<<11)|(1<<7)); //enable read/error/timeout and disable write interrupt
			*regs->ucon = (*regs->ucon & ~0xf) | 0x5; // tx/rx polled/int mode
			break;
		}
		default:
			panic("uart_set_mode: unsupport modei\n ");

	}
}

static void uart_interrupt(void *arg)
{
	/* to-do: the interrupt routine */
	int port = (int)arg;
	const uart_regs_t *regs = uregs(port);
	u_int32_t utr_status;
	u_int32_t uer_status;

	utr_status = *regs->utrstat;
	*regs->utrstat = utr_status;
	uer_status = *regs->uerstat;
	*regs->uerstat = uer_status;

	if (utr_status & (1 << 6)) { // Error interrupt
		/* Error, eat up that byte and move on */
		volatile int hole;
		hole = *regs->urxh;
	}

	if (utr_status & (3 << 3)) { // Rx or timeout interrupt
		CBUFFER *pcb = &uart_status[port].rx_cbuf;
		if (uart_status[port].fifo_en) {
			while ((rx_fifo_count(port) > 0) && cb_free_space(pcb)) {
				cb_putc_unsafe(pcb, *regs->urxh);
			}
		} else {
			if (utr_status & (1 << 0)) {
				cb_putc_unsafe(pcb, *regs->urxh);
			}
		}
		/* signal reader */
		event_signal(&uart_status[port].rx_event);
	}
	return;
}
 
static void uart_dump_config(int port)
{
	if (port >= UARTS_COUNT) return;

	if (port < 0) {
		for (port = 0; port < UARTS_COUNT; port++) {
			uart_dump_config(port);
		}
		return;
	}

	printf("uart %d config: baud %d sample %d flow %d fifo %d ", port, 
			uart_status[port].baud, uart_status[port].sample_rate, 
			uart_status[port].flow_en, uart_status[port].fifo_en);
	switch (uart_status[port].mode) {
		case MODE_POLL:
			printf("polled ");
			break;
		case MODE_INT:
			printf("interrupt-mode");
			break;
	}
	printf("\n");
}

static int uart_debug_reader_task(void *arg)
{
	int	port = (int)arg;
	u_int8_t port_mask = 0;

	switch (port) {
#if defined(DEBUG_SERIAL_PORT)
		case DEBUG_SERIAL_PORT  : port_mask = kPowerNVRAMiBootDebugIAPSerial; break;
#endif
#if defined(DEBUG_SERIAL_PORT2)
		case DEBUG_SERIAL_PORT2 : port_mask = kPowerNVRAMiBootDebugAltSerial; break;
#endif
		default: break;
	}

	for(;;) {
		char c;
		int len;

		/* use interrupt mode  */
		len = uart_read(port, &c, 1, true);
		if ((len > 0) && ((DebugUartReady & port_mask) != 0)) {
			debug_pushchar(c);
		}
	}
	return 0;
}

int uart_init(void)
{
	int			port, oversample = 16;
	enum uart_clk		clock = NCLK;
	const uart_regs_t	*regs;

#ifdef UART_CLOCK_OVERRIDE
	clock = UART_CLOCK_OVERRIDE;
#endif

#ifdef UART_OVERSAMPLE_OVERRIDE
	oversample = UART_OVERSAMPLE_OVERRIDE;
#endif

	for (port = 0; port < UARTS_COUNT; port++) {
		if (!(UARTS_MASK & (1 << port)))
			continue;

		regs = uregs(port);

		cb_create(&uart_status[port].rx_cbuf, UART_RX_LEN);

		event_init(&uart_status[port].rx_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

		uart_status[port].port = port;
		uart_status[port].baud = 115200;

		/* make sure the clock is on for this block */
		clock_gate(regs->clock, true);

		/* put the uarts in a default state */
		*regs->ulcon = 0x03; /* 81N, not IR */
		*regs->ucon  = 0x05; /*interrupt/polled mode, PCLK source, all ints disabled */

#if defined(HDQGAUGE_SERIAL_PORT)
		if (HDQGAUGE_SERIAL_PORT == port) {
		    uart_status[port].baud = 57600;
		    *regs->ulcon = 0x07;    /* 82N */
		}
#endif

		uart_set_clk(port, clock);

		/* Set sample rate to 16x to avoid issues with the lower ones*/
		uart_set_sample_rate(port, oversample);

		uart_set_flow_control(port, UART_FLOWCONTROL_MASK & (1 << port));

		uart_set_fifo_enable(port, true);

		uart_set_mode(port, MODE_POLL);

		/* register the interrupt handler */
		install_int_handler(regs->irq, &uart_interrupt, (void *)port);
		unmask_int(regs->irq);
	}

#if defined(DEBUG_SERIAL_PORT)
	if (DEBUG_SERIAL_PORT != 0)
		uart_set_flow_control(DEBUG_SERIAL_PORT, false);
	/* the fifo trigger level for Rx can not be zero -- 4 bytes is the lowest, so no fifo */
	uart_set_fifo_enable(DEBUG_SERIAL_PORT, false);
	uart_set_mode(DEBUG_SERIAL_PORT, MODE_INT);
	task_start(task_create("uart reader", uart_debug_reader_task, (void *)DEBUG_SERIAL_PORT, 0x200));
#endif
#if defined(DEBUG_SERIAL_PORT2)
	if (DEBUG_SERIAL_PORT2 != 0)
		uart_set_flow_control(DEBUG_SERIAL_PORT2, false);
	/* the fifo trigger level for Rx can not be zero -- 4 bytes is the lowest, so no fifo */
	uart_set_fifo_enable(DEBUG_SERIAL_PORT2, false);
	uart_set_mode(DEBUG_SERIAL_PORT2, MODE_INT);
	task_start(task_create("uart reader2", uart_debug_reader_task, (void *)DEBUG_SERIAL_PORT2, 0x200));
#endif

#if DEBUG_UART_ENABLE_DEFAULT
	DebugUartReady |= kPowerNVRAMiBootDebugIAPSerial | kPowerNVRAMiBootDebugAltSerial;
#endif
	return 0;
}

int uart_hw_init_extended(u_int32_t port, u_int32_t baudrate, u_int32_t databits, enum uart_parity parity, u_int32_t stopbits)
{
	const uart_regs_t *regs = uregs(port);
	u_int32_t ulcon = 0;

	uart_status[port].baud = baudrate;

	switch (databits) {
		case 5:			ulcon |= (0 << 0);	break;
		case 6:			ulcon |= (1 << 0);	break;
		case 7:			ulcon |= (2 << 0);	break;
		case 8:			ulcon |= (3 << 0);	break;
		default:		return -1;
	}

	switch (parity) {
		case PARITY_NONE:	ulcon |= (0 << 3);	break;
		case PARITY_ODD:	ulcon |= (4 << 3);	break;
		case PARITY_EVEN:	ulcon |= (5 << 3);	break;
		default:		return -1;
	}

	switch(stopbits) {
		case 1:			ulcon |= (0 << 2);	break;
		case 2:			ulcon |= (1 << 2);	break;
		default:		return -1;
	}

	uart_set_fifo_enable(port, false);

	*regs->ulcon = ulcon;

	uart_set_baud_rate(port, uart_status[port].baud);

	uart_set_fifo_enable(port, true);

	return 0;
}

int uart_hw_set_rx_buf(u_int32_t port, bool interrupt, size_t buffer_len)
{
	/* Reconfigure a port for interrupt/polled and explicit buffer len */
	struct uart_status *st = &uart_status[port];
	*uregs(port)->ucon &= ~(1<<12); // disable interrupt
	if (cb_initialized(&st->rx_cbuf))
		cb_free(&st->rx_cbuf);
	cb_create(&st->rx_cbuf, buffer_len);
	uart_set_mode(port, interrupt ? MODE_INT : MODE_POLL);
	return 0;
}

static int uart_write(int port, const void *_buf, size_t len, bool wait)
{
	const uart_regs_t *regs = uregs(port);
	const u_int8_t *buf = (const u_int8_t *)_buf;
	u_int32_t i;
	switch (uart_status[port].mode) {
		case MODE_INT:
		case MODE_POLL: {
			for (i=0; i < len; i++) {
				if (uart_status[port].fifo_en) {
					while ((*regs->ufstat) & 0x200) {
					}
				} else {
					while (!((*regs->utrstat) & 0x04))
						;
				}
				if (uart_status[port].flow_en) {
					while (((*regs->utrstat) & 4) == 0);	// Wait Transmitter Empty
					while (((*regs->umstat) & 1) == 0);	// Wait CTS Asserted
				}
				*regs->utxh = buf[i];
			}
			break;
		}
		default:
			panic("uart_write: unhandled mode\n");
	}
			
	return len;
}

static int tx_fifo_count(int port)
{
	const uart_regs_t *regs = uregs(port);
	int fifo_count = *regs->ufstat & ((1<<9)|(0xf<<4));
	fifo_count = ((fifo_count >> 4) & 0xf) | (fifo_count >> 5);
	return fifo_count;
}

static int rx_fifo_count(int port)
{
	const uart_regs_t *regs = uregs(port);
	int fifo_count = *regs->ufstat & ((1<<8)|0xf);
	fifo_count = (fifo_count & 0xf) | (fifo_count >> 4);
	return fifo_count;
}

static void drain_rx_fifo(int port)
{
	volatile char hole;

	if (uart_status[port].fifo_en) {
		const uart_regs_t *regs = uregs(port);

		while (rx_fifo_count(port) > 0) {
			hole = *regs->urxh;
		}
	}
}

static int uart_read(int port, void *_buf, size_t len, bool wait)
{
	const uart_regs_t *regs = uregs(port);
	u_int8_t *buf = (u_int8_t *)_buf;
	u_int32_t i = 0;

	switch (uart_status[port].mode) {
		case MODE_POLL: {
			volatile int hole;
			for (i=0; i < len; i++) {
retry_polled_read:
				if (uart_status[port].fifo_en) {
					while (rx_fifo_count(port) == 0) {
						if (!wait)
							return i;
						task_yield();
					}
				} else {
					while (!((*regs->utrstat) & 0x01)) {
						if (!wait)
							return i;
						task_yield();
					}
				}
				if (*regs->uerstat) {
					/* some sort of error queued up, eat this character and move on */
				   	hole = *regs->urxh;
					goto retry_polled_read;
				}
				buf[i] = *regs->urxh;
			}
			break;
		}
		case MODE_INT: {
			/* in interrupt mode, always wait */
			CBUFFER *pcb = &uart_status[port].rx_cbuf;
retry_intr_read:
			if (wait && (!cb_readable_size(pcb)))
				event_wait(&uart_status[port].rx_event);
			
			/* disable rx interrupt */
			*regs->ucon &= ~(1<<12);

			i += cb_read_unsafe(pcb, buf + i, len - i);

			/* enable rx interrupt */
			*regs->ucon |= (1<<12);

			if ((i < len) && wait)
				goto retry_intr_read;
			break;
		}
		default:
			panic("uart_read: unhandled mode\n");
	}

	return i;
}

static int uart_rx_complete(int port)
{
	return uart_status[port].rx_complete;
}

int uart_putc(int port, char c)
{
	return uart_write(port, &c, 1, true);
}

int uart_puts(int port, const char *s)
{
	int len = strlen(s);
	uart_write(port, s, len, true);
	return 0;
}

int uart_getc(int port, bool wait)  /* returns -1 if no data available */
{
	char c;
	int len;

	len = uart_read(port, &c, 1, wait);
	if (len == 0)
		return -1;

	return c;
}

int uart_send_break(int port, bool enable)
{
	const uart_regs_t *regs = uregs(port);
	if (enable)
		*regs->ucon |= (1 << 4);
	else
		*regs->ucon &= ~(1 << 4);
	    
	return 0;
}
