/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __AUSB_H
#define __AUSB_H

#include <platform/soc/hwregbase.h>

#define rAUSB_USB20PHY_CTL		(*(volatile uint32_t *)(AUSB_CTL_REG_BASE_ADDR + AUSB_CTL_USB20PHY_REG_OFFSET + 0x00))
#define rAUSB_USB20PHY_OTGSIG		(*(volatile uint32_t *)(AUSB_CTL_REG_BASE_ADDR + AUSB_CTL_USB20PHY_REG_OFFSET + 0x04))
#define rAUSB_USB20PHY_CFG0		(*(volatile uint32_t *)(AUSB_CTL_REG_BASE_ADDR + AUSB_CTL_USB20PHY_REG_OFFSET + 0x08))
#define rAUSB_USB20PHY_CFG1		(*(volatile uint32_t *)(AUSB_CTL_REG_BASE_ADDR + AUSB_CTL_USB20PHY_REG_OFFSET + 0x0C))
#define rAUSB_USB20PHY_BATCTL		(*(volatile uint32_t *)(AUSB_CTL_REG_BASE_ADDR + AUSB_CTL_USB20PHY_REG_OFFSET + 0x10))
#define rAUSB_USB20PHY_TEST		(*(volatile uint32_t *)(AUSB_CTL_REG_BASE_ADDR + AUSB_CTL_USB20PHY_REG_OFFSET + 0x1C))


#if !AUSB_USB20PHY_ONLY_VERSION
#define rAUSB_WIDGET_OTG_QOS		(*(volatile uint32_t *)(AUSB_PL301_WIDGET_BASE_ADDR + 0x14))
#define rAUSB_WIDGET_OTG_CACHE		(*(volatile uint32_t *)(AUSB_PL301_WIDGET_BASE_ADDR + 0x18))
#define rAUSB_WIDGET_OTG_ADDR		(*(volatile uint32_t *)(AUSB_PL301_WIDGET_BASE_ADDR + 0x1C))
#define rAUSB_WIDGET_EHCI0_QOS		(*(volatile uint32_t *)(AUSB_PL301_WIDGET_BASE_ADDR + 0x34))
#define rAUSB_WIDGET_EHCI0_CACHE	(*(volatile uint32_t *)(AUSB_PL301_WIDGET_BASE_ADDR + 0x38))
#define rAUSB_WIDGET_EHCI0_ADDR		(*(volatile uint32_t *)(AUSB_PL301_WIDGET_BASE_ADDR + 0x3C))
#define rAUSB_WIDGET_OHCI0_QOS		(*(volatile uint32_t *)(AUSB_PL301_WIDGET_BASE_ADDR + 0x54))
#define rAUSB_WIDGET_OHCI0_CACHE	(*(volatile uint32_t *)(AUSB_PL301_WIDGET_BASE_ADDR + 0x58))
#define rAUSB_WIDGET_OHCI0_ADDR		(*(volatile uint32_t *)(AUSB_PL301_WIDGET_BASE_ADDR + 0x5C))
#define rAUSB_WIDGET_EHCI1_QOS		(*(volatile uint32_t *)(AUSB_PL301_WIDGET_BASE_ADDR + 0x74))
#define rAUSB_WIDGET_EHCI1_CACHE	(*(volatile uint32_t *)(AUSB_PL301_WIDGET_BASE_ADDR + 0x78))
#define rAUSB_WIDGET_EHCI1_ADDR		(*(volatile uint32_t *)(AUSB_PL301_WIDGET_BASE_ADDR + 0x7C))
#endif

// Errata bits
enum
{
	ERRATA_PHY_JITTER_IMPROVEMENT	= 1
};

#endif /* __AUSB_H */
