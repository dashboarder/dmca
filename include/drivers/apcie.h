/*                                                                              
 * Copyright (c) 2012-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef APCIE_INIT_H
#define APCIE_INIT_H

#include <lib/devicetree.h>
#include <drivers/pci.h>
#include <sys/types.h>

// 10 second timeout when enabling a link
#define APCIE_ENABLE_TIMEOUT	10000000

struct apcie_link_config {
	uint32_t	perst_gpio;
	uint32_t	clkreq_gpio;
	unsigned int	dart_id;
	utime_t		t_perst_delay;
	utime_t		t_boot_to_perst;
	utime_t		t_refclk_to_perst;
};

struct apcie_link_status {
	bool		enabled;
};

void apcie_init(void);
void apcie_use_external_refclk(bool use);
void apcie_set_s3e_mode(bool reset_on_enable);

void apcie_config_read_raw(void *dst, uint32_t bdfo, int size);
void apcie_config_write_raw(const void *src, uint32_t bdfo, int size);

bool apcie_enable_link(uint32_t link);
void apcie_disable_link(uint32_t link);
void apcie_disable_all(void);
uint32_t apcie_get_link_enable_count(void);
void apcie_setup_root_port_bridge(uint32_t link, struct apcie_link_config *config);
void apcie_update_devicetree(DTNode *apcie_node);
pci_device_t apcie_get_port_bridge(uint32_t link);
void apcie_free_port_bridge(uint32_t link);

extern struct apcie_link_config platform_apcie_link_configs[];
extern struct apcie_link_status apcie_link_statuses[];
extern struct apcie_link_config *apcie_link_configs;

#endif

