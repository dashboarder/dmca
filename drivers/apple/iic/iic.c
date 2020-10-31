/*
 * Copyright (C) 2010 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <debug.h>
#include <drivers/iic.h>
#include <platform.h>
#include <platform/int.h>
#include <platform/clocks.h>
#include <platform/timer.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/soc/pmgr.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <sys.h>
#include <sys/menu.h>
#include <sys/task.h>

#include "iic.h"

#ifndef IICS_MASK
#define IICS_MASK		((1 << IICS_COUNT) - 1)
#endif

static struct iic_device *get_iic_device(int port);
static void iic_setup(struct iic_device *dev);
static void iic_set_clock(struct iic_device *dev, u_int32_t period);
static void iic_reset_bus(struct iic_device *dev);

void iic_init(void)
{
	int iic;

	dprintf(DEBUG_INFO, "iic_init()\n");

	for (iic = 0; iic < IICS_COUNT; iic++) {
		if (!((1 << iic) & IICS_MASK))
			continue;
		iic_setup(get_iic_device(iic));
	}
}

void iic_set_filter(int iic, uint32_t value) {
	rFILTER(iic) = value;
}

void iic_set_frequency(int iic, u_int32_t frequency)
{
	struct iic_device *dev;
	u_int32_t period = 1000000000/frequency;

	dev = get_iic_device(iic);
	if (dev == 0) return;

	iic_set_clock(dev, period);
}

int iic_read(int iic, u_int8_t address, const void *send_data, size_t send_len, void *data, size_t len, iic_fmt_t fmt)
{
	struct iic_device *dev = get_iic_device(iic);
	u_int8_t *dptr = (u_int8_t *)send_data;
	u_int32_t dat, to_send;
	int ret = 0;

	if (!dev) return -1;
	if (!(IICS_MASK & (1 << iic))) return -1;
	if (len < 1) return 0;

	while (dev->busy) task_yield();
	dev->busy = true;

	/* Possibilities:
	   - no sub-address + N bytes
	   - sub-address + N bytes
	   - sub-address + N bytes combined (restarted read)
	*/

	rSMSTA(iic) = kS5L8940XIICSMSTAxen | kS5L8940XIICSMSTAmtn;

	to_send = send_len;
	if (send_len) {
		to_send++;
		enter_critical_section();
		dat = (kS5L8940XIICMTXFIFOWrite |
				 kS5L8940XIICMTXFIFOStart |
				 kS5L8940XIICMTXFIFOData(address&~1));
		rMTXFIFO(iic) = dat;
		while (send_len--) {
			dat = kS5L8940XIICMTXFIFOData(*dptr);
			if ((fmt == IIC_NORMAL) && (send_len == 0))
				dat |= kS5L8940XIICMTXFIFOStop;
			rMTXFIFO(iic) = dat;
			dptr++;
		}
		exit_critical_section();
	}
	dptr = data;
	enter_critical_section();
	rMTXFIFO(iic) = dat = (kS5L8940XIICMTXFIFOStart |
			       kS5L8940XIICMTXFIFOData(address|1));
	rMTXFIFO(iic) = dat = (kS5L8940XIICMTXFIFORead | kS5L8940XIICMTXFIFOStop |
			       kS5L8940XIICMTXFIFOData(len));
	exit_critical_section();
	while (len--) {
		while ((dat = rMRXFIFO(iic)) & kS5L8940XIICMRXFIFOEmpty) {
			if (rSMSTA(iic) & kS5L8940XIICSMSTAmtn) {
				dprintf(DEBUG_INFO, "iic_read(%d) NAK\n", iic);
				rCTL(iic) = rCTL(iic) | kS5L8940XIICCTLMTR; /* clear the FIFO first */
				rSMSTA(iic) = kS5L8940XIICSMSTAmtn;	    /* now blast the NAK bit */
				ret = -1;
				goto out;
			}
		}
		*dptr++ = kS5L8940XIICMRXFIFOData(dat);
	}

 out:
	dev->busy = false;
	return ret;
}

