#ifndef PCI_PRIVATE_H
#define PCI_PRIVATE_H

#include <drivers/pci.h>

struct pci_device {
	char name[16];
	uint32_t refcount;

	// This device's number on its bus
	uint8_t device_num;

	// Cached copies of config space registers
	uint16_t vendor_id;
	uint16_t device_id;
	uint32_t class_code;
	uint8_t revision_id;
	uint8_t header_type;
	bool multifunction;

	// cached copied of pcie-specific registers
	uint8_t pcie_device_port_type; // from PCIe r3 7.8.2

	// Populated in pci_bridge_probe by probing the BARs
	uint8_t num_bars;
	uint64_t bar_size[6];
	bool bar_64bit[6];
	uint64_t bar_pci_addr[6];
	uintptr_t bar_host_addr[6];

	uint16_t first_cap_offset;
	uint16_t pcie_cap_offset;
	uint16_t pm_cap_offset;

	// The bridge upstream from this device, NULL for the host bridge
	pci_device_t bridge;

	////////// bridge-specific stuff
	// Provided by the platform driver to support config reads/writes
	pci_config_read_cb_t config_read;
	pci_config_write_cb_t config_write;
	void *bridge_priv;

	// Caches of the bridge numbering config space registers. For host bridges,
	// primary_bus is ignored and secondary/subordinate_bus just track which
	// bus numbers have been vended (since host bridges don't have config space)
	uint8_t primary_bus;
	uint8_t secondary_bus;
	uint8_t subordinate_bus;

	// Tracks 32-bit memory space allocations. For now the driver will just support
	// 32-bit addresses. 64-bit addresses would be tracked in a separate range, so
	// making the type 32-bit is safe
	uint32_t memory_base;
	uint32_t memory_size;
	uint32_t memory_allocated;

	// Keep track of any devices on this bridge's bus
	struct pci_device *devices[32];
	// And of how many we've allocated
	uint32_t num_children;
};


#endif
