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

#ifdef  APCIE_DEBUG_LEVEL
#define DEBUG_LEVEL APCIE_DEBUG_LEVEL
#endif

#include <debug.h>
#include <arch/arm/arm.h>
#include <lib/devicetree.h>
#include <drivers/apcie.h>
#include <drivers/dart.h>
#include <platform.h>
#include <platform/apcie.h>
#include <platform/apcie_regs.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/timer.h>
#include <platform/tunables.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <platform/soc/hwregbase.h>
#include <sys.h>
#include <sys/menu.h>

#include SUB_PLATFORM_SPDS_HEADER(apcie_common)
#include SUB_PLATFORM_SPDS_HEADER(apcie_phy_glue)
#include SUB_PLATFORM_SPDS_HEADER(pma_cmn_registers)
#include SUB_PLATFORM_SPDS_HEADER(pma_rx_lane_registers)
#include SUB_PLATFORM_SPDS_HEADER(pma_tx_lane_registers)
#include SUB_PLATFORM_SPDS_HEADER(x2_pcie_rc)

#include SUB_PLATFORM_TUNABLE_HEADER(apcie_common)
#include SUB_PLATFORM_TUNABLE_HEADER(apcie_config)
#include SUB_PLATFORM_TUNABLE_HEADER(x2_pcie_rc)

#define MAX_DT_PROPERTY_SIZE	4096

////////////////////////////////////////////////////////////////////////////////

// APCIe common tunables

#if APPLICATION_SECUREROM

// T8010 A0
static const struct tunable_struct common_tunables_a0 [] = {
	PCIE_APCIE_COMMON_SECURE_ROM_TUNABLES_T8010_A0
};

#else	// !APPLICATION_SECUREROM

// T8010 A0
static const struct tunable_struct common_tunables_a0 [] = {
	PCIE_APCIE_COMMON_DEFAULT_TUNABLES_T8010_A0
};

// APCIe common NVMe tunables

// T8010 A0
static const struct tunable_struct common_nvme_tunables_a0 [] = {
	PCIE_APCIE_COMMON_NVME_TUNABLES_T8010_A0
};

// T8010 chip revision table
static const struct tunable_chip_struct tunables_nvme_common [] = {
	{CHIP_REVISION_A0, APCIE_COMMON_BASE_ADDR, common_nvme_tunables_a0 , NULL, false},
};

#endif	// APPLICATION_SECUREROM

// T8010 chip revision table
static const struct tunable_chip_struct tunables_pcie_common [] = {
	{CHIP_REVISION_A0, APCIE_COMMON_BASE_ADDR, common_tunables_a0 , NULL, false},
};

////////////////////////////////////////////////////////////////////////////////

// APCIe config tunables

#if APPLICATION_SECUREROM

// T8010 A0
static const struct tunable_struct port0_config_tunable_a0[] = {
	PCIE_APCIE_PORT0_APCIE_CONFIG_SECURE_ROM_TUNABLES_T8010_A0
};

static const struct tunable_struct port1_config_tunable_a0[] = {
	PCIE_APCIE_PORT1_APCIE_CONFIG_SECURE_ROM_TUNABLES_T8010_A0
};

static const struct tunable_struct port2_config_tunable_a0[] = {
	PCIE_APCIE_PORT2_APCIE_CONFIG_SECURE_ROM_TUNABLES_T8010_A0
};

static const struct tunable_struct port3_config_tunable_a0[] = {
	PCIE_APCIE_PORT3_APCIE_CONFIG_SECURE_ROM_TUNABLES_T8010_A0
};

#else	// !APPLICATION_SECUREROM

// T8010 A0
static const struct tunable_struct port0_config_tunable_a0[] = {
	PCIE_APCIE_PORT0_APCIE_CONFIG_DEFAULT_TUNABLES_T8010_A0
};

static const struct tunable_struct port1_config_tunable_a0[] = {
	PCIE_APCIE_PORT1_APCIE_CONFIG_DEFAULT_TUNABLES_T8010_A0
};

static const struct tunable_struct port2_config_tunable_a0[] = {
	PCIE_APCIE_PORT2_APCIE_CONFIG_DEFAULT_TUNABLES_T8010_A0
};

static const struct tunable_struct port3_config_tunable_a0[] = {
	PCIE_APCIE_PORT3_APCIE_CONFIG_DEFAULT_TUNABLES_T8010_A0
};

#endif	// !APPLICATION_SECUREROM

// T8010 chip revision table
static const struct tunable_chip_struct tunables_pcie_config_port_0[] = {
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(0), port0_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_1[] = {
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(1), port1_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_2[] = {
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(2), port2_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_3[] = {
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(3), port3_config_tunable_a0, NULL, false},
};

// T8010 port chip revision table
static const struct tunable_chip_array tunables_pcie_config_ports[APCIE_NUM_LINKS] = {
	{ tunables_pcie_config_port_0, ARRAY_SIZE(tunables_pcie_config_port_0), 0 },
	{ tunables_pcie_config_port_1, ARRAY_SIZE(tunables_pcie_config_port_1), 0 },
	{ tunables_pcie_config_port_2, ARRAY_SIZE(tunables_pcie_config_port_2), 0 },
	{ tunables_pcie_config_port_3, ARRAY_SIZE(tunables_pcie_config_port_3), 0 },
};

////////////////////////////////////////////////////////////////////////////////

// PCIe root complex tunables

#if APPLICATION_SECUREROM

// T8010 A0
static const struct tunable_struct rc0_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC0_SECURE_ROM_TUNABLES_T8010_A0
};

static const struct tunable_struct rc1_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC1_SECURE_ROM_TUNABLES_T8010_A0
};

