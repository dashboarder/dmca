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
#include <debug.h>
#include <drivers/display.h>
#include <drivers/iic.h>
#include <platform/clocks.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>

#include "tmds.h"

int tmds_init(struct display_timing *timing, enum colorspace color, u_int32_t *display_id)
{
	u_int8_t data[2];

	clock_set_frequency(timing->host_clock_id, 0, 0, 0, 0, timing->pixel_clock);
	/* XXX what about CLCD dithering? */

	gpio_configure(GPIO_LCD_VCLK, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_VSYNC, GPIO_CFG_FUNC1);	// XXX GPIO OUT_1 eventually
	gpio_configure(GPIO_LCD_HSYNC, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_VDEN, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_VDEN, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D0, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D1, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D2, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D3, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D4, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D5, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D6, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D7, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D8, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D9, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D10, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D11, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D12, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D13, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D14, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D15, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D16, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D17, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D18, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D19, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D20, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D21, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D22, GPIO_CFG_FUNC1);
	gpio_configure(GPIO_LCD_D23, GPIO_CFG_FUNC1);

	data[0] = kTFP_DE_CTL;
	data[1] = 0;		/* Active high DE is assumed */
	if (!timing->neg_hsync)
		data[1] |= kTFP_DE_CTL_HS_POL;
	if (!timing->neg_vsync)
		data[1] |= kTFP_DE_CTL_VS_POL;
	iic_write(TMDS_IIC_BUS, kTFP_ADDR_W, data, 2);

	data[0] = kTFP_CTL_1_MODE;
	data[1] = kTFP_CTL_1_MODE_nPD | kTFP_CTL_1_MODE_BSEL;
	if (timing->neg_vclk)
		data[1] |= kTFP_CTL_1_MODE_EDGE;
	iic_write(TMDS_IIC_BUS, kTFP_ADDR_W, data, 2);

	return 0;
}

int tmds_quiesce(void)
{
	dprintf(DEBUG_CRITICAL, "tmds_quiesce()\n");
	return 0;
}

