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
#define	DBEMODECNTL_AAP_ENABLE		(1 << 31)
#define	DBEMODECNTL_DPB_ENABLE		(1 << 30)
#define	DBEMODECNTL_BN_DITHER_ENABLE	(1 << 29)
#define	DBEMODECNTL_ST_DITHER_ENABLE	(1 << 28)
#define	DBEMODECNTL_DPB_BUSY_MASK	(1 << 23)
#define	DBEMODECNTL_PMGR_CLK_GATE_ENABLE (1 << 22)
#define	DBEMODECNTL_DYN_CLK_GATE_ENABLE	(1 << 21)
#define	DBEMODECNTL_BLK_CLK_GATE_ENABLE	(1 << 20)

#define DBEMODECNTL_DITHER_ENABLE	(1 << 28)
#define DBEMODECNTL_PRC_ENABLE		(1 << 27)
#define DBEMODECNTL_WPC_ENABLE		(1 << 26)

#define rDBEVFTGCTL			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x8))
#define	 DBEVFTGCT_VFTG_ENABLE		(1 << 31)
#define	 DBEVFTGCT_VFTG_STATUS		(1 << 30)
#define	 DBEVFTGCT_FRAME_COUNT_ENABLE	(1 << 29)
#define	 DBEVFTGCT_FRAME_COUNT_RESET	(1 << 28)
#define  DBEVFTGCT_IDLE_FRAME_VBLANK_ENABLE (1 << 24)
#define	 DBEVFTGCT_VSYNC_POLARITY(n)	((n) << 19)
#define	 DBEVFTGCT_HSYNC_POLARITY(n)	((n) << 18)
#define	 DBEVFTGCT_SCAN_SELECT(n)	((n) << 16)
#define	 DBEVFTGCT_UPDATE_ENABLE_TIMING	(1 << 15)
#define	 DBEVFTGCT_UPDATE_REQ_TIMING	(1 << 14)
#define	 DBEVFTGCT_CH2_SEL(n)		((n) << 12)
#define	 DBEVFTGCT_CH1_SEL(n)		((n) << 10)
#define	 DBEVFTGCT_CH0_SEL(n)		((n) << 8)
#define	 DBEVFTGCT_VERTICAL_STATUS(n)	((n) << 4)
#define	 DBEVFTGCT_HORIZONTAL_STATUS(n)	((n) << 0)

#define rDBESCRNSZ			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0xC))
#define rDBEFRONTPORCH			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x10))
#define rDBESYNCPULSE			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x14))
#define rDBEBACKPORCH			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x18))
#define rDBECOUNTER_STATUS		(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x1C))
#define rDBECOUNTER_POSITION		(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x20))
#define rDBEVBLANK_POSITION		(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x24))
#define rDBEVBLANKCLKGATE		(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x28))
#define rDBEVBLANKBUSY			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x2C))
#define rDBEISR				(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x30))
#define rDBECONST_COLOR			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x34))
#define rDBECRC_CTL			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x38))
#define rDBECRCWINDOW			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x3C))
#define rDBECRCRESULT			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x40))
#define rDBEFIFO_CONFIG			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x44))
#define rDBEFIFO_STATUS			(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x48))
#define rDBESPARE_CONFIG0		(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x4C))
#define rDBESPARE_CONFIG1		(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x50))
#define rDBESPARE_CONFIG2		(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x54))
#define rDBESPARE_CONFIG3		(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x58))
#define rDBESPARE_STATUS0		(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x5C))
#define rDBESPARE_STATUS1		(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x60))
#define rDBESPARE_STATUS2		(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x64))
#define rDBESPARE_STATUS3		(*(volatile u_int32_t *)(DISP0_ADBE_BASE_ADDR + 0x68))

#define rAAP_FORMAT_CONTROL_REG0	(*(volatile u_int32_t *)(DISP0_AAP_BASE_ADDR + 0x0))
#define rAAP_FORMAT_CONTROL_REG1	(*(volatile u_int32_t *)(DISP0_AAP_BASE_ADDR + 0x4))
#define  AAP_FORMAT_CONTROL_REG1_AUTOSIZE (1 << 7)
#define  AAP_FORMAT_CONTROL_REG1_AUTOPOS  (1 << 6)
#define  AAP_FORMAT_CONTROL_REG1_FCMODE(n)  ((n & 0x3) << 4)
#define  AAP_FORMAT_CONTROL_REG1_RSVD(n)  ((n & 0xf) << 2)
#define  AAP_FORMAT_CONTROL_REG1_VS_POL	  (1 << 1)
#define  AAP_FORMAT_CONTROL_REG1_HS_POL   (1 << 0)
#define rAAP_FORMAT_HS_POS_LSB		(*(volatile u_int32_t *)(DISP0_AAP_BASE_ADDR + 0x8))
#define rAAP_FORMAT_HS_POS_MSB		(*(volatile u_int32_t *)(DISP0_AAP_BASE_ADDR + 0xC))
#define rAAP_FORMAT_FRAME_WIDTH_LSB	(*(volatile u_int32_t *)(DISP0_AAP_BASE_ADDR + 0x10))
#define rAAP_FORMAT_FRAME_WIDTH_MSB	(*(volatile u_int32_t *)(DISP0_AAP_BASE_ADDR + 0x14))
#define rAAP_FORMAT_FRAME_HEIGHT_LSB	(*(volatile u_int32_t *)(DISP0_AAP_BASE_ADDR + 0x18))
#define rAAP_FORMAT_FRAME_HEIGHT_MSB	(*(volatile u_int32_t *)(DISP0_AAP_BASE_ADDR + 0x1C))
#define rAAP_FORMAT_VS_POS_LSB		(*(volatile u_int32_t *)(DISP0_AAP_BASE_ADDR + 0x20))
#define rAAP_FORMAT_VS_POS_MSB		(*(volatile u_int32_t *)(DISP0_AAP_BASE_ADDR + 0x24))

#endif /* __ADBE_REGS_H */
