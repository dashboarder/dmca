/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <compiler.h>
#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <libDER/libDER_config.h>
#include <libDER/asn1Types.h>
#include <libDER/DER_Decode.h>
#include "image4_partial.h"

// TODO - libDER/asn1Types.h is missing ASN1_EOC
#ifndef ASN1_EOC
#define ASN1_EOC	0x00
#endif

static int der_expect(const DERItem *bounds, DERTag tag, DERDecodedInfo *info,
		      bool partial);
static int der_restrict(DERItem *bounds, const DERItem *item);
static int der_compare_ia5string(const DERItem *item, const char *s);
static int der_skip_over(DERItem *bounds, const DERItem *item);
static int der_expect_ia5string(DERItem *bounds, const char *s);

#ifdef TEST
static void hexdump(const void *buf, size_t len);
#endif

// Parse a partial buffer enough to identify and get type.
int image4_get_partial(const void *buf, size_t buf_bytes,
		       uint32_t *ret_type, uint32_t *ret_size)
{
	DERItem bounds;
	DERDecodedInfo info;
	// See sample Image4 header at the top of this file.
	bounds.data = (DERByte *) buf;
	bounds.length = buf_bytes;
	dprintf(DEBUG_SPEW, "image4_get_partial %p,%u\n",
		bounds.data, (unsigned) bounds.length);

	// Outer SEQUENCE. Allow partial.
	dprintf(DEBUG_SPEW, "Parse outer SEQUENCE\n");
	if (der_expect(&bounds, ASN1_CONSTR_SEQUENCE, &info, true) != 0)
		return -1;
	// Infer Image4 size from the bounds of the sequence.
	if (ret_size != NULL) {
		DERSize offset = info.content.data - bounds.data;
		if (offset + info.content.length < offset) {
			dprintf(DEBUG_SPEW, "Sneaky size wraparound %llu\n",
				(uint64_t) offset + info.content.length);
			return -1;
		}
		*ret_size = offset + info.content.length;
	}
	// Update bounds to contents of SEQUENCE.
	if (der_restrict(&bounds, &info.content) != 0)
		return -1;

	// IA5String "IMG4". Update bounds to skip over.
	dprintf(DEBUG_SPEW, "Parse IA5String \"IMG4\"\n");
	if (der_expect_ia5string(&bounds, "IMG4") != 0)
		return -1;
	
	// Inner SEQUENCE. Allow partial.
	dprintf(DEBUG_SPEW, "Parse inner sequence\n");
	if (der_expect(&bounds, ASN1_CONSTR_SEQUENCE, &info, true) != 0)
		return -1;
	if (der_restrict(&bounds, &info.content) != 0)
		return -1;

	// IA5String "IM4P". Update bounds to skip over.
	dprintf(DEBUG_SPEW, "Parse IA5String \"IM4P\"\n");
	if (der_expect_ia5string(&bounds, "IM4P") != 0)
		return -1;

	// IA5String "type". We're interested in this one.
	dprintf(DEBUG_SPEW, "Parse IA5String type\n");
	if (der_expect(&bounds, ASN1_IA5_STRING, &info, false) != 0)
		return -1;
	if (info.content.length != sizeof(*ret_type)) {
		dprintf(DEBUG_SPEW, "Expected %d byte tag, got %d\n",
			(int) sizeof(*ret_type), (int) info.content.length);
		return -1;
	}
	if (ret_type != NULL) {
		uint32_t local_type;
		memcpy(&local_type, info.content.data, sizeof(local_type));
		*ret_type = swap32(local_type);
	}
	return 0;
}

static int
der_expect(const DERItem *bounds, DERTag tag, DERDecodedInfo *info,
	   bool partial)
{
	DERReturn ret;
	dprintf(DEBUG_SPEW, "Bounds %p,%u\n",
		bounds->data, (unsigned) bounds->length);
	ret = DERDecodeItemPartialBuffer(bounds, info, partial);
	if (ret != DR_Success) {
		dprintf(DEBUG_SPEW, "Error decoding DER: %d\n", (int) ret);
		return ret;
	} else if (info->tag != tag) {
		dprintf(DEBUG_SPEW,
			"Didn't get expected tag 0x%llx, got 0x%llx\n",
			(uint64_t) tag, (uint64_t) info->tag);
		return -1;
	} else {
		// Got expected tag.
		return 0;
	}
}

