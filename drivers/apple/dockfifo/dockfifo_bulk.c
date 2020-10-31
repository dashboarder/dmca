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
#include <cbuffer.h>
#include <platform.h>
#include <platform/dockfifo_config.h>
#include <platform/int.h>
#include <sys.h>
#include <sys/task.h>

#include "cobs.h"
#include "dockfifo_regs.h"
#include "endian_tools.h"
#include "lib/cksum.h"

// #undef DEBUG_LEVEL
// #define DEBUG_LEVEL DEBUG_SPEW

// Slowest rate of polling write fifo when full.
#define MAX_CHECK_INTERVAL	100000

// Maximum size of frame before stuffing bytes inserted, not including CRC.
#define MAX_UNSTUFFED_FRAME	(DOCKFIFO_BULK_MRU + 4)

// Maximum size of frame after stuffing bytes inserted (growing its size).
// Worst case need 1 extra coding byte per 254 byte segment.
#define MAX_STUFFED_FRAME	(MAX_UNSTUFFED_FRAME + \
				 (MAX_UNSTUFFED_FRAME + 253) / 254)

// Small buffer to help speed up RX transfers.
#define RX_CBUF_SIZE		256

struct PacketBuffer {
	uint8_t stuffed_buf[MAX_STUFFED_FRAME];
	uint8_t unstuffed_buf[MAX_UNSTUFFED_FRAME];
};

static struct task_event dockfifo_bulk_rx_event;

static struct PacketBuffer rx_packet;
static struct PacketBuffer tx_packet;

// When not in sync, we need a framing byte before accepting input.
static bool rx_sync = true;

// Small cbuffer to accelerate reading from the DBGFIFO.
static CBUFFER rx_cbuf;


//=======================
// Local funtions
//=======================

static void dockfifo_bulk_fill_rx_cbuf(void)
{
	size_t min_occupancy = 0;
	size_t bytes = cb_free_space(&rx_cbuf);
	while (bytes > 0) {
                if ((min_occupancy >= 4) && (bytes >= 4)) {
			dprintf(DEBUG_SPEW,
				"dockfifo_bulk_fill_rx_cbuf 4B %d %d\n",
				(int) min_occupancy, (int) bytes);
                        // At least 4 bytes available and 4 bytes to read.
                        uint32_t rdata_4 =
				rDBGFIFO_R_DATA(DOCKFIFO_BULK_READ, 4);
			cb_write_unsafe(&rx_cbuf,
					(const uint8_t *) &rdata_4, 4);
			bytes -= 4;
			min_occupancy -= 4;
		} else {
                        // Read between 1 and 3 bytes, including FIFO status.
                        uint32_t to_read = bytes;
			if (to_read > 3) {
				to_read = 3;
			}
                        uint32_t rdata_n =
				rDBGFIFO_R_DATA(DOCKFIFO_BULK_READ, to_read);
                        // Get FIFO status *before* read took place.
                        uint32_t occupancy = rdata_n & 0x7f;
			dprintf(DEBUG_SPEW,
				"dockfifo_bulk_fill_rx_cbuf occ:%d %d %d\n",
				(int) occupancy,
				(int) min_occupancy,
				(int) bytes);
                        if (occupancy == 0) {
                                // If no bytes were available, break
                                // the loop instead of busy polling.
                                break;
                        }
                        // Between 1 and 3 inclusive bytes were pumped
                        // from the FIFO.
                        uint32_t got_bytes = to_read;
			if (got_bytes > occupancy) {
				got_bytes = occupancy;
			}
			cb_write_unsafe(&rx_cbuf,
					((const uint8_t *) &rdata_n) + 1,
					got_bytes);
                        bytes -= got_bytes;
                        min_occupancy = occupancy - got_bytes;
                }
        }
}

static uint8_t dockfifo_bulk_read_byte(void)
{
	// Grab a byte from the rx cbuffer if possible.
	int c = cb_getc_unsafe(&rx_cbuf);
	if (c >= 0) {
		return (uint8_t) c;
	}
	// Otherwise, try to fill up the cbuffer.
	for (;;) {
		// Fill cbuffer from DBGFIFO.
		dockfifo_bulk_fill_rx_cbuf();
		// Try again.
		c = cb_getc_unsafe(&rx_cbuf);
		if (c >= 0) {
			return (uint8_t) c;
		}
		// Still nothing - wait for interrupt.
		dprintf(DEBUG_SPEW,
			"dbgfifo_bulk_read_byte wait IRQ avail %d\n",
			(int) (rDBGFIFO_R_STAT(DOCKFIFO_BULK_WRITE) & 0xffff));
		unmask_int(dockfifo_configs[DOCKFIFO_BULK_READ].irq);
		event_wait(&dockfifo_bulk_rx_event);
		dprintf(DEBUG_SPEW,
			"dbgfifo_bulk_read_byte got IRQ avail %d\n",
			(int) (rDBGFIFO_R_STAT(DOCKFIFO_BULK_WRITE) & 0xffff));
	}
}

