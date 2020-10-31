/*
 * Copyright (C) 2007-2009 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <platform.h>
#include <platform/clocks.h>
#include <platform/power.h>
#include <platform/soc/hwclocks.h>
#include <platform/usbconfig.h>
#include <sys.h>

#include "usbphy.h"

static bool usb_phy_powered = false;

// <rdar://problem/10320827> Add support for multiple USB OTG controllers to iBoot
// <rdar://problem/10304507> G1 USBBPHY USB OTG2 bit assignments are totally inconsistent
#if WITH_CONJOINED_USB_PHYS

#if USBPHY_VERSION > 2
#error "Conjoined USB PHYs not compatible with PHY version > 2"
#endif

#define SELECTED_PHY (1)

#define SET_REG(reg, value) set_reg(reg, SELECTED_PHY, reg##_SHIFT, (value))
#define SET_REG_BITS(reg, value, mask) set_reg_bits(reg, SELECTED_PHY, reg##_SHIFT, (value), (mask))
#define GET_REG(reg) get_reg(reg, SELECTED_PHY, reg##_SHIFT)

static void set_reg(volatile u_int32_t *reg, int index, int shift, u_int32_t value)
{
	u_int32_t mask = ((1 << shift) - 1) << (shift * index);
	*reg = (*reg & ~mask) | (value << (shift * index));
}

static void set_reg_bits(volatile u_int32_t *reg, int index, int shift, u_int32_t value, u_int32_t mask)
{
	mask = mask << (shift * index);
	*reg = (*reg & ~mask) | (value << (shift * index));
}

static u_int32_t get_reg(volatile u_int32_t *reg, int index, int shift)
{
	return (*reg >> shift) & ((1 << shift) - 1);
}

#else

#define SET_REG(reg, value) *(reg) = (value)
#define SET_REG_BITS(reg, value, mask) *(reg) = (*(reg) & ~(mask)) | (value)
#define GET_REG(reg) *(reg)

#endif


static void gate_clocks(bool enable)
{
	if (enable) {
#if USBPHY_VERSION > 2
		clock_gate(CLK_UPERF, true);
#endif

		clock_gate(CLK_USBOTG, true);
		clock_gate(CLK_USBPHY, true);
	} else {
		clock_gate(CLK_USBOTG, false);
		clock_gate(CLK_USBPHY, false);

#if USBPHY_VERSION > 2
		clock_gate(CLK_UPERF, false);
#endif
	}
}


//---------------------------------------------------------------------------
// FUNCTION:    usbphy_power_up
//    
// DESCRIPTION: Does initial powering up to bring up the PHY 
//
// RETURN:      None 
//
// NOTES:       This should be one of the first things done upon startup. 
//---------------------------------------------------------------------------
void usbphy_power_up(void)
{
	u_int32_t clk_sel = 0;

	if (usb_phy_powered) return;

	platform_power_set_gate(PWRBIT_USB, true);
	spin(10*1000);

	gate_clocks(true);
    
	// Clear the StopPclk & GateHclk bits in the power and clock gating register
	*USB_PCGCCTL &= ~(PCGCCTL_StopPclk | PCGCCTL_GateHclk);

	spin(100);

#if USBPHY_VERSION > 1
	*USB_USBHOSTSET = (1 << 9); 			// enable hsic, this will cause STB and DAT to stay low
#endif

	switch (clock_get_frequency(CLK_USBPHYCLK)) {
	       case 12000000UL : clk_sel = 0; break;
	       case 24000000UL : clk_sel = 1; break;
	       case 48000000UL : clk_sel = 2; break;
	       default	       : clk_sel = 3; break;
	}

#if USBPHY_VERSION > 0
	SET_REG(USB_PHYPWR, (0UL<<4) |			// powerup analog block
			    (0UL<<3) |			// powerup OTG block
			    (USBPHY_CLK_TYPE << 1) |	// select the reference clock
			    (0UL<<0));	       		// disable suspend signal for power save 

	SET_REG(USB_UVLDCON, (1 << 2));	// Set vbus_vld_ext_sel

#else
	*USB_PHYPWR =   (0UL<<4) |			// powerup analog block
			(0UL<<3) |			// powerup XO block
			(USBPHY_CLK_TYPE << 2) |	// select the reference for XO block
			(0UL<<1) |	     		// powerup PLL
			(0UL<<0);			// disable suspend signal for power save 

	/* Correct the field assinment for version 0 */
	clk_sel ^= 1;
#endif

#ifdef USBPHY_UOTGTUNE1
	SET_REG(USB_UOTGTUNE1, USBPHY_UOTGTUNE1);
#endif
#ifdef USBPHY_UOTGTUNE2
	SET_REG(USB_UOTGTUNE2, USBPHY_UOTGTUNE2);
