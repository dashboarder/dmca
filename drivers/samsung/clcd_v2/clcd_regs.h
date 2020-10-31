/*
 * Copyright (C) 2009-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __SAMSUNG_CLCD_REGS_H
#define __SAMSUNG_CLCD_REGS_H

#include <platform/soc/hwregbase.h>

#define rCLCD_STATUS			(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0000))
#define rCLCD_CFG			(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0004))
#define rCLCD_INT_EN			(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0008))
#define rCLCD_INT_STATUS		(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x000C))
#define rCLCD_VER			(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0010))
#define rCLCD_DITH_CFG			(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0014))
#define rCLCD_DITH_CONST		(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0018))
#define rCLCD_DPHSYNC_CTRL		(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0020))
#define rCLCD_DPVSYNC_CTRL		(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0024))
#define rCLCD_GM_CON			(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x002C))
#define  CLCD_GM_CON_GM_CO_ENABLE	(1 << 0)
#define  CLCD_GM_CON_GM_CO_TBL_UPDATED	(1 << 6)
#define rCLCD_GM_STATUS			(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0030))
#define rCLCD_GM_TBL_ACC_CON		(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0034))
#define  CLCD_GM_OFF			(0)
#define  CLCD_GM_DEC_RED		(1)
#define  CLCD_GM_DEC_GREEN		(2)
#define  CLCD_GM_DEC_BLUE		(3)
#define  CLCD_GM_COR_RED		(4)
#define  CLCD_GM_COR_GREEN		(5)
#define  CLCD_GM_COR_BLUE		(6)
#define  CLCD_GM_ENC			(7)
#define rCLCD_GM_TBL_WDATA		(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0038))
#define rCLCD_GM_TBL_RDATA		(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x003C))
#define rCLCD_OTF_CON1			(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0050))
#define rCLCD_OTF_CON2			(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0054))
#define rCLCD_OTF_TCON1			(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0058))
#if (DISP_VERSION < 3)
#define  CLCD_OTF_TCON1_VSPP		(1 << 24)
#define  CLCD_OTF_TCON1_VBPD(_l)	(((_l)-1) << 16)
#define  CLCD_OTF_TCON1_VFPD(_l)	(((_l)-1) << 8)
#define  CLCD_OTF_TCON1_VSPW(_l)	(((_l)-1) << 0)
#else
#define  CLCD_OTF_TCON1_VSPP		(1 << 30)
#define  CLCD_OTF_TCON1_VBPD(_l)	(((_l)-1) << 20)
#define  CLCD_OTF_TCON1_VFPD(_l)	(((_l)-1) << 10)
#define  CLCD_OTF_TCON1_VSPW(_l)	(((_l)-1) << 0)
#endif
#define rCLCD_OTF_TCON2			(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x005C))
#if (DISP_VERSION < 4)
#define  CLCD_OTF_TCON2_HBPD(_p)	(((_p)-1) << 16)
#define  CLCD_OTF_TCON2_HFPD(_p)	(((_p)-1) << 8)
#define  CLCD_OTF_TCON2_HSPW(_p)	(((_p)-1) << 0)
#else
#define  CLCD_OTF_TCON2_HBPD(_p)	(((_p)-1) << 20)
#define  CLCD_OTF_TCON2_HFPD(_p)	(((_p)-1) << 10)
#define  CLCD_OTF_TCON2_HSPW(_p)	(((_p)-1) << 0)
#endif
#define rCLCD_OTF_TCON3			(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0060))
#define  CLCD_OTF_TCON3_HOZVAL(_p)	(((_p)-1) << 16)
#define  CLCD_OTF_TCON3_LINEVAL(_p)	(((_p)-1) << 0)
#define rCLCD_OTF_INT_EN		(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0064))
#define rCLCD_OTF_INT_STATUS		(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0068))
#define rCLCD_MIE_SAT_CON		(*(volatile u_int32_t *)(CLCD_BASE_ADDR + 0x0074))

#define rCLCD_DITHER_ENABLE		(*(volatile u_int32_t *)(CLCD_DITHER_BASE_ADDR + 0x0000))
#define rCLCD_DITHER_METHOD		(*(volatile u_int32_t *)(CLCD_DITHER_BASE_ADDR + 0x0004))

#endif /* __SAMSUNG_CLCD_REGS_H */
