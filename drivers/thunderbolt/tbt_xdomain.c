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
#include <debug.h>
#include <list.h>
#include <stdint.h>
#include <stdlib.h>
#include "tbt_control_port.h"
#include "tbt_protocol.h"
#include "tbt_xdomain.h"
#include "tbt_xd_packet.h"
#include "uuid.h"

struct tbt_xd_discovery {
	tbt_cp_t *control_port;
	void *xd_service_handle;


	// The configuration ROM
	const uint32_t *rom;
	// The number of 32-bit dwords in the ROM
	uint16_t rom_dwords;
	// The ROM generation number to send back with ROM read responses
	uint32_t rom_generation;
	// The xdomain discovery service's UUID in Thunderbolt network order
	uint8_t tbt_protocol_uuid[16];
	// The device's UUID in Thunderbolt network order
	uint8_t tbt_device_uuid[16];

	// The route string from the last received xdomain request, used to figure
	// out where to send the ROM generation changed packet when a new boot stage
	// starts
	uint64_t last_route_string;

	// Buffer used to assemble response packets before transmission
	uint8_t tx_buffer[TBT_CFG_MAX_HEADER_AND_PAYLOAD];
};

static void tbt_xd_discovery_process_request(tbt_cp_t *cp, const uint8_t *packet, size_t bytes, void *priv);
static void tbt_xd_discovery_process_response(tbt_cp_t *cp, const uint8_t *packet, size_t bytes, void *priv);
static void tbt_xd_discovery_process_uuid_request(tbt_xd_discovery_t *xdd, const uint8_t *packet, size_t bytes, bool v2);
static void tbt_xd_discovery_process_rom_changed_request(tbt_xd_discovery_t *xdd, const uint8_t *packet, size_t bytes);
static void tbt_xd_discovery_process_rom_read_request(tbt_xd_discovery_t *xdd, const uint8_t *packet, size_t bytes);

static const uuid_t discovery_uuid = { 0xB638D70E, 0x42FF, 0x40BB, 0x97, 0xC2, { 0x90, 0xE2, 0xC0, 0xB2, 0xFF, 0x07 } };

tbt_xd_discovery_t *tbt_xd_discovery_create(tbt_cp_t *cp, const uint32_t *configuration_rom, uint16_t dwords, uint32_t generation)
{
	tbt_xd_discovery_t *xdd;

	xdd = calloc(sizeof(*xdd), 1);

	xdd->control_port = cp;
	xdd->rom = configuration_rom;
	xdd->rom_dwords = dwords;
	xdd->rom_generation = generation;

	uuid_host_to_tbt(&discovery_uuid, xdd->tbt_protocol_uuid);
	uuid_host_to_tbt(uuid_get_device_uuid(), xdd->tbt_device_uuid);

	xdd->xd_service_handle = tbt_cp_register_xd_service(cp, tbt_xd_discovery_process_request, tbt_xd_discovery_process_response, xdd);

	return xdd;
}

void tbt_xd_discovery_quiesce_and_free(tbt_xd_discovery_t *xdd)
{
	tbt_cp_unregister_xd_service(xdd->control_port, xdd->xd_service_handle);
	free(xdd);
}

static void tbt_xd_discovery_process_request(tbt_cp_t *cp, const uint8_t *packet, size_t bytes, void *priv)
{
	tbt_xd_discovery_t *xdd = priv;
	uint32_t packet_type;
	uint64_t route_string;

	// The packet needs to at least be big enough for a protocol UUID and packet type
	if (bytes < TBT_XD_REQUEST_HEADER_LEN) {
		return;
	}

	// Make note of the route string in case we still need to send a ROM changed
	// request. We send this request to the last address that sent us an XD request
	route_string = tbt_cp_packet_get_route_string(packet);
	xdd->last_route_string = route_string & TBT_CFG_ROUTE_STRING_MASK;

	// Now we can check to see if the packet has the discovery protocol's UUID
	if (tbt_xd_packet_compare_protocol_uuid(packet, xdd->tbt_protocol_uuid) != 0)
		return;

	packet_type = tbt_xd_packet_get_type(packet);

	switch (packet_type) {
		case TBT_XD_UUID_REQUEST:
			tbt_xd_discovery_process_uuid_request(xdd, packet, bytes, false);
			break;
		case TBT_XD_ROM_READ_REQUEST:
			tbt_xd_discovery_process_rom_read_request(xdd, packet, bytes);
			break;
		case TBT_XD_ROM_CHANGED_REQUEST:
			tbt_xd_discovery_process_rom_changed_request(xdd, packet, bytes);
			break;
		case TBT_XD_UUID_V2_REQUEST:
			tbt_xd_discovery_process_uuid_request(xdd, packet, bytes, true);
			break;
	}
}

