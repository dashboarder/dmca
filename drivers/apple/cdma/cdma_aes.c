/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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

#include "cdma.h"

#define CDMA_AES_UID_MASK	(1 << 0)
#define CDMA_AES_UID_PLUS_MASK	(1 << 1)	// v4+

static int dma_setup_channel(u_int32_t cmd_dir, int dma_channel,
    u_int32_t mem_addr, u_int32_t dev_addr, u_int32_t length,
    u_int32_t word_size, u_int32_t burst_size, bool filter);

static void dma_wait_channel_halted(int dma_channel);
static void cdma_dump_aes_channel(int channel);

static struct cdma_command cdma_aes_command[4] __attribute__((aligned(32)));

void
platform_disable_keys(u_int32_t gid, u_int32_t uid)
{
	u_int32_t keydis = ((gid << 1) | (uid & CDMA_AES_UID_MASK)) & 0x0000FFFF;

	cdma_clock_enable(DMA_MEMORY_TX, true);
	rCDMA_FILTER_KEYDIS = keydis;
	cdma_clock_enable(DMA_MEMORY_TX, false);

	// SKG disable feature is broken.
	// To workaroud, UID+ will tag along UID. If UID is disabled, we assume UID+ is also disabled
#if CDMA_VERSION >= 4
	if (uid & CDMA_AES_UID_MASK) {
		rCDMA_FILTER_SKGCNTL = CDMA_FSKGCNTL_SKGDIS;
	}
#endif /* CDMA_VERSION >= 4 */	
}

int
aes_hw_crypto_cmd(u_int32_t cmd, void *src, void *dst, size_t len, u_int32_t opts, const void *key, void *iv)
{
	u_int32_t cbc_val, dec_val, kl_val, ks_val, kx_val;

	clock_gate(CLK_CDMA, true);
	cdma_clock_enable(DMA_MEMORY_TX, true);
	cdma_clock_enable(DMA_MEMORY_RX, true);
	
	/* clip length to multiple of 16 bytes */
	if (len & 0xf)
		goto fail;

	switch (cmd & AES_CMD_MODE_MASK) {
		case AES_CMD_ECB : cbc_val = 0; break;
		case AES_CMD_CBC : cbc_val = 1; break;
		default : goto fail;
	}

	switch (cmd & AES_CMD_DIR_MASK) {
		case AES_CMD_ENC : dec_val = 1; break;
		case AES_CMD_DEC : dec_val = 0; break;
		default : goto fail;
	}

	/* Set the IV */
	if (iv != 0) {
		rCDMA_FILTER_IVR0(CDMA_FILTER_CONTEXT_NORMAL_M2M) = ((u_int32_t *)iv)[0];
		rCDMA_FILTER_IVR1(CDMA_FILTER_CONTEXT_NORMAL_M2M) = ((u_int32_t *)iv)[1];
		rCDMA_FILTER_IVR2(CDMA_FILTER_CONTEXT_NORMAL_M2M) = ((u_int32_t *)iv)[2];
		rCDMA_FILTER_IVR3(CDMA_FILTER_CONTEXT_NORMAL_M2M) = ((u_int32_t *)iv)[3];
	} else {
		rCDMA_FILTER_IVR0(CDMA_FILTER_CONTEXT_NORMAL_M2M) = 0;
		rCDMA_FILTER_IVR1(CDMA_FILTER_CONTEXT_NORMAL_M2M) = 0;
		rCDMA_FILTER_IVR2(CDMA_FILTER_CONTEXT_NORMAL_M2M) = 0;
		rCDMA_FILTER_IVR3(CDMA_FILTER_CONTEXT_NORMAL_M2M) = 0;
	}

	/* Get the key length */
	kl_val = 0;
	switch (opts & AES_KEY_SIZE_MASK) {
		case AES_KEY_SIZE_128 : kl_val = 0; break;
		case AES_KEY_SIZE_192 : kl_val = 1; break;
		case AES_KEY_SIZE_256 : kl_val = 2; break;
		default : goto fail;
	}

	/* Set the key */
	ks_val = 0;
	kx_val = 0;
	switch (opts & AES_KEY_TYPE_MASK) {
		case AES_KEY_TYPE_USER :

			ks_val = 1;

			switch (kl_val) {
				default:
					rCDMA_FILTER_KBR7(CDMA_FILTER_CONTEXT_NORMAL_M2M) = ((u_int32_t *)key)[7];
					rCDMA_FILTER_KBR6(CDMA_FILTER_CONTEXT_NORMAL_M2M) = ((u_int32_t *)key)[6];
				case 1:
					rCDMA_FILTER_KBR5(CDMA_FILTER_CONTEXT_NORMAL_M2M) = ((u_int32_t *)key)[5];
					rCDMA_FILTER_KBR4(CDMA_FILTER_CONTEXT_NORMAL_M2M) = ((u_int32_t *)key)[4];
				case 0:
					rCDMA_FILTER_KBR3(CDMA_FILTER_CONTEXT_NORMAL_M2M) = ((u_int32_t *)key)[3];
					rCDMA_FILTER_KBR2(CDMA_FILTER_CONTEXT_NORMAL_M2M) = ((u_int32_t *)key)[2];
					rCDMA_FILTER_KBR1(CDMA_FILTER_CONTEXT_NORMAL_M2M) = ((u_int32_t *)key)[1];
					rCDMA_FILTER_KBR0(CDMA_FILTER_CONTEXT_NORMAL_M2M) = ((u_int32_t *)key)[0];
			}

		case AES_KEY_TYPE_UID0 : kx_val = 0; break;
		case AES_KEY_TYPE_GID0 : kx_val = 1; break;
		case AES_KEY_TYPE_GID1 : kx_val = 2; break;
	}

	/* make sure the requested key is available */
	if ((0 == ks_val) && (rCDMA_FILTER_KEYDIS & (1 << kx_val))) {
		dprintf(DEBUG_INFO, "CDMA: requested AES key has been disabled\n");
		return -1;
	}

	rCDMA_FILTER_CSR(CDMA_FILTER_CONTEXT_NORMAL_M2M) = 
							((DMA_MEMORY_TX << 8) |		// DMA Channel
							(dec_val << 16) |  		// decode / encode
							(cbc_val << 17) |		// cbc / ecb
							(kl_val << 18) |		// key length
							(ks_val << 20) |		// key select: fixed / variable
							(kx_val << 21));		// key index

	if (0 != dma_setup_channel(DMA_CMD_DIR_MEM, DMA_MEMORY_TX, (u_int32_t)src, 0, len, 0, 0, true))
		goto fail;

	if (0 != dma_setup_channel(DMA_CMD_DIR_MEM, DMA_MEMORY_RX, (u_int32_t)dst, 0, len, 0, 0, false))
		goto fail;

	platform_cache_operation(CACHE_CLEAN, src, len);
	platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, dst, len);
	
	/* Start the channel */
	rCDMA_CHANNEL_CSR(DMA_MEMORY_TX) |= (1 << 0);
	rCDMA_CHANNEL_CSR(DMA_MEMORY_RX) |= (1 << 0);

	/* Wait for both channels to halt (this ensures both are marked idle) */
	dma_wait_channel_halted(DMA_MEMORY_TX);
	dma_wait_channel_halted(DMA_MEMORY_RX);

