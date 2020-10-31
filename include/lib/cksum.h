/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __LIB_CKSUM_H
#define __LIB_CKSUM_H

#include <stdint.h>
#include <sys/types.h>

__BEGIN_DECLS

/* various checksum & crc routines */

 	/* POSIX 1003.2 checksum (same as cksum command) */
#define INITIAL_CRC (0)
uint32_t crc(const void *buf, size_t len);

	/* same as above, but on a single byte */
uint32_t crc_byte(uint32_t thecrc, uint32_t byte_val);

	/* adler32 */
uint32_t adler32(const uint8_t *buf, long len);

	/* crc32 CCITT*/
uint32_t update_crc32(uint32_t crc, const uint8_t *buf, int len);

uint32_t crc32(const uint8_t *buf, int len);

void siphash(uint8_t *out, const uint8_t *in, size_t inlen, const uint8_t *k);
void siphash_aligned(uint64_t *out, const uint64_t *in_aligned, size_t inlen, const uint64_t *k);

#define SIPHASH_KEY_SIZE	(16)
#define SIPHASH_HASH_SIZE	(8)

__END_DECLS

#endif

