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
#include <platform.h>
#include <platform/soc/hwclocks.h>

#include "aes_ap.h"

void _load_input_block(const uint32_t *src);
void _store_output_block(uint32_t *dst);

#if AES_AP_VERSION < 1

void
aes_ap_disable_keys(uint32_t gid, uint32_t uid)
{
	uint32_t keydis = (((gid & AES_DIS_GID_MASK) << AES_DIS_SET_GID_SHIFT) | ((uid & AES_DIS_UID_MASK) << AES_DIS_SET_UID_SHIFT));

	dprintf(DEBUG_SPEW, "gid:0x%08x uid:0x%08x\n", gid, uid);

	if (!gid && !uid) return;

	clock_gate(CLK_AES0, true);
	rAES_AP_DIS = keydis;
	clock_gate(CLK_AES0, false);
}

bool
aes_ap_keys_disabled(uint32_t gid, uint32_t uid)
{
	uint32_t key_disabled;

	dprintf(DEBUG_SPEW, "gid:0x%08x uid:0x%08x\n", gid, uid);

	if (!gid && !uid) return true;

	clock_gate(CLK_AES0, true);
	key_disabled = rAES_AP_DIS;
	clock_gate(CLK_AES0, false);

	return (key_disabled & ((gid & AES_DIS_GID_MASK) << AES_DIS_GID_SHIFT)) && (key_disabled & ((uid & AES_DIS_UID_MASK) << AES_DIS_UID_SHIFT));
}

#endif // AES_AP_VERSION < 1

