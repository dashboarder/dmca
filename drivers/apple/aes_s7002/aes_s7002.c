/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
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
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwdmachannels.h>
#include <sys/task.h>

#include "aes_s7002.h"

void _load_input_block(const uint32_t *src);
void _store_output_block(uint32_t *dst);

#if AES_USE_DMA
int 
aes_execute_cmd(uint32_t cmd, void *addr, uint32_t length, bool block)
{
	struct task_event	event;
	struct dma_segment	seg;
	void			*fifo;
	int32_t			ret;
	int 			dma_channel;

	// build an SGL for the one segment
	switch (cmd & DMA_CMD_DIR_MASK) {
		case DMA_CMD_DIR_TX:
			seg.paddr = (uintptr_t)addr;
			fifo = (void*)0x47380040;
			dma_channel = DMA_AES_WR;
			break;
		case DMA_CMD_DIR_RX:
			seg.paddr = (uintptr_t)addr;
			fifo = (void*)0x47380080;
			dma_channel = DMA_AES_RD;
			break;
		default:
			return -1;
	}
	seg.length = length;

	if (block) {
		// run the operation
		event_init(&event, EVENT_FLAG_AUTO_UNSIGNAL, false);
		ret = dma_execute_async(cmd, dma_channel, &seg, fifo, length, 0, 0,
				(dma_completion_function)event_signal, (void *)&event);

		if (!ret)
			// and block waiting for it to complete 
			event_wait(&event);
	} else {
		ret = dma_execute_async(cmd, dma_channel, &seg, fifo, length, 0, 0,
				NULL, NULL);
	}

	return ret;
}
#endif

int
aes_hw_crypto_cmd(u_int32_t cmd, void *src, void *dst, size_t len, u_int32_t opts, const void *key, void *iv)
{
	uint32_t key_in_ctrl;
	uint32_t blocks;
	uint32_t *local_iv, *local_key;

	dprintf(DEBUG_SPEW, "aes_hw_crypto_cmd: cmd: %08x, src: %p, dst: %p, len: %zx, opts: %08x, key: %p, iv: %p \n",
		cmd, src, dst, len, opts, key, iv);

	clock_gate(CLK_AES0, true);

	ASSERT(src != NULL);
	ASSERT(dst != NULL);

	local_iv = (uint32_t *)iv;
	local_key = (uint32_t *)key;

	/* Make sure length is multiple of 16 bytes */
	ASSERT((len > 0) && ((len & 0xf) == 0));
	blocks = len / 16;

	key_in_ctrl = 0;

	switch (cmd & AES_CMD_MODE_MASK) {
		case AES_CMD_ECB : key_in_ctrl |= KEY_IN_CTRL_MOD_ECB; break;
		case AES_CMD_CBC : key_in_ctrl |= KEY_IN_CTRL_MOD_CBC; break;
		default : goto fail;
	}

	switch (cmd & AES_CMD_DIR_MASK) {
		case AES_CMD_ENC : key_in_ctrl |= KEY_IN_CTRL_DIR_ENC; break;
		case AES_CMD_DEC : key_in_ctrl |= KEY_IN_CTRL_DIR_DEC; break;
		default : goto fail;
	}

	/* Get the key length */
	switch (opts & AES_KEY_SIZE_MASK) {
		case AES_KEY_SIZE_128 : key_in_ctrl |= KEY_IN_CTRL_LEN_128; break;
		case AES_KEY_SIZE_192 : key_in_ctrl |= KEY_IN_CTRL_LEN_192; break;
		case AES_KEY_SIZE_256 : key_in_ctrl |= KEY_IN_CTRL_LEN_256; break;
		default : goto fail;
	}

	bool key_disabled = false;

	/* Set the key */
	switch (opts & AES_KEY_TYPE_MASK) {
		case AES_KEY_TYPE_USER :
			key_in_ctrl |= KEY_IN_CTRL_SEL_SW;
			
			switch (opts & AES_KEY_SIZE_MASK) {
				default:
					rAES_S7002_KEY_IN7 = local_key[7];
					rAES_S7002_KEY_IN6 = local_key[6];
				case AES_KEY_SIZE_192:
					rAES_S7002_KEY_IN5 = local_key[5];
					rAES_S7002_KEY_IN4 = local_key[4];
				case AES_KEY_SIZE_128:
					rAES_S7002_KEY_IN3 = local_key[3];
					rAES_S7002_KEY_IN2 = local_key[2];
					rAES_S7002_KEY_IN1 = local_key[1];
					rAES_S7002_KEY_IN0 = local_key[0];
			}
			break;

		case AES_KEY_TYPE_UID0 : 
			key_in_ctrl |= KEY_IN_CTRL_SEL_UID1;
			key_disabled = platform_keys_disabled(0,1); 
			break;
		case AES_KEY_TYPE_GID0 : 
			key_in_ctrl |= KEY_IN_CTRL_SEL_GID0;
			key_disabled = platform_keys_disabled(1,0);
			break;
		case AES_KEY_TYPE_GID1 : 
			key_in_ctrl |= KEY_IN_CTRL_SEL_GID1;
			key_disabled = platform_keys_disabled(1,0);
			break;
	}
	
	if (key_disabled) {
		dprintf(DEBUG_INFO, "SIO_AES: requested AES key has been disabled\n"); 
		return -1;
	}

	/* Make sure AES block is not busy */
	ASSERT(rAES_S7002_KEY_IN_STS & KEY_IN_STS_RDY);
	ASSERT(rAES_S7002_TXT_IN_STS & TXT_IN_FIFO_SPACE_AVAIL);
	ASSERT(!(rAES_S7002_TXT_OUT_STS & TXT_OUT_DATA_AVAIL));

	/* Setup Key configurator, and load the context */
	rAES_S7002_KEY_IN_CTRL = key_in_ctrl;
	rAES_S7002_KEY_IN_CTRL |= KEY_IN_CTRL_VAL_SET;

	dprintf(DEBUG_SPEW, "sio_aes: key_in_ctrl:0x%08x, rAES_S7002_KEY_IN_CTRL:0x%08x\n", key_in_ctrl, rAES_S7002_KEY_IN_CTRL);

	/* Load IV */
        if (iv != 0) {
                rAES_S7002_IV_IN0 = local_iv[0];
                rAES_S7002_IV_IN1 = local_iv[1];
                rAES_S7002_IV_IN2 = local_iv[2];
                rAES_S7002_IV_IN3 = local_iv[3];
        } else {
	        rAES_S7002_IV_IN0 = 0;
	        rAES_S7002_IV_IN1 = 0;
	        rAES_S7002_IV_IN2 = 0;
	        rAES_S7002_IV_IN3 = 0;
        }
	rAES_S7002_IV_IN_CTRL = (0 << IV_IN_CTRL_CTX_SHIFT);
	rAES_S7002_IV_IN_CTRL |= IV_IN_CTRL_VAL_SET;

	/* Perform AES operation */

	/* Load first block */
	rAES_S7002_TXT_IN_CTRL = ((0 << TXT_IN_CTRL_IV_CTX_SHIFT) | (0 << TXT_IN_CTRL_KEY_CTX_SHIFT));

#if AES_USE_DMA
	/* Do cache operations */
	platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, (void *)((uint32_t)dst & ~(CPU_CACHELINE_SIZE-1)), ROUNDUP(len, CPU_CACHELINE_SIZE) + CPU_CACHELINE_SIZE);

	/* start the operation */
	aes_execute_cmd(DMA_CMD_DIR_RX, dst, len, false);
	
	/* Do cache operations */
	platform_cache_operation(CACHE_CLEAN, (void *)((uint32_t)src & ~(CPU_CACHELINE_SIZE-1)), ROUNDUP(len, CPU_CACHELINE_SIZE) + CPU_CACHELINE_SIZE);
	
	/* start the operation */
	aes_execute_cmd(DMA_CMD_DIR_TX, src, len, true);
