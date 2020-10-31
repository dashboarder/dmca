/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/spi.h>
#include <platform.h>
#include <platform/gpio.h>
#include <platform/soc/hwregbase.h>

#include "gpio.h"

#if WITH_TARGET_CONFIG
# include <target/pinconfig.h>
#else
# include <platform/pinconfig.h>
#endif

u_int32_t gpio_read(gpio_t gpio)
{
	int pad = GPIO2PAD(gpio);
	int pin = GPIO2PIN(gpio);

#if WITH_HW_SPI && defined(GPIO_PAD_SPI)
	if (pad == GPIO_PAD_SPI) return spi_gpio_read(pin);
#endif

	return (rPDATn(pad) & (1<<pin)) ? 1UL : 0UL;
}

void gpio_write(gpio_t gpio, u_int32_t val)
{
	int pad = GPIO2PAD(gpio);
	int pin = GPIO2PIN(gpio);

#if WITH_HW_SPI && defined(GPIO_PAD_SPI)
	if (pad == GPIO_PAD_SPI) {	
		spi_gpio_write(pin, val);
		return;
	}
#endif

	rFSEL = (pad << 16) | (pin << 8) | (val ? PCON_OUT_1 : PCON_OUT_0);
}

static const int gpio_config_map[GPIO_CFG_MAX] = {
	[GPIO_CFG_IN]		= PCON_IN,
	[GPIO_CFG_OUT]		= PCON_OUT,
	[GPIO_CFG_OUT_0]	= PCON_OUT_0,
	[GPIO_CFG_OUT_1]	= PCON_OUT_1,
	[GPIO_CFG_FUNC(0)]	= PCON_FUNC2,
	[GPIO_CFG_FUNC(1)]	= PCON_FUNC3,
	[GPIO_CFG_FUNC(2)]	= PCON_FUNC4,
	[GPIO_CFG_FUNC(3)]	= PCON_FUNC5
};

void gpio_configure(gpio_t gpio, u_int32_t config)
{
	int pad = GPIO2PAD(gpio);
	int pin = GPIO2PIN(gpio);
	int func;

#if WITH_HW_SPI && defined(GPIO_PAD_SPI)
	if (pad == GPIO_PAD_SPI) {
		spi_gpio_configure(pin, config);
		return;
	}
#endif

	if (config > GPIO_CFG_MAX) return;

	if (config == GPIO_CFG_DFLT) {
		func = (gpio_default_config[pad].pcon >> (pin * 4)) & 0xF;
	} else {
		func = gpio_config_map[config];
 	}

	if (func == PCON_DISABLE) {
		rPPIEn(pad) &= ~(1 << pin);
		rFSEL = (pad << 16) | (pin << 8) | PCON_IN;
	} else {
		rFSEL = (pad << 16) | (pin << 8) | func;
		rPPIEn(pad) |= (1 << pin);
	}

	/* Once we reconfigure, reset the pullup/pulldown to default. This could cause
	   a glitch on outputs, but will avoid spurious transitions on inputs. We're
	   between a rock and a hard place because of the GPIO block's design. */
	if (config == GPIO_CFG_DFLT) {
		uint16_t pupdn = gpio_default_config[pad].pupdn;
		uint16_t pullup = pupdn & (1 << pin);
		uint16_t pulldown = (pupdn >> 8) & (1 << pin);

		if ((pullup != 0) && (pulldown != 0)) {
			panic("Pullup and pulldown at same time on GPIO 0x%x", gpio);
		} else if (pullup != 0) {
			rPPURn(pad) |= (1 << pin);
			rPPDRn(pad) &= ~(1 << pin);
		} else if (pulldown != 0) {
			rPPURn(pad) &= ~(1 << pin);
			rPPDRn(pad) |= (1 << pin);
		} else {
			rPPURn(pad) &= ~(1 << pin);
			rPPDRn(pad) &= ~(1 << pin);
		}
	}
}

void gpio_configure_pupdn(gpio_t gpio, int32_t pupdn)
{
	int pad = GPIO2PAD(gpio);
	int pin = 1 << GPIO2PIN(gpio);

	if	  (pupdn < GPIO_NO_PUPDN) {
		/* GPIO_PDN */
		rPPURn(pad) &= ~pin;
		rPPDRn(pad) |= pin;
	} else if (pupdn > GPIO_NO_PUPDN) {
	       	/* GPIO_PUP */
		rPPURn(pad) |= pin;
		rPPDRn(pad) &= ~pin;
	} else {
		/* GPIO_NO_PUPDN */
		rPPURn(pad) &= ~pin;
		rPPDRn(pad) &= ~pin;
	}
}

int gpio_init_pinconfig(void)
{
	int pad;
	int pin;

	dprintf(DEBUG_SPEW, "gpio_init_pinconfig()\n");

	/* XXX is there a safe way to set the pullup/pulldown config and set the pin modes? */

	/* 
	 * since most of these pins are probably going to default to input coming out of the
	 * boot rom, we'll set the pullup/pulldown first
	 */

	/* load the default pullup/pulldown configuration, disallowing
	   the pullup and pulldown to be set at the same time */
	for (pad = 0; pad < GPIO_GROUP_COUNT; pad++) {
		uint32_t pullup;
		uint32_t pulldown;
		uint16_t pupdn;

		pupdn = gpio_default_config[pad].pupdn;
		pullup = pupdn & 0xFF;
		pulldown = (pupdn >> 8) & 0xFF;
		if ((pullup & pulldown) != 0)
			panic("Pullup and pulldown conflict on pins 0x%x of pad %d", pullup & pulldown, pad);
		rPPURn(pad) = pullup;
		rPPDRn(pad) = pulldown;
	}

	/* load the default PCON values, which should also set initial PDATs for gpio out */
	for (pad = 0; pad < GPIO_GROUP_COUNT; pad++)
	{
		uint32_t pcon;
		uint32_t ppie;
		ppie = 0xFF;
		pcon = gpio_default_config[pad].pcon;

		for (pin = 0; pin < 8; pin++) {
			if (((pcon >> (pin * 4)) & 0xF) == PCON_DISABLE) {
				pcon &= ~(0xF << (pin * 4));
				ppie &= ~(1 << pin);
			}
		}
		rPCONn(pad) = pcon;
		rPPIEn(pad) = ppie;
	}

#ifdef DEFAULT_ODEN
	/* set port configuration for I2Cx and TWC */
	rODEN = DEFAULT_ODEN;
#endif

#if defined(DEFAULT_DSTR0)
	/* set the chip default drive strength */
	rDSTR0 = DEFAULT_DSTR0;
#endif

#ifdef DEFAULT_OSC_DSTR
	/* set oscillator drive strength */
	rOSC_DSTR = DEFAULT_OSC_DSTR;
#endif

	return 0;
}

#if APPLICATION_IBOOT

int gpio_diag_pinconfig(void)
{
	return 0;
}

#endif /* APPLICATION_IBOOT */
