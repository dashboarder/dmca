/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#pragma once

#define OSCAR_ROM_BAUDRATE	1500000
#define OSCAR_CONSOLE_BAUDRATE	(OSCAR_ROM_BAUDRATE / 26)	/* close to 57600 */

/* N56 dev */
#define POWER_GPIO_OSCAR_RESET	(5)				/* GPIO6 */
#define OSCAR_SERIAL_PORT	(8)
