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

#ifdef  APCIE_DEBUG_LEVEL
#define DEBUG_LEVEL APCIE_DEBUG_LEVEL
#endif

#include <debug.h>
#include <drivers/apcie.h>
#include <drivers/pci.h>
#include <lib/devicetree.h>
#include <platform.h>
#include <platform/apcie_regs.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/timer.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <platform/soc/hwregbase.h>
#include <sys.h>
#include <sys/menu.h>

struct apcie_link_status apcie_link_statuses[APCIE_NUM_LINKS];
struct apcie_link_config *apcie_link_configs;

static pci_device_t host_bridge;
static pci_device_t port_bridges[APCIE_NUM_LINKS];

void apcie_config_read_raw(void *dst, uint32_t bdfo, int size)
{
	uintptr_t addr = PCI_CONFIG_BASE | bdfo;
	uint32_t  value;

	dprintf(DEBUG_SPEW, "apcie_config_read[0x%lx, %d]", addr, size);

	switch (size) {
		case 1:
			*(uint8_t *)dst = *(volatile uint8_t *)addr;
			value = *(uint8_t *)dst;
			break;
		case 2:
			*(uint16_t *)dst = *(volatile uint16_t *)addr;
			value = *(uint16_t *)dst;
			break;
		case 4:
			*(uint32_t *)dst = *(volatile uint32_t *)addr;
			value = *(uint32_t *)dst;
			break;
		default:
			panic("apcie_config_read: invalid size %d", size);
	}

	dprintf(DEBUG_SPEW, " = 0x%x\n", value);
}

static void apcie_config_read(void *priv, void *dst, uint32_t bdfo, int size)
{
	struct apcie_link_status *link_status = (struct apcie_link_status *)priv;

	if (link_status == NULL && apcie_get_link_enable_count() == 0) {
		panic("apcie: config read with no links enabled");
		memset(dst, 0xff, size);
		return;
	}
	if (link_status != NULL && !link_status->enabled) {
		panic("apcie: config read from disabled link");
		memset(dst, 0xff, size);
		return;
	}

	apcie_config_read_raw(dst, bdfo, size);
}

void apcie_config_write_raw(const void *src, uint32_t bdfo, int size)
{
	uintptr_t addr = PCI_CONFIG_BASE | bdfo;
	switch (size) {
		case 1:
			dprintf(DEBUG_SPEW, "apcie_config_write[0x%lx, %d] = 0x%x\n",
				addr, size, *(uint8_t *)src);
			*(volatile uint8_t *)addr = *(uint8_t *)src;
			break;
		case 2:
			dprintf(DEBUG_SPEW, "apcie_config_write[0x%lx, %d] = 0x%x\n",
				addr, size, *(uint16_t *)src);
			*(volatile uint16_t *)addr = *(uint16_t *)src;
			break;
		case 4:
			dprintf(DEBUG_SPEW, "apcie_config_write[0x%lx, %d] = 0x%x\n",
				addr, size, *(uint32_t *)src);
			*(volatile uint32_t *)addr = *(uint32_t *)src;
			break;
		default:
			panic("apcie_config_read: invalid size %d", size);
	}
}

static void apcie_config_write(void *priv, const void *src, uint32_t bdfo, int size)
{
	struct apcie_link_status *link_status = (struct apcie_link_status *)priv;

	if (link_status == NULL && apcie_get_link_enable_count() == 0) {
		panic("apcie: config write with no links enabled");
	}
	if (link_status != NULL && !link_status->enabled) {
		panic("apcie: config write to disabled link");
	}

	apcie_config_write_raw(src, bdfo, size);
}

/* Configure the virtual PCI2PCI bridge associated with the port. */
void apcie_setup_root_port_bridge(uint32_t link, struct apcie_link_config *config)
{
	pci_device_t bridge;
	uint8_t header_type;

	dprintf(DEBUG_INFO, "apcie: Probing port bridge for link %d\n", link);

	ASSERT(port_bridges[link] == 0);

	bridge = pci_bridge_probe(host_bridge, link, 100);
	if (bridge == NULL) {
		panic("apcie: failed to probe port bridge on link %u\n", link);
	}

	header_type = pci_get_header_type(bridge);
	if (header_type != 1) {
		panic("PCI2PCI bridge has wrong header type %d (expected 1)", header_type);
	}

	pci_bridge_assign_secondary_bus(bridge);

	pci_bus_master_enable(bridge, true);
	pci_memory_space_enable(bridge, true);
	pci_bridge_serr_enable(bridge, true);

	port_bridges[link] = bridge;
}

