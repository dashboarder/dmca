/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* if no hardware IIC, use the software fallback */
#if !WITH_HW_IIC

#include <debug.h>
#include <drivers/iic.h>
#include <platform.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <sys.h>
#include <sys/task.h>

#ifndef IICS_MASK
#define IICS_MASK		((1 << IICS_COUNT) - 1)
#endif

static struct		iic_soft_device *get_iic_soft_device(int port);
static void		iic_soft_setup(struct iic_soft_device *dev);
static void		iic_soft_reset_bus(struct iic_soft_device *dev);

static void		_setSCL(struct iic_soft_device *dev, bool assert);
static void		_setSDA(struct iic_soft_device *dev, bool assert);
static u_int32_t	_getSDA(struct iic_soft_device *dev);
static void		_sendStart(struct iic_soft_device *dev);
static void		_sendRestart(struct iic_soft_device *dev);
static void		_sendStop(struct iic_soft_device *dev);
static void		_sendAck(struct iic_soft_device *dev, bool ack);
static bool		_writeByte(struct iic_soft_device *dev, u_int8_t data);
static u_int8_t		_readByte(struct iic_soft_device *dev, bool ack);


/* struct definition */
struct iic_soft_device {
	u_int32_t	_i2cPeriod;
	u_int32_t	_i2cSlaveDevice;

	u_int32_t	iicAddressLength;
	const u_int8_t	*iicAddressBuffer;
	u_int32_t	iicDataLength;
	u_int8_t	*iicDataBuffer;

	u_int32_t	_i2cDelay;
	u_int32_t	_i2cSDA;
	gpio_t		gpio_scl;
	gpio_t		gpio_sda;
};


void iic_init(void)
{
	int iic;

	dprintf(DEBUG_INFO, "iic_soft_init()\n");

	for (iic = 0; iic < IICS_COUNT; iic++)
		iic_soft_setup(get_iic_soft_device(iic));
}

void iic_set_filter(int iic, uint32_t value) {}

void iic_set_frequency(int iic, u_int32_t frequency) {}

int iic_read(int iic, u_int8_t address, const void *send_data, size_t send_len, void *data, size_t len, iic_fmt_t fmt)
{
	struct iic_soft_device *dev = get_iic_soft_device(iic);
	u_int32_t cnt = 0;

	if (dev == 0) return -1;
	if (send_len == 0) return -1;

	dev->_i2cSlaveDevice = address;

	dev->iicAddressLength = send_len;
	dev->iicAddressBuffer = (const u_int8_t *)send_data;
	dev->iicDataLength    = len;
	dev->iicDataBuffer    = (u_int8_t *)data;

	/* Send slave address */
	_sendStart(dev);
	if (!_writeByte(dev, dev->_i2cSlaveDevice & ~(0x1))) {
	    _sendStop(dev);
	    return -1;
	}

	/* Send address bytes */
	for (cnt = 0; cnt < dev->iicAddressLength; cnt++) {
	    if (!_writeByte(dev, dev->iicAddressBuffer[cnt])) {
		_sendStop(dev);
		return -1;
	    }
	}

	if (fmt == IIC_NORMAL) {
		/* Stop */
		_sendStop(dev);
		_sendStart(dev);
	} else {
		_sendRestart(dev);
	}

	/* Send slave address */
	if (!_writeByte(dev, dev->_i2cSlaveDevice | (0x1))) {
	    _sendStop(dev);
	    return -1;
	}
	/* clock data in */
	for (cnt = 0; cnt < dev->iicDataLength; cnt++) {
	    dev->iicDataBuffer[cnt] = _readByte(dev, cnt < (dev->iicDataLength -1));
	}
	/* Stop */
	_sendStop(dev);
	return 0;
}

int iic_write(int iic, u_int8_t address, const void *data, size_t len)
{
	struct iic_soft_device *dev = get_iic_soft_device(iic);
	u_int32_t cnt =0;

	if (dev == 0) return -1;

	dev->_i2cSlaveDevice = address;

	dev->iicAddressLength = 0;
	dev->iicAddressBuffer = 0;
	dev->iicDataLength    = len;
	dev->iicDataBuffer    = (u_int8_t *)data;

	/* Send slave address */
	_sendStart(dev);
	if (!_writeByte(dev, dev->_i2cSlaveDevice & ~(0x1))) {
	    _sendStop(dev);
            return -1;
	}

	/* Send data bytes */
	for (cnt = 0; cnt < dev->iicDataLength; cnt++) {
	    if (!_writeByte(dev, dev->iicDataBuffer[cnt])) {
		_sendStop(dev);
		return -1;
	    }
	}

	/* Send stop */
	_sendStop(dev);
	return 0;
}