static const struct tunable_struct rc2_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC2_SECURE_ROM_TUNABLES_T8010_A0
};

static const struct tunable_struct rc3_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC3_SECURE_ROM_TUNABLES_T8010_A0
};

#else	// !APPLICATION_SECUREROM

// T8010 A0
static const struct tunable_struct rc0_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC0_DEFAULT_TUNABLES_T8010_A0
};

static const struct tunable_struct rc1_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC1_DEFAULT_TUNABLES_T8010_A0
};

static const struct tunable_struct rc2_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC2_DEFAULT_TUNABLES_T8010_A0
};

static const struct tunable_struct rc3_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC3_DEFAULT_TUNABLES_T8010_A0
};

#endif	// !APPLICATION_SECUREROM

// T8010 chip revision table
static const struct tunable_chip_struct tunables_pcie_config_rc_0[] = {
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(0), rc0_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_1[] = {
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(1), rc1_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_2[] = {
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(2), rc2_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_3[] = {
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(3), rc3_config_tunable_a0, NULL, false},
};

// T8010 port chip revision table
static const struct tunable_chip_array tunables_pcie_config_rc[APCIE_NUM_LINKS] = {
	{ tunables_pcie_config_rc_0, ARRAY_SIZE(tunables_pcie_config_rc_0), 0 },
	{ tunables_pcie_config_rc_1, ARRAY_SIZE(tunables_pcie_config_rc_1), 0 },
	{ tunables_pcie_config_rc_2, ARRAY_SIZE(tunables_pcie_config_rc_2), 0 },
	{ tunables_pcie_config_rc_3, ARRAY_SIZE(tunables_pcie_config_rc_3), 0 },
};

////////////////////////////////////////////////////////////////////////////////

// PHY tunables

// T8010 A0
static const struct tunable_struct phy_glue_tunable_a0[] = {
#ifdef	PCIE_APCIE_PHY_GLUE_DEFAULT_TUNABLES_T8010_A0
	PCIE_APCIE_PHY_GLUE_DEFAULT_TUNABLES_T8010_A0
#else
	// Temporary until tunables are actually defined.
	TUNABLE_TABLE_END_MARKER
#endif
};

const struct tunable_chip_struct tunables_pcie_phy[] = {
	{CHIP_REVISION_A0, APCIE_PHY_BASE_ADDR, phy_glue_tunable_a0, NULL, false},
};

////////////////////////////////////////////////////////////////////////////////

static bool s3e_mode;
static bool s3e_reset_on_enable_disable;
static uint32_t link_enable_count;


#define SET_REG(reg, value)			set_reg(APCIE_COMMON_BASE_ADDR, APCIE_COMMON_BLK_APCIE_ ## reg ## _OFFSET, (value))
#define GET_REG(reg)				get_reg(APCIE_COMMON_BASE_ADDR, APCIE_COMMON_BLK_APCIE_ ## reg ## _OFFSET)
#define OR_REG(reg, value)			or_reg(APCIE_COMMON_BASE_ADDR, APCIE_COMMON_BLK_APCIE_ ## reg ## _OFFSET, (value))
#define AND_REG(reg, value)			and_reg(APCIE_COMMON_BASE_ADDR, APCIE_COMMON_BLK_APCIE_ ## reg ## _OFFSET, (value))

#define PORT_STRIDE				(APCIE_COMMON_BLK_APCIE_PORT_CTRL_1_OFFSET - APCIE_COMMON_BLK_APCIE_PORT_CTRL_0_OFFSET)
#define PORT_REG(reg, port)			(APCIE_COMMON_BLK_APCIE_ ## reg ## _OFFSET) + (PORT_STRIDE * (port))
#define SET_PORT_REG(reg, port, value)		set_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port), value)
#define GET_PORT_REG(reg, port)			get_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port))
#define OR_PORT_REG(reg, port, value)		or_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port), value)
#define AND_PORT_REG(reg, port, value)		and_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port), value)

#define PORT_REG_BIT_SHIFT(reg, bit)		(APCIE_COMMON_BLK_APCIE_ ## reg ## _ ## bit ## _SHIFT)
#define SET_PORT_REG_BIT(reg, port, b)		or_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port), 1 << PORT_REG_BIT_SHIFT(reg, b))
#define CLR_PORT_REG_BIT(reg, port, b)		and_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port), ~(1 << PORT_REG_BIT_SHIFT(reg, b)))
#define GET_PORT_REG_BIT(reg, port, b)		((get_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port)) >> PORT_REG_BIT_SHIFT(reg, b)) & 1)

#define SET_APCIE_CONFIG_REG(reg, port, value)	set_reg(APCIE_PORT_BASE_ADDR(port), (reg), (value))
#define GET_APCIE_CONFIG_REG(reg, port)		get_reg(APCIE_PORT_BASE_ADDR(port), (reg))
#define OR_APCIE_CONFIG_REG(reg, port, value)	or_reg(APCIE_PORT_BASE_ADDR(port), (reg), (value))
#define AND_APCIE_CONFIG_REG(reg, port, value)	and_reg(APCIE_PORT_BASE_ADDR(port), (reg), (value))

#define PHY_SET_REG(reg, value)			set_reg(APCIE_PHY_BASE_ADDR, APCIE_PHY_GLUE_BLK_ ## reg ## _OFFSET, (value))
#define PHY_GET_REG(reg)			get_reg(APCIE_PHY_BASE_ADDR, APCIE_PHY_GLUE_BLK_ ## reg ## _OFFSET)
#define PHY_OR_REG(reg, value)			or_reg(APCIE_PHY_BASE_ADDR, APCIE_PHY_GLUE_BLK_ ## reg ## _OFFSET, (value))
#define PHY_AND_REG(reg, value)			and_reg(APCIE_PHY_BASE_ADDR, APCIE_PHY_GLUE_BLK_ ## reg ## _OFFSET, (value))

#define SET_PMA_CMN_REG(reg, value)		set_reg(APCIE_PHY_PMA_COMMON_BASE_ADDR, PMA_CMN_REGISTERS_BLK_ ## reg ## _OFFSET, (value))
#define GET_PMA_CMN_REG(reg)			get_reg(APCIE_PHY_PMA_COMMON_BASE_ADDR, PMA_CMN_REGISTERS_BLK_ ## reg ## _OFFSET)
#define OR_PMA_CMN_REG(reg, value)		or_reg(APCIE_PHY_PMA_COMMON_BASE_ADDR, PMA_CMN_REGISTERS_BLK_ ## reg ## _OFFSET, (value))
#define AND_PMA_CMN_REG(reg, value)		and_reg(APCIE_PHY_PMA_COMMON_BASE_ADDR, PMA_CMN_REGISTERS_BLK_ ## reg ## _OFFSET, (value))

#define PMA_RX_REG(reg, lane)			(PMA_RX_LANE_REGISTERS_BLK_ ## reg ## _OFFSET) + (APCIE_PHY_PMA_RX_LANE_STRIDE * (lane))
#define SET_PMA_RX_REG(reg, lane, value)	set_reg(APCIE_PHY_PMA_RX_LANE_BASE_ADDR, PMA_RX_REG(reg, lane), value)
#define GET_PMA_RX_REG(reg, lane)		get_reg(APCIE_PHY_PMA_RX_LANE_BASE_ADDR, PMA_RX_REG(reg, lane))
#define OR_PMA_RX_REG(reg, lane, value)		or_reg(APCIE_PHY_PMA_RX_LANE_BASE_ADDR, PMA_RX_REG(reg, lane), value)
#define AND_PMA_RX_REG(reg, lane, value)	and_reg(APCIE_PHY_PMA_RX_LANE_BASE_ADDR, PMA_RX_REG(reg, lane), value)

#define PMA_TX_REG(reg, lane)			(PMA_TX_LANE_REGISTERS_BLK_ ## reg ## _OFFSET) + (APCIE_PHY_PMA_TX_LANE_STRIDE * (lane))
#define SET_PMA_TX_REG(reg, lane, value)	set_reg(APCIE_PHY_PMA_TX_LANE_BASE_ADDR, PMA_TX_REG(reg, lane), value)
#define GET_PMA_TX_REG(reg, lane)		get_reg(APCIE_PHY_PMA_TX_LANE_BASE_ADDR, PMA_TX_REG(reg, lane))
#define OR_PMA_TX_REG(reg, lane, value)		or_reg(APCIE_PHY_PMA_TX_LANE_BASE_ADDR, PMA_TX_REG(reg, lane), value)
#define AND_PMA_TX_REG(reg, lane, value)	and_reg(APCIE_PHY_PMA_TX_LANE_BASE_ADDR, PMA_TX_REG(reg, lane), value)


static void set_reg(uintptr_t base, uint32_t offset, uint32_t value)
{
	dprintf(DEBUG_SPEW, "apcie: set_reg[0x%lx] = 0x%x\n", base + offset, value);
	*(volatile uint32_t *)(base + offset) = value;
}

static uint32_t get_reg(uintptr_t base, uint32_t offset)
{
	uint32_t value;

	dprintf(DEBUG_SPEW, "acpie: get_reg[0x%lx]", base + offset);
	value = *(volatile uint32_t *)(base + offset);

	dprintf(DEBUG_SPEW, " = 0x%x\n", value);
	return value;
}

static void or_reg(uintptr_t base, uint32_t offset, uint32_t value)
{
	uint32_t orig;

	orig = get_reg(base, offset);
	set_reg(base, offset, orig | value);
}

static void and_reg(uintptr_t base, uint32_t offset, uint32_t value)
{
	uint32_t orig;

	orig = get_reg(base, offset);
	set_reg(base, offset, orig & value);
}

void apcie_set_s3e_mode(bool reset_on_enable)
{
	s3e_mode = true;
	s3e_reset_on_enable_disable = reset_on_enable;
}

static void apcie_enable_root_complex(void)
{
	// PCIe Initialization Sequence v1.12.02, section 6.1.2:
	// 1. Program PCIE PLL parameters from PMGR fuses done in pmgr.c set_pll().

	// 2. Turn on PCIE in PCIE_PS register
	clock_gate(CLK_PCIE, true);

	// 3. Enable the 24Mhz clock
	clock_gate(CLK_PCIE_AUX, true);

	// 4. Enable the ref clock
	clock_gate(CLK_PCIE_REF, true);

	// Apply common tunables.
	platform_apply_tunables(tunables_pcie_common , ARRAY_SIZE(tunables_pcie_common ), "PCIe common");
#if !APPLICATION_SECUREROM
	platform_apply_tunables(tunables_nvme_common , ARRAY_SIZE(tunables_nvme_common ), "NVMe common");
#endif

	// 5. Enable clock auto clock management for the "common" clock domain -- not doing this in iBoot.

	// 6. Enable dynamic clock gating of per-link aux_xlk -- not doing this in iBoot.

	// 7. NAND_SYS_CLK/S3E_RESET_N reset sequence defined in Section 5.5.
	if (s3e_mode) {

		if (s3e_reset_on_enable_disable) {
			AND_REG(NANDRST_CTRL, ~APCIE_COMMON_BLK_APCIE_NANDRST_CTRL_NANDRST_UMASK);
		}

		gpio_configure(GPIO_NAND_SYS_CLK, GPIO_CFG_FUNC0);
		OR_REG(NANDSYSCLK_CTRL, APCIE_COMMON_BLK_APCIE_NANDSYSCLK_CTRL_NANDSYSCLK_EN_UMASK);
		// S3e spec requires 32 24 Mhz clock cycles from NANDSYSCLK to reset deassertion.
		// But, APCIe spec requires we wait 100 microseconds (Cayman PCIe spec section 5.5)
		spin(100);

		// De-assert S3E_RESET_N if it's already asserted. Except when platform specified
		// s3e_reset_on_enable_disable (for example, in iBEC), we only de-assert the reset,
		// and then leave it de-asserted
		OR_REG(NANDRST_CTRL, APCIE_COMMON_BLK_APCIE_NANDRST_CTRL_NANDRST_UMASK);
	}

	// 8. Select APCIE lane configuration
	SET_REG(LANE_CFG, APCIE_COMMON_BLK_APCIE_LANE_CFG_VALID_UMASK | platform_get_apcie_lane_cfg());

	// 9. PCIe Phy is powered up:
	// APCIE_PHY_POWERUP_STS.powerup_done is set by the HW when Phy power-up sequence is completed
	while((GET_REG(PHY_POWERUP_STS) & APCIE_COMMON_BLK_APCIE_PHY_POWERUP_STS_POWERUP_DONE_UMASK) == 0)
		spin(1);

	// 10. Software de-asserts phy_apb_reset_n
	OR_REG(PHY_CONFIG_RST_CTRL, APCIE_COMMON_BLK_APCIE_PHY_CONFIG_RST_CTRL_PHY_CFG_RST_N_UMASK);

	// Apply SPDS-generated PHY tunable values
	platform_apply_tunables(tunables_pcie_phy, ARRAY_SIZE(tunables_pcie_phy), "PCIe PHY");

	// 11. Select external REFCLK if desired -- not desired.

	// The following is from Section 6.1.3 PCIe link bring-up sequence, step
	// 1.a.(iii). When internal REFCLK is selected, SW must ensure that REFCLK
	// provided by PCIe REFCLK PLL is stable before proceeding to link bring-up
	while ((GET_REG(PHY_POWERUP_STS) & APCIE_COMMON_BLK_APCIE_PHY_POWERUP_STS_PMGR_REFCLK_GOOD_UMASK) == 0)
		spin(1);
}

static void apcie_disable_root_complex(void)
{
	if (s3e_reset_on_enable_disable) {
		AND_REG(NANDRST_CTRL, ~APCIE_COMMON_BLK_APCIE_NANDRST_CTRL_NANDRST_UMASK);
	}

	clock_set_device_reset(CLK_PCIE, true);

	spin(10);

	// WARNING: DO NOT power off PCIe CLK_PCIE_AUX, CLK_PCIE_REF, and CLK_PCIE
	//	    here. The SEP and AES blocks will be unhappy if you do that.
	//	    Just turn the clock off but leave the power on.
	power_on(CLK_PCIE_AUX);
	power_on(CLK_PCIE_REF);
	power_on(CLK_PCIE);

	clock_set_device_reset(CLK_PCIE, false);
}

// Implements the enable sequence from APCIe spec section 6.1.3 as it applies to
// APCIE_COMMON registers. The portion of the sequence that affects the APCIE_CONFIG
// registers is handled in common code
static void apcie_enable_link_hardware(uint32_t link)
{
	ASSERT(link < APCIE_NUM_LINKS);

	dprintf(DEBUG_INFO, "apcie: Enabling link %d\n", link);

	// Make sure PERST# starts out as asserted
	CLR_PORT_REG_BIT(PORT_RST_CTRL_0, link, APCIE_PERST_N);
 
	// 1(i). 100Mhz REFCLK to the link partners is enabled
	SET_PORT_REG_BIT(PORT_REFCLK_CTRL_0, link, REFCLK_EN);

	// 1(iii). SW must ensure that REFCLK provided by PCIe REFCLK PLL is stable
	// before proceeding to the next step
	while (GET_PORT_REG_BIT(PHY_POWERUP_STS, link, PMGR_REFCLK_GOOD) == 0)
		spin(10);

	// 2. Configure CLKREQ# GPIO for the target link -- done in the common
	// code before calling this function

	// 3. Wait until the endpoint asserts CLKREQ# -- done in the common
	// code before calling this function

	// 4. Update link configuration -- skip this step in SecureROM/iBoot

	// 5. Enable target PCIE link by setting APCIE_PORT_CTRL_{0/1/2/3}.port_en
	SET_PORT_REG_BIT(PORT_CTRL_0, link, PORT_EN);

#if SUPPORT_FPGA
	SET_PORT_REG_BIT(PORT_CTRL_0, link, MUXED_AUXCLK_AUTO_DIS);
#endif

	// 6. De-assert PERST# for the target PCIE link.
	SET_PORT_REG_BIT(PORT_RST_CTRL_0, link, APCIE_PERST_N);
	while (GET_PORT_REG_BIT(PORT_STS_0, link, PORT_STS) == 0)
		spin(10);

	// 7. Enable dynamic REFCLK clock gating -- skip this step in SecureROM/iBoot
}

static void apcie_disable_link_hardware(uint32_t link)
{
	ASSERT(link < APCIE_NUM_LINKS);

	CLR_PORT_REG_BIT(PORT_RST_CTRL_0, link, APCIE_PERST_N);
	CLR_PORT_REG_BIT(PORT_REFCLK_CTRL_0, link, REFCLK_EN);
	CLR_PORT_REG_BIT(PORT_CTRL_0, link, PORT_EN);

	while (GET_PORT_REG_BIT(PORT_STS_0, link, PORT_STS) != 0)
		spin(10);
}

bool apcie_enable_link(uint32_t link)
{
	struct apcie_link_config *config = &apcie_link_configs[link];
	struct apcie_link_status *status = &apcie_link_statuses[link];
	utime_t start;
	uint32_t counter_command;
	uint32_t force_width;
	uint32_t l1sub_cap;
	uint16_t link_control2;

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

	// Apply root complex config tunables
	platform_apply_tunables(tunables_pcie_config_rc[link].tunable_chip,
				tunables_pcie_config_rc[link].num_tunable_chips,
				"PCIe config");

	// Apply APCIe config port tunables
	platform_apply_tunables(tunables_pcie_config_ports[link].tunable_chip,
				tunables_pcie_config_ports[link].num_tunable_chips,
				"APCIe config");

	// Enable fabric timeout.
	OR_APCIE_CONFIG_REG(APCIE_CONFIG_BLK_AF_TMOT_CTRL_OFFSET, link, APCIE_CONFIG_BLK_AF_TMOT_CTRL_AF_TMOT_EN_UMASK);

	// PCIe spec requires a 100 microsecond delay from REFCLK starting
	// to PERST being deasserted
	spin(100);

	// 5) De-assert PERST# for the target PCIE link.
	gpio_configure(config->perst_gpio, GPIO_CFG_OUT_1);

	// Limit link speed to Gen1
	apcie_config_read_raw(&link_control2, PCIE_RC_CAP_LINK_CONTROL2_OFFSET(link), sizeof(link_control2));
	link_control2 &= ~PCIE_RC_CAP_LINK_CONTROL2_PCIE_TARGET_LINK_SPEED_UMASK;
	link_control2 |= 1;
	apcie_config_write_raw(&link_control2, PCIE_RC_CAP_LINK_CONTROL2_OFFSET(link), sizeof(link_control2));

	// Limit link width if requested by the platform. If the platform returns 0,
	// let the hardware autonegotiate the width using its default parameters
	force_width = platform_get_pcie_link_width(link);
	if (force_width != 0) {
		ASSERT(force_width <= 2);

		uint32_t gen2_ctrl;
		apcie_config_read_raw(&gen2_ctrl, PCIE_RC_PORT_LOGIC_GEN2_CTRL_OFFSET(link), sizeof(gen2_ctrl));
		gen2_ctrl &= ~PCIE_RC_PORT_LOGIC_GEN2_CTRL_NUM_OF_LANES_UMASK;
		gen2_ctrl |= (force_width << PCIE_RC_PORT_LOGIC_GEN2_CTRL_NUM_OF_LANES_SHIFT);
		apcie_config_write_raw(&gen2_ctrl, PCIE_RC_PORT_LOGIC_GEN2_CTRL_OFFSET(link), sizeof(gen2_ctrl));

		uint32_t port_link_ctrl;
		apcie_config_read_raw(&port_link_ctrl, PCIE_RC_PORT_LOGIC_PORT_LINK_CTRL_OFFSET(link), sizeof(port_link_ctrl));
		port_link_ctrl &= ~PCIE_RC_PORT_LOGIC_PORT_LINK_CTRL_LINK_CAPABLE_UMASK;
		port_link_ctrl |= ((1 << (force_width - 1)) << PCIE_RC_PORT_LOGIC_PORT_LINK_CTRL_LINK_CAPABLE_SHIFT);
		apcie_config_write_raw(&port_link_ctrl, PCIE_RC_PORT_LOGIC_PORT_LINK_CTRL_OFFSET(link), sizeof(port_link_ctrl));
	}

	// Update L1 substate capabilites register to indicate the root complex
	// has the capability of supporting T_POWER_ON=0us.
	apcie_config_read_raw(&l1sub_cap, PCIE_RC_PORT_LOGIC_L1SUB_CAPABILITY_REG_OFFSET(link), sizeof(l1sub_cap));
	l1sub_cap &= ~PCIE_RC_PORT_LOGIC_L1SUB_CAPABILITY_REG_PWR_ON_VALUE_SUPPORT_UMASK;
	l1sub_cap &= ~PCIE_RC_PORT_LOGIC_L1SUB_CAPABILITY_REG_COMM_MODE_SUPPORT_UMASK;
	apcie_config_write_raw(&l1sub_cap, PCIE_RC_PORT_LOGIC_L1SUB_CAPABILITY_REG_OFFSET(link), sizeof(l1sub_cap));

	// Clear the link counters
	counter_command = get_reg(APCIE_PORT_COUNTER_BASE_ADDR(link), APCIE_LCOUNT_BLK_LCOUNT_COMMAND_0_OFFSET);
	counter_command |= APCIE_COUNTER_CLEAR | APCIE_COUNTER_ENABLE;
	set_reg(APCIE_PORT_COUNTER_BASE_ADDR(link), APCIE_LCOUNT_BLK_LCOUNT_COMMAND_0_OFFSET, counter_command);

	// Disable PHY powergating: <rdar://problem/20630305> H9: PCIe Sequence with Disable PHY power gating and disable txboost
	uint32_t lane_cfg = platform_get_apcie_lane_cfg();
	uint32_t phy_sig_mask;

	uint32_t link_widths[][4] = {
		[APCIE_LANE_CFG_X1_X1_X1_X1] = { 1, 1, 1, 1 },
		[APCIE_LANE_CFG_X2_X1_X1]    = { 2, 0, 1, 1 },
		[APCIE_LANE_CFG_X1_X1_X2]    = { 1, 1, 2, 0 },
		[APCIE_LANE_CFG_X2_X2]       = { 2, 0, 2, 0 },
	};

	ASSERT(lane_cfg < ARRAY_SIZE(link_widths));
	ASSERT(link < ARRAY_SIZE(link_widths[0]));

	uint32_t link_width = link_widths[lane_cfg][link];
	ASSERT(link_width != 0);

	phy_sig_mask = ((1 << link_width) - 1) << link;

	// Wait for the phy lanes to power up.
	while ((PHY_GET_REG(PCIE_PHY_SIG) & phy_sig_mask) != phy_sig_mask) {
		spin(10);
	}

#if APPLICATION_SECUREROM

	// 6) Always enable DFW adapatation for SecureROM.

	// For each lane...
	for (uint32_t lane = 0; phy_sig_mask != 0; lane++, phy_sig_mask >>= 1) {
		// ... if we're enabling this lane ...
		if (phy_sig_mask & 1) {
			// ... set DFE adapation to always enabled.
			OR_PMA_RX_REG(RX_REE_GEN_CTRL_EN, lane, PMA_RX_LANE_REGISTERS_BLK_RX_REE_GEN_CTRL_EN_15_UMASK);
			OR_PMA_RX_REG(RX_REE_FREE0_CFGD, lane, PMA_RX_LANE_REGISTERS_BLK_RX_REE_FREE0_CFGD_6_UMASK);
			OR_PMA_TX_REG(XCVR_DIAG_DCYA, lane, PMA_TX_LANE_REGISTERS_BLK_XCVR_DIAG_DCYA_0_UMASK);
		}
	}

#else	// !APPLICATION_SECUREROM

	// 5) Disable PCIE PHY common block power gating.

	// Disable PCIE PHY power gating.
	OR_PMA_CMN_REG(CMN_CDIAG_FUNC_PWR_CTRL, PMA_CMN_REGISTERS_BLK_CMN_CDIAG_FUNC_PWR_CTRL_14_UMASK);
	OR_PMA_CMN_REG(CMN_CDIAG_XCTRL_PWR_CTRL, PMA_CMN_REGISTERS_BLK_CMN_CDIAG_XCTRL_PWR_CTRL_14_UMASK);

	// Update common PHY PLL timers.
	uint32_t phy_reg;
	phy_reg = GET_PMA_CMN_REG(CMN_SSM_PLLPRE_TMR);
	phy_reg &= ~PMA_CMN_REGISTERS_BLK_CMN_SSM_PLLPRE_TMR_11_0_UMASK;
	phy_reg |= PMA_CMN_REGISTERS_BLK_CMN_SSM_PLLPRE_TMR_11_0_INSRT(0x64);
	SET_PMA_CMN_REG(CMN_SSM_PLLPRE_TMR, phy_reg);
	phy_reg = GET_PMA_CMN_REG(CMN_SSM_PLLLOCK_TMR);
	phy_reg &= ~PMA_CMN_REGISTERS_BLK_CMN_SSM_PLLLOCK_TMR_11_0_UMASK;
	phy_reg |= PMA_CMN_REGISTERS_BLK_CMN_SSM_PLLLOCK_TMR_11_0_INSRT(0x19);
	SET_PMA_CMN_REG(CMN_SSM_PLLLOCK_TMR, phy_reg);

	// For each lane...
	for (uint32_t lane = 0; phy_sig_mask != 0; lane++, phy_sig_mask >>= 1) {
		// ... if we're enabling this lane ...
		if (phy_sig_mask & 1) {
			// ... disable power gating ...
			OR_PMA_TX_REG(XCVR_DIAG_FUNC_PWR_CTRL, lane, PMA_TX_LANE_REGISTERS_BLK_XCVR_DIAG_FUNC_PWR_CTRL_14_UMASK);

			// ... and disable transmitter boost ...
			SET_PMA_TX_REG(TX_DIAG_TX_BOOST, lane, 0);

			// ... and update the lane calibration timer ...
			phy_reg = GET_PMA_TX_REG(XCVR_PSM_LANECAL_TMR, lane);
			phy_reg &= ~PMA_TX_LANE_REGISTERS_BLK_XCVR_PSM_LANECAL_TMR_11_0_UMASK;
			phy_reg |= PMA_TX_LANE_REGISTERS_BLK_XCVR_PSM_LANECAL_TMR_11_0_INSRT(0x600);
			SET_PMA_TX_REG(XCVR_PSM_LANECAL_TMR, lane, phy_reg);

			// ... and improve SIG DET performance ...
			SET_PMA_RX_REG(RX_DIAG_SIGDET_TUNE, lane, 0x3105);

			// ... and improve PI stability ...
			phy_reg = GET_PMA_RX_REG(RX_DIAG_ILL_IQE_TRIM3, lane);
			phy_reg &= ~PMA_RX_LANE_REGISTERS_BLK_RX_DIAG_ILL_IQE_TRIM3_7_0_UMASK;
			phy_reg |= PMA_RX_LANE_REGISTERS_BLK_RX_DIAG_ILL_IQE_TRIM3_7_0_INSRT(0x9f);
			SET_PMA_RX_REG(RX_DIAG_ILL_IQE_TRIM3, lane, phy_reg);

			phy_reg = GET_PMA_RX_REG(RX_DIAG_ILL_IQE_TRIM5, lane);
			phy_reg &= ~PMA_RX_LANE_REGISTERS_BLK_RX_DIAG_ILL_IQE_TRIM5_7_0_UMASK;
			phy_reg |= PMA_RX_LANE_REGISTERS_BLK_RX_DIAG_ILL_IQE_TRIM5_7_0_INSRT(0x01);
			SET_PMA_RX_REG(RX_DIAG_ILL_IQE_TRIM5, lane, phy_reg);

			// ... and improve CDR performace.
			phy_reg = GET_PMA_RX_REG(RX_CDRLF_CNFG, lane);
			phy_reg &= ~PMA_RX_LANE_REGISTERS_BLK_RX_CDRLF_CNFG_4_0_UMASK;
			phy_reg |= PMA_RX_LANE_REGISTERS_BLK_RX_CDRLF_CNFG_4_0_INSRT(0xa);
			SET_PMA_RX_REG(RX_CDRLF_CNFG, lane, phy_reg);

			// NOTE: Because iBoot always forces links to Gen1 speed,
			//	 the steps for non-Gen1 speeds have been omitted.
		}
	}
#endif	// APPLICATION_SECUREROM

	// 7) Software sets LINKCFG.ltssm_en to start link training
	OR_APCIE_CONFIG_REG(APCIE_CONFIG_BLK_LINKCFG_OFFSET, link, APCIE_CONFIG_BLK_LINKCFG_LTSSM_EN_UMASK);

	start = system_time();
	while ((GET_APCIE_CONFIG_REG(APCIE_CONFIG_BLK_LINKSTS_OFFSET, link) & APCIE_CONFIG_BLK_LINKSTS_LINK_STATUS_UMASK) == 0) {
		if (time_has_elapsed(start, APCIE_ENABLE_TIMEOUT)) {
			uint32_t linksts = GET_APCIE_CONFIG_REG(APCIE_CONFIG_BLK_LINKSTS_OFFSET, link);
			dprintf(DEBUG_CRITICAL, "apcie: Timeout waiting for LinkUp on link %u\n", link);
			dprintf(DEBUG_CRITICAL, "apcie: LINKSTS 0x%08x (LTSSM state %d)\n",
				linksts, APCIE_CONFIG_BLK_LINKSTS_LTSSM_STATE_XTRCT(linksts));
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

	set_reg(APCIE_PORT_COUNTER_BASE_ADDR(link), APCIE_LCOUNT_BLK_LCOUNT_COMMAND_0_OFFSET, 0);

	// Request PMETO and clear the previous status indications
	SET_APCIE_CONFIG_REG(APCIE_CONFIG_BLK_PMETO_OFFSET, link,
				 APCIE_CONFIG_BLK_PMETO_FLD_UMASK
			       | APCIE_CONFIG_BLK_PMETO_PME_TO_ACK_MSG_UMASK
			       | APCIE_CONFIG_BLK_PMETO_TIMEOUT_UMASK);
	start = system_time();
	while (GET_APCIE_CONFIG_REG(APCIE_CONFIG_BLK_PMETO_OFFSET, link) & APCIE_CONFIG_BLK_PMETO_FLD_UMASK) {
		if (time_has_elapsed(start, 10000)) {
			dprintf(DEBUG_CRITICAL, "apcie: timeout waiting for PME_To_Ack, continuing\n");
			break;
		}
		spin(10);
	}

	start = system_time();
	while ((GET_APCIE_CONFIG_REG(APCIE_CONFIG_BLK_LINKSTS_OFFSET, link) & APCIE_CONFIG_BLK_LINKSTS_L2_STATE_UMASK) == 0) {
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
	DTNode *node;
	uint32_t propSize;
	uint8_t *dt_prop = (uint8_t *)malloc(MAX_DT_PROPERTY_SIZE);
	uint8_t *next_dt_prop = dt_prop;
	char bridge[16];

	// Apply common tunables.
	bzero(dt_prop, MAX_DT_PROPERTY_SIZE);
	next_dt_prop = dt_prop;
	next_dt_prop = platform_apply_dt_tunables(tunables_pcie_common,
						  ARRAY_SIZE(tunables_pcie_common),
						  next_dt_prop,
						  0, // Already at the correct offset
						  "apcie common");

	next_dt_prop = platform_apply_dt_tunables(tunables_nvme_common ,
						  ARRAY_SIZE(tunables_nvme_common ),
						  next_dt_prop,
						  0, // Already at the correct offset
						  "apcie common");

	propSize = next_dt_prop - dt_prop;
	RELEASE_ASSERT(propSize < MAX_DT_PROPERTY_SIZE);
	dt_set_prop(apcie_node, "apcie-common-tunables", dt_prop, propSize);

	// Apply port-specific config tunables.
	for (uint32_t i = 0; i < APCIE_NUM_LINKS; i++) {
		snprintf(bridge, sizeof(bridge), "pci-bridge%d", i);
		if (dt_find_node(apcie_node, bridge, &node)) {
			// APCIe port config tunables.
			bzero(dt_prop, MAX_DT_PROPERTY_SIZE);
			next_dt_prop = dt_prop;
			next_dt_prop = platform_apply_dt_tunables(tunables_pcie_config_ports[i].tunable_chip,
								  tunables_pcie_config_ports[i].num_tunable_chips,
								  next_dt_prop,
								  tunables_pcie_config_ports[i].dt_base,
								  bridge);
			propSize = next_dt_prop - dt_prop;
			RELEASE_ASSERT(propSize < MAX_DT_PROPERTY_SIZE);
			dt_set_prop(node, "apcie-config-tunables", dt_prop, propSize);

			// PCIe root complex config tunables.
			snprintf(bridge, sizeof(bridge), "pcie-rc%d", i);
			bzero(dt_prop, MAX_DT_PROPERTY_SIZE);
			next_dt_prop = dt_prop;
			next_dt_prop = platform_apply_dt_tunables(tunables_pcie_config_rc[i].tunable_chip,
								  tunables_pcie_config_rc[i].num_tunable_chips,
								  next_dt_prop,
								  tunables_pcie_config_rc[i].dt_base,
								  bridge);
			propSize = next_dt_prop - dt_prop;
			RELEASE_ASSERT(propSize < MAX_DT_PROPERTY_SIZE);
			dt_set_prop(node, "pcie-rc-tunables", dt_prop, propSize);
		} else {
			dprintf(DEBUG_INFO,"PCIe bridge node %s not found -- skipping tunable update\n", bridge);
		}
	}

	// Apply SPDS PHY tunables.
	bzero(dt_prop, MAX_DT_PROPERTY_SIZE);
	next_dt_prop = dt_prop;
	next_dt_prop = platform_apply_dt_tunables(tunables_pcie_phy,
						  ARRAY_SIZE(tunables_pcie_phy),
						  next_dt_prop,
						  APCIE_PHY_BASE_ADDR,
						  "PCIe PHY");

	propSize = next_dt_prop - dt_prop;
	RELEASE_ASSERT(propSize < MAX_DT_PROPERTY_SIZE);

	dt_set_prop(apcie_node, "phy-params", dt_prop, propSize);
	free(dt_prop);
	dt_prop = next_dt_prop = NULL;
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