#else
	uint32_t *local_src = (uint32_t *)src;
	uint32_t *local_dst = (uint32_t *)dst;

	rAES_S7002_TXT_IN_CTRL = ((0 << TXT_IN_CTRL_IV_CTX_SHIFT) | (0 << TXT_IN_CTRL_KEY_CTX_SHIFT));	
	_load_input_block(local_src);
	while(--blocks) {
		/* Wait until we can shove next block in */
		while ((rAES_S7002_TXT_IN_STS & TXT_IN_FIFO_SPACE_AVAIL) != TXT_IN_FIFO_SPACE_AVAIL);
		
		/* Load next block */
		local_src += 4;
		rAES_S7002_TXT_IN_CTRL = ((0 << TXT_IN_CTRL_IV_CTX_SHIFT) | (0 << TXT_IN_CTRL_KEY_CTX_SHIFT));		
		_load_input_block(local_src);
		
		/* Store result of the previous operation */
		while ((rAES_S7002_TXT_OUT_STS & TXT_OUT_DATA_AVAIL) != TXT_OUT_DATA_AVAIL);
		_store_output_block(local_dst);
		local_dst += 4;
	}
	/* Store result of the last operation */
	while ((rAES_S7002_TXT_OUT_STS & TXT_OUT_DATA_AVAIL) != TXT_OUT_DATA_AVAIL);
	_store_output_block(local_dst);
#endif

	/* Clear user key from the registers */
	if ((opts & AES_KEY_TYPE_MASK) == AES_KEY_TYPE_USER) {
		rAES_S7002_KEY_IN7 = 0;
		rAES_S7002_KEY_IN6 = 0;
		rAES_S7002_KEY_IN5 = 0;
		rAES_S7002_KEY_IN4 = 0;
		rAES_S7002_KEY_IN3 = 0;
		rAES_S7002_KEY_IN2 = 0;
		rAES_S7002_KEY_IN1 = 0;
		rAES_S7002_KEY_IN0 = 0;
	}

	clock_gate(CLK_AES0, false);

	return 0;

fail:
	panic("AES_S7002: bad arguments\n");
}

void _load_input_block(const uint32_t *src)
{
	dprintf(DEBUG_SPEW, "src: ");
	for(int i = 0; i < 4; i++) dprintf(DEBUG_SPEW, "%08x ", src[i]);
	dprintf(DEBUG_SPEW, "\n");

	rAES_S7002_TXT_IN = src[0];
	rAES_S7002_TXT_IN = src[1];
	rAES_S7002_TXT_IN = src[2];
	rAES_S7002_TXT_IN = src[3];
}

void _store_output_block(uint32_t *dst) 
{
	dst[0] = rAES_S7002_TXT_OUT;
	dst[1] = rAES_S7002_TXT_OUT;
	dst[2] = rAES_S7002_TXT_OUT;
	dst[3] = rAES_S7002_TXT_OUT;

	dprintf(DEBUG_SPEW, "dst: ");
	for(int i = 0; i < 4; i++) dprintf(DEBUG_SPEW, "%08x ", dst[i]);
	dprintf(DEBUG_SPEW, "\n");
}
