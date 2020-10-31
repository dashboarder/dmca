/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#ifndef __DRIVERS_USBPHY_H
#define __DRIVERS_USBPHY_H

#include <sys/types.h>

#if WITH_DEVICETREE
#include <lib/devicetree.h>
#endif

__BEGIN_DECLS

void usbphy_power_up(void);
void usbphy_enable_pullup(void);
void usbphy_power_down(void);
bool usbphy_is_cable_connected(void);
bool usbphy_set_dpdm_monitor(int select);
#if WITH_CONJOINED_USB_PHYS
void usbphy_select_phy(int index);
#endif

#if WITH_DEVICETREE
extern void usbphy_update_device_tree(DTNode *pmgr_node);
#endif

__END_DECLS


#endif

