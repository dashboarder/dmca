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
#ifndef __SAMSUNG_RGBOUT_REGS_H
#define __SAMSUNG_RGBOUT_REGS_H

#include <platform/soc/hwregbase.h>

#define rRGBOUTCTL			(*(volatile u_int32_t *)(RGBOUT_BASE_ADDR + 0x00))
#define rRGBOUTCFG			(*(volatile u_int32_t *)(RGBOUT_BASE_ADDR + 0x04))
#define rRGBOUTSRESET                   (*(volatile u_int32_t *)(RGBOUT_BASE_ADDR + 0x08))                           
#define rRGBOUTDITHCFG			(*(volatile u_int32_t *)(RGBOUT_BASE_ADDR + 0x1C))
#define rRGBOUTOTFCON			(*(volatile u_int32_t *)(RGBOUT_BASE_ADDR + 0x24))
#define rRGBOUTOTFTCON3			(*(volatile u_int32_t *)(RGBOUT_BASE_ADDR + 0x34))
#define rRGBOUTOTFTCON6			(*(volatile u_int32_t *)(RGBOUT_BASE_ADDR + 0x40))
#define rRGBOUTGMCON			(*(volatile u_int32_t *)(RGBOUT_BASE_ADDR + 0x4C))
#define rRGBOUTGMTBLACCCON              (*(volatile u_int32_t *)(RGBOUT_BASE_ADDR + 0x54))
#define rRGBOUTGMTBLWDATA               (*(volatile u_int32_t *)(RGBOUT_BASE_ADDR + 0x58))
#define rRGBOUTPOPLATENCY		(*(volatile u_int32_t *)(RGBOUT_BASE_ADDR + 0x6C))
#define rRGBOUTDPCONSTCOLOR		(*(volatile u_int32_t *)(RGBOUT_BASE_ADDR + 0x7C))

#endif /* __SAMSUNG_RGBOUT_REGS_H */