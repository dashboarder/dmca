/*
 * Copyright (C) 2008-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __SAMSUNG_DSIM_H
#define __SAMSUNG_DSIM_H

#include <platform/soc/hwregbase.h>

#define rDSIM_STATUS			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x00))
#define   rDSIM_STATUS_PllStable	(0x80000000)
#define   rDSIM_STATUS_SwRstRls		(0x00100000)
#define   rDSIM_STATUS_Direction	(0x00010000)
#define   rDSIM_STATUS_TxReadyHsClk	(0x00000400)
#define   rDSIM_STATUS_UlpsClk		(0x00000200)
#define   rDSIM_STATUS_StopstateClk	(0x00000100)
#define   rDSIM_STATUS_UlpsDatMsk	(0xf << 4)
#define   rDSIM_STATUS_UlpsDat(d)	(((d) & 0xf) << 4)
#define   rDSIM_STATUS_StopstateDatMsk	(0xf << 0)
#define   rDSIM_STATUS_StopstateDat(d)	(((d) & 0xf) << 0)
#define rDSIM_SWRST			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x04))
#define   rDSIM_SWRST_FuncRst		(0x00010000)
#define   rDSIM_SWRST_SwRst		(0x00000001)
#define rDSIM_CLKCTRL			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x08))
#define   rDSIM_CLKCTRL_TxRequestHsClk	(0x80000000)
#define   rDSIM_CLKCTRL_EscClkEn	(0x10000000)
#define   rDSIM_CLKCTRL_PllBypass	(0x08000000)
#define   rDSIM_CLKCTRL_ByteClkSrcMsk	(0x3 << 25)
#define   rDSIM_CLKCTRL_ByteClkSrc(d)	(((d) & 0x3) << 25)
#define   rDSIM_CLKCTRL_ByteClkEn	(0x01000000)
#define   rDSIM_CLKCTRL_LaneEscClkEnMsk	(0xf << 20)
#define   rDSIM_CLKCTRL_LaneEscClkEn(d)	(((d) & 0xf) << 20)
#define   rDSIM_CLKCTRL_LaneEscClkEnClk	(0x00080000)
#define   rDSIM_CLKCTRL_EscPrescalar(d)	((d) & 0xffff)
#define rDSIM_TIMEOUT			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x0C))
#define rDSIM_CONFIG			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x10))
#define   rDSIM_CONFIG_DPDN_Test_En	(0x80000000)
#define   rDSIM_CONFIG_TxTypeSfr	(0x10000000)
#define   rDSIM_CONFIG_SyncInform	(0x08000000)
#define   rDSIM_CONFIG_BurstMode	(0x04000000)
#define   rDSIM_CONFIG_VideoMode	(0x02000000)
#define   rDSIM_CONFIG_AutoMode		(0x01000000)
#define   rDSIM_CONFIG_HseMode		(0x00800000)
#define   rDSIM_CONFIG_HfpMode		(0x00400000)
#define   rDSIM_CONFIG_HbpMode		(0x00200000)
#define   rDSIM_CONFIG_HsaMode		(0x00100000)
#define   rDSIM_CONFIG_MainVc(d)	(((d) & 0x3) << 18)
#define   rDSIM_CONFIG_MainPixFormat(d)	(((d) & 0x7) << 12)
#define   rDSIM_CONFIG_NumOfDatLaneMsk	(0x3 << 5)
#define   rDSIM_CONFIG_NumOfDatLane(d)	(((d) & 0x3) << 5)
#define   rDSIM_CONFIG_LaneEnMsk	(0xf << 1)
#define   rDSIM_CONFIG_LaneEn(d)	(((d) & 0xf) << 1)
#define   rDSIM_CONFIG_LaneClkEn	(0x00000001)
#define rDSIM_ESCMODE			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x14))
#define   rDSIM_ESCMODE_ForceStopstate	(0x00100000)
#define   rDSIM_ESCMODE_ForceBTA	(0x00010000)
#if DSIM_VERSION == 3
#define   rDSIM_ESCMODE_Auto_Ulps_Data	(1 << 9)
#define   rDSIM_ESCMODE_Auto_Ulps_Clk	(1 << 8)
#endif // DSIM_VERSION == 3
#define   rDSIM_ESCMODE_CmdLpdt		(0x00000080)
#define   rDSIM_ESCMODE_TxLpdt		(0x00000040)
#define   rDSIM_ESCMODE_TxTriggerRst	(0x00000010)
#define   rDSIM_ESCMODE_TxUlpsDat	(0x00000008)
#define   rDSIM_ESCMODE_TxUlpsExit	(0x00000004)
#define   rDSIM_ESCMODE_TxUlpsClk	(0x00000002)
#define   rDSIM_ESCMODE_TxUlpsClkExit	(0x00000001)
#define rDSIM_MDRESOL			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x18))
#define   rDSIM_MDRESOL_MainStandby	(0x80000000)
#define   rDSIM_MDRESOL_MainVResol(d)	(((d) & 0x7ff) << 16)
#define   rDSIM_MDRESOL_MainHResol(d)	((d) & 0x7ff)
#define rDSIM_MVPORCH			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x1C))
#define   rDSIM_MVPORCH_CmdAllow(d)	(((d) & 0xf) << 28)
#define   rDSIM_MVPORCH_StableVfp(d)	(((d) & 0x7ff) << 16)
#define   rDSIM_MVPORCH_MainVbp(d)	((d) & 0x7ff)
#define rDSIM_MHPORCH			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x20))
#define rDSIM_MSYNC			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x24))
#define rDSIM_SSCNT			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x28))
#define rDSIM_INTSRC			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x2C))
#define   rDSIM_INTSRC_SwRstRelease	(0x40000000)
#define   rDSIM_INTSRC_PllStable	(0x80000000)
#define   rDSIM_INTSRC_LpdrTout		(0x00200000)
#define   rDSIM_INTSRC_RxDatDone	(0x00040000)
#define   rDSIM_INTSRC_RxAck		(0x00010000)
#define   rDSIM_INTSRC_AnyError		(0x0000ffff)
#define rDSIM_INTMSK			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x30))
#define rDSIM_PKTHDR			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x34))
#define rDSIM_PAYLOAD			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x38))
#define rDSIM_RXFIFO			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x3C))
#define rDSIM_FIFOTHLD			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x40))
#define rDSIM_FIFOCTRL			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x44))
#define   rDSIM_FIFOCTRL_FullRx		(0x02000000)
#define   rDSIM_FIFOCTRL_EmptyRx	(0x01000000)
#define   rDSIM_FIFOCTRL_FullHSfr	(0x00800000)
#define   rDSIM_FIFOCTRL_EmptyHSfr	(0x00400000)
#define   rDSIM_FIFOCTRL_FullLSfr	(0x00200000)
#define   rDSIM_FIFOCTRL_EmptyLSfr	(0x00100000)
#define   rDSIM_FIFOCTRL_FullHI80	(0x00080000)
#define   rDSIM_FIFOCTRL_EmptyHI80	(0x00040000)
#define   rDSIM_FIFOCTRL_FullLI870	(0x00020000)
#define   rDSIM_FIFOCTRL_EmptyLI80	(0x00010000)
#define   rDSIM_FIFOCTRL_FullHMain	(0x00000800)
#define   rDSIM_FIFOCTRL_EmptyHMain	(0x00000400)
#define   rDSIM_FIFOCTRL_FullLMain	(0x00000200)
#define   rDSIM_FIFOCTRL_EmptyLMain	(0x00000100)
#define   rDSIM_FIFOCTRL_nInitRx	(0x00000010)
#define   rDSIM_FIFOCTRL_nInitSfr	(0x00000008)
#define   rDSIM_FIFOCTRL_nInitI80	(0x00000004)
#define   rDSIM_FIFOCTRL_nInitMain	(0x00000001)
#define rDSIM_MEMACCHR			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x48))
#define rDSIM_PLLCTRL			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x4C))
#define   rDSIM_PLLCTRL_HSzeroCtl(d)	(((d) & 0xf) << 28)
#define   rDSIM_PLLCTRL_FreqBand(d)	(((d) & 0xf) << 24)
#define   rDSIM_PLLCTRL_PllEn		(0x00800000)
#define   rDSIM_PLLCTRL_PreprCtl(d)	(((d) & 0x7) << 20)
#if DSIM_VERSION == 0
#define   rDSIM_PLLCTRL_P(d)		(((d) & 0x3f) << 14)
#define   rDSIM_PLLCTRL_M(d)		(((d) & 0x3ff) << 4)
#else
#define   rDSIM_PLLCTRL_P(d)		(((d) & 0x3f) << 13)
#define   rDSIM_PLLCTRL_M(d)		(((d) & 0x1ff) << 4)
#endif
#define   rDSIM_PLLCTRL_S(d)		(((d) & 0x7) << 1)
#define   rDSIM_PLLCTRL_DpDnSwap	(0x00000001)
#define rDSIM_PLLTMR			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x50))
#define rDSIM_PHYACCHR			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x54))
#if DSIM_VERSION < 2
#define   rDSIM_PHYACCHR_UP_CODE(d)	(((d) & 0xc) << (26 - 2))
#else
#define   rDSIM_PHYACCHR_UP_CODE(d)	(0 << 0)
#endif
#define   rDSIM_PHYACCHR_AFC_ENABLE	(0x00004000)
#define   rDSIM_PHYACCHR_DPHYCTL(d)	(((d) & 0x3f) << 8)
#define   rDSIM_PHYACCHR_AFC(d)		(((d) & 0x7) << 5)
#if DSIM_VERSION < 2
#define   rDSIM_PHYACCHR_DOWN_CODE(d)	(((d) & 0x1) << (3 - 0))
#else
#define   rDSIM_PHYACCHR_DOWN_CODE(d)	(0 << 0) //NOOP
#endif
#define rDSIM_PHYACCHR2			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x58))
#if DSIM_VERSION < 2
#define   rDSIM_PHYACCHR2_UP_CODE(d)	((((d) & 0x2) << (17 - 1)) | (((d) & 0x1) << (8 - 0)))
#define   rDSIM_PHYACCHR2_DOWN_CODE(d)	(0 << 0) //NOOP
#else
#define   rDSIM_PHYACCHR2_UP_CODE(d)	(((d) & 0x7) << 19)
#define   rDSIM_PHYACCHR2_DOWN_CODE(d)	(((d) & 0x7) << 4)
#endif
#define   rDSIM_PHYACCHR2_PrprCtlClk(d)	(((d) & 0x7) << 16)

#if DSIM_VERSION == 3
#define rDSIM_ULPSIN			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x60))
#define rDSIM_ULPSOUT			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x64))
#define rDSIM_ULPSEND			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x68))
#endif //DSIM_VERSION == 3

#define rDSIM_SUPPRESS			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x6C))
#define   rDSIM_SUPPRESS_Line_Timeout_Disable (0x00000001)

#if DSIM_VERSION == 3
#define rDSIM_I80_LPVALID_ST		(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x70))
#endif //DSIM_VERSION == 3

#define rDSIM_VERINFO			(*(volatile u_int32_t *)(DSIM_BASE_ADDR + 0x0C))

#endif /* __SAMSUNG_DSIM_H */
