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
#if WITH_HW_SEP
#include <drivers/sep/sep_client.h>
#endif
#include <platform.h>
#include <platform/soc/hwclocks.h>

#include "aes_v2.h"

#define COMMAND_FIFO_SIZE 128

typedef enum {
	BLOCK_MODE_ECB = 0,
	BLOCK_MODE_CBC = 1,
	BLOCK_MODE_CTR = 2,
} block_mode_t;

typedef enum {
	KEY_LEN_128 = 0,
	KEY_LEN_192 = 1,
	KEY_LEN_256 = 2,
} key_len_t;

typedef enum {
	KEY_SELECT_SOFTWARE = 0,
	KEY_SELECT_UID1 = 1,
	KEY_SELECT_GID_AP_1 = 2,
	KEY_SELECT_GID_AP_2 = 3,
	KEY_SELECT_HDCP_0 = 4,
	KEY_SELECT_HDCP_1 = 5,
	KEY_SELECT_HDCP_2 = 6,
	KEY_SELECT_HDCP_3 = 7,
} key_select_t;

typedef enum {
	KEY_FUNC_NONE = 0,
	KEY_FUNC_LEGACY = 1,
	KEY_FUNC_FAIRPLAY_LEGACY = 2,
	KEY_FUNC_FAIRPLAY_H8F = 3,
} key_func_t;

typedef enum {
	OPCODE_KEY       = 0x01,
	OPCODE_IV        = 0x02,
	OPCODE_DSB       = 0x03,
	OPCODE_SKG       = 0x04,
	OPCODE_DATA      = 0x05,
	OPCODE_STORE_IV  = 0x06,
	OPCODE_WRITE_REG = 0x07,
	OPCODE_FLAG      = 0x08,
} command_opcodes_t;

#define COMMAND_OPCODE_SHIFT	 (28)
#define COMMAND_OPCODE_MASK      (0xF)

typedef struct command_key {
	#define COMMAND_KEY_COMMAND_KEY_CONTEXT_SHIFT (27)
	#define COMMAND_KEY_COMMAND_KEY_CONTEXT_MASK (0x1)

	#define COMMAND_KEY_COMMAND_KEY_SELECT_SHIFT (24)
	#define COMMAND_KEY_COMMAND_KEY_SELECT_MASK  (0x7)

	#define COMMAND_KEY_COMMAND_KEY_LENGTH_SHIFT (22)
	#define COMMAND_KEY_COMMAND_KEY_LENGTH_MASK  (0x3)

	#define COMMAND_KEY_COMMAND_WRAPPED_SHIFT (21)
	#define COMMAND_KEY_COMMAND_WRAPPED_MASK     (0x1)

	#define COMMAND_KEY_COMMAND_ENCRYPT_SHIFT (20)
	#define COMMAND_KEY_COMMAND_ENCRYPT_MASK     (0x1)

	#define COMMAND_KEY_COMMAND_KEY_FUNC_SHIFT (18)
	#define COMMAND_KEY_COMMAND_KEY_FUNC_MASK    (0x3)

	#define COMMAND_KEY_COMMAND_BLOCK_MODE_SHIFT (16)
	#define COMMAND_KEY_COMMAND_BLOCK_MODE_MASK  (0x3)

	#define COMMAND_KEY_COMMAND_COMMAND_ID_SHIFT (0)
	#define COMMAND_KEY_COMMAND_COMMAND_ID_MASK  (0xFF)
	uint32_t command;
	uint32_t key[8];
} command_key_t;

typedef struct aes_command_iv {
	#define COMMAND_IV_COMMAND_IV_CONTEXT_SHIFT (26)
	#define COMMAND_IV_COMMAND_IV_CONTEXT_MASK (0x3)
	
	#define COMMAND_IV_COMMAND_HDCP_KEY_SHIFT (25)
	#define COMMAND_IV_COMMAND_HDCP_KEY_MASK (0x1)
	
	#define COMMAND_IV_COMMAND_IV_IN_HDCP_SHIFT (23)
	#define COMMAND_IV_COMMAND_IV_IN_HDCP_MASK (0x3)
	uint32_t command;
	uint32_t iv[4];
} command_iv_t;

