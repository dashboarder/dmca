/*
 * Copyright (C) 2011-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <lib/devicetree.h>
#include <platform.h>
#include <platform/soc/hwclocks.h>
#include <platform/usbconfig.h>
#include <platform/soc/chipid.h>
#include <sys.h>

#include "ausb.h"

// USB Widget related constants 
#if !AUSB_USB20PHY_ONLY_VERSION
	#if APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC)
		static const uint32_t high_addr_bits = (1 << 8) | ((SDRAM_BASE >> 32) & 0xf);
	#elif APPLICATION_SECUREROM || (APPLICATION_IBOOT && WITH_DFU_MODE)
		static const uint32_t high_addr_bits = (1 << 8) | ((SRAM_BASE >> 32) & 0xf);
	#else 
		#error Not Supported 
	#endif
#endif

///////////////////////////////////////////////////////////
//
//   USB PHY Control
//
///////////////////////////////////////////////////////////

static bool usbphy_powered;

void usbphy_power_up(void)
{
	if (usbphy_powered)
		return;

	clock_gate(CLK_USB, true);
	clock_gate(CLK_USBCTLREG, true);
	clock_gate(CLK_USB_OTG, true);

#if !AUSB_USB20PHY_ONLY_VERSION
	// Set the USB OTG Widget register
	rAUSB_WIDGET_OTG_ADDR = high_addr_bits;
#endif

	//  * expects SIDDQ and VBUSVLDEXTSEL set to 1 out of reset.

	//  * program tuning values
	rAUSB_USB20PHY_CFG0 = USBPHY_OTGTUNE0;

#if defined(USBPHY_OTGTUNE1)
	rAUSB_USB20PHY_CFG1 = USBPHY_OTGTUNE1;
#endif

	// <rdar://problem/15495710> ECO_216: Add register control to FSEL of USB PiCOPHY to get better jitter performance
	// The bit USB20PHY_CFG1::PLLBTUNE needs to be set for Fiji revisions > A0 and all Capri revisions
#if defined(SUB_PLATFORM_T7000) || defined(SUB_PLATFORM_T7001)
	rAUSB_USB20PHY_CFG1 |= (1 << 2);
#endif
	
	// <rdar://problem/16373379> Fiji >= B1 and Capri >= A1 need to adjust USB20PHY_CFG1:PLLITUNE
#if defined(SUB_PLATFORM_T7000)
	if(platform_get_chip_revision() >= CHIP_REVISION_B1)
		rAUSB_USB20PHY_CFG1 |= (2 << 4);
#endif
#if defined(SUB_PLATFORM_T7001)
	if(platform_get_chip_revision() >= CHIP_REVISION_A1)
		rAUSB_USB20PHY_CFG1 |= (2 << 4);
#endif

	// Maui A0/A1 uses different tunable than >= B0
#if defined(SUB_PLATFORM_S8000)
	if(platform_get_chip_revision() <= CHIP_REVISION_A1)
		rAUSB_USB20PHY_CFG0 = USBPHY_OTGTUNE0_AX;
#endif

	//  * assert PHY_RESET
	rAUSB_USB20PHY_CTL |= (1 << 0);

	//  * clear analog power-down (SIDDQ) and otg-block power-down bits
	rAUSB_USB20PHY_CTL &= ~((1 << 3) | (1 << 2));

	//  * wait 10us delay
	spin(10);

	//  * de-assert PHY_RESET
	rAUSB_USB20PHY_CTL &= ~(1 << 0);

	//  * wait 1000us delay: This wait for PLL to be locked after POR.
	spin(1000);

	usbphy_powered = true;
}

void usbphy_enable_pullup(void)
{
	if (!usbphy_powered)
		return;
	
	// Enable D+ PU
	rAUSB_USB20PHY_OTGSIG |= (1 << 1);	
}

void usbphy_power_down(void)
{
	if (!usbphy_powered)
		return;

	//  * Assert PHY_RESET
	rAUSB_USB20PHY_CTL |= (1 << 0);
	
	// * disable D+ PU
	rAUSB_USB20PHY_OTGSIG &= ~(1 << 1);
	
	//  * wait 10us delay
	spin(10);
	
	//  * De-assert PHY_RESET
	rAUSB_USB20PHY_CTL &= ~(1 << 0);
	
	//  * wait 1000us delay: This wait for PLL to be locked after POR.
	spin(1000);

	// * set analog and otg block power down bits
	rAUSB_USB20PHY_CTL |= ((1 << 3) | (1 << 2));

	clock_gate(CLK_USB_OTG, false);
	clock_gate(CLK_USBCTLREG, false);
	clock_gate(CLK_USB, false);

	usbphy_powered = false;
}

bool usbphy_is_cable_connected(void)
{
	bool connected;

	if (!usbphy_powered) {
		clock_gate(CLK_USB, true);
		clock_gate(CLK_USBCTLREG, true);
	}

	connected = (rAUSB_USB20PHY_OTGSIG & (1 << 8)) != 0;

	if (!usbphy_powered) {
		clock_gate(CLK_USBCTLREG, false);
		clock_gate(CLK_USB, false);
	}

	return connected;
}

bool usbphy_set_dpdm_monitor(int select)
{
	// XXX IBOOT64_TODO
	return false;
}

#if WITH_DEVICETREE

void update_devicetree_property(DTNode *node, char * propName, uint32_t value, bool doOR)
{
	void *propData;
	uint32_t propSize;
	
	if (FindProperty(node, &propName, &propData, &propSize)) {
		if (propSize >= sizeof(uint32_t)) {
			if(doOR) {
				*(uint32_t *)propData |= value;
			}
			else {
				*(uint32_t *)propData = value;
			}
		}
	}
}

void usbphy_update_device_tree(DTNode *node)
{
	uint32_t phy_tuning_val;

	phy_tuning_val = 0;

#ifdef USBPHY_UOTGTUNE
	phy_tuning_val = USBPHY_UOTGTUNE;
#endif

	// Fill in the phy-tuning-val property
	update_devicetree_property(node, "phy-tuning-val", phy_tuning_val, false);

	// <rdar://problem/15495710> ECO_216: Add register control to FSEL of USB PiCOPHY to get better jitter performance
	// The bit USB20PHY_CFG1::PLLBTUNE needs to be set for Fiji revisions > A0 and all Capri revisions
#if defined(SUB_PLATFORM_T7000) || defined(SUB_PLATFORM_T7001)
	update_devicetree_property(node, "cfg1-device", (1 << 2), true);
	update_devicetree_property(node, "cfg1-host", (1 << 2), true);
#endif
	
	// <rdar://problem/16373379> Fiji >= B1 and Capri >= A1 need to adjust USB20PHY_CFG1:PLLITUNE
#if defined(SUB_PLATFORM_T7000)
	if(platform_get_chip_revision() >= CHIP_REVISION_B1) {
		update_devicetree_property(node, "cfg1-device", (2 << 4), true);
		update_devicetree_property(node, "cfg1-host", (2 << 4), true);
	}
#endif
#if defined(SUB_PLATFORM_T7001)
	if(platform_get_chip_revision() >= CHIP_REVISION_A1) {
		update_devicetree_property(node, "cfg1-device", (2 << 4), true);
		update_devicetree_property(node, "cfg1-host", (2 << 4), true);
	}
#endif

	// Maui A0/A1 uses different tunable than >= B0
#if defined(SUB_PLATFORM_S8000)
	if(platform_get_chip_revision() <= CHIP_REVISION_A1) {
		update_devicetree_property(node, "cfg0-device", USBPHY_OTGTUNE0_AX, false);
	}
#endif
}
#endif

///////////////////////////////////////////////////////////
//
//   USB Widgets Control
//
///////////////////////////////////////////////////////////

#if !AUSB_USB20PHY_ONLY_VERSION
void ausb_setup_widgets()
{
	clock_gate(CLK_USB, true);

	// USB_CTL clock should be enabled
	rAUSB_WIDGET_OTG_ADDR 	= high_addr_bits;
	rAUSB_WIDGET_OHCI0_ADDR	= high_addr_bits;

	// Only if EHCI is NOT capable of 64 bit addressing ...
#if (USBEHCI_ADDR_EXT_WIDGET_EN == 1)
	rAUSB_WIDGET_EHCI0_ADDR	= high_addr_bits;
	rAUSB_WIDGET_EHCI1_ADDR	= high_addr_bits;
#endif

	clock_gate(CLK_USB, false);
}
#endif
