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

#include <debug.h>
#include <cbuffer.h>
#include <drivers/power.h>
#include <platform.h>
#include <platform/dockfifo_config.h>
#include <platform/int.h>
#include <sys.h>
#include <sys/task.h>
#include <platform/timer.h>

#include "dockfifo_regs.h"

extern int DebugUartReady;

#define DOCKFIFO_UART_RX_LEN		(16)

// Allow a 30ms stall of wall clock time before DockFIFO starts dropping characters
#define DOCKFIFO_WR_MAX_STALL_US 	(30*1000)

static CBUFFER dockfifo_uart_rx_cbuf;
static struct task_event dockfifo_uart_rx_event;

static uint64_t prev_dockfifo_drained_time; // Last time we've seen the DockFIFO drained by an external agent
static uint64_t prev_dockfifo_spaces;	    // Previous w_stat level of the DockFIFO.
static uint32_t dockfifo_capacity;


#define INSTRUMENT_DOCKFIFO_DRAIN_COUNTER 1

#if INSTRUMENT_DOCKFIFO_DRAIN_COUNTER

#define DOCKFIFO_STAT_INCR(x) ((x)++)

static unsigned int dockfifo_stat_active_stall_loops;
static unsigned int dockfifo_stat_dropped_characters;

#else
#define DOCKFIFO_STAT_INCR(x)
#endif

//=======================
// Local funtions
//=======================

static int dockfifo_drain_on_stall()
{
	// Called when DockFIFO runs out of spaces.
	// Check if the DockFIFO reader has stalled. If so, empty the DockFIFO ourselves.
	// Return number of bytes drained.

	if (timer_ticks_to_usecs(timer_get_ticks() - prev_dockfifo_drained_time) >= DOCKFIFO_WR_MAX_STALL_US) {
		// It's been more than DOCKFIFO_WR_MAX_STALL_US and nobody read from the FIFO
		// Drop a character.
		DOCKFIFO_STAT_INCR(dockfifo_stat_dropped_characters);

		(void)rDBGFIFO_R_DATA(DOCKFIFO_UART_READ, 1);
		prev_dockfifo_spaces++;
		return 1;
	}
	return 0;
}

static int32_t dockfifo_uart_write_byte(uint8_t byte)
{

	/**
	 * DockFIFO Draining algorithm:
	 *
	 * We want DockFIFO UART to try hard to preserve characters as long as someone is
	 * listening to the DockFIFO. But we also want DockFIFO to drop characters rapidly
	 * when the FIFO is full and nobody is listening.
	 *
	 * So, when the DockFIFO is full, we will hang and poll for a max of DOCKFIFO_WR_MAX_STALL_US
	 * If this time expires, we will begin instantly dropping the oldest character when we notice
	 * the DockFIFO to be full (e.g. nobody is listening to DockFIFO).
	 *
	 * But, the moment we see someone start emptying the DockFIFO, the timer resets.
	 * This will achieve a good balance between making sure we don't drop chars when the host is listening,
	 * and making sure UART write overhead is low when nobody is attached to it.
	 *
	 **/

	// Atomically check for free space in write fifo and enqueue a byte.
	enter_critical_section();
	for (;;) {
		uint32_t spaces = rDBGFIFO_W_STAT(DOCKFIFO_UART_WRITE) & 0xffff;

		if (spaces >= dockfifo_capacity || spaces > prev_dockfifo_spaces) {
			// More spaces showed up. That can only mean someone read the FIFO.
			// Note that if the DockFIFO is empty we cannot tell if someone is listening,
			// we can only give them the benefit of the doubt.

			prev_dockfifo_drained_time = timer_get_ticks();
		}
		prev_dockfifo_spaces = spaces;

		if (spaces > 0 || dockfifo_drain_on_stall()) {
			// We either had spaces, or just kicked out a stale byte on a stalled DockFIFO.
			break;
		}
		exit_critical_section();
		
		// If we reached here, the DockFIFO is still full, probably due to heavy UART
		// traffic with an active reader.
		DOCKFIFO_STAT_INCR(dockfifo_stat_active_stall_loops);

		enter_critical_section();
	}
	rDBGFIFO_W_DATA(DOCKFIFO_UART_WRITE, 1) = byte;
	prev_dockfifo_spaces--; // After writing a byte we have one fewer space than previously expected.
	exit_critical_section();
	return 0;
}