int
aes_hw_crypto_cmd(u_int32_t cmd, void *src, void *dst, size_t len, u_int32_t opts, const void *key, void *iv)
{
	uint32_t key_in_ctrl;
	uint32_t blocks;
	uint32_t *local_src, *local_dst, *local_iv, *local_key;

	dprintf(DEBUG_SPEW, "aes_hw_crypto_cmd: cmd: %08x, src: %p, dst: %p, len: %08zx, opts: %08x, key: %p, iv: %p \n",
		cmd, src, dst, len, opts, key, iv);

	clock_gate(CLK_AES0, true);

	ASSERT(src != NULL);
	ASSERT(dst != NULL);

	local_src = (uint32_t *)src;
	local_dst = (uint32_t *)dst;
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

	/* Set the key */
	switch (opts & AES_KEY_TYPE_MASK) {
		case AES_KEY_TYPE_USER :
			key_in_ctrl |= KEY_IN_CTRL_SEL_SW;
			
			switch (opts & AES_KEY_SIZE_MASK) {
				default:
					rAES_AP_KEY_IN7 = local_key[7];
					rAES_AP_KEY_IN6 = local_key[6];
				case AES_KEY_SIZE_192:
					rAES_AP_KEY_IN5 = local_key[5];
					rAES_AP_KEY_IN4 = local_key[4];
				case AES_KEY_SIZE_128:
					rAES_AP_KEY_IN3 = local_key[3];
					rAES_AP_KEY_IN2 = local_key[2];
					rAES_AP_KEY_IN1 = local_key[1];
					rAES_AP_KEY_IN0 = local_key[0];
			}
			break;

		case AES_KEY_TYPE_UID0 : key_in_ctrl |= KEY_IN_CTRL_SEL_UID1; break;
		case AES_KEY_TYPE_GID0 : key_in_ctrl |= KEY_IN_CTRL_SEL_GID0; break;
		case AES_KEY_TYPE_GID1 : key_in_ctrl |= KEY_IN_CTRL_SEL_GID1; break;
	}

	/* Make sure the requested key is not disabled */
	if (((opts & AES_KEY_TYPE_MASK) != AES_KEY_TYPE_USER) && ((rAES_AP_DIS & (opts & AES_KEY_TYPE_MASK)) == (opts & AES_KEY_TYPE_MASK))) {
		dprintf(DEBUG_INFO, "SIO_AES: requested AES key has been disabled\n");
		return -1;
	}

	/* Make sure AES block is not busy */
	ASSERT(rAES_AP_KEY_IN_STS & KEY_IN_STS_RDY);
	ASSERT(rAES_AP_TXT_IN_STS & TXT_IN_STS_RDY);
	ASSERT(!(rAES_AP_TXT_OUT_STS & TXT_OUT_STS_VAL_SET));

	/* Setup Key configurator, and load the context */
	rAES_AP_KEY_IN_CTRL = key_in_ctrl;
	rAES_AP_KEY_IN_CTRL |= KEY_IN_CTRL_VAL_SET;

	dprintf(DEBUG_SPEW, "sio_aes: key_in_ctrl:0x%08x, rAES_AP_KEY_IN_CTRL:0x%08x\n", key_in_ctrl, rAES_AP_KEY_IN_CTRL);

	/* Load IV */
        if (iv != 0) {
                rAES_AP_IV_IN0 = local_iv[0];
                rAES_AP_IV_IN1 = local_iv[1];
                rAES_AP_IV_IN2 = local_iv[2];
                rAES_AP_IV_IN3 = local_iv[3];
        } else {
	        rAES_AP_IV_IN0 = 0;
	        rAES_AP_IV_IN1 = 0;
	        rAES_AP_IV_IN2 = 0;
	        rAES_AP_IV_IN3 = 0;
        }
	rAES_AP_IV_IN_CTRL = (0 << IV_IN_CTRL_CTX_SHIFT);
	rAES_AP_IV_IN_CTRL |= IV_IN_CTRL_VAL_SET;

	/* Perform AES operation */

	/* Load first block */
	_load_input_block(local_src);
	rAES_AP_TXT_IN_CTRL = ((0 << TXT_IN_CTRL_IV_CTX_SHIFT) | (0 << TXT_IN_CTRL_KEY_CTX_SHIFT) | TXT_IN_CTRL_VAL_SET);
	while(--blocks) {
		/* Wait until we can shove next block in */
		while ((rAES_AP_TXT_IN_STS & TXT_IN_STS_RDY) != TXT_IN_STS_RDY);
		
		/* Load next block */
		local_src += 4;
		_load_input_block(local_src);
		rAES_AP_TXT_IN_CTRL = ((0 << TXT_IN_CTRL_IV_CTX_SHIFT) | (0 << TXT_IN_CTRL_KEY_CTX_SHIFT));
		rAES_AP_TXT_IN_CTRL |= TXT_IN_CTRL_VAL_SET;
		
		/* Store result of the previous operation */
		while ((rAES_AP_TXT_OUT_STS & TXT_OUT_STS_VAL_SET) != TXT_OUT_STS_VAL_SET);
		_store_output_block(local_dst);
		local_dst += 4;
	}
	/* Store result of the last operation */
	while ((rAES_AP_TXT_OUT_STS & TXT_OUT_STS_VAL_SET) != TXT_OUT_STS_VAL_SET);
	_store_output_block(local_dst);

	/* Clear user key from the registers */
	if ((opts & AES_KEY_TYPE_MASK) == AES_KEY_TYPE_USER) {
		rAES_AP_KEY_IN7 = 0;
		rAES_AP_KEY_IN6 = 0;
		rAES_AP_KEY_IN5 = 0;
		rAES_AP_KEY_IN4 = 0;
		rAES_AP_KEY_IN3 = 0;
		rAES_AP_KEY_IN2 = 0;
		rAES_AP_KEY_IN1 = 0;
		rAES_AP_KEY_IN0 = 0;
	}

	clock_gate(CLK_AES0, false);

	return 0;

fail:
	panic("SIO_AES: bad arguments\n");
}

void _load_input_block(const uint32_t *src)
{
	dprintf(DEBUG_SPEW, "src: ");
	for(int i = 0; i < 4; i++) dprintf(DEBUG_SPEW, "%08x ", src[i]);
	dprintf(DEBUG_SPEW, "\n");

	rAES_AP_TXT_IN0 = src[0];
	rAES_AP_TXT_IN1 = src[1];
	rAES_AP_TXT_IN2 = src[2];
	rAES_AP_TXT_IN3 = src[3];
}

void _store_output_block(uint32_t *dst) 
{
	dst[0] = rAES_AP_TXT_OUT0;
	dst[1] = rAES_AP_TXT_OUT1;
	dst[2] = rAES_AP_TXT_OUT2;
	dst[3] = rAES_AP_TXT_OUT3;

	dprintf(DEBUG_SPEW, "dst: ");
	for(int i = 0; i < 4; i++) dprintf(DEBUG_SPEW, "%08x ", dst[i]);
	dprintf(DEBUG_SPEW, "\n");
}
