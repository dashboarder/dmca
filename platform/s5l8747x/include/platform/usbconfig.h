/*
 * Copyright (C) 2007-2009 Apple Inc. All rights reserved.
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

#define USBPHY_CLK_TYPE		(2) /* Use CLKCORE */

#define USBPHY_UOTGTUNE1	(0x347)
#define USBPHY_UOTGTUNE2	(0xE3F)

#define USBOTG_AHB_DMA_BURST	(DWCUSB_GAHBCFG_HBST_SINGLE)

#endif /* ! __PLATFORM_USBCONFIG_H */
