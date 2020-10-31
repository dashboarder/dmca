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
#ifndef __PLATFORM_USBCONFIG_H
#define __PLATFORM_USBCONFIG_H

#if SUB_PLATFORM_T8010
/*
 * per Cayman Tunables Specification, SPDS version 0.0.1
 */

#define USBPHY_OTGTUNE0		(0x37377BC3)
#define USBPHY_OTGTUNE1		(0x00020C04)

#else
#error "Unknown SUB_PLATFORM"
#endif

#define USBOTG_AHB_DMA_BURST	(DWCUSB_GAHBCFG_HBST_INCR16)

#endif /* ! __PLATFORM_USBCONFIG_H */