static int
der_restrict(DERItem *bounds, const DERItem *item)
{
	// Update *bounds to *item, keeping staying within *bounds.
	RELEASE_ASSERT(item->data >= bounds->data);
	DERByte *buf_start = bounds->data;
	DERByte *buf_end = bounds->data + bounds->length;
	// This should have been guaranteed elsewhere, but worth checking.
	// Zero length returned *bounds allowed.
	if ((item->data < buf_start) || (item->data > buf_end)) {
		dprintf(DEBUG_SPEW, "DERItem.data=%p outside bounds %p,%p\n",
			item->data, buf_start, buf_end);
		return -1;
	}
	// Set *bounds to *item, then restrict range.
	bounds->data = item->data;
	bounds->length = __min((size_t)item->length, (size_t)(buf_end - bounds->data));
	return 0;
}

static int
der_compare_ia5string(const DERItem *item, const char *s)
{
	int cmp;
	size_t len = strlen(s);
	dprintf(DEBUG_SPEW, "content %p,%u\n", item->data, item->length);
	if (item->length != len) {
		dprintf(DEBUG_SPEW, "IA5String expected %s but length %d\n",
			s, (int) len);
		return -1;
	}
	cmp = memcmp(item->data, s, len);
	if (cmp != 0) {
		dprintf(DEBUG_SPEW, "Expected %s, got:\n", s);
		dhexdump(DEBUG_SPEW, item->data, len);
	}
	return cmp;
}

static int
der_skip_over(DERItem *bounds, const DERItem *item)
{
	DERSize skip, offset;
	// Calculate number of bytes to move bounds->data to end of *item.
	RELEASE_ASSERT(item->data >= bounds->data);
	offset = item->data - bounds->data;
	skip = offset + item->length;
	if ((skip < offset) || (skip > bounds->length)) {
		dprintf(DEBUG_SPEW, "Skipping over item -> outside bounds\n");
		return -1;
	}
	bounds->data += skip;
	bounds->length -= skip;
	return 0;
}

static int
der_expect_ia5string(DERItem *bounds, const char *s)
{
	// Get IA5String info, compare data, update bounds to skip over.
	DERDecodedInfo info;
	if (der_expect(bounds, ASN1_IA5_STRING, &info, false) != 0)
		return -1;
	if (der_compare_ia5string(&info.content, s) != 0)
		return -1;
	if (der_skip_over(bounds, &info.content) != 0)
		return -1;
	return 0;
}


#ifdef TEST
// Run test with:
// cc -Werror -Wall -O -o image4_partial image4_partial.c ../../pki/libDER/DER_Decode.c -DTEST -DDEBUG_LEVEL=30 -I../../../include -I../../pki && ./image4_partial
// Try both -m32 and -m64
#include <assert.h>

int main() {
	static const uint8_t big_img4[] = {		// 4GB-1 file
		0x30, 0x84, 0xff, 0xff, 0xff, 0xf9,	// SEQUENCE { 4GB-7
		0x16, 0x04, 0x49, 0x4d, 0x47, 0x34,	// IA5String 'IMG4'
		0x30, 0x84, 0xff, 0xff, 0xff, 0xed,	// SEQUENCE { 4GB-19
		0x16, 0x04, 0x49, 0x4d, 0x34, 0x50,	// IA5String 'IM4P'
		0x16, 0x04, 0x41, 0x42, 0x43, 0x44,	// IA5String 'ABCD'
	};
	uint32_t type, size;
	int ret = image4_get_partial(big_img4, sizeof(big_img4),
				     &type, &size);
	assert(ret == 0);
	assert(memcmp(&type, "ABCD", 4) == 0);
	assert(size == 0xffffffff);
	printf("Pass\n");
	return 0;
}

static void hexdump(const void *buf, size_t len) {
	const uint8_t *p = buf;
	size_t i;
	for (i = 0; i < len; ++i)
		printf("[%llu] = 0x%02x '%c'\n", (uint64_t) i, p[i], p[i]);
}

void _panic(const char *func, const char *str, ...) {
	va_list args;
	va_start(args, str);
	printf("\n%s: ", func);
	vprintf(str, args);
	printf("\n\n");
	va_end(args);
	exit(1);
}
#endif // defined(TEST)
