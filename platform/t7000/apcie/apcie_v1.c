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

#include <debug.h>
#include <arch/arm/arm.h>
#include <drivers/apcie.h>
#include <drivers/dart.h>
#include <drivers/pci.h>
#include <lib/devicetree.h>
#include <platform.h>
#include <platform/apcie_regs.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/timer.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <platform/soc/hwregbase.h>
#include <sys.h>
#include <sys/menu.h>
#include "apcie_common_regs.h"

static bool s3e_mode;
static bool s3e_reset_on_enable;
static uint32_t link_enable_count;

struct apcie_dbi_overrides {
	uint32_t offset;
	uint32_t mask;
	uint32_t value;
};

static struct apcie_dbi_overrides apcie_dbi_overrides[] = {
	// Enable 64-bit addresses
	{ 0x024, 0x00000001, 1 },
	// Don't advertise L0s support
	{ 0x07c, 0x00000400, 0 },
	// Set T_POWER_OFF field in the L1_substates register to 2 (<rdar://problem/17813587>)
	{ 0xb44, 0x00000003, 2 },
};

bool external_refclk;


// selects internal vs external refclk. must be called before
// enabling the first link
void apcie_use_external_refclk(bool use)
{
	external_refclk = use;
}

void apcie_set_s3e_mode(bool reset_on_enable)
{
	s3e_mode = true;
	s3e_reset_on_enable = reset_on_enable;
}

#if !SUB_PLATFORM_T7000
static void apcie_enable_phy_clkreq(uint32_t phy)
{
	// Note, the function number might change for future platforms
	if (phy == 0) {
		gpio_configure(GPIO_PCIE_PHY01_CLKREQ, GPIO_CFG_FUNC1);
	} else if (phy == 1) {
		gpio_configure(GPIO_PCIE_PHY23_CLKREQ, GPIO_CFG_FUNC1);
	} else {
		panic("apcie: unknown phy %u", phy);
	}
	rAPCIE_PHY_CLKREQ_CTRL(phy) |= 1;
}

static void apcie_disable_phy_clkreq(uint32_t phy)
{
	rAPCIE_PHY_CLKREQ_CTRL(phy) &= ~1;
	if (phy == 0) {
		gpio_configure(GPIO_PCIE_PHY01_CLKREQ, GPIO_CFG_DFLT);
	} else if (phy == 1) {
		gpio_configure(GPIO_PCIE_PHY23_CLKREQ, GPIO_CFG_DFLT);
	} else {
		panic("apcie: unknown phy %u", phy);
	}
}
#endif

static void apcie_enable_external_refclk(void)
{
	// Sequence from section 6.1.5 of Fiji/Capri APCIe spec (updated to r1.04.44)

	// 1) Set APCIE_COMMON_APCIE_RST_CTRL_{0/1}.apcie_phy_sw_reset to place the PHY in reset.
	for (int i = 0; i < APCIE_NUM_LINKS; i += 2)
		rAPCIE_PHY_RST_CTRL(i / 2) |= APCIE_PHY_RST_CTRL_phy_sw_reset;
	// 2) To access the PHY, the link shall be enabled.
	for (int i = 0; i < APCIE_NUM_LINKS; i+= 2)
		rAPCIE_PORT_CTRL(i) |= APCIE_PORT_CTRL_port_en;
	// 3) De-assert PERST# to the root controller
	for (int i = 0; i < APCIE_NUM_LINKS; i+= 2)
		rAPCIE_PORT_RST_CTRL(i) |= APCIE_PORT_RST_CTRL_perst_n;
	// 4) Select the external REFCLK
	for (int i = 0; i < APCIE_NUM_LINKS; i+= 2)
		rAPCIE_PHY_PHY_REF_USE_PAD(i) = 1;
	//    c) Insert a DMB barrier instruction to insure that the external REFCLK switch occurs
	//       before the port is disabled.
	platform_memory_barrier();
	// 5) Assert PERST# to the root controller
	for (int i = 0; i < APCIE_NUM_LINKS; i+= 2)
		rAPCIE_PORT_RST_CTRL(i) &= ~APCIE_PORT_RST_CTRL_perst_n;
	// 6) Disable the link(s)
	for (int i = 0; i < APCIE_NUM_LINKS; i+= 2)
		rAPCIE_PORT_CTRL(i) &= ~APCIE_PORT_CTRL_port_en;
	// 7) Release the PHY(s) from reset
	for (int i = 0; i < APCIE_NUM_LINKS; i += 2)
		rAPCIE_PHY_RST_CTRL(i / 2) &= ~APCIE_PHY_RST_CTRL_phy_sw_reset;
}

