/*
 * Copyright (C) 2009-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __PLATFORM_BOOT_PINS_H
#define __PLATFORM_BOOT_PINS_H

#include <platform.h>
#include <platform/gpio.h>
#include <sys/types.h>

struct boot_interface_pin {
	gpio_t	  pin;
	u_int32_t enable;
	u_int32_t disable;
};

#include SUB_PLATFORM_HEADER(boot_pins)

#endif /* ! __PLATFORM_BOOT_PINS_H */