static void dockfifo_bulk_write_byte(uint8_t byte)
{
	// Wait until the FIFO has at least one byte available.
	uint32_t check_interval = 1;
	while ((rDBGFIFO_W_STAT(DOCKFIFO_BULK_WRITE) & 0xffff) == 0) {
		task_sleep(check_interval);
		check_interval *= 2;
		if (check_interval > MAX_CHECK_INTERVAL) {
			check_interval = MAX_CHECK_INTERVAL;
		}
	}
	rDBGFIFO_W_DATA(DOCKFIFO_BULK_WRITE, 1) = byte;
}

static void dockfifo_bulk_write_bytes(const uint8_t *data, size_t count)
{
	while (count > 0) {
		// Wait until the FIFO has available space.
		uint32_t check_interval = 1;
		uint16_t avail;
		for (;;) {
			avail = rDBGFIFO_W_STAT(DOCKFIFO_BULK_WRITE) & 0xffff;
			if (avail > 0) {
				break;
			}
			task_sleep(check_interval);
			check_interval *= 2;
			if (check_interval > MAX_CHECK_INTERVAL) {
				check_interval = MAX_CHECK_INTERVAL;
			}
		}
		// Fill the available space up to the remaining count.
		size_t fill = (avail < count) ? avail : count;
		// Use 32 bit writes as much as possible.
		for (size_t i = 0; i < fill / sizeof(uint32_t); ++i) {
			uint32_t word;
			memcpy(&word, data, sizeof(word));
			data += sizeof(word);
			rDBGFIFO_W_DATA(DOCKFIFO_BULK_WRITE, 4) = word;
		}
		// Write remaining bits out in a single transaction.
		size_t remainder = fill % sizeof(uint32_t);
		if (remainder > 0) {
			uint32_t word = 0;
			memcpy(&word, data, remainder);
			data += remainder;
			rDBGFIFO_W_DATA(DOCKFIFO_BULK_WRITE, remainder) = word;
		}
		count -= fill;
	}
}

static void dockfifo_bulk_interrupt_handler(void *arg)
{
	int32_t which_bulk = (int32_t) arg;
	if (which_bulk != DOCKFIFO_BULK_READ) {
		panic("dockfifo_bulk_interrupt_handler wrong arg: %d",
		      (int) which_bulk);
	}
	// Just mask the interrupt and defer to a task. The hardware
	// FIFO is large and has pacing.
	uint16_t avail = rDBGFIFO_R_STAT(DOCKFIFO_BULK_READ) & 0xffff;
	if (avail > 0) {
		//dprintf(DEBUG_SPEW, "irq with %d data\n", (int) avail);
		mask_int(dockfifo_configs[DOCKFIFO_BULK_READ].irq);
		event_signal(&dockfifo_bulk_rx_event);
	} else {
		dprintf(DEBUG_INFO, "Spurious DBGFIFO bulk irq\n");
	}
}

//=======================
// Global funtions
//=======================

int32_t dockfifo_bulk_read_frame(void *buf, size_t bytes, size_t *received)
{
	*received = 0;

	// Get into sync if necessary.
	while (!rx_sync) {
		uint8_t byte = dockfifo_bulk_read_byte();
		if (byte == 0) {
			dprintf(DEBUG_INFO, "Found sync byte\n");
			rx_sync = true;
		} else {
			dprintf(DEBUG_INFO, "Bulk re-sync, dropping 0x%02x\n",
				byte);
		}
	}

	// Block reading bytes until we have a complete frame.
	size_t pos = 0;
	for (;;) {
		uint8_t byte = dockfifo_bulk_read_byte();
		if (byte == 0) {
			if (pos > 0) {
				// Got non-empty frame.
				dprintf(DEBUG_SPEW, "Got frame, %d bytes\n",
					(int) pos);
				break;
			} else {
				// Allowed to have EOF-SOF combinations.
				dprintf(DEBUG_SPEW, "Ignoring empty frame\n");
			}
		} else if (pos < MAX_STUFFED_FRAME) {
			// Got a data byte, and room to insert it.
			dprintf(DEBUG_SPEW, "Data[%d] = 0x%02x\n",
				(int) pos, byte);
			rx_packet.stuffed_buf[pos] = byte;
			++pos;
		} else {
			// MRU overflow.
			dprintf(DEBUG_INFO, "Bulk overflow 0x%02x\n", byte);
			rx_sync = false;
			break;
		}
	}
	if (!rx_sync) {
		// Bad framing.
		return -1;
	}

	// COBS decode.
	size_t unstuffed_bytes = 0;
	if (!cobs_decode(rx_packet.unstuffed_buf,
			 sizeof(rx_packet.unstuffed_buf),
			 rx_packet.stuffed_buf, pos,
			 &unstuffed_bytes)) {
		// Abandon dispatch if the COBS encoding is bad.
		dprintf(DEBUG_INFO, "Bad COBS data\n");
		return -1;
	}

	// Must be at least enough space for a CRC.
	if (unstuffed_bytes < 4) {
		dprintf(DEBUG_INFO, "Runt packet len %d\n",
			(int) unstuffed_bytes);
		return -1;
	}

	size_t payload_size = unstuffed_bytes - 4;

	// Must be enough room to receive the buffer up to the CRC.
	if (payload_size > bytes) {
		dprintf(DEBUG_INFO, "Frame payload too large: %d vs %d\n",
			(int) payload_size, (int) bytes);
		return -1;
	}

	// Check CRC-32 is good.
	uint32_t computed_crc = crc32(rx_packet.unstuffed_buf, payload_size);
	uint32_t sent_crc = read_le_32(rx_packet.unstuffed_buf, payload_size);
	if (computed_crc != sent_crc) {
		dprintf(DEBUG_INFO,
			"Bad CRC %d: computed 0x%08x received 0x%08x\n",
			(int) payload_size, computed_crc, sent_crc);
		return -1;
	}
	dprintf(DEBUG_SPEW, "Received frame with CRC 0x%08x\n", sent_crc);

	// Success - we have a correctly received frame.
	memcpy(buf, rx_packet.unstuffed_buf, payload_size);
	*received = payload_size;
	return 0;
}