static void apcie_enable_root_complex(void)
{
	clock_gate(CLK_PCIE, true);
#if SUB_PLATFORM_T7000
	// Fiji uses the clock mesh for refclk, Capri has a dedicated PLL for refclk
	if (!external_refclk)
		clock_gate(CLK_PCIE_REF, true);
#endif
	clock_gate(CLK_PCIE_AUX, true);
	spin(10);

	if (external_refclk)
		apcie_enable_external_refclk();

	if (s3e_mode) {
		if (s3e_reset_on_enable) {
			gpio_configure(GPIO_S3E_RESETN, GPIO_CFG_OUT_0);
		}

		gpio_configure(GPIO_NAND_SYS_CLK, GPIO_CFG_FUNC0);
		rAPCIE_NANDSYSCLK_CTRL = 1;

		// S3e spec requires 32 24 Mhz clock cycles from NANDSYSCLK to reset deassertion.
		// The Maui spec requires 100 microseconds, so we'll follow that to match the behavior
		// we'll be using in Maui
		spin(100);

		gpio_configure(GPIO_S3E_RESETN, GPIO_CFG_OUT_1);
	}
}

static void apcie_disable_root_complex(void)
{
	clock_set_device_reset(CLK_PCIE, true);
	spin(1);
	clock_gate(CLK_PCIE, false);

	clock_set_device_reset(CLK_PCIE, false);
}

// Implements the enable sequence from APCIe spec section 6.1 as it applies to
// APCIE_COMMON registers. The portion of the sequence that affects the APCIE_CONFIG
// registers is handled in common code
static void apcie_enable_link_hardware(uint32_t link)
{
	ASSERT(link < APCIE_NUM_LINKS);

	dprintf(DEBUG_INFO, "apcie: Enabling link %d%s\n", link, external_refclk ? " with external refclk" : "");

	// Make sure PERST# starts out as asserted
	rAPCIE_PORT_RST_CTRL(link) &= ~APCIE_PORT_RST_CTRL_perst_n;

	// 2) 100Mhz REFCLK is enabled
	if (!external_refclk) {
		rAPCIE_PORT_REFCLK_CTRL(link) |= APCIE_PORT_REFCLK_CTRL_refclk_en;

#if !SUB_PLATFORM_T7000
		while(!clock_get_pcie_refclk_good())
			spin(10);
#endif
	} else {
		// On Capri, the PHY asserts an external clkreq enable GPIO
		// when it wants the external refclk to be enabled. On Fiji the
		// external refclk is required to always be enabled
#if !SUB_PLATFORM_T7000
		apcie_enable_phy_clkreq(link / 2);
#endif
	}

	// 3) 10 us wait
	spin(10);

	// 4) Target PCIE link is enabled
	rAPCIE_PORT_OVERRIDE_CTRL(link) &= ~APCIE_PORT_OVERRIDE_CTRL_pipe_ovr_en;
	rAPCIE_PORT_CTRL(link) |= APCIE_PORT_CTRL_port_en;

#if SUPPORT_FPGA
	rAPCIE_PORT_CTRL(link) |= APCIE_PORT_CTRL_muxed_auxclk_auto_dis;
#endif

	// 5) De-assert PERST# for the target PCIE link.
	rAPCIE_PORT_RST_CTRL(link) |= APCIE_PORT_RST_CTRL_perst_n;

	// 6b) DBI register overrides
	// Unlock access to the readonly registers
	rAPCIE_CONFIG_SPACE32(link, 0x8bc) |= 1;
	for (unsigned i = 0; i < sizeof(apcie_dbi_overrides) / sizeof(apcie_dbi_overrides[0]); i++) {
		uint32_t offset = apcie_dbi_overrides[i].offset;
		uint32_t mask = apcie_dbi_overrides[i].mask;
		uint32_t value = apcie_dbi_overrides[i].value;
		rAPCIE_CONFIG_SPACE32(link, offset) = (rAPCIE_CONFIG_SPACE32(link, offset) & ~mask) | value;
	}
	// Lock access to the readonly registers
	rAPCIE_CONFIG_SPACE32(link, 0x8bc) &= ~1;
}

static void apcie_disable_link_hardware(uint32_t link)
{
	ASSERT(link < APCIE_NUM_LINKS);

	rAPCIE_PORT_RST_CTRL(link) &= ~APCIE_PORT_RST_CTRL_perst_n;
	rAPCIE_PORT_REFCLK_CTRL(link) &= ~APCIE_PORT_REFCLK_CTRL_refclk_en;
	rAPCIE_PORT_CTRL(link) &= ~APCIE_PORT_CTRL_port_en;
	rAPCIE_PORT_OVERRIDE_CTRL(link) |= APCIE_PORT_OVERRIDE_CTRL_pipe_ovr_en;
#if !SUB_PLATFORM_T7000
	if (external_refclk) {
		// If the other link on this PHY is already disabled, go ahead
		// and disable the external refclk settings and GPIO
		if ((rAPCIE_PORT_CTRL(link ^ 1) & APCIE_PORT_CTRL_port_en) == 0) {
			apcie_disable_phy_clkreq(link / 2);
		}
	}
#endif
}


bool apcie_enable_link(uint32_t link)
{
	struct apcie_link_config *config = &apcie_link_configs[link];
	struct apcie_link_status *status = &apcie_link_statuses[link];
	utime_t start;
	uint32_t force_width;

	ASSERT(link < APCIE_NUM_LINKS);

	if (status->enabled)
		return true;

	// Clocks to PCIe don't default to on since it isn't used in POR configurations
	if (link_enable_count == 0) {
		apcie_enable_root_complex();
	}

	// Make sure PERST# starts out as asserted
	gpio_configure(config->perst_gpio, GPIO_CFG_OUT_0);
	gpio_configure(config->clkreq_gpio, GPIO_CFG_IN);

	// wait for CLKREQ# to be asserted
	dprintf(DEBUG_INFO, "apcie: waiting for clkreq...");
	start = system_time();
	while (gpio_read(config->clkreq_gpio) != 0) {
		if (time_has_elapsed(start, APCIE_ENABLE_TIMEOUT)) {
			dprintf(DEBUG_CRITICAL, "apcie: timeout waiting for CLKREQ# on link %u\n", link);
			goto cleanup;
		}
		spin(1);
	}
	dprintf(DEBUG_INFO, " done\n");

	gpio_configure(config->clkreq_gpio, GPIO_CFG_FUNC0);

	// Do required initialization in APCIE_COMMON registers
	apcie_enable_link_hardware(link);

	// Apply tunables
	rAPCIE_CONFIG_INBCTRL(link) |= APCIE_CONFIG_INBCTRL_CPL_MUST_PUSH_EN;
	rAPCIE_CONFIG_OUTBCTRL(link) |= APCIE_CONFIG_OUTBCTRL_EARLY_PT_RESP_EN;
	rAPCIE_CONFIG_PMETOACK_TMOTLIM(link) = 0x28;

	// PCIe spec requires a 100 microsecond delay from REFCLK starting
	// to PERST being deasserted
	spin(100);

	// 5) De-assert PERST# for the target PCIE link.
	gpio_configure(config->perst_gpio, GPIO_CFG_OUT_1);

	// Limit link speed to Gen1
	rAPCIE_PCIE_CAP_LINK_CONTROL2(link) = (rAPCIE_PCIE_CAP_LINK_CONTROL2(link) & ~0xfU) | 1;

	// Limit link width if requested by the platform. If the platform returns 0,
	// let the hardware autonegotiate the width using its default parameters
	force_width = platform_get_pcie_link_width(link);
	if (force_width != 0) {
		ASSERT(force_width <= 2);

		uint32_t gen2_ctrl;
		gen2_ctrl = rAPCIE_PORT_LOGIC_GEN2_CTRL(link);
		gen2_ctrl &= ~(0x1ff << 8);
		gen2_ctrl |= (force_width << 8);
		rAPCIE_PORT_LOGIC_GEN2_CTRL(link) = gen2_ctrl;

		uint32_t port_link_ctrl;
		port_link_ctrl = rAPCIE_PORT_LOGIC_PORT_LINK_CTRL(link);
		port_link_ctrl &= ~(0x3f << 16);
		port_link_ctrl |= ((1 << (force_width - 1)) << 16);
		rAPCIE_PORT_LOGIC_PORT_LINK_CTRL(link) = port_link_ctrl;
	}

	rAPCIE_COUNTER_COMMAND(link) = APCIE_COUNTER_CLEAR | APCIE_COUNTER_ENABLE;

	// 6c) Software sets LINKCFG.ltssm_en to start link training
	rAPCIE_CONFIG_LINKCFG(link) |= APCIE_CONFIG_LINKCFG_LTSSM_EN;


	start = system_time();
	while ((rAPCIE_CONFIG_LINKSTS(link) & APCIE_CONFIG_LINKSTS_LINK_STATUS) == 0) {
		if (time_has_elapsed(start, APCIE_ENABLE_TIMEOUT)) {
			uint32_t linkpmgrsts;
			dprintf(DEBUG_CRITICAL, "apcie: Timeout waiting for LinkUp on link %u\n", link);
			dprintf(DEBUG_CRITICAL, "apcie: LINKSTS 0x%08x\n", rAPCIE_CONFIG_LINKSTS(link));
			linkpmgrsts = rAPCIE_CONFIG_LINKPMGRSTS(link);
			dprintf(DEBUG_CRITICAL, "apcie: LINKPMGRSTS 0x%08x (LTSSM state %d)\n", linkpmgrsts, (linkpmgrsts >> 9) & 0x1F);

			goto cleanup;
		}
		spin(10);
	}

	status->enabled = true;
	link_enable_count++;

	dart_init(config->dart_id);

	apcie_setup_root_port_bridge(link, config);

	return true;

cleanup:
	gpio_configure(config->perst_gpio, GPIO_CFG_OUT_0);
	spin(10);
	gpio_configure(config->perst_gpio, GPIO_CFG_DFLT);
	gpio_configure(config->clkreq_gpio, GPIO_CFG_DFLT);

	apcie_disable_link_hardware(link);

	if (link_enable_count == 0) {
		apcie_disable_root_complex();
	}

	return false;
}

