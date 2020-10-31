/*
 * Copyright (C) 2008-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/aes.h>
#include <drivers/dma.h>
#include <platform.h>
#include <platform/int.h>
#include <platform/soc/hwdmachannels.h>
#include <platform/soc/hwisr.h>
#include <platform/soc/hwclocks.h>
#include <sys/task.h>
#include <sys/menu.h>
#include <sys.h>

#include "cdma.h"

#define kCDMASuccess				(0)
#define kCDMAErrorNegativeSegment 	(-1)
#define kCDMAErrorSegmentsTooLong 	(-2)
#define kCDMAErrorEarlyZeroSegment 	(-3)
#define kCDMAErrorInvalidWordSize 	(-4)
#define kCDMAErrorInvalidBurstSize 	(-5)
#define kCDMAErrorNotIdle			(-6)
#define kCDMAErrorNegativeLength	(-7)
#define kCDMAErrorBadFifoAddress	(-8)
#define kCDMAErrorLargeSegment		(-9)

/*
 * Tunable: how many commands to allocate for a channel when it is set up.
 */
#ifndef CDMA_CHANNEL_CMDS
# define CDMA_CHANNEL_CMDS	32
#endif

typedef enum {
	st_NONE,
	st_REWIND,
	st_CONT
} CHANNEL_INTERRUPT_STATE;

struct cdma_channel {
	int			c_status;
#define CDMA_STATUS_UNINIT			0
#define CDMA_STATUS_IDLE			1
#define CDMA_STATUS_BUSY			2
#define CDMA_STATUS_BUSY_UNINIT 	3

	bool			c_is_read;

	u_int32_t		c_fifo_address;
	u_int32_t		c_request_id_bits;
	u_int32_t		c_ccache_c;
	u_int32_t		c_ccache_r;
	u_int32_t		c_ccache_w;
	
	struct cdma_command	*c_chain;

	/* state for longer multi-segment operations */
	u_int32_t		c_index;
	u_int32_t		c_resid;
	struct dma_segment	*c_sgl;

	/* state for re-configuring after a peripheral stops us prematurely */
	u_int32_t		c_segment_offset;
	u_int32_t		c_previous_offset;
	u_int32_t		c_previous_index;
	u_int32_t		c_previous_resid;
	
	/* callback for async operations */
	dma_completion_function	c_callback;
	void			*c_callback_arg;

	/* saved for debugging */
	u_int32_t		c_length;

	/* channel crypto config */
	struct dma_aes_config	*c_aes;
	u_int32_t		c_filter;
	u_int32_t		c_chunk;

	/* Debugging - remove later */
	CHANNEL_INTERRUPT_STATE irqState;
};

static struct cdma_channel dma_channel_softc[DMA_CHANNEL_COUNT];
#define CDMA_SOFTC(_x)	(&dma_channel_softc[(_x) - 1])	/* NB: channels are 1-based */

static void dma_init_channel(int dma_channel, bool want_interrupt);
static void dma_continue_async(u_int32_t dma_channel);
static u_int32_t dma_generate_segments(int dma_channel);
static u_int32_t dma_generate_aes_segments(int dma_channel);
static void dma_int_handler(void *arg);

static void cdma_dump_channel(int channel);

/* filter contexts in use - context 1 is owned by the AES driver and not represented here */
static u_int32_t	cdma_fc_inuse = 0;

#if CDMA_VERSION > 1
/*
 * The platform is required to provide a table of associations between channels,
 * request IDs and FIFO addresses.
 */
struct cdma_channel_config {
	u_int32_t	channel_low;
	u_int32_t	channel_high;
	bool		aes_ok;
	u_int32_t	request_id;
	u_int32_t	fifo_address;
};

static struct cdma_channel_config cdma_channel_configs[] = DMA_CHANNEL_CONFIG;
#define CDMA_CHANNEL_CONFIG_COUNT	(sizeof(cdma_channel_configs) / sizeof(struct cdma_channel_config))
#endif

void dma_init(void)
{
	dprintf(DEBUG_CRITICAL, "cdma_init()\n");
	clock_gate(CLK_CDMA, true);
	
#if CDMA_VERSION >= 4
	rCDMA_CLOCK_ON(1) = ((1 << CDMA_CLOCK_AES_WRAP) | (1 << CDMA_CLOCK_AES_CORE));
#endif /* CDMA_VERSION >= 4 */
}

