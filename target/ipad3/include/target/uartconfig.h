/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __TARGET_UARTCONFIG_H
#define __TARGET_UARTCONFIG_H

/* define two debug serial ports */
#define DEBUG_SERIAL_PORT		(0)

/* define radio serial port */
#define RADIO_SERIAL_PORT		(1)
#define RADIO_MAX_READ_ERRORS		(10)

/* define HDQ serial port */
#define HDQGAUGE_SERIAL_PORT		(5)

#define UART_FLOWCONTROL_MASK		(0x08)

#endif
