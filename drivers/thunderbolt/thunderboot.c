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
#include <debug.h>
#include <drivers/ipipe.h>
#include <drivers/sha1.h>
#include <drivers/thunderbolt/nhi.h>
#include <drivers/thunderbolt/thunderboot.h>
#include <lib/cbuf.h>
#include <platform.h>
#include <stdint.h>
#include "tbt_control_port.h"
#include "tbt_xdomain.h"
#include "tbt_xd_packet.h"
#include "thunderboot_protocol.h"
#include "uuid.h"

// We expect the host to be reachable via an xdomain link
// at port 3 of the TBT router connected to the SoC
#define DEFAULT_HOST_ROUTE_STRING	(3)

#if WITH_TBT_MODE_DFU
#define PROTOCOL_SETTINGS		(0x00000000)
#else
#define PROTOCOL_SETTINGS		(0x00000001)
#endif

#define TBT_SERVICE_OFFSET		(0x22)

#define XDD_VAL(size)			(0x76000000 | (size)) // 'v' - size must be 1, next word has value
#define XDD_TXT(size)			(0x74000000 | (size)) // 't' - next word has dword offset
#define XDD_DIR(size)			(0x44000000 | (size)) // 'D' - next word has dword offset

uint32_t    TBT_XDP_DEFAULT[] =
{
    0x55584401, // header
    0x00000018, // size of root directory in dwords
    'vend',     'orid',     XDD_VAL(1), 0x00000001,
    'vend',     'orid',     XDD_TXT(3), 0x0000001A,
    'devi',     'ceid',     XDD_VAL(1), 0x00000001,
    'devi',     'ceid',     XDD_TXT(5), 0x0000001D,
    'devi',     'cerv',     XDD_VAL(1), 0x00000100, // >0x80000000 is OS
    'indd',     'evp\0',    XDD_DIR(24), TBT_SERVICE_OFFSET,

    'Appl',     'e In',     'c.\0\0', // vendorid text value
    'Appl',     'e Mo',     'bile',     ' Dev',     'ice\0', // deviceid text value

    0x00000000, 0x00000000, 0x00000000, 0x00000000, // UUID placeholder (comes from hash of values below)
    'prtc',     'id\0\0',   XDD_VAL(1), 0x00000069,
    'prtc',     'vers',     XDD_VAL(1), 0x00000001,
    'prtc',     'revs',     XDD_VAL(1), 0x00000000,
    'prtc',     'stns',     XDD_VAL(1), PROTOCOL_SETTINGS,
    0xffffffff, 0xffff766e, XDD_TXT(64), 0x0000003A, // USB serial number string pointer (0xffffffxx is vendor-specific range)

    0x00000000, 0x00000000, 0x00000000, 0x00000000, // USB serial number string placeholder
    0x00000000, 0x00000000, 0x00000000, 0x00000000, // 
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

#if APPLICATION_SECUREROM
#define ROM_GENERATION		(1)
#elif APPLICATION_IBOOT && (PRODUCT_IBSS || PRODUCT_LLB)
#define ROM_GENERATION		(2)
#elif APPLICATION_IBOOT && (PRODUCT_IBOOT)
#define ROM_GENERATION		(3)
#elif APPLICATION_IBOOT && (PRODUCT_IBEC)
#define ROM_GENERATION		(4)
#else
#error "Unknown Configuration ROM generation"
#endif

struct thunderboot {
	nhi_t *nhi;
	tbt_cp_t *cp;
	tbt_xd_discovery_t *xdd;
	void *xd_service_handle;

	// Buffer for assembling outgoing packets
	uint8_t tx_buffer[256];

	// UUIDs converted to network byte order
	uint8_t tbt_protocol_uuid[16];
	uint8_t tbt_device_uuid[16];

	// handle for iPipe driver (protocol embedded in Thunderboot)
	ipipe_t *ipipe;

	bool host_logged_in;
	// if the host logged in, the route string of the xdomain link it's on
	uint64_t host_route_string;
	// host's UUID in network byte order
	uint8_t host_uuid[16];

	struct callout rom_changed_callout;
};

static uuid_t thunderboot_protocol_uuid =
	{ 0xC7C43652, 0xAE71, 0x4577, 0xB2, 0x49, { 0x07, 0xB9, 0xC6, 0xED, 0x40, 0x52 } };

static thunderboot_t *g_thunderboot;

static void thunderboot_handle_xdomain_response(tbt_cp_t *cp, const uint8_t *packet, size_t bytes, void *priv);
static void thunderboot_handle_xdomain_request(tbt_cp_t *cp, const uint8_t *packet, size_t bytes, void *priv);
static void thunderboot_process_login_request(thunderboot_t *tb, const uint8_t *packet, size_t bytes);
static void thunderboot_process_ping_request(thunderboot_t *tb, const uint8_t *packet, size_t bytes);
static void thunderboot_process_ipipe_request(thunderboot_t *tb, const uint8_t *packet, size_t bytes);
static void thunderboot_send_error_response(thunderboot_t *tb, const uint8_t *packet, uint32_t status);
void thunderboot_ipipe_send(void *priv, const uint8_t *packet, uint32_t bytes);

thunderboot_t * thunderboot_init(pci_device_t bridge, uint32_t dart_id)
{
	thunderboot_t *tb;
	nhi_t *nhi;
	const char *serial_number;
	const char *more_other;
	uint32_t idregs[5];
	uint32_t topology_id[2];
	uint8_t hash[CCSHA1_OUTPUT_SIZE];
	uuid_t service_uuid;

	// Just one thunderboot device allowed right now
	ASSERT(g_thunderboot == NULL);

	dprintf(DEBUG_INFO, "thunderboot: initializing\n");

	nhi = nhi_init(bridge, dart_id);
	if (nhi == NULL) {
		dprintf(DEBUG_CRITICAL, "thunderboot: couldn't initialize NHI\n");
		return NULL;
	}

	tb = calloc(sizeof(*tb), 1);
	tb->nhi = nhi;

	// Set up the control port driver and point it at its NHI
	tb->cp = tbt_cp_create(tb->nhi);

	if (tbt_cp_read(tb->cp, 0, TBT_CFG_SPACE_DEVICE, 0, 0, idregs, 5)) {
		dprintf(DEBUG_INFO, "thunderboot: nhi ID register 0x%x\n", idregs[0]);
	} else {
		dprintf(DEBUG_CRITICAL, "thunderboot: couldn't read device ID\n");
		goto error;
	}

	// The root switch's topology ID is 0. We just need to set it to valid
	// so that we can start getting messages
	topology_id[1] = 0x80000000U;
	topology_id[0] = 0;
	if (!tbt_cp_write(tb->cp, 0, TBT_CFG_SPACE_DEVICE, 0, 2, topology_id, 2)) {
		dprintf(DEBUG_CRITICAL, "thunderboot: couldn't set topology ID\n");
		goto error;
	}

	// Stash some useful UUIDs
	uuid_host_to_tbt(&thunderboot_protocol_uuid, tb->tbt_protocol_uuid);
	uuid_host_to_tbt(uuid_get_device_uuid(), tb->tbt_device_uuid);

	// Need to fill in runtime details in the xdomain discovery dictionary
	serial_number = platform_get_usb_serial_number_string(true);
	more_other = platform_get_usb_more_other_string(true);
	ASSERT(strlen(serial_number) < 128);
	ASSERT(strlen(more_other) < 128);
	strlcpy((char *)TBT_XDP_DEFAULT + sizeof(TBT_XDP_DEFAULT) - 256, serial_number, 128);
	strlcpy((char *)TBT_XDP_DEFAULT + sizeof(TBT_XDP_DEFAULT) - 256 + strlen(serial_number), more_other, 128);
	// strings need to be byte-swapped because... just don't ask
	for (size_t i = 0; i < 256 / sizeof(TBT_XDP_DEFAULT[0]); i++) {
		size_t offset = (sizeof(TBT_XDP_DEFAULT) - 256) / sizeof(TBT_XDP_DEFAULT[0]) + i;
		TBT_XDP_DEFAULT[offset] = htonl(TBT_XDP_DEFAULT[offset]);
	}

	// To generate the thunderboot service's UUID, hash the contents of the service directory
	sha1_calculate(TBT_XDP_DEFAULT + TBT_SERVICE_OFFSET, sizeof(TBT_XDP_DEFAULT) - 4 * TBT_SERVICE_OFFSET, hash);
	uuid_generate_v5(hash, &service_uuid);
	memcpy((uint8_t *)(TBT_XDP_DEFAULT + TBT_SERVICE_OFFSET), &service_uuid, sizeof(service_uuid));

	// route string where we send stuff until we get a login request
	tb->host_route_string = DEFAULT_HOST_ROUTE_STRING;
	
	// Set up the xdomain discovery protocol driver and register is as an xdomain
	// packet handler with the control port driver
	tb->xdd = tbt_xd_discovery_create(tb->cp, TBT_XDP_DEFAULT, sizeof(TBT_XDP_DEFAULT)/4, ROM_GENERATION);
	// Let the host know that they should read our dictionary
	tbt_xd_discovery_send_rom_changed_request(tb->xdd, tb->host_route_string);

	// Register ourselves as an xdomain packet handler
	tb->xd_service_handle = tbt_cp_register_xd_service(tb->cp, thunderboot_handle_xdomain_request, thunderboot_handle_xdomain_response, tb);

	if (g_thunderboot == NULL)
		g_thunderboot = tb;

	// XXX: use the right value here
	tb->ipipe = ipipe_init(THUNDERBOOT_SERIAL_REQUEST_MAX_DATA, &thunderboot_ipipe_send, tb);

	dprintf(DEBUG_INFO, "thunderboot: initialized\n");

	return tb;
error:
	thunderboot_quiesce_and_free(tb);
	return NULL;
}

void thunderboot_quiesce_and_free(thunderboot_t *tb)
{
	if (tb == NULL) {
		tb = g_thunderboot;
		if (tb == NULL)
			return;
	}

	dprintf(DEBUG_INFO, "thunderboot: quiescing\n");

	if (tb == g_thunderboot)
		g_thunderboot = NULL;

	if (tb->xdd != NULL)
		tbt_xd_discovery_quiesce_and_free(tb->xdd);

	if (tb->xd_service_handle != NULL)
		tbt_cp_unregister_xd_service(tb->cp, tb->xd_service_handle);

	if (tb->ipipe != NULL)
		ipipe_quiesce_and_free(tb->ipipe);

	if (tb->cp != NULL)
		tbt_cp_quiesce_and_free(tb->cp);

	if (tb->nhi != NULL)
		nhi_quiesce_and_free(tb->nhi);

	free(tb);
}

static void thunderboot_handle_xdomain_request(tbt_cp_t *cp, const uint8_t *packet, size_t bytes, void *priv)
{
	thunderboot_t *tb = priv;
	uint32_t packet_type;

	// The packet needs to at least be big enough for a protocol UUID and packet type
	if (bytes < THUNDERBOOT_REQUEST_HEADER_LEN)
		return;

	// Now we can check to see if the packet has the discovery protocol's UUID
	if (tbt_xd_packet_compare_protocol_uuid(packet, tb->tbt_protocol_uuid) != 0)
		return;

	packet_type = tbt_xd_packet_get_type(packet);

	if (packet_type != THUNDERBOOT_LOGIN_REQUEST && !tb->host_logged_in) {
		dprintf(DEBUG_INFO, "thunderboot: packet received before login\n");
		thunderboot_send_error_response(tb, packet, THUNDERBOOT_STATUS_NOT_LOGGED_IN);
	}

	switch (packet_type) {
		case THUNDERBOOT_LOGIN_REQUEST:
			thunderboot_process_login_request(tb, packet, bytes);
			break;
		case THUNDERBOOT_PING_REQUEST:
			thunderboot_process_ping_request(tb, packet, bytes);
			break;
		case THUNDERBOOT_IPIPE_REQUEST:
			thunderboot_process_ipipe_request(tb, packet, bytes);
			break;
		default:
			thunderboot_send_error_response(tb, packet, THUNDERBOOT_STATUS_UNKNOWN_TYPE);
	}
}

void thunderboot_handle_xdomain_response(tbt_cp_t *cp, const uint8_t *packet, size_t bytes, void *priv)
{
	// Right now we ignore all xdomain responses, but we need to provide
	// a handler to the xdomain driver
}

/* Copies bytes from the packet into the supplied buffer. The offset is relative
 * to the first byte after the thunderboot packet type field */
static void thunderboot_packet_get_payload(const uint8_t *packet, void *buffer, size_t offset, size_t bytes)
{
	size_t xd_offset = offset + THUNDERBOOT_REQUEST_HEADER_LEN - TBT_XD_REQUEST_HEADER_LEN;
	tbt_xd_packet_get_payload(packet, buffer, xd_offset, bytes);
}

/* Copies bytes into the packet from the supplied buffer. The offset is in bytes relative
 * to the first byte after the thunderboot packet type field */
static void thunderboot_packet_set_payload(uint8_t *packet, const void *buffer, size_t offset, size_t bytes)
{
	size_t xd_offset = offset + THUNDERBOOT_REQUEST_HEADER_LEN - TBT_XD_REQUEST_HEADER_LEN;
	tbt_xd_packet_set_payload(packet, buffer, xd_offset, bytes);
}

/* Fetches a 32-bit dword from the packet, converting it from network to host
 * byte order. The offset is in bytes relative to the first byte after the
 * thunderboot packet type field */
static uint32_t thunderboot_packet_get_payload_dword(const uint8_t *packet, size_t offset)
{
	uint32_t dword;
	thunderboot_packet_get_payload(packet, &dword, offset, 4);
	return dword;
}

/* Sets a 32-bit dword in the packet, converting it to network from host
 * byte order. The offset is in bytes relative to the first byte after the
 * thunderboot packet type field */
static void thunderboot_packet_set_payload_dword(uint8_t *packet, uint32_t dword, size_t offset)
{
	thunderboot_packet_set_payload(packet, &dword, offset, 4);
}

/* Sets a byte in the packet. The offset is in bytes relative to the first byte after the
 * thunderboot packet type field */
static void thunderboot_packet_set_payload_byte(uint8_t *packet, uint8_t byte, size_t offset)
{
	thunderboot_packet_set_payload(packet, &byte, offset, 1);
}

static void thunderboot_packet_get_sender_uuid(const uint8_t *packet, uint8_t *uuid)
{
	tbt_xd_packet_get_payload(packet, uuid, 0, 16);
}

static void thunderboot_packet_set_sender_uuid(uint8_t *packet, uint8_t *uuid)
{
	tbt_xd_packet_set_payload(packet, uuid, 0, 16);
}

static void thunderboot_send_error_response(thunderboot_t *tb, const uint8_t *request, uint32_t status)
{
	uint8_t *packet = tb->tx_buffer;
	uint64_t route;
	uint8_t pdf = TBT_CFG_XDOMAIN_RESPONSE_PDF;
	uint8_t seq;
	size_t len;

	route = tbt_cp_packet_get_route_string(request) & TBT_CFG_ROUTE_STRING_MASK;
	seq = tbt_xd_packet_get_seq(request);

	memset(packet, 0, TBT_CFG_MAX_HEADER_AND_PAYLOAD);

	tbt_cp_packet_set_route_string(packet, route);
	tbt_xd_packet_set_protocol_uuid(packet, tb->tbt_protocol_uuid);
	tbt_xd_packet_set_type(packet, THUNDERBOOT_ERROR_RESPONSE);
	thunderboot_packet_set_sender_uuid(packet, tb->tbt_device_uuid);

	thunderboot_packet_set_payload_dword(packet, status, 0);

	len = THUNDERBOOT_RESPONSE_HEADER_LEN + 4;
	tbt_xd_packet_set_len(packet, len);
	tbt_xd_packet_set_seq(packet, seq);

	tbt_cp_send(tb->cp, pdf, packet, len);
}

static void thunderboot_send_login_response(thunderboot_t *tb, uint64_t route, uint8_t seq, uint32_t status)
{
	uint8_t *packet = tb->tx_buffer;
	uint8_t pdf = TBT_CFG_XDOMAIN_RESPONSE_PDF;
	size_t len;

	memset(packet, 0, TBT_CFG_MAX_HEADER_AND_PAYLOAD);

	tbt_cp_packet_set_route_string(packet, route);
	tbt_xd_packet_set_protocol_uuid(packet, tb->tbt_protocol_uuid);
	tbt_xd_packet_set_type(packet, THUNDERBOOT_LOGIN_RESPONSE);
	thunderboot_packet_set_sender_uuid(packet, tb->tbt_device_uuid);

	thunderboot_packet_set_payload_dword(packet, status, 0);
	
	len = THUNDERBOOT_RESPONSE_HEADER_LEN + 4;
	tbt_xd_packet_set_len(packet, len);
	tbt_xd_packet_set_seq(packet, seq);

	tbt_cp_send(tb->cp, pdf, packet, len);
}

static void thunderboot_process_login_request(thunderboot_t *tb, const uint8_t *packet, size_t bytes)
{
	uint32_t status = THUNDERBOOT_STATUS_OK;
	uint64_t route;
	uint8_t destination_uuid[16];

	route = tbt_cp_packet_get_route_string(packet) & TBT_CFG_ROUTE_STRING_MASK;

	if (bytes < THUNDERBOOT_LOGIN_REQUEST_LEN) {
		status = THUNDERBOOT_STATUS_MALFORMED;
		goto done;
	}

	thunderboot_packet_get_payload(packet, destination_uuid, 0, sizeof(destination_uuid)); 

	if (memcmp(destination_uuid, tb->tbt_device_uuid, sizeof(destination_uuid)) != 0) {
		status = THUNDERBOOT_STATUS_WRONG_UUID;
		goto done;
	}

	tb->host_route_string = route;
	thunderboot_packet_get_sender_uuid(packet, tb->host_uuid);

	if (!tb->host_logged_in) {
		tb->host_logged_in = true;
		callout_dequeue(&tb->rom_changed_callout);
	}

done:
	thunderboot_send_login_response(tb, route, tbt_xd_packet_get_seq(packet), status);
	return;
}

static void thunderboot_send_ping_response(thunderboot_t *tb, uint64_t route, uint8_t seq, uint64_t nonce, uint32_t status)
{
	uint8_t *packet = tb->tx_buffer;
	uint8_t pdf = TBT_CFG_XDOMAIN_RESPONSE_PDF;
	size_t len;

	memset(packet, 0, TBT_CFG_MAX_HEADER_AND_PAYLOAD);

	tbt_cp_packet_set_route_string(packet, route);
	tbt_xd_packet_set_protocol_uuid(packet, tb->tbt_protocol_uuid);
	tbt_xd_packet_set_type(packet, THUNDERBOOT_PING_RESPONSE);
	thunderboot_packet_set_sender_uuid(packet, tb->tbt_device_uuid);

	thunderboot_packet_set_payload_dword(packet, status, 0);
	thunderboot_packet_set_payload(packet, &nonce, 4, 8);
	
	len = THUNDERBOOT_PING_RESPONSE_LEN;
	tbt_xd_packet_set_len(packet, len);
	tbt_xd_packet_set_seq(packet, seq);

	tbt_cp_send(tb->cp, pdf, packet, len);
}

static void thunderboot_process_ping_request(thunderboot_t *tb, const uint8_t *packet, size_t bytes)
{
	uint32_t status = THUNDERBOOT_STATUS_OK;
	uint64_t route;
	uint64_t nonce = 0;

	route = tbt_cp_packet_get_route_string(packet) & TBT_CFG_ROUTE_STRING_MASK;

	if (bytes < THUNDERBOOT_PING_REQUEST_LEN) {
		status = THUNDERBOOT_STATUS_MALFORMED;
		goto done;
	}

	thunderboot_packet_get_payload(packet, &nonce, 0, sizeof(nonce)); 

done:
	thunderboot_send_ping_response(tb, route, tbt_xd_packet_get_seq(packet), nonce, status);
	return;
}

static void thunderboot_send_ipipe_response(thunderboot_t *tb, uint64_t route, uint8_t seq, uint32_t status)
{
	uint8_t *packet = tb->tx_buffer;
	uint8_t pdf = TBT_CFG_XDOMAIN_RESPONSE_PDF;
	size_t len;

	memset(packet, 0, TBT_CFG_MAX_HEADER_AND_PAYLOAD);

	tbt_cp_packet_set_route_string(packet, route);
	tbt_xd_packet_set_protocol_uuid(packet, tb->tbt_protocol_uuid);
	tbt_xd_packet_set_type(packet, THUNDERBOOT_IPIPE_RESPONSE);
	thunderboot_packet_set_sender_uuid(packet, tb->tbt_device_uuid);

	thunderboot_packet_set_payload_dword(packet, status, 0);
	
	len = THUNDERBOOT_RESPONSE_HEADER_LEN + 4;
	tbt_xd_packet_set_len(packet, len);
	tbt_xd_packet_set_seq(packet, seq);

	tbt_cp_send(tb->cp, pdf, packet, len);
}

static void thunderboot_process_ipipe_request(thunderboot_t *tb, const uint8_t *packet, size_t bytes)
{
	uint64_t route;
	uint32_t payload_size;
	const uint8_t *payload;
	uint32_t status = THUNDERBOOT_STATUS_OK;

	route = tbt_cp_packet_get_route_string(packet) & TBT_CFG_ROUTE_STRING_MASK;

	if (bytes < THUNDERBOOT_IPIPE_REQUEST_HEADER_LEN) {
		status = THUNDERBOOT_STATUS_MALFORMED;
		goto done;
	}

	payload_size = bytes - THUNDERBOOT_IPIPE_REQUEST_HEADER_LEN;
	payload = packet + THUNDERBOOT_IPIPE_REQUEST_HEADER_LEN;

	thunderboot_send_ipipe_response(tb, route, tbt_xd_packet_get_seq(packet), THUNDERBOOT_STATUS_OK);

	ipipe_handle_packet(tb->ipipe, payload, payload_size);

	return;

done:
	thunderboot_send_ipipe_response(tb, route, tbt_xd_packet_get_seq(packet), status);
	return;
}

void thunderboot_ipipe_send(void *priv, const uint8_t *ipipe_packet, uint32_t bytes)
{
	thunderboot_t *tb = (thunderboot_t *)priv;
	uint8_t *packet = tb->tx_buffer;
	uint8_t pdf = TBT_CFG_XDOMAIN_REQUEST_PDF;
	uint8_t seq;
	size_t len;

	// XXX: better to queue up serial stuff and not call the send function
	//      this can be deferred until after the ROM because we don't do serial for the ROM
	// Drop packets until the host has logged in
	if (!tb->host_logged_in)
		return;

	memset(packet, 0, TBT_CFG_MAX_HEADER_AND_PAYLOAD);

	tbt_cp_packet_set_route_string(packet, tb->host_route_string);
	tbt_xd_packet_set_protocol_uuid(packet, tb->tbt_protocol_uuid);
	tbt_xd_packet_set_type(packet, THUNDERBOOT_IPIPE_REQUEST);
	thunderboot_packet_set_sender_uuid(packet, tb->tbt_device_uuid);

	thunderboot_packet_set_payload(packet, ipipe_packet, 0, bytes);

	len = THUNDERBOOT_IPIPE_REQUEST_HEADER_LEN + ((bytes + 3) & ~3);
	seq = tbt_cp_next_pdf_seq(tb->cp, pdf);
	tbt_xd_packet_set_len(packet, len);
	tbt_xd_packet_set_seq(packet, seq);

	tbt_cp_send(tb->cp, pdf, packet, len);
}
