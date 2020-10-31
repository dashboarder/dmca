/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* This file contains Tunables for s8001 */

#include <platform/tunables.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwregbase.h>
#include <soc/module/address_map.h>

/**************************************** BEGIN CHIP_REVISION_ALL ****************************************/
// <rdar://problem/21347912> Add support for product / platform specific tunables
// <rdar://problem/20619805> Add PMGR MISC_CFG_DISP.SEL_DISP_PMGR_DWI_VSYNC tunable to SPDS tunables
#define SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8001_EXTRA \
{  /*! PLL0_CFG               */          0x00004,        sizeof(uint32_t), 0xc0000000u,    0x40000000u}, \
{  /*! PLL1_CFG               */          0x04004,        sizeof(uint32_t), 0xc0000000u,    0x40000000u}, \
{  /*! PLL6_CFG               */          0x18004,        sizeof(uint32_t), 0xc0000000u,    0x40000000u}, \
{  /*! PLL7_CFG               */          0x1c004,        sizeof(uint32_t), 0xc0000000u,    0x00000000u}, \
{  /*! VOLMAN_SOC_DELAY       */          0xa001c,        sizeof(uint32_t), 0xfffffcffu,    0xbb816405u}, \
{  /*! VOLMAN_GFX_SRAM_DELAY  */          0xa0020,        sizeof(uint32_t), 0xfffffcffu,    0xbb816405u}, \
{  /*! VOLMAN_CPU_SRAM_DELAY  */          0xa0024,        sizeof(uint32_t), 0xfffffcffu,    0xbb816405u}, \
{  /*! VOLMAN_GFX_DELAY       */          0xa0028,        sizeof(uint32_t), 0xfffffcffu,    0xbb816405u}, \
{  /*! VOLMAN_CPU_DELAY       */          0xa002c,        sizeof(uint32_t), 0xfffffcffu,    0xbb816405u}, \
{  /*! MISC_CFG_DISP_OFFSET      */       0xdc040,        sizeof(uint32_t), 0x00000001u,    0x00000000u}

// <rdar://problem/20620055> Add CPU TVM tunables in tunables PDF to SPDS tunables
// <rdar://problem/20545557> Add DVMR tunables in tunables PDF to SPDS tunables
#define ACC_DEFAULT_TUNABLES_S8001_EXTRA \
/* Configure temperature ranges and measurement offsets for DVFM/DVTM */ \
/* temp_thread0 0x8 temp_thresh1 0x21 temp_offset0 0 temp_offset1 0 */ \
{  /* ACC_PWRCTL_DVFM_CFG    */           0x220040,       sizeof(uint32_t), 0x0fffffffu,    0x00001088u}, \
/* temp_thresh2 0x3a temp_offset2 0 */ \
{  /* ACC_PWRCTL_DVFM_CFG1   */           0x220058,       sizeof(uint32_t), 0x001fffffu,    0x0000003au}, \
{  /* ACC_PWRCTL_DVMR_SCR   */            0x2206b8,       sizeof(uint64_t), 0x00000fffu,    0x000000421}

#define SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8001_EXTRA \
/* <rdar://problem/20868785> Ensure that LPO OFF_MODE is set appropriately */ \
{  /*! MINIPMGR_LPO_CFG      */           0x00004,        sizeof(uint32_t), 0xc0000000u, 0x80000000u   }

// <rdar://problem/17755256> SwitchFabric Hash lock bit is not set
#define SWITCHFABRIC_DEFAULT_TUNABLES_S8001_EXTRA \
{  /*! SWITCHFABRIC_REGS_AMAP_LOCK */     0x00000,        sizeof(uint32_t), 0x00000001u, 0x00000001u   }
/**************************************** END CHIP_REVISION_ALL ****************************************/

/**************************************** BEGIN CHIP_REVISION_B0 ****************************************/
#include <soc/s8001/b0/tunable/minipmgr.h>
#include <soc/s8001/b0/tunable/pmgr.h>
#include <soc/s8001/b0/tunable/acc.h>
#include <soc/s8001/b0/tunable/sbasyncfifo_widgets.h>
#include <soc/s8001/b0/tunable/socbusmux.h>
#include <soc/s8001/b0/tunable/switchfabric.h>
#include <soc/s8001/b0/tunable/pms_csr.h>
#include <soc/s8001/b0/tunable/afc_aiu_sb.h>
#include <soc/s8001/b0/tunable/sb_glue.h>
#include <soc/s8001/b0/tunable/misc1.h>
#include <soc/s8001/b0/tunable/cp_com.h>


