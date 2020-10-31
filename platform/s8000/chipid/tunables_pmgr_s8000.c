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

/* This file contains Tunables for s8000 */

#include <platform/tunables.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwregbase.h>
#include <soc/module/address_map.h>

/**************************************** BEGIN CHIP_REVISION_ALL ****************************************/
// <rdar://problem/21406702> Maui: Add PMGR_PLL_PCIE_CFG.LOCK_MODE tunable to SPDS
// <rdar://problem/21347912> Add support for product / platform specific tunables
// <rdar://problem/20536609> Add GFX TVM tunables in PDF tunables to SPDS tunables
// <rdar://problem/20537375> Add SOC TVM tunables in PDF tunables to SPDS tunables
#define SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8000_EXTRA \
{  /*! PLL0_CFG               */          0x00004,        sizeof(uint32_t), 0xc0000000u,    0x40000000u}, \
{  /*! PLL1_CFG               */          0x04004,        sizeof(uint32_t), 0xc0000000u,    0x40000000u}, \
{  /*! PLL_PCIE_CFG           */          0x18004,        sizeof(uint32_t), 0xc0000000u,    0x40000000u}, \
{  /*! VOLMAN_SOC_DELAY       */          0xa001c,        sizeof(uint32_t), 0xfffffcffu,    0xbb81642fu}, \
{  /*! VOLMAN_GFX_SRAM_DELAY  */          0xa0020,        sizeof(uint32_t), 0xfffffcffu,    0xbb816405u}, \
{  /*! VOLMAN_CPU_SRAM_DELAY  */          0xa0024,        sizeof(uint32_t), 0xfffffcffu,    0xbb816405u}, \
{  /*! VOLMAN_GFX_DELAY       */          0xa0028,        sizeof(uint32_t), 0xfffffcffu,    0xbb816405u}, \
{  /*! VOLMAN_CPU_DELAY       */          0xa002c,        sizeof(uint32_t), 0xfffffcffu,    0xbb816405u}, \
{  /*! SOC_TVM_THRESH0        */          0xc8004,        sizeof(uint32_t), 0x000001ffu,    0x00000000u}, \
{  /*! SOC_TVM_THRESH1        */          0xc8008,        sizeof(uint32_t), 0x000001ffu,    0x00000019u}, \
{  /*! SOC_TVM_THRESH2        */          0xc800c,        sizeof(uint32_t), 0x000001ffu,    0x00000032u}, \
{  /*! SOC_TVM_TEMP0_CFG      */          0xc8010,        sizeof(uint32_t), 0x01ff01ffu,    0x001c01f4u}, \
{  /*! SOC_TVM_TEMP1_CFG      */          0xc8014,        sizeof(uint32_t), 0x01ff01ffu,    0x001c01f4u}, \
{  /*! SOC_TVM_TEMP2_CFG      */          0xc8018,        sizeof(uint32_t), 0x01ff01ffu,    0x001c01f4u}, \
{  /*! GFX_TVM_CTL            */          0xc8200,        sizeof(uint32_t), 0x00000f00u,    0x00000f00u}, \
{  /*! GFX_TVM_THRESH0        */          0xc8204,        sizeof(uint32_t), 0x000001ffu,    0x00000000u}, \
{  /*! GFX_TVM_THRESH1        */          0xc8208,        sizeof(uint32_t), 0x000001ffu,    0x00000019u}, \
{  /*! GFX_TVM_THRESH2        */          0xc820c,        sizeof(uint32_t), 0x000001ffu,    0x00000032u}, \
{  /*! GFX_TVM_TEMP0_CFG      */          0xc8210,        sizeof(uint32_t), 0x01ff01ffu,    0x001501f9u}, \
{  /*! GFX_TVM_TEMP1_CFG      */          0xc8214,        sizeof(uint32_t), 0x01ff01ffu,    0x001501f9u}, \
{  /*! GFX_TVM_TEMP2_CFG      */          0xc8218,        sizeof(uint32_t), 0x01ff01ffu,    0x001501f9u}

