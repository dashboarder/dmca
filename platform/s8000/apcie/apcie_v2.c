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
#include SUB_PLATFORM_SPDS_HEADER(apcie_config)
#include SUB_PLATFORM_SPDS_HEADER(x2_pcie_rc)

#include SUB_PLATFORM_TUNABLE_HEADER(apcie_common)
#include SUB_PLATFORM_TUNABLE_HEADER(apcie_config)

#if SUB_PLATFORM_S8000
#include SUB_PLATFORM_SPDS_HEADER(ausp_shm)
#include SUB_PLATFORM_SPDS_HEADER(ausr_shm_cfg)
#include SUB_PLATFORM_SPDS_HEADER(ausr_cfg)
#include SUB_PLATFORM_SPDS_HEADER(aust_shm_cfg)
#include SUB_PLATFORM_TUNABLE_HEADER(ausr_cfg)
#include SUB_PLATFORM_TUNABLE_HEADER(ausr_shm_cfg)
#include SUB_PLATFORM_TUNABLE_HEADER(aust_shm_cfg)
#include SUB_PLATFORM_TUNABLE_HEADER(x1_pcie_rc)
#include SUB_PLATFORM_TUNABLE_HEADER(x2_pcie_rc)
#elif SUB_PLATFORM_S8001
#include SUB_PLATFORM_SPDS_HEADER(apcie_phy_glue)
#include SUB_PLATFORM_SPDS_HEADER(pma_cmn_registers)
#include SUB_PLATFORM_SPDS_HEADER(pma_rx_lane_registers)
#include SUB_PLATFORM_SPDS_HEADER(pma_tx_lane_registers)
#include SUB_PLATFORM_TUNABLE_HEADER(apcie_phy_glue)
#include SUB_PLATFORM_TUNABLE_HEADER(x2_pcie_rc)
#elif SUB_PLATFORM_S8003
#include SUB_PLATFORM_SPDS_HEADER(apcie_phy_glue)
#include SUB_PLATFORM_SPDS_HEADER(pma_cmn_registers)
#include SUB_PLATFORM_SPDS_HEADER(pma_rx_lane_registers)
#include SUB_PLATFORM_SPDS_HEADER(pma_tx_lane_registers)
#include SUB_PLATFORM_TUNABLE_HEADER(x1_pcie_rc)
#include SUB_PLATFORM_TUNABLE_HEADER(x2_pcie_rc)
#include SUB_PLATFORM_TUNABLE_HEADER(apcie_phy_glue)
#include SUB_PLATFORM_TUNABLE_HEADER(x1_pcie_rc)
#include SUB_PLATFORM_TUNABLE_HEADER(x2_pcie_rc)
#endif

#define MAX_DT_PROPERTY_SIZE	4096

////////////////////////////////////////////////////////////////////////////////

// Common tunables

#if	SUB_PLATFORM_S8000

// S8000 B0
static const struct tunable_struct common_tunable_b0[] = {
	PCIE_APCIE_COMMON_DEFAULT_TUNABLES_S8000_B0
};

// S8000 C0
static const struct tunable_struct common_tunable_c0[] = {
	PCIE_APCIE_COMMON_DEFAULT_TUNABLES_S8000_C0
};

// S8000 chip revision table
static const struct tunable_chip_struct tunables_pcie_common[] = {
	{CHIP_REVISION_C0, APCIE_COMMON_BASE_ADDR, common_tunable_c0, NULL, false},
	{CHIP_REVISION_B0, APCIE_COMMON_BASE_ADDR, common_tunable_b0, NULL, false},
};

#elif SUB_PLATFORM_S8001

// S8001 A0
static const struct tunable_struct common_tunable_a0[] = {
	PCIE_APCIE_COMMON_DEFAULT_TUNABLES_S8001_A0
};

// S8001 B0
static const struct tunable_struct common_tunable_b0[] = {
	PCIE_APCIE_COMMON_DEFAULT_TUNABLES_S8001_B0
};

// S8001 chip revision table
static const struct tunable_chip_struct tunables_pcie_common[] = {
	{CHIP_REVISION_B0, APCIE_COMMON_BASE_ADDR, common_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, APCIE_COMMON_BASE_ADDR, common_tunable_a0, NULL, false},
};

#elif	SUB_PLATFORM_S8003

// S8003 A0
static const struct tunable_struct common_tunable_a0[] = {
	PCIE_APCIE_COMMON_DEFAULT_TUNABLES_S8003_A0
};

// S8003 A1
static const struct tunable_struct common_tunable_a1[] = {
	PCIE_APCIE_COMMON_DEFAULT_TUNABLES_S8003_A1
};

// S8003 chip revision table
static const struct tunable_chip_struct tunables_pcie_common[] = {
	{CHIP_REVISION_A1, APCIE_COMMON_BASE_ADDR, common_tunable_a1, NULL, false},
	{CHIP_REVISION_A0, APCIE_COMMON_BASE_ADDR, common_tunable_a0, NULL, false},
};

#endif	// SUB_PLATFORM_S800x

////////////////////////////////////////////////////////////////////////////////

// Config tunables

#if	SUB_PLATFORM_S8000

