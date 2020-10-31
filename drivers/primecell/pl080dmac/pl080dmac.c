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
#include <drivers/dma.h>
#include <platform.h>
#include <platform/int.h>
#include <platform/pl080dmac_config.h>
#include <platform/soc/hwdmachannels.h>
#include <sys/task.h>
#include <sys/menu.h>
#include <sys.h>

#include "pl080dmac_regs.h"

#define CHANNEL_STATUS_UNINIT		0
#define CHANNEL_STATUS_IDLE		1
#define CHANNEL_STATUS_BUSY		2
#define CHANNEL_STATUS_BUSY_UNINIT 	3

struct pl080dmac_channel {
	uint8_t				status;

	uint32_t			actual_xfer_size;
	uint32_t			current_xfer_size;
	uint32_t			total_xferred;

	/* callback for async operations */
	dma_completion_function		callback;
	void				*callback_arg;
};

static bool pl080dmac_inited[PL080DMAC_COUNT];
static struct pl080dmac_channel pl080dmac_channels[PL080DMAC_COUNT][PL080DMAC_SUPPORTED_CHANNEL_COUNT];

static void pl080dmac_dump_channel(int dma_channel);
static void pl080dmac_tc_int_handler(void *arg);
static void pl080dmac_err_int_handler(void *arg);

void dma_init()
{
	uint8_t pl080dmac_idx;

	for (pl080dmac_idx = 0; pl080dmac_idx < PL080DMAC_COUNT; pl080dmac_idx++) {
		clock_gate(pl080dmac_configs[pl080dmac_idx].dmac_clk, true);

		if (pl080dmac_configs[pl080dmac_idx].dmac_tc_irq) {
			set_int_type(pl080dmac_configs[pl080dmac_idx].dmac_tc_irq, INT_TYPE_IRQ | INT_TYPE_LEVEL);
			install_int_handler(pl080dmac_configs[pl080dmac_idx].dmac_tc_irq, pl080dmac_tc_int_handler, (void *)pl080dmac_idx);
			unmask_int(pl080dmac_configs[pl080dmac_idx].dmac_tc_irq);
		}

		if (pl080dmac_configs[pl080dmac_idx].dmac_err_irq) {
			set_int_type(pl080dmac_configs[pl080dmac_idx].dmac_err_irq, INT_TYPE_IRQ | INT_TYPE_LEVEL);
			install_int_handler(pl080dmac_configs[pl080dmac_idx].dmac_err_irq, pl080dmac_err_int_handler, (void *)pl080dmac_idx);
			unmask_int(pl080dmac_configs[pl080dmac_idx].dmac_err_irq);
		}

		// Enable DMAC
		rPL080DMAC_CFG(pl080dmac_idx) = (1 << 0);

		pl080dmac_inited[pl080dmac_idx] = true;
	}
}

