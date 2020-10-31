/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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

#define USBPHY_UOTGTUNE1        (0x649)
#define USBPHY_UOTGTUNE2        (0x2FF3)

#define USBOTG_AHB_DMA_BURST	(DWCUSB_GAHBCFG_HBST_INCR16)

#endif /* ! __PLATFORM_USBCONFIG_H */