static int32_t dockfifo_uart_write(const uint8_t *data, size_t count, bool wait)
{
	RELEASE_ASSERT(data != NULL);
	int32_t ret = 0;
	for (size_t i = 0; ret == 0 && i < count; ++i) {
		ret = dockfifo_uart_write_byte(data[i]);
	}
	return ret;
}

static int32_t dockfifo_uart_read(uint8_t *data, size_t count, bool wait)
{
	int32_t bytes_read = 0;

	RELEASE_ASSERT(data != NULL);

retry_read:
	if (wait && (!cb_readable_size(&dockfifo_uart_rx_cbuf))) 
		event_wait(&dockfifo_uart_rx_event);

	/* disable rx interrupt */
	mask_int(dockfifo_configs[DOCKFIFO_UART_READ].irq);

	bytes_read += cb_read_unsafe(&dockfifo_uart_rx_cbuf, data + bytes_read, count - bytes_read);

	/* enable rx interrupt */
	unmask_int(dockfifo_configs[DOCKFIFO_UART_READ].irq);

	if (((size_t)bytes_read < count) && wait)
		goto retry_read;

	return bytes_read;
}

static int dockfifo_uart_reader_task(void *arg)
{
	for(;;) {
		char c;
		int32_t len;

		len = dockfifo_uart_read((uint8_t *)&c, 1, true);
		if ((len > 0) && ((DebugUartReady & kPowerNVRAMiBootDebugIAPSerial) != 0)) 
			debug_pushchar(c);
	}
	return 0;
}

static void dockfifo_uart_interrupt(void *arg)
{
	int32_t which_uart = (int32_t)arg;

	if (which_uart == DOCKFIFO_UART_READ) { // Rx
		while (((rDBGFIFO_R_STAT(DOCKFIFO_UART_READ) & 0xffff) != 0) && cb_free_space(&dockfifo_uart_rx_cbuf))
			cb_putc_unsafe(&dockfifo_uart_rx_cbuf, (rDBGFIFO_R_DATA(DOCKFIFO_UART_READ, 1) >> 8) & 0xff);

		/* signal reader */
		event_signal(&dockfifo_uart_rx_event);
	}
}

//=======================
// Global funtions
//=======================

int32_t dockfifo_uart_init()
{
	// Setup hardware
	clock_gate(CLK_SPU, true);
	clock_gate(CLK_DOCKFIFO, true);

	// reset fifos
	rDBGFIFO_CNFG(DOCKFIFO_UART_READ) = (1 << 31);
	rDBGFIFO_CNFG(DOCKFIFO_UART_WRITE) = (1 << 31);
	spin(1);
	rDBGFIFO_CNFG(DOCKFIFO_UART_READ) = (0 << 31);
	rDBGFIFO_CNFG(DOCKFIFO_UART_WRITE) = (0 << 31);

	// Disable autodraining of the FIFO. We now purely manage it in software.
	rDBGFIFO_DRAIN(DOCKFIFO_UART_WRITE) = 0;

	// Empty the DockFIFO by draining it until OCCUPANCY is 0, then measure its capacity
	while (rDBGFIFO_R_DATA(DOCKFIFO_UART_WRITE, 3) & 0x7F);

	dockfifo_capacity = rDBGFIFO_W_STAT(DOCKFIFO_UART_WRITE) & 0xffff;

	cb_create(&dockfifo_uart_rx_cbuf, DOCKFIFO_UART_RX_LEN);
	event_init(&dockfifo_uart_rx_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

	install_int_handler(dockfifo_configs[DOCKFIFO_UART_READ].irq, &dockfifo_uart_interrupt, (void *)DOCKFIFO_UART_READ);
	unmask_int(dockfifo_configs[DOCKFIFO_UART_READ].irq);

	task_start(task_create("dockfifo_uart reader", dockfifo_uart_reader_task, NULL, 0x200));

	return 0;
}

int32_t dockfifo_uart_putc(char c)
{
	return dockfifo_uart_write((uint8_t *)&c, 1, true);
}

int32_t dockfifo_uart_getc(bool wait)  /* returns -1 if no data available */
{
	char c;
	int32_t len;

	len = dockfifo_uart_read((uint8_t *)&c, 1, wait);
	if (len == 0)
		return -1;

	return c;
}

void dockfifo_enable_clock_gating(int num)
{
	rDBGFIFO_CNFG(num) |= DBGFIFO_CNFG_CG_ENA;
}