/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <drivers/pci.h>
#include <platform.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/menu.h>
#include "pci_private.h"

#define PDBG_REG	DEBUG_SPEW
#define PDBG_SPEW	DEBUG_SPEW
#define PDBG_PROBE	DEBUG_SPEW
#define PDBG_MAP	DEBUG_SPEW

pci_device_t pci_bridges[256];

void pci_init(void)
{
	platform_register_pci_busses();
}

static void pci_register_bridge(pci_device_t bridge)
{
	ASSERT(pci_bridges[bridge->secondary_bus] == NULL);

	pci_bridges[bridge->secondary_bus] = bridge;
}

static void pci_unregister_bridge(pci_device_t bridge)
{
	ASSERT(pci_bridges[bridge->secondary_bus] != NULL);

	pci_bridges[bridge->secondary_bus] = NULL;
}

pci_device_t pci_create_host_bridge(const char *name, uint8_t bus_num, uint32_t memory_base, uint32_t memory_size, void *priv, pci_config_read_cb_t read_cb, pci_config_write_cb_t write_cb)
{
	pci_device_t new_bridge;

	new_bridge = calloc(sizeof(*new_bridge), 1);

	strlcpy(new_bridge->name, name, sizeof(new_bridge->name));
	new_bridge->secondary_bus = bus_num;
	new_bridge->subordinate_bus = bus_num;
	new_bridge->memory_base = memory_base;
	new_bridge->memory_size = memory_size;
	new_bridge->memory_allocated = 0;
	new_bridge->config_read = read_cb;
	new_bridge->config_write = write_cb;
	new_bridge->bridge_priv = priv;

	pci_register_bridge(new_bridge);

	return new_bridge;
}

// When the host bridge's last child goes away, reset all of the
// allocations to 0, allowing us to reprobe under the host bridge
// again if needed
static void pci_reinitialize_host_bridge(pci_device_t host_bridge)
{
	ASSERT(host_bridge->num_children == 0);
	ASSERT(host_bridge->bridge == NULL);

	host_bridge->subordinate_bus = host_bridge->secondary_bus;
	host_bridge->memory_allocated = 0;
}

// 8 bit wide read from the given device/offset under the given bridge
// Function is assumed to be 0 (no multi-function devices)
static uint8_t pci_bridge_config_read8(pci_device_t bridge, uint8_t device_num, uint16_t offset)
{
	uint8_t value;
	uint32_t bdfo = PCI_BDFO(bridge->secondary_bus, device_num, 0, offset);
	bridge->config_read(bridge->bridge_priv, &value, bdfo, 1);

	dprintf(PDBG_REG, "pci(%02x:%02x:00):  8-bit config read 0x%04x -> 0x%02x\n",
	        bridge->secondary_bus, device_num, offset, value);

	return value;
}	

// 16 bit wide read from the given device/offset under the given bridge
// Function is assumed to be 0 (no multi-function devices)
static uint16_t pci_bridge_config_read16(pci_device_t bridge, uint8_t device_num, uint16_t offset)
{
	uint16_t value;
	uint32_t bdfo = PCI_BDFO(bridge->secondary_bus, device_num, 0, offset);
	bridge->config_read(bridge->bridge_priv, &value, bdfo, 2);

	dprintf(PDBG_REG, "pci(%02x:%02x:00): 16-bit config read 0x%04x -> 0x%04x\n",
	        bridge->secondary_bus, device_num, offset, value);

	return value;
}	

// 32 bit wide read from the given device/offset under the given bridge
// Function is assumed to be 0 (no multi-function devices)
static uint32_t pci_bridge_config_read32(pci_device_t bridge, uint8_t device_num, uint16_t offset)
{
	uint32_t value;
	uint32_t bdfo = PCI_BDFO(bridge->secondary_bus, device_num, 0, offset);
	bridge->config_read(bridge->bridge_priv, &value, bdfo, 4);

	dprintf(PDBG_REG, "pci(%02x:%02x:00): 32-bit config read 0x%04x -> 0x%08x\n",
	        bridge->secondary_bus, device_num, offset, value);

	return value;
}	

// 8 bit wide write to the given device/offset under the given bridge
// Function is assumed to be 0 (no multi-function devices)
static void pci_bridge_config_write8(pci_device_t bridge, uint8_t device_num, uint16_t offset, uint8_t value)
{
	uint32_t bdfo = PCI_BDFO(bridge->secondary_bus, device_num, 0, offset);

	dprintf(PDBG_REG, "pci(%02x:%02x:00):  8-bit config write 0x%04x <- 0x%02x\n",
	        bridge->secondary_bus, device_num, offset, value);

	bridge->config_write(bridge->bridge_priv, &value, bdfo, 1);
}	

// 16 bit wide write to the given device/offset under the given bridge
// Function is assumed to be 0 (no multi-function devices)
static void pci_bridge_config_write16(pci_device_t bridge, uint8_t device_num, uint16_t offset, uint16_t value)
{
	uint32_t bdfo = PCI_BDFO(bridge->secondary_bus, device_num, 0, offset);

	dprintf(PDBG_REG, "pci(%02x:%02x:00): 16-bit config write 0x%04x <- 0x%04x\n",
	        bridge->secondary_bus, device_num, offset, value);

	bridge->config_write(bridge->bridge_priv, &value, bdfo, 2);
}	

// 32 bit wide write to the given device/offset under the given bridge
// Function is assumed to be 0 (no multi-function devices)
static void pci_bridge_config_write32(pci_device_t bridge, uint8_t device_num, uint16_t offset, uint32_t value)
{
	uint32_t bdfo = PCI_BDFO(bridge->secondary_bus, device_num, 0, offset);

	dprintf(PDBG_REG, "pci(%02x:%02x:00): 32-bit config write 0x%04x <- 0x%08x\n",
	        bridge->secondary_bus, device_num, offset, value);

	bridge->config_write(bridge->bridge_priv, &value, bdfo, 4);
}	

pci_device_t pci_bridge_probe(pci_device_t bridge, uint8_t device_num, utime_t timeout)
{
	pci_device_t new_dev;
	utime_t start_time;	
	uint8_t bus_num;
	uint32_t idreg;
	uint16_t pcie_cap;

	if (bridge->devices[device_num] != NULL)
		return bridge->devices[device_num];

	bus_num = bridge->secondary_bus;

	dprintf(PDBG_PROBE, "pci(%02x:%02x:00): probe start\n", bus_num, device_num);

	start_time = system_time();
	do {
		idreg = pci_bridge_config_read32(bridge, device_num, PCI_CONFIG_VENDOR_ID);
		if ((idreg != 0xffffffff) && (idreg != 0xffff0001))
			break;
	} while (!time_has_elapsed(start_time, timeout));

	if ((idreg == 0xffffffff) || (idreg == 0xffff0001)) {
		dprintf(PDBG_PROBE, "pci(%02x:%02x:00): probe timeout\n", bus_num, device_num);
		return NULL;
	}

	new_dev = calloc(sizeof(*new_dev), 1);

	new_dev->bridge = bridge;
	new_dev->device_num = device_num;
	snprintf(new_dev->name, sizeof(new_dev->name), "%02x:%02x:%02x", bus_num, device_num, 0);

	// it's safe to use the pci_config_read_XXX functions from here forwards on new_dev

	new_dev->vendor_id = idreg & 0xffff;
	new_dev->device_id = (idreg >> 16) & 0xffff;

	idreg = pci_config_read32(new_dev, PCI_CONFIG_REVISION_ID);
	new_dev->revision_id = (idreg & 0xff);
	new_dev->class_code = ((idreg >> 8) & 0xffffff);

	idreg = pci_config_read8(new_dev, PCI_CONFIG_HEADER_TYPE);
	new_dev->header_type = idreg & 0x7f;
	new_dev->multifunction = (idreg & 0x80) != 0;
	if (new_dev->header_type != 0 && new_dev->header_type != 1) {
		dprintf(DEBUG_CRITICAL, "PCI device has unknown header type %d\n", idreg);
		dprintf(DEBUG_CRITICAL, "pci(%s): unknown header type %d\n", new_dev->name, idreg);
		goto error;
	}

	dprintf(PDBG_PROBE, "pci(%s): probed id %04x:%04x rev %02x type %02x class %06x\n",
		new_dev->name, new_dev->vendor_id, new_dev->device_id, new_dev->revision_id,
		new_dev->header_type, new_dev->class_code);

	if (new_dev->header_type == 0) {
		new_dev->num_bars = 6;
	} else {
		new_dev->num_bars = 2;
		// Bridges inherit the config read callbacks from their parents
		new_dev->config_read = bridge->config_read;
		new_dev->config_write = bridge->config_write;
		new_dev->bridge_priv = bridge->bridge_priv;
		// And a bridge's primary bus is its parent's secondary bus
		new_dev->primary_bus = bus_num;
	}

	for (uint32_t i = 0; i < new_dev->num_bars; i++) {
		uint32_t bar;
		uint32_t original;
		uint32_t masked;
		uint32_t type;
		uint64_t bar_size;
		bool is_64;
	       
		bar = PCI_CONFIG_BASE_ADDRESS0 + 4 * i;

		original = pci_config_read32(new_dev, bar);
		pci_config_write32(new_dev, bar, 0xffffffff);
		masked = pci_config_read32(new_dev, bar);
		pci_config_write32(new_dev, bar, original);

		// 0 is unimplemented
		if (masked == 0) {
			dprintf(PDBG_SPEW, "    bar %u unimplemented\n", i);
			continue;
		}

		// We don't support I/O BARs
		if ((masked & 1) != 0) {
			dprintf(PDBG_SPEW, "    bar %u I/O\n", i);
			continue;
		}

		type = (masked & 0xf) >> 1;
		// type 1 needs to be below 1 MiB, which we don't support, and type 3 is reserved
		if (type == 1 || type == 3) {
			dprintf(PDBG_SPEW, "    bar %u unknown type %u\n", i, type);
			continue;
		}

		// type 2 is 64-bit
		if (type == 2) {
			uint64_t masked2;

			// 64-bit is only valid on even BARs
			if (i % 2 != 0)
				continue;

			original = pci_config_read32(new_dev, bar + 4);
			pci_config_write32(new_dev, bar + 4, 0xffffffff);
			masked2 = pci_config_read32(new_dev, bar + 4);
			pci_config_write32(new_dev, bar + 4, original);

			masked2 <<= 32;
			masked2 |= masked;

			bar_size = ~(masked2 & ~0xfULL) + 1;
			is_64 = true;
			// Mark the next bar as being part of this one
			new_dev->bar_64bit[i+1] = true;
		} else {
			bar_size = ~(masked & ~0xf) + 1;
			is_64 = false;
		}

		new_dev->bar_size[i] = bar_size;
		new_dev->bar_64bit[i] = is_64;

		dprintf(PDBG_PROBE, "    bar %u size 0x%llx%s\n", i, bar_size, is_64 ? " (64-bit)" : "");

		// skip reading the next BAR because we just did it
		if (is_64)
			i++;
	}

	pcie_cap = pci_find_capability(new_dev, PCI_CAP_ID_PCIE);
	if (pcie_cap != 0) {
		uint16_t cap; 
		cap = pci_config_read16(new_dev, pcie_cap + PCI_PCIE_CAP_PCIE_CAPABILITIES);
		new_dev->pcie_device_port_type = (cap >> 4) & 0xf;
	} else {
		new_dev->pcie_device_port_type = PCI_PCIE_TYPE_UNKNOWN;
	}

	bridge->devices[device_num] = new_dev;
	bridge->num_children++;

	return new_dev;

error:
	free(new_dev);
	return NULL;
}

// Called after pci_bridge_probe finds a device that's a bridge in order to set
// the new bridge's secondary and subordinate bus number registers and adjust the
// subordinate bus numbers up the chain so that the new bridge's secondary bus
// is visible to the host bridge
void pci_bridge_assign_secondary_bus(pci_device_t bridge)
{
	pci_device_t parent;
	uint8_t parent_subordinate;

	ASSERT(pci_is_bridge(bridge));
	// Can only be called once for a bridge
	ASSERT(bridge->secondary_bus == 0);
	ASSERT(bridge->subordinate_bus == 0);
	// And can't be called on the host bridge
	ASSERT(bridge->bridge != NULL);

	parent = bridge->bridge;
	parent_subordinate = parent->subordinate_bus;

	// Oops, we're out of buses!
	ASSERT(parent_subordinate != 255);

	dprintf(PDBG_MAP, "pci(%s): setting bridge secondary bus 0x%02x\n",
		parent->name, parent_subordinate + 1);

	// Bridges start with just their secondary bus under them, more
	// probing is needed before giving it control of more buses
	bridge->secondary_bus = parent_subordinate + 1;
	bridge->subordinate_bus = parent_subordinate + 1;
	pci_config_write8(bridge, PCI_CONFIG_SECONDARY_BUS_NUMBER, bridge->secondary_bus);
	pci_config_write8(bridge, PCI_CONFIG_SUBORDINATE_BUS_NUMBER, bridge->subordinate_bus);

	// Walk the chain of bridges up to the host bridge, bumping everyone's
	// subordinate bus number so that they can see the new bus
	while(parent != NULL) {
		// If the subordinate buses don't match going all the way up the chain,
		// then someone is allocating buses out of order, which can lead to overlap
		ASSERT(parent->subordinate_bus == parent_subordinate);

		parent->subordinate_bus = parent_subordinate + 1;

		// Host bridges don't have PCI2PCI bridge registers, but everyone else should
		if (parent->bridge != NULL) {
			dprintf(PDBG_MAP, "pci(%s): setting bridge bus range %02x:%02x\n",
				parent->name, parent->secondary_bus, parent->subordinate_bus);
			pci_config_write8(parent, PCI_CONFIG_SUBORDINATE_BUS_NUMBER, parent->subordinate_bus);
		}

		parent = parent->bridge;
	}

	// Now that we've claimed our secondary bus, register as its owner
	pci_register_bridge(bridge);
}

void pci_bridge_serr_enable(pci_device_t bridge, bool enable)
{
	uint16_t v = pci_config_read16(bridge, PCI_CONFIG_COMMAND);

	if (enable)
		v |= PCI_CMD_SERR_ENABLE;
	else
		v &= ~PCI_CMD_SERR_ENABLE;

	pci_config_write16(bridge, PCI_CONFIG_COMMAND, v);

}

void pci_free(pci_device_t dev)
{
	unsigned i;

	for (i = 0; i < sizeof(dev->devices) / sizeof(dev->devices[0]); i++) {
		if (dev->devices[i] != NULL)
			panic("Freeing PCI bridge \"%s\" before its %uth child \"%s\"",
				dev->name, i, dev->devices[i]->name);
	}

	if (dev->bridge != NULL) {
		dev->bridge->devices[dev->device_num] = NULL;
		dev->bridge->num_children--;
		// If we just killed the last child of the host bridge, reset the
		// allocation trackers in the host bridge. This allows us to reconfigure
		// the PCIe system by tearing down all devices and then reprobing from
		// the host bridge down. We don't support more complicated reconfiguration.
		if (dev->bridge->bridge == NULL && dev->bridge->num_children == 0) {
			pci_reinitialize_host_bridge(dev->bridge);
		}

	}

	if (pci_is_bridge(dev) && dev->bridge != NULL)
		pci_unregister_bridge(dev);

	free(dev);
}

uint8_t pci_config_read8(pci_device_t dev, uint16_t offset)
{
	pci_device_t bridge;
	bridge = dev->bridge;
	ASSERT(bridge != NULL);

	return pci_bridge_config_read8(bridge, dev->device_num, offset);
}

uint16_t pci_config_read16(pci_device_t dev, uint16_t offset)
{
	pci_device_t bridge;
	bridge = dev->bridge;
	ASSERT(bridge != NULL);

	return pci_bridge_config_read16(bridge, dev->device_num, offset);
}

uint32_t pci_config_read32(pci_device_t dev, uint16_t offset)
{
	pci_device_t bridge;
	bridge = dev->bridge;
	ASSERT(bridge != NULL);

	return pci_bridge_config_read32(bridge, dev->device_num, offset);
}

void pci_config_write8(pci_device_t dev, uint16_t offset, uint8_t value)
{
	pci_device_t bridge;
	bridge = dev->bridge;
	ASSERT(bridge != NULL);

	pci_bridge_config_write8(bridge, dev->device_num, offset, value);
}

void pci_config_write16(pci_device_t dev, uint16_t offset, uint16_t value)
{
	pci_device_t bridge;
	bridge = dev->bridge;
	ASSERT(bridge != NULL);

	pci_bridge_config_write16(dev->bridge, dev->device_num, offset, value);
}

void pci_config_write32(pci_device_t dev, uint16_t offset, uint32_t value)
{
	pci_device_t bridge;
	bridge = dev->bridge;
	ASSERT(bridge != NULL);

	pci_bridge_config_write32(bridge, dev->device_num, offset, value);
}

bool pci_is_bridge(pci_device_t dev)
{
	return dev->header_type == 1;
}

const char *pci_get_name(pci_device_t dev)
{
	ASSERT(dev != NULL);
	return dev->name;
}

void pci_set_name(pci_device_t dev, const char *name)
{
	ASSERT(strlen(name) < sizeof(dev->name));

	strlcpy(dev->name, name, sizeof(dev->name));
}

pci_device_t pci_get_bridge(pci_device_t dev)
{
	ASSERT(dev != NULL);
	return dev->bridge;
}

uint16_t pci_get_vendor_id(pci_device_t dev)
{
	ASSERT(dev != NULL);
	return dev->vendor_id;
}

uint16_t pci_get_device_id(pci_device_t dev)
{
	ASSERT(dev != NULL);
	return dev->device_id;
}

uint32_t pci_get_class_code(pci_device_t dev)
{
	ASSERT(dev != NULL);
	return dev->class_code;
}

uint8_t pci_get_revision_id(pci_device_t dev)
{
	ASSERT(dev != NULL);
	return dev->revision_id;
}

uint8_t pci_get_header_type(pci_device_t dev)
{
	ASSERT(dev != NULL);
	return dev->header_type;
}

void pci_memory_space_enable(pci_device_t dev, bool enable)
{
	uint16_t cmd = pci_config_read16(dev, PCI_CONFIG_COMMAND);

	if (enable)
		cmd |= PCI_CMD_MEMORY_SPACE_ENABLE;
	else
		cmd &= ~PCI_CMD_MEMORY_SPACE_ENABLE;

	pci_config_write16(dev, PCI_CONFIG_COMMAND, cmd);
}

void pci_bus_master_enable(pci_device_t dev, bool enable)
{
	uint16_t cmd = pci_config_read16(dev, PCI_CONFIG_COMMAND);

	if (enable)
		cmd |= PCI_CMD_BUS_MASTER_ENABLE;
	else
		cmd &= ~PCI_CMD_BUS_MASTER_ENABLE;

	pci_config_write16(dev, PCI_CONFIG_COMMAND, cmd);
}

uint64_t pci_get_bar_size(pci_device_t dev, unsigned bar_idx)
{
	ASSERT(dev != NULL);
	ASSERT(bar_idx < 6);
	ASSERT(dev->header_type == 0 || bar_idx < 2);
	return dev->bar_size[bar_idx];
}

// This function is recursive, but we shouldn't ever have a deep enough hierarchy to
// run into issues
// There is an assumption in this function that bridges never make allocations from their
// BARs. If that assumption is false, things could get dicey
static bool pci_bridge_expand_memory_space(pci_device_t bridge, uint64_t new_size)
{
	pci_device_t parent;
	uint64_t needed;
	uint64_t parent_starting_allocation;

	parent = bridge->bridge;

	needed = new_size - bridge->memory_size;

	if (parent != NULL)
		dprintf(PDBG_MAP, "pci(%s): expanding bridge memory space to 0x%llx (0x%llx more)\n",
		        bridge->name, new_size, needed);

	// Do a a sanity check to make sure the allocation amounts agree all
	// the way up to the host bridge. If a child bridge's base + size isn't equal to
	// its parent bridge's base + allocated, someone is doing allocations out
	// of order, and we're hosed. Of course, if nothing has been allocated on
	// this bridge, we're ok
	if (parent != NULL) {
		parent_starting_allocation = parent->memory_allocated;

	       	if (bridge->memory_size != 0) {
			uint64_t bridge_limit = bridge->memory_base + bridge->memory_size;
			uint64_t parent_limit = parent->memory_base + parent->memory_allocated;
			if (bridge_limit != parent_limit) {
				ASSERT(false /* out of order allocations */);
				return false;
			}
		}

		if (!pci_bridge_expand_memory_space(parent, parent->memory_size + needed))
			return false;
	} else if (parent == NULL) {
		// If we're at the host bridge, make sure it still has addresses left to vend
		if (bridge->memory_allocated + needed > bridge->memory_size) {
			ASSERT(false /* PCI address space exhausted */);
			return false;
		}

		// We're good, update the host bridge's counter of allocated memory
		bridge->memory_allocated = new_size;

		return true;
	}

	// Recursion above finished, we're now unwinding the stack. Everything looks reasonable,
	// so go ahead and expand the memory space size. We'll be doing this in reverse order from
	// the checks above, doing the host bridge first and walking back down the hierarchy.
	// By modifying parent->allocated and bridge->size, we make sure that we don't ever change
	// the size of the host bridge's memory_size (which is  not PCI-configurable).
	// It'll also make sure that there's a chunk that's unallocated in the deepest bridge (the
	// one for which this request was made to begin with)

	// But first, if we've never had an allocation, we need to set our base
	if (bridge->memory_base == 0) {
		bridge->memory_base = parent->memory_base + parent_starting_allocation;
		dprintf(PDBG_MAP, "pci(%s): setting bridge memory base 0x%x\n",
		        bridge->name, bridge->memory_base);
		pci_config_write16(bridge, PCI_CONFIG_MEMORY_BASE, (bridge->memory_base >> 16) & ~0xfU);
	}

	// Now update our limit to reflect the new size
	parent->memory_allocated += needed;
	bridge->memory_size += needed;

	dprintf(PDBG_MAP, "pci(%s): setting bridge memory range %08x:%08x\n",
		parent->name, bridge->memory_base, bridge->memory_base + bridge->memory_size - 1);

	pci_config_write16(bridge, PCI_CONFIG_MEMORY_LIMIT,
			((bridge->memory_base + bridge->memory_size - 1) >> 16) & ~0xfU);

	// Disable the prefetchable memory base/limit bridge memory range.
	pci_config_write16(bridge, PCI_CONFIG_PREFETCHABLE_BASE, 0xFFF0);
	pci_config_write16(bridge, PCI_CONFIG_PREFETCHABLE_LIMIT, 0x0000);

	return true;
}

uintptr_t pci_map_bar(pci_device_t dev, unsigned bar_idx)
{
	pci_device_t bridge;
	uint64_t pci_addr;
	uintptr_t host_addr;
	uint64_t bar_size;
	uint64_t allocated;
	uint32_t bar_offset;

	ASSERT(bar_idx < dev->num_bars);
	// 64-bit BARs can only be even-numbered
	if (dev->bar_64bit[bar_idx] && bar_idx % 2 != 0) {
		dprintf(DEBUG_CRITICAL, "pci(%s): trying to map odd portion %u of 64-bit bar\n",
			dev->name, bar_idx);
		return 0;
	}

	// BARs can only be mapped once
	ASSERT(dev->bar_host_addr[bar_idx] == 0);

	// Make sure the BAR is implemented
	bar_size = dev->bar_size[bar_idx];
	if (bar_size == 0) {
		dprintf(DEBUG_CRITICAL, "pci(%s): trying to map unimplemented bar %u\n",
			dev->name, bar_idx);
		return 0;
	}

	dprintf(PDBG_MAP, "pci(%s): trying to map bar %u size 0x%llx\n",
	        dev->name, bar_idx, bar_size);

	bridge = dev->bridge;

	// Figure out how much of the bridge's allocation is used. BARs mappings need to
	// be aligned to their size, so if the allocation isn't a multiple of this mapping's
	// size, we'll need to mark a bunch of address space as used to make us aligned again
	allocated = bridge->memory_allocated;
	if (((bar_size - 1) & allocated) != 0)
		allocated += bar_size - (allocated & (bar_size - 1));

	// Can we fit this BAR into the space we already have allocated?
	if (allocated + bar_size > bridge->memory_size) {
		uint64_t bridge_allocation;
		// Putain, we are out of space. We're going to need to ask our ancestors for
		// more space. Bridges allocate in 1MB chunks, so start by rounding up
		bridge_allocation = allocated + bar_size;
		bridge_allocation = (bridge_allocation + 0x100000 - 1) & ~(0x100000ULL - 1);

		dprintf(PDBG_MAP, "pci(%s): 0x%x memory space, 0x%x used (0x%llx aligned)\n",
			bridge->name, bridge->memory_size, bridge->memory_allocated, allocated);

		// Ask for our memory space size to be bumped up
		if (!pci_bridge_expand_memory_space(bridge, bridge_allocation)) {
			dprintf(DEBUG_CRITICAL, "pci(%s): couldn't allocate bridge memory space\n", bridge->name);
			return 0;
		}
	}

	// The bridge has enough memory space, so we'll tack this BAR at the end
	// of the allocated region
	pci_addr = bridge->memory_base + allocated;
	host_addr = platform_map_pci_to_host_addr(pci_addr);
	// And then mark the addresses we used as claimed
	bridge->memory_allocated = allocated + bar_size;

	dev->bar_pci_addr[bar_idx] = pci_addr;
	dev->bar_host_addr[bar_idx] = host_addr;

	dprintf(PDBG_MAP, "pci(%s): mapping bar %u at 0x%llx (host %p) size 0x%llx\n",
	        dev->name, bar_idx, pci_addr, (void *)host_addr, bar_size);

	bar_offset = PCI_CONFIG_BASE_ADDRESS0 + (4 * bar_idx);
	pci_config_write32(dev, bar_offset, (uint32_t)pci_addr);
	if (dev->bar_64bit[bar_idx])
		pci_config_write32(dev, bar_offset + 4, (uint32_t)(pci_addr >> 32));

	return host_addr;
}

uint16_t pci_find_capability(pci_device_t dev, uint16_t capability_id)
{
	uint16_t status;
	uint8_t cap_pointer;
	uint16_t id_and_next;
	uint8_t this_cap_id;

	// See if we've already cached the offset for a common capability
	if (capability_id == PCI_CAP_ID_PM && dev->pm_cap_offset != 0)
		return dev->pm_cap_offset;
	if (capability_id == PCI_CAP_ID_PCIE && dev->pcie_cap_offset != 0)
		return dev->pcie_cap_offset;

	if (dev->first_cap_offset == 0) {
		status = pci_config_read16(dev, PCI_CONFIG_STATUS);
		if (status == 0xFFFF || (status & PCI_STATUS_CAPABILITIES_LIST) == 0) {
			dprintf(DEBUG_INFO, "pci(%s): no capability list\n", dev->name);
			return 0;
		}

		cap_pointer = pci_config_read8(dev, PCI_CONFIG_CAPABILITIES_PTR);
		cap_pointer &= 0xFC;

		if (cap_pointer >= 0x40 && cap_pointer != 0xfc)
			dev->first_cap_offset = cap_pointer;
	} else {
		cap_pointer = dev->first_cap_offset;
	}

	while (cap_pointer >= 0x40 && cap_pointer != 0xfc) {
		id_and_next = pci_config_read16(dev, cap_pointer);
		this_cap_id = (id_and_next & 0xFF);

		// Cache the offsets for common capabilities
		if (this_cap_id == PCI_CAP_ID_PM)
			dev->pm_cap_offset = cap_pointer;
		else if (this_cap_id == PCI_CAP_ID_PCIE)
			dev->pcie_cap_offset = cap_pointer;

		if (this_cap_id == capability_id)
			return cap_pointer;

		cap_pointer = (id_and_next >> 8) & 0xFC;
	}

	dprintf(DEBUG_INFO, "pci(%s): capability 0x%02x not found\n", dev->name, capability_id);

	return 0;
}

uint16_t pci_find_extended_capability(pci_device_t dev, uint16_t capability_id, uint8_t min_version)
{
	uint32_t header;
	uint16_t header_id;
	uint8_t header_version;
	uint16_t header_pointer;
	uint16_t next_pointer;

	header_pointer = 0x100;

	do {
		header = pci_config_read32(dev, header_pointer);
		if (header == 0 || header == 0xFFFFFFFF)
			break;

		header_id = header & 0xFFFF;
		header_version = (header >> 16) & 0xF;
		next_pointer = (header >> 20) & 0xFFC;

		dprintf(PDBG_PROBE, "found ecap %04x at %04x next %04x\n", header_id, header_pointer, next_pointer);

		if (header_id == capability_id && header_version >= min_version)
			return header_pointer;

		header_pointer = next_pointer;
	} while (header_pointer != 0);

	return 0;
}

void pci_set_powerstate(pci_device_t dev, uint8_t powerstate)
{
	uint16_t pme_cap;
	uint16_t pmcsr;

	pme_cap = pci_find_capability(dev, PCI_CAP_ID_PM);
	if (pme_cap != 0) {
		uint32_t offset = pme_cap + PCI_PM_CAP_PMCSR;

		pmcsr = pci_config_read16(dev, offset);
		pmcsr = (pmcsr & ~3) | (powerstate & 3);
		pci_config_write16(dev, offset, pmcsr);
	}
}

bool pci_enable_pcie_ltr(pci_device_t dev)
{
	uint32_t pcie_offset;
	uint32_t ltr_offset;
	uint32_t cap2 = 0;
	uint32_t control2;

	// The host bridge doesn't have any associated PCIe config space; it's just
	// an abstraction for the hardware that translates fabric requests into PCIe
	// requests. Return true here to end the recursion below
	if (dev->bridge == NULL)
		return true;

	pcie_offset = pci_find_capability(dev, PCI_CAP_ID_PCIE);
	if (pcie_offset == 0) {
		dprintf(DEBUG_INFO, "pci: %s: can't enable LTR on non-PCIe endpoint\n", dev->name);
		return false;
	}

	cap2 = pci_config_read32(dev, pcie_offset + PCI_PCIE_CAP_DEVICE_CAPABILITIES2);
	if ((cap2 & (1 << 11)) == 0) {
		dprintf(DEBUG_INFO, "pci: %s: LTR not supported\n", dev->name);
		return false;
	}

	if (dev->pcie_device_port_type == PCI_PCIE_TYPE_ENDPOINT ||
	    dev->pcie_device_port_type == PCI_PCIE_TYPE_UPSTREAM) {
		ltr_offset = pci_find_extended_capability(dev, PCI_EXT_CAP_ID_LTR, 1);
		if (ltr_offset == 0) {
			dprintf(DEBUG_INFO, "pci: %s: LTR capability not found\n", dev->name);
			return false;
		}
	} else {
		ltr_offset = 0;
	}

	if (!pci_enable_pcie_ltr(dev->bridge))
		return false;

	control2 = pci_config_read32(dev, pcie_offset + PCI_PCIE_CAP_DEVICE_CONTROL2);
	control2 |= (1 << 10);
	pci_config_write32(dev, pcie_offset + PCI_PCIE_CAP_DEVICE_CONTROL2, control2);

	// Set a reasonable default for the max LTR value. As far as we can tell, this
	// is a pretty meaningless parameter, so we're just hardcoding it for now until
	// there's an actual reason to set it to something specific. This capability
	// is only present in ports that face upstream
	if (ltr_offset != 0) {
		// scale of 32^4 = 1,048,576 ns and value of 8 works out to roughly 8 ms
		// See PCIe r3.0 section 6.18 and section 7.25
		uint16_t max_latency = 8 | (4 << 10);
		pci_config_write16(dev, ltr_offset + PCI_LTR_CAP_MAX_SNOOP_LATENCY, max_latency);
		pci_config_write16(dev, ltr_offset + PCI_LTR_CAP_MAX_NO_SNOOP_LATENCY, max_latency);
	}

	return true;
}

void pci_enable_pcie_aspm(pci_device_t dev)
{
	pci_device_t bridge_dev;
	uint32_t bridge_pcie_cap;
	uint32_t endpoint_pcie_cap;
	uint32_t bridge_link_cap;
	uint32_t endpoint_link_cap;
	uint32_t aspm_support;
	uint16_t bridge_link_control;
	uint16_t endpoint_link_control;

	bridge_dev = dev->bridge;

	bridge_pcie_cap = pci_find_capability(bridge_dev, PCI_CAP_ID_PCIE);
	endpoint_pcie_cap = pci_find_capability(dev, PCI_CAP_ID_PCIE);

	if (bridge_pcie_cap == 0 || endpoint_pcie_cap == 0) {
		dprintf(DEBUG_CRITICAL, "pci: %s: Can't enable ASPM, PCIe capability not found\n", dev->name);
		return;
	}

	bridge_link_cap = pci_config_read32(bridge_dev, bridge_pcie_cap + PCI_PCIE_CAP_LINK_CAPABILITIES);
	endpoint_link_cap = pci_config_read32(dev, endpoint_pcie_cap + PCI_PCIE_CAP_LINK_CAPABILITIES);

	// <rdar://problem/20988960> Force enable ASPM+L1SS when S3E doesn't advertise support
	// If we ever had to support endpoints other than S3E, we could conditionalize this on the
	// S3E VID/PID. For now, this hack is good enough.
	endpoint_link_cap |= (0x2 << 10);
	
	aspm_support = (bridge_link_cap >> 10) & (endpoint_link_cap >> 10) & 0x3;

	// ASPM must be supported at both ends of the link
	if (aspm_support == 0) {
		dprintf(DEBUG_CRITICAL, "pci: %s: Can't enable ASPM (bridge cap 0x%08x endpoint cap 0x%08x)\n",
				dev->name, bridge_link_cap, endpoint_link_cap);
		return;
	}

	if (aspm_support) {
		// PCIe requires us to enable L1 in the upstream port before enabling it downstream
		bridge_link_control = pci_config_read16(bridge_dev, bridge_pcie_cap + PCI_PCIE_CAP_LINK_CONTROL);
		bridge_link_control &= ~3;
		bridge_link_control |= aspm_support;
		pci_config_write16(bridge_dev, bridge_pcie_cap + PCI_PCIE_CAP_LINK_CONTROL, bridge_link_control);

		endpoint_link_control = pci_config_read16(dev, endpoint_pcie_cap + PCI_PCIE_CAP_LINK_CONTROL);
		endpoint_link_control &= ~3;
		endpoint_link_control |= aspm_support;
		pci_config_write16(dev, endpoint_pcie_cap + PCI_PCIE_CAP_LINK_CONTROL, endpoint_link_control);
	}
}

void pci_set_pcie_common_clock(pci_device_t dev)
{
	pci_device_t bridge;
	uint32_t bridge_offset;
	uint32_t dev_offset;
	uint32_t control;

	bridge = dev->bridge;
	if (bridge == NULL || bridge->bridge == NULL) {
		dprintf(DEBUG_CRITICAL, "pci: can't enable common clock on root port bridge\n");
		return;
	}

	bridge_offset = pci_find_capability(bridge, PCI_CAP_ID_PCIE);
	if (bridge_offset == 0) {
		dprintf(DEBUG_CRITICAL, "pci: no express capability upstream\n");
		return;
	}
	dev_offset = pci_find_capability(dev, PCI_CAP_ID_PCIE);
	if (dev_offset == 0) {
		dprintf(DEBUG_CRITICAL, "pci: no express capability on endpoint\n");
		return;
	}

	control = pci_config_read16(dev, dev_offset + PCI_PCIE_CAP_LINK_CONTROL);
	control |= (1 << 6);
	pci_config_write16(dev, dev_offset + PCI_PCIE_CAP_LINK_CONTROL, control);

	control = pci_config_read16(bridge, bridge_offset + PCI_PCIE_CAP_LINK_CONTROL);
	control |= (1 << 6);
	pci_config_write16(bridge, bridge_offset + PCI_PCIE_CAP_LINK_CONTROL, control);
}

void pci_enable_pcie_cpm(pci_device_t dev)
{
	uint32_t pcie_cap;
	uint16_t link_control;

	if (dev->bridge == NULL || dev->bridge->bridge == NULL) {
		dprintf(DEBUG_CRITICAL, "pci: %s: can't enable CPM on root port bridge\n", dev->name);
		return;
	}

	pcie_cap = pci_find_capability(dev, PCI_CAP_ID_PCIE);

	if (pcie_cap == 0) {
		dprintf(DEBUG_CRITICAL, "pci: %s: Can't enable CPM, PCIe capability not found\n", dev->name);
		return;
	}

	link_control = pci_config_read16(dev, pcie_cap + PCI_PCIE_CAP_LINK_CONTROL);
	link_control |= (1 << 8);
	pci_config_write16(dev, pcie_cap + PCI_PCIE_CAP_LINK_CONTROL, link_control);
}

void pci_enable_pcie_l1ss(pci_device_t dev)
{
	pci_device_t bridge;
	uint32_t bridge_offset;
	uint32_t dev_offset;
	uint32_t bridge_cap;
	uint32_t dev_cap;
	uint32_t supported;
	uint32_t dev_t_power_on_scale;
	uint32_t dev_t_power_on_value;
	uint32_t bridge_t_common_mode;
	uint32_t dev_t_common_mode;
	uint32_t t_common_mode;
	uint32_t ltr_threshold;
	uint32_t bridge_control1;
	uint32_t bridge_control2;
	uint32_t dev_control1;
	uint32_t dev_control2;

	if (dev->pcie_device_port_type != PCI_PCIE_TYPE_ENDPOINT &&
	    dev->pcie_device_port_type != PCI_PCIE_TYPE_UPSTREAM) {
		dprintf(DEBUG_CRITICAL, "pci: %s: can't get L1SS parameters for non-upstream port\n", dev->name);
		return;
	}

	bridge = dev->bridge;
	if (bridge == NULL || bridge->bridge == NULL) {
		dprintf(DEBUG_CRITICAL, "pci: %s: can't enable L1SS on root port bridge\n", dev->name);
		return;
	}

	bridge_offset = pci_find_extended_capability(bridge, PCI_EXT_CAP_ID_L1SS, 1);
	if (bridge_offset == 0) {
		dprintf(DEBUG_CRITICAL, "pci: %s: no L1SS capability upstream\n", dev->name);
		return;
	}
	dev_offset = pci_find_extended_capability(dev, PCI_EXT_CAP_ID_L1SS, 1);
	if (dev_offset == 0) {
		dprintf(DEBUG_CRITICAL, "pci: %s: no L1SS capability on endpoint\n", dev->name);
		return;
	}

	// Make sure upstream and downstream support L1 PM substates, and that they have
	// supported substates in common
	bridge_cap = pci_config_read32(bridge, bridge_offset + PCI_L1SS_CAP_CAPABILITIES);
	dev_cap = pci_config_read32(dev, dev_offset + PCI_L1SS_CAP_CAPABILITIES);

	// <rdar://problem/20988960> Force enable ASPM+L1SS when S3E doesn't advertise support
	// If we ever had to support endpoints other than S3E, we could conditionalize this on the
	// S3E VID/PID. For now, this hack is good enough.
	dev_cap |= 0x1f;

	supported = bridge_cap & dev_cap & 0x1f;

	// L1 substates ECN, section 7.xx.3: "For compatibility with possible future extensions,
	// software must not enable L1 PM Substates unless the L1 PM Substates Supported bit in
	// the L1 PM Substates Capabilities Register is Set."
	// So, we need to make sure that bit 4 is set on both ends, and that some common bits
	// are set in bits 0-3
	if ((supported & 0x10) == 0 || (supported & 0xf) == 0) {
		dprintf(DEBUG_CRITICAL, "pci: %s: no common L1SS support (dev %08x upstream %08x)\n",
			dev->name, dev_cap, bridge_cap);
		return;
	}

	// Bit 4 just says if L1 is supported in general, but bits 0-3 map directly to control register bits
	supported &= 0xf;

	dev_t_power_on_scale = (dev_cap >> 16) & 0x3;
	dev_t_power_on_value = (dev_cap >> 19) & 0x1f;
	// t_power_on scale needs to be valid on endpoint
	if (dev_t_power_on_scale == 3) {
		dprintf(DEBUG_CRITICAL, "pci: %s: invalid L1SS t_power_on (dev %08x upstream %08x)\n",
			dev->name, dev_cap, bridge_cap);
		return;
	}

	// The root complex value isn't correct, so we hardcode the t_common_mode value in iBoot/SecureROM
	bridge_t_common_mode = platform_get_pcie_l1ss_t_common_mode();
	dev_t_common_mode = (dev_cap >> 8) & 0xff;
	t_common_mode = __max(bridge_t_common_mode, dev_t_common_mode);

	ltr_threshold = platform_get_pcie_l1ss_ltr_threshold();

	// L1SS must be disabled before setting timing parameters
	// Disable happens endpoint-first
	dev_control1 = pci_config_read32(dev, dev_offset + PCI_L1SS_CAP_CONTROL1);
	dev_control1 &= ~0xf;
	pci_config_write32(dev, dev_offset + PCI_L1SS_CAP_CONTROL1, dev_control1);
	bridge_control1 = pci_config_read32(bridge, bridge_offset + PCI_L1SS_CAP_CONTROL1);
	bridge_control1 &= ~0xf;
	pci_config_write32(bridge, bridge_offset + PCI_L1SS_CAP_CONTROL1, bridge_control1);

	// set control2 on bridge with t_power_on
	bridge_control2 = pci_config_read32(bridge, bridge_offset + PCI_L1SS_CAP_CONTROL2);
	bridge_control2 &= ~((0x3 << 0) | (0x1f << 3));
	// t_power_on_scale from endpoint
	bridge_control2 |= dev_t_power_on_scale;
	bridge_control2 |= dev_t_power_on_value << 3;
	pci_config_write32(bridge, bridge_offset + PCI_L1SS_CAP_CONTROL2, bridge_control2);

	// set control2 on endpoint with t_power_on
	dev_control2 = pci_config_read32(dev, dev_offset + PCI_L1SS_CAP_CONTROL2);
	dev_control2 &= ~((0x3 << 0) | (0x1f << 3));
	// t_power_on_scale from endpoint
	dev_control2 |= dev_t_power_on_scale;
	dev_control2 |= dev_t_power_on_value << 3;
	pci_config_write32(dev, dev_offset + PCI_L1SS_CAP_CONTROL2, dev_control2);

	// set control 1 on root port with t_common_mode, enables and LTR threshold
	// enables always need to happen upstream first (opposite of disable)
	bridge_control1 |= supported;
	if ((supported & ((1 << 3) | (1 << 1))) != 0) {
		// t_common_mode and LTR threshold only applies when L1.2 is supported
		bridge_control1 &= ~((0xff << 8) | (0x3ff << 16) | (0x7 << 29));
		bridge_control1 |= t_common_mode << 8;

		// set LTR threshold using a 1024 ns scale
		bridge_control1 |= ltr_threshold << 16;
		bridge_control1 |= 2 << 29;
	}
	pci_config_write32(bridge, bridge_offset + PCI_L1SS_CAP_CONTROL1, bridge_control1);

	// set control 1 on endpoint with enables and LTR threshold
	dev_control1 |= supported;
	if ((supported & ((1 << 3) | (1 << 1))) != 0) {
		// LTR threshold only applies when L1.2 is supported
		dev_control1 &= ~((0x3ff << 16) | (0x7 << 29));

		// set LTR threshold using a 1024 ns scale
		dev_control1 |= ltr_threshold << 16;
		dev_control1 |= 2 << 29;
	}
	pci_config_write32(dev, dev_offset + PCI_L1SS_CAP_CONTROL1, dev_control1);
}

#if DEBUG_BUILD
static void pci_explore(pci_device_t bridge, bool map_bars)
{
	pci_device_t dev;

	ASSERT(bridge != NULL);

	for (int i = 0; i < 32; i++) {
		dev = pci_bridge_probe(bridge, i, 0);
		if (dev == NULL) {
			continue;
		}

		if (map_bars) {
			for (int j = 0; j < dev->num_bars; j++) {
				if (dev->bar_size[j] != 0 && dev->bar_host_addr[j] == 0) {
					pci_map_bar(dev, j);
				}

				if (dev->bar_64bit[j])
					j++;
			}
		}

		if (pci_is_bridge(dev)) {
			if (dev->secondary_bus == 0)
				pci_bridge_assign_secondary_bus(dev);
			pci_explore(dev, map_bars);
		}
	}
}

static void pci_show_device(pci_device_t dev)
{
	if (dev->bridge == NULL) {
		printf("Host bridge\n");
		return;
	}

	printf("Device %s (%02x:%02x:%02x)\n", dev->name, dev->bridge->secondary_bus, dev->device_num, 0);

	printf("    Vendor:%04x Device:%04x Revision:%02x ClassCode:%06x Type:%02x\n",
		dev->vendor_id, dev->device_id, dev->revision_id, dev->class_code, dev->header_type);

	if (pci_is_bridge(dev)) {
		printf("    PriBus:%02x SecBus:%02x SubBus:%02x\n",
		       dev->primary_bus, dev->secondary_bus, dev->subordinate_bus);
		printf("    MemoryBase:%08x MemoryLimit:%08x (FIXME)\n", // FIXME
			dev->memory_base, dev->memory_base + dev->memory_size - 1);
	}

	for (uint32_t i = 0; i < dev->num_bars; i++) {
		const char *format_str;
	       
		if (dev->bar_size[i] == 0)
			continue;
		if (dev->bar_64bit[i] && i % 2 != 0)
			continue;

		if (dev->bar_64bit[i])
			format_str = "    BAR%u MEM %016llx/%016llx (host 0x%llx)\n";
		else
			format_str = "    BAR%u MEM %08llx/%08llx (host 0x%llx)\n";

		printf(format_str, i, dev->bar_pci_addr[i], dev->bar_size[i], dev->bar_host_addr[i]);
	}
}

static void pci_list_capabilities(pci_device_t dev)
{
	uint32_t idreg;
	uint16_t status;
	uint8_t cap_pointer;
	uint16_t id_and_next;

	idreg = pci_config_read32(dev, PCI_CONFIG_VENDOR_ID);
	if (idreg == 0xffffffff || idreg == 0xffff0001) {
		printf("Device not present\n");
		return;
	}

	status = pci_config_read16(dev, PCI_CONFIG_STATUS);
	if ((status & (1 << 4)) == 0) {
		printf("Device has no capabilties list (status 0x%04x)\n", status);
		return;
	}

	cap_pointer = pci_config_read8(dev, PCI_CONFIG_CAPABILITIES_PTR);
	cap_pointer &= 0xFC;

	while (cap_pointer >= 0x40) {
		id_and_next = pci_config_read16(dev, cap_pointer);

		printf("    Capability 0x%02x Offset:0x%02x\n", id_and_next & 0xFF, cap_pointer);

		cap_pointer = (id_and_next >> 8) & 0xFC;
	}
}

static void pci_list_extended_capabilities(pci_device_t dev)
{
	uint32_t idreg;
	uint32_t header;
	uint16_t header_id;
	uint8_t header_version;
	uint16_t header_pointer;

	idreg = pci_config_read32(dev, PCI_CONFIG_VENDOR_ID);
	if (idreg == 0xffffffff || idreg == 0xffff0001) {
		printf("Device not present\n");
		return;
	}

	header_pointer = 0x100;

	do {
		header = pci_config_read32(dev, header_pointer);
		if (header == 0 || header == 0xFFFFFFFF)
			break;

		header_id = header & 0xFFFF;
		header_version = (header >> 16) & 0xF;

		printf("    Extended Capability 0x%04x Ver:0x%02x Offset:0x%04x\n",
		       header_id, header_version, header_pointer);

		header_pointer = (header >> 20) & 0xFFC;
	} while (header_pointer != 0);
}

static void pci_list(bool verbose)
{
	pci_device_t dev;

	for (int i = 0; i < 256; i++) {
		if (pci_bridges[i] == NULL)
			continue;
		for(int j = 0; j < 32; j++) {
			dev = pci_bridges[i]->devices[j];
			if (dev != NULL) {
				pci_show_device(dev);
				if (verbose) {
					pci_list_capabilities(dev);
					pci_list_extended_capabilities(dev);
				}
			}
		}
	}
}

static pci_device_t pci_lookup_device(char *address)
{
	char *rest;
	char *next;
	long bus, device, function;

	bus = strtol(address, &rest, 16);

	if (rest == address || rest[0] != ':')
		return NULL;
	if (bus < 0 || bus > 255)
		return NULL;

	next = rest + 1;

	device = strtol(next, &rest, 16);
	if (rest == address || rest[0] != ':')
		return NULL;
	if (device < 0 || device > 31)
		return NULL;

	next = rest + 1;

	function = strtol(next, &rest, 16);
	if (rest == address || rest[0] != '\0')
		return NULL;
	if (function != 0)
		return NULL;

	if (pci_bridges[bus] == NULL)
		return NULL;

	return pci_bridges[bus]->devices[device];
}

static int do_pci_debug_cmd(int argc, struct cmd_arg *args)
{
	int result = -1;
	const char *cmd = NULL;
       
	if (argc >= 2)
		cmd = args[1].str;

	if (cmd == NULL) {
		result = -1;
	} else {
		if (strcmp(cmd, "enumerate") == 0) {
			bool map_bars = false;
			if (argc >= 3 && args[2].str != NULL) {
				if (strcmp(args[2].str, "-map") == 0)
					map_bars = true;
			}

			if (pci_bridges[0] == NULL) {
				printf("Can't find host bridge\n");
				result = -1;
			} else {
				pci_explore(pci_bridges[0], map_bars);
				result = 0;
			}
		} else if (strcmp(cmd, "list") == 0) {
			bool verbose = false;
			if (argc >= 3 && args[2].str != NULL) {
				if (strcmp(args[2].str, "-v") == 0)
					verbose = true;
			}
			pci_list(verbose);
			result = 0;
		} else if (strcmp(cmd, "aspm") == 0 && argc == 3) {
			pci_device_t dev = pci_lookup_device(args[2].str);
			if (dev == NULL) {
				printf("Couldn't find device \"%s\"\n", args[2].str);
				result = -1;
			} else {
				pci_enable_pcie_aspm(dev);
				result = 0;
			}
		} else if (strcmp(cmd, "cpm") == 0 && argc == 3) {
			pci_device_t dev = pci_lookup_device(args[2].str);
			if (dev == NULL) {
				printf("Couldn't find device \"%s\"\n", args[2].str);
				result = -1;
			} else {
				pci_enable_pcie_cpm(dev);
				result = 0;
			}
		} else if (strcmp(cmd, "l1ss") == 0 && (argc == 3)) {
			pci_device_t dev = pci_lookup_device(args[2].str);
			if (dev == NULL) {
				printf("Couldn't find device \"%s\"\n", args[2].str);
				result = -1;
			} else {
				pci_enable_pcie_l1ss(dev);
				result = 0;
			}
		} else if(strcmp(cmd, "ltr") == 0 && argc == 3) {
			pci_device_t dev = pci_lookup_device(args[2].str);
			if (dev == NULL) {
				printf("Couldn't find device \"%s\"\n", args[2].str);
				result = -1;
			} else {
				pci_enable_pcie_ltr(dev);
				result = 0;
			}
		}
	}

	if (result < 0) {
		printf("Usage:\n");
		printf("\taspm <dev>\t\t - Enables PCIe ASPM on the given device (bb:dd:ff)\n");
		printf("\tcpm <dev>\t\t - Enables PCIe CPM on the given device (bb:dd:ff)\n");
		printf("\tenumerate [-map]\t - Enumerates every PCI device reachable from the host bridge\n");
		printf("\tlist [-v]\t\t - Lists all enumerated PCI devices\n");
		printf("\tl1ss <dev>\t - Enables PCIe L1SS on the given device (bb:dd:ff)\n");
		printf("\tltr <dev>\t\t - Enables PCIe LTR on the given device (bb:dd:ff) and all upstream devices\n");
	}

	return result;
}
#endif

MENU_COMMAND_DEBUG(pci, do_pci_debug_cmd, "PCI debug commands", NULL);
