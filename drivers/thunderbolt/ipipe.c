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
#include <drivers/ipipe.h>
#include <lib/cbuf.h>
#include <platform.h>
#include <platform/memmap.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <stdint.h>
#include "ipipe_protocol.h"

#define IPIPE_FILE_RX_INDEX		(1)
#define IPIPE_CONSOLE_INDEX		(2)
#define IPIPE_FILE_TX_INDEX		(3)

#define IPIPE_HEADER_LEN		(4)

#define IDBG_PROTO_ERR			(DEBUG_CRITICAL)

struct ipipe {
	uint32_t max_packet_size;
	ipipe_tx_callback_t *tx_callback;
	void *priv;

	uint8_t *tx_buffer;

	bool shutdown;

	bool enabled;

	// Console stuff
	struct cbuf *console_output_cbuf;
	struct task_event console_out_event;
	struct task_event console_out_done_event;
	struct task *console_task;

	// File get (including "DFU" mode) stuff
	bool file_rx_active;
	uint32_t file_rx_len;
	uint8_t *file_rx_buffer;
	uint32_t file_rx_buffer_len;
	struct task_event file_rx_event;
#if !WITH_TBT_MODE_DFU
	struct callout file_rx_callout;
#endif
};

static int ipipe_console_task(void *arg);
static void ipipe_handle_console_packet(ipipe_t *ipipe, const uint8_t *packet, uint32_t bytes);

static void ipipe_handle_file_rx_packet(ipipe_t *ipipe, const uint8_t *packet, uint32_t bytes);
static void ipipe_send_file_rx_ack(ipipe_t *ipipe, uint32_t offset);
static void ipipe_send_file_rx_error(ipipe_t *ipipe, uint32_t offset, uint32_t error);
static void ipipe_file_rx_failed(ipipe_t *ipipe);
static void ipipe_handle_file_rx_callout(struct callout *co, void *arg);

static void ipipe_handle_file_tx_packet(ipipe_t *ipipe, const uint8_t *packet, uint32_t bytes);

ipipe_t *g_ipipe;