static void dma_init_channel(int dma_channel, bool want_interrupt)
{
	struct cdma_channel	*cp;
	u_int32_t		vector;
	int			i;

	cp = CDMA_SOFTC(dma_channel);

	/* is the channel already set up? */
	if (unlikely(cp->c_status == CDMA_STATUS_BUSY_UNINIT)) {

		dprintf(DEBUG_SPEW, "CDMA: setting up channel %d\n", dma_channel);

		/* allocate command chain */
		cp->c_chain = memalign(sizeof(struct cdma_command) * CDMA_CHANNEL_CMDS, 32);

		/* initialise segment linkage */
		for (i = 0; i < (CDMA_CHANNEL_CMDS - 1); i++)
			cp->c_chain[i].nxt = mem_static_map_physical((uintptr_t)&cp->c_chain[i + 1]);

		/* mark channel as busy */
		cp->c_status = CDMA_STATUS_BUSY;

		/* invalidate fifo:request ID cache */
		cp->c_fifo_address = 0;
		cp->c_request_id_bits = 0;

#if CDMA_VERSION < 3
		cp->c_ccache_c = 0;
		cp->c_ccache_r = 0;
		cp->c_ccache_w = 0;

#elif CDMA_VERSION < 5
		/* Force shared by requesting cached for reads and writes */
		cp->c_ccache_c = CDMA_CACHE_CACHE;
		cp->c_ccache_r = CDMA_CACHE_CACHE;
		cp->c_ccache_w = CDMA_CACHE_CACHE;
#else
		/* Force shared by request write allocate for reads */
		/* and read allocate for writes */
		cp->c_ccache_c = CDMA_CACHE_BUFFER | CDMA_CACHE_CACHE | CDMA_CACHE_WALLOC;
		cp->c_ccache_r = CDMA_CACHE_BUFFER | CDMA_CACHE_CACHE | CDMA_CACHE_WALLOC;
		cp->c_ccache_w = CDMA_CACHE_CACHE | CDMA_CACHE_RALLOC;
#endif

		/* install interrupt handler */
		/* NB: assumes channels allocated in linear order */
		vector = INT_CDMA_DMAC1 + dma_channel - 1;
		if (want_interrupt) {
			set_int_type(vector, INT_TYPE_IRQ | INT_TYPE_LEVEL);
			install_int_handler(vector, dma_int_handler, (void *)dma_channel);
			unmask_int(vector);
		} else {
			mask_int(vector);
		}
	}
	cp->irqState = st_NONE;
}

