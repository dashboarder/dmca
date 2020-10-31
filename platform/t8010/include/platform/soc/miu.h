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
#ifndef __PLATFORM_SOC_MIU_H
#define __PLATFORM_SOC_MIU_H

#include <lib/devicetree.h>
#include <platform/soc/hwregbase.h>
#include SUB_PLATFORM_SPDS_HEADER(afc_aiu_sb)
#include SUB_PLATFORM_SPDS_HEADER(amcc)
#include SUB_PLATFORM_SPDS_HEADER(cp_com)
#include SUB_PLATFORM_SPDS_HEADER(minipmgr)
#include SUB_PLATFORM_SPDS_HEADER(misc1)
#include SUB_PLATFORM_SPDS_HEADER(pms_csr)
#include SUB_PLATFORM_SPDS_HEADER(sb_glue)
#include SUB_PLATFORM_SPDS_HEADER(socbusmux)
#include SUB_PLATFORM_SPDS_HEADER(switchfabric)

// https://seg-docs.ecs.apple.com/projects/cayman//release/UserManual/regs/MINIPMGR.html?baseaddr=0x210200000#MINIPMGR_SECURITY
#define	rSECUREROMCTRL_ROMADDRREMAP		(*(volatile u_int32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_SECURITY_MCC_ROMADDRREMAP_OFFSET))
#define  SECUREROMCTRL_ROMADDRREMAP_SRAM	(1 << 0)
#define  SECUREROMCTRL_ROMADDRREMAP_SDRAM	(1 << 1)

// https://seg-docs.ecs.apple.com/projects/cayman//release/UserManual/regs/amcc.html?baseaddr=0x200000000
#define	MCCLOCKREGION_TZ0BASEADDR(_n)		(AMCC_BASE_ADDR(_n) + AMCC_MCC_LOCK_REGION_TZ0_BASE_ADDR_OFFSET)
#define	MCCLOCKREGION_TZ0ENDADDR(_n)		(AMCC_BASE_ADDR(_n) + AMCC_MCC_LOCK_REGION_TZ0_END_ADDR_OFFSET)
#define	MCCLOCKREGION_TZ0LOCK(_n)		(AMCC_BASE_ADDR(_n) + AMCC_MCC_LOCK_REGION_TZ0_LOCK_OFFSET)

#define	rMCCLOCKREGION_TZ0BASEADDR(_n)		(*(volatile u_int32_t *)(MCCLOCKREGION_TZ0BASEADDR(_n)))
#define	rMCCLOCKREGION_TZ0ENDADDR(_n)		(*(volatile u_int32_t *)(MCCLOCKREGION_TZ0ENDADDR(_n)))
#define	rMCCLOCKREGION_TZ0LOCK(_n)		(*(volatile u_int32_t *)(MCCLOCKREGION_TZ0LOCK(_n)))

#define rAMCC_IRERRDBG_INTEN(_n)		(*(volatile uint32_t *)(AMCC_BASE_ADDR(_n) + AMCC_IRERRDBG_INTEN_OFFSET))
#define rAMCC_IRERRDBG_INTSTS(_n)		(*(volatile uint32_t *)(AMCC_BASE_ADDR(_n) + AMCC_IRERRDBG_INTSTS_OFFSET))
#define rAMCC_IRERRDBG_MCC_AF_ERR_LOG2(_n)	(*(volatile uint32_t *)(AMCC_BASE_ADDR(_n) + AMCC_IRERRDBG_MCC_AF_ERR_LOG2_OFFSET))

// https://seg-docs.ecs.apple.com/projects/cayman//release/UserManual/regs/SWITCHFABRIC.html?baseaddr=0x200800000
#define rSWITCH_FAB_CPG_CNTL			(*(volatile u_int32_t *)(SWTCH_FAB_BASE_ADDR + SWITCHFABRIC_BLK_REGS_CPG_CNTL_OFFSET))

// https://seg-docs.ecs.apple.com/projects/cayman//release/UserManual/regs/CP_COM.html?baseaddr=0x200d00000
#define rCP_COM_DYN_CLK_GATING_CTRL		(*(volatile u_int32_t *)(CP_COM_BASE_ADDR + CP_COM_CFG_DYN_CLK_GATE_CTRL_OFFSET))
#define rCP_COM_INT_NORM_REQUEST		(*(volatile u_int32_t *)(CP_COM_INT_BASE_ADDR + CP_COM_INT_NORM_REQUEST_OFFSET))
#define rCP_COM_INT_NORM_MASK_SET		(*(volatile u_int32_t *)(CP_COM_INT_BASE_ADDR + CP_COM_INT_NORM_MASK_SET_OFFSET))
#define rCP_COM_INT_NORM_MASK_CLR		(*(volatile u_int32_t *)(CP_COM_INT_BASE_ADDR + CP_COM_INT_NORM_MASK_CLR_OFFSET))

// https://seg-docs.ecs.apple.com/projects/cayman//release/UserManual/regs/SOCBUSMUX.html?baseaddr=0x204000000
#define rSOCBUSMUX_CPG_CNTL			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + SOCBUSMUX_BLK_REGS_CPG_CNTL_OFFSET))

// https://seg-docs.ecs.apple.com/projects/cayman//release/UserManual/regs/Misc1.html?baseaddr=0x20a100000
#define rASIO_CLK_CTRL				(*(volatile u_int32_t *)(ASIO_MISC_1_BASE_ADDR + MISC1_BLK_CLOCK_CONTROL_OFFSET))

// https://seg-docs.ecs.apple.com/projects/cayman//release/UserManual/regs/pmsCSR.html?baseaddr=0x20e400000
#define rPMS_SRAM_CPG_CTRL			(*(volatile u_int32_t *)(PMS_CRS_BASE_ADDR + PMS_CSR_PMSCSR_PMP_PMS_SRAM_CPG_CTRL_OFFSET))
#define rPMP_CPG_CTRL				(*(volatile u_int32_t *)(PMS_CRS_BASE_ADDR + PMS_CSR_PMSCSR_PMP_PMP_CPG_CTRL_OFFSET))
#define rPMS_CPG_CTRL				(*(volatile u_int32_t *)(PMS_CRS_BASE_ADDR + PMS_CSR_PMSCSR_PMP_PMS_CPG_CTRL_OFFSET))

// https://seg-docs.ecs.apple.com/projects/cayman//release/UserManual/regs/afc_aiu_sb.html?baseaddr=0x20f000000
#define rAIU_SB_CPG_CNTL			(*(volatile u_int32_t *)(AFC_AIU_SB_BASE_ADDR + AFC_AIU_SB_SELF_OFFSET))

// https://seg-docs.ecs.apple.com/projects/cayman//release/UserManual/regs/DYNAMIC_CLK_GATING.html?baseaddr=0x20f1c0000
#define rDYN_CLK_GATING				(*(volatile u_int32_t *)(SB_GLUE_REG_BASE + SB_GLUE_BLK_DYNAMIC_CLK_GATING_OFFSET))
#define rDYN_CLK_GATING2			(*(volatile u_int32_t *)(SB_GLUE_REG_BASE + SB_GLUE_BLK_DYNAMIC_CLK_GATING2_OFFSET))

enum remap_select {
  REMAP_SRAM = 0,
  REMAP_SDRAM
};

extern void miu_select_remap(enum remap_select sel);
extern void miu_bypass_prep(void);
extern void miu_update_device_tree(DTNode *pmgr_node);

#endif /* ! __PLATFORM_SOC_MIU_H */
