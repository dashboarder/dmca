/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <drivers/sha1.h>
#include <platform.h>
#include <stdint.h>
#include <string.h>
#include "uuid.h"

const uuid_t url_namespace_uuid = { 0x6ba7b811, 0x9dad, 0x11d1, 0x80, 0xb4, { 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8 } };

void uuid_host_to_network(const uuid_t *source, uint8_t *dest)
{
	dest[0] = source->time_low >> 0;
	dest[1] = source->time_low >> 8;
	dest[2] = source->time_low >> 16;
	dest[3] = source->time_low >> 24;
	dest[4] = source->time_mid >> 0;
	dest[5] = source->time_mid >> 8;
	dest[6] = source->time_hi_and_version >> 0;
	dest[7] = source->time_hi_and_version >> 8;
	dest[8] = source->clock_seq_hi_and_reserved;
	dest[9] = source->clock_seq_low;
	dest[10] = source->node[0];
	dest[11] = source->node[1];
	dest[12] = source->node[2];
	dest[13] = source->node[3];
	dest[14] = source->node[4];
	dest[15] = source->node[5];
}

void uuid_network_to_host(const uint8_t *src, uuid_t *dest)
{
	dest->time_low = src[0] << 24 | src[1] << 16 | src[2] << 8 | src[3];
	dest->time_mid = src[4] << 8 | src[5];
	dest->time_hi_and_version = src[6] << 8 | src[7];
	dest->clock_seq_hi_and_reserved = src[8];
	dest->clock_seq_low = src[9];
	dest->node[0] = src[10];
	dest->node[1] = src[11];
	dest->node[2] = src[12];
	dest->node[3] = src[13];
	dest->node[4] = src[14];
	dest->node[5] = src[15];
}

void uuid_host_to_tbt(const uuid_t *source, uint8_t *dest)
{
	uint32_t dword;

	memcpy(dest, (uint8_t *)source, 4);

	memcpy(dest + 6, (uint8_t *)source + 4, 2);
	memcpy(dest + 4, (uint8_t *)source + 6, 2);

	memcpy(&dword, (uint8_t *)source + 8, 4);
	dword = htonl(dword);
	memcpy(dest + 8, &dword, 4);

	memcpy(&dword, (uint8_t *)source + 12, 4);
	dword = htonl(dword);
	memcpy(dest + 12, &dword, 4);
}

void uuid_tbt_to_host(const uint8_t *source, uuid_t *dest)
{
	uint32_t dword;

	memcpy((uint8_t *)dest, source, 4);

	memcpy((uint8_t *)dest + 6, source + 4, 2);
	memcpy((uint8_t *)dest + 4, source + 6, 2);

	memcpy(&dword, source + 8, 4);
	dword = htonl(dword);
	memcpy((uint8_t *)dest + 8, &dword, 4);

	memcpy(&dword, source + 12, 4);
	dword = htonl(dword);
	memcpy((uint8_t *)dest + 12, &dword, 4);
}

void uuid_generate_v5(const uint8_t *hash, uuid_t *dest)
{
	uuid_network_to_host(hash, dest);
	dest->time_hi_and_version &= 0x0FFF;
	dest->time_hi_and_version |= 5 << 12;
	dest->clock_seq_hi_and_reserved &= 0x3F;
	dest->clock_seq_hi_and_reserved |= 0x80;
}

const uuid_t *uuid_get_device_uuid(void)
{
	uint8_t bytes_to_hash[16 + 33 + 8 + 16 + 1];
	uint8_t hash[CCSHA1_OUTPUT_SIZE];
	char *url;

	static bool uuid_generated = false;
	static uuid_t uuid;

	if (!uuid_generated) {
		uuid_generated = true;

		uuid_host_to_network(&url_namespace_uuid, bytes_to_hash);
		url = (char *)bytes_to_hash + 16;

		const char *format_str = "http://coreos.apple.com/socuuid/%08x/%016llx";
		snprintf(url, sizeof(bytes_to_hash) - 16, format_str, platform_get_chip_id(), platform_get_ecid_id());

		sha1_calculate(bytes_to_hash, sizeof(bytes_to_hash) - 1, hash);

		uuid_generate_v5(hash, &uuid);
	}

	return &uuid;
}

void uuid_print(const uuid_t *src)
{
	printf("%08x-%04x-%04x-%02x%02x-", src->time_low, src->time_mid, src->time_hi_and_version,
			src->clock_seq_hi_and_reserved, src->clock_seq_low);
	for (int i = 0; i < 6; i++)
		printf("%02x", src->node[i]);
}
