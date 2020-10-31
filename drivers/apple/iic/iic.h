/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#ifndef __APPLE_IIC_H
#define __APPLE_IIC_H

#include <platform/gpio.h>
#include <platform/soc/hwregbase.h>

#define rMTXFIFO(_i)			(*(volatile u_int32_t *)(IIC_BASE_ADDR + ((_i) * IIC_SPACING) + 0x00))
#define rMRXFIFO(_i)			(*(volatile u_int32_t *)(IIC_BASE_ADDR + ((_i) * IIC_SPACING) + 0x04))
#define rMCNT(_i)			(*(volatile u_int32_t *)(IIC_BASE_ADDR + ((_i) * IIC_SPACING) + 0x08))
#define rXFSTA(_i)			(*(volatile u_int32_t *)(IIC_BASE_ADDR + ((_i) * IIC_SPACING) + 0x0C))
#define rSMSTA(_i)			(*(volatile u_int32_t *)(IIC_BASE_ADDR + ((_i) * IIC_SPACING) + 0x14))
#define rIMASK(_i)			(*(volatile u_int32_t *)(IIC_BASE_ADDR + ((_i) * IIC_SPACING) + 0x18))
#define rCTL(_i)			(*(volatile u_int32_t *)(IIC_BASE_ADDR + ((_i) * IIC_SPACING) + 0x1C))
#define rVERSION(_i)			(*(volatile u_int32_t *)(IIC_BASE_ADDR + ((_i) * IIC_SPACING) + 0x28))
#define rFILTER(_i)			(*(volatile u_int32_t *)(IIC_BASE_ADDR + ((_i) * IIC_SPACING) + 0x38))

#define kS5L8940XIICMTXFIFOWrite		(0 << 10)
#define kS5L8940XIICMTXFIFORead			(1 << 10)
#define kS5L8940XIICMTXFIFOStop			(1 << 9)
#define kS5L8940XIICMTXFIFOStart		(1 << 8)
#define kS5L8940XIICMTXFIFOData(_d)		(((_d) & 0xff) << 0)

#define kS5L8940XIICMRXFIFOEmpty		(1 << 8)
#define kS5L8940XIICMRXFIFOData(_v)		(((_v) >> 0) & 0xff)

#define kS5L8940XIICMCNTRxCnt(_v)		(((_v) >> 8) & 0xff)
#define kS5L8940XIICMCNTTxCnt(_v)		(((_v) >> 0) & 0xff)

#define kS5L8940XIICXFSTAmst(_v)		(((_v) >> 28) & 0xf)
#define kS5L8940XIIC_IDLE	0
#define kS5L8940XIICXFSTAxfifo(_v)		(((_v) >> 21) & 0x3)
#define kS5L8940XIICXFSTAxcnt(_v)		(((_v) >> 0) & 0xfffff)

#define kS5L8940XIICSMSTAxip			(1 << 28)
#define kS5L8940XIICSMSTAxen			(1 << 27)
#define kS5L8940XIICSMSTAujf			(1 << 26)
#define kS5L8940XIICSMSTAjmd			(1 << 25)
#define kS5L8940XIICSMSTAjam			(1 << 24)
#define kS5L8940XIICSMSTAmto			(1 << 23)
#define kS5L8940XIICSMSTAmtn			(1 << 21)
#define kS5L8940XIICSMSTAmrf			(1 << 20)
#define kS5L8940XIICSMSTAmrne			(1 << 19)
#define kS5L8940XIICSMSTAmtr			(1 << 18)
#define kS5L8940XIICSMSTAmtf			(1 << 17)
#define kS5L8940XIICSMSTAmte			(1 << 16)
#define kS5L8940XIICSMSTAtom			(1 << 6)

#define kS5L8940XIICCTLMRR			(1 << 10)
#define kS5L8940XIICCTLMTR			(1 << 9)
#define kS5L8940XIICCTLUJM			(1 << 8)
#define kS5L8940XIICCTLCLK(_c)			(((_c) & 0xff) << 0)

#define kS5L8940XIICMIXDIV			4

struct iic_device {
	int             bus;
	int		clk;
	int             irq;
	bool            busy;
	gpio_t		gpio_scl;
	gpio_t		gpio_sda;
};

#endif /* ! __APPLE_IIC_H */
