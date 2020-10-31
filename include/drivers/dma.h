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
#ifndef __DRIVERS_DMA_H
#define __DRIVERS_DMA_H

#include <sys/types.h>

__BEGIN_DECLS

#define DMA_CMD_DIR_MEM		(0x00000000)
#define DMA_CMD_DIR_TX		(0x00000001)
#define DMA_CMD_DIR_RX		(0x00000002)
#define DMA_CMD_DIR_MASK	(0x00000003)

void dma_init(void);

/*
 *  dma_execute_cmd - simple DMA operation
 *
 *    cmd         - operation mode: MEM (MEMORY-MEMORY), TX (MEMORY-DEVICE) or RX (DEVICE-MEMORY)
 *    dma_channel - dma channel number
 *    src         - source buffer or fifo
 *    dst         - desination buffer or fifo
 *    length      - size in bytes of the operation
 *    word_size   - size in bytes of each word
 *    burst_size  - size in words of each burst
 *
 *    Returns 0 on success or -1 on failure
 */
int dma_execute_cmd(u_int32_t cmd, int dma_channel,
		    void *src, void *dst, u_int32_t length,
		    u_int32_t word_size, u_int32_t burst_size);


/*
 * dma_execute_async - complex async DMA operation
 *
 *    cmd            - operation mode: MEM (MEMORY-MEMORY), TX (MEMORY-DEVICE) or RX (DEVICE-MEMORY)
 *    dma_channel    - dma channel number
 *    sgl            - memory scatter/gather list
 *    fifo           - device fifo address
 *    length         - size in bytes of the operation
 *    word_size      - size in bytes of each word
 *    burst_size     - size in words of each burst
 *    completion     - completion callback
 *    completion_arg - argument passed to callback
 *
 *    Returns 0 on success or -1 on failure
 *
 */

struct dma_segment {
	u_int32_t	paddr;
	u_int32_t	length;
};

typedef void	(* dma_completion_function)(void *);

int dma_execute_async(u_int32_t cmd, int dma_channel,
		      struct dma_segment *sgl, void *fifo, u_int32_t length, 
		      u_int32_t word_size, u_int32_t burst_size,
		      dma_completion_function completion, void *completion_arg);

void dma_use_int(int dma_channel, bool use);

bool dma_poll(int dma_channel);

/*
 * dma_cancel - cancel DMA operation on a given channel.
 *
 * When this returns, the numbered DMA channel is guaranteed to be idle.
 */

void dma_cancel(int dma_channel);

/*
 * DMA channel encryption
 *
 * A channel is configured to perform AES en/decryption by calling dma_set_aes()
 * with a dma_aes_config structure.  If NULL is passed as an argument, encryption
 * is disabled for the channel; the en/decryption state is also reset at the completion
 * or aborting of the transaction.
 *
 * The caller is responsible for ensuring the dma_aes_config structure remains valid
 * for the duration of the operation.
 *
 * A constant key is used for a given DMA operation, however the caller may specify
 * that a transfer should be broken into constant-sized chunks.  For each chunk, the
 * iv_func hook will be invoked with the chunk index (offset from 0 at the start of
 * the transaction).
 * 
 */

struct dma_aes_config {
	/* AES configuration */
	u_int32_t	command;	/* AES_CMD bits from <drivers/aes.h> */
	u_int32_t	keying;		/* AES_KEY bits from <drivers/aes.h> */
	void		*key;		/* AES key for AES_KEY_TYPE_USER */

	/* IV generation */
	u_int32_t	chunk_size;	/* AES chunk size */
	void		(* iv_func)(void *arg, u_int32_t chunk_index, void *iv_buffer);
	void		*iv_func_arg;
};

int dma_set_aes(int dma_channel, struct dma_aes_config *config);


__END_DECLS

#endif /* __DRIVERS_DMA_H */
