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
#include <drivers/dart.h>
#include <drivers/thunderbolt/nhi.h>
#include <drivers/pci.h>
#include <platform.h>
#include <stdlib.h>
#include <sys/task.h>
#include <sys.h>
#include "nhi_protocol.h"

#define NHID_INIT			(DEBUG_INFO)
// These will lead to deadlock if thunderbolt is used for the serial console
#define NHID_REG			(DEBUG_SPEW)
#define NHID_DMA			(DEBUG_SPEW)
#define NHID_RXPACKET_DUMP		(DEBUG_NEVER)
#define NHID_TXPACKET_DUMP		(DEBUG_NEVER)

#define NHI_PROBE_MAX_BRIDGES		(8)
#define NHI_PROBE_MIN_BAR_SIZE		(0x40000)

#define NHI_MAX_RINGS			(1)

#define NHI_BOOKKEEPING_IOVM_BASE	(0x80100000)
#define NHI_BOOKKEEPING_PAGES		(4)
#define NHI_BOOKKEEPING_SIZE		(NHI_BOOKKEEPING_PAGES * DART_PAGE_SIZE)
#define NHI_BUFFER_IOVM_BASE		(0x80200000)
#if NHI_BOOKKEEPING_IOVM_BASE + NHI_BOOKKEEPING_SIZE > NHI_BUFFER_IOVM_BASE
#error "NHI bookkeeping and buffer IOVM ranges overlap"
#endif

typedef struct nhi_ring {
	uint64_t buffers_iovm_addr;
	uint64_t descriptors_iovm_addr;
	uint32_t num_buffers;
	uint32_t buffer_size;
	uint32_t next_buffer_index;
	
	// All data structures after this comment are mapped via the
	// DART, so software MUST NOT assume they will stay constant.
	// Values must be copied to a safe location and then validated
	void *buffers;
	void *descriptors;
} nhi_ring_t;

struct nhi {
	nhi_ring_t tx_rings[NHI_MAX_RINGS];
	nhi_ring_t rx_rings[NHI_MAX_RINGS];

	uintptr_t bookkeeping_base;
	size_t bookkeeping_size;
	size_t bookkeeping_used;

	size_t buffer_space_mapped;

	pci_device_t pci_device;
	uintptr_t bar0;
	uint32_t dart_id;
	pci_device_t upstream_bridge;
};

static void nhi_quiesce_pci(nhi_t *nhi);

static void nhi_write_reg(nhi_t *nhi, uint32_t offset, uint32_t value)
{
	dprintf(NHID_REG, "nhi: write register 0x%05x (0x%llx) <- 0x%08x\n", offset, nhi->bar0 + offset, value);
	*(volatile uint32_t *)(nhi->bar0 + offset) = value;
}

static uint32_t nhi_read_reg(nhi_t *nhi, uint32_t offset)
{
	uint32_t value;
	value = *(volatile uint32_t *)(nhi->bar0 + offset);
	dprintf(NHID_REG, "nhi: read register 0x%05x (0x%llx) -> 0x%08x\n", offset, nhi->bar0 + offset, value);
	return value;
}

static nhi_rx_buffer_desc_t *nhi_get_rx_buffer_desc(nhi_t *nhi, uint32_t ring_idx, uint32_t buffer_idx)
{
	nhi_ring_t *ring;

	ASSERT(nhi != NULL);
	ASSERT(ring_idx < NHI_MAX_RINGS);

	ring = &nhi->rx_rings[ring_idx];
	ASSERT(buffer_idx < ring->num_buffers);

	return (nhi_rx_buffer_desc_t *)ring->descriptors + buffer_idx;
}

static nhi_tx_buffer_desc_t *nhi_get_tx_buffer_desc(nhi_t *nhi, uint32_t ring_idx, uint32_t buffer_idx)
{
	nhi_ring_t *ring;

	ASSERT(nhi != NULL);
	ASSERT(ring_idx < NHI_MAX_RINGS);

	ring = &nhi->tx_rings[ring_idx];
	ASSERT(buffer_idx < ring->num_buffers);

	return (nhi_tx_buffer_desc_t *)ring->descriptors + buffer_idx;
}

static void *nhi_ring_get_buffer(nhi_ring_t *ring, uint32_t buffer_idx)
{
	ASSERT(ring != NULL);
	ASSERT(buffer_idx < ring->num_buffers);

	return (void *)((uint8_t *)ring->buffers + buffer_idx * ring->buffer_size);
}

static uint64_t nhi_ring_get_buffer_iovm_addr(nhi_ring_t *ring, uint32_t buffer_idx)
{
	ASSERT(ring != NULL);
	ASSERT(buffer_idx < ring->num_buffers);

	return ring->buffers_iovm_addr + buffer_idx * ring->buffer_size;
}

void nhi_debug_write(pci_device_t bridge, uint32_t vcap, uint8_t space, uint8_t port, uint16_t offset, uint32_t value)
{
	uint32_t cmd;
	
	cmd = offset | (port << 13) | (space << 19) | (1 << 21) | (0 << 22);

	pci_config_write32(bridge, vcap + 0x34, value);
	pci_config_write32(bridge, vcap + 0x30, cmd | (1 << 30));
	spin(20);

	for (int retries = 5; retries > 0; retries--) {
		do {
			cmd = pci_config_read32(bridge, vcap + 0x30);
			if ((cmd & (1U << 30)) == 0)
				break;
			if (cmd == 0xffffffff)
				return;
			spin(100);
		} while(1);

		if (cmd & (1U << 31)) {
			spin(2000);
			continue;
		}
		break;
	}
}

uint32_t nhi_debug_read(pci_device_t bridge, uint32_t vcap, uint8_t space, uint8_t port, uint16_t offset)
{
	uint32_t cmd;
	
	cmd = offset | (port << 13) | (space << 19) | (0 << 22);

	pci_config_write32(bridge, vcap + 0x30, cmd | (1 << 30));
	spin(20);

	for (int retries = 5; retries > 0; retries--) {
		do {
			cmd = pci_config_read32(bridge, vcap + 0x30);
			if ((cmd & (1U << 30)) == 0)
				break;
			if (cmd == 0xffffffff)
				return 0xcdcdcdcd;
			spin(100);
		} while(1);

		if (cmd & (1U << 31)) {
			spin(2000);
			continue;
		}
		return pci_config_read32(bridge, vcap + 0x38);
	}

	return 0xabababab;
}

static void debug_dump_packet(uint8_t *buffer, uint32_t bytes)
{
	for (uint32_t off = 0; off < bytes; off += 4) {
		uint32_t dword;
		if (off % 16 == 0)
			printf("[0x%03x] ", off);
		memcpy(&dword, buffer + off, 4);
		printf("%08x ", ntohl(dword));
		if (off % 16 == 12)
			printf("\n");
	}
	if ((bytes) % 16 != 0)
		printf("\n");
}

void nhi_dump_config_space(pci_device_t bridge)
{
	uint32_t offset;

	offset = pci_find_extended_capability(bridge, 0xb, 0);
	if (offset == 0) {
		printf("couldn't get vendor-specific ecap\n");
		return;
	}

#if 0
	for (int j = 0; j < 0; j++) {
		printf("Port %d port configuration space\n", j);
		for (uint32_t i = 0; i < 8; i += 1) {
			uint32_t value = nhi_debug_read(bridge, offset, 1, j, i);
			spin(1000);
			if (i % 4 == 0)
				printf("%02x ", i);
			printf(" %08x", value);
			if (i % 4 == 3)
				printf("\n");
		}
	}
	printf("\n");

	for (int j = 0; j < 6; j++) {
		printf("Port %d path configuration space\n", j);
		for (uint32_t i = 0; i < 16; i += 1) {
			uint32_t value = nhi_debug_read(bridge, offset, 0, j, i);
			spin(1000);
			if (i % 4 == 0)
				printf("%02x ", i);
			printf(" %08x", value);
			if (i % 4 == 3)
				printf("\n");
		}
	}
	printf("\n");
#endif

	printf("Device configuration space\n");
	for (int i = 0; i < 5; i += 1) {
		uint32_t value = nhi_debug_read(bridge, offset, 2, 0, i);
			spin(1000);
		if (i % 4 == 0)
			printf("%02x ", i);
		printf(" %08x", value);
		if (i % 4 == 3)
			printf("\n");
	}
	printf("\n");
}

nhi_t *nhi_init(pci_device_t bridge, uint32_t dart_id)
{
	nhi_t *nhi = NULL;
	uint32_t class_code;
	uint32_t vendor_id;
	uintptr_t bar0;
	pci_device_t dev;
	pci_device_t temp_bridge;
	pci_device_t probed_bridges[NHI_PROBE_MAX_BRIDGES] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

	ASSERT(bridge != NULL);

	temp_bridge = bridge;
	for (unsigned i = 0; i < NHI_PROBE_MAX_BRIDGES; i++) {
		dev = pci_bridge_probe(temp_bridge, 0, 50000);
		if (dev == NULL)
			break;

		if (dev == temp_bridge) {
			// We got the bridge back. Usually a sign of misconfiguration
			// in our PCI stack
			dprintf(DEBUG_CRITICAL, "nhi: probe from bridge %s returned itself\n", pci_get_name(temp_bridge));
			dev = NULL;
			break;
		}
		
		if (pci_is_bridge(dev)) {
			dprintf(NHID_INIT, "nhi: probe found bridge: %s\n", pci_get_name(dev));
			pci_bridge_assign_secondary_bus(dev);
			probed_bridges[i] = dev;
			temp_bridge = dev;
			dev = NULL;
		} else {
			break;
		}
	}

	if (dev == NULL) {
		dprintf(DEBUG_CRITICAL, "nhi: couldn't find a PCI device\n");
		goto error;
	}

	dprintf(NHID_INIT, "nhi: found NHI PCI device %s\n", pci_get_name(dev));

	class_code = pci_get_class_code(dev);
	vendor_id = pci_get_vendor_id(dev);

	if (class_code != NHI_CLASS_CODE) {
		dprintf(DEBUG_CRITICAL, "nhi: incorrect class code 0x%x\n", class_code);
		goto error;
	}
	if (vendor_id != NHI_VENDOR_ID) {
		dprintf(DEBUG_CRITICAL, "nhi: incorrect vendor ID 0x%x\n", vendor_id);
		goto error;
	}

	if (pci_get_bar_size(dev, 0) < NHI_PROBE_MIN_BAR_SIZE) {
		dprintf(DEBUG_CRITICAL, "nhi: BAR0 size 0x%llx too small\n", pci_get_bar_size(dev, 0));
		goto error;
	}
	bar0 = pci_map_bar(dev, 0);
	if (bar0 == 0) {
		dprintf(DEBUG_CRITICAL, "nhi: couldn't map BAR0\n");
		goto error;
	}

	dprintf(NHID_INIT, "nhi: BAR0 mapped at 0x%llx\n", bar0);

	nhi = calloc(sizeof(*nhi), 1);
	nhi->pci_device = dev;
	nhi->bar0 = bar0;
	nhi->dart_id = dart_id;

	nhi->bookkeeping_size = NHI_BOOKKEEPING_PAGES * DART_PAGE_SIZE;
	nhi->bookkeeping_base = (uintptr_t)memalign(nhi->bookkeeping_size, DART_PAGE_SIZE);
	dart_map_page_range(nhi->dart_id, nhi->bookkeeping_base, NHI_BOOKKEEPING_IOVM_BASE, NHI_BOOKKEEPING_PAGES, false);

	pci_bus_master_enable(dev, true);
	pci_memory_space_enable(dev, true);

	// set bus master and memory space enable on all the bridges our probe found
	for (unsigned i = 0; i < NHI_PROBE_MAX_BRIDGES && probed_bridges[i] != NULL; i++) {
		pci_bus_master_enable(probed_bridges[i], true);
		pci_memory_space_enable(probed_bridges[i], true);
	}
	nhi->upstream_bridge = probed_bridges[0];

	return nhi;

error:
	dprintf(DEBUG_CRITICAL, "nhi: initialization failed\n");
	if (nhi != NULL) {
		if (nhi->pci_device != NULL)
			nhi_quiesce_pci(nhi);
		if (nhi->bookkeeping_base != 0) {
			dart_unmap_page_range(nhi->dart_id, NHI_BOOKKEEPING_IOVM_BASE, NHI_BOOKKEEPING_PAGES);
			free((void *)nhi->bookkeeping_base);
			nhi->bookkeeping_base = 0;
		}

		free(nhi);
	} else {
		for (unsigned i = 0; i < NHI_PROBE_MAX_BRIDGES; i++) {
			if (probed_bridges[NHI_PROBE_MAX_BRIDGES - i - 1] == NULL)
				continue;
			pci_free(probed_bridges[NHI_PROBE_MAX_BRIDGES - i - 1]);
		}
		if (dev != NULL)
			pci_free(dev);
	}
	return NULL;
}

static void nhi_quiesce_pci(nhi_t *nhi)
{
	pci_device_t device;
	pci_device_t next_device;

	pci_set_powerstate(nhi->pci_device, PCI_PM_POWERSTATE_D3);

	// Disable bridges up to and including the upstream-most one managed by the driver.
	// nhi_init will have stored the first bridge it found (if any) while looking for
	// the NHI
	next_device = nhi->pci_device;
	do {
		device = next_device;
		pci_bus_master_enable(device, false);
		pci_memory_space_enable(device, false);
		next_device = pci_get_bridge(device);
		pci_free(device);
	} while (nhi->upstream_bridge != NULL && device != nhi->upstream_bridge);
	nhi->upstream_bridge = NULL;
	nhi->pci_device = NULL;
}

void nhi_quiesce_and_free(nhi_t *nhi)
{
	dart_unmap_page_range(nhi->dart_id, NHI_BOOKKEEPING_IOVM_BASE, NHI_BOOKKEEPING_PAGES);
	free((void *)nhi->bookkeeping_base);
	nhi->bookkeeping_base = 0;

	nhi_quiesce_pci(nhi);

	for (unsigned i = 0; i < NHI_MAX_RINGS; i++) {
		if (nhi->tx_rings[i].buffers != NULL)
			free(nhi->tx_rings[i].buffers);
	}

	for (unsigned i = 0; i < NHI_MAX_RINGS; i++) {
		if (nhi->rx_rings[i].buffers != NULL)
			free(nhi->rx_rings[i].buffers);
	}

	free(nhi);

	dprintf(NHID_INIT, "nhi: quiesced\n");
}

static uint64_t nhi_dart_map(nhi_t *nhi, void *addr, size_t bytes)
{
	ASSERT(nhi != NULL);
	ASSERT(addr != NULL);
	ASSERT(bytes % DART_PAGE_SIZE == 0);
	ASSERT(nhi->buffer_space_mapped + bytes > nhi->buffer_space_mapped);

	uint64_t iovmaddr = NHI_BUFFER_IOVM_BASE + nhi->buffer_space_mapped;
	uint32_t pages = bytes / DART_PAGE_SIZE;

	dart_map_page_range(nhi->dart_id, (uintptr_t)addr, iovmaddr, pages, false);

	nhi->buffer_space_mapped += bytes;

	return iovmaddr;
}

static void *nhi_allocate_bookkeeping(nhi_t *nhi, size_t bytes) {
	void *allocated;
	ASSERT(nhi != NULL);
	ASSERT(nhi->bookkeeping_base != 0);
	ASSERT(bytes > 0);
	ASSERT(bytes <= nhi->bookkeeping_size - nhi->bookkeeping_used);

	allocated = (void *)(nhi->bookkeeping_base + nhi->bookkeeping_used);
	nhi->bookkeeping_used += bytes;

	return allocated;
}

static uint64_t nhi_translate_bookkeeping(nhi_t *nhi, void *addr) {
	ASSERT(nhi != NULL);

	ASSERT((uintptr_t)addr >= nhi->bookkeeping_base);
	ASSERT((uintptr_t)addr < nhi->bookkeeping_base + nhi->bookkeeping_used);

	return NHI_BOOKKEEPING_IOVM_BASE + (uintptr_t)addr - nhi->bookkeeping_base;
}

static void nhi_alloc_ring(nhi_t *nhi, nhi_ring_t *ring, uint32_t num_buffers, uint32_t buffer_size)
{
	size_t total_size;

	ASSERT(ring != NULL);
	ASSERT(ring->buffers == NULL);
	ASSERT(ring->descriptors == NULL);
	ASSERT(num_buffers < 0x10000);
	ASSERT(buffer_size <= 4096);

	ring->num_buffers = num_buffers;
	ring->buffer_size = buffer_size;

	// rx and tx buffer descriptor have the same size
	ring->descriptors = nhi_allocate_bookkeeping(nhi, sizeof(nhi_tx_buffer_desc_t) * num_buffers);
	ring->descriptors_iovm_addr = nhi_translate_bookkeeping(nhi, ring->descriptors);

	total_size = num_buffers * buffer_size;
	total_size = (total_size + DART_PAGE_SIZE - 1) & ~(DART_PAGE_SIZE - 1);
	ring->buffers = memalign(total_size, DART_PAGE_SIZE);
	ring->buffers_iovm_addr = nhi_dart_map(nhi, ring->buffers, total_size);

}

void nhi_init_tx_ring(nhi_t *nhi, uint32_t ring_idx, uint32_t num_buffers, uint32_t buffer_size)
{
	nhi_ring_t *ring;
	uint32_t addr_lo;
	uint32_t addr_hi;
	uint32_t flags;

	ASSERT(nhi != NULL);
	ASSERT(ring_idx < NHI_MAX_RINGS);
	ASSERT(num_buffers < 0x10000);
	ASSERT(buffer_size <= 4096);
	ASSERT(buffer_size % 4 == 0);

	dprintf(NHID_INIT, "nhi: tx ring %u init with %u buffers of %u bytes\n", ring_idx, num_buffers, buffer_size);

	ring = &nhi->tx_rings[ring_idx];

	nhi_alloc_ring(nhi, ring, num_buffers, buffer_size);

	dprintf(NHID_INIT, "nhi: tx ring %u buffers at %p\n", ring_idx, ring->buffers);
	dprintf(NHID_INIT, "nhi: tx ring %u buffer descriptors at %p\n", ring_idx, ring->descriptors);

	for (uint32_t i = 0; i < num_buffers; i++) {
		nhi_tx_buffer_desc_t *desc;
		uint64_t buffer_addr;

		desc = nhi_get_tx_buffer_desc(nhi, ring_idx, i);
		memset(desc, 0, sizeof(*desc));
		buffer_addr = nhi_ring_get_buffer_iovm_addr(ring, i);
		desc->addr_lo = (uint32_t)buffer_addr;
		desc->addr_hi = (uint32_t)(buffer_addr >> 32);
		desc->request_status = 1;
		desc->descriptor_done = 1;
	}

	addr_lo = (uint32_t)ring->descriptors_iovm_addr;
	addr_hi = (uint32_t)(ring->descriptors_iovm_addr >> 32);
	// buffer size of 4096 is expressed as 0
	nhi_write_reg(nhi, NHI_REG_TX_RING_ADDR_LO(ring_idx), addr_lo);
	nhi_write_reg(nhi, NHI_REG_TX_RING_ADDR_HI(ring_idx), addr_hi);
	nhi_write_reg(nhi, NHI_REG_TX_RING_SIZE(ring_idx), num_buffers);

	// for now, only support raw mode rings
	flags = NHI_TX_TABLE_VALID | NHI_TX_TABLE_RAW_MODE;
	nhi_write_reg(nhi, NHI_REG_TX_TABLE_FLAGS(ring_idx), flags);
}

void nhi_disable_tx_ring(nhi_t *nhi, uint32_t ring_idx)
{
	ASSERT(nhi != NULL);
	ASSERT(ring_idx < NHI_MAX_RINGS);

	nhi_write_reg(nhi, NHI_REG_TX_TABLE_FLAGS(ring_idx), 0);
}

void nhi_init_rx_ring(nhi_t *nhi, uint32_t ring_idx, uint32_t num_buffers, uint32_t buffer_size)
{
	nhi_ring_t *ring;
	uint32_t addr_lo;
	uint32_t addr_hi;
	uint32_t sizes;
	uint32_t flags;

	ASSERT(nhi != NULL);
	ASSERT(ring_idx < NHI_MAX_RINGS);
	ASSERT(num_buffers < 0x10000);
	ASSERT(buffer_size >= 256);
	ASSERT(buffer_size <= 0x1000);
	ASSERT(buffer_size % 4 == 0);

	dprintf(NHID_INIT, "nhi: rx ring %u init with %u buffers of %u bytes\n", ring_idx, num_buffers, buffer_size);

	ring = &nhi->rx_rings[ring_idx];

	nhi_alloc_ring(nhi, ring, num_buffers, buffer_size);

	dprintf(NHID_INIT, "nhi: rx ring %u buffers at %p\n", ring_idx, ring->buffers);
	dprintf(NHID_INIT, "nhi: rx ring %u buffer descriptors at %p\n", ring_idx, ring->descriptors);

	for (uint32_t i = 0; i < num_buffers; i++) {
		nhi_rx_buffer_desc_t *desc;
		uint64_t buffer_addr;

		desc = nhi_get_rx_buffer_desc(nhi, ring_idx, i);
		memset(desc, 0, sizeof(*desc));
		buffer_addr = nhi_ring_get_buffer_iovm_addr(ring, i);
		desc->addr_lo = (uint32_t)buffer_addr;
		desc->addr_hi = (uint32_t)(buffer_addr >> 32);
		desc->request_status = 1;
	}

	addr_lo = (uint32_t)ring->descriptors_iovm_addr;
	addr_hi = (uint32_t)(ring->descriptors_iovm_addr >> 32);
	// buffer size of 4096 is expressed as 0
	sizes = NHI_RX_RING_DESC_RING_SIZE_BUFFER_SIZE(num_buffers, buffer_size);
	nhi_write_reg(nhi, NHI_REG_RX_RING_ADDR_LO(ring_idx), addr_lo);
	nhi_write_reg(nhi, NHI_REG_RX_RING_ADDR_HI(ring_idx), addr_hi);
	nhi_write_reg(nhi, NHI_REG_RX_RING_SIZE_BUFFER_SIZE(ring_idx), sizes);

	// Feed all but the last frame to the NHI... keep the last one back because
	// producer == consumer means an empty ring
	nhi_write_reg(nhi, NHI_REG_RX_RING_INDEXES(ring_idx), num_buffers - 1);
	ring->next_buffer_index = 0;

	// for now, accept every PDF value as start and end of frame
	nhi_write_reg(nhi, NHI_REG_RX_TABLE_PDF_BITMASKS(ring_idx), 0xffffffff);
	// for now, only support raw mode rings
	flags = NHI_RX_TABLE_VALID | NHI_RX_TABLE_RAW_MODE;
	nhi_write_reg(nhi, NHI_REG_RX_TABLE_FLAGS(ring_idx), flags);
}

void nhi_disable_rx_ring(nhi_t *nhi, uint32_t ring_idx)
{
	ASSERT(nhi != NULL);
	ASSERT(ring_idx < NHI_MAX_RINGS);

	nhi_write_reg(nhi, NHI_REG_RX_TABLE_FLAGS(ring_idx), 0);
}

int32_t nhi_send_sgl(nhi_t *nhi, uint32_t ring_idx, const nhi_sgl_t *sgl, uint8_t sof_pdf, uint8_t eof_pdf)
{
	nhi_ring_t *ring;
	volatile nhi_tx_buffer_desc_t *desc;
	uint32_t bytes = 0;
	uint8_t *buffer;
	utime_t start_time;

	ASSERT(nhi != NULL);
	ASSERT(ring_idx < NHI_MAX_RINGS);

	ring = &nhi->tx_rings[ring_idx];
	desc = nhi_get_tx_buffer_desc(nhi, ring_idx, ring->next_buffer_index);
	buffer = nhi_ring_get_buffer(ring, ring->next_buffer_index);

	start_time = system_time();
	while (!desc->descriptor_done) {
		if (time_has_elapsed(start_time, 2000000))
			return NHI_STATUS_TIMEOUT;
		task_yield();
		platform_memory_barrier();
	}

	dprintf(NHID_DMA, "nhi: tx ring %u queueing buffer %u (%p) for transmit, pdfs %04x/%04x\n",
		ring_idx, ring->next_buffer_index, buffer, sof_pdf, eof_pdf);

	while (sgl != NULL) {
		ASSERT(bytes + sgl->bytes > bytes);
		ASSERT(bytes + sgl->bytes <= ring->buffer_size);

		memcpy(buffer + bytes, sgl->buffer, sgl->bytes);
		bytes += sgl->bytes;

		sgl = sgl->next;
	}

#if NHID_TXPACKET_DUMP <= DEBUG_LEVEL
	printf("nhi: tx packet:\n");
	debug_dump_packet(buffer, bytes);
#endif

	// Update the DMA descriptor the NHI will DMA from host memory
	desc->length = bytes;
	desc->sof_pdf = sof_pdf;
	desc->eof_pdf = eof_pdf;
	desc->descriptor_done = 0;
	desc->interrupt_enable = 0;
	desc->request_status = 1;

	// Update our internal accounting of the next free descriptor in the ring
	ring->next_buffer_index++;
	if (ring->next_buffer_index == ring->num_buffers)
		ring->next_buffer_index = 0;

	platform_memory_barrier();

	// And let the NHI know that there's a new descriptor for it to process
	nhi_write_reg(nhi, NHI_REG_TX_RING_INDEXES(ring_idx), ring->next_buffer_index << 16);

	return NHI_STATUS_OK;
}