void apcie_disable_all(void)
{
	for (int i = 0; i < APCIE_NUM_LINKS; i++)
		apcie_disable_link(i);
}

void apcie_init(void)
{
	uint32_t memory_base;
	// At some point, we may allow targets to override the PCI
	// configuration, but this will do for now
	apcie_link_configs = platform_apcie_link_configs;

	memory_base = platform_map_host_to_pci_addr(PCI_32BIT_BASE);
	host_bridge = pci_create_host_bridge("host bridge", 0, memory_base, PCI_32BIT_LEN,
			                     NULL, apcie_config_read, apcie_config_write);
}

pci_device_t apcie_get_port_bridge(uint32_t link)
{
	ASSERT(link < APCIE_NUM_LINKS);

	return port_bridges[link];
}

void apcie_free_port_bridge(uint32_t link)
{
	ASSERT(link < APCIE_NUM_LINKS);

	if (port_bridges[link] != NULL) {
		pci_free(port_bridges[link]);
		port_bridges[link] = NULL;
	}
}

static int do_apcie_debug_cmd(int argc, struct cmd_arg *args)
{
	int result = -1;
	int link;
	const char *cmd = args[1].str;

	if (cmd == NULL) {
		result = -1;
	} else {
		if (strcmp(cmd, "enable") == 0 && argc == 3) {
			link = args[2].n;
			if (link >= 0 && link < APCIE_NUM_LINKS) {
				apcie_enable_link(link);
				result = 0;
			}
		} else if (strcmp(cmd, "disable") == 0 && argc == 3) {
			link = args[2].n;
			if (link >= 0 && link < APCIE_NUM_LINKS) {
				apcie_disable_link(link);
				result = 0;
			}
		} else if (strcmp(cmd, "counters") == 0 && argc == 4) {
			link = args[3].n;
			if (strcmp(args[2].str, "read")  == 0 &&
			    link >= 0 && link < APCIE_NUM_LINKS) {
				if (apcie_link_statuses[link].enabled) {
					rAPCIE_COUNTER_COMMAND(link) = APCIE_COUNTER_ENABLE | APCIE_COUNTER_CAPTURE;
					printf("time_L0:\t%u\n", rAPCIE_COUNTER_VALUE(link, 0));
					printf("time_L1:\t%u\n", rAPCIE_COUNTER_VALUE(link, 1));
					printf("time_L1.1:\t%u\n", rAPCIE_COUNTER_VALUE(link, 2));
					printf("time_L1.2:\t%u\n", rAPCIE_COUNTER_VALUE(link, 3));
					printf("entry_L0:\t%u\n", rAPCIE_COUNTER_VALUE(link, 4));
					printf("entry_L1:\t%u\n", rAPCIE_COUNTER_VALUE(link, 5));
					printf("entry_L1.1:\t%u\n", rAPCIE_COUNTER_VALUE(link, 6));
					printf("entry_L1.2:\t%u\n", rAPCIE_COUNTER_VALUE(link, 7));
				} else {
					printf("link not enabled\n");
				}
				result = 0;
			} else if(strcmp(args[2].str, "reset") == 0 &&
			          link >= 0 && link < APCIE_NUM_LINKS) {
				if (apcie_link_statuses[link].enabled) {
					rAPCIE_COUNTER_COMMAND(link) = APCIE_COUNTER_ENABLE | APCIE_COUNTER_CLEAR;
				} else {
					printf("link not enabled\n");
				}
				result = 0;
			}
				  
		}
	}

	if (result < 0) {
		printf("Usage:\n");
		printf("\tenable <link>\t\t - Enables the specified link\n");
		printf("\tdisable <link>\t\t - Disables the specified link\n");
		printf("\tcounters read <link>\t - Reads counters for the specified link\n");
		printf("\tcounters reset <link>\t - Resets counters for the specified link\n");
	}

	return result;
}

MENU_COMMAND_DEBUG(apcie, do_apcie_debug_cmd, "APCIe debug commands", NULL);
