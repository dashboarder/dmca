/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __TARGET_RADIOCONFIG_H
#define __TARGET_RADIOCONFIG_H

/* configuration per target */

#define RADIO_VERIFY_BASEBAND_EXISTS		1

static const gpio_t target_radio_quiesce_pins[] =
{
	GPIO_UART1_TXD
};

#define RADIO_ENABLE_IMEI_OVERRIDE 0

#endif /* ! __TARGET_RADIOCONFIG_H */
