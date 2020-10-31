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

#include <debug.h>
#include <cbuffer.h>
#include <drivers/power.h>
#include <platform.h>
#include <platform/dockchannel_config.h>
#include <platform/int.h>
#include <platform/soc/hwclocks.h>
#include <sys.h>
#include <sys/task.h>
#include <platform/timer.h>
#include "dockchannel_regs.h"

extern int DebugUartReady;

#define DOCKCHANNEL_UART_RX_LEN		(16)

// Allow a 30ms stall of wall clock time before DockFIFO starts dropping characters
#define DOCKCHANNEL_WR_MAX_STALL_US 	(60*1000)

static CBUFFER dockchannel_uart_rx_cbuf;
static struct task_event dockchannel_uart_rx_event;

static uint64_t prev_dockchannel_drained_time; // Last time we've seen the DockFIFO drained by an external agent
static uint64_t prev_dockchannel_spaces;	    // Previous w_stat level of the DockFIFO.
static uint32_t dockchannel_capacity;


#define INSTRUMENT_DOCKCHANNEL_DRAIN_COUNTER 1

#if INSTRUMENT_DOCKCHANNEL_DRAIN_COUNTER

#define DOCKCHANNEL_STAT_INCR(x) ((x)++)

static unsigned int dockchannel_stat_active_stall_loops;
static unsigned int dockchannel_stat_dropped_characters;

#else
#define DOCKCHANNEL_STAT_INCR(x)
#endif

//=======================
// Local funtions
//=======================

// <rdar://problem/20473403> M8: Remove drain timer logic in DockChannel driver after verifying on silicon
static int dockchannel_drain_on_stall()
{
	// Called when DockFIFO runs out of spaces.
	// Check if the DockFIFO reader has stalled. If so, empty the DockFIFO ourselves.
	// Return number of bytes drained.

	if (timer_ticks_to_usecs(timer_get_ticks() - prev_dockchannel_drained_time) >= DOCKCHANNEL_WR_MAX_STALL_US) {
		// It's been more than DOCKFIFO_WR_MAX_STALL_US and nobody read from the FIFO
		// Drop a character.
		DOCKCHANNEL_STAT_INCR(dockchannel_stat_dropped_characters);

		(void)rDOCKCHANNELS_DOCK_RDATA1(DOCKCHANNEL_UART);
		prev_dockchannel_spaces++;
		return 1;
	}
	return 0;
}

static int32_t dockchannel_uart_write_byte(uint8_t byte)
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
	    uint32_t spaces = rDOCKCHANNELS_DEV_WSTAT(DOCKCHANNEL_UART) & 0x1ff;

		if (spaces >= dockchannel_capacity || spaces > prev_dockchannel_spaces) {
		    // More spaces showed up. That can only mean someone read the FIFO.
		    // Note that if the DockFIFO is empty we cannot tell if someone is listening,
		    // we can only give them the benefit of the doubt.

		    prev_dockchannel_drained_time = timer_get_ticks();
		}
		prev_dockchannel_spaces = spaces;

		if (spaces > 0 || dockchannel_drain_on_stall()) {
	        // We either had spaces, or just kicked out a stale byte on a stalled DockFIFO.
	        break;
		}
		exit_critical_section();

		// If we reached here, the DockFIFO is still full, probably due to heavy UART
		// traffic with an active reader.
		DOCKCHANNEL_STAT_INCR(dockchannel_stat_active_stall_loops);

		enter_critical_section();
	}
	rDOCKCHANNELS_DEV_WDATA1(DOCKCHANNEL_UART) = byte;
	prev_dockchannel_spaces--; // After writing a byte we have one fewer space than previously expected.
	exit_critical_section();
	return 0;
}

static int32_t dockchannel_uart_write(const uint8_t *data, size_t count, bool wait)
{
	RELEASE_ASSERT(data != NULL);
	int32_t ret = 0;
	for (size_t i = 0; ret == 0 && i < count; ++i) {
		ret = dockchannel_uart_write_byte(data[i]);
	}
	return ret;
}

static int32_t dockchannel_uart_read(uint8_t *data, size_t count, bool wait)
{
	int32_t bytes_read = 0;

	RELEASE_ASSERT(data != NULL);

retry_read:
	if (wait && (!cb_readable_size(&dockchannel_uart_rx_cbuf))) 
		event_wait(&dockchannel_uart_rx_event);

	/* disable rx interrupt */
	rDOCKCHANNELS_AGENT_AP_INTR_CTRL &= ~(1<<1);
	mask_int(INT_DOCKCHANNELS_AP);

	bytes_read += cb_read_unsafe(&dockchannel_uart_rx_cbuf, data + bytes_read, count - bytes_read);

	/* enable rx interrupt */
	rDOCKCHANNELS_AGENT_AP_INTR_CTRL |= (1<<1);
	unmask_int(INT_DOCKCHANNELS_AP);

	if (((size_t)bytes_read < count) && wait)
		goto retry_read;

	return bytes_read;
}

static int dockchannel_uart_reader_task(void *arg)
{
	for(;;) {
		char c;
		int32_t len;

		len = dockchannel_uart_read((uint8_t *)&c, 1, true);
		if ((len > 0) && ((DebugUartReady & kPowerNVRAMiBootDebugIAPSerial) != 0)) 
			debug_pushchar(c);
	}
	return 0;
}