bool iic_probe(int iic, u_int8_t address)
{
	/* might be nice to implement this */
	return false;
}


/* Private */
static struct iic_soft_device _iic_soft_device[] = {
#if IICS_COUNT > 0
	{
		.gpio_scl = GPIO_IIC0_SCL,
		.gpio_sda = GPIO_IIC0_SDA,
	},
#endif
#if IICS_COUNT > 1
	{
		.gpio_scl = GPIO_IIC1_SCL,
		.gpio_sda = GPIO_IIC1_SDA,
	},
#endif
#if IICS_COUNT > 2
	{
		.gpio_scl = GPIO_IIC2_SCL,
		.gpio_sda = GPIO_IIC2_SDA,
	}
#endif
};

static struct iic_soft_device *get_iic_soft_device(int port)
{
	if (!((1 << port) & IICS_MASK))
		return 0;
	return &_iic_soft_device[port];
}

static void iic_soft_setup(struct iic_soft_device *dev)
{
	if (dev == 0) return;

	/* set the clock as 400kHz */
//	dev->_i2cPeriod = 2500;
//	dev->_i2cDelay = (dev->_i2cPeriod + 3)/4;

	/*reset the bus */
	iic_soft_reset_bus(dev);
}

static void iic_soft_reset_bus(struct iic_soft_device *dev)
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

	gpio_configure(dev->gpio_scl, GPIO_CFG_IN);
	gpio_configure(dev->gpio_sda, GPIO_CFG_IN);
}

static void _setSCL(struct iic_soft_device *dev, bool assert)
{
	if (assert) {
		gpio_configure(dev->gpio_scl, GPIO_CFG_IN);
	} else {
		gpio_configure(dev->gpio_scl, GPIO_CFG_OUT_0);
	}
}

static void _setSDA(struct iic_soft_device *dev, bool assert)
{
	if (assert) {
		gpio_configure(dev->gpio_sda, GPIO_CFG_IN);
	} else {
		gpio_configure(dev->gpio_sda, GPIO_CFG_OUT_0);
	}
}

static u_int32_t _getSDA(struct iic_soft_device *dev)
{
	u_int32_t tmp;

	gpio_configure(dev->gpio_sda, GPIO_CFG_IN);

	tmp = gpio_read(dev->gpio_sda);

	return tmp;
}


/* Send Start: SDA High -> Low while SCL is high */
static void _sendStart(struct iic_soft_device *dev)
{
	spin(1);
	spin(1);
	_setSDA(dev, false);
	spin(1);
}

/* Send Restart: XXX */
static void _sendRestart(struct iic_soft_device *dev)
{
	_setSCL(dev, false);
	spin(1);
	_setSDA(dev, true);
	spin(1);
	_setSCL(dev, true);
	spin(1);
	_setSDA(dev, false);
}

/* Send Stop: SDA Low -> High while SCL is high */
static void _sendStop(struct iic_soft_device *dev)
{
	_setSCL(dev, false);
	spin(1);
	_setSDA(dev, false);
	spin(1);
	_setSCL(dev, true);
	spin(1);
	_setSDA(dev, true);
}


static void _sendAck(struct iic_soft_device *dev, bool ack)
{
	_setSCL(dev, false);
	spin(1);
	_setSDA(dev, !ack);
	spin(1);
	_setSCL(dev, true);
	spin(1);
	spin(1);
}

static bool _writeByte(struct iic_soft_device *dev, u_int8_t data)
{
	u_int32_t cnt;
	u_int32_t ack;

	/* Send the byte */
	for (cnt = 0; cnt < 8; cnt++) {
	    _setSCL(dev, false);
	    spin(1);
	    _setSDA(dev, data & 0x80);
	    spin(1);
	    _setSCL(dev, true);
	    spin(1);
	    spin(1);

	    data = (data << 1);	
	}

	/* Look for the ACK */
	_setSCL(dev, false);
	spin(1);
	_setSDA(dev, true);
	spin(1);
	_setSCL(dev, true);
	spin(1);
	ack = !_getSDA(dev);
	spin(1);

	return (ack != 0);
}

static u_int8_t _readByte(struct iic_soft_device *dev, bool ack)
{
	u_int32_t cnt;
	u_int8_t data = 0;

	for (cnt = 0; cnt < 8; cnt++) {
	    _setSCL(dev, false);
	    spin(1);
	    _setSDA(dev, true);
	    spin(1);
	    _setSCL(dev, true);
	    spin(1);
	    data <<= 1;
	    data |= _getSDA(dev);
	    spin(1);
	}
	_sendAck(dev, ack);

	return data;
}

#endif /* !WITH_HW_IIC */