typedef struct aes_command_data {
	#define COMMAND_DATA_COMMAND_KEY_CONTEXT_SHIFT (27)
	#define COMMAND_DATA_COMMAND_KEY_CONTEXT_MASK (0x1)
	
	#define COMMAND_DATA_COMMAND_IV_CONTEXT_SHIFT (25)
	#define COMMAND_DATA_COMMAND_IV_CONTEXT_MASK (0x3)
	
	#define COMMAND_DATA_COMMAND_LENGTH_SHIFT (0)
	#define COMMAND_DATA_COMMAND_LENGTH_MASK (0xFFFFFF)
	uint32_t command;
	#define COMMAND_DATA_UPPER_ADDR_SOURCE_SHIFT (16)
	#define COMMAND_DATA_UPPER_ADDR_SOURCE_MASK (0xFF)
	
	#define COMMAND_DATA_UPPER_ADDR_DEST_SHIFT (0)
	#define COMMAND_DATA_UPPER_ADDR_DEST_MASK (0xFF)
	uint32_t upper_addr;
	uint32_t source_addr;
	uint32_t dest_addr;
} command_data_t;

typedef struct aes_command_store_iv {
	#define COMMAND_STORE_IV_COMMAND_CONTEXT_SHIFT (26)
	#define COMMAND_STORE_IV_COMMAND_CONTEXT_MASK (0x3)
	
	#define COMMAND_STORE_IV_COMMAND_UPPER_ADDR_DEST_SHIFT (0)
	#define COMMAND_STORE_IV_COMMAND_UPPER_ADDR_DEST_MASK (0xFF)
	uint32_t command;
	uint32_t dest_addr;
} command_store_iv_t;

#define COMMAND_FLAG_ID_CODE_SHIFT (0)
#define COMMAND_FLAG_ID_CODE_MASK (0xFF)
#define COMMAND_FLAG_STOP_COMMANDS_SHIFT (26)
#define COMMAND_FLAG_SEND_INTERRUPT_SHIFT (27)

#if WITH_DPA
static bool dpa_seeded(void) {
	if ((rAES_STATUS & AES_BLK_STATUS_TEXT_DPA_RANDOM_SEEDED_UMASK) == 0 ||
	    (rAES_STATUS & AES_BLK_STATUS_KEY_UNWRAP_DPA_RANDOM_SEEDED_UMASK) == 0) {
		return false;
	} else {
		return true;
	}
}
#endif

uint32_t command_fifo_level(void) {
	return ((rAES_COMMAND_FIFO_STATUS) & AES_BLK_COMMAND_FIFO_STATUS_LEVEL_UMASK) >> 8;
}

void push_commands(uint32_t *cmd, uint32_t size) {
	uint32_t words;
	ASSERT(size % sizeof(uint32_t) == 0);

	words = size / sizeof(uint32_t);

	ASSERT(command_fifo_level() + words <= COMMAND_FIFO_SIZE);

	for (uint32_t i = 0; i < words; i ++) {
		dprintf(DEBUG_SPEW, "sio_aes: cmd[%d]: 0x%08x\n", i, cmd[i]);
		rAES_COMMAND_FIFO = cmd[i];
	}
}

uint32_t key_size(uint8_t mode) {
	switch(mode) {
		case KEY_LEN_128: return 128;
		case KEY_LEN_192: return 192;
		case KEY_LEN_256: return 256;
		default: panic("Unknown key size: %d\n", mode);
	}
	return 0;
}

void push_command_key(uint32_t command, uint8_t *key) 
{
	command_key_t ckey;

	memset(&ckey, 0, sizeof(ckey));
	
	ckey.command = OPCODE_KEY << COMMAND_OPCODE_SHIFT | command;
	
	if(((command >> COMMAND_KEY_COMMAND_KEY_SELECT_SHIFT) & COMMAND_KEY_COMMAND_KEY_SELECT_MASK) == 0) {
		// software key (Key Select == 0)
		ASSERT(key != NULL);
		
		uint32_t key_len = 
			key_size((command >> COMMAND_KEY_COMMAND_KEY_LENGTH_SHIFT) & COMMAND_KEY_COMMAND_KEY_LENGTH_MASK) / 8;
		memcpy(ckey.key, key, key_len);

		// Key is not wrapped
		push_commands((uint32_t*)&ckey.command, sizeof(ckey.command));
		push_commands((uint32_t*)&ckey.key[0], key_len);
	} else {
		// non-software keys just need the command word
		push_commands((uint32_t*)&ckey.command, sizeof(ckey.command));
	}
}