// <rdar://problem/20544302> Add CPU TVM tunables in tunables PDF to SPDS tunables
// <rdar://problem/20545027> Add DVMR tunables in tunables PDF to SPDS tunables
#define ACC_DEFAULT_TUNABLES_S8000_EXTRA \
/* Configure temperature ranges and measurement offsets for DVFM/DVTM */ \
/* temp_thread0 0x8 temp_thresh1 0x21 temp_offset0 0 temp_offset1 0 */ \
{  /* ACC_PWRCTL_DVFM_CFG    */           0x220040,       sizeof(uint32_t), 0x0fffffffu,    0x00001088u}, \
/* temp_thresh2 0x3a temp_offset2 0 */ \
{  /* ACC_PWRCTL_DVFM_CFG1   */           0x220058,       sizeof(uint32_t), 0x001fffffu,    0x0000003au}, \
{  /* ACC_PWRCTL_DVMR_SCR   */            0x2206b8,       sizeof(uint64_t), 0x00000fffu,    0x000000421}

#define SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8000_EXTRA \
/* <rdar://problem/20868785> Ensure that LPO OFF_MODE is set appropriately */ \
{  /*! MINIPMGR_LPO_CFG      */           0x00004,        sizeof(uint32_t), 0xc0000000u, 0x80000000u   }

// <rdar://problem/17755256> SwitchFabric Hash lock bit is not set
#define SWITCHFABRIC_DEFAULT_TUNABLES_S8000_EXTRA \
{  /*! SWITCHFABRIC_REGS_AMAP_LOCK */     0x00000,        sizeof(uint32_t), 0x00000001u, 0x00000001u   }
/**************************************** END CHIP_REVISION_ALL ****************************************/

/**************************************** BEGIN CHIP_REVISION_C0 ****************************************/
#include <soc/s8000/c0/tunable/minipmgr.h>
#include <soc/s8000/c0/tunable/pmgr.h>
#include <soc/s8000/c0/tunable/acc.h>
#include <soc/s8000/c0/tunable/sbasyncfifo_widgets.h>
#include <soc/s8000/c0/tunable/socbusmux.h>
#include <soc/s8000/c0/tunable/switchfabric.h>
#include <soc/s8000/c0/tunable/pms_csr.h>
#include <soc/s8000/c0/tunable/afc_aiu_sb.h>
#include <soc/s8000/c0/tunable/sb_glue.h>
#include <soc/s8000/c0/tunable/misc1.h>
#include <soc/s8000/c0/tunable/cp_com.h>

