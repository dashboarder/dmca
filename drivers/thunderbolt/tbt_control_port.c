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
#include <drivers/thunderbolt/nhi.h>
#include <list.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/menu.h>
#include <sys/task.h>
#include "tbt_control_port.h"
#include "tbt_cp_crc.h"
#include "tbt_xdomain.h"
#include "tbt_protocol.h"

#define CPDBG_DUMP_PACKET	DEBUG_NEVER
#define CPDBG_INIT		DEBUG_INFO
#define CPDBG_IO		DEBUG_SPEW
#define CPDBG_EVENT		DEBUG_SPEW

struct xd_service {
	struct list_node list;
	// Callback function to handle incoming xdomain requests for this service
	tbt_xd_callback_t request_callback;
	// Callback function to handle incoming xdomain responses for this service
	tbt_xd_callback_t response_callback;
	// Private data passed in when the service was register
	void *priv;
};

struct tbt_cp {
	nhi_t *nhi;
	uint8_t rx_buffer[256];
	uint8_t tx_buffer[256];
	uint8_t pdf_next_seq[TBT_CFG_MAX_PDF + 1];

	bool shutdown;
	struct task *polling_task;

	struct list_node xd_services;

	uint32_t *read_req_buffer;
	uint32_t read_req_header;
	uint16_t read_req_quads;
	bool read_req_pending;
	struct task_event read_event;

	uint32_t write_req_header;
	bool write_req_pending;
	struct task_event write_event;
};

static void tbt_cp_process_xd_request(tbt_cp_t *cp, uint8_t *packet, size_t bytes);
static void tbt_cp_process_xd_response(tbt_cp_t *cp, uint8_t *packet, size_t bytes);
static void tbt_cp_process_read_response(tbt_cp_t *cp, uint8_t *packet, size_t bytes);
static void tbt_cp_process_write_response(tbt_cp_t *cp, uint8_t *packet, size_t bytes);
static void tbt_cp_process_plug_event(tbt_cp_t *cp, uint8_t *packet, size_t bytes);
static void tbt_cp_process_error(tbt_cp_t *cp, uint8_t *packet, size_t bytes);
static int tbt_cp_polling_task(void *arg);

static tbt_cp_t *g_tbt_cp;

void tbt_cp_dump_route_string(uint64_t route_string)
{
	if (route_string & (1ULL << 63))
		printf("CM:");

	printf("%02x", route_string & 0x3F);

	for (int i = 1; i < 7; i++) {
		if (((route_string >> (i * 8)) & 0x3F) == 0)
			break;

		printf(",%02x", (route_string >> (i * 8)) & 0x3F);
	}
}

uint64_t tbt_cp_packet_get_route_string(const uint8_t *packet)
{
	uint64_t route_string = 0;

	for (int i = 0; i < 8; i++)
		route_string |= (uint64_t)packet[i] << ((8 - i - 1) * 8);

	return route_string;
}

void tbt_cp_packet_set_route_string(uint8_t *packet, uint64_t route_string)
{
	for (int i = 0; i < 8; i++)
		packet[i] = route_string >> ((8 - i - 1) * 8);
}

void tbt_cp_dump_xdomain_message(uint8_t *packet, size_t bytes)
{
#if CPDBG_DUMP_PACKET <= DEBUG_LEVEL
	uint64_t route = tbt_cp_packet_get_route_string(packet);
	uint32_t header;
	uint32_t seq;
	uint32_t payload_len;
	char *len_message = "";
	size_t offset;

	memcpy(&header, packet + 8, sizeof(uint32_t));
	header = htonl(header);
	seq = (header >> 27) & 0x3;
	payload_len = header & 0x3f;

	if (payload_len * 4 > bytes - 12)
		len_message = " (INVALID)";

	printf("Route String:  ");
	tbt_cp_dump_route_string(route);
	printf("\n");
	printf("Length:        0x%02x%s\n", payload_len, len_message);
	printf("Sequence Num:  0x%02x\n", seq);
	printf("Payload:\n");

	if (payload_len * 4 > bytes - 12)
		payload_len = (bytes - 12) / 4;

	offset = 0;
	while (offset < payload_len * 4) {
		for (int i = 0; i < 16 && offset < payload_len * 4; i++, offset++)
			printf("%02x ", packet[offset + 12]);
		printf("\n");
	}
#endif
}