void push_command_iv(uint8_t context, uint8_t hdcp_key, bool iv_in_hdcp, uint8_t* iv) 
{
	command_iv_t civ;
	
	memset(&civ, 0, sizeof(civ));
	
	civ.command = (OPCODE_IV << COMMAND_OPCODE_SHIFT) |
		((context & COMMAND_IV_COMMAND_IV_CONTEXT_MASK) << COMMAND_IV_COMMAND_IV_CONTEXT_SHIFT) |
		((hdcp_key & COMMAND_IV_COMMAND_HDCP_KEY_MASK) << COMMAND_IV_COMMAND_HDCP_KEY_SHIFT) |
		((iv_in_hdcp & COMMAND_IV_COMMAND_IV_IN_HDCP_MASK) << COMMAND_IV_COMMAND_IV_IN_HDCP_SHIFT);
	
	if (iv) {
		memcpy(&civ.iv, iv, sizeof(civ.iv));
	}
	
	push_commands((uint32_t*)&civ, sizeof(command_iv_t));
}

void push_command_data(uint8_t key_ctx, uint8_t iv_ctx, uint64_t src, uint64_t dst, uint32_t len) 
{
	command_data_t data;
	
	memset(&data, 0, sizeof(data));
	
	data.command = (OPCODE_DATA << COMMAND_OPCODE_SHIFT) |
		((len & COMMAND_DATA_COMMAND_LENGTH_MASK) << COMMAND_DATA_COMMAND_LENGTH_SHIFT) |
		((key_ctx & COMMAND_DATA_COMMAND_KEY_CONTEXT_MASK) << COMMAND_DATA_COMMAND_KEY_CONTEXT_SHIFT) |
		((iv_ctx & COMMAND_DATA_COMMAND_IV_CONTEXT_MASK) << COMMAND_DATA_COMMAND_IV_CONTEXT_SHIFT);
	data.upper_addr =
		(((src >> 32) & COMMAND_DATA_UPPER_ADDR_SOURCE_MASK) << COMMAND_DATA_UPPER_ADDR_SOURCE_SHIFT) |
		(((dst >> 32) & COMMAND_DATA_UPPER_ADDR_DEST_MASK) << COMMAND_DATA_UPPER_ADDR_DEST_SHIFT);
	data.source_addr = (uint32_t)src;
	data.dest_addr = (uint32_t)dst;
	
	push_commands((uint32_t*)&data, sizeof(command_data_t));
}

void push_command_flag(uint16_t id_code, bool stop, bool interrupt)
{
	uint32_t flag = 0;
	
	flag = (OPCODE_FLAG << COMMAND_OPCODE_SHIFT) |
		((id_code & COMMAND_FLAG_ID_CODE_MASK) << COMMAND_FLAG_ID_CODE_SHIFT);

	if (stop)
		flag |= 1 << COMMAND_FLAG_SEND_INTERRUPT_SHIFT;
	if (interrupt)
		flag |= 1 << COMMAND_FLAG_STOP_COMMANDS_SHIFT;

	push_commands(&flag, sizeof(flag));
}