void tbt_xd_discovery_send_rom_changed_request(tbt_xd_discovery_t *xdd, uint64_t route)
{
	uint8_t *request = xdd->tx_buffer;
	uint8_t pdf = TBT_CFG_XDOMAIN_REQUEST_PDF;
	uint8_t seq;
	size_t len;

	memset(request, 0, TBT_CFG_MAX_HEADER_AND_PAYLOAD);
	tbt_cp_packet_set_route_string(request, route);
	tbt_xd_packet_set_protocol_uuid(request, xdd->tbt_protocol_uuid);
	tbt_xd_packet_set_type(request, TBT_XD_ROM_CHANGED_REQUEST);

	tbt_xd_packet_set_payload(request, xdd->tbt_device_uuid, 0, sizeof(xdd->tbt_device_uuid));

	len = TBT_XD_ROM_CHANGED_REQUEST_LEN;

	seq = tbt_cp_next_pdf_seq(xdd->control_port, pdf);
	tbt_xd_packet_set_len(request, len);
	tbt_xd_packet_set_seq(request, seq);

	tbt_cp_send(xdd->control_port, pdf, request, len);
}

static void tbt_xd_discovery_process_response(tbt_cp_t *cp, const uint8_t *packet, size_t bytes, void *priv)
{
	uint32_t packet_type;

	packet_type = tbt_xd_packet_get_type(packet);

	switch (packet_type) {
		case TBT_XD_ROM_CHANGED_RESPONSE:
			break;
		case TBT_XD_ERROR_RESPONSE:
			break;
	}
}

static void tbt_xd_discovery_process_rom_changed_request(tbt_xd_discovery_t *xdd, const uint8_t *packet, size_t bytes)
{
	uint8_t *response = xdd->tx_buffer;
	uint8_t pdf = TBT_CFG_XDOMAIN_RESPONSE_PDF;
	uint8_t seq;
	size_t len;
	uint64_t route;

	route = tbt_cp_packet_get_route_string(packet) & TBT_CFG_ROUTE_STRING_MASK;

	memset(response, 0, TBT_CFG_MAX_HEADER_AND_PAYLOAD);
	tbt_cp_packet_set_route_string(response, route);
	tbt_xd_packet_set_protocol_uuid(response, xdd->tbt_protocol_uuid);
	tbt_xd_packet_set_type(response, TBT_XD_ROM_CHANGED_RESPONSE);

	len = TBT_XD_ROM_CHANGED_RESPONSE_LEN;

	seq = tbt_cp_next_pdf_seq(xdd->control_port, pdf);
	tbt_xd_packet_set_len(response, len);
	tbt_xd_packet_set_seq(response, seq);

	tbt_cp_send(xdd->control_port, pdf, response, len);
}