// S8000 B0
static const struct tunable_struct port0_config_tunable_b0[] = {
	PCIE_APCIE_PORT0_APCIE_CONFIG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct port1_config_tunable_b0[] = {
	PCIE_APCIE_PORT1_APCIE_CONFIG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct port2_config_tunable_b0[] = {
	PCIE_APCIE_PORT2_APCIE_CONFIG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct port3_config_tunable_b0[] = {
	PCIE_APCIE_PORT3_APCIE_CONFIG_DEFAULT_TUNABLES_S8000_B0
};

// S8000 C0
static const struct tunable_struct port0_config_tunable_c0[] = {
	PCIE_APCIE_PORT0_APCIE_CONFIG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct port1_config_tunable_c0[] = {
	PCIE_APCIE_PORT1_APCIE_CONFIG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct port2_config_tunable_c0[] = {
	PCIE_APCIE_PORT2_APCIE_CONFIG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct port3_config_tunable_c0[] = {
	PCIE_APCIE_PORT3_APCIE_CONFIG_DEFAULT_TUNABLES_S8000_C0
};

// S8000 chip revision table
static const struct tunable_chip_struct tunables_pcie_config_port_0[] = {
	{CHIP_REVISION_C0, APCIE_PORT_BASE_ADDR(0), port0_config_tunable_c0, NULL, false},
	{CHIP_REVISION_B0, APCIE_PORT_BASE_ADDR(0), port0_config_tunable_b0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_1[] = {
	{CHIP_REVISION_C0, APCIE_PORT_BASE_ADDR(1), port1_config_tunable_c0, NULL, false},
	{CHIP_REVISION_B0, APCIE_PORT_BASE_ADDR(1), port1_config_tunable_b0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_2[] = {
	{CHIP_REVISION_C0, APCIE_PORT_BASE_ADDR(2), port2_config_tunable_c0, NULL, false},
	{CHIP_REVISION_B0, APCIE_PORT_BASE_ADDR(2), port2_config_tunable_b0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_3[] = {
	{CHIP_REVISION_C0, APCIE_PORT_BASE_ADDR(3), port3_config_tunable_c0, NULL, false},
	{CHIP_REVISION_B0, APCIE_PORT_BASE_ADDR(3), port3_config_tunable_b0, NULL, false},
};

// S8000 port chip revision table
static const struct tunable_chip_array tunables_pcie_config_ports[APCIE_NUM_LINKS] = {
	{ tunables_pcie_config_port_0, ARRAY_SIZE(tunables_pcie_config_port_0), 0 },
	{ tunables_pcie_config_port_1, ARRAY_SIZE(tunables_pcie_config_port_1), 0 },
	{ tunables_pcie_config_port_2, ARRAY_SIZE(tunables_pcie_config_port_2), 0 },
	{ tunables_pcie_config_port_3, ARRAY_SIZE(tunables_pcie_config_port_3), 0 },
};

#elif	SUB_PLATFORM_S8001

// S8001 A0
static const struct tunable_struct port0_config_tunable_a0[] = {
	PCIE_APCIE_PORT0_APCIE_CONFIG_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct port1_config_tunable_a0[] = {
	PCIE_APCIE_PORT1_APCIE_CONFIG_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct port2_config_tunable_a0[] = {
	PCIE_APCIE_PORT2_APCIE_CONFIG_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct port3_config_tunable_a0[] = {
	PCIE_APCIE_PORT3_APCIE_CONFIG_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct port4_config_tunable_a0[] = {
	PCIE_APCIE_PORT4_APCIE_CONFIG_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct port5_config_tunable_a0[] = {
	PCIE_APCIE_PORT5_APCIE_CONFIG_DEFAULT_TUNABLES_S8001_A0
};


// S8001 B0
static const struct tunable_struct port0_config_tunable_b0 [] = {
	PCIE_APCIE_PORT0_APCIE_CONFIG_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct port1_config_tunable_b0 [] = {
	PCIE_APCIE_PORT1_APCIE_CONFIG_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct port2_config_tunable_b0 [] = {
	PCIE_APCIE_PORT2_APCIE_CONFIG_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct port3_config_tunable_b0 [] = {
	PCIE_APCIE_PORT3_APCIE_CONFIG_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct port4_config_tunable_b0 [] = {
	PCIE_APCIE_PORT4_APCIE_CONFIG_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct port5_config_tunable_b0 [] = {
	PCIE_APCIE_PORT5_APCIE_CONFIG_DEFAULT_TUNABLES_S8001_B0
};

// S8001 chip revision table
static const struct tunable_chip_struct tunables_pcie_config_port_0[] = {
	{CHIP_REVISION_B0, APCIE_PORT_BASE_ADDR(0), port0_config_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(0), port0_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_1[] = {
	{CHIP_REVISION_B0, APCIE_PORT_BASE_ADDR(1), port1_config_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(1), port1_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_2[] = {
	{CHIP_REVISION_B0, APCIE_PORT_BASE_ADDR(2), port2_config_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(2), port2_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_3[] = {
	{CHIP_REVISION_B0, APCIE_PORT_BASE_ADDR(3), port3_config_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(3), port3_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_4[] = {
	{CHIP_REVISION_B0, APCIE_PORT_BASE_ADDR(4), port4_config_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(4), port4_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_5[] = {
	{CHIP_REVISION_B0, APCIE_PORT_BASE_ADDR(5), port5_config_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(5), port5_config_tunable_a0, NULL, false},
};

// S8001 port chip revision table
static const struct tunable_chip_array tunables_pcie_config_ports[APCIE_NUM_LINKS] = {
	{ tunables_pcie_config_port_0, ARRAY_SIZE(tunables_pcie_config_port_0), 0 },
	{ tunables_pcie_config_port_1, ARRAY_SIZE(tunables_pcie_config_port_1), 0 },
	{ tunables_pcie_config_port_2, ARRAY_SIZE(tunables_pcie_config_port_2), 0 },
	{ tunables_pcie_config_port_3, ARRAY_SIZE(tunables_pcie_config_port_3), 0 },
	{ tunables_pcie_config_port_4, ARRAY_SIZE(tunables_pcie_config_port_4), 0 },
	{ tunables_pcie_config_port_5, ARRAY_SIZE(tunables_pcie_config_port_5), 0 },
};

#elif	SUB_PLATFORM_S8003

// S8003 A0
static const struct tunable_struct port0_config_tunable_a0[] = {
	PCIE_APCIE_PORT0_APCIE_CONFIG_DEFAULT_TUNABLES_S8003_A0
};

static const struct tunable_struct port1_config_tunable_a0[] = {
	PCIE_APCIE_PORT1_APCIE_CONFIG_DEFAULT_TUNABLES_S8003_A0
};

static const struct tunable_struct port2_config_tunable_a0[] = {
	PCIE_APCIE_PORT2_APCIE_CONFIG_DEFAULT_TUNABLES_S8003_A0
};

static const struct tunable_struct port3_config_tunable_a0[] = {
	PCIE_APCIE_PORT3_APCIE_CONFIG_DEFAULT_TUNABLES_S8003_A0
};

// S8003 A1
static const struct tunable_struct port0_config_tunable_a1[] = {
	PCIE_APCIE_PORT0_APCIE_CONFIG_DEFAULT_TUNABLES_S8003_A1
};

static const struct tunable_struct port1_config_tunable_a1[] = {
	PCIE_APCIE_PORT1_APCIE_CONFIG_DEFAULT_TUNABLES_S8003_A1
};

static const struct tunable_struct port2_config_tunable_a1[] = {
	PCIE_APCIE_PORT2_APCIE_CONFIG_DEFAULT_TUNABLES_S8003_A1
};

static const struct tunable_struct port3_config_tunable_a1[] = {
	PCIE_APCIE_PORT3_APCIE_CONFIG_DEFAULT_TUNABLES_S8003_A1
};


// S8003 chip revision table
static const struct tunable_chip_struct tunables_pcie_config_port_0[] = {
	{CHIP_REVISION_A1, APCIE_PORT_BASE_ADDR(0), port0_config_tunable_a1, NULL, false},
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(0), port0_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_1[] = {
	{CHIP_REVISION_A1, APCIE_PORT_BASE_ADDR(1), port1_config_tunable_a1, NULL, false},
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(1), port1_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_2[] = {
	{CHIP_REVISION_A1, APCIE_PORT_BASE_ADDR(2), port2_config_tunable_a1, NULL, false},
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(2), port2_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_port_3[] = {
	{CHIP_REVISION_A1, APCIE_PORT_BASE_ADDR(3), port3_config_tunable_a1, NULL, false},
	{CHIP_REVISION_A0, APCIE_PORT_BASE_ADDR(3), port3_config_tunable_a0, NULL, false},
};

// S8003 port chip revision table
static const struct tunable_chip_array tunables_pcie_config_ports[APCIE_NUM_LINKS] = {
	{ tunables_pcie_config_port_0, ARRAY_SIZE(tunables_pcie_config_port_0), 0 },
	{ tunables_pcie_config_port_1, ARRAY_SIZE(tunables_pcie_config_port_1), 0 },
	{ tunables_pcie_config_port_2, ARRAY_SIZE(tunables_pcie_config_port_2), 0 },
	{ tunables_pcie_config_port_3, ARRAY_SIZE(tunables_pcie_config_port_3), 0 },
};

#endif	// SUB_PLATFORM_S800x

////////////////////////////////////////////////////////////////////////////////

// PCIe root complex tunables

#if	SUB_PLATFORM_S8000

// S8000 B0
static const struct tunable_struct rc0_config_tunable_b0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC0_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct rc1_config_tunable_b0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC1_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct rc2_config_tunable_b0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC2_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct rc3_config_tunable_b0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC3_DEFAULT_TUNABLES_S8000_B0
};

// S8000 C0
static const struct tunable_struct rc0_config_tunable_c0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC0_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct rc1_config_tunable_c0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC1_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct rc2_config_tunable_c0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC2_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct rc3_config_tunable_c0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC3_DEFAULT_TUNABLES_S8000_C0
};

// S8000 chip revision table
static const struct tunable_chip_struct tunables_pcie_config_rc_0[] = {
	{CHIP_REVISION_C0, PCIE_PORT_BASE_ADDR(0), rc0_config_tunable_c0, NULL, false},
	{CHIP_REVISION_B0, PCIE_PORT_BASE_ADDR(0), rc0_config_tunable_b0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_1[] = {
	{CHIP_REVISION_C0, PCIE_PORT_BASE_ADDR(1), rc1_config_tunable_c0, NULL, false},
	{CHIP_REVISION_B0, PCIE_PORT_BASE_ADDR(1), rc1_config_tunable_b0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_2[] = {
	{CHIP_REVISION_C0, PCIE_PORT_BASE_ADDR(2), rc2_config_tunable_c0, NULL, false},
	{CHIP_REVISION_B0, PCIE_PORT_BASE_ADDR(2), rc2_config_tunable_b0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_3[] = {
	{CHIP_REVISION_C0, PCIE_PORT_BASE_ADDR(3), rc3_config_tunable_c0, NULL, false},
	{CHIP_REVISION_B0, PCIE_PORT_BASE_ADDR(3), rc3_config_tunable_b0, NULL, false},
};

// S8000 port chip revision table
static const struct tunable_chip_array tunables_pcie_config_rc[APCIE_NUM_LINKS] = {
	{ tunables_pcie_config_rc_0, ARRAY_SIZE(tunables_pcie_config_rc_0), 0 },
	{ tunables_pcie_config_rc_1, ARRAY_SIZE(tunables_pcie_config_rc_1), 0 },
	{ tunables_pcie_config_rc_2, ARRAY_SIZE(tunables_pcie_config_rc_2), 0 },
	{ tunables_pcie_config_rc_3, ARRAY_SIZE(tunables_pcie_config_rc_3), 0 },
};

#elif	SUB_PLATFORM_S8001

// S8001 A0
static const struct tunable_struct rc0_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC0_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct rc1_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC1_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct rc2_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC2_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct rc3_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC3_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct rc4_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC4_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct rc5_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC5_DEFAULT_TUNABLES_S8001_A0
};


// S8001 B0
static const struct tunable_struct rc0_config_tunable_b0 [] = {
	PCIE_PCIE_CONFIG_PCIE_RC0_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct rc1_config_tunable_b0 [] = {
	PCIE_PCIE_CONFIG_PCIE_RC1_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct rc2_config_tunable_b0 [] = {
	PCIE_PCIE_CONFIG_PCIE_RC2_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct rc3_config_tunable_b0 [] = {
	PCIE_PCIE_CONFIG_PCIE_RC3_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct rc4_config_tunable_b0 [] = {
	PCIE_PCIE_CONFIG_PCIE_RC4_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct rc5_config_tunable_b0 [] = {
	PCIE_PCIE_CONFIG_PCIE_RC5_DEFAULT_TUNABLES_S8001_B0
};

// S8001 chip revision table
static const struct tunable_chip_struct tunables_pcie_config_rc_0[] = {
	{CHIP_REVISION_B0, PCIE_PORT_BASE_ADDR(0), rc0_config_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(0), rc0_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_1[] = {
	{CHIP_REVISION_B0, PCIE_PORT_BASE_ADDR(1), rc1_config_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(1), rc1_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_2[] = {
	{CHIP_REVISION_B0, PCIE_PORT_BASE_ADDR(2), rc2_config_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(2), rc2_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_3[] = {
	{CHIP_REVISION_B0, PCIE_PORT_BASE_ADDR(3), rc3_config_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(3), rc3_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_4[] = {
	{CHIP_REVISION_B0, PCIE_PORT_BASE_ADDR(4), rc4_config_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(4), rc4_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_5[] = {
	{CHIP_REVISION_B0, PCIE_PORT_BASE_ADDR(5), rc5_config_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(5), rc5_config_tunable_a0, NULL, false},
};

// S8001 port chip revision table
static const struct tunable_chip_array tunables_pcie_config_rc[APCIE_NUM_LINKS] = {
	{ tunables_pcie_config_rc_0, ARRAY_SIZE(tunables_pcie_config_rc_0), 0 },
	{ tunables_pcie_config_rc_1, ARRAY_SIZE(tunables_pcie_config_rc_1), 0 },
	{ tunables_pcie_config_rc_2, ARRAY_SIZE(tunables_pcie_config_rc_2), 0 },
	{ tunables_pcie_config_rc_3, ARRAY_SIZE(tunables_pcie_config_rc_3), 0 },
	{ tunables_pcie_config_rc_4, ARRAY_SIZE(tunables_pcie_config_rc_4), 0 },
	{ tunables_pcie_config_rc_5, ARRAY_SIZE(tunables_pcie_config_rc_5), 0 },
};

#elif	SUB_PLATFORM_S8003

// S8003 A0
static const struct tunable_struct rc0_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC0_DEFAULT_TUNABLES_S8003_A0
};

static const struct tunable_struct rc1_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC1_DEFAULT_TUNABLES_S8003_A0
};

static const struct tunable_struct rc2_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC2_DEFAULT_TUNABLES_S8003_A0
};

static const struct tunable_struct rc3_config_tunable_a0[] = {
	PCIE_PCIE_CONFIG_PCIE_RC3_DEFAULT_TUNABLES_S8003_A0
};

// S8003 A1
static const struct tunable_struct rc0_config_tunable_a1[] = {
	PCIE_PCIE_CONFIG_PCIE_RC0_DEFAULT_TUNABLES_S8003_A1
};

static const struct tunable_struct rc1_config_tunable_a1[] = {
	PCIE_PCIE_CONFIG_PCIE_RC1_DEFAULT_TUNABLES_S8003_A1
};

static const struct tunable_struct rc2_config_tunable_a1[] = {
	PCIE_PCIE_CONFIG_PCIE_RC2_DEFAULT_TUNABLES_S8003_A1
};

static const struct tunable_struct rc3_config_tunable_a1[] = {
	PCIE_PCIE_CONFIG_PCIE_RC3_DEFAULT_TUNABLES_S8003_A1
};


// S8003 chip revision table
static const struct tunable_chip_struct tunables_pcie_config_rc_0[] = {
	{CHIP_REVISION_A1, PCIE_PORT_BASE_ADDR(0), rc0_config_tunable_a1, NULL, false},
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(0), rc0_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_1[] = {
	{CHIP_REVISION_A1, PCIE_PORT_BASE_ADDR(1), rc1_config_tunable_a1, NULL, false},
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(1), rc1_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_2[] = {
	{CHIP_REVISION_A1, PCIE_PORT_BASE_ADDR(2), rc2_config_tunable_a1, NULL, false},
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(2), rc2_config_tunable_a0, NULL, false},
};

static const struct tunable_chip_struct tunables_pcie_config_rc_3[] = {
	{CHIP_REVISION_A1, PCIE_PORT_BASE_ADDR(3), rc3_config_tunable_a1, NULL, false},
	{CHIP_REVISION_A0, PCIE_PORT_BASE_ADDR(3), rc3_config_tunable_a0, NULL, false},
};

// S8003 port chip revision table
static const struct tunable_chip_array tunables_pcie_config_rc[APCIE_NUM_LINKS] = {
	{ tunables_pcie_config_rc_0, ARRAY_SIZE(tunables_pcie_config_rc_0), 0 },
	{ tunables_pcie_config_rc_1, ARRAY_SIZE(tunables_pcie_config_rc_1), 0 },
	{ tunables_pcie_config_rc_2, ARRAY_SIZE(tunables_pcie_config_rc_2), 0 },
	{ tunables_pcie_config_rc_3, ARRAY_SIZE(tunables_pcie_config_rc_3), 0 },
};

#endif	// SUB_PLATFORM_S800x

////////////////////////////////////////////////////////////////////////////////

// PHY tunables

#if SUB_PLATFORM_S8000

// S8000 ALL
/* Maui tunables spec section 6.6: PCIE Tx De-emphasis and Rx Equalization Settings */
#define PCIE_APCIE_PHY_AUST0_SHM_CFG_DEFAULT_TUNABLES_S8000_ALL_EXTRA \
{ AUST_SHM_CFG_BLK_CFG2_TXA_OFFSET, sizeof(uint32_t), 0xffffffff, 0x0000212a},

#define PCIE_APCIE_PHY_AUSR0_SHM_CFG_DEFAULT_TUNABLES_S8000_ALL_EXTRA \
{ AUSR_SHM_CFG_BLK_RXA_CFG1_OFFSET, sizeof(uint32_t), 0xffffffff, 0x00000520},

#define PCIE_APCIE_PHY_AUST1_SHM_CFG_DEFAULT_TUNABLES_S8000_ALL_EXTRA \
{ AUST_SHM_CFG_BLK_CFG2_TXA_OFFSET, sizeof(uint32_t), 0xffffffff, 0x0000212a},

#define PCIE_APCIE_PHY_AUSR1_SHM_CFG_DEFAULT_TUNABLES_S8000_ALL_EXTRA \
{ AUSR_SHM_CFG_BLK_RXA_CFG1_OFFSET, sizeof(uint32_t), 0xffffffff, 0x00000520},

// S8000 B0
static const struct tunable_struct phy_aust0_shm_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUST0_SHM_CFG_DEFAULT_TUNABLES_S8000_ALL_EXTRA
	PCIE_APCIE_PHY_AUST0_SHM_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_aust1_shm_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUST1_SHM_CFG_DEFAULT_TUNABLES_S8000_ALL_EXTRA
	PCIE_APCIE_PHY_AUST1_SHM_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_aust2_shm_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUST2_SHM_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_aust3_shm_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUST3_SHM_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_aust4_shm_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUST4_SHM_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_ausr0_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUSR0_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_ausr1_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUSR1_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_ausr2_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUSR2_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_ausr3_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUSR3_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_ausr4_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUSR4_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_ausr0_shm_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUSR0_SHM_CFG_DEFAULT_TUNABLES_S8000_ALL_EXTRA
	PCIE_APCIE_PHY_AUSR0_SHM_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_ausr1_shm_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUSR1_SHM_CFG_DEFAULT_TUNABLES_S8000_ALL_EXTRA
	PCIE_APCIE_PHY_AUSR1_SHM_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_ausr2_shm_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUSR2_SHM_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_ausr3_shm_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUSR3_SHM_CFG_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct phy_ausr4_shm_cfg_tunable_b0[] = {
	PCIE_APCIE_PHY_AUSR4_SHM_CFG_DEFAULT_TUNABLES_S8000_B0
};


// S8000 C0
static const struct tunable_struct phy_aust0_shm_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUST0_SHM_CFG_DEFAULT_TUNABLES_S8000_ALL_EXTRA
	PCIE_APCIE_PHY_AUST0_SHM_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_aust1_shm_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUST1_SHM_CFG_DEFAULT_TUNABLES_S8000_ALL_EXTRA
	PCIE_APCIE_PHY_AUST1_SHM_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_aust2_shm_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUST2_SHM_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_aust3_shm_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUST3_SHM_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_aust4_shm_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUST4_SHM_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_ausr0_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUSR0_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_ausr1_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUSR1_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_ausr2_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUSR2_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_ausr3_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUSR3_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_ausr4_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUSR4_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_ausr0_shm_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUSR0_SHM_CFG_DEFAULT_TUNABLES_S8000_ALL_EXTRA
	PCIE_APCIE_PHY_AUSR0_SHM_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_ausr1_shm_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUSR1_SHM_CFG_DEFAULT_TUNABLES_S8000_ALL_EXTRA
	PCIE_APCIE_PHY_AUSR1_SHM_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_ausr2_shm_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUSR2_SHM_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_ausr3_shm_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUSR3_SHM_CFG_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct phy_ausr4_shm_cfg_tunable_c0[] = {
	PCIE_APCIE_PHY_AUSR4_SHM_CFG_DEFAULT_TUNABLES_S8000_C0
};

// Must be sorted from newest to oldest chip revision.
static const struct tunable_chip_struct tunables_pcie_phy[] = {
	{CHIP_REVISION_C0, APCIE_PHY_AUST0_SHM_CFG_BASE_ADDR, phy_aust0_shm_cfg_tunable_c0, NULL, false},
	{CHIP_REVISION_C0, APCIE_PHY_AUST1_SHM_CFG_BASE_ADDR, phy_aust1_shm_cfg_tunable_c0, NULL, false},
	{CHIP_REVISION_C0, APCIE_PHY_AUST2_SHM_CFG_BASE_ADDR, phy_aust2_shm_cfg_tunable_c0, NULL, false},
	{CHIP_REVISION_C0, APCIE_PHY_AUST3_SHM_CFG_BASE_ADDR, phy_aust3_shm_cfg_tunable_c0, NULL, false},
	{CHIP_REVISION_C0, APCIE_PHY_AUST4_SHM_CFG_BASE_ADDR, phy_aust4_shm_cfg_tunable_c0, NULL, false},

	{CHIP_REVISION_C0, APCIE_PHY_AUSR0_CFG_BASE_ADDR,     phy_ausr0_cfg_tunable_c0,     NULL, false},
	{CHIP_REVISION_C0, APCIE_PHY_AUSR1_CFG_BASE_ADDR,     phy_ausr1_cfg_tunable_c0,     NULL, false},
	{CHIP_REVISION_C0, APCIE_PHY_AUSR2_CFG_BASE_ADDR,     phy_ausr2_cfg_tunable_c0,     NULL, false},
	{CHIP_REVISION_C0, APCIE_PHY_AUSR3_CFG_BASE_ADDR,     phy_ausr3_cfg_tunable_c0,     NULL, false},
	{CHIP_REVISION_C0, APCIE_PHY_AUSR4_CFG_BASE_ADDR,     phy_ausr4_cfg_tunable_c0,     NULL, false},

	{CHIP_REVISION_C0, APCIE_PHY_AUSR0_SHM_CFG_BASE_ADDR, phy_ausr0_shm_cfg_tunable_c0, NULL, false},
	{CHIP_REVISION_C0, APCIE_PHY_AUSR1_SHM_CFG_BASE_ADDR, phy_ausr1_shm_cfg_tunable_c0, NULL, false},
	{CHIP_REVISION_C0, APCIE_PHY_AUSR2_SHM_CFG_BASE_ADDR, phy_ausr2_shm_cfg_tunable_c0, NULL, false},
	{CHIP_REVISION_C0, APCIE_PHY_AUSR3_SHM_CFG_BASE_ADDR, phy_ausr3_shm_cfg_tunable_c0, NULL, false},
	{CHIP_REVISION_C0, APCIE_PHY_AUSR4_SHM_CFG_BASE_ADDR, phy_ausr4_shm_cfg_tunable_c0, NULL, false},

	{CHIP_REVISION_B0, APCIE_PHY_AUST0_SHM_CFG_BASE_ADDR, phy_aust0_shm_cfg_tunable_b0, NULL, false},
	{CHIP_REVISION_B0, APCIE_PHY_AUST1_SHM_CFG_BASE_ADDR, phy_aust1_shm_cfg_tunable_b0, NULL, false},
	{CHIP_REVISION_B0, APCIE_PHY_AUST2_SHM_CFG_BASE_ADDR, phy_aust2_shm_cfg_tunable_b0, NULL, false},
	{CHIP_REVISION_B0, APCIE_PHY_AUST3_SHM_CFG_BASE_ADDR, phy_aust3_shm_cfg_tunable_b0, NULL, false},
	{CHIP_REVISION_B0, APCIE_PHY_AUST4_SHM_CFG_BASE_ADDR, phy_aust4_shm_cfg_tunable_b0, NULL, false},

	{CHIP_REVISION_B0, APCIE_PHY_AUSR0_CFG_BASE_ADDR,     phy_ausr0_cfg_tunable_b0,     NULL, false},
	{CHIP_REVISION_B0, APCIE_PHY_AUSR1_CFG_BASE_ADDR,     phy_ausr1_cfg_tunable_b0,     NULL, false},
	{CHIP_REVISION_B0, APCIE_PHY_AUSR2_CFG_BASE_ADDR,     phy_ausr2_cfg_tunable_b0,     NULL, false},
	{CHIP_REVISION_B0, APCIE_PHY_AUSR3_CFG_BASE_ADDR,     phy_ausr3_cfg_tunable_b0,     NULL, false},
	{CHIP_REVISION_B0, APCIE_PHY_AUSR4_CFG_BASE_ADDR,     phy_ausr4_cfg_tunable_b0,     NULL, false},

	{CHIP_REVISION_B0, APCIE_PHY_AUSR0_SHM_CFG_BASE_ADDR, phy_ausr0_shm_cfg_tunable_b0, NULL, false},
	{CHIP_REVISION_B0, APCIE_PHY_AUSR1_SHM_CFG_BASE_ADDR, phy_ausr1_shm_cfg_tunable_b0, NULL, false},
	{CHIP_REVISION_B0, APCIE_PHY_AUSR2_SHM_CFG_BASE_ADDR, phy_ausr2_shm_cfg_tunable_b0, NULL, false},
	{CHIP_REVISION_B0, APCIE_PHY_AUSR3_SHM_CFG_BASE_ADDR, phy_ausr3_shm_cfg_tunable_b0, NULL, false},
	{CHIP_REVISION_B0, APCIE_PHY_AUSR4_SHM_CFG_BASE_ADDR, phy_ausr4_shm_cfg_tunable_b0, NULL, false},
};

// S0000 B0/C0 fuse-based PHY tunables updated at runtime.
static struct tunable_struct phy_values_efuse[] = {
	{ AUSP_SHM_BLK_CFG_BIAS_OFFSET,
	  sizeof(uint32_t),
	  AUSP_SHM_BLK_CFG_BIAS_CURRENT_CAL_RX_UMASK,
	  0						// Value updated at runtime
	},
	{ AUSP_SHM_BLK_PLL_VCO_CFG_OFFSET,
	  sizeof(uint32_t),
	  AUSP_SHM_BLK_PLL_VCO_CFG_PLL_V2I_I_SET_UMASK | AUSP_SHM_BLK_PLL_VCO_CFG_PLL_V2I_PI_SET_UMASK,
	  0						// Value updated at runtime
	},
	TUNABLE_TABLE_END_MARKER
};

static const struct tunable_chip_struct tunables_pcie_phy_efuse[] = {
	{CHIP_REVISION_C0, APCIE_PHY_AUSP_SHM_BASE_ADDR, phy_values_efuse, NULL, false},
	{CHIP_REVISION_B0, APCIE_PHY_AUSP_SHM_BASE_ADDR, phy_values_efuse, NULL, false},
};

#elif	SUB_PLATFORM_S8001

// S8001 A0
static const struct tunable_struct phy_glue_tunable_a0[] = {
	PCIE_APCIE_PHY_GLUE_DEFAULT_TUNABLES_S8001_A0
};

// S8001 B0
static const struct tunable_struct phy_glue_tunable_b0[] = {
	PCIE_APCIE_PHY_GLUE_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_chip_struct tunables_pcie_phy[] = {
	{CHIP_REVISION_B0, APCIE_PHY_BASE_ADDR, phy_glue_tunable_b0, NULL, false},
	{CHIP_REVISION_A0, APCIE_PHY_BASE_ADDR, phy_glue_tunable_a0, NULL, false},
};

#elif	SUB_PLATFORM_S8003

// S8003 A0
static const struct tunable_struct phy_glue_tunable_a0[] = {
	PCIE_APCIE_PHY_GLUE_DEFAULT_TUNABLES_S8003_A0
};

// S8003 A1
static const struct tunable_struct phy_glue_tunable_a1[] = {
	PCIE_APCIE_PHY_GLUE_DEFAULT_TUNABLES_S8003_A1
};

static const struct tunable_chip_struct tunables_pcie_phy[] = {
	{CHIP_REVISION_A1, APCIE_PHY_BASE_ADDR, phy_glue_tunable_a1, NULL, false},
	{CHIP_REVISION_A0, APCIE_PHY_BASE_ADDR, phy_glue_tunable_a0, NULL, false},
};

#endif	// SUB_PLATFORM_S800x

////////////////////////////////////////////////////////////////////////////////

static bool s3e_mode;
static uint32_t link_enable_count;
static bool s3e_reset_on_enable_disable;

#define PORT_STRIDE			(APCIE_COMMON_BLK_APCIE_PORT_CTRL_1_OFFSET - APCIE_COMMON_BLK_APCIE_PORT_CTRL_0_OFFSET)

#define SET_REG(reg, value)		set_reg(APCIE_COMMON_BASE_ADDR, APCIE_COMMON_BLK_APCIE_ ## reg ## _OFFSET, (value))
#define GET_REG(reg)			get_reg(APCIE_COMMON_BASE_ADDR, APCIE_COMMON_BLK_APCIE_ ## reg ## _OFFSET)
#define OR_REG(reg, value)		or_reg(APCIE_COMMON_BASE_ADDR, APCIE_COMMON_BLK_APCIE_ ## reg ## _OFFSET, (value))
#define AND_REG(reg, value)		and_reg(APCIE_COMMON_BASE_ADDR, APCIE_COMMON_BLK_APCIE_ ## reg ## _OFFSET, (value))

#define PORT_REG(reg, port)		(APCIE_COMMON_BLK_APCIE_ ## reg ## _0_OFFSET) + (PORT_STRIDE * (port))
#define SET_PORT_REG(reg, port, value)	set_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port), value)
#define GET_PORT_REG(reg, port)		get_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port))
#define OR_PORT_REG(reg, port, value)	or_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port), value)
#define AND_PORT_REG(reg, port, value)	and_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port), value)

#define PORT_REG_BIT_SHIFT(reg, bit)	(APCIE_COMMON_BLK_APCIE_ ## reg ## _0_ ## bit ## _SHIFT)
#define SET_PORT_REG_BIT(reg, port, b)	or_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port), 1 << PORT_REG_BIT_SHIFT(reg, b))
#define CLR_PORT_REG_BIT(reg, port, b)	and_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port), ~(1 << PORT_REG_BIT_SHIFT(reg, b)))
#define GET_PORT_REG_BIT(reg, port, b)	((get_reg(APCIE_COMMON_BASE_ADDR, PORT_REG(reg, port)) >> PORT_REG_BIT_SHIFT(reg, b)) & 1)

#define PHY_SET_REG(reg, value)		set_reg(APCIE_PHY_BASE_ADDR, APCIE_PHY_GLUE_BLK_ ## reg ## _OFFSET, (value))
#define PHY_GET_REG(reg)		get_reg(APCIE_PHY_BASE_ADDR, APCIE_PHY_GLUE_BLK_ ## reg ## _OFFSET)
#define PHY_OR_REG(reg, value)		or_reg(APCIE_PHY_BASE_ADDR, APCIE_PHY_GLUE_BLK_ ## reg ## _OFFSET, (value))
#define PHY_AND_REG(reg, value)		and_reg(APCIE_PHY_BASE_ADDR, APCIE_PHY_GLUE_BLK_ ## reg ## _OFFSET, (value))

#if SUB_PLATFORM_S8001 || SUB_PLATFORM_S8003
#define SET_PMA_CMN_REG(reg, value)	set_reg(APCIE_PHY_PMA_COMMON_BASE_ADDR, PMA_CMN_REGISTERS_BLK_ ## reg ## _OFFSET, (value))
#define GET_PMA_CMN_REG(reg)		get_reg(APCIE_PHY_PMA_COMMON_BASE_ADDR, PMA_CMN_REGISTERS_BLK_ ## reg ## _OFFSET)
#define OR_PMA_CMN_REG(reg, value)	or_reg(APCIE_PHY_PMA_COMMON_BASE_ADDR, PMA_CMN_REGISTERS_BLK_ ## reg ## _OFFSET, (value))
#define AND_PMA_CMN_REG(reg, value)	and_reg(APCIE_PHY_PMA_COMMON_BASE_ADDR, PMA_CMN_REGISTERS_BLK_ ## reg ## _OFFSET, (value))

#define PMA_RX_REG(reg, lane)		 (PMA_RX_LANE_REGISTERS_BLK_ ## reg ## _OFFSET) + (APCIE_PHY_PMA_RX_LANE_STRIDE * (lane))
#define SET_PMA_RX_REG(reg, lane, value) set_reg(APCIE_PHY_PMA_RX_LANE_BASE_ADDR, PMA_RX_REG(reg, lane), value)
#define GET_PMA_RX_REG(reg, lane)	 get_reg(APCIE_PHY_PMA_RX_LANE_BASE_ADDR, PMA_RX_REG(reg, lane))
#define OR_PMA_RX_REG(reg, lane, value)  or_reg(APCIE_PHY_PMA_RX_LANE_BASE_ADDR, PMA_RX_REG(reg, lane), value)
#define AND_PMA_RX_REG(reg, lane, value) and_reg(APCIE_PHY_PMA_RX_LANE_BASE_ADDR, PMA_RX_REG(reg, lane), value)

#define PMA_TX_REG(reg, lane)		 (PMA_TX_LANE_REGISTERS_BLK_ ## reg ## _OFFSET) + (APCIE_PHY_PMA_TX_LANE_STRIDE * (lane))
#define SET_PMA_TX_REG(reg, lane, value) set_reg(APCIE_PHY_PMA_TX_LANE_BASE_ADDR, PMA_TX_REG(reg, lane), value)
#define GET_PMA_TX_REG(reg, lane)	 get_reg(APCIE_PHY_PMA_TX_LANE_BASE_ADDR, PMA_TX_REG(reg, lane))
#define OR_PMA_TX_REG(reg, lane, value)  or_reg(APCIE_PHY_PMA_TX_LANE_BASE_ADDR, PMA_TX_REG(reg, lane), value)
#define AND_PMA_TX_REG(reg, lane, value) and_reg(APCIE_PHY_PMA_TX_LANE_BASE_ADDR, PMA_TX_REG(reg, lane), value)
#endif

static void set_reg(uint64_t base, uint32_t offset, uint32_t value)
{
	dprintf(DEBUG_SPEW, "apcie: set_reg[0x%llx] = 0x%x\n", base + offset, value);
	*(volatile uint32_t *)(base + offset) = value;
}

static uint32_t get_reg(uint64_t base, uint32_t offset)
{
	uint32_t value;
	
	dprintf(DEBUG_SPEW, "acpie: get_reg[0x%llx]", base + offset);
	value = *(volatile uint32_t *)(base + offset);

	dprintf(DEBUG_SPEW, " = 0x%x\n", value);
	return value;
}

static void or_reg(uint64_t base, uint32_t offset, uint32_t value)
{
	uint32_t orig;

	orig = get_reg(base, offset);
	set_reg(base, offset, orig | value);
}

static void and_reg(uint64_t base, uint32_t offset, uint32_t value)
{
	uint32_t orig;

	orig = get_reg(base, offset);
	set_reg(base, offset, orig & value);
}


#if SUB_PLATFORM_S8000
static uint32_t get_ausp_shm_reg(uint32_t offset)
{
	uint32_t value;

	value = *(volatile uint32_t  *)(APCIE_PHY_AUSP_SHM_BASE_ADDR + offset);

	return value;
}

static void set_ausp_shm_reg(uint32_t offset, uint32_t value)
{
	*(volatile uint32_t *)(APCIE_PHY_AUSP_SHM_BASE_ADDR + offset) = value;
}
#endif

void apcie_set_s3e_mode(bool reset_on_enable)
{
	s3e_mode = true;
	s3e_reset_on_enable_disable = reset_on_enable;
}

static void apcie_enable_root_complex(void)
{
	// 1. Turn on PCIE in PCIE_PS register
	clock_gate(CLK_PCIE, true);
	// 2. Enable the 24Mhz clock
	clock_gate(CLK_PCIE_AUX, true);
	// 4. Enable the ref clock
	clock_gate(CLK_PCIE_REF, true);
	// 5. Enable the PCIe links
	clock_gate(CLK_PCIE_LINK0, true);
	clock_gate(CLK_PCIE_LINK1, true);
	clock_gate(CLK_PCIE_LINK2, true);
	clock_gate(CLK_PCIE_LINK3, true);
#if SUB_PLATFORM_S8001
	clock_gate(CLK_PCIE_LINK4, true);
	clock_gate(CLK_PCIE_LINK5, true);
#endif

	// Apply common tunables.
	platform_apply_tunables(tunables_pcie_common, ARRAY_SIZE(tunables_pcie_common), "PCIe common");

	if (s3e_mode) {
		
		if (s3e_reset_on_enable_disable) {
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
			AND_REG(NANDRST_CTRL, ~1);
#elif SUB_PLATFORM_S8001
			AND_PORT_REG(NANDRST_CTRL, 0, ~1);
#endif
		}

		gpio_configure(GPIO_NAND_SYS_CLK, GPIO_CFG_FUNC0);
		OR_REG(NANDSYSCLK_CTRL, 1);
		// S3e spec requires 32 24 Mhz clock cycles from NANDSYSCLK to reset deassertion.
		// But, APCIe spec requires we wait 100 microseconds (Maui_aPCIe_Spec section 5.4)
		spin(100);

		// De-assert S3E_RESET_N if it's already enabled. Except when platform specified
		// s3e_reset_on_enable_disable (for example, in iBEC), we only de-assert the reset, and
		// then leave it de-asserted
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
			OR_REG(NANDRST_CTRL, 1);
#elif SUB_PLATFORM_S8001
			OR_PORT_REG(NANDRST_CTRL, 0, 1);
#endif
	}

	// 7.Select APCIE lane configuration
	SET_REG(LANE_CFG, (1 << 4) | platform_get_apcie_lane_cfg());

	// 8. PCIe Phy is powered up:
	// APCIE_PHY_POWERUP_STS.powerup_done is set by the HW when Phy power-up sequence is completed
	while((GET_REG(PHY_POWERUP_STS) & 1) == 0)
		spin(1);

	// 9. Software de-asserts phy_apb_reset_n
	OR_REG(PHY_CONFIG_RST_CTRL, 1);

	// Update the fuse-based PHY tunables.
#if SUB_PLATFORM_S8000
	phy_values_efuse[0].value = AUSP_SHM_BLK_CFG_BIAS_CURRENT_CAL_RX_INSRT(chipid_get_pcie_rx_ldo());
	phy_values_efuse[1].value = AUSP_SHM_BLK_PLL_VCO_CFG_PLL_V2I_I_SET_INSRT(chipid_get_pcie_txpll_vco_v2i_i_set()) |
				    AUSP_SHM_BLK_PLL_VCO_CFG_PLL_V2I_PI_SET_INSRT(chipid_get_pcie_txpll_vco_v2i_pi_set());

	// Apply fuse-based tunable values
	platform_apply_tunables(tunables_pcie_phy_efuse, ARRAY_SIZE(tunables_pcie_phy_efuse), "PCIe PHY fused");
#endif

	// Apply SPDS-generated tunable values
	platform_apply_tunables(tunables_pcie_phy, ARRAY_SIZE(tunables_pcie_phy), "PCIe PHY");

	// 11. When internal REFCLK is selected, SW must ensure that REFCLK provided by PCIe
	//     REFCLK PLL is stable before proceeding to link bring-up
	while ((GET_REG(PHY_POWERUP_STS) & (1 << 4)) == 0)
		spin(1);
}

static void apcie_disable_root_complex(void)
{
	if (s3e_reset_on_enable_disable) {
#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
		AND_REG(NANDRST_CTRL, ~1);
#elif SUB_PLATFORM_S8001
		AND_PORT_REG(NANDRST_CTRL, 0, ~1);
#endif
	}

	// See <rdar://problem/16006506> for related discussion on the link resets
	// We need to assert reset to the individual link power domains, and then to the main one
	clock_set_device_reset(CLK_PCIE_LINK0, true);
	clock_set_device_reset(CLK_PCIE_LINK1, true);
	clock_set_device_reset(CLK_PCIE_LINK2, true);
	clock_set_device_reset(CLK_PCIE_LINK3, true);
#if SUB_PLATFORM_S8001
	clock_set_device_reset(CLK_PCIE_LINK4, true);
	clock_set_device_reset(CLK_PCIE_LINK5, true);
#endif
	clock_set_device_reset(CLK_PCIE, true);

	spin(10);

	clock_gate(CLK_PCIE_LINK0, false);
	clock_gate(CLK_PCIE_LINK1, false);
	clock_gate(CLK_PCIE_LINK2, false);
	clock_gate(CLK_PCIE_LINK3, false);
#if SUB_PLATFORM_S8001
	clock_gate(CLK_PCIE_LINK4, false);
	clock_gate(CLK_PCIE_LINK5, false);
#endif
	clock_gate(CLK_PCIE_AUX, false);
	clock_gate(CLK_PCIE_REF, false);
	clock_gate(CLK_PCIE, false);

	clock_set_device_reset(CLK_PCIE, false);
	clock_set_device_reset(CLK_PCIE_LINK0, false);
	clock_set_device_reset(CLK_PCIE_LINK1, false);
	clock_set_device_reset(CLK_PCIE_LINK2, false);
	clock_set_device_reset(CLK_PCIE_LINK3, false);
#if SUB_PLATFORM_S8001
	clock_set_device_reset(CLK_PCIE_LINK4, false);
	clock_set_device_reset(CLK_PCIE_LINK5, false);
#endif
}

// Implements the enable sequence from APCIe spec section 6.1 as it applies to
// APCIE_COMMON registers. The portion of the sequence that affects the APCIE_CONFIG
// registers is handled in common code
static void apcie_enable_link_hardware(uint32_t link)
{
	ASSERT(link < APCIE_NUM_LINKS);

	dprintf(DEBUG_INFO, "apcie: Enabling link %d\n", link);

	dprintf(DEBUG_SPEW, "setting PERST\n");
	// Make sure PERST# starts out as asserted
	CLR_PORT_REG_BIT(PORT_RST_CTRL, link, APCIE_PERST_N);
 
	dprintf(DEBUG_SPEW, "enabling REFCLK\n");
	// 1) 100Mhz REFCLK to the link partners is enabled
	SET_PORT_REG_BIT(PORT_REFCLK_CTRL, link, REFCLK_EN);

	// (2) and (3) are done in the common code before calling this function

	// 4) Update link configuration
	// skip this step in SecureROM/iBoot

	dprintf(DEBUG_SPEW, "enabling port\n");
	// 5) Enable target PCIE link by setting APCIE_PORT_CTRL_{0/1/2/3}.port_en
	SET_PORT_REG_BIT(PORT_CTRL, link, PORT_EN);

	dprintf(DEBUG_SPEW, "waiting for PHY cal\n");
	// 5d) Software must wait until APCIE_PHY_CAL_STS_{0/1/2/3}.cal_done is set
	while (GET_PORT_REG_BIT(PHY_CAL_STS, link, CAL_DONE) == 0)
		spin(10);

#if SUPPORT_FPGA
	SET_PORT_REG_BIT(PORT_CTRL, link, MUXED_AUXCLK_AUTO_DIS);
#endif

	dprintf(DEBUG_SPEW, "clearing PERST\n");
	// 6) De-assert PERST# for the target PCIE link.
	SET_PORT_REG_BIT(PORT_RST_CTRL, link, APCIE_PERST_N);

	// 7) Enable dynamic REFCLK clock gating
	// skip this step in SecureROM/iBoot
}

static void apcie_disable_link_hardware(uint32_t link)
{
	ASSERT(link < APCIE_NUM_LINKS);

	CLR_PORT_REG_BIT(PORT_RST_CTRL, link, APCIE_PERST_N);

	CLR_PORT_REG_BIT(PORT_REFCLK_CTRL, link, REFCLK_EN);
	CLR_PORT_REG_BIT(PORT_CTRL, link, PORT_EN);
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

	// Apply root complex config tunables
	platform_apply_tunables(tunables_pcie_config_rc[link].tunable_chip,
				tunables_pcie_config_rc[link].num_tunable_chips,
				"PCIe config");

	// Apply APCIe config port tunables
	platform_apply_tunables(tunables_pcie_config_ports[link].tunable_chip,
				tunables_pcie_config_ports[link].num_tunable_chips,
				"APCIe config");

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

#if SUB_PLATFORM_S8001
	// <rdar://problem/21952769> ECO_124 SNPS Gen3 PCIE RC IP's LTSSM gets stuck when exiting L0 and entering L1 - Enable ECO
	rAPCIE_CONFIG_SPACE32(link, X2_PCIE_RC_X2_PCIE_DSP_PF0_PORT_LOGIC_SLAVE_STATE_OVR_OFF_OFFSET) |= 1;
#endif

	rAPCIE_COUNTER_COMMAND(link) = APCIE_COUNTER_CLEAR | APCIE_COUNTER_ENABLE;

#if SUB_PLATFORM_S8001 || SUB_PLATFORM_S8003
	// Disable PHY powergating: <rdar://problem/20384273> Re-enable ASPM+L1SS for all Endpoints [Malta]
	uint32_t lane_cfg = platform_get_apcie_lane_cfg();
	uint32_t phy_sig_mask;
	uint32_t phy_reg;

#if SUB_PLATFORM_S8001
	uint32_t link_widths[][6] = {
		[APCIE_LANE_CFG_X1_X1_X1_X1_X1_X1] = { 1, 1, 1, 1, 1, 1},
		[APCIE_LANE_CFG_X2_X1_X1_X1_X1] =    { 2, 0, 1, 1, 1, 1},
		[APCIE_LANE_CFG_X1_X1_X2_X1_X1] =    { 1, 1, 2, 0, 1, 1},
		[APCIE_LANE_CFG_X2_X2_X1_X1] =       { 2, 0, 2, 0, 1, 1},
		[APCIE_LANE_CFG_X1_X1_X1_X1_X2] =    { 1, 1, 1, 1, 2, 0},
		[APCIE_LANE_CFG_X2_X1_X1_X2] =       { 2, 0, 1, 1, 2, 0},
		[APCIE_LANE_CFG_X1_X1_X2_X2] =       { 1, 1, 2, 0, 2, 0},
		[APCIE_LANE_CFG_X2_X2_X2] =          { 2, 0, 2, 0, 2, 0},
	};

	ASSERT(lane_cfg < sizeof(link_widths) / sizeof(link_widths[0]));
	ASSERT(link < sizeof(link_widths[0]) / sizeof(link_widths[0][0]));

	uint32_t link_width = link_widths[lane_cfg][link];
	ASSERT(link_width != 0);

	phy_sig_mask = (link_width | 1) << link;
#elif SUB_PLATFORM_S8003
	switch (link) {
	case 0:
		phy_sig_mask = (lane_cfg == APCIE_LANE_CFG_X1_X1_X1_X1) ? 0x01 : 0x03;
		break;
	case 1:
		phy_sig_mask = 0x04;
		break;
	case 2:
		phy_sig_mask = (lane_cfg == APCIE_LANE_CFG_X2_X1_X2) ? 0x18 : 0x08;
		break;
	case 3:
		ASSERT(lane_cfg != APCIE_LANE_CFG_X2_X1_X2);
		phy_sig_mask = 0x10;
		break;
	default:
		panic("Invalid PCIe link: %d", link);
	}
#else
#error "Invalid platform"
#endif

	// Wait for the phy lanes to power up.
	while ((PHY_GET_REG(PCIE_PHY_SIG) & phy_sig_mask) != phy_sig_mask) {
		spin(10);
	}

	// Disable PCIE PHY common block power gating.
	OR_PMA_CMN_REG(CMN_CDIAG_FUNC_PWR_CTRL, PMA_CMN_REGISTERS_BLK_CMN_CDIAG_FUNC_PWR_CTRL_14_UMASK);
	OR_PMA_CMN_REG(CMN_CDIAG_XCTRL_PWR_CTRL, PMA_CMN_REGISTERS_BLK_CMN_CDIAG_XCTRL_PWR_CTRL_14_UMASK);

	// Update common PHY PLL timers.
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

			// ... and increase the margin of PI to avoid AER errors and link down ...
			SET_PMA_RX_REG(RX_IQPI_ILL_LOCK_CALCNT_START0, lane, 0xa9);
			SET_PMA_RX_REG(RX_EPI_ILL_LOCK_CALCNT_START0, lane, 0xa9);
			SET_PMA_RX_REG(RX_IQPI_ILL_LOCK_CALCNT_START1, lane, 0x14d);
			SET_PMA_RX_REG(RX_EPI_ILL_LOCK_CALCNT_START1, lane, 0x14d);
		}
	}
#endif	// SUB_PLATFORM_S8001 || SUB_PLATFORM_S8003

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
	DTNode *node;
	uint32_t propSize;
	char *propName;
	void *propData;
	uint8_t *dt_prop = (uint8_t *)malloc(MAX_DT_PROPERTY_SIZE);
	uint8_t *next_dt_prop;
	char bridge[16];

	// Apply common tunables.
	bzero(dt_prop, MAX_DT_PROPERTY_SIZE);
	next_dt_prop = dt_prop;
	next_dt_prop = platform_apply_dt_tunables(tunables_pcie_common,
						  ARRAY_SIZE(tunables_pcie_common),
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

#if SUB_PLATFORM_S8000
	// Apply fuse-based PHY tunables.
	bzero(dt_prop, MAX_DT_PROPERTY_SIZE);
	next_dt_prop = dt_prop;
	next_dt_prop = platform_apply_dt_tunables(tunables_pcie_phy_efuse,
						  ARRAY_SIZE(tunables_pcie_phy_efuse),
						  next_dt_prop,
						  APCIE_PHY_BASE_ADDR,
						  "PCIe PHY fused");
#endif

	// Apply SPDS PHY tunables.
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

	// Enable Maui A1 errata if necessary
	if (platform_get_chip_revision() < CHIP_REVISION_B0) {
		propName = "maui-a1-errata";
		if (FindProperty(apcie_node, &propName, &propData, &propSize)) {
			ASSERT(propSize == sizeof(uint32_t));
			*(uint32_t *)propData = 1;
		}
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
#if SUB_PLATFORM_S8001
	[4] = {
		.perst_gpio =		GPIO_PCIE4_PERST,
		.clkreq_gpio =		GPIO_PCIE4_CLKREQ,
		.dart_id = 		PCIE_PORT4_DART_ID,
	},
	[5] = {
		.perst_gpio =		GPIO_PCIE5_PERST,
		.clkreq_gpio =		GPIO_PCIE5_CLKREQ,
		.dart_id = 		PCIE_PORT5_DART_ID,
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