#if CDMA_IS_INCOHERENT
	/* If core did any speculative prefetching during DMA, invalidate the cache lines again */
	platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, dst, len);
#endif

	/* Clear user key from the registers */
	if (ks_val != 0) {
		rCDMA_FILTER_KBR0(CDMA_FILTER_CONTEXT_NORMAL_M2M) = 0;
		rCDMA_FILTER_KBR1(CDMA_FILTER_CONTEXT_NORMAL_M2M) = 0;
		rCDMA_FILTER_KBR2(CDMA_FILTER_CONTEXT_NORMAL_M2M) = 0;
		rCDMA_FILTER_KBR3(CDMA_FILTER_CONTEXT_NORMAL_M2M) = 0;
		rCDMA_FILTER_KBR4(CDMA_FILTER_CONTEXT_NORMAL_M2M) = 0;
		rCDMA_FILTER_KBR5(CDMA_FILTER_CONTEXT_NORMAL_M2M) = 0;
		rCDMA_FILTER_KBR6(CDMA_FILTER_CONTEXT_NORMAL_M2M) = 0;
		rCDMA_FILTER_KBR7(CDMA_FILTER_CONTEXT_NORMAL_M2M) = 0;
	}

	cdma_clock_enable(DMA_MEMORY_TX, false);
	cdma_clock_enable(DMA_MEMORY_RX, false);
	
	return 0;
fail:
	panic("CDMA AES: bad arguments");
}