ipipe_t *ipipe_init(uint32_t max_packet_size, ipipe_tx_callback_t *tx_callback, void *priv)
{
	ipipe_t *ipipe;

	ASSERT(max_packet_size > 64);

	ipipe = calloc(sizeof(*ipipe), 1);
	ipipe->max_packet_size = max_packet_size;
	ipipe->tx_callback = tx_callback;
	ipipe->priv = priv;

	ipipe->tx_buffer = malloc(max_packet_size);

#if WITH_TBT_MODE_RECOVERY
	ipipe->console_output_cbuf = cbuf_create(4096, NULL);
	event_init(&ipipe->console_out_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	event_init(&ipipe->console_out_done_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	ipipe->console_task = task_create("ipipe_console", ipipe_console_task, ipipe, 0);
	task_start(ipipe->console_task);
#endif

	event_init(&ipipe->file_rx_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

	g_ipipe = ipipe;

	return ipipe;
}

void ipipe_quiesce_and_free(ipipe_t *ipipe)
{
	if (g_ipipe == ipipe)
		g_ipipe = NULL;

#if WITH_TBT_MODE_RECOVERY
	ipipe->shutdown = true;
	event_signal(&ipipe->console_out_event);
	task_wait_on(ipipe->console_task);
	task_destroy(ipipe->console_task);
#endif

	free(ipipe->tx_buffer);
	free(ipipe);
}

static void ipipe_packet_set_header(uint8_t *packet, uint16_t pipe_idx, uint16_t bytes)
{
	packet[0] = pipe_idx & 0xff;
	packet[1] = (pipe_idx >> 8) & 0xff;
	packet[2] = (bytes & 0xff);
	packet[3] = (bytes >> 8) & 0xff;
}

static void ipipe_send(ipipe_t *ipipe, uint8_t *packet, uint32_t bytes)
{
	ipipe->tx_callback(ipipe->priv, packet, bytes);
}

// Process a data packet. Packets are expected to have the following format.
// Invalid packets are simply dropped.
// Bytes 0-1: Pipe number
// Bytes 2-3: Payload size (bytes)
void ipipe_handle_packet(ipipe_t *ipipe, const uint8_t *packet, uint32_t bytes)
{
	uint16_t pipe_idx;
	uint16_t ipipe_bytes;
	const uint8_t *payload;

	// Packet needs to be big enough to hold the iPipe header
	if (bytes < IPIPE_HEADER_LEN) {
		dprintf(IDBG_PROTO_ERR, "packet length %u too short for header\n", bytes);
		return;
	}

	pipe_idx = packet[0] | (packet[1] << 8);
	ipipe_bytes = packet[2] | (packet[3] << 8);

	// And packet needs to be big enough for header and the payload size listed in the header
	if (ipipe_bytes + IPIPE_HEADER_LEN > bytes) {
		dprintf(IDBG_PROTO_ERR, "packet length %u too short for payload %u\n", bytes, ipipe_bytes);
		return;
	}

	payload = packet + IPIPE_HEADER_LEN;

	if (pipe_idx == IPIPE_CONSOLE_INDEX) {
		ipipe_handle_console_packet(ipipe, payload, ipipe_bytes);

	} else if (pipe_idx == IPIPE_FILE_RX_INDEX) { 
		ipipe_handle_file_rx_packet(ipipe, payload, ipipe_bytes);

	} else if (pipe_idx == IPIPE_FILE_TX_INDEX) {
		ipipe_handle_file_tx_packet(ipipe, payload, ipipe_bytes);
	}
}

//
// iPipe file RX
//

void ipipe_start_file_rx(ipipe_t *ipipe, void *load_address, uint32_t length)
{
	if (!security_allow_memory(load_address, length)) {
		if (ipipe->file_rx_active) {
			ipipe->file_rx_active = false;
			event_signal(&ipipe->file_rx_event);
		}
		ipipe->file_rx_len = 0;
		return;
	}

	ipipe->file_rx_buffer = load_address;
	ipipe->file_rx_buffer_len = length;
	ipipe->file_rx_len = 0;
	ipipe->file_rx_active = true;
}

int thunderboot_get_dfu_image(void *load_address, int load_length)
{
	int status = -1;

	ASSERT(load_length > 0);
	ASSERT(load_address != NULL);

	ipipe_t *ipipe = g_ipipe;
	if (ipipe == NULL)
		goto exit;

	ipipe_start_file_rx(ipipe, load_address, load_length);
	if (!ipipe->file_rx_active)
		return -1;

	event_wait(&ipipe->file_rx_event);

	if (ipipe->file_rx_len == 0)
		status = -1;
	else
		status = ipipe->file_rx_len;

exit:
	return status;
}

static void ipipe_handle_file_rx_packet(ipipe_t *ipipe, const uint8_t *packet, uint32_t bytes)
{
	uint32_t offset;
	uint32_t payload_size;

	// packet needs to be big enough for the header
	if (bytes < 4) {
		ipipe_send_file_rx_error(ipipe, 0, IPIPE_ERR_FRAMING);
		return;
	}

	offset = packet[0] | (packet[1] << 8) | (packet[2] << 16) | (packet[3] << 24);
	payload_size = bytes - 4;

	// in non-DFU mode, hosts can start a transfer with a reset packet (offset 0 and no payload)
	// in order to reset back to the default load address.
	// in DFU-mode, this is just a no-op
	if (offset == 0 && payload_size == 0) {
#if !WITH_TBT_MODE_DFU
		ipipe_start_file_rx(ipipe, (void *)DEFAULT_LOAD_ADDRESS, DEFAULT_RAMDISK_SIZE);
#endif
		return;
	}

	if (!ipipe->file_rx_active) {
		ipipe_send_file_rx_error(ipipe, offset, IPIPE_ERR_TBD);
		return;
	}

	// a payload size of 0 with a non-zero offset means that the transfer is done
	if (payload_size == 0) {
		ipipe->file_rx_active = false;
		if (offset != ipipe->file_rx_len) {
			ipipe_send_file_rx_error(ipipe, offset, IPIPE_ERR_TBD);
			ipipe->file_rx_len = 0;
		} else {
			ipipe_send_file_rx_ack(ipipe, ipipe->file_rx_len);
		}
		event_signal(&ipipe->file_rx_event);

#if !WITH_TBT_MODE_DFU
		callout_dequeue(&ipipe->file_rx_callout);
#endif
	} else {
		if (offset > ipipe->file_rx_buffer_len ||
		    offset + payload_size < offset ||
		    offset + payload_size > ipipe->file_rx_buffer_len) {
			ipipe_send_file_rx_error(ipipe, offset, IPIPE_ERR_TBD);
			ipipe_file_rx_failed(ipipe);
			return;
		}

		if (!security_allow_memory((uint8_t *)ipipe->file_rx_buffer + offset, payload_size)) {
			ipipe_send_file_rx_error(ipipe, offset, IPIPE_ERR_TBD);
			ipipe_file_rx_failed(ipipe);
			return;
		}

		memcpy((uint8_t *)ipipe->file_rx_buffer + offset, packet + 4, payload_size);

		if (offset + payload_size > ipipe->file_rx_len)
			ipipe->file_rx_len = offset + payload_size;

#if !WITH_TBT_MODE_DFU
		callout_reset(&ipipe->file_rx_callout, 0);
#endif
	}

	ipipe_send_file_rx_ack(ipipe, offset);
}

static void ipipe_file_rx_failed(ipipe_t *ipipe)
{
	ipipe->file_rx_active = false;
	ipipe_send_file_rx_error(ipipe, -1, IPIPE_ERR_TBD);
	ipipe->file_rx_len = 0;

	dprintf(DEBUG_CRITICAL, "transfer failed\n");

	event_signal(&ipipe->file_rx_event);
#if !WITH_TBT_MODE_DFU
	callout_dequeue(&ipipe->file_rx_callout);
#endif
}

static void ipipe_send_file_rx_ack(ipipe_t *ipipe, uint32_t offset)
{
	uint8_t *packet = ipipe->tx_buffer;

	ipipe_packet_set_header(packet, IPIPE_FILE_RX_INDEX, 8);

	packet[IPIPE_HEADER_LEN + 0] = offset & 0xff;
	packet[IPIPE_HEADER_LEN + 1] = (offset >> 8) & 0xff;
	packet[IPIPE_HEADER_LEN + 2] = (offset >> 16) & 0xff;
	packet[IPIPE_HEADER_LEN + 3] = (offset >> 24) & 0xff;

	packet[IPIPE_HEADER_LEN + 4] = 0;
	packet[IPIPE_HEADER_LEN + 5] = 0;
	packet[IPIPE_HEADER_LEN + 6] = 0;
	packet[IPIPE_HEADER_LEN + 7] = 0;

	ipipe_send(ipipe, packet, IPIPE_HEADER_LEN + 8);
}

static void ipipe_send_file_rx_error(ipipe_t *ipipe, uint32_t offset, uint32_t error)
{
	uint8_t *packet = ipipe->tx_buffer;

	ipipe_packet_set_header(packet, IPIPE_FILE_RX_INDEX, 8);

	packet[IPIPE_HEADER_LEN + 0] = offset & 0xff;
	packet[IPIPE_HEADER_LEN + 1] = (offset >> 8) & 0xff;
	packet[IPIPE_HEADER_LEN + 2] = (offset >> 16) & 0xff;
	packet[IPIPE_HEADER_LEN + 3] = (offset >> 24) & 0xff;

	packet[IPIPE_HEADER_LEN + 4] = error & 0xff;
	packet[IPIPE_HEADER_LEN + 5] = (error >> 8) & 0xff;
	packet[IPIPE_HEADER_LEN + 6] = (error >> 16) & 0xff;
	packet[IPIPE_HEADER_LEN + 7] = (error >> 24) & 0xff;

	ipipe_send(ipipe, packet, IPIPE_HEADER_LEN + 8);
}

//
// iPipe file TX
//

static void ipipe_handle_file_tx_packet(ipipe_t *ipipe, const uint8_t *packet, uint32_t bytes)
{
	// XXX: todo
}

//
// iPipe console
//

static void ipipe_handle_console_packet(ipipe_t *ipipe, const uint8_t *packet, uint32_t bytes)
{
	for(unsigned i = 0; i < bytes; i++)
		debug_pushchar(packet[i]);
}

static int32_t ipipe_console_flush_write_cbuf(ipipe_t *ipipe)
{
	uint8_t *packet = ipipe->tx_buffer;
	uint32_t bytes = 0;

	do {
		char c;
		if (cbuf_read_char(ipipe->console_output_cbuf, &c) == 0)
			break;
		packet[IPIPE_HEADER_LEN + bytes] = c;

		bytes++;
	} while (bytes + IPIPE_HEADER_LEN < ipipe->max_packet_size);

	if (bytes == 0)
		return 0;

	ipipe_packet_set_header(packet, IPIPE_CONSOLE_INDEX, bytes);

	ipipe_send(ipipe, packet, bytes + IPIPE_HEADER_LEN);

	return bytes;
}

static int ipipe_console_task(void *arg)
{
	ipipe_t *ipipe = (ipipe_t *)arg;

	while(!ipipe->shutdown) {
		if (ipipe_console_flush_write_cbuf(ipipe) == 0)
			event_wait(&ipipe->console_out_event);
		else
			event_signal(&ipipe->console_out_done_event);
	}

	return 0;
}

void thunderboot_putchar(int c)
{
	ipipe_t *ipipe = g_ipipe;

	if (ipipe == NULL)
		return;

	while (cbuf_write_char(ipipe->console_output_cbuf, c) == 0)
		event_wait(&ipipe->console_out_done_event);

	event_signal(&ipipe->console_out_event);
}

int thunderboot_serial_send_cmd_string(uint8_t *buffer, uint32_t len)
{
	// XXX: this isn't robust to losing the connection mid-command
	for (unsigned i = 0; i < len; i++)
		thunderboot_putchar(buffer[i]);

	return 0;
}

static int do_ipipe_puts(int argc, struct cmd_arg *argv)
{
	if (g_ipipe == NULL) {
		printf("iPipe not initialized\n");
		return -1;
	}

	for (int i = 1; i < argc; i++) {
		if (i > 1)
			thunderboot_putchar(' ');
		for (int j = 0; argv[i].str[j] != 0; j++)
			thunderboot_putchar(argv[i].str[j]);
	}

	return 0;
}

void thunderboot_transfer_prepare(void *load_address, uint32_t length)
{
	ipipe_t *ipipe = g_ipipe;

	ipipe_start_file_rx(ipipe, load_address, length);

#if !WITH_TBT_MODE_DFU
	// callout to abort the transfer if more than a second elapses without a packet
	callout_enqueue(&ipipe->file_rx_callout, 10000000, ipipe_handle_file_rx_callout, ipipe);
#endif
}

# if !WITH_TBT_MODE_DFU
static void ipipe_handle_file_rx_callout(struct callout *co, void *arg)
{
	ipipe_t *ipipe = (ipipe_t *)arg;

	printf("TBT timeout\n");
	ipipe_file_rx_failed(ipipe);
}
#endif

int thunderboot_transfer_wait(void)
{
	ipipe_t *ipipe = g_ipipe;

	while (ipipe->file_rx_active)
		event_wait(&ipipe->file_rx_event);

	if (ipipe->file_rx_len == 0)
		return -1;
	else
		return ipipe->file_rx_len;

}

MENU_COMMAND_DEBUG(ipipe_puts, do_ipipe_puts, "ipipe puts", NULL);