static const struct tunable_struct pmsasyncfifo_tunable_c0[] = {
	SOUTH_BRIDGE_PMSASYNCFIFO_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct pmgr_tunable_c0[] = {
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8000_EXTRA,
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct minipmgr_tunable_c0[] = {
	SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8000_EXTRA,
	SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct64 acc_tunable_c0[] = {
	ACC_DEFAULT_TUNABLES_S8000_EXTRA,
	ACC_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct socbusmux_tunable_c0[] = {
	SOCBUSMUX_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct switchfabric_tunable_c0[] = {
	SWITCHFABRIC_DEFAULT_TUNABLES_S8000_EXTRA,
	SWITCHFABRIC_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct pms_csr_tunable_c0[] = {
	SOUTH_BRIDGE_PMS_PMS_CSR_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct sb_glue_tunable_c0[] = {
	SOUTH_BRIDGE_DYNAMIC_CLK_GATING_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct afc_aiu_sb_tunable_c0[] = {
	SOUTH_BRIDGE_SBR_AXI2AF_AFC_AIU_SB_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct asio_misc1_tunable_c0[] = {
	SOUTH_BRIDGE_ASIO_MISC1_DEFAULT_TUNABLES_S8000_C0
};

static const struct tunable_struct cp_com_tunable_c0[] = {
	CP_CP_COM_DEFAULT_TUNABLES_S8000_C0
};
/***************************************** END CHIP_REVISION_C0 *****************************************/

/**************************************** BEGIN CHIP_REVISION_B0 ****************************************/
#include <soc/s8000/b0/tunable/minipmgr.h>
#include <soc/s8000/b0/tunable/pmgr.h>
#include <soc/s8000/b0/tunable/acc.h>
#include <soc/s8000/b0/tunable/sbasyncfifo_widgets.h>
#include <soc/s8000/b0/tunable/socbusmux.h>
#include <soc/s8000/b0/tunable/switchfabric.h>
#include <soc/s8000/b0/tunable/pms_csr.h>
#include <soc/s8000/b0/tunable/afc_aiu_sb.h>
#include <soc/s8000/b0/tunable/sb_glue.h>
#include <soc/s8000/b0/tunable/misc1.h>
#include <soc/s8000/b0/tunable/cp_com.h>

#define SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8000_B0_EXTRA \
/* <rdar://problem/19929273> Maui MCC EMA setting change to 3% */ \
{  /*! EMA_FIXED_SOC1        */           0xac004,        sizeof(uint32_t), 0x07fff000u,    0x030d1000u}

static const struct tunable_struct pmsasyncfifo_tunable_b0[] = {
	SOUTH_BRIDGE_PMSASYNCFIFO_DEFAULT_TUNABLES_S8000_B0,
};

static const struct tunable_struct pmgr_tunable_b0[] = {
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8000_EXTRA,
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8000_B0_EXTRA,
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct minipmgr_tunable_b0[] = {
	SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8000_EXTRA,
	SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct64 acc_tunable_b0[] = {
	ACC_DEFAULT_TUNABLES_S8000_EXTRA,
	ACC_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct socbusmux_tunable_b0[] = {
	SOCBUSMUX_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct switchfabric_tunable_b0[] = {
	SWITCHFABRIC_DEFAULT_TUNABLES_S8000_EXTRA,
	SWITCHFABRIC_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct pms_csr_tunable_b0[] = {
	SOUTH_BRIDGE_PMS_PMS_CSR_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct sb_glue_tunable_b0[] = {
	SOUTH_BRIDGE_DYNAMIC_CLK_GATING_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct afc_aiu_sb_tunable_b0[] = {
	SOUTH_BRIDGE_SBR_AXI2AF_AFC_AIU_SB_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct asio_misc1_tunable_b0[] = {
	SOUTH_BRIDGE_ASIO_MISC1_DEFAULT_TUNABLES_S8000_B0
};

static const struct tunable_struct cp_com_tunable_b0[] = {
	CP_CP_COM_DEFAULT_TUNABLES_S8000_B0
};
/***************************************** END CHIP_REVISION_B0 *****************************************/

/**************************************** BEGIN CHIP_REVISION_A1 ****************************************/
#include <soc/s8000/a1/tunable/minipmgr.h>
#include <soc/s8000/a1/tunable/pmgr.h>
#include <soc/s8000/a1/tunable/acc.h>
#include <soc/s8000/a1/tunable/sbasyncfifo_widgets.h>
#include <soc/s8000/a1/tunable/socbusmux.h>
#include <soc/s8000/a1/tunable/switchfabric.h>
#include <soc/s8000/a1/tunable/pms_csr.h>
#include <soc/s8000/a1/tunable/afc_aiu_sb.h>
#include <soc/s8000/a1/tunable/sb_glue.h>
#include <soc/s8000/a1/tunable/misc1.h>
#include <soc/s8000/a1/tunable/cp_com.h>

#define SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8000_A1_EXTRA \
/* <rdar://problem/18700978> iBoot to set M$ EMAA register to new value to increase Maui Yield */ \
{  /*! EMA_FIXED_SOC1        */           0xac004,        sizeof(uint32_t), 0x07fff000u,    0x032d1000u}

// <rdar://problem/20545899> Maui: Add PMGR THERMAL tunables to SPDS for Maui A1
#define SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8000_A1_THERMAL_EXTRA \
{  /*! PMGR_THERMAL_0_PIECE0 */           0xc0028,        sizeof(uint32_t), 0x7ffffu,    0x7943bu      }, \
{  /*! PMGR_THERMAL_0_PIECE1 */           0xc002c,        sizeof(uint32_t), 0xffc7ffffu, 0x5007d02fu   }, \
{  /*! PMGR_THERMAL_0_PIECE2 */           0xc0030,        sizeof(uint32_t), 0xffc7ffffu, 0x95c02426u   }, \
{  /*! PMGR_THERMAL_1_PIECE0 */           0xc0128,        sizeof(uint32_t), 0x7ffffu,    0x7943bu      }, \
{  /*! PMGR_THERMAL_1_PIECE1 */           0xc012c,        sizeof(uint32_t), 0xffc7ffffu, 0x5007d02fu   }, \
{  /*! PMGR_THERMAL_1_PIECE2 */           0xc0130,        sizeof(uint32_t), 0xffc7ffffu, 0x95c02426u   }, \
{  /*! PMGR_THERMAL_2_PIECE0 */           0xc0228,        sizeof(uint32_t), 0x7ffffu,    0x7943bu      }, \
{  /*! PMGR_THERMAL_2_PIECE1 */           0xc022c,        sizeof(uint32_t), 0xffc7ffffu, 0x5007d02fu   }, \
{  /*! PMGR_THERMAL_2_PIECE2 */           0xc0230,        sizeof(uint32_t), 0xffc7ffffu, 0x95c02426u   }

#define SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8000_A1_EXTRA \
/* <rdar://problem/19300047> 19300047 */ \
{  /*! MINIPMGR_LPO_CFG      */           0x00004,        sizeof(uint32_t), 0x30000000u, 0x00000000u   }

static const struct tunable_struct pmsasyncfifo_tunable_a1[] = {
	SOUTH_BRIDGE_PMSASYNCFIFO_DEFAULT_TUNABLES_S8000_A1,
};

static const struct tunable_struct pmgr_tunable_a1[] = {
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8000_EXTRA,
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8000_A1_EXTRA,
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8000_A1_THERMAL_EXTRA,
	SOUTH_BRIDGE_PMS_PMGR_DEFAULT_TUNABLES_S8000_A1
};

static const struct tunable_struct minipmgr_tunable_a1[] = {
	SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8000_EXTRA,
	SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8000_A1_EXTRA,
	SOUTH_BRIDGE_AOP_MINIPMGR_DEFAULT_TUNABLES_S8000_A1
};

static const struct tunable_struct64 acc_tunable_a1[] = {
	ACC_DEFAULT_TUNABLES_S8000_EXTRA,
	ACC_DEFAULT_TUNABLES_S8000_A1
};

static const struct tunable_struct socbusmux_tunable_a1[] = {
	SOCBUSMUX_DEFAULT_TUNABLES_S8000_A1
};

static const struct tunable_struct switchfabric_tunable_a1[] = {
	SWITCHFABRIC_DEFAULT_TUNABLES_S8000_EXTRA,
	SWITCHFABRIC_DEFAULT_TUNABLES_S8000_A1
};

static const struct tunable_struct pms_csr_tunable_a1[] = {
	SOUTH_BRIDGE_PMS_PMS_CSR_DEFAULT_TUNABLES_S8000_A1
};

static const struct tunable_struct sb_glue_tunable_a1[] = {
	SOUTH_BRIDGE_DYNAMIC_CLK_GATING_DEFAULT_TUNABLES_S8000_A1
};

static const struct tunable_struct afc_aiu_sb_tunable_a1[] = {
	SOUTH_BRIDGE_SBR_AXI2AF_AFC_AIU_SB_DEFAULT_TUNABLES_S8000_A1
};

static const struct tunable_struct asio_misc1_tunable_a1[] = {
	SOUTH_BRIDGE_ASIO_MISC1_DEFAULT_TUNABLES_S8000_A1
};

static const struct tunable_struct cp_com_tunable_a1[] = {
	CP_CP_COM_DEFAULT_TUNABLES_S8000_A1
};
/***************************************** END CHIP_REVISION_A1 *****************************************/

// For each chip, highest revision must come first.
// Notice reconfig == false for minipmgr, since it's an AOP.
const struct tunable_chip_struct tunables_pmgr[] = {
	// C0
	{CHIP_REVISION_C0, SOUTH_BRIDGE_PMS_PMGR_PADDR_S8000_C0,                pmgr_tunable_c0,         NULL,           true},
	{CHIP_REVISION_C0, SOUTH_BRIDGE_AOP_MINIPMGR_PADDR_S8000_C0,            minipmgr_tunable_c0,     NULL,           false},
	{CHIP_REVISION_C0, ACC_PADDR_S8000_C0,                                  NULL,                    acc_tunable_c0, true},
	{CHIP_REVISION_C0, SOUTH_BRIDGE_PMSASYNCFIFO_PADDR_S8000_C0,            pmsasyncfifo_tunable_c0, NULL,           true},
	{CHIP_REVISION_C0, SOCBUSMUX_PADDR_S8000_C0,                            socbusmux_tunable_c0,    NULL,           true},
	{CHIP_REVISION_C0, SWITCHFABRIC_PADDR_S8000_C0,                         switchfabric_tunable_c0, NULL,           true},
	{CHIP_REVISION_C0, SOUTH_BRIDGE_PMS_PMS_CSR_PADDR_S8000_C0,             pms_csr_tunable_c0,      NULL,           true},
	{CHIP_REVISION_C0, SOUTH_BRIDGE_DYNAMIC_CLK_GATING_PADDR_S8000_C0,      sb_glue_tunable_c0,      NULL,           true},
	{CHIP_REVISION_C0, SOUTH_BRIDGE_SBR_AXI2AF_AFC_AIU_SB_PADDR_S8000_C0,   afc_aiu_sb_tunable_c0,   NULL,           true},
	{CHIP_REVISION_C0, SOUTH_BRIDGE_ASIO_MISC1_PADDR_S8000_C0,              asio_misc1_tunable_c0,   NULL,           true},
	{CHIP_REVISION_C0, CP_CP_COM_PADDR_S8000_C0,                            cp_com_tunable_c0,       NULL,           true},

	// B0
	{CHIP_REVISION_B0, SOUTH_BRIDGE_PMS_PMGR_PADDR_S8000_B0,                pmgr_tunable_b0,         NULL,           true},
	{CHIP_REVISION_B0, SOUTH_BRIDGE_AOP_MINIPMGR_PADDR_S8000_B0,            minipmgr_tunable_b0,     NULL,           false},
	{CHIP_REVISION_B0, ACC_PADDR_S8000_B0,                                  NULL,                    acc_tunable_b0,  true},
	{CHIP_REVISION_B0, SOUTH_BRIDGE_PMSASYNCFIFO_PADDR_S8000_B0,            pmsasyncfifo_tunable_b0, NULL,           true},
	{CHIP_REVISION_B0, SOCBUSMUX_PADDR_S8000_B0,                            socbusmux_tunable_b0,    NULL,           true},
	{CHIP_REVISION_B0, SWITCHFABRIC_PADDR_S8000_B0,                         switchfabric_tunable_b0, NULL,           true},
	{CHIP_REVISION_B0, SOUTH_BRIDGE_PMS_PMS_CSR_PADDR_S8000_B0,             pms_csr_tunable_b0,      NULL,           true},
	{CHIP_REVISION_B0, SOUTH_BRIDGE_DYNAMIC_CLK_GATING_PADDR_S8000_B0,      sb_glue_tunable_b0,      NULL,           true},
	{CHIP_REVISION_B0, SOUTH_BRIDGE_SBR_AXI2AF_AFC_AIU_SB_PADDR_S8000_B0,   afc_aiu_sb_tunable_b0,   NULL,           true},
	{CHIP_REVISION_B0, SOUTH_BRIDGE_ASIO_MISC1_PADDR_S8000_B0,              asio_misc1_tunable_b0,   NULL,           true},
	{CHIP_REVISION_B0, CP_CP_COM_PADDR_S8000_B0,                            cp_com_tunable_b0,       NULL,           true},

	// A1
	{CHIP_REVISION_A1, SOUTH_BRIDGE_PMS_PMGR_PADDR_S8000_A1,                pmgr_tunable_a1,         NULL,           true},
	{CHIP_REVISION_A1, SOUTH_BRIDGE_AOP_MINIPMGR_PADDR_S8000_A1,            minipmgr_tunable_a1,     NULL,           false},
	{CHIP_REVISION_A1, ACC_PADDR_S8000_A1,                                  NULL,                    acc_tunable_a1,  true},
	{CHIP_REVISION_A1, SOUTH_BRIDGE_PMSASYNCFIFO_PADDR_S8000_A1,            pmsasyncfifo_tunable_a1, NULL,           true},
	{CHIP_REVISION_A1, SOCBUSMUX_PADDR_S8000_A1,                            socbusmux_tunable_a1,    NULL,           true},
	{CHIP_REVISION_A1, SWITCHFABRIC_PADDR_S8000_A1,                         switchfabric_tunable_a1, NULL,           true},
	{CHIP_REVISION_A1, SOUTH_BRIDGE_PMS_PMS_CSR_PADDR_S8000_A1,             pms_csr_tunable_a1,      NULL,           true},
	{CHIP_REVISION_A1, SOUTH_BRIDGE_DYNAMIC_CLK_GATING_PADDR_S8000_A1,      sb_glue_tunable_a1,      NULL,           true},
	{CHIP_REVISION_A1, SOUTH_BRIDGE_SBR_AXI2AF_AFC_AIU_SB_PADDR_S8000_A1,   afc_aiu_sb_tunable_a1,   NULL,           true},
	{CHIP_REVISION_A1, SOUTH_BRIDGE_ASIO_MISC1_PADDR_S8000_A1,              asio_misc1_tunable_a1,   NULL,           true},
	{CHIP_REVISION_A1, CP_CP_COM_PADDR_S8000_A1,                            cp_com_tunable_a1,       NULL,           true},

	{0, 0, NULL, NULL, 0}
};