int
dma_execute_cmd(
	u_int32_t cmd,
	int dma_channel,
	void *src,
	void *dst,
	u_int32_t length,
	u_int32_t word_size,
	u_int32_t burst_size)
{
	struct task_event	event;
	struct dma_segment	seg;
	void			*fifo;
	int32_t	ret;

	/* build an SGL for the one segment */
	switch (cmd & DMA_CMD_DIR_MASK) {
	case DMA_CMD_DIR_TX:
		seg.paddr = (u_int32_t)src;
		fifo = dst;
		break;
	case DMA_CMD_DIR_RX:
		seg.paddr = (u_int32_t)dst;
		fifo = src;
		break;
	default:
	case DMA_CMD_DIR_MEM :		/* XXX not legitimate here, to be removed */
		return EINVAL;
	}
	seg.length = length;

	/* run the operation */
	event_init(&event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	ret = dma_execute_async(
		cmd,
		dma_channel,
		&seg,
		fifo,
		length,
		word_size,
		burst_size,
		(dma_completion_function)event_signal,
		(void *)&event);

	if (!ret) {
		/* and block waiting for it to complete */
		event_wait(&event);
	}

	return ret;
}

int
dma_execute_async(
	u_int32_t cmd,
	int dma_channel,
	struct dma_segment *sgl,
	void *fifo,
	u_int32_t length, 
	u_int32_t word_size,
	u_int32_t burst_size,
	dma_completion_function completion,
	void *completion_arg)
{
	struct dma_segment *cur_sgl;
	struct cdma_channel	*cp;
	u_int32_t		dw_fld, dbs_fld;
	u_int32_t seg_length = 0;

	ASSERT(sgl);
	ASSERT((dma_channel > 2) && (dma_channel <= DMA_CHANNEL_COUNT));
	ASSERT(length > 0);
	ASSERT((length % word_size) == 0);
	
	/* 
	 * Verify that the total lenght of the transfer < 2GB(neg) because the DBR cannot be negative! 
	 * Any number this big would be wrong anyways. We don't want to assert because there is
	 * calling logic that spits out some useful information upon error.
	 */
	if ((int32_t)length <= 0) {
		dprintf(DEBUG_CRITICAL, "kCDMAErrorNegativeLength: %d", (int32_t)length);
		return kCDMAErrorNegativeLength;
	}

	cp = CDMA_SOFTC(dma_channel);
	
	enter_critical_section();

	if (cp->c_status != CDMA_STATUS_IDLE && cp->c_status != CDMA_STATUS_UNINIT) {
		exit_critical_section();
		dprintf(DEBUG_CRITICAL, "kCDMAErrorNotIdle");
		return kCDMAErrorNotIdle;
	}
	
	/* mark channel as busy */
	cp->c_status = (cp->c_status == CDMA_STATUS_UNINIT) ? 
					CDMA_STATUS_BUSY_UNINIT : CDMA_STATUS_BUSY;
	
	exit_critical_section();
	
	/* do channel state init */
	dma_init_channel(dma_channel, true);
	
	/* turn on channel clock */
	cdma_clock_enable(dma_channel, true);

#if CDMA_VERSION > 1
	/*
	 * Check the channel request ID cache.
	 *
	 * For CDMA_VERSION == 1, these bits are always zero.
	 */
	if (unlikely((u_int32_t)fifo != cp->c_fifo_address)) {
		u_int32_t i;
		for (i = 0; i < CDMA_CHANNEL_CONFIG_COUNT; i++) {
			if (cdma_channel_configs[i].fifo_address == (u_int32_t)fifo) {

				/* sanity */
				ASSERT((unsigned)dma_channel >= cdma_channel_configs[i].channel_low);
				ASSERT((unsigned)dma_channel <= cdma_channel_configs[i].channel_high);
				if (NULL != cp->c_aes) {
					ASSERT(true == cdma_channel_configs[i].aes_ok);
				}

				/* cache the request ID */
				cp->c_fifo_address = cdma_channel_configs[i].fifo_address;
				cp->c_request_id_bits = CDMA_DCR_REQ(cdma_channel_configs[i].request_id);

				/* and done */
				break;
			}
		}
		/* sanity */
		ASSERT((u_int32_t)fifo == cp->c_fifo_address);
		if (unlikely((u_int32_t)fifo != cp->c_fifo_address))
			return kCDMAErrorBadFifoAddress;
	}	
#endif
	
	/* initialise the channel sgl state */
	cp->c_index = 0;
	cp->c_sgl = sgl;
	cp->c_resid = cp->c_length = length;
	cp->c_segment_offset = 0;

	/* Validate segments length match out length */
	cur_sgl = sgl;
	while(1) {
 		/* Check that the sgl->length is less than 2GB (neg). 
		 * Also verify that a segment is no larger than 16mb. 
		 * It is highly unlikely that this will ever occur.
		 */
 		if ((int32_t)cur_sgl->length < 0 ) {
			dprintf(DEBUG_CRITICAL, "kCDMAErrorNegativeSegment: 0x%08x", cur_sgl->length);
			return kCDMAErrorNegativeSegment;
		}
		if (cur_sgl->length > (16 * 1024 * 1024)) {
			dprintf(DEBUG_CRITICAL, "kCDMAErrorLargeSegment: 0x%08x", cur_sgl->length);
			return kCDMAErrorLargeSegment;
		}
		
 		seg_length += cur_sgl->length;

 		/* 
 		 * Check that we haven't gone over the length.
 		 * Check that we dont have a zero length segment before we get to the end.
 		 */		
 		if (seg_length > length) return kCDMAErrorSegmentsTooLong;
 		if (seg_length < length && cur_sgl->length == 0) return kCDMAErrorEarlyZeroSegment;
		
		/* Finished the check */
		if (seg_length == length) break;

		cur_sgl++;
 	}

	/* save the completion */
	cp->c_callback = completion;
	cp->c_callback_arg = completion_arg;

	/* determine the command direction */
	cp->c_is_read = (cmd == DMA_CMD_DIR_TX);

	/* configure the channel */
	switch (word_size) {
	case 1	: dw_fld = CDMA_DCR_DW_1;	break;
	case 2	: dw_fld = CDMA_DCR_DW_2;	break;
	case 4	: dw_fld = CDMA_DCR_DW_4;	break;
	default :
		return kCDMAErrorInvalidWordSize;
	}

	switch (burst_size) {
	case 1  : dbs_fld = CDMA_DCR_DBS_1;	break;
	case 2  : dbs_fld = CDMA_DCR_DBS_2;	break;
	case 4  : dbs_fld = CDMA_DCR_DBS_4;	break;
	case 8  : dbs_fld = CDMA_DCR_DBS_8;	break;
	case 16 : dbs_fld = CDMA_DCR_DBS_16;	break;
	case 32 : dbs_fld = CDMA_DCR_DBS_32;	break;
	default :
		return kCDMAErrorInvalidBurstSize;
	}

	/* 
	 * Must always abort the channel before configuring it in order to reset the state machine.
	 * Note that moving from IDLE (which looks like HALTED) to HALTED, or even from HALTED to
	 * HALTED can generate a spurious HALT interrupt, so mask everything.
	 */
	rCDMA_CHANNEL_CSR(dma_channel) = CDMA_CSR_ABT;

	/* configure channel config */
	rCDMA_CHANNEL_DCR(dma_channel) =
	    (cp->c_is_read ? CDMA_DCR_DTD_TX : CDMA_DCR_DTD_RX) |
	    dw_fld |
	    dbs_fld |
	    cp->c_request_id_bits;

#if CDMA_CHANNEL_ARASAN_QUIRKS
	if ((1 << dma_channel) & CDMA_CHANNEL_ARASAN_QUIRKS) {
		/* for Arasan, TX direction must be streaming mode */
		if (cp->c_is_read)
			rCDMA_CHANNEL_DCR(dma_channel) |= CDMA_DCR_ST;
	}
#endif

	rCDMA_CHANNEL_DAR(dma_channel) = (u_int32_t)fifo;
	rCDMA_CHANNEL_DBR(dma_channel) = length;

	/* init for AES if required */
	if (cp->c_aes) {
		ASSERT(0 == (length % cp->c_aes->chunk_size));
		cp->c_chunk = 0;
	}
	
	/* configure for initial data and start the channel */
	dma_continue_async(dma_channel);
	
	return kCDMASuccess;
}

void
dma_use_int(int dma_channel, bool use)
{
	u_int32_t vector = INT_CDMA_DMAC1 + dma_channel - 1;

	if (use)
	{
		unmask_int(vector);
	}
	else
	{
		mask_int(vector);
	}
}

bool
dma_poll(int dma_channel)
{
	bool ret = false;
	struct cdma_channel *cp;
	u_int32_t csr;

	cp = CDMA_SOFTC(dma_channel);

	if (CDMA_STATUS_IDLE == cp->c_status)
	{
		ret = true;
		goto exit;
	}

	enter_critical_section();

	csr = rCDMA_CHANNEL_CSR(dma_channel);

	if (((0 != (CDMA_CSR_CIE & csr)) &&
	     (0 != (CDMA_CSR_CIR & csr))) ||
	    ((0 != (CDMA_CSR_HIE & csr)) &&
	     (0 != (CDMA_CSR_HIR & csr))))
	{
		dma_int_handler((void *)dma_channel);
	}

	exit_critical_section();

	if (CDMA_STATUS_IDLE == cp->c_status)
	{
		ret = true;
		goto exit;
	}

exit:

	return ret;
}

static void
dma_continue_async(u_int32_t dma_channel)
{
	struct cdma_channel	*cp;
	u_int32_t		count;

	cp = CDMA_SOFTC(dma_channel);

	RELEASE_ASSERT(0 < cp->c_resid);

	/* save state for restart */
	cp->c_previous_index = cp->c_index;
	cp->c_previous_resid = cp->c_resid;
	cp->c_previous_offset = cp->c_segment_offset;

	if (NULL == cp->c_aes) {
		count = dma_generate_segments(dma_channel);
	} else {
		count = dma_generate_aes_segments(dma_channel);
	}
	
	/* we're done with messing with the command chain, make sure it's in memory */
	ASSERT(count > 0);
	platform_cache_operation(CACHE_CLEAN, cp->c_chain, sizeof(*cp->c_chain) * count);
	
	/* clear and enable interrupts and (re)start the channel */
	rCDMA_CHANNEL_CAR(dma_channel) = mem_static_map_physical((uintptr_t)cp->c_chain);
	rCDMA_CHANNEL_CSR(dma_channel) =
	    CDMA_CSR_GO |
	    CDMA_CSR_HIE |
	    CDMA_CSR_ERR |
	    CDMA_CSR_HIR |
	    CDMA_CSR_CIR |
	    (cp->c_aes ? CDMA_CSR_FC(cp->c_filter) : 0) |
	    CDMA_CSR_CCACHE(cp->c_ccache_c);
}

static u_int32_t
dma_generate_segments(int dma_channel)
{
	struct cdma_channel	*cp;
	struct cdma_command	*cc;
	struct dma_segment	*sge;
	int			i;
	u_int32_t		segment_length;

	cp = CDMA_SOFTC(dma_channel);

	/* generate segment commands */
	for (i = 0; i < (CDMA_CHANNEL_CMDS - 1); i++) {

		/* get the next command */
		cc = cp->c_chain + i;

		/* get the next segment */
		sge = cp->c_sgl + cp->c_index++;
		
		/*
		 * Make a memory command for this segment.
		 *
		 * The segment offset may be nonzero for the first
		 * segment only, and is used to compensate for operations
		 * that are terminated presumptively by the peripheral in
		 * the middle of a segment.
		 */
		segment_length = sge->length - cp->c_segment_offset;
		
		cc->ctrl = CDMA_CTRL_CMD_MEM | CDMA_CTRL_CACHE(cp->c_is_read ? cp->c_ccache_r : cp->c_ccache_w);
		cc->addr = sge->paddr + cp->c_segment_offset;
		cc->length = segment_length;
		if ( 0==cc->length )
		{
			panic("Caught trying to generate zero-length cdma segment on channel %d, irqState: %d",dma_channel,(int)cp->irqState);
		}

		cp->c_segment_offset = 0;
		
		if (segment_length < cp->c_resid) {
			
			/* account for this segment */
			cp->c_resid -= segment_length;
		} else {
			ASSERT(segment_length == cp->c_resid);
			cp->c_resid = 0;
			break;
		}
	}

	/* barrier the last MEM command to ensure that data drains before we reach the HLT and interrupt */
	cc->ctrl |= CDMA_CTRL_BAR;

	/* always make the next command a HALT */
	(cc + 1)->ctrl = CDMA_CTRL_CMD_HLT;

	return(i + 2);
}

static u_int32_t
dma_generate_aes_segments(int dma_channel)
{
	struct cdma_channel	*cp;
	struct cdma_command	*cc;
	struct dma_segment	*sge;
	int			i;
	u_int32_t		segment_length;
	u_int32_t		chunk_done;

	cp = CDMA_SOFTC(dma_channel);

	i = 0;
	cc = cp->c_chain;
	sge = cp->c_sgl + cp->c_index;

	/* loop emitting pairs of LDIV and chunk-sized transfers */
	for (; cp->c_resid > 0; cp->c_resid -= cp->c_aes->chunk_size) {
		/*
		 * Assuming a virtually contiguous input buffer, in the worst case
		 * we will emit three commands here, make sure there is room for them
		 * plus the terminating HLT.
		 */
		if ((i + 4) >= CDMA_CHANNEL_CMDS)
			break;
		
		/*
		 * We will always come here at the beginning of a chunk, so 
		 * emit an LDIV and get the supplier to fill in the IV.
		 */
		cc->ctrl = CDMA_CTRL_CMD_LDIV;
		cp->c_aes->iv_func(cp->c_aes->iv_func_arg, cp->c_chunk, &cc->iv0);
		cc++;
		i++;
		cp->c_chunk++;
		chunk_done = 0;

		/*
		 * Emit part or all of a chunk.  We may run out of bytes in this
		 * segment, which is OK (we'll make another one).
		 */
		while (chunk_done < cp->c_aes->chunk_size) {
			ASSERT(i < (CDMA_CHANNEL_CMDS - 1));
			
			/* find the space left in this sge */
			segment_length = sge->length - cp->c_segment_offset;

			/* limit the segment to completing one chunk */
			if ((chunk_done + segment_length) > cp->c_aes->chunk_size)
				segment_length = (cp->c_aes->chunk_size - chunk_done);

			/* make the command */
			cc->ctrl = CDMA_CTRL_CMD_MEM | CDMA_CTRL_CACHE(cp->c_is_read ? cp->c_ccache_r : cp->c_ccache_w) | CDMA_CTRL_FE | ((chunk_done == 0) ? CDMA_CTRL_FR : 0);
			cc->addr = sge->paddr + cp->c_segment_offset;
			cc->length = segment_length;
			if ( 0==cc->length )
			{
				panic("Caught trying to generate zero-length cdma segment on channel %d, irqState: %d",dma_channel,(int)cp->irqState);
			}

			/* update running state */
			chunk_done += segment_length;
			cp->c_segment_offset += segment_length;
			cc++;
			i++;

			/* did we just finish that sge? */
			if (cp->c_segment_offset >= sge->length) {
				sge++;
				cp->c_index++;
				cp->c_segment_offset = 0;
			}
		}
	}

	if (0 == cp->c_resid) {
		/* we must have generated at least two segments... */
		ASSERT(cc != cp->c_chain);
	}

	/* barrier the last MEM command to ensure that data drains before we reach the HLT and interrupt */
	(cc - 1)->ctrl |= CDMA_CTRL_BAR;
	
	/* make the next command a HALT */
	cc->ctrl = CDMA_CTRL_CMD_HLT;

	return(i + 2);
}

static void
dma_int_handler(void *arg)
{
	u_int32_t		dma_channel = (u_int32_t)arg;
	struct cdma_channel	*cp;
	int			i;
	u_int32_t		scan_resid;
	u_int32_t		csr;
	struct dma_segment	*sge;

	cp = CDMA_SOFTC(dma_channel);
	csr = rCDMA_CHANNEL_CSR(dma_channel);

	ASSERT(CDMA_CSR_RUN(csr) != CDMA_CSR_RUN_RUNNING);

	if (unlikely(csr & CDMA_CSR_ERR)) {
#if CDMA_VERSION > 1
		panic("CDMA: channel %d error interrupt, error status 0x%0x", dma_channel, 
			rCDMA_CHANNEL_ERR(dma_channel) & rCDMA_CHANNEL_ERR_MASK);
#else
		panic("CDMA: channel %d error interrupt", dma_channel);
#endif
	}
	if (unlikely(csr & CDMA_CSR_CIR))
		panic("CDMA: channel %d spurious CIR, status 0x%0x", dma_channel, csr);

	if (unlikely(!(csr & CDMA_CSR_HIR)))
		panic("CDMA: channel %d spurious interrupt, status 0x%0x", dma_channel, csr);
	
	/* clear interrupt status, mask further interrupts */
	rCDMA_CHANNEL_CSR(dma_channel) = CDMA_CSR_HIR;

	/*
	 * Is the channel done?
	 * Note that we maintain the DBR even if the channel is quirked for
	 * streaming mode so that this test is valid in either case.
	 */
	if (cp->c_resid == 0 && rCDMA_CHANNEL_DBR(dma_channel) <= 0) {

		/* clear AES config */
		dma_set_aes(dma_channel, NULL);

		/* turn off the channel clock */
		cdma_clock_enable(dma_channel, false);
		
		/* mark as idle so that the channel can be re-used in the callback */
		cp->c_status = CDMA_STATUS_IDLE;
		
		/* call the user-supplied callback */
		if (cp->c_callback != NULL)
			cp->c_callback(cp->c_callback_arg);
		
	} else if (CDMA_CSR_RUN_HALTED == CDMA_CSR_RUN(rCDMA_CHANNEL_CSR(dma_channel))) {
		/* Check that hardware and software are in agreement about how much data is left to
		 * transfer.  We never expect to encounter an interrupt where the hardware hasn't
		 * consumed the entire segment list we last provided.  This check relies on a BAR to
		 * have drained the FIFO towards memory (but the DBR can get ahead on an RX by up
		 * to one FIFO length - hence the inequality). */
		int32_t dbr = rCDMA_CHANNEL_DBR(dma_channel);
		if (dbr > (int32_t)cp->c_resid) {
			panic("Failed DBR/resid check: ch%d, c_resid=0x%08x DBR=0x%08x\n", dma_channel, cp->c_resid, dbr);
		}

		/* the channel has hit the halt terminating the current segment set */
		cp->irqState = st_CONT;
		/* re-configure & restart the channel */
		dma_continue_async(dma_channel);
	} else {
		/* the channel has been stopped by the peripheral */
		ASSERT(CDMA_CSR_RUN_STOPPED == CDMA_CSR_RUN(rCDMA_CHANNEL_CSR(dma_channel)));

		/* fix up residual count and work out where we ended up in the current segment set */
		/* XXX this should probably just be   cp->c_resid = rCDMA_CHANNEL_DBR(dma_channel) */
		cp->c_resid += rCDMA_CHANNEL_DBR(dma_channel);
		scan_resid = cp->c_previous_resid - cp->c_resid;

		/* walk up the segment list trying to work out where the peripheral stopped */
		for (i = 0; i < CDMA_CHANNEL_CMDS; i++) {
			/* get the next segment */
			sge = cp->c_sgl + cp->c_previous_index + i;

			/* did it stop in this segment? */
			if ((sge->length - cp->c_previous_offset) >= scan_resid)
				break;

			/* account for the segment */
			scan_resid -= (sge->length - cp->c_previous_offset);
			cp->c_previous_offset = 0;
		}

		/* it should be impossible to not find the terminating case in the segments we loaded */
		ASSERT(i < CDMA_CHANNEL_CMDS);
		
		/* rewind the DMA state back to the segment in which it stopped */
		if (((sge->length - cp->c_previous_offset) == scan_resid)) {
		    /* current segment is finished, so skip it b4 reconfigure the DMA */
			cp->c_index = cp->c_previous_index + i + 1;
			cp->c_segment_offset = 0;
		} else {
		    /* re-configure the CDMA */
			cp->c_index = cp->c_previous_index + i;
			cp->c_segment_offset = cp->c_previous_offset + scan_resid;
		}

		cp->irqState = st_REWIND;

		/* now we can re-configure the CDMA */
		dma_continue_async(dma_channel);
	}
}

void
dma_cancel(int dma_channel)
{
	struct cdma_channel	*cp;
	utime_t		start_time;
	
	cp = CDMA_SOFTC(dma_channel);
	ASSERT((dma_channel > 0) && (dma_channel <= DMA_CHANNEL_COUNT));

	/* it's legal but not interesting to cancel a channel that hasn't been initialised */
	if (CDMA_STATUS_UNINIT == cp->c_status)
		return;

	dprintf(DEBUG_INFO, "CDMA: cancelling channel\n");
	
	/*
	 * Abort the channel.
	 *
	 * In the general case, we would also have to wait for the CDMA to either be
	 * processing a MEM command (in which case ABORT is effective immediately) or
	 * stopped for some other reason.  Since we know that our command lists are always
	 * MEM or HLT, we can be sure this will be immediately effective.
	 */
	start_time = system_time();

	cdma_clock_enable(dma_channel, true);

	if (CDMA_CSR_RUN_RUNNING == CDMA_CSR_RUN(rCDMA_CHANNEL_CSR(dma_channel))) {

#if CDMA_VERSION < 2
		/* version 1 CDMA requires pause before abort */
		rCDMA_CHANNEL_CSR(dma_channel) = CDMA_CSR_PS;
		while (!(rCDMA_CHANNEL_CSR(dma_channel) & CDMA_CSR_PSD)) {
			if (time_has_elapsed(start_time, 10))
				panic("CDMA: channel %d pause timeout during abort", dma_channel);
		}
#endif

		rCDMA_CHANNEL_CSR(dma_channel) = CDMA_CSR_ABT;
		while (CDMA_CSR_RUN_RUNNING == CDMA_CSR_RUN(rCDMA_CHANNEL_CSR(dma_channel))) {
			if (time_has_elapsed(start_time, 10000))
				panic("CDMA: channel %d timeout during abort", dma_channel);
		}
	}

	/* clear the crypto config */
	dma_set_aes(dma_channel, NULL);

	/* turn off the channel clock */
	cdma_clock_enable(dma_channel, false);
	
	/* mark the channel idle */
	cp->c_status = CDMA_STATUS_IDLE;
}

/*
 * Set the AES configuration for (dma_channel).
 *
 * If (config) is NULL, AES is disabled for the channel.
 */
int
dma_set_aes(int dma_channel, struct dma_aes_config *config)
{
	struct cdma_channel	*cp;
	int			i;
	u_int32_t		fcsr;
	bool			clock_state;
		
	
	ASSERT((dma_channel > 0) && (dma_channel <= DMA_CHANNEL_COUNT));
	cp = CDMA_SOFTC(dma_channel);

	cp->c_aes = config;

	if (NULL != config) {

		/* 
		 * For v2+ we must turn on at least one AES-using channel before
		 * the AES block will be clocked.  Since we can't be sure whether
		 * one is on or not, turn this one on and remember whether it was
		 * already on so that we don't turn it off inadvertently.
		 */
		clock_state = cdma_clock_enable(dma_channel, true);

		/* assign an AES filter context */
		if (0 == cp->c_filter) {
			enter_critical_section();
			for (i = 2; i < CDMA_FILTER_CONTEXTS; i++) {
				if (!(cdma_fc_inuse & (1 << i))) {
					cp->c_filter = i;
					cdma_fc_inuse |= (1 << i);
					break;
				}
			}
			exit_critical_section();
		}
		if (0 == cp->c_filter)
			panic("CDMA: no AES filter contexts: 0x%08x", cdma_fc_inuse);

		/* invariants */
		fcsr = CDMA_FCSR_CHANNEL(dma_channel) | CDMA_FCSR_CBC;

		/* direction */
		if ((cp->c_aes->command & AES_CMD_DIR_MASK) == AES_CMD_ENC)
			fcsr |= CDMA_FCSR_ENC;

		/* key size */
		switch (cp->c_aes->keying & AES_KEY_SIZE_MASK) {
		case AES_KEY_SIZE_128:
			fcsr |= CDMA_FCSR_KL_128;
			break;
		case AES_KEY_SIZE_192:
			fcsr |= CDMA_FCSR_KL_192;
			break;
		case AES_KEY_SIZE_256:
			fcsr |= CDMA_FCSR_KL_256;
			break;
		default:
			ASSERT(false);
			return(-1);
		}

		/* key selection/load */
		switch (cp->c_aes->keying & AES_KEY_TYPE_MASK) {
		case AES_KEY_TYPE_USER:
			fcsr |= CDMA_FCSR_KS_VARIABLE;
			ASSERT(NULL != cp->c_aes->key);
			switch (cp->c_aes->keying & AES_KEY_SIZE_MASK) {
			case AES_KEY_SIZE_256:
				rCDMA_FILTER_KBR7(cp->c_filter) = ((u_int32_t *)cp->c_aes->key)[7];
				rCDMA_FILTER_KBR6(cp->c_filter) = ((u_int32_t *)cp->c_aes->key)[6];
			case AES_KEY_SIZE_192:
				rCDMA_FILTER_KBR5(cp->c_filter) = ((u_int32_t *)cp->c_aes->key)[5];
				rCDMA_FILTER_KBR4(cp->c_filter) = ((u_int32_t *)cp->c_aes->key)[4];
			case AES_KEY_SIZE_128:
				rCDMA_FILTER_KBR3(cp->c_filter) = ((u_int32_t *)cp->c_aes->key)[3];
				rCDMA_FILTER_KBR2(cp->c_filter) = ((u_int32_t *)cp->c_aes->key)[2];
				rCDMA_FILTER_KBR1(cp->c_filter) = ((u_int32_t *)cp->c_aes->key)[1];
				rCDMA_FILTER_KBR0(cp->c_filter) = ((u_int32_t *)cp->c_aes->key)[0];
				break;
			default:
				ASSERT(false);
				return(-1);
			}
			break;
		case AES_KEY_TYPE_UID0:
			fcsr |= CDMA_FCSR_KEY(0);
			break;
		case AES_KEY_TYPE_GID0:
			fcsr |= CDMA_FCSR_KEY(1);
			break;
		case AES_KEY_TYPE_GID1:
			fcsr |= CDMA_FCSR_KEY(2);
			break;
		default:
			ASSERT(false);
			return(-1);
		}

		/* and configure */
		rCDMA_FILTER_CSR(cp->c_filter) = fcsr;

		/*
		 * It's safe to turn the channel clock off now if it was off when we started.
		 */
		cdma_clock_enable(dma_channel, clock_state);
	}
	
	return(0);
}

bool
cdma_clock_enable(int channel, bool state)
{
	bool			previous = true;
#if CDMA_VERSION > 1
	int			bank;
	int			bit;

	bank = channel / 32;
	bit = channel % 32;

	/* save the previous state for our caller */
	previous = rCDMA_CLOCK_STATUS(bank) & (1 << bit);

	/* frob clock as requested */
	if (state != previous) {
		if (state) {
			rCDMA_CLOCK_ON(bank) = 1 << bit;
		} else {
			rCDMA_CLOCK_OFF(bank) = 1 << bit;
		}
	}
#endif
	return(previous);
}

static void
cdma_dump_channel(int channel)
{
	struct cdma_channel	*cp;
	struct cdma_command	*cmd;
	struct dma_segment	*seg;
	int			i;
	u_int32_t		resid;
	int			filter;
	bool			clock_state;

	cp = CDMA_SOFTC(channel);
	if (CDMA_STATUS_UNINIT == cp->c_status) {
		dprintf(DEBUG_CRITICAL, "CDMA: channel %d not set up\n", channel);
		return;
	}

	clock_state = cdma_clock_enable(channel, true);
	dprintf(DEBUG_CRITICAL, "CDMA: channel %d %s(%d), index %d resid 0x%x\n", 
	       channel,
	       ((cp->c_status == CDMA_STATUS_IDLE) ? "idle" : ((cp->c_status == CDMA_STATUS_BUSY) ? "busy" : "unknown")),
	       cp->c_status, cp->c_index, cp->c_resid);
	if (cp->c_status != CDMA_STATUS_IDLE) {
		dprintf(DEBUG_CRITICAL, "CDMA: channel %d length 0x%x\n", channel, cp->c_length);
		resid = cp->c_length;
		seg = cp->c_sgl;
		dprintf(DEBUG_CRITICAL, "CDMA:   seg addr/seg length\n");
		for (i = 0; ; i++) {
			dprintf(DEBUG_CRITICAL, "CDMA:   0x%08x/0x%x\n", seg->paddr, seg->length);
			if (seg->length >= resid)
				break;
			resid -= seg->length;
			seg++;
		}
		cmd = cp->c_chain;
		resid = cp->c_length;
		for (i = 0; i < CDMA_CHANNEL_CMDS; i++) {
			dprintf(DEBUG_CRITICAL, "CDMA: NXT 0x%08x  CMD 0x%08x  ADDR 0x%08x  LEN 0x%08x\n",
			       cmd->nxt, cmd->ctrl, cmd->addr, cmd->length);
			if ((cmd->ctrl & CDMA_CTRL_CMD_MASK) == CDMA_CTRL_CMD_MEM) {
				if (cmd->length >= resid)
					break;
				resid -= cmd->length;
			}
			cmd++;
		}
	}
#if CDMA_VERSION > 1
	dprintf(DEBUG_CRITICAL, "CDMA: CSR 0x%08x  DCR 0x%08x  DAR 0x%08x  DBR 0x%08x  MAR 0x%08x  CAR 0x%08x  ERR 0x%08x\n",
	    rCDMA_CHANNEL_CSR(channel), rCDMA_CHANNEL_DCR(channel), rCDMA_CHANNEL_DAR(channel),
	    rCDMA_CHANNEL_DBR(channel), rCDMA_CHANNEL_MAR(channel), rCDMA_CHANNEL_CAR(channel), 
	    rCDMA_CHANNEL_ERR(channel) & rCDMA_CHANNEL_ERR_MASK);
#else
	dprintf(DEBUG_CRITICAL, "CDMA: CSR 0x%08x  DCR 0x%08x  DAR 0x%08x  DBR 0x%08x  MAR 0x%08x  CAR 0x%08x\n",
	    rCDMA_CHANNEL_CSR(channel), rCDMA_CHANNEL_DCR(channel), rCDMA_CHANNEL_DAR(channel),
	    rCDMA_CHANNEL_DBR(channel), rCDMA_CHANNEL_MAR(channel), rCDMA_CHANNEL_CAR(channel));
#endif
	dprintf(DEBUG_CRITICAL, "CDMA: CMD_NXT 0x%08x  CMD_CMD 0x%08x  CMD_ADDR 0x%08x  CMD_LEN 0x%08x\n",
	    rCDMA_CHANNEL_CMND0(channel), rCDMA_CHANNEL_CMND1(channel), rCDMA_CHANNEL_CMND2(channel),
	    rCDMA_CHANNEL_CMND3(channel));

	filter = (rCDMA_CHANNEL_CSR(channel) >> 8) & 0xff;
	if (filter > 0) {
		dprintf(DEBUG_CRITICAL, "FILT: CSR 0x%08x\n", rCDMA_FILTER_CSR(filter));
		dprintf(DEBUG_CRITICAL, "FILT: IV 0x%08x 0x%08x 0x%08x 0x%08x\n",
		    rCDMA_FILTER_IVR0(filter),
		    rCDMA_FILTER_IVR1(filter),
		    rCDMA_FILTER_IVR2(filter),
		    rCDMA_FILTER_IVR3(filter));
		dprintf(DEBUG_CRITICAL, "FILT: KEY 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		    rCDMA_FILTER_KBR0(filter),
		    rCDMA_FILTER_KBR1(filter),
		    rCDMA_FILTER_KBR2(filter),
		    rCDMA_FILTER_KBR3(filter),
		    rCDMA_FILTER_KBR4(filter),
		    rCDMA_FILTER_KBR5(filter),
		    rCDMA_FILTER_KBR6(filter),
		    rCDMA_FILTER_KBR7(filter));
	}
	cdma_clock_enable(channel, clock_state);
}

static int
do_cdma(int argc, struct cmd_arg *args)
{
	int	i;

	if (argc != 2) {
		dprintf(DEBUG_CRITICAL, "not enough arguments\n");
usage:
		dprintf(DEBUG_CRITICAL, "usage:\n");
		dprintf(DEBUG_CRITICAL, "%s list       - list CDMA channels in use\n", args[0].str);
		dprintf(DEBUG_CRITICAL, "%s <channel>  - print status for <channel>\n", args[0].str);
		return(-1);
	}

	if (!strcmp(args[1].str, "list")) {
		dprintf(DEBUG_CRITICAL, "CDMA channels in use:");
		for (i = 1; i <= DMA_CHANNEL_COUNT; i++)
			if (CDMA_STATUS_UNINIT != CDMA_SOFTC(i)->c_status)
				dprintf(DEBUG_CRITICAL, " %d", i);
		dprintf(DEBUG_CRITICAL, "\n");
	} else if ((args[1].u > 0) && (args[1].u <= DMA_CHANNEL_COUNT)) {
			cdma_dump_channel(args[1].u);
	} else {
		goto usage;
	}
	return(0);
}

MENU_COMMAND_DEBUG(cdma, do_cdma, "CDMA status", NULL);

static void
do_cdma_panic(void *arg __unused)
{
	int	i;

	for (i = 1; i <= DMA_CHANNEL_COUNT; i++)
		if (CDMA_STATUS_UNINIT != CDMA_SOFTC(i)->c_status)
			cdma_dump_channel(i);
}

PANIC_HOOK(cdma, do_cdma_panic, NULL);
