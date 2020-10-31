/*
 * Copyright (C) 2009-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __APPLE_DISPLAYPIPE_H
#define __APPLE_DISPLAYPIPE_H

#include <platform/soc/hwregbase.h>

#define rDPCVERSION			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x1018))
#define rDPCSTATE			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x101C))
#define rDPCGO				(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x1020))
#define rDPCRESTART			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x1024))
#define rDPCIRQENAB			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x1028))
#define DPCIRQ_MSTRERR		(1<<11)
#define rDPCIRQ				(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x102C))
#define rDPCSIZE			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x1030))
#define rDPCFRMCNT			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x1034))
#define rDPCENAB			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x1038))
#if DISP_VERSION < 5
#define rDPCPFTOP			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x103C))
#endif
#define rDPCPFSIZE			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x1040))
#define rDPCPFSTAT			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x1044))
#define rDPCPFHEAD			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x1048))
#if DISP_VERSION < 5
#define rDPCPFDMA			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x104C))
#define  DPCPFDMA_AUTOMODE		(1 << 4)
#define  DPCPFDMA_BURSTSIZE_1WORD	(0 << 8)
#define  DPCPFDMA_BURSTSIZE_2WORDS	(1 << 8)
#define  DPCPFDMA_BURSTSIZE_4WORDS	(2 << 8)
#define  DPCPFDMA_BURSTSIZE_8WORDS	(3 << 8)
#define  DPCPFDMA_BURSTSIZE_16WORDS	(4 << 8)
#define  DPCPFDMA_BURSTSIZE_MASK	(7 << 8)
#define  DPCPFDMA_WATERMARK(n)		((n) << 16)
#define  DPCPFDMA_WATERMARK_MASK	(0x7FF << 16)
#else 
#define rDPCPFMODE			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x104C))
#define  DPCPFMODE_WBEN			(1 << 1)
#define  DPCPFMODE_AUTOEN		(1 << 0)
#endif
#define rDPCPFFLUSH			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x1050))
#define rDPCCLKCNTL			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x1054))
#define  DPCCLKCNTL_FLOOR(n)		((n && 0xFFFF) << 16)
#define  DPCCLKCNTL_PIPE_ENABLE		(1 << 12)
#define  DPCCLKCNTL_LB_ENABLE		(1 << 8)
#define  DPCCLKCNTL_V_ENABLE		(1 << 4)
#define  DPCCLKCNTL_GATEENAB		(1 << 0)
#define rDPCMEMACC			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x1058))

#if (DISP_VERSION < 3)
#define rDPCPANCNFG			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x105C))
#define  DPCPANCNFG_PANENAB		(1 << 0)
#define  DPCPANCNFG_PANU0ENAB		(1 << 8)
#define  DPCPANCNFG_PANU1ENAB		(1 << 9)
#define  DPCPANCNFG_PANVENAB		(1 << 10)
#define  DPCPANCNFG_PANBENAB		(1 << 11)
#define  DPCPANCNFG_PANTIMER(n)		((n) << 16)
#define  DPCPANCNFG_PANTIMER_MASK	(0xFFFF << 16)
#else
#define rDPCQOSCNFG			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x105C))
#define  DPCQOSCNFG_QOS_ENABLE		(1 << 0)
#define  DPCQOSCNFG_UIFIFO_ENABLE	(1 << 1)
#define  DPCQOSCNFG_VIDFIFO_ENABLE	(1 << 2)
#if (DISP_VERSION < 4)
#define  DPCQOSCNFG_QOS_OPTIMISTIC	(1 << 3)
#else
#define  DPCQOSCNFG_QOS_OPTIMISTIC	(1 << 4)
#endif
#define  DPCQOSCNFG_PIPE_ENABLE		(1 << 8)
#define  DPCQOSCNFG_QOS_TIMER(n)	((n) << 16)
#endif

#define rDPBBACKCOLOR			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x203C))
#define rDPBLAY1CNFG			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x2040))
#define rDPBLAY2CNFG			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x2044))
#define rDPBLAY3CNFG			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x2048))
#define rDPBCRCENAB			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x204C))
#define rDPBCRCDATA			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x2050))
#define rDPBCRCSNAP			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x2054))
#define rDPBOUTSIZE			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x2058))
#define rDPBCLKLVL			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x205C))
#define  DPBCLKLVL_OFFLVL(n)		((n) << 0)
#define  DPBCLKLVL_ONLVL(n)		((n) << 16)
#define rDPBUNDRCNFG			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x2064))
#if (DISP_VERSION < 3)
#define rDPBPANLVL			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x2060))
#else
#define rDPBQOSLVL			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x2060))
#define  DPBQOSLVL_MED_WATERMARK(n)	((n) << 0)
#define  DPBQOSLVL_HIGH_WATERMARK(n)	((n) << 16)
#define rDPBCSCCOEFR(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x4 * (n)) + 0x2068))
#define rDPBCSCCOEFG(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x4 * (n)) + 0x2074))
#define rDPBCSCCOEFB(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x4 * (n)) + 0x2080))
#define rDPBCSCCOFFR			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x208C))
#define rDPBCSCCOFFG			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x2090))
#define rDPBCSCCOFFB			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x2094))
#endif

#if (DISP_VERSION > 4)
#define rDPUREQCFG			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x403C))
#define	DPUREQCFG_REQ_CNT(_n)		((_n) & 0xFF)
#endif
#define rDPUSRCFMT(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4040))
#define rDPUSRCBASE(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4044))
#define rDPUSRCSTRD(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4048))
#define  DPUSRCSTRD_SRCBURST_1BLOCK	(0 << 0)
#define  DPUSRCSTRD_SRCBURST_2BLOCKS	(1 << 0)
#define  DPUSRCSTRD_SRCBURST_4BLOCKS	(2 << 0)
#define  DPUSRCSTRD_SRCBURST_8BLOCKS	(3 << 0)
#define  DPUSRCSTRD_SRCBURST_MASK	(3 << 0)
#define  DPUSRCSTRD_SRCSTRIDE_MASK	(~0x3F)
#if (DISP_VERSION < 3)
#define rDPUSRCRGNENAB(n)		(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x404C))
#define rDPUOUTSIZE(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4070))
#define rDPUCLKLVL(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4074))
#define rDPUPANLVL(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4078))
#define rDPUSRCSTRXY(n, r)		(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4050 + (4 * (r))))
#define rDPUSRCENDXY(n, r)		(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4060 + (4 * (r))))
#else
#define rDPUSRCXY(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x404C))
#define rDPUSRCWH(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4050))
#define rDPUDSTXY(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4054))
#define rDPUDSTWH(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4058))
#define rDPUSRCRGN(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x405C))
#define rDPUSRCSTRXY(n, r)		(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4060 + (4 * (r))))
#define rDPUSRCENDXY(n, r)		(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4070 + (4 * (r))))
#define rDPUDDAINITX(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4080))
#define rDPUDDAINITY(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4084))
#define rDPUDDASTEPX(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4088))
#define rDPUDDASTEPY(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x408C))
#define rDPUMMUCNTL(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + (0x1000 * (n)) + 0x4090))
#endif

#define rDPCPERFCNTL			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x6008))
#define rDPCPERFSTAT			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x600C))
#define rDPCPERFCNFG(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x6010 + (4 * (n))))
#define rDPCPERFDATA(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x6020 + (4 * (n))))
#define rDPCPERFSNAP(n)			(*(volatile u_int32_t *)(DISPLAYPIPE_BASE_ADDR + 0x6030 + (4 * (n))))

#endif /* ! __APPLE_DISPLAYPIPE_H */