uint32_t create_key_command(bool context, uint8_t select, uint8_t len, bool wrapped, bool encrypt, uint8_t func, uint8_t mode, uint8_t id) {
	return (OPCODE_KEY << COMMAND_OPCODE_SHIFT) |
		((context & COMMAND_KEY_COMMAND_KEY_CONTEXT_MASK) << COMMAND_KEY_COMMAND_KEY_CONTEXT_SHIFT) |
		((select & COMMAND_KEY_COMMAND_KEY_SELECT_MASK) << COMMAND_KEY_COMMAND_KEY_SELECT_SHIFT) |
		((len & COMMAND_KEY_COMMAND_KEY_LENGTH_MASK) << COMMAND_KEY_COMMAND_KEY_LENGTH_SHIFT) |
		((wrapped & COMMAND_KEY_COMMAND_WRAPPED_MASK) << COMMAND_KEY_COMMAND_WRAPPED_SHIFT) |
		((encrypt & COMMAND_KEY_COMMAND_ENCRYPT_MASK) << COMMAND_KEY_COMMAND_ENCRYPT_SHIFT) |
		((func & COMMAND_KEY_COMMAND_KEY_FUNC_MASK) << COMMAND_KEY_COMMAND_KEY_FUNC_SHIFT) |
		((mode & COMMAND_KEY_COMMAND_BLOCK_MODE_MASK) << COMMAND_KEY_COMMAND_BLOCK_MODE_SHIFT) |
		((id & COMMAND_KEY_COMMAND_COMMAND_ID_MASK) << COMMAND_KEY_COMMAND_COMMAND_ID_SHIFT);
}

void wait_for_command_flag(void)
{
	while ((rAES_INT_STATUS & AES_BLK_INT_STATUS_FLAG_COMMAND_UMASK) == 0) {}

	rAES_INT_STATUS = AES_BLK_INT_STATUS_FLAG_COMMAND_UMASK;

	dprintf(DEBUG_SPEW, "sio_aes: AES_STATUS=0x%08x\n", rAES_STATUS);
	dprintf(DEBUG_SPEW, "sio_aes: AES_COMMAND_FIFO_STATUS=0x%08x\n", rAES_COMMAND_FIFO_STATUS);
	dprintf(DEBUG_SPEW, "sio_aes: INT_STATUS=0x%08x\n", rAES_INT_STATUS);

	ASSERT((rAES_COMMAND_FIFO_STATUS & AES_BLK_COMMAND_FIFO_STATUS_EMPTY_UMASK) != 0);
	if (rAES_INT_STATUS) {
		panic("AES command failed, AES_STATUS=0x%08x, AES_COMMAND_FIFO_STATUS=0x%08x, INT_STATUS=0x%08x", rAES_STATUS, rAES_COMMAND_FIFO_STATUS, rAES_INT_STATUS);
	}
}

