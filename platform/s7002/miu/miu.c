/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch.h>
#include <debug.h>
#include <drivers/miu.h>
#include <platform.h>
#include <platform/memmap.h>
#include <platform/miu.h>
#include <platform/soc/chipid.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <platform/soc/reconfig.h>
#include <platform/clocks.h>
#include <platform/soc/hwclocks.h>
#include <platform/timer.h>

#if (APPLICATION_IBOOT)

static void miu_configure_bridge(const u_int32_t *bridge_settings);

#define STATIC_BRIDGE_SHIFT		(28)
#define STATIC_BRIDGE_OFFSET_MASK	((1 << STATIC_BRIDGE_SHIFT) - 1)

#define PMGR_REGISTERS		(0 << STATIC_BRIDGE_SHIFT)
#define CPU_FABRIC_WIDGETS	(1 << STATIC_BRIDGE_SHIFT)
#define NRT_FABRIC_WIDGETS	(2 << STATIC_BRIDGE_SHIFT)

static const u_int32_t bridge_registers[] = {
	PMGR_BASE_ADDR,
	CPU_FABRIC_BASE_ADDR,
	NRT_FABRIC_BASE_ADDR,
};

// This array is composed of 3-item tuples consisting of:
//	Bridge id | register offset (with optional | RECONFIG_RAM_CMD_READ)
//	Register data value or comparison value if RECONFIG_RAM_CMD_READ is specified
//	Mask value if RECONFIG_RAM_CMD_READ is specified (used only by reconfig engine)
static const u_int32_t bridge_settings_static[] = {
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_AMCRDRATELIMIT,	0, 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_AMCWRALIMIT,		0, 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_AMCRTRLIMIT,		(0x3f << 8) | (0x3f << 0), 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_AMCWTRLIMIT,		(0x3f << 8) | (0x3f << 0), 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_SPURDRATELIMIT,	0, 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_SPUWRALIMIT,		0, 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_SPURTRLIMIT,		(0x3f << 8) | (0x3f << 0), 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_SPUWTRLIMIT,		(0x3f << 8) | (0x3f << 0), 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_SPUWGATHER,		(0x01 << 8), 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_LIOWGATHER,		(0x01 << 8), 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_AUERDRATELIMIT,	0, 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_AUEWRALIMIT,		0, 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_AUERTRLIMIT,		(0x3f << 8) | (0x3f << 0), 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_AUEWTRLIMIT,		(0x3f << 8) | (0x3f << 0), 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_AUEWGATHER,		(0x01 << 8), 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_ANSRDRATELIMIT,	0, 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_ANSWRALIMIT,		0, 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_ANSRTRLIMIT,		(0x08 << 8) | (0x08 << 0), 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_ANSWTRLIMIT,		(0x05 << 8) | (0x05 << 0), 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_ANSWGATHER,		(0x01 << 8), 0,
	CPU_FABRIC_WIDGETS | CPU_Fabric_pl301Wrap0_AXI0_ARCHANARBMI0,	0, 0,

	// Turn on the clock that controls the NRT Fabric widgets. The MSR clock
	// is behind the Media clock so we turn it on to keep the Media clock
	// from gating.
	PMGR_REGISTERS     | PMGR_MSR_PS,				(0x1 << 8) | (0x1 << 9) | (0xf << 0), 0,
	PMGR_REGISTERS     | PMGR_MEDIA_CLK_CFG,			0x80100000, 0,
	PMGR_REGISTERS     | PMGR_MEDIA_CLK_CFG | RECONFIG_RAM_CMD_READ,0, 0x40000000,

	NRT_FABRIC_WIDGETS | NRT_Fabric_pl301Wrap1_AMCRDRATELIMIT,	0, 0,
	NRT_FABRIC_WIDGETS | NRT_Fabric_pl301Wrap1_AMCWRALIMIT,		0, 0,
	NRT_FABRIC_WIDGETS | NRT_Fabric_pl301Wrap1_AMCRTRLIMIT,		(0x3f << 8) | (0x3f << 0), 0,
	NRT_FABRIC_WIDGETS | NRT_Fabric_pl301Wrap1_AMCWTRLIMIT,		(0x3f << 8) | (0x3f << 0), 0,
	NRT_FABRIC_WIDGETS | NRT_Fabric_pl301Wrap1_MSRRDRATELIMIT,	0, 0,
	NRT_FABRIC_WIDGETS | NRT_Fabric_pl301Wrap1_MSRWRALIMIT,		0, 0,
	NRT_FABRIC_WIDGETS | NRT_Fabric_pl301Wrap1_MSRRTRLIMIT,		(0x3f << 8) | (0x3f << 0), 0,
	NRT_FABRIC_WIDGETS | NRT_Fabric_pl301Wrap1_MSRWTRLIMIT,		(0x3f << 8) | (0x3f << 0), 0,
	NRT_FABRIC_WIDGETS | NRT_Fabric_pl301Wrap1_SDIORDRATELIMIT,	0, 0,
	NRT_FABRIC_WIDGETS | NRT_Fabric_pl301Wrap1_SDIOWRALIMIT,	0, 0,
	NRT_FABRIC_WIDGETS | NRT_Fabric_pl301Wrap1_SDIORTRLIMIT,	(0x3f << 8) | (0x3f << 0), 0,
	NRT_FABRIC_WIDGETS | NRT_Fabric_pl301Wrap1_SDIOWTRLIMIT,	(0x04 << 8) | (0x04 << 0), 0,
	// Need a read to make sure writes are pushed out to the fabric
	// before the following steps are executed.
	NRT_FABRIC_WIDGETS | NRT_Fabric_pl301Wrap1_SDIOWTRLIMIT | RECONFIG_RAM_CMD_READ,	(0x04 << 8) | (0x04 << 0), (0xf << 8) | (0xf << 0),

	// Reprogram the clock configuration to its expected value.
	// NOTE: We can't write PMGR_MSR_PS back to its original value here.
	//	 Attempting to do so will result in a hang.
	PMGR_REGISTERS     | PMGR_MEDIA_CLK_CFG,			0x83100000, 0,
	PMGR_REGISTERS     | PMGR_MEDIA_CLK_CFG | RECONFIG_RAM_CMD_READ,0, 0x40000000,
};

