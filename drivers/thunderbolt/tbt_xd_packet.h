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
#ifndef TBT_XD_PACKET_H
#define TBT_XD_PACKET_H

#include <debug.h>
#include "tbt_protocol.h"

// Sets the length of field of an xdomain packet. The length is given
// in bytes, but must be a multiple of 4
static inline void tbt_xd_packet_set_len(uint8_t *packet, uint8_t len)
{
	ASSERT((len & 0x3) == 0);

	// length (in dwords) is in bits 0-5 of the DWORD 2
	// clients include the length of the header when calling, but
	// the spec doesn't include the header length, so subtract 12
	packet[11] = (len - 12) >> 2;
}

// Sets the sequence number field of an xdomain packet. The sequence
// number must be a 2-bit number
static inline void tbt_xd_packet_set_seq(uint8_t *packet, uint8_t seq)
{
	ASSERT((seq & ~0x3U) == 0);

	// sequence number is in bits 27-28 of the DWORD 2
	packet[8] = seq << 3;
}

// Gets the sequence number field of an xdomain packet
static inline uint8_t tbt_xd_packet_get_seq(const uint8_t *packet)
{
	// sequence number is in bits 27-28 of the DWORD 2
	return (packet[8] >> 3) & 0x3;
}

/* Sets the protocol UUID of the given xdomain request or response packet to the
 * given UUID in Thunderbolt network byte order
 */
static inline void tbt_xd_packet_set_protocol_uuid(uint8_t *packet, const uint8_t *uuid)
{
	memcpy(packet + TBT_XD_REQUEST_PROTOCOL_UUID_OFFSET, uuid, 16);
}

/* Compares the protocol UUID of the given xdomain request or response packet to the
 * given UUID in Thunderbolt network byte order
 */
static inline int tbt_xd_packet_compare_protocol_uuid(const uint8_t *packet, const uint8_t *uuid)
{
	return memcmp(packet + TBT_XD_REQUEST_PROTOCOL_UUID_OFFSET, uuid, 16);
}

static inline uint32_t tbt_xd_packet_get_type(const uint8_t *packet)
{
	uint32_t packet_type;
	memcpy(&packet_type, packet + TBT_XD_REQUEST_PACKET_TYPE_OFFSET, sizeof(packet_type));
	return ntohl(packet_type);
}

static inline void tbt_xd_packet_set_type(uint8_t *packet, uint32_t packet_type)
{
	packet_type = htonl(packet_type);
	memcpy(packet + TBT_XD_REQUEST_PACKET_TYPE_OFFSET, &packet_type, sizeof(packet_type));
}

static inline void tbt_xd_packet_get_payload(const uint8_t *packet, void *buffer, size_t offset, size_t bytes)
{
	ASSERT(offset + bytes >= offset);
	ASSERT(offset < TBT_XD_REQUEST_MAX_PAYLOAD);
	ASSERT(offset + bytes <= TBT_XD_REQUEST_MAX_PAYLOAD);

	memcpy(buffer, packet + TBT_XD_REQUEST_HEADER_LEN + offset, bytes);
}

static inline void tbt_xd_packet_set_payload(uint8_t *packet, const void *buffer, size_t offset, size_t bytes)
{
	ASSERT(offset + bytes >= offset);
	ASSERT(offset < TBT_XD_REQUEST_MAX_PAYLOAD);
	ASSERT(offset + bytes <= TBT_XD_REQUEST_MAX_PAYLOAD);

	memcpy(packet + TBT_XD_REQUEST_HEADER_LEN + offset, buffer, bytes);
}

static inline uint32_t tbt_xd_packet_get_payload_dword(const uint8_t *packet, size_t offset)
{
	uint32_t dword;
	tbt_xd_packet_get_payload(packet, &dword, offset, sizeof(dword));

	return ntohl(dword);
}

static inline void tbt_xd_packet_set_payload_dword(uint8_t *packet, uint32_t dword, size_t offset)
{
	dword = htonl(dword);
	tbt_xd_packet_set_payload(packet, &dword, offset, sizeof(dword));
}


#endif
