/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#ifndef __SAMSUNG_TIMER_H
#define __SAMSUNG_TIMER_H

#include <platform/soc/hwregbase.h>

/**
*   Timer A
*/
#define rTACON				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x0))
#define rTACMD				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x4))
#define rTADATA0			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x8))
#define rTADATA1			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xC))
#define rTAPRE				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x10))
#define rTACNT				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x14))
/**
*   Timer B
*/
#define rTBCON				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x20))
#define rTBCMD				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x24))
#define rTBDATA0			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x28))
#define rTBDATA1			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x2C))
#define rTBPRE				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x30))
#define rTBCNT				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x34))
/**
*   Timer C
*/
#define rTCCON				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x40))
#define rTCCMD				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x44))
#define rTCDATA0			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x48))
#define rTCDATA1			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x4C))
#define rTCPRE				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x50))
#define rTCCNT				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x54))
/**
*   Timer D
*/
#define rTDCON				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x60))
#define rTDCMD				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x64))
#define rTDDATA0			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x68))
#define rTDDATA1			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x6C))
#define rTDPRE				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x70))
#define rTDCNT				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x74))

//64-Bit Timer Register
#define rTM64_CNTH			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x80))
#define rTM64_CNTL			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x84))
#define rTM64_CON			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x88))

#define rTM64_DATA0H			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x8C))
#define rTM64_DATA0L			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x90))
#define rTM64_DATA1H			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x94))
#define rTM64_DATA1L			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x98))
/**
*   Timer E
*/
#define rTECON				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xA0))
#define rTECMD				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xA4))
#define rTEDATA0			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xA8))
#define rTEDATA1			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xAC))
#define rTEPRE				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xB0))
#define rTECNT				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xB4))
/**
*   Timer F
*/
#define rTFCON				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xC0))
#define rTFCMD				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xC4))
#define rTFDATA0			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xC8))
#define rTFDATA1			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xCC))
#define rTFPRE				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xD0))
#define rTFCNT				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xD4))
/**
*   Timer G
*/
#define rTGCON				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xE0))
#define rTGCMD				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xE4))
#define rTGDATA0			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xE8))
#define rTGDATA1			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xEC))
#define rTGPRE				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xF0))
#define rTGCNT				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xF4))
/**
*   Timer H
*/
#define rTHCON				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x100))
#define rTHCMD				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x104))
#define rTHDATA0			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x108))
#define rTHDATA1			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x10C))
#define rTHPRE				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x100))
#define rTHCNT				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x104))

#if TIMER_VERSION == 0
#define rTM32_INT			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0xF8))
#else
#define rTM32_INT			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x118))
#endif

#define rTM_INT				(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x10000))
/**
*   Timer counter register (RO)
*/
#define rTM32_CNT_E			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x10004))
#define rTM32_CNT_F			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x10008))
#define rTM32_CNT_G			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x1000C))

#define rTM64_CNTH2			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x10010))
#define rTM64_CNTL2			(*(volatile u_int32_t *)(TIMER_BASE_ADDR + 0x10014))


#define rWDTCON				(*(volatile u_int32_t *)(WDT_BASE_ADDR + 0x00))
#define rWDTCNT				(*(volatile u_int32_t *)(WDT_BASE_ADDR + 0x04))

#endif /* __SAMSUNG_TIMER_H */
