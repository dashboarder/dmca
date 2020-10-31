/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __ADBE_REGS_H
#define __ADBE_REGS_H

#include <platform/soc/hwregbase.h>

#define rDBEMODECNTL			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x4))
#define DBEMODECNTL_ENABLE		(1 << 31) 
#define DBEMODECNTL_PRE_CSC_LUT_ENABLE	(1 << 30) 
#define DBEMODECNTL_CSC_ENABLE		(1 << 29) 
#define DBEMODECNTL_POST_CSC_LUT_ENABLE	(1 << 28) 
#define DBEMODECNTL_AAP_ENABLE		(1 << 27) 
#define DBEMODECNTL_DPB_ENABLE		(1 << 26) 
#define DBEMODECNTL_BN_DITHER_ENABLE	(1 << 25) 
#define DBEMODECNTL_ST_DITHER_ENABLE	(1 << 24) 
#define DBEMODECNTL_DYN_CLK_GATE_ENABLE	(1 << 23) 
#define DBEMODECNTL_AFC_TEARDOWN_ENABLE	(1 << 22) 
#define DBEMODECNTL_PMGR_CLK_GATE_ENABL	(1 << 21) 
#define DBEMODECNTL_BLK_CLK_GATE_ENABLE	(1 << 20) 
#define DBEMODECNTL_VSYNC_POLARITY	(1 << 19) 
#define DBEMODECNTL_HSYNC_POLARITY	(1 << 18) 
#define DBEMODECNTL_I_P_SELECT(x)	(x << 16) 
#define DBEMODECNTL_UPDATE_ENABLE_TIMING (1 << 15) 
#define DBEMODECNTL_UPDATE_REQ_TIMING	(1 << 14) 
#define DBEMODECNTL_CH2_SEL(x)		(x << 12) 
#define DBEMODECNTL_CH1_SEL(x)		(x << 10) 
#define DBEMODECNTL_CH0_SEL(x)		(x << 8) 
#define DBEMODECNTL_CRC_ENABLE		(1 << 7) 
#define DBEMODECNTL_CRC_MULTIFRAME_ENABLE (1 << 6) 
#define DBEMODECNTL_CRC_WINDOW_ENABLE	(1 << 5) 
#define DBEMODECNTL_CRC_VALID		(1 << 4) 
#define DBEMODECNTL_VFTG_STATUS		(1 << 0) 

#define rDBECONSTCOLOR			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x8))
#define rDBESCRNSZ			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0xC))
#define rDBEFRONTPORCH			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x10))
#define rDBESYNCPULSE			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x14))
#define rDBEBACKPORCH			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x18))
#define rDBEVBLANKPOS			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x1C))
#define rDBEVBLANKCLKGATE		(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x20))
#define rDBEVBLANKIDLE			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x24))
#define rDBECRCWINDOW			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x28))
#define rDBECRCRESULT			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x2C))

//Only Alcatraz has the color manager on the back end.  No need to generated a new driver just for it.
#define	rCM_DEGAMMA_RED(n)		(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x0400 + ((n) * 4)))
#define	rCM_DEGAMMA_GREEN(n)		(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x0800 + ((n) * 4)))
#define	rCM_DEGAMMA_BLUE(n)		(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x0c00 + ((n) * 4)))
#define	rCM_CSC_00			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1000)
#define	rCM_MATRIX_BASE(n)		(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1000 + ((n) * 4)))
#define	rCM_CSC_01			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1004))
#define	rCM_CSC_02			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1008))
#define	rCM_CSC_C0			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x100c))
#define	rCM_CSC_CI0			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1010))
#define	rCM_CSC_10			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1014))
#define	rCM_CSC_11			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1018))
#define	rCM_CSC_12			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x101c))
#define	rCM_CSC_C1			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1020))
#define	rCM_CSC_CI1			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1024))
#define	rCM_CSC_20			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1028))
#define	rCM_CSC_21			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1020))
#define	rCM_CSC_22			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1030))
#define	rCM_CSC_C2			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1034))
#define	rCM_CSC_CI2			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1038))
#define	rCM_LUT_CTL			(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x103c))
#define	 CM_LUT_CTL			(DISP_CM_BASE_ADDR + 0x103c)
#define	 CM_LUT_CTL_UPDATE_ENABLE_ENG_BLUE	(1 << 23)
#define	 CM_LUT_CTL_UPDATE_REQ_ENG_BLUE		(1 << 22)
#define	 CM_LUT_CTL_BYPASS_ENG_BLUE		(1 << 21)
#define	 CM_LUT_CTL_UPDATE_ENABLE_DEG_BLUE	(1 << 19)
#define	 CM_LUT_CTL_UPDATE_REQ_DEG_BLUE		(1 << 18)
#define	 CM_LUT_CTL_BYPASS_DEG_BLUE		(1 << 17)
#define	 CM_LUT_CTL_UPDATE_ENABLE_ENG_GREEN	(1 << 15)
#define	 CM_LUT_CTL_UPDATE_REQ_ENG_GREEN	(1 << 14)
#define	 CM_LUT_CTL_BYPASS_ENG_GREEN		(1 << 13)
#define	 CM_LUT_CTL_UPDATE_ENABLE_DEG_GREEN	(1 << 11)
#define	 CM_LUT_CTL_UPDATE_REQ_DEG_GREEN	(1 << 10)
#define	 CM_LUT_CTL_BYPASS_DEG_GREEN		(1 << 9)
#define	 CM_LUT_CTL_UPDATE_ENABLE_ENG_RED	(1 << 7)
#define	 CM_LUT_CTL_UPDATE_REQ_ENG_RED		(1 << 6)
#define	 CM_LUT_CTL_BYPASS_ENG_RED		(1 << 5)
#define	 CM_LUT_CTL_UPDATE_ENABLE_DEG_RED	(1 << 3)
#define	 CM_LUT_CTL_UPDATE_REQ_DEG_RED		(1 << 2)
#define	 CM_LUT_CTL_BYPASS_DEG_RED		(1 << 1)
#define	rCM_ENGAMMA_RED(n)		(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x1200 + ((n) * 4)))
#define	rCM_ENGAMMA_GREEN(n)		(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x2200 + ((n) * 4)))
#define	rCM_ENGAMMA_BLUE(n)		(*(volatile u_int32_t *)(DISP_CM_BASE_ADDR + 0x3200 + ((n) * 4)))
#endif /* __ADBE_REGS_H */
