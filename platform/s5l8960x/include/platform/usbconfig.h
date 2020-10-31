/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_USBCONFIG_H
#define __PLATFORM_USBCONFIG_H

/*
 * per Alcatraz Tunables 0.23:
 */
#define USBPHY_OTGTUNE0		(0x374493F3)

#define USBOTG_AHB_DMA_BURST	(DWCUSB_GAHBCFG_HBST_INCR16)

/* 
 * Do we need to extend EHCI DMA Address register with additional bits
 * as per AUSBPL301_Widgets Register Specification
 */
#define USBEHCI_ADDR_EXT_WIDGET_EN	1

#ifdef WITH_TARGET_USB_CONFIG
#include <target/usbconfig.h>
#endif /* WITH_TARGET_USB_CONFIG */

#endif /* ! __PLATFORM_USBCONFIG_H */
