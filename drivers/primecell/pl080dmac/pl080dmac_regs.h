/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _PL080DMAC_REGS_H
#define _PL080DMAC_REGS_H

#include <platform/soc/hwregbase.h>

#define rPL080DMAC_INTSTATUS(_n)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0000)
#define rPL080DMAC_INTTCSTATUS(_n)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0004)
#define rPL080DMAC_INTTCCLR(_n)			*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0008)
#define rPL080DMAC_INTERRSTATUS(_n)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x000C)
#define rPL080DMAC_INTERRCLR(_n)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0010)
#define rPL080DMAC_RAWINTTCSTATUS(_n)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0014)
#define rPL080DMAC_RAWINTERRSTATUS(_n)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0018)
#define rPL080DMAC_ENBLDCHNLS(_n)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x001C)
#define rPL080DMAC_SOFTBREQ(_n)			*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0020)
#define rPL080DMAC_SOFTSREQ(_n)			*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0024)
#define rPL080DMAC_SOFTLBREQ(_n)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0028)
#define rPL080DMAC_SOFTLSREQ(_n)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x002C)
#define rPL080DMAC_CFG(_n)			*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0030)
#define rPL080DMAC_SYNC(_n)			*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0034)
#define rPL080DMAC_CHSRCADDR(_n, _c)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0100 + ((_c) * 0x20))
#define rPL080DMAC_CHDESTADDR(_n, _c)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0104 + ((_c) * 0x20))
#define rPL080DMAC_CHLLI(_n, _c)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0108 + ((_c) * 0x20))
#define rPL080DMAC_CHCTRL(_n, _c)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x010C + ((_c) * 0x20))
#define rPL080DMAC_CHCFG(_n, _c)		*(volatile uint32_t *)(PL080DMAC_BASE_ADDR + ((_n) * PL080DMAC_SPACING) + 0x0110 + ((_c) * 0x20))

#endif /* _PL080DMAC_REGS_H */