int iic_write(int iic, u_int8_t address, const void *data, size_t len)
{
	struct iic_device *dev = get_iic_device(iic);
	const u_int8_t *dptr = data;
	u_int32_t dat;
	int ret = 0;

	if (!dev) return -1;
	if (!(IICS_MASK & (1 << iic))) return -1;

	/* Don't support "quick command" */
	if (len < 1) return -1;

	while (dev->busy) task_yield();
	dev->busy = true;

	rSMSTA(iic) = kS5L8940XIICSMSTAxen | kS5L8940XIICSMSTAmtn;

	address &= ~1;
	enter_critical_section();
	rMTXFIFO(iic) = (kS5L8940XIICMTXFIFOWrite | kS5L8940XIICMTXFIFOStart |
			 kS5L8940XIICMTXFIFOData(address));
	/* XXX bust out early if NAK observed? */
	while (len--) {
		dat = kS5L8940XIICMTXFIFOData(*dptr);
		if (len == 0)
			dat |= kS5L8940XIICMTXFIFOStop;
		rMTXFIFO(iic) = dat;
		dptr++;
	}
	exit_critical_section();
	/* Spin til the transaction is ended or standard 1-second timeout */
	SPIN_W_TMO_WHILE(!(rSMSTA(iic) & kS5L8940XIICSMSTAxen));
	/* Check for a NAK and clean up if necessary */
	if (rSMSTA(iic) & kS5L8940XIICSMSTAmtn) {
		dprintf(DEBUG_INFO, "iic_write(%d) NAK\n", iic);
		rCTL(iic) = rCTL(iic) | kS5L8940XIICCTLMTR; /* clear the FIFO first */
		rSMSTA(iic) = kS5L8940XIICSMSTAmtn;	    /* now blast the NAK bit */
		ret = -1;
	}

	dev->busy = false;
	return ret;
}

bool iic_probe(int iic, u_int8_t address)
{
	/* might be nice to implement this */
	return false;
}

// private

static struct iic_device _iic_device[] = {
#if IICS_COUNT > 0
	{
		.bus = 0,
		.clk = CLK_I2C0,
		.irq = INT_IIC0,
		.gpio_scl = GPIO_IIC0_SCL,
		.gpio_sda = GPIO_IIC0_SDA,
	},
#endif
#if IICS_COUNT > 1
	{
		.bus = 1,
		.clk = CLK_I2C1,
		.irq = INT_IIC1,
		.gpio_scl = GPIO_IIC1_SCL,
		.gpio_sda = GPIO_IIC1_SDA,
	},
#endif
#if IICS_COUNT > 2
	{
		.bus = 2,
		.clk = CLK_I2C2,
		.irq = INT_IIC2,
		.gpio_scl = GPIO_IIC2_SCL,
		.gpio_sda = GPIO_IIC2_SDA,
	},
#endif	
#if IICS_COUNT > 3
	{
		.bus = 3,
		.clk = CLK_I2C3,
		.irq = INT_IIC3,
		.gpio_scl = GPIO_IIC3_SCL,
		.gpio_sda = GPIO_IIC3_SDA,
	},
#endif
};

static struct iic_device *get_iic_device(int port)
{
	if (!((1 << port) & IICS_MASK)) return 0;
	
	return &_iic_device[port];
}

static void iic_setup(struct iic_device *dev)
{
	if (dev == 0) return;

	/* make sure the clock is enabled for iic */
	clock_gate(dev->clk, true);

	/* Set for 400KHz */
	iic_set_clock(dev, 2500);

	iic_reset_bus(dev);

	rSMSTA(dev->bus) = 0xffffffff;
	rIMASK(dev->bus) = 0;
}

static void iic_set_clock(struct iic_device *dev, u_int32_t period)
{
	u_int32_t source;
	u_int32_t target_Hz;
	unsigned div;

	/* Obey the minimum supported by the block */
	div = kS5L8940XIICMIXDIV;
	/* There's a built-in divide by 16 */
	source = (clock_get_frequency(dev->clk)) >> 4;
	target_Hz = 1000000000 / period;
	while (source > (target_Hz * div))
		div++;
	rCTL(dev->bus) = kS5L8940XIICCTLCLK(div) | kS5L8940XIICCTLUJM;
	dprintf(DEBUG_INFO, "iic_set_clock(%d) = %d\n", dev->bus, div);
}

static void iic_reset_bus(struct iic_device *dev)
{
	u_int32_t cnt;
	
	// Write 00 so all devices will complete any partial transaction
	// SDA starts low and goes high after the loop
	// SCL start and ends the loop high
	// The first and last iterations of the loop produce the
	// start and stop conditions

	gpio_configure(dev->gpio_sda, GPIO_CFG_OUT_0);
	
	for (cnt = 0; cnt < (2 * 9 + 1); cnt++) {
		gpio_configure(dev->gpio_scl, (cnt & 1) ? GPIO_CFG_OUT_0 : GPIO_CFG_IN);
		spin(5);
	}
	
	gpio_configure(dev->gpio_scl, GPIO_CFG_FUNC0);
	gpio_configure(dev->gpio_sda, GPIO_CFG_FUNC0);

	rCTL(dev->bus) = rCTL(dev->bus) | kS5L8940XIICCTLMRR | kS5L8940XIICCTLMTR;
}