void apcie_disable_link(uint32_t link)
{
	struct apcie_link_config *config = &apcie_link_configs[link];
	struct apcie_link_status *status = &apcie_link_statuses[link];
	utime_t start;

	ASSERT(link < APCIE_NUM_LINKS);

	if (!status->enabled)
		return;

	dprintf(DEBUG_INFO, "apcie: Disabling link %d\n", link);

	rAPCIE_COUNTER_COMMAND(link) = 0;

	// Request PMETO and clear the previous status indications
	rAPCIE_CONFIG_PMETO(link) = 1 | (3 << 4);

	start = system_time();
	while (rAPCIE_CONFIG_PMETO(link) & 1) {
		if (time_has_elapsed(start, 10000)) {
			dprintf(DEBUG_CRITICAL, "apcie: timeout waiting for PME_To_Ack, continuing\n");
			break;
		}
		spin(10);
	}

	start = system_time();
	while ((rAPCIE_CONFIG_LINKSTS(link) & APCIE_CONFIG_LINKSTS_L2_STATE) == 0) {
		if (time_has_elapsed(start, 10000)) {
			dprintf(DEBUG_CRITICAL, "apcie: link did not go into L2, continuing\n");
			break;
		}
		spin(10);
	}

	gpio_configure(config->perst_gpio, GPIO_CFG_OUT_0);
	// delay to allow PERST# signal to settle before letting the pulldown hold it low
	// and to allow the endpoint to handle PERST# before removing REFCLK
	spin(25);
	gpio_configure(config->perst_gpio, GPIO_CFG_DFLT);
	gpio_configure(config->clkreq_gpio, GPIO_CFG_DFLT);

#if DEBUG_BUILD
	dart_assert_unmapped(config->dart_id);
#endif

	apcie_disable_link_hardware(link);

	status->enabled = false;
	link_enable_count--;

	if (link_enable_count == 0) {
		apcie_disable_root_complex();
	}

	apcie_free_port_bridge(link);
}

uint32_t apcie_get_link_enable_count(void)
{
	return link_enable_count;
}

#if WITH_DEVICETREE
void apcie_update_devicetree(DTNode *apcie_node)
{
	uint32_t propSize;
	char *propName;
	void *propData;

	propName = "dbi-overrides";
	if (FindProperty(apcie_node, &propName, &propData, &propSize)) {
		if (propSize < sizeof(apcie_dbi_overrides))
			panic("dbi-overrides property is too small");
		memcpy(propData, apcie_dbi_overrides, sizeof(apcie_dbi_overrides));
	}
}
#endif

struct apcie_link_config platform_apcie_link_configs[APCIE_NUM_LINKS] = {
	[0] = {
		.perst_gpio =		GPIO_PCIE0_PERST,
		.clkreq_gpio =		GPIO_PCIE0_CLKREQ,
		.dart_id = 		PCIE_PORT0_DART_ID,
	},
	[1] = {
		.perst_gpio =		GPIO_PCIE1_PERST,
		.clkreq_gpio =		GPIO_PCIE1_CLKREQ,
		.dart_id = 		PCIE_PORT1_DART_ID,
	},
#if SUB_PLATFORM_T7001
	[2] = {
		.perst_gpio =		GPIO_PCIE2_PERST,
		.clkreq_gpio =		GPIO_PCIE2_CLKREQ,
		.dart_id = 		PCIE_PORT2_DART_ID,
	},
	[3] = {
		.perst_gpio =		GPIO_PCIE3_PERST,
		.clkreq_gpio =		GPIO_PCIE3_CLKREQ,
		.dart_id = 		PCIE_PORT3_DART_ID,
	},
#endif
};

void platform_register_pci_busses(void)
{
	apcie_init();
}

uint64_t platform_map_host_to_pci_addr(uintptr_t addr)
{
	// Right now we're only supporting 32-bit PCI addresses
	// for aPCIe, so just lop off the top 32 bits
	return addr & 0xFFFFFFFF;
}

uintptr_t platform_map_pci_to_host_addr(uint64_t addr)
{
	// Right now we're only supporting 32-bit PCI addresses
	// for aPCIe
	ASSERT((addr & ~0xFFFFFFFFULL) == 0);
	return PCI_32BIT_BASE + (addr & (PCI_32BIT_LEN - 1));
}
