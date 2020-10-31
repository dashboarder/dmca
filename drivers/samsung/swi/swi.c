/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <platform.h>
#include <platform/clocks.h>
#include <platform/gpio.h>
#include <platform/soc/hwclocks.h>
#include <target/gpiodef.h>

#include "swi.h"

#define SWI_FREQUENCY	(2000000)

int swi_backlight_enable(u_int32_t iset_code, u_int32_t backlight_level)
{
	u_int32_t swi_div;

	clock_gate(CLK_SWI, true);

	swi_div = (clock_get_frequency(CLK_NCLK) + SWI_FREQUENCY - 1) / SWI_FREQUENCY;

	rSWI_CON = SWI_CON_SWI_CLK_DIV(swi_div - 1) | SWI_CON_SWI_OFF_STATE_HIGH | SWI_CON_SWI_EN;

	/* Some targets support 8 bits, others support 11.  Assume that the input value is in range. */
	rSWI_ITR_DATA = 0x0080 | iset_code | ((backlight_level & 0x780) << 1) | (backlight_level & 0x7F);

	rSWI_ITR_COM = SWI_ITR_COM_SWI_ITR_MODE_16BIT | SWI_ITR_COM_SWI_ITR_SET;
	while ((rSWI_ITR_COM & SWI_ITR_COM_SWI_ITR_SET) != 0);

#ifdef GPIO_BACKLIGHT_EN
	gpio_write(GPIO_BACKLIGHT_EN, (backlight_level != 0) ? GPIO_BACKLIGHT_POLARITY : !GPIO_BACKLIGHT_POLARITY);
#endif

	return 0;
}