int32_t nhi_send_buffer(nhi_t *nhi, uint32_t ring_idx, void *buffer, uint32_t bytes, uint8_t sof_pdf, uint8_t eof_pdf)
{
	nhi_sgl_t sgl;

	sgl.buffer = buffer;
	sgl.bytes = bytes;
	sgl.next = NULL;

	return nhi_send_sgl(nhi, ring_idx, &sgl, sof_pdf, eof_pdf);
}

bool nhi_rx_buffer_available(nhi_t *nhi, uint32_t ring_idx)
{
	nhi_ring_t *ring;
	volatile nhi_rx_buffer_desc_t *next;
	ASSERT(nhi != NULL);
	ASSERT(ring_idx < NHI_MAX_RINGS);

	ring = &nhi->rx_rings[ring_idx];

	next = nhi_get_rx_buffer_desc(nhi, ring_idx, ring->next_buffer_index);

	platform_memory_barrier();

	return next->descriptor_done != 0;
}

int32_t nhi_rx_buffer(nhi_t *nhi, uint32_t ring_idx, void *buffer, uint32_t bytes, uint8_t *sof_pdf, uint8_t *eof_pdf)
{
	nhi_ring_t *ring;
	nhi_rx_buffer_desc_t *next;
	nhi_rx_buffer_desc_t descriptor;
	int32_t status = NHI_STATUS_UNKNOWN_ERROR;
	utime_t start_time;

	ASSERT(nhi != NULL);
	ASSERT(ring_idx < NHI_MAX_RINGS);

	ring = &nhi->rx_rings[ring_idx];
	ASSERT(bytes <= ring->buffer_size);

	start_time = system_time();
	while (!nhi_rx_buffer_available(nhi, ring_idx)) {
		if (time_has_elapsed(start_time, 2000000))
			return NHI_STATUS_TIMEOUT;
		task_yield();
	}

	// XXX: Are we guaranteed that buffer update is atomic and/or length write never
	//      passes status update write?
	next = nhi_get_rx_buffer_desc(nhi, ring_idx, ring->next_buffer_index);
	memcpy(&descriptor, next, sizeof(descriptor));

	platform_memory_barrier();

	if (descriptor.crc_error) {
		dprintf(DEBUG_CRITICAL, "nhi: rx ring %u CRC error\n", ring_idx);
		status = NHI_STATUS_CRC_ERROR;
		goto done;
	}

	if (descriptor.length > bytes) {
		dprintf(DEBUG_CRITICAL, "nhi: rx ring %u length overrun (%u vs %u)\n",
			ring_idx, descriptor.length, bytes);
		status = NHI_STATUS_INVALID_DESCRIPTOR;
		goto done;
	}

	if (sof_pdf != NULL)
		*sof_pdf = descriptor.sof_pdf;
	if (eof_pdf != NULL)
		*eof_pdf = descriptor.eof_pdf;

	status = descriptor.length;
	memcpy(buffer, nhi_ring_get_buffer(ring, ring->next_buffer_index), descriptor.length);

	dprintf(NHID_DMA, "nhi: ring %u rx packet pdfs %x/%x bytes %u\n", ring_idx,
		descriptor.sof_pdf, descriptor.eof_pdf, descriptor.length);
#if NHID_RXPACKET_DUMP <= DEBUG_LEVEL
	printf("nhi: rx packet:\n");
	debug_dump_packet(buffer, descriptor.length);
#endif

done:
	// Use the buffer we just processed as the free buffer, and give the previous
	// free buffer to the DMA engine
	nhi_write_reg(nhi, NHI_REG_RX_RING_INDEXES(ring_idx), ring->next_buffer_index);
	ring->next_buffer_index++;
	if (ring->next_buffer_index == ring->num_buffers)
		ring->next_buffer_index = 0;
	// Reset the buffer we just processed
	next->details = 0;
	next->length = ring->buffer_size;
	next->request_status = 1;

	return status;
}