int dma_execute_cmd(uint32_t cmd, int dma_channel, void *src, 
		    void *dst, uint32_t length, uint32_t word_size, 
		    uint32_t burst_size)
{
	struct task_event	event;
	struct dma_segment	seg;
	void			*fifo;
	int32_t			ret;

	// build an SGL for the one segment
	switch (cmd & DMA_CMD_DIR_MASK) {
		case DMA_CMD_DIR_TX:
			seg.paddr = (uintptr_t)src;
			fifo = dst;
			break;
		case DMA_CMD_DIR_RX:
			seg.paddr = (uintptr_t)dst;
			fifo = src;
			break;
		default:
			return -1;
	}
	seg.length = length;

	// run the operation
	event_init(&event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	ret = dma_execute_async(cmd, dma_channel, &seg, fifo, length, word_size, burst_size,
			(dma_completion_function)event_signal, (void *)&event);

	if (!ret)
		// and block waiting for it to complete 
		event_wait(&event);

	return ret;
}

int dma_execute_async(uint32_t cmd, int dma_channel,
		      struct dma_segment *sgl, void *fifo, uint32_t length, 
		      uint32_t word_size, uint32_t burst_size,
		      dma_completion_function completion, void *completion_arg)
{
	uint8_t dmac_idx;
	struct pl080dmac_channel *ch;
	const struct pl080dmac_channel_config *ch_config;
	uint8_t expected_direction;
	uint16_t xfer_size, xfer_width;

	dmac_idx = (dma_channel >> DMA_SELECTOR_SHIFT) & DMA_CHANNEL_MASK;
	dma_channel = dma_channel & DMA_CHANNEL_MASK;
	ch = &pl080dmac_channels[dmac_idx][dma_channel];
	ch_config = &pl080dmac_configs[dmac_idx].dmac_channel_config[dma_channel];
	expected_direction = (ch_config->type == PL080DMAC_PERPH_SRC) ? DMA_CMD_DIR_RX : DMA_CMD_DIR_TX;

	// sanity check
	ASSERT(dmac_idx <= PL080DMAC_COUNT);
	ASSERT(dma_channel <= PL080DMAC_CHANNEL_COUNT);
	ASSERT(sgl);
	ASSERT(length > 0);
	ASSERT((uintptr_t)fifo == ch_config->fifo_address);

	// make sure controller is inited
	ASSERT(pl080dmac_inited[dmac_idx] != false);

	// word_size and burst_size default are in pl080dmac_config table, overrides not allowed
	ASSERT(word_size == 0);
	ASSERT(burst_size == 0);

	// validate command
	ASSERT((cmd & DMA_CMD_DIR_MASK) == expected_direction);

	// update channel status to busy but not initialized
	enter_critical_section();
	ch->status = CHANNEL_STATUS_BUSY_UNINIT;
	exit_critical_section();

	// save the completion
	ch->callback = completion;
	ch->callback_arg = completion_arg;

	// calcuate transfer size, and cap it to max_trasnfer_size  
	xfer_width = (expected_direction == DMA_CMD_DIR_RX) ? 
		(ch_config->control >> 21) & 7 : (ch_config->control >> 18) & 7;
		
	// make sure length is multiple of transfer width
	ASSERT((length % (1 << xfer_width)) == 0);
	
	xfer_size = length >> xfer_width;
	ch->actual_xfer_size = xfer_size;
	xfer_size = (xfer_size < PL080DMAC_MAX_TRANSFER_SIZE) ? xfer_size : PL080DMAC_MAX_TRANSFER_SIZE;
	ch->current_xfer_size = xfer_size;
	ch->total_xferred = 0;

	// initialize and configure channel
	// PL080DMAC TRM, section 3.2.7
	// step 2
	rPL080DMAC_INTTCCLR(dmac_idx) = (1 << dma_channel);
	rPL080DMAC_INTERRCLR(dmac_idx) = (1 << dma_channel);
	// step 3,4,6,7 
	if (ch_config->type == PL080DMAC_PERPH_SRC) {
		rPL080DMAC_CHCTRL(dmac_idx, dma_channel) = (1 << 31) | (ch_config->control) | xfer_size;
		rPL080DMAC_CHCFG(dmac_idx, dma_channel) = (1 << 15) | (1 << 14) | (ch_config->config);
		rPL080DMAC_CHSRCADDR(dmac_idx, dma_channel) = ch_config->fifo_address;
		rPL080DMAC_CHDESTADDR(dmac_idx, dma_channel) = sgl->paddr;
	} else {
		rPL080DMAC_CHCTRL(dmac_idx, dma_channel) = (1 << 31) | (ch_config->control) | xfer_size;
		rPL080DMAC_CHCFG(dmac_idx, dma_channel) = (1 << 15) | (1 << 14) | (ch_config->config);
		rPL080DMAC_CHDESTADDR(dmac_idx, dma_channel) = ch_config->fifo_address;
		rPL080DMAC_CHSRCADDR(dmac_idx, dma_channel) = sgl->paddr;
	}
	// we only support 1 segment (linear dma)
	rPL080DMAC_CHLLI(dmac_idx, dma_channel) = 0;

	// update channel status to busy
	enter_critical_section();
	ch->status = CHANNEL_STATUS_BUSY;
	exit_critical_section();

	// start channel
	rPL080DMAC_CHCFG(dmac_idx, dma_channel) |= (1 << 0);

	return 0;
}

void dma_cancel(int dma_channel)
{
	uint8_t dmac_idx;
	struct pl080dmac_channel *ch;
	utime_t start_time;

	dmac_idx = (dma_channel >> DMA_SELECTOR_SHIFT) & DMA_CHANNEL_MASK;
	dma_channel = dma_channel & DMA_CHANNEL_MASK;
	ch = &pl080dmac_channels[dmac_idx][dma_channel];

	// make sure controller is inited
	ASSERT(pl080dmac_inited[dmac_idx] != false);
	
	// sanity check
	ASSERT(dmac_idx <= PL080DMAC_COUNT);
	ASSERT(dma_channel <= PL080DMAC_CHANNEL_COUNT);

	// it's legal but not interesting to cancel a channel that hasn't been initialized
	if (CHANNEL_STATUS_UNINIT == ch->status)
		return;

	start_time = system_time();

	rPL080DMAC_CHCFG(dmac_idx, dma_channel) = 0;
	while ((rPL080DMAC_ENBLDCHNLS(dmac_idx) >> dma_channel) != 0) {
		if (time_has_elapsed(start_time, 10000))
			panic("PL080DMAC%d: channel %d timeout during abort", dmac_idx, dma_channel);
	}

	// mark the channel idle
	ch->status = CHANNEL_STATUS_IDLE;
}

void dma_use_int(int dma_channel, bool use)
{
	uint8_t dmac_idx;

	dmac_idx = (dma_channel >> DMA_SELECTOR_SHIFT) & DMA_CHANNEL_MASK;
	dma_channel = dma_channel & DMA_CHANNEL_MASK;

	if (use)
		rPL080DMAC_CHCFG(dmac_idx, dma_channel) |= ((1 << 14) | (1 << 15));
	else 
		rPL080DMAC_CHCFG(dmac_idx, dma_channel) &= ~((1 << 14) | (1 << 15));
}

bool dma_poll(int dma_channel)
{
	bool ret = false;
	struct pl080dmac_channel *ch;
	uint8_t dmac_idx;

	dmac_idx = (dma_channel >> DMA_SELECTOR_SHIFT) & DMA_CHANNEL_MASK;
	dma_channel = dma_channel & DMA_CHANNEL_MASK;
	ch = &pl080dmac_channels[dmac_idx][dma_channel];

	if (CHANNEL_STATUS_IDLE == ch->status) {
		ret = true;
		goto exit;
	}

	enter_critical_section();

	if (rPL080DMAC_INTTCSTATUS(dmac_idx) >> dma_channel)
		pl080dmac_tc_int_handler((void *)dmac_idx);
	else if (rPL080DMAC_INTERRSTATUS(dmac_idx) >> dma_channel)
		pl080dmac_err_int_handler((void *)dmac_idx);

	exit_critical_section();

	if (CHANNEL_STATUS_IDLE == ch->status) {
		ret = true;
		goto exit;
	}

exit:
	return ret;
}

int dma_set_aes(int dma_channel, struct dma_aes_config *config)
{
	// Not supported (PL080DMAC doesn't support AES)
	return -1;
}

static void pl080dmac_tc_int_handler(void *args)
{
	uint8_t pl080dmac_idx;
	uint32_t tc_intsts;
	uint8_t ch;
	struct pl080dmac_channel *ch_info;

	pl080dmac_idx = (uint8_t)args;
	
	RELEASE_ASSERT(pl080dmac_idx < PL080DMAC_COUNT);

	tc_intsts = rPL080DMAC_INTTCSTATUS(pl080dmac_idx);
	rPL080DMAC_INTTCCLR(pl080dmac_idx) = tc_intsts;

	while(tc_intsts != 0) {
		ch = 31 - clz(tc_intsts);
		ch_info = &pl080dmac_channels[pl080dmac_idx][ch];

		ch_info->total_xferred += ch_info->current_xfer_size;

		if (ch_info->total_xferred < ch_info->actual_xfer_size) {
			// start again
		}
		else {
			ch_info->status = CHANNEL_STATUS_IDLE;

			// call the user-supplied callback
			if (ch_info->callback != NULL)
				ch_info->callback(ch_info->callback_arg);
		}

		tc_intsts &= ~(1 << ch);
	}
}

static void pl080dmac_err_int_handler(void *args)
{
	uint8_t pl080dmac_idx;
	uint32_t err_intsts;

	pl080dmac_idx = (uint8_t)args;
	
	RELEASE_ASSERT(pl080dmac_idx < PL080DMAC_COUNT);

	err_intsts = rPL080DMAC_INTERRSTATUS(pl080dmac_idx);
	rPL080DMAC_INTERRCLR(pl080dmac_idx) = err_intsts;

	panic("PL080DMAC reported error on channels:0x%08x\n", err_intsts);
}
