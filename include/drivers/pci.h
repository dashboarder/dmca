/*
 * Copyright (C) 2008-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __DRIVERS_PCI_PCI_H
#define __DRIVERS_PCI_PCI_H

#include <sys.h>
#include <sys/types.h>

typedef void (*pci_config_read_cb_t)(void *priv, void *dst, uint32_t bdfo, int size);
typedef void (*pci_config_write_cb_t)(void *priv, const void *src, uint32_t bdfo, int size);

typedef struct pci_device * pci_device_t;

void pci_init(void);
void pci_free(pci_device_t dev);

// Config space accessors
uint8_t pci_config_read8(pci_device_t dev, uint16_t offset);
uint16_t pci_config_read16(pci_device_t dev, uint16_t offset);
uint32_t pci_config_read32(pci_device_t dev, uint16_t offset);
void pci_config_write8(pci_device_t dev, uint16_t offset, uint8_t value);
void pci_config_write16(pci_device_t dev, uint16_t offset, uint16_t value);
void pci_config_write32(pci_device_t dev, uint16_t offset, uint32_t value);

uint64_t pci_get_bar_size(pci_device_t dev, unsigned bar_idx);
pci_device_t pci_get_bridge(pci_device_t dev);
uint32_t pci_get_class_code(pci_device_t dev);
uint16_t pci_get_device_id(pci_device_t dev);
uint8_t pci_get_header_type(pci_device_t dev);
uint8_t pci_get_revision_id(pci_device_t dev);
uint16_t pci_get_vendor_id(pci_device_t dev);

uintptr_t pci_map_bar(pci_device_t dev, unsigned bar_idx);
void pci_bus_master_enable(pci_device_t dev, bool enable);
void pci_memory_space_enable(pci_device_t dev, bool enable);

// Capabilities
uint16_t pci_find_capability(pci_device_t dev, uint16_t capability_id);
uint16_t pci_find_extended_capability(pci_device_t dev, uint16_t capability_id, uint8_t min_version);

// Power management
void pci_set_powerstate(pci_device_t dev, uint8_t powerstate);
void pci_enable_pcie_aspm(pci_device_t dev);
void pci_enable_pcie_l1ss(pci_device_t dev);
bool pci_enable_pcie_ltr(pci_device_t dev);

// Random info
void pci_set_name(pci_device_t dev, const char *name);
const char *pci_get_name(pci_device_t dev);

// Bridges
bool pci_is_bridge(pci_device_t dev);
pci_device_t pci_create_host_bridge(const char *name, uint8_t bus_num, uint32_t memory_base, uint32_t memory_size, void *priv, pci_config_read_cb_t read_cb, pci_config_write_cb_t write_cb);
pci_device_t pci_bridge_probe(pci_device_t bridge, uint8_t device_num, utime_t timeout);
void pci_bridge_assign_secondary_bus(pci_device_t bridge);
void pci_bridge_serr_enable(pci_device_t dev, bool enable);

#define PCI_BUS_SHIFT 20
#define PCI_DEVICE_SHIFT 15
#define PCI_FUNCTION_SHIFT 12
#define PCI_BDFO(bus, device, function, offset) ((((uint32_t)bus) << PCI_BUS_SHIFT) | (((uint32_t)device) << PCI_DEVICE_SHIFT) | (((uint32_t)function) << PCI_FUNCTION_SHIFT) | (offset))
#define PCI_BDF(bus, device, function) (PCI_BDFO((bus), (device), (function), 0))
#define PCI_DFO(device, function, offset) (PCI_BDFO(0, (device), (function), (offset)))
#define PCI_DFO_MASK ((1 << PCI_BUS_SHIFT)-1)
#define PCI_DEVICE_MASK (0x1F << PCI_DEVICE_SHIFT)
#define PCI_FUNCTION_MASK (0x7 << PCI_FUNCTION_SHIFT)
#define PCI_BUS(bdfo) (bdfo >> PCI_BUS_SHIFT)
#define PCI_DEVICE(bdfo) (((bdfo) & PCI_DEVICE_MASK) >> PCI_DEVICE_SHIFT)
#define PCI_FUNCTION(bdfo) (((bdfo) & PCI_FUNCTION_MASK) >> PCI_FUNCTION_SHIFT)

#define PCI_CONFIG_VENDOR_ID			0x00
#define PCI_CONFIG_DEVICE_ID			0x02
#define PCI_CONFIG_COMMAND			0x04
#define PCI_CONFIG_STATUS			0x06
#define PCI_CONFIG_REVISION_ID			0x08
#define PCI_CONFIG_CLASS_CODE			0x09
#define PCI_CONFIG_CACHE_LINE_SIZE		0x0C
#define PCI_CONFIG_LATENCY_TIMER		0x0D
#define PCI_CONFIG_HEADER_TYPE			0x0E
#define PCI_CONFIG_BIST				0x0F
#define PCI_CONFIG_BASE_ADDRESS0		0x10
#define PCI_CONFIG_BASE_ADDRESS1		0x14
#define PCI_CONFIG_BASE_ADDRESS2		0x18
#define PCI_CONFIG_BASE_ADDRESS3		0x1C
#define PCI_CONFIG_BASE_ADDRESS4		0x20
#define PCI_CONFIG_BASE_ADDRESS5		0x24
#define PCI_CONFIG_CARDBUS_CIS_PTR		0x28
#define PCI_CONFIG_SUBSYSTEM_VENDOR_ID		0x2C
#define PCI_CONFIG_SUBSYSTEM_ID			0x2E
#define PCI_CONFIG_EXPANSION_ROM_BASE		0x30
#define PCI_CONFIG_CAPABILITIES_PTR		0x34
#define PCI_CONFIG_INTERRUPT_LINE		0x3C
#define PCI_CONFIG_INTERRUPT_PIN		0x3D
#define PCI_CONFIG_MINIMUM_GRANT		0x3E
#define PCI_CONFIG_MAXIMUM_LATENCY		0x3F

#define PCI_CONFIG_PRIMARY_BUS_NUMBER		0x18
#define PCI_CONFIG_SECONDARY_BUS_NUMBER		0x19
#define PCI_CONFIG_SUBORDINATE_BUS_NUMBER	0x1A
#define PCI_CONFIG_SECONDARY_LATENCY_TIMER	0x1B
#define PCI_CONFIG_IO_BASE			0x1C
#define PCI_CONFIG_IO_LIMIT			0x1D
#define PCI_CONFIG_SECONDARY_STATUS		0x1E
#define PCI_CONFIG_MEMORY_BASE			0x20
#define PCI_CONFIG_MEMORY_LIMIT			0x22
#define PCI_CONFIG_PREFETCHABLE_BASE		0x24
#define PCI_CONFIG_PREFETCHABLE_LIMIT		0x26
#define PCI_CONFIG_PREFETCHABLE_BASE_UPPER32	0x28
#define PCI_CONFIG_PREFETCHABLE_LIMIT_UPPER32	0x2C
#define PCI_CONFIG_IO_BASE_UPPER16		0x30
#define PCI_CONFIG_IO_LIMIT_UPPER16		0x32
#define PCI_CONFIG_BRIDGE_CONTROL		0x3E

#define PCI_CMD_IO_SPACE_ENABLE			(1 << 0)
#define PCI_CMD_MEMORY_SPACE_ENABLE		(1 << 1)
#define PCI_CMD_BUS_MASTER_ENABLE		(1 << 2) 
#define PCI_CMD_SERR_ENABLE			(1 << 8) 

#define PCI_BRIDGE_CTRL_SERR_ENABLE		(1 << 1)

#define PCI_STATUS_CAPABILITIES_LIST		(1 << 4)

#define PCI_CAP_ID_PM				(0x01)

#define PCI_PM_CAP_PMC				(0x02)
#define PCI_PM_CAP_PMCSR			(0x04)
#define PCI_PM_CAP_PMCSR_BSE			(0x06)
#define PCI_PM_CAP_DATA				(0x07)

#define PCI_PM_POWERSTATE_D0			(0x0)
#define PCI_PM_POWERSTATE_D3			(0x3)

// PCIe r3.0 section 7.8
#define PCI_CAP_ID_PCIE				(0x10)

#define PCI_PCIE_CAP_PCIE_CAPABILITIES		(0x02)
#define PCI_PCIE_CAP_LINK_CAPABILITIES		(0x0C)
#define PCI_PCIE_CAP_LINK_CONTROL		(0x10)
#define PCI_PCIE_CAP_DEVICE_CAPABILITIES2	(0x24)
#define PCI_PCIE_CAP_DEVICE_CONTROL2		(0x28)
#define PCI_PCIE_CAP_LINK_CAPABILITIES2		(0x2C)
#define PCI_PCIE_CAP_LINK_CONTROL2		(0x30)

// PCIe r3.0 section 7.8.2
#define PCI_PCIE_TYPE_ENDPOINT			(0)
#define PCI_PCIE_TYPE_ROOT_PORT			(4)
#define PCI_PCIE_TYPE_UPSTREAM			(5)
#define PCI_PCIE_TYPE_DOWNSTREAM		(6)
#define PCI_PCIE_TYPE_UNKNOWN			(UINT8_MAX) // for endpoints without a PCIe capability

// PCIe r3.0 section 7.8.6/7.8.18
#define PCI_PCIE_SPEED_GEN1			(0)
#define PCI_PCIE_SPEED_GEN2			(1)
#define PCI_PCIE_SPEED_GEN3			(2)

// PCIe r3.0 section 7.25
#define PCI_EXT_CAP_ID_LTR			(0x18)

#define PCI_LTR_CAP_MAX_SNOOP_LATENCY		(0x04)
#define PCI_LTR_CAP_MAX_NO_SNOOP_LATENCY	(0x06)

#define PCI_EXT_CAP_ID_L1SS			(0x1E)

#define PCI_L1SS_CAP_CAPABILITIES		(0x04)
#define PCI_L1SS_CAP_CONTROL1			(0x08)
#define PCI_L1SS_CAP_CONTROL2			(0x0c)

#endif