int32_t dockfifo_bulk_write_frame(const void *buf, size_t bytes)
{
	// Need room for pipe number and CRC-32.
	if (bytes > sizeof(tx_packet.unstuffed_buf) - 4) {
		dprintf(DEBUG_INFO, "No room to serialize packet\n");
		return -1;
	}
	
	// Copy packet to unstuffed packet buffer, append with CRC-32.
	memcpy(tx_packet.unstuffed_buf, buf, bytes);
	uint32_t crc = crc32(tx_packet.unstuffed_buf, bytes);
	write_le_32(tx_packet.unstuffed_buf, bytes, crc);

	// COBS byte-stuffing.
	size_t stuffed_size = 0;
	if (!cobs_encode(tx_packet.stuffed_buf, sizeof(tx_packet.stuffed_buf),
			 tx_packet.unstuffed_buf, bytes + 4,
			 &stuffed_size)) {
		dprintf(DEBUG_INFO, "No room to serialize framing\n");
		return -1;
	}

	dprintf(DEBUG_SPEW, "Send frame with CRC 0x%08x\n", crc);
	// SOF.
	dprintf(DEBUG_SPEW, "Send SOF\n");
	dockfifo_bulk_write_byte(0x00);
	// COBS stuffed data.
	dprintf(DEBUG_SPEW, "Send %d data\n", (int) stuffed_size);
	dockfifo_bulk_write_bytes(tx_packet.stuffed_buf, stuffed_size);
	// EOF.
	dprintf(DEBUG_SPEW, "Send EOF\n");
	dockfifo_bulk_write_byte(0x00);

	return 0;
}

int32_t dockfifo_bulk_init()
{
	dprintf(DEBUG_SPEW, "dockfifo_bulk_init\n");

	// Initialize rx cbuffer.
	cb_create(&rx_cbuf, RX_CBUF_SIZE);

	// Setup hardware.
	clock_gate(CLK_SPU, true);
	clock_gate(CLK_DOCKFIFO, true);

	// Reset fifos.
	rDBGFIFO_CNFG(DOCKFIFO_BULK_READ) = (1 << 31);
	rDBGFIFO_CNFG(DOCKFIFO_BULK_WRITE) = (1 << 31);
	spin(1);
	rDBGFIFO_CNFG(DOCKFIFO_BULK_READ) = (0 << 31);
	rDBGFIFO_CNFG(DOCKFIFO_BULK_WRITE) = (0 << 31);

	// Initialize local events.
	event_init(&dockfifo_bulk_rx_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

	dprintf(DEBUG_SPEW, "interrupt: %d\n",
		(int) dockfifo_configs[DOCKFIFO_BULK_READ].irq);
	install_int_handler(dockfifo_configs[DOCKFIFO_BULK_READ].irq,
			    &dockfifo_bulk_interrupt_handler,
			    (void *)DOCKFIFO_BULK_READ);

	// Enable access to fifo
	platform_dockfifo_access_enable(DOCKFIFO_BULK_READ, true);
	platform_dockfifo_access_enable(DOCKFIFO_BULK_WRITE, true);

	return 0;
}

int32_t dockfifo_bulk_quiesce()
{
	dprintf(DEBUG_SPEW, "dockfifo_bulk_quiesce\n");

	mask_int(dockfifo_configs[DOCKFIFO_BULK_READ].irq);

	// Disable access to fifo
	platform_dockfifo_access_enable(DOCKFIFO_BULK_READ, false);
	platform_dockfifo_access_enable(DOCKFIFO_BULK_WRITE, false);

	// Reset rx cbuffer.
	cb_reset(&rx_cbuf);

	// Reset fifos.
	rDBGFIFO_CNFG(DOCKFIFO_BULK_READ) = (1 << 31);
	rDBGFIFO_CNFG(DOCKFIFO_BULK_WRITE) = (1 << 31);
	spin(1);
	rDBGFIFO_CNFG(DOCKFIFO_BULK_READ) = (0 << 31);
	rDBGFIFO_CNFG(DOCKFIFO_BULK_WRITE) = (0 << 31);

	clock_gate(CLK_DOCKFIFO, false);
	clock_gate(CLK_SPU, false);

	return 0;
}
