/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <platform/clocks.h>
#include <platform/int.h>
#include <platform/soc/hwclocks.h>
#include <drivers/sha1.h>

#include "sha2.h"

#if (BYTE_ORDER != LITTLE_ENDIAN)
#error "Sha2 driver currently requires a little endian machine"
#endif

#define SHA2_BYTE_REVERSE	0

/* Eventually these move to a global header, but for now we're not
 * exporting anything beyond sha1_hw_calculate */
#define SHA_160		160
#define SHA_224		224
#define SHA_256		256
#define SHA_384		384
#define SHA_512		512

static void sha_hw_calculate(int sha_type, const void *buf, size_t len,
			     u_int32_t *result)
{
	const u_int32_t *wptr;
	const u_int8_t *bptr;
	u_int32_t cmd = SHA2_MSGCTL_FIRST, final[3];
	size_t blkwords = (sha_type < SHA_384) ? 16 : 32, hashsize;
	unsigned datawords, words, cur_word, index, type, blocks=0;
	u_int64_t totalbits = len*8;

	clock_gate(CLK_SHA2, true);

	/* XXX use CDMA? */

	/* PIO sequence:
	   - configure the registers
	   - load the first block
	   - issue rdy command with first flag
	   - loop:
	     - load next block
	     - issue rdy command
	     - poll til rdy clears
	   - wait for hash idle
	   - read hash value
	*/
	switch (sha_type) {
	case SHA_160:
		type = SHA_TYPE_160;
		hashsize = 5;
		break;
	case SHA_224:
		type = SHA_TYPE_224;
		hashsize = 7;
		break;
	case SHA_256:
		type = SHA_TYPE_256;
		hashsize = 8;
		break;
	case SHA_384:
		type = SHA_TYPE_384;
		hashsize = 12;
		break;
	case SHA_512:
		type = SHA_TYPE_512;
		hashsize = 16;
		break;
	default:
		panic("Unknown sha type\n");
	}

	rSHA2_CONFIG = SHA2_CONFIG_TYPE(type) | SHA2_PIO_MODE;

	datawords = len / 4;
	final[0] = 0x80;
	if (len & 3) {
		bptr = (const u_int8_t *)(buf + (datawords*4));
		int leftover = len & 3;
		if (leftover == 1)
			final[0] = bptr[0] | (1 << 15);
		else if (leftover == 2)
			final[0] = bptr[0] | ((unsigned)bptr[1] << 8) | (1 << 23);
		else if (leftover == 3)
			final[0] = bptr[0] | ((unsigned)bptr[1] << 8) | ((unsigned)bptr[2] << 16) | (1 << 31);
	}
#if SHA2_BYTE_REVERSE
	final[0] = swap32(final[0]);
	final[1] = totalbits >> 32;
	final[2] = totalbits & 0xffffffff;
#else
	/* Big endian 64-bit bit-length */
	final[1] = swap32(totalbits >> 32);
	final[2] = swap32(totalbits & 0xffffffff);
#endif

	wptr = (u_int32_t *)buf;
	/* This loop takes care of all the complete words in the data,
	   performing the SHA on any complete message blocks along the
	   way. */
	words = (datawords + 3 + (blkwords - 1)) & ~(blkwords - 1);
	for (index = 0, cur_word = 0; cur_word < words; cur_word++, buf+=4) {
		if (cur_word < datawords) {
#if WITH_UNALIGNED_MEM
#if SHA2_BYTE_REVERSE
			rSHA2_MSGBLK(index) = swap32(*wptr);
			wptr++;
#else
			rSHA2_MSGBLK(index) = *wptr++; /* rely on little endianness */
#endif
#else
			rSHA2_MSGBLK(index) = (buf[0] << 0)|(buf[1] << 8)|(buf[2] << 16)|(buf[3] << 24);
#endif
		} else {
			if (cur_word == datawords)
				rSHA2_MSGBLK(index) = final[0];
			else if (cur_word == words-2)
				rSHA2_MSGBLK(index) = final[1];
			else if (cur_word == words-1)
				rSHA2_MSGBLK(index) = final[2];
			else
				rSHA2_MSGBLK(index) = 0;
		}
		if (++index == blkwords) {
			index = 0;
			rSHA2_MSGCTL = SHA2_MSGCTL_RDY | cmd;
			if (cmd)
				cmd = 0;
			else
				while (rSHA2_MSGCTL & SHA2_MSGCTL_RDY);
			blocks++;
		}
	}

	while (rSHA2_STATUS & SHA2_HASH_BUSY);

	for (index = 0; index < hashsize; index++, result++)
#if SHA2_BYTE_REVERSE
		*result = swap32(rSHA2_HASH(index));
#else
		*result = rSHA2_HASH(index); /* again exploit little endianness */
#endif

	clock_gate(CLK_SHA2, false);
}

void sha1_hw_calculate(const void *buf, size_t len, u_int32_t *result)
{
	sha_hw_calculate(SHA_160, buf, len, result);
}

#if 0
#include <debug.h>
#include <sys/menu.h>

#if defined(WITH_MENU) && WITH_MENU

int do_sha(int argc, struct cmd_arg *args)
{
	u_int8_t hash[64];
	u_int32_t cnt, algo = SHA_160;
	void *buf;
	size_t len;

	if (argc < 3) {
		printf("not enough arguments.\n");
		printf("%s [160,224,256,384,512] <address> <len>\n", args[0].str);
		printf("   default is SHA-1 (160)\n");
		return -1;
	}

	if (argc == 4) {
		algo = args[1].u;
		buf = (void *)args[2].u;
		len = args[3].u;
	} else {
		buf = (void *)args[1].u;
		len = args[2].u;
	}
	if (!security_allow_memory(buf, len)) {
		printf("Permission Denied\n");
		return -1;
	}

	memset(hash, 0, sizeof(hash));
	sha_hw_calculate(algo, buf, len, (u_int32_t *)hash);
	printf("sha-%d of %p for 0x%08lx bytes: ", algo, buf, len);
	for (cnt = 0; cnt < (algo/8); cnt++)
		printf("%02x%s", hash[cnt]);
	printf("\n");

	return 0;
}

MENU_COMMAND_DEVELOPMENT(sha, do_sha, "SHA hash of memory", NULL);

#endif
#endif
