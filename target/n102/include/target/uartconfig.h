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
#ifndef __TARGET_UARTCONFIG_H
#define __TARGET_UARTCONFIG_H

/* define one debug serial ports */
#define DEBUG_SERIAL_PORT		(0)

/* only enable those UARTs required by iBoot - namely debug serial port */
#define UARTS_MASK		(0x01)

#define HDQGAUGE_SERIAL_PORT		(5)

#if SUPPORT_FPGA
#define UART_OVERSAMPLE_OVERRIDE        (11)
#endif

#endif