#endif	// APPLICATION_IBOOT

int miu_initialize_internal_ram(void)
{
#if APPLICATION_SECUREROM
	// Ensure that rPMGR_SCRATCH0-3 get cleared
	rPMGR_SCRATCH0 = 0;
	rPMGR_SCRATCH1 = 0;
	rPMGR_SCRATCH2 = 0;
	rPMGR_SCRATCH3 = 0;
#endif /* APPLICATION_SECUREROM */

	// Save the Security Epoch in the top byte of PMGR_SCRATCH0
	rPMGR_SCRATCH0 &= ~0xFF000000;
	rPMGR_SCRATCH0 |= (platform_get_security_epoch()) << 24;

	return 0;
}

int miu_init(void)
{
#if APPLICATION_IBOOT && !PRODUCT_IBEC
	// Verify that the Security Epoch in PMGR_SCRATCH0 matches
	if ((rPMGR_SCRATCH0 >> 24) != platform_get_security_epoch()) {
		panic("miu_init: Epoch Mismatch\n");
	}
#endif
#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	miu_configure_bridge(bridge_settings_static);
#endif

	return 0;
}

void miu_suspend(void)
{
	/* nothing required for suspend */
}

int miu_initialize_dram(bool resume)
{
#if APPLICATION_IBOOT && WITH_HW_AMC
	mcu_initialize_dram(resume);
#endif
	return 0;
}

void miu_select_remap(enum remap_select sel)
{
	switch (sel) {
		case REMAP_SRAM:
			rPIO_REMAP_CTL = (rPIO_REMAP_CTL & ~1) | (1 << 0);		// Resources mapped to address 0x0 are now remapped SPU_SRAM
			break;

		case REMAP_SDRAM:
			rFABRIC_REMAP_REG = (rFABRIC_REMAP_REG & ~1) | (1 << 0);	//  0x0000_0000 to 0x0010_0000 is mapped to DRAM
			break;

		// reset back to default behavior
		default:
			rPIO_REMAP_CTL = 0;
			rFABRIC_REMAP_REG = 0;
			break;
	}
}

void miu_bypass_prep(void)
{
}

#if (APPLICATION_IBOOT && (PRODUCT_IBSS || PRODUCT_LLB))
static void miu_configure_bridge(const u_int32_t *bridge_settings)
{
	volatile u_int32_t *reg;
	u_int32_t bridge, offset, data, mask;
	u_int32_t i;

	for (i = 0; i < ARRAY_SIZE(bridge_settings_static); i += 3) {
		bridge = bridge_settings[i] >> STATIC_BRIDGE_SHIFT;
		offset = bridge_settings[i] & STATIC_BRIDGE_OFFSET_MASK & ~RECONFIG_RAM_CMD_READ;
		data = bridge_settings[i + 1];
		reg = (volatile u_int32_t *)(bridge_registers[bridge] + offset);
		if (bridge_settings[i] & RECONFIG_RAM_CMD_READ) {
			mask = bridge_settings_static[i + 2];
			SPIN_W_TMO_UNTIL((*reg & mask) == data);
		} else {
			*reg = data;
		}
	}
}
#endif	// (APPLICATION_IBOOT && (PRODUCT_IBSS || PRODUCT_LLB))

#if APPLICATION_IBOOT
void miu_configure_bridge_soc_reconfig_ram(void)
{
	addr_t reg;
	u_int32_t bridge, offset, data, mask;
	u_int32_t i;

	for (i = 0; i < ARRAY_SIZE(bridge_settings_static); i += 3) {
		bridge = bridge_settings_static[i] >> STATIC_BRIDGE_SHIFT;
		offset = bridge_settings_static[i] & STATIC_BRIDGE_OFFSET_MASK;
		data = bridge_settings_static[i + 1];
		mask = bridge_settings_static[i + 2];
		reg = (addr_t)(bridge_registers[bridge] + offset);
		reconfig_append_command(RECONFIG_TYPE_SOC, reg, data, mask);
	}
}
#endif	// APPLICATION_IBOOT

#if WITH_DEVICETREE

void miu_update_device_tree(DTNode *pmgr_node)
{
	// Nothing to do here
}

#endif
