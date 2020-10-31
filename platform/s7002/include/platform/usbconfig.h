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
#ifndef __PLATFORM_USBCONFIG_H
#define __PLATFORM_USBCONFIG_H

#ifdef WITH_TARGET_USB_CONFIG
#include <target/usbconfig.h>
#endif /* WITH_TARGET_USB_CONFIG */

#define USBPHY_OTGTUNE0		(0x374413F3)

#define USBOTG_AHB_DMA_BURST	(DWCUSB_GAHBCFG_HBST_INCR16)

#endif /* ! __PLATFORM_USBCONFIG_H */