static void dockchannel_uart_interrupt(void *arg)
{
	uint32_t intr_status = rDOCKCHANNELS_AGENT_AP_INTR_STATUS;

	// Clear status to deassert interrupt
	rDOCKCHANNELS_AGENT_AP_INTR_STATUS = (1<<1);

	// Disable interrupts and mask so that we dont get interrupted while processing
	// this data
	rDOCKCHANNELS_AGENT_AP_INTR_CTRL &= ~(1<<1);
	mask_int(INT_DOCKCHANNELS_AP);

	// Read watermark interrupt
	if ((intr_status >> 1) & 1) { // Rx


		while (((rDOCKCHANNELS_DEV_RSTAT(DOCKCHANNEL_UART) & 0x1ff) != 0) && cb_free_space(&dockchannel_uart_rx_cbuf))
			cb_putc_unsafe(&dockchannel_uart_rx_cbuf, (rDOCKCHANNELS_DEV_RDATA1(DOCKCHANNEL_UART) >> 8) & 0xff);

		/* signal reader */
		event_signal(&dockchannel_uart_rx_event);
	} else {
		// Spurious interrupt since we only enable the dock channel 0 read watermark interrupt
		// Unmask interrupts and continue
		rDOCKCHANNELS_AGENT_AP_INTR_CTRL |= (1<<1);
		unmask_int(INT_DOCKCHANNELS_AP);	
	}
}

//=======================
// Global funtions
//=======================

int32_t dockchannel_uart_init()
{
	// Setup clock
	clock_gate(CLK_AOP, true);

	// reset fifos
	rDOCKCHANNELS_DEV_FIFO_CTRL(DOCKCHANNEL_UART) = 1;	
	rDOCKCHANNELS_DOCK_FIFO_CTRL(DOCKCHANNEL_UART) = 1;

	// Empty the DockChannel by draining it until OCCUPANCY is 0, then measure its capacity
	while (rDOCKCHANNELS_DOCK_RDATA3(DOCKCHANNEL_UART) & 0x7F);

	dockchannel_capacity = rDOCKCHANNELS_DEV_WSTAT(DOCKCHANNEL_UART) & 0xffff;

	// Setup drain timer
	// Each tick is an AOP clock cycle and we want the period to be 30 ms
	// which is ~ twice the kanzi sampling time
	uint32_t period = clock_get_frequency(CLK_AOP) * (0.003);

	if (period == 0)
		dprintf(DEBUG_CRITICAL, "Disabling DockChannel Drain Timer. This can lead to loss of data. \n");

	rDOCKCHANNELS_DOCK_DRAIN_CFG(DOCKCHANNEL_UART) = period; 

	dprintf(DEBUG_SPEW, "%s: Drain timer period %d \n", __FUNCTION__, period);

	cb_create(&dockchannel_uart_rx_cbuf, DOCKCHANNEL_UART_RX_LEN);
	event_init(&dockchannel_uart_rx_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

	// Using watermark for the purpose of signalling not empty fifo
	rDOCKCHANNELS_DEV_CONFIG_RD_WATERMARK(DOCKCHANNEL_UART) = 1;
	rDOCKCHANNELS_AGENT_AP_INTR_CTRL = (1<<1);

	// AP has a shared interrupt for all channels. We only enable the relevant interrupts (read watermark)
	// for dock channel in iBoot. If there are multiple dockchannel clients we will need an interrupt filter
	// type function.
	install_int_handler(INT_DOCKCHANNELS_AP, &dockchannel_uart_interrupt, (void *)NULL);
	unmask_int(INT_DOCKCHANNELS_AP);

	task_start(task_create("dockchannel_uart reader", dockchannel_uart_reader_task, NULL, 0x200));

	dprintf(DEBUG_SPEW, "%s\n", __FUNCTION__);

	return 0;
}

int32_t dockchannel_uart_putc(char c)
{
	return dockchannel_uart_write((uint8_t *)&c, 1, true);
}

int32_t dockchannel_uart_getc(bool wait)  /* returns -1 if no data available */
{
	char c;
	int32_t len;

	len = dockchannel_uart_read((uint8_t *)&c, 1, wait);
	if (len == 0)
		return -1;

	return c;
}

void dockchannel_enable_clock_gating(uint32_t channel)
{
	rDOCKCHANNELS_GLOBAL(DOCK_CHANNELS_DOCKCHANNELS_GLOBAL_DC_CFG_OFFSET) |= (1 << (4 * (channel + 1)));
}

void dockchannel_enable_top_clock_gating()
{
	rDOCKCHANNELS_GLOBAL(DOCK_CHANNELS_DOCKCHANNELS_GLOBAL_DC_CFG_OFFSET) |= 1;
}

void dockchannel_access_enable(uint32_t enable_flags)
{
	uint32_t curr_value = rDOCKCHANNELS_GLOBAL(DOCK_CHANNELS_DOCKCHANNELS_GLOBAL_DOCK_ACCESS_OFFSET);

	dprintf(DEBUG_SPEW, "%s: dockchannel_access_reg:0x%08x, requested:0x%08x\n", __FUNCTION__, curr_value, enable_flags);

	// XXX Easy WA but bug (18708180) should have been fixed. Decide whether to keep or not. 
	if (curr_value)
	        panic("dockchannel_access_enable already nonzero (0x%08x), this can cause a hang due to <rdar://problem/18708180>", curr_value);

	rDOCKCHANNELS_GLOBAL(DOCK_CHANNELS_DOCKCHANNELS_GLOBAL_DOCK_ACCESS_OFFSET) = enable_flags;

	dprintf(DEBUG_SPEW, "%s: finished, dockchannel_access_reg:0x%08x \n", __FUNCTION__, rDOCKCHANNELS_GLOBAL(DOCK_CHANNELS_DOCKCHANNELS_GLOBAL_DOCK_ACCESS_OFFSET));
}