static int
dma_setup_channel(u_int32_t cmd_dir, int dma_channel,
    u_int32_t mem_addr, u_int32_t dev_addr, u_int32_t length,
    u_int32_t word_size, u_int32_t burst_size, bool filter)
{
	struct cdma_command *cdma_cmd;
	u_int32_t dw_val, dbs_val, dcr_val, ccache_val = 0;
	int result = -1;

	/* must always abort the channel before configuring it */
	rCDMA_CHANNEL_CSR(dma_channel) = CDMA_CSR_ABT;
	
	cdma_cmd = &cdma_aes_command[(dma_channel * 2) - 2];
	cdma_cmd->ctrl = (filter ? CDMA_CTRL_FE | CDMA_CTRL_FR : 0) | CDMA_CTRL_BAR | CDMA_CTRL_CMD_MEM;
#if (CDMA_VERSION == 3) || (CDMA_VERSION == 4)
	ccache_val = CDMA_CTRL_CACHE(CDMA_CACHE_CACHE);
	cdma_cmd->ctrl |= CDMA_CTRL_CACHE(CDMA_CACHE_CACHE);
#elif CDMA_VERSION >= 5
	ccache_val = CDMA_CTRL_CACHE(CDMA_CACHE_BUFFER | CDMA_CACHE_CACHE | CDMA_CACHE_WALLOC);
	cdma_cmd->ctrl |= CDMA_CTRL_CACHE(CDMA_CACHE_BUFFER | CDMA_CACHE_CACHE);
	if (dma_channel == DMA_MEMORY_TX)
		cdma_cmd->ctrl |= CDMA_CTRL_CACHE(CDMA_CACHE_WALLOC);
	else
		cdma_cmd->ctrl |= CDMA_CTRL_CACHE(CDMA_CACHE_RALLOC);
#endif
	cdma_cmd->addr = mem_addr;
	cdma_cmd->length = length;

	/* terminate the chain */
	cdma_cmd->nxt = (u_int32_t)(cdma_cmd + 1);
	(cdma_cmd + 1)->ctrl = CDMA_CTRL_CMD_HLT;
	
	if (cmd_dir == DMA_CMD_DIR_MEM) {
		dcr_val = CDMA_CSR_MTM;
	} else {
		dcr_val = 0;	/* ! CDMA_CSR_MTM */

		switch (word_size) {
			case 1 : dw_val = 0; break;
			case 2 : dw_val = 1; break;
			case 4 : dw_val = 2; break;
			default : goto exit; break;
		}

		switch (burst_size) {
			case 1  : dbs_val = 0; break;
			case 2  : dbs_val = 1; break;
			case 4  : dbs_val = 2; break;
			case 8  : dbs_val = 3; break;
			case 16 : dbs_val = 4; break;
			case 32 : dbs_val = 5; break;
			default : goto exit; break;
		}

		rCDMA_CHANNEL_DCR(dma_channel) =
				((cmd_dir == DMA_CMD_DIR_TX) ? CDMA_DCR_DTD_TX : CDMA_DCR_DTD_RX) |
				(dw_val << 2) |
				(dbs_val << 4);

		rCDMA_CHANNEL_DAR(dma_channel) = dev_addr;
	}

	platform_cache_operation(CACHE_CLEAN , cdma_cmd, sizeof(struct cdma_command));

	rCDMA_CHANNEL_DBR(dma_channel) = length;
	rCDMA_CHANNEL_CAR(dma_channel) = (u_int32_t)cdma_cmd;

	/* Clear channel status and configure */
	rCDMA_CHANNEL_CSR(dma_channel) = CDMA_CSR_CIR | CDMA_CSR_HIR | CDMA_CSR_ERR | (filter << 8) | dcr_val | ccache_val;

	result = 0;

exit:
	return result;
}

static void
dma_wait_channel_halted(int dma_channel)
{
	while (CDMA_CSR_RUN(rCDMA_CHANNEL_CSR(dma_channel)) == CDMA_CSR_RUN_RUNNING)
		task_yield();
}

static void
cdma_dump_aes_channel(int channel)
{
	int			filter;
	bool			clock_state;

	clock_state = cdma_clock_enable(channel, true);
	printf("CDMA: CSR 0x%08x  DCR 0x%08x  DAR 0x%08x  DBR 0x%08x  MAR 0x%08x  CAR 0x%08x\n",
	    rCDMA_CHANNEL_CSR(channel), rCDMA_CHANNEL_DCR(channel), rCDMA_CHANNEL_DAR(channel),
	    rCDMA_CHANNEL_DBR(channel), rCDMA_CHANNEL_MAR(channel), rCDMA_CHANNEL_CAR(channel));
	printf("CDMA: CMD_NXT 0x%08x  CMD_CMD 0x%08x  CMD_ADDR 0x%08x  CMD_LEN 0x%08x\n",
	    rCDMA_CHANNEL_CMND0(channel), rCDMA_CHANNEL_CMND1(channel), rCDMA_CHANNEL_CMND2(channel),
	    rCDMA_CHANNEL_CMND3(channel));

	filter = (rCDMA_CHANNEL_CSR(channel) >> 8) & 0xff;
	if (filter > 0) {
		printf("FILT: CSR 0x%08x\n", rCDMA_FILTER_CSR(filter));
		printf("FILT: IV 0x%08x 0x%08x 0x%08x 0x%08x\n",
		    rCDMA_FILTER_IVR0(filter),
		    rCDMA_FILTER_IVR1(filter),
		    rCDMA_FILTER_IVR2(filter),
		    rCDMA_FILTER_IVR3(filter));
		printf("FILT: KEY 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
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