static const struct tunable_struct pmsasyncfifo_tunable_b0[] = {
	SOUTH_BRIDGE_PMSASYNCFIFO_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct pmgr_tunable_b0[] = {
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8001_EXTRA,
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct minipmgr_tunable_b0[] = {
	SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8001_EXTRA,
	SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct64 acc_tunable_b0[] = {
	ACC_DEFAULT_TUNABLES_S8001_EXTRA,
	ACC_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct socbusmux_tunable_b0[] = {
	SOCBUSMUX_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct switchfabric_tunable_b0[] = {
	SWITCHFABRIC_DEFAULT_TUNABLES_S8001_EXTRA,
	SWITCHFABRIC_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct pms_csr_tunable_b0[] = {
	SOUTH_BRIDGE_PMS_PMS_CSR_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct sb_glue_tunable_b0[] = {
	SOUTH_BRIDGE_DYNAMIC_CLK_GATING_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct afc_aiu_sb_tunable_b0[] = {
	SOUTH_BRIDGE_SBR_AXI2AF_AFC_AIU_SB_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct asio_misc1_tunable_b0[] = {
	SOUTH_BRIDGE_ASIO_MISC1_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct cp0_cp_com_tunable_b0[] = {
	CP0_CP_COM_DEFAULT_TUNABLES_S8001_B0
};

static const struct tunable_struct cp1_cp_com_tunable_b0[] = {
	CP1_CP_COM_DEFAULT_TUNABLES_S8001_B0
};

/***************************************** END CHIP_REVISION_B0 *****************************************/

/**************************************** BEGIN CHIP_REVISION_A0 ****************************************/
#include <soc/s8001/a0/tunable/minipmgr.h>
#include <soc/s8001/a0/tunable/pmgr.h>
#include <soc/s8001/a0/tunable/acc.h>
#include <soc/s8001/a0/tunable/sbasyncfifo_widgets.h>
#include <soc/s8001/a0/tunable/socbusmux.h>
#include <soc/s8001/a0/tunable/switchfabric.h>
#include <soc/s8001/a0/tunable/pms_csr.h>
#include <soc/s8001/a0/tunable/afc_aiu_sb.h>
#include <soc/s8001/a0/tunable/sb_glue.h>
#include <soc/s8001/a0/tunable/misc1.h>
#include <soc/s8001/a0/tunable/cp_com.h>

static const struct tunable_struct pmsasyncfifo_tunable_a0[] = {
	SOUTH_BRIDGE_PMSASYNCFIFO_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct pmgr_tunable_a0[] = {
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8001_EXTRA,
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct minipmgr_tunable_a0[] = {
	SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8001_EXTRA,
	SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct64 acc_tunable_a0[] = {
	ACC_DEFAULT_TUNABLES_S8001_EXTRA,
	ACC_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct socbusmux_tunable_a0[] = {
	SOCBUSMUX_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct switchfabric_tunable_a0[] = {
	SWITCHFABRIC_DEFAULT_TUNABLES_S8001_EXTRA,
	SWITCHFABRIC_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct pms_csr_tunable_a0[] = {
	SOUTH_BRIDGE_PMS_PMS_CSR_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct sb_glue_tunable_a0[] = {
	SOUTH_BRIDGE_DYNAMIC_CLK_GATING_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct afc_aiu_sb_tunable_a0[] = {
	SOUTH_BRIDGE_SBR_AXI2AF_AFC_AIU_SB_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct asio_misc1_tunable_a0[] = {
	SOUTH_BRIDGE_ASIO_MISC1_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct cp0_cp_com_tunable_a0[] = {
	CP0_CP_COM_DEFAULT_TUNABLES_S8001_A0
};

static const struct tunable_struct cp1_cp_com_tunable_a0[] = {
	CP1_CP_COM_DEFAULT_TUNABLES_S8001_A0
};
/***************************************** END CHIP_REVISION_A0 *****************************************/

// For each chip, highest revision must come first.
// Notice reconfig == false for minipmgr, since it's an AOP.
const struct tunable_chip_struct tunables_pmgr[] = {
	// B0
	{CHIP_REVISION_B0, SOUTH_BRIDGE_PMS_PMGR_PADDR_S8001_B0,                pmgr_tunable_b0,         NULL,           true},
	{CHIP_REVISION_B0, SOUTH_BRIDGE_AOP_MINIPMGR_PADDR_S8001_B0,            minipmgr_tunable_b0,     NULL,           false},
	{CHIP_REVISION_B0, ACC_PADDR_S8001_B0,                                  NULL,                    acc_tunable_b0, true},
	{CHIP_REVISION_B0, SOUTH_BRIDGE_PMSASYNCFIFO_PADDR_S8001_B0,            pmsasyncfifo_tunable_b0, NULL,           true},
	{CHIP_REVISION_B0, SOCBUSMUX_PADDR_S8001_B0,                            socbusmux_tunable_b0,    NULL,           true},
	{CHIP_REVISION_B0, SWITCHFABRIC_PADDR_S8001_B0,                         switchfabric_tunable_b0, NULL,           true},
	{CHIP_REVISION_B0, SOUTH_BRIDGE_PMS_PMS_CSR_PADDR_S8001_B0,             pms_csr_tunable_b0,      NULL,           true},
	{CHIP_REVISION_B0, SOUTH_BRIDGE_DYNAMIC_CLK_GATING_PADDR_S8001_B0,      sb_glue_tunable_b0,      NULL,           true},
	{CHIP_REVISION_B0, SOUTH_BRIDGE_SBR_AXI2AF_AFC_AIU_SB_PADDR_S8001_B0,   afc_aiu_sb_tunable_b0,   NULL,           true},
	{CHIP_REVISION_B0, SOUTH_BRIDGE_ASIO_MISC1_PADDR_S8001_B0,              asio_misc1_tunable_b0,   NULL,           true},
	{CHIP_REVISION_B0, CP0_CP_COM_PADDR_S8001_B0,                           cp0_cp_com_tunable_b0,   NULL,           true},
	{CHIP_REVISION_B0, CP1_CP_COM_PADDR_S8001_B0,                           cp1_cp_com_tunable_b0,   NULL,           true},

	// A0
	{CHIP_REVISION_A0, SOUTH_BRIDGE_PMS_PMGR_PADDR_S8001_A0,                pmgr_tunable_a0,         NULL,           true},
	{CHIP_REVISION_A0, SOUTH_BRIDGE_AOP_MINIPMGR_PADDR_S8001_A0,            minipmgr_tunable_a0,     NULL,           false},
	{CHIP_REVISION_A0, ACC_PADDR_S8001_A0,                                  NULL,                    acc_tunable_a0, true},
	{CHIP_REVISION_A0, SOUTH_BRIDGE_PMSASYNCFIFO_PADDR_S8001_A0,            pmsasyncfifo_tunable_a0, NULL,           true},
	{CHIP_REVISION_A0, SOCBUSMUX_PADDR_S8001_A0,                            socbusmux_tunable_a0,    NULL,           true},
	{CHIP_REVISION_A0, SWITCHFABRIC_PADDR_S8001_A0,                         switchfabric_tunable_a0, NULL,           true},
	{CHIP_REVISION_A0, SOUTH_BRIDGE_PMS_PMS_CSR_PADDR_S8001_A0,             pms_csr_tunable_a0,      NULL,           true},
	{CHIP_REVISION_A0, SOUTH_BRIDGE_DYNAMIC_CLK_GATING_PADDR_S8001_A0,      sb_glue_tunable_a0,      NULL,           true},
	{CHIP_REVISION_A0, SOUTH_BRIDGE_SBR_AXI2AF_AFC_AIU_SB_PADDR_S8001_A0,   afc_aiu_sb_tunable_a0,   NULL,           true},
	{CHIP_REVISION_A0, SOUTH_BRIDGE_ASIO_MISC1_PADDR_S8001_A0,              asio_misc1_tunable_a0,   NULL,           true},
	{CHIP_REVISION_A0, CP0_CP_COM_PADDR_S8001_A0,                           cp0_cp_com_tunable_a0,   NULL,           true},
	{CHIP_REVISION_A0, CP1_CP_COM_PADDR_S8001_A0,                           cp1_cp_com_tunable_a0,   NULL,           true},

	{0, 0, NULL, NULL, 0}
};
