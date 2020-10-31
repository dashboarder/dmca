/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
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

#if SUB_PLATFORM_S8000
/*
 * per Maui Tunables Specification, r0.18
 */

#define USBPHY_OTGTUNE0		(0x37477BB3)
#define USBPHY_OTGTUNE0_AX	(0x37477BF3)	// Maui A0/A1 uses different tuning values than B0
#define USBPHY_OTGTUNE1		(0x20E0C)

#elif SUB_PLATFORM_S8001

/*
 * per Elba Tunables Specification, r0.23
 */

#define USBPHY_OTGTUNE0		(0x373723F3)
#define USBPHY_OTGTUNE1		(0x20E04)

#elif SUB_PLATFORM_S8003

/*
 * per Malta Tunables Specification, r0.31
 */

#define USBPHY_OTGTUNE0		(0x37377BC3)
#define USBPHY_OTGTUNE1		(0x20C04)

#else
#error "Unknown SUB_PLATFORM"
#endif

#define USBOTG_AHB_DMA_BURST	(DWCUSB_GAHBCFG_HBST_INCR16)

#ifdef WITH_TARGET_USB_CONFIG
#include <target/usbconfig.h>
#endif /* WITH_TARGET_USB_CONFIG */

#endif /* ! __PLATFORM_USBCONFIG_H */