static void tbt_xd_discovery_process_uuid_request(tbt_xd_discovery_t *xdd, const uint8_t *packet, size_t bytes, bool v2)
{
	uint8_t *response = xdd->tx_buffer;
	uint8_t pdf = TBT_CFG_XDOMAIN_RESPONSE_PDF;
	uint8_t seq;
	size_t len;
	uint64_t route;

	route = tbt_cp_packet_get_route_string(packet) & TBT_CFG_ROUTE_STRING_MASK;

	memset(response, 0, TBT_CFG_MAX_HEADER_AND_PAYLOAD);
	tbt_cp_packet_set_route_string(response, route);
	tbt_xd_packet_set_protocol_uuid(response, xdd->tbt_protocol_uuid);
	tbt_xd_packet_set_type(response, TBT_XD_UUID_RESPONSE);

	tbt_xd_packet_set_payload(response, xdd->tbt_device_uuid, 0, sizeof(xdd->tbt_device_uuid));

	len = TBT_XD_UUID_RESPONSE_LEN;

	if (v2) {
		tbt_xd_packet_set_payload_dword(response, route >> 32, sizeof(xdd->tbt_device_uuid));
		tbt_xd_packet_set_payload_dword(response, route, sizeof(xdd->tbt_device_uuid) + 4);
		len += 8;
	}

	seq = tbt_cp_next_pdf_seq(xdd->control_port, pdf);
	tbt_xd_packet_set_len(response, len);
	tbt_xd_packet_set_seq(response, seq);

	tbt_cp_send(xdd->control_port, TBT_CFG_XDOMAIN_RESPONSE_PDF, response, len);
}

static void tbt_xd_discovery_process_rom_read_request(tbt_xd_discovery_t *xdd, const uint8_t *packet, size_t bytes)
{
	uint8_t *response = xdd->tx_buffer;
	uint8_t pdf = TBT_CFG_XDOMAIN_RESPONSE_PDF;
	uint8_t seq;
	size_t len;
	uint64_t route;
	uint8_t host_uuid[16];
	uint32_t offset;
	uint16_t response_dwords;
	uint32_t offset_and_size;

	if (bytes != TBT_XD_ROM_READ_REQUEST_LEN) {
		// XXX: Send error response
		return;
	}

	route = tbt_cp_packet_get_route_string(packet) & TBT_CFG_ROUTE_STRING_MASK;
	offset = tbt_xd_packet_get_payload_dword(packet, 32);

	if (offset >= xdd->rom_dwords) {
		// XXX: Send error response
		return;
	}

	response_dwords = TBT_XD_ROM_READ_RESPONSE_MAX_DATA / 4;
	if (xdd->rom_dwords < offset + response_dwords)
		response_dwords = xdd->rom_dwords - offset;

	memset(response, 0, TBT_CFG_MAX_HEADER_AND_PAYLOAD);
	tbt_cp_packet_set_route_string(response, route);
	tbt_xd_packet_set_protocol_uuid(response, xdd->tbt_protocol_uuid);
	tbt_xd_packet_set_type(response, TBT_XD_ROM_READ_RESPONSE);

	// Grab the host UUID from the packet, we'll parrot it back in our response
	tbt_xd_packet_get_payload(packet, host_uuid, 0, sizeof(host_uuid));

	// Packet starts with our UUID followed by the host's UUID
	tbt_xd_packet_set_payload(response, xdd->tbt_device_uuid, 0, sizeof(xdd->tbt_device_uuid));
	tbt_xd_packet_set_payload(response, host_uuid, 16, sizeof(host_uuid));

	offset_and_size = offset | (xdd->rom_dwords << 16);
	tbt_xd_packet_set_payload_dword(response, offset_and_size, 32);
	tbt_xd_packet_set_payload_dword(response, xdd->rom_generation, 36);

	for (int i = 0; i < response_dwords; i++)
		tbt_xd_packet_set_payload_dword(response, xdd->rom[offset + i], TBT_XD_ROM_READ_RESPONSE_DATA_OFFSET + i * 4);

	len = TBT_XD_RESPONSE_HEADER_LEN + TBT_XD_ROM_READ_RESPONSE_DATA_OFFSET + 4 * response_dwords;
	seq = tbt_cp_next_pdf_seq(xdd->control_port, pdf);
	tbt_xd_packet_set_len(response, len);
	tbt_xd_packet_set_seq(response, seq);

	tbt_cp_send(xdd->control_port, pdf, response, len);
}

