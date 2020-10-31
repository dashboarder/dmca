/*
 * Copyright (C) 2008-2009 Apple Inc. All rights reserved.
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

#define USBPHY_CLK_TYPE		(3) /* Use CLKCORE */

#if SUB_PLATFORM_S5L8940X
#define USBPHY_UOTGTUNE1        (0x349)
#define USBPHY_UOTGTUNE2        (0xF13)
#elif SUB_PLATFORM_S5L8942X | SUB_PLATFORM_S5L8947X
#define USBPHY_UOTGTUNE1        (0x349)
#define USBPHY_UOTGTUNE2        (0x6FF3)
#endif

#define USBOTG_AHB_DMA_BURST	(DWCUSB_GAHBCFG_HBST_INCR8)

#ifdef WITH_TARGET_USB_CONFIG
#include <target/usbconfig.h>
#endif /* WITH_TARGET_USB_CONFIG */

#endif /* ! __PLATFORM_USBCONFIG_H */