#endif

	spin(10);

#if WITH_CONJOINED_USB_PHYS
	// <rdar://problem/10491915> G1: Implement G1-specific USB PHY settings
	*USB_URSV |= (1 << 4);
#endif

	SET_REG_BITS(USB_PHYCON, clk_sel, 0x3);

	// reset the PHY
	SET_REG_BITS(USB_URSTCON, 1, 1);
	spin(20); // need to assert for at least 10us
	SET_REG_BITS(USB_URSTCON, 0, 1);
	spin(1000); // need to wait for at least 805us

	usb_phy_powered = true;
}

void usbphy_enable_pullup(void)
{
	if (!usb_phy_powered)
		return;
	
	// Enable D+ PU
	SET_REG_BITS(USB_UVLDCON, (1 << 1), (1 << 1));	// Set vbus_vld_ext
}

//---------------------------------------------------------------------------
// FUNCTION:    usbphy_power_down
//    
// DESCRIPTION: Powers down the PHY upon USB shutdown 
//
// RETURN:      None 
//
// NOTES:       This should be one of the last things done upon shutdown. 
//---------------------------------------------------------------------------
void usbphy_power_down(void)
{
	platform_power_set_gate(PWRBIT_USB, true);
	gate_clocks(true);

	// set bits 0-4 to 1 to power down the PHY
	SET_REG(USB_PHYPWR, (1UL<<5)-1);

#if USBPHY_VERSION == 0
	// assert S/W Reset for PHY_CLK and HCLK Domains
	SET_REG_BITS(USB_URSTCON, (1<<2)|(1<<1), (1<<2)|(1<<1));
#endif

	// assert S/W Reset for PHY
	SET_REG_BITS(USB_URSTCON, (1<<0), (1<<0));
	spin(1000);

#if USBPHY_VERSION > 0
	*USB_UVLDCON &= ~(1 << 1);		// Clear vbus_vld_ext
	SET_REG_BITS(USB_UVLDCON, 0, (1<<1));
#endif

	gate_clocks(false);
	platform_power_set_gate(PWRBIT_USB, false);

	usb_phy_powered = false;
}

bool usbphy_is_cable_connected(void)
{
	bool connected;

	if (!usb_phy_powered) {
#if USBPHY_VERSION > 2
		clock_gate(CLK_UPERF, true);
#endif
		clock_gate(CLK_USBPHY, true);
	}

	connected = (GET_REG(USB_UCONDET) & 1) != 0;

	if (!usb_phy_powered) {
#if USBPHY_VERSION > 2
		clock_gate(CLK_UPERF, false);
#endif
		clock_gate(CLK_USBPHY, false);
	}

	return connected;
}

bool usbphy_set_dpdm_monitor(int select)
{
	u_int32_t bits;

	switch (select) {
	  case kUSB_DP:
		bits = 1 << 1;
		break;
	  case kUSB_DM:
		bits = 2 << 1;
		break;
	  case kUSB_NONE:
		bits = 0 << 1;
		break;
#if USBPHY_VERSION >= 4
	  case kUSB_DCD:
		bits = (1 << 14) | (1 << 13) | (1 << 10) | (1 << 9) | (1 << 1);
		break;
	  case kUSB_CP1:
		bits = (1 << 12) | (1 << 11) | (1 << 10) | (1 << 9) | (2 << 1);
		break;
#endif
	  default:
		return false;
	};

	if (!usb_phy_powered) {
#if USBPHY_VERSION > 2
		clock_gate(CLK_UPERF, true);
#endif
		clock_gate(CLK_USBPHY, true);
	}

#if USBPHY_VERSION >= 4
	*USB_UPADCON = (*USB_UPADCON & ((0x3f << 3) | (1 << 0))) | bits;
#else
	SET_REG_BITS(USB_UPADCON, bits, 3 << 1);
#endif

	if (!usb_phy_powered) {
#if USBPHY_VERSION > 2
		clock_gate(CLK_UPERF, false);
#endif
		clock_gate(CLK_USBPHY, false);
	}
	
	return true;
}

#if WITH_DEVICETREE
void usbphy_update_device_tree(DTNode *node)
{
	u_int32_t propSize;
	char *propName;
	void *propData;
	u_int32_t phy_tuning2_val;

    phy_tuning2_val = 0;

#ifdef USBPHY_UOTGTUNE2
	phy_tuning2_val = USBPHY_UOTGTUNE2;
#endif

	// Fill in the phy-tuning-val property
	propName = "phy-tuning-val";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		if (propSize >= sizeof(phy_tuning2_val)) {
			*(u_int32_t *)propData = phy_tuning2_val;
		}
	}
}
#endif