int
aes_hw_crypto_cmd(u_int32_t cmd, void *src, void *dst, size_t len, u_int32_t opts, const void *key, void *iv)
{
	uintptr_t *local_src, *local_dst;
	uint8_t *local_iv, *local_key;

	dprintf(DEBUG_SPEW, "aes_hw_crypto_cmd: cmd: 0x%08x, src: %p, dst: %p, len: 0x%08zx, opts: 0x%08x, key: %p, iv: %p \n",
		cmd, src, dst, len, opts, key, iv);

	clock_gate(CLK_AES0, true);
	
#if WITH_DPA
	if (!dpa_seeded()) {
		sep_client_seed_aes();
		if(!dpa_seeded()) {
			panic("AES DPA not Seeded");
		}
	}
#endif

	ASSERT(src != NULL);
	ASSERT(dst != NULL);

	local_src = (uintptr_t *)src;
	local_dst = (uintptr_t *)dst;
	local_iv = (uint8_t *)iv;
	local_key = (uint8_t *)key;

	/* Make sure length is multiple of 16 bytes */
	ASSERT((len > 0) && ((len & 0xf) == 0));

	uint8_t key_select = KEY_SELECT_SOFTWARE;
	uint8_t key_len = KEY_LEN_128;
	uint8_t key_mode = BLOCK_MODE_ECB;
	bool encrypt = false;

	switch (cmd & AES_CMD_MODE_MASK) {
		case AES_CMD_ECB : key_mode = BLOCK_MODE_ECB; break;
		case AES_CMD_CBC : key_mode = BLOCK_MODE_CBC; break;
		default : goto fail;
	}

	switch (cmd & AES_CMD_DIR_MASK) {
		case AES_CMD_ENC : encrypt = true; break;
		case AES_CMD_DEC : encrypt = false; break;
		default : goto fail;
	}

	/* Get the key length */
	switch (opts & AES_KEY_SIZE_MASK) {
		case AES_KEY_SIZE_128 : key_len = KEY_LEN_128; break;
		case AES_KEY_SIZE_192 : key_len = KEY_LEN_192; break;
		case AES_KEY_SIZE_256 : key_len = KEY_LEN_256; break;
		default : goto fail;
	}
	
	bool key_disabled = false;

	/* Set the key */
	/* Make sure the requested key is not disabled */
	switch (opts & AES_KEY_TYPE_MASK) {
	case AES_KEY_TYPE_USER : 
		key_select = KEY_SELECT_SOFTWARE;
		break;
	case AES_KEY_TYPE_UID0 : 
		key_select = KEY_SELECT_UID1;
		key_disabled = platform_keys_disabled(0,1);
		break;
	case AES_KEY_TYPE_GID0 : 
		key_select = KEY_SELECT_GID_AP_1;
		key_disabled = platform_keys_disabled(1,0);
		break;
	case AES_KEY_TYPE_GID1 : 
		key_select = KEY_SELECT_GID_AP_2;
		key_disabled = platform_keys_disabled(2,0);
		break;
	default : goto fail;
	}
	
	if (key_disabled) {
		dprintf(DEBUG_INFO, "SIO_AES: requested AES key has been disabled\n"); 
		return -1;
	}

	uint32_t key_command = 
		create_key_command(0, key_select, key_len, false, encrypt, KEY_FUNC_NONE, key_mode, 0);

	dprintf(DEBUG_SPEW, "sio_aes: key_command:0x%08x\n", key_command);
	
	/* Verify everything is flushed out of write buffers */
	platform_memory_barrier();

	// We're going to poll for the flag interrupt after the command finishes, so clear it now
	rAES_INT_STATUS = AES_BLK_INT_STATUS_FLAG_COMMAND_UMASK;

	/* Start the block consuming commands */
	dprintf(DEBUG_SPEW, "sio_aes: AES_CONTROL_START\n");
	rAES_CONTROL = AES_BLK_CONTROL_START_UMASK;

#if WITH_NON_COHERENT_DMA
	platform_cache_operation(CACHE_CLEAN, (void *)((uint32_t)local_key & ~(CPU_CACHELINE_SIZE-1)), ROUNDUP(len, CPU_CACHELINE_SIZE) + CPU_CACHELINE_SIZE);
#endif /*WITH_NON_COHERENT_DMA*/

	// Push the key command out to the block
	push_command_key(key_command, local_key);

#if WITH_NON_COHERENT_DMA
	platform_cache_operation(CACHE_CLEAN, (void *)((uint32_t)local_iv & ~(CPU_CACHELINE_SIZE-1)), ROUNDUP(len, CPU_CACHELINE_SIZE) + CPU_CACHELINE_SIZE);
#endif /* WITH_NON_COHERENT_DMA */

	/* Load IV */
	push_command_iv(0, 0, 0, local_iv);

#if WITH_NON_COHERENT_DMA
	platform_cache_operation(CACHE_CLEAN, (void *)((uint32_t)local_src & ~(CPU_CACHELINE_SIZE-1)), ROUNDUP(len, CPU_CACHELINE_SIZE) + CPU_CACHELINE_SIZE);
#endif /*WITH_NON_COHERENT_DMA*/

	/* Perform AES operation */
	push_command_data(0, 0, (uint64_t)local_src, (uint64_t)local_dst, len);

	// Raise an interrupt and stop executing commands once all AF responses are received
	push_command_flag(0, true, true);

	wait_for_command_flag();

	platform_memory_barrier();

#if WITH_NON_COHERENT_DMA
	platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, (void *)((uint32_t)local_dst & ~(CPU_CACHELINE_SIZE-1)), ROUNDUP(len, CPU_CACHELINE_SIZE) + CPU_CACHELINE_SIZE);
#endif /*WITH_NON_COHERENT_DMA*/

	/* Stop the block consuming commands */
	dprintf(DEBUG_SPEW, "sio_aes: AES_CONTROL_STOP\n");
	rAES_CONTROL = AES_BLK_CONTROL_STOP_UMASK;
	
	clock_gate(CLK_AES0, false);

	return 0;

fail:
	panic("SIO_AES: bad arguments\n");
}