void tbt_cp_dump_message(uint8_t *packet, size_t bytes)
{
#if CPDBG_DUMP_PACKET <= DEBUG_LEVEL
	uint64_t route = tbt_cp_packet_get_route_string(packet);
		uint32_t value;

	printf("Route String: ");
	tbt_cp_dump_route_string(route);
	printf("\nPayload:\n");

	for (size_t i = 0; i < bytes - 12; i += 4) {
		memcpy(&value, packet + 8 + i, 4);
		printf("%08x ", value);
		if (i % 16 == 12)
			printf("\n");
	}
	if ((bytes - 12) % 16 != 0)
		printf("\n");
	memcpy(&value, packet + bytes - 4, 4);
	printf("CRC: %08x\n", value);
#endif
}

tbt_cp_t *tbt_cp_create(nhi_t *nhi)
{
	tbt_cp_t *cp;

	cp = calloc(sizeof(*cp), 1);

	dprintf(CPDBG_INIT, "tbt_cp: initializing\n");

	cp->nhi = nhi;
	cp->xd_services = LIST_INITIAL_VALUE(cp->xd_services);

	// Control messages are sent/received on ring 0
	// 16 buffers should be plenty. The packets are 256 bytes each
	// These functions will panic if someone already took the ring
	nhi_init_tx_ring(nhi, 0, 16, 256);
	nhi_init_rx_ring(nhi, 0, 16, 256);

	event_init(&cp->read_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	event_init(&cp->write_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

	// start a thread to handle packets for the control port
	cp->polling_task = task_create("tbt_polling", tbt_cp_polling_task, cp, 0);
	task_start(cp->polling_task);

	// Pointer used for debug command access to the control port, assumes only 1 exists
	g_tbt_cp = cp;

	return cp;
}

void tbt_cp_quiesce_and_free(tbt_cp_t *cp)
{
	cp->shutdown = true;

	task_wait_on(cp->polling_task);

	task_destroy(cp->polling_task);
	cp->polling_task = NULL;

	nhi_disable_tx_ring(cp->nhi, 0);
	nhi_disable_rx_ring(cp->nhi, 0);

	free(cp);
}

void *tbt_cp_register_xd_service(tbt_cp_t *cp, tbt_xd_callback_t req_cb, tbt_xd_callback_t resp_cb, void *priv)
{
	struct xd_service *new_service;

	ASSERT(req_cb != NULL);
	ASSERT(resp_cb != NULL);

	new_service = calloc(sizeof(*new_service), 1);
	new_service->request_callback = req_cb;
	new_service->response_callback = resp_cb;
	new_service->priv = priv;

	list_add_tail(&cp->xd_services, &new_service->list);

	return (void *)&new_service->list;
}

void tbt_cp_unregister_xd_service(tbt_cp_t *cp, void *handle)
{
	struct list_node *item = (struct list_node *)handle;
	struct xd_service *service;

	if (!list_in_list(item))
		panic("unregistering nonregistered xdomain service");

	service = containerof(item, struct xd_service, list);

	list_delete(item);
	free(service);
}


bool tbt_cp_packet_available(tbt_cp_t *cp)
{
	ASSERT(cp != NULL);

	return nhi_rx_buffer_available(cp->nhi, 0);
}

void tbt_cp_process_packet(tbt_cp_t *cp)
{
	uint8_t eof_pdf;
	int32_t bytes;
	uint32_t packet_crc;
	uint32_t calc_crc;

	bytes = nhi_rx_buffer(cp->nhi, 0, cp->rx_buffer, sizeof(cp->rx_buffer), NULL, &eof_pdf);

	if (bytes < 0) {
		dprintf(DEBUG_CRITICAL, "tbt_cp: error %d receiving packet\n", bytes);
		return;
	}

	// Packet must be big enough for header + CRC
	if (bytes < TBT_CFG_HEADER_LEN + 4) {
		dprintf(DEBUG_CRITICAL, "tbt_cp: runt packet\n");
		return;
	}

	calc_crc = tbt_cp_crc(cp->rx_buffer, bytes - 4);
	memcpy(&packet_crc, cp->rx_buffer + bytes - 4, 4);
	packet_crc = ntohl(packet_crc);
	if (calc_crc != packet_crc) {
		dprintf(DEBUG_CRITICAL, "tbt_cp: CRC mismatch %x v %x\n", calc_crc, packet_crc);
		return;
	}

	switch (eof_pdf) {
		case TBT_CFG_READ_PDF:
			tbt_cp_dump_message(cp->rx_buffer, bytes);
			tbt_cp_process_read_response(cp, cp->rx_buffer, bytes - 4);
			break;
		case TBT_CFG_WRITE_PDF:
			tbt_cp_dump_message(cp->rx_buffer, bytes);
			tbt_cp_process_write_response(cp, cp->rx_buffer, bytes - 4);
			break;
		case TBT_CFG_PLUG_EVENT_PDF:
			tbt_cp_process_plug_event(cp, cp->rx_buffer, bytes - 4);
			break;
		case TBT_CFG_ERROR_PDF:
			tbt_cp_dump_message(cp->rx_buffer, bytes);
			tbt_cp_process_error(cp, cp->rx_buffer, bytes - 4);
			break;
		case TBT_CFG_XDOMAIN_REQUEST_PDF:
			tbt_cp_dump_xdomain_message(cp->rx_buffer, bytes);
			tbt_cp_process_xd_request(cp, cp->rx_buffer, bytes - 4);
			break;
		case TBT_CFG_XDOMAIN_RESPONSE_PDF:
			tbt_cp_dump_xdomain_message(cp->rx_buffer, bytes);
			tbt_cp_process_xd_response(cp, cp->rx_buffer, bytes - 4);
			break;
		default:
			dprintf(DEBUG_INFO, "tbt_cp: got unexpected PDF 0x%02x\n", eof_pdf);
			tbt_cp_dump_message(cp->rx_buffer, bytes);
			break;
	}
}

// Control port packets have a 2-bit sequence number. The OS stack increments the
// sequence number per PDF, so we'll do the same. This function gets the next
// sequence number and increments a 2-bit counter to provide for the subsequent value
uint8_t tbt_cp_next_pdf_seq(tbt_cp_t *cp, uint8_t pdf)
{
	uint8_t next;

	ASSERT(cp != NULL);
	ASSERT(pdf <= TBT_CFG_MAX_PDF);

	next = cp->pdf_next_seq[pdf];
	cp->pdf_next_seq[pdf] = (next + 1) & 3;

	return next;
}

// Queues the given packet to be sent
bool tbt_cp_send(tbt_cp_t *cp, uint8_t pdf, uint8_t *packet, size_t len)
{
	uint32_t crc;
	nhi_sgl_t sgl[2];

	ASSERT(len <= TBT_CFG_MAX_HEADER_AND_PAYLOAD);

	crc = htonl(tbt_cp_crc(packet, len));

	sgl[0].buffer = packet;
	sgl[0].bytes = len;
	sgl[0].next = &sgl[1];
	sgl[1].buffer = &crc;
	sgl[1].bytes = sizeof(crc);
	sgl[1].next = NULL;

	return nhi_send_sgl(cp->nhi, 0, sgl, pdf, pdf) == NHI_STATUS_OK;
}

void tbt_cp_send_acknowledgement(tbt_cp_t *cp, uint64_t route_string, uint8_t port, uint8_t code)
{
	uint32_t packet[3];

	packet[0] = route_string >> 32;
	packet[1] = route_string;
	packet[2] = htonl((port << 8) | code);

	tbt_cp_send(cp, TBT_CFG_ERROR_PDF, (uint8_t *)packet, sizeof(packet));
}

bool tbt_cp_read(tbt_cp_t *cp, uint64_t topology_id, uint8_t space, uint8_t port, uint16_t index, uint32_t *buffer, uint16_t quadlets)
{
	bool ret;
	uint32_t request[3];
	uint32_t header;
	uint8_t pdf = TBT_CFG_READ_PDF;
	uint8_t seq = tbt_cp_next_pdf_seq(cp, pdf);

	ASSERT(quadlets < 60);

	dprintf(CPDBG_IO, "tbt_cp: reading %d quads from %08llx %x:%02x:%04x\n", quadlets, topology_id, space, port, index);

	request[0] = htonl(topology_id >> 32);
	request[1] = htonl(topology_id & 0xffffffff);
	header = 0;
	header |= (index & 0x1fff) << 0;	
	header |= (quadlets & 0x3f) << 13;	
	header |= (port & 0x3f) << 19;	
	header |= (space & 0x3) << 25;	
	header |= seq << 27;
	request[2] = htonl(header);

	if (!tbt_cp_send(cp, pdf, (uint8_t *)request, sizeof(request)))
		return false;

	// If we ever switch to interrupts, this will need to be reconsidered
	cp->read_req_header = header;
	cp->read_req_buffer = buffer;
	cp->read_req_quads = quadlets;
	cp->read_req_pending = true;

	event_wait_timeout(&cp->read_event, 200000);

	// if the read request is no longer pending, then tbt_cp_process_read_response saw a good response
	ret = !cp->read_req_pending;
	cp->read_req_pending = false;

	if (ret) {
		for (unsigned i = 0; i < quadlets; i++)
			buffer[i] = ntohl(buffer[i]);
	}

	return ret;
}

bool tbt_cp_write(tbt_cp_t *cp, uint64_t topology_id, uint8_t space, uint8_t port, uint16_t index, const uint32_t *buffer, uint16_t quadlets)
{
	uint8_t pdf = TBT_CFG_WRITE_PDF;
	uint8_t seq = tbt_cp_next_pdf_seq(cp, pdf);
	uint32_t header;
	uint32_t topo_high;
	uint32_t topo_low;
	uint32_t swapped;
	bool ret;

	dprintf(CPDBG_IO, "tbt_cp: write %d quads to %016llx %x:%02x:%04x\n", quadlets, topology_id, space, port, index);

	ASSERT(quadlets < 60);

	topo_high = htonl(topology_id >> 32);
	topo_low = htonl(topology_id & 0xffffffff);
	header = 0;
	header |= (index & 0x1fff) << 0;	
	header |= (quadlets & 0x3f) << 13;	
	header |= (port & 0x3f) << 19;	
	header |= (space & 0x3) << 25;	
	header |= seq << 27;
	header = htonl(header);

	memcpy(cp->tx_buffer + 0, &topo_high, 4);
	memcpy(cp->tx_buffer + 4, &topo_low, 4);
	memcpy(cp->tx_buffer + 8, &header, 4);
	for(uint16_t i = 0; i < quadlets; i++) {
		swapped = htonl(buffer[i]);
		memcpy(cp->tx_buffer + 12 + 4 * i, &swapped, 4);
	}

	if (!tbt_cp_send(cp, pdf, cp->tx_buffer, 12 + 4 * quadlets))
		return false;

	// If we ever use interrupts, this will need to be reordered compared to the send call
	cp->write_req_header = ntohl(header);
	cp->write_req_pending = true;

	event_wait_timeout(&cp->write_event, 200000);

	// If write_req_pending is false, then tbt_cp_process_write_response saw a good response
	ret = !cp->write_req_pending;
	cp->write_req_pending = false;

	return true;
}

static void tbt_cp_process_read_response(tbt_cp_t *cp, uint8_t *packet, size_t bytes)
{
	uint16_t len;
	uint32_t header;

	if (!cp->read_req_pending) {
		dprintf(DEBUG_CRITICAL, "tbt_cp: spurious read request response\n");
		return;
	}

	memcpy(&header, packet + 8, 4);
	header = ntohl(header);
	len = (header >> 13) & 0x3f;

	dprintf(CPDBG_IO, "tbt_cp: read response header=0x%08x bytes=%u\n", header, (unsigned)bytes);

	// The far end is supposed to just copy the entire header back to us. They seem
	// to put garbage in the port number when we aren't reading port space, so
	// ignore it in the comparison
	if ((header & 0x1E07FFFF) != (cp->read_req_header & 0x1E07FFFF)) {
		dprintf(DEBUG_CRITICAL, "tbt_cp: read request header mismatch, got %x expected %x mismatch %x\n", header, cp->read_req_header, header ^ cp->read_req_header);
		return;
	}

	// Belt and suspenders check on the length, which is included in the header
	if (len != cp->read_req_quads) {
		dprintf(DEBUG_CRITICAL, "tbt_cp: read request len mismatch, got %u expected %u\n", len, cp->read_req_quads);
		return;
	}

	if (bytes != (len * 4) + 12) {
		dprintf(DEBUG_CRITICAL, "tbt_cp: read request payload len mismatch, got %u expected %u\n", bytes, (len * 4) + 16);
		return;
	}

	memcpy(cp->read_req_buffer, packet + 12, cp->read_req_quads * 4);

	cp->read_req_pending = false;

	event_signal(&cp->read_event);
}

static void tbt_cp_process_write_response(tbt_cp_t *cp, uint8_t *packet, size_t bytes)
{
	uint32_t header;

	if (!cp->write_req_pending) {
		dprintf(DEBUG_CRITICAL, "tbt_cp: spurious write request response\n");
		return;
	}

	memcpy(&header, packet + 8, 4);
	header = ntohl(header);

	dprintf(CPDBG_IO, "tbt_cp: write response header=0x%08x bytes=%u\n", header, (unsigned)bytes);

	// The far end is supposed to just copy the entire header back to us. We'll
	// mask off the reserved bits, but otherwise everything had better match
	if ((header & 0x1fffffff) != cp->write_req_header) {
		dprintf(DEBUG_CRITICAL, "tbt_cp: write request header mismatch, got %x expected %x\n", header, cp->read_req_header);
		return;
	}
	cp->write_req_pending = false;

	event_signal(&cp->write_event);
}

static void tbt_cp_process_plug_event(tbt_cp_t *cp, uint8_t *packet, size_t bytes)
{
	uint64_t route_string;
	uint32_t details;
	uint8_t port;

	// Send the acknowledgement back to the sender, clearing the CM bit
	route_string = tbt_cp_packet_get_route_string(packet);
	route_string &= ~(1ULL << 63);

	memcpy(&details, packet + 8, sizeof(details));
	details = ntohl(details);

	port = details & 0x3f;

	dprintf(CPDBG_EVENT, "tbt_cp: Got plug event from %016llx for port %x\n", route_string, port);

	tbt_cp_send_acknowledgement(cp, route_string, port, 7);
}

static void tbt_cp_process_error(tbt_cp_t *cp, uint8_t *packet, size_t bytes)
{
	uint64_t route_string;

	// XXX
	dprintf(DEBUG_INFO, "Got an error packet. We should probably handle these?\n");

	route_string = tbt_cp_packet_get_route_string(packet);
}

static void tbt_cp_process_xd_request(tbt_cp_t *cp, uint8_t *packet, size_t bytes)
{
	struct xd_service *service;

	list_for_every_entry(&cp->xd_services, service, struct xd_service, list) {
		service->request_callback(cp, packet, bytes, service->priv);
	}
}

static void tbt_cp_process_xd_response(tbt_cp_t *cp, uint8_t *packet, size_t bytes)
{
	struct xd_service *service;

	list_for_every_entry(&cp->xd_services, service, struct xd_service, list) {
		service->response_callback(cp, packet, bytes, service->priv);
	}
}

static int tbt_cp_polling_task(void *arg)
{
	tbt_cp_t *cp = (tbt_cp_t *)arg;

	dprintf(CPDBG_INIT, "tbt_cp: polling task started\n");

	while (!cp->shutdown) {
		while (tbt_cp_packet_available(cp)) {
			tbt_cp_process_packet(cp);
		}

		task_yield();
	}

	dprintf(CPDBG_INIT, "tbt_cp: polling task finished\n");

	return 0;
}

#if DEBUG_BUILD
static int do_tbtcp_debug_cmd(int argc, struct cmd_arg *args)
{
	int result = -1;
	const char *cmd = NULL;

	// Nothing to do if there's no control port
	if (g_tbt_cp == NULL)
		return -1;
       
	if (argc >= 2)
		cmd = args[1].str;

	if (cmd != NULL) {
		if (strcmp(cmd, "read") == 0 && argc >= 6) {
			uint64_t topo_id = args[2].n;
			uint8_t space = args[3].n;
			uint8_t port = args[4].n;
			uint16_t index = args[5].n;
			uint32_t buffer;

			if (tbt_cp_read(g_tbt_cp, topo_id, space, port, index, &buffer, 1)) {
				printf("0x%08x\n", buffer);
				result = 0;
			} else {
				printf("read failure\n");
			}
		} else if (strcmp(cmd, "write") == 0 && argc >= 7) {
			uint64_t topo_id = args[2].n;
			uint8_t space = args[3].n;
			uint8_t port = args[4].n;
			uint16_t index = args[5].n;
			uint32_t buffer = args[6].n;

			if (tbt_cp_write(g_tbt_cp, topo_id, space, port, index, &buffer, 1)) {
				result = 0;
			} else {
				printf("write failure\n");
			}
		}
	}

	return result;
}
#endif

MENU_COMMAND_DEBUG(tbtcp, do_tbtcp_debug_cmd, "TBT control port debug commands", NULL);
