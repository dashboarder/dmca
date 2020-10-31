#ifndef ENDIAN_TOOLS_H
#define ENDIAN_TOOLS_H	1

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

#include <stdint.h>

static inline uint16_t read_le_16(const void *buf, size_t offset)
{
	const uint8_t *p = (const uint8_t *) buf + offset;
	return
		(((uint16_t) p[0]) << 0) |
		(((uint16_t) p[1]) << 8);
}

static inline uint32_t read_le_32(const void *buf, size_t offset)
{
	const uint8_t *p = (const uint8_t *) buf + offset;
	return
		(((uint32_t) p[0]) << 0) |
		(((uint32_t) p[1]) << 8) |
		(((uint32_t) p[2]) << 16) |
		(((uint32_t) p[3]) << 24);
}

static inline void write_le_16(void *buf, size_t offset, uint16_t value)
{
	uint8_t *p = (uint8_t *) buf + offset;
	p[0] = (value >> 0) & 0xff;
	p[1] = (value >> 8) & 0xff;
}

static inline void write_le_32(void *buf, size_t offset, uint32_t value)
{
	uint8_t *p = (uint8_t *) buf + offset;
	p[0] = (value >> 0) & 0xff;
	p[1] = (value >> 8) & 0xff;
	p[2] = (value >> 16) & 0xff;
	p[3] = (value >> 24) & 0xff;
}

#endif  // defined(ENDIAN_TOOLS_H)
