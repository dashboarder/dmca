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

#ifndef __SAMSUNG_USBPHY_H
#define __SAMSUNG_USBPHY_H

#include <platform/soc/hwregbase.h>
#include <sys/types.h>
#include <drivers/usbphy.h>

// HW-OTG control registers
#define USB_PCGCCTL			((volatile u_int32_t *)(USBOTG_BASE_ADDR + 0x0E00))

#define PCGCCTL_PhySuspended		(1UL << 4)
#define PCGCCTL_RstPdwnModule		(1UL << 3)
#define PCGCCTL_PwrClmp			(1UL << 2)
#define PCGCCTL_GateHclk		(1UL << 1)
#define PCGCCTL_StopPclk		(1UL << 0)

// HS-OTG Phy control registers
#define USB_PHYPWR			((volatile u_int32_t *)(USBPHY_BASE_ADDR + 0x0000))
#define USB_PHYCON			((volatile u_int32_t *)(USBPHY_BASE_ADDR + 0x0004))
#define USB_URSTCON			((volatile u_int32_t *)(USBPHY_BASE_ADDR + 0x0008))
#define USB_UVLDCON			((volatile u_int32_t *)(USBPHY_BASE_ADDR + 0x001C))
#define USB_UCONDET			((volatile u_int32_t *)(USBPHY_BASE_ADDR + 0x0028))
#define USB_URESCON			((volatile u_int32_t *)(USBPHY_BASE_ADDR + 0x0034))
#define USB_UOTGTUNE1			((volatile u_int32_t *)(USBPHY_BASE_ADDR + 0x0040))
#define USB_UOTGTUNE2			((volatile u_int32_t *)(USBPHY_BASE_ADDR + 0x0044))
#define USB_UPADCON			((volatile u_int32_t *)(USBPHY_BASE_ADDR + 0x0048))
#define USB_USBHOSTSET			((volatile u_int32_t *)(USBPHY_BASE_ADDR + 0x0060))
#define USB_URSV			((volatile u_int32_t *)(USBPHY_BASE_ADDR + 0x0100))

// <rdar://problem/10320827> Add support for multiple USB OTG controllers to iBoot
// <rdar://problem/10304507> G1 USBBPHY USB OTG2 bit assignments are totally inconsistent
#define USB_PHYPWR_SHIFT		5
#define USB_PHYCON_SHIFT		5
#define USB_URSTCON_SHIFT		5
#define USB_UVLDCON_SHIFT		9
#define USB_UCONDET_SHIFT		10
#define USB_UOTGTUNE1_SHIFT		11
#define USB_UOTGTUNE2_SHIFT		13
#define USB_UPADCON_SHIFT		3

#endif /* ! __SAMSUNG_USBPHY_H */
