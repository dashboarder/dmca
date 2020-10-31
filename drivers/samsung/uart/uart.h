/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#ifndef __SAMSUNG_UART_H
#define __SAMSUNG_UART_H

#include <platform/soc/hwregbase.h>

#define rULCON0				(*(volatile u_int32_t *)(UART0_BASE_ADDR + 0x00))
#define rUCON0				(*(volatile u_int32_t *)(UART0_BASE_ADDR + 0x04))
#define rUFCON0				(*(volatile u_int32_t *)(UART0_BASE_ADDR + 0x08))
#define rUMCON0				(*(volatile u_int32_t *)(UART0_BASE_ADDR + 0x0C))
#define rUTRSTAT0			(*(volatile u_int32_t *)(UART0_BASE_ADDR + 0x10))
#define rUERSTAT0			(*(volatile u_int32_t *)(UART0_BASE_ADDR + 0x14))
#define rUFSTAT0			(*(volatile u_int32_t *)(UART0_BASE_ADDR + 0x18))
#define rUMSTAT0			(*(volatile u_int32_t *)(UART0_BASE_ADDR + 0x1C))
#define rUBRDIV0			(*(volatile u_int32_t *)(UART0_BASE_ADDR + 0x28))
#define rUABRCNT0			(*(volatile u_int32_t *)(UART0_BASE_ADDR + 0x2C))
#define rUTXH0				(*(volatile u_int32_t *)(UART0_BASE_ADDR + 0x20))
#define rURXH0				(*(volatile u_int32_t *)(UART0_BASE_ADDR + 0x24))

#define rULCON1				(*(volatile u_int32_t *)(UART1_BASE_ADDR + 0x00))
#define rUCON1				(*(volatile u_int32_t *)(UART1_BASE_ADDR + 0x04))
#define rUFCON1				(*(volatile u_int32_t *)(UART1_BASE_ADDR + 0x08))
#define rUMCON1				(*(volatile u_int32_t *)(UART1_BASE_ADDR + 0x0C))
#define rUTRSTAT1			(*(volatile u_int32_t *)(UART1_BASE_ADDR + 0x10))
#define rUERSTAT1			(*(volatile u_int32_t *)(UART1_BASE_ADDR + 0x14))
#define rUFSTAT1			(*(volatile u_int32_t *)(UART1_BASE_ADDR + 0x18))
#define rUMSTAT1			(*(volatile u_int32_t *)(UART1_BASE_ADDR + 0x1C))
#define rUBRDIV1			(*(volatile u_int32_t *)(UART1_BASE_ADDR + 0x28))
#define rUABRCNT1			(*(volatile u_int32_t *)(UART1_BASE_ADDR + 0x2C))
#define rUTXH1				(*(volatile u_int32_t *)(UART1_BASE_ADDR + 0x20))
#define rURXH1				(*(volatile u_int32_t *)(UART1_BASE_ADDR + 0x24))

#define rULCON2				(*(volatile u_int32_t *)(UART2_BASE_ADDR + 0x00))
#define rUCON2				(*(volatile u_int32_t *)(UART2_BASE_ADDR + 0x04))
#define rUFCON2				(*(volatile u_int32_t *)(UART2_BASE_ADDR + 0x08))
#define rUMCON2				(*(volatile u_int32_t *)(UART2_BASE_ADDR + 0x0C))
#define rUTRSTAT2			(*(volatile u_int32_t *)(UART2_BASE_ADDR + 0x10))
#define rUERSTAT2			(*(volatile u_int32_t *)(UART2_BASE_ADDR + 0x14))
#define rUFSTAT2			(*(volatile u_int32_t *)(UART2_BASE_ADDR + 0x18))
#define rUMSTAT2			(*(volatile u_int32_t *)(UART2_BASE_ADDR + 0x1C))
#define rUBRDIV2			(*(volatile u_int32_t *)(UART2_BASE_ADDR + 0x28))
#define rUABRCNT2			(*(volatile u_int32_t *)(UART2_BASE_ADDR + 0x2C))
#define rUTXH2				(*(volatile u_int32_t *)(UART2_BASE_ADDR + 0x20))
#define rURXH2				(*(volatile u_int32_t *)(UART2_BASE_ADDR + 0x24))

#define rULCON3				(*(volatile u_int32_t *)(UART3_BASE_ADDR + 0x00))
#define rUCON3				(*(volatile u_int32_t *)(UART3_BASE_ADDR + 0x04))
#define rUFCON3				(*(volatile u_int32_t *)(UART3_BASE_ADDR + 0x08))
#define rUMCON3				(*(volatile u_int32_t *)(UART3_BASE_ADDR + 0x0C))
#define rUTRSTAT3			(*(volatile u_int32_t *)(UART3_BASE_ADDR + 0x10))
#define rUERSTAT3			(*(volatile u_int32_t *)(UART3_BASE_ADDR + 0x14))
#define rUFSTAT3			(*(volatile u_int32_t *)(UART3_BASE_ADDR + 0x18))
#define rUMSTAT3			(*(volatile u_int32_t *)(UART3_BASE_ADDR + 0x1C))
#define rUBRDIV3			(*(volatile u_int32_t *)(UART3_BASE_ADDR + 0x28))
#define rUABRCNT3			(*(volatile u_int32_t *)(UART3_BASE_ADDR + 0x2C))
#define rUTXH3				(*(volatile u_int32_t *)(UART3_BASE_ADDR + 0x20))
#define rURXH3				(*(volatile u_int32_t *)(UART3_BASE_ADDR + 0x24))

#define rULCON4				(*(volatile u_int32_t *)(UART4_BASE_ADDR + 0x00))
#define rUCON4				(*(volatile u_int32_t *)(UART4_BASE_ADDR + 0x04))
#define rUFCON4				(*(volatile u_int32_t *)(UART4_BASE_ADDR + 0x08))
#define rUMCON4				(*(volatile u_int32_t *)(UART4_BASE_ADDR + 0x0C))
#define rUTRSTAT4			(*(volatile u_int32_t *)(UART4_BASE_ADDR + 0x10))
#define rUERSTAT4			(*(volatile u_int32_t *)(UART4_BASE_ADDR + 0x14))
#define rUFSTAT4			(*(volatile u_int32_t *)(UART4_BASE_ADDR + 0x18))
#define rUMSTAT4			(*(volatile u_int32_t *)(UART4_BASE_ADDR + 0x1C))
#define rUBRDIV4			(*(volatile u_int32_t *)(UART4_BASE_ADDR + 0x28))
#define rUABRCNT4			(*(volatile u_int32_t *)(UART4_BASE_ADDR + 0x2C))
#define rUTXH4				(*(volatile u_int32_t *)(UART4_BASE_ADDR + 0x20))
#define rURXH4				(*(volatile u_int32_t *)(UART4_BASE_ADDR + 0x24))

#define rULCON5				(*(volatile u_int32_t *)(UART5_BASE_ADDR + 0x00))
#define rUCON5				(*(volatile u_int32_t *)(UART5_BASE_ADDR + 0x04))
#define rUFCON5				(*(volatile u_int32_t *)(UART5_BASE_ADDR + 0x08))
#define rUMCON5				(*(volatile u_int32_t *)(UART5_BASE_ADDR + 0x0C))
#define rUTRSTAT5			(*(volatile u_int32_t *)(UART5_BASE_ADDR + 0x10))
#define rUERSTAT5			(*(volatile u_int32_t *)(UART5_BASE_ADDR + 0x14))
#define rUFSTAT5			(*(volatile u_int32_t *)(UART5_BASE_ADDR + 0x18))
#define rUMSTAT5			(*(volatile u_int32_t *)(UART5_BASE_ADDR + 0x1C))
#define rUBRDIV5			(*(volatile u_int32_t *)(UART5_BASE_ADDR + 0x28))
#define rUABRCNT5			(*(volatile u_int32_t *)(UART5_BASE_ADDR + 0x2C))
#define rUTXH5				(*(volatile u_int32_t *)(UART5_BASE_ADDR + 0x20))
#define rURXH5				(*(volatile u_int32_t *)(UART5_BASE_ADDR + 0x24))

#define rULCON6				(*(volatile u_int32_t *)(UART6_BASE_ADDR + 0x00))
#define rUCON6				(*(volatile u_int32_t *)(UART6_BASE_ADDR + 0x04))
#define rUFCON6				(*(volatile u_int32_t *)(UART6_BASE_ADDR + 0x08))
#define rUMCON6				(*(volatile u_int32_t *)(UART6_BASE_ADDR + 0x0C))
#define rUTRSTAT6			(*(volatile u_int32_t *)(UART6_BASE_ADDR + 0x10))
#define rUERSTAT6			(*(volatile u_int32_t *)(UART6_BASE_ADDR + 0x14))
#define rUFSTAT6			(*(volatile u_int32_t *)(UART6_BASE_ADDR + 0x18))
#define rUMSTAT6			(*(volatile u_int32_t *)(UART6_BASE_ADDR + 0x1C))
#define rUBRDIV6			(*(volatile u_int32_t *)(UART6_BASE_ADDR + 0x28))
#define rUABRCNT6			(*(volatile u_int32_t *)(UART6_BASE_ADDR + 0x2C))
#define rUTXH6				(*(volatile u_int32_t *)(UART6_BASE_ADDR + 0x20))
#define rURXH6				(*(volatile u_int32_t *)(UART6_BASE_ADDR + 0x24))

#define rULCON7				(*(volatile u_int32_t *)(UART7_BASE_ADDR + 0x00))
#define rUCON7				(*(volatile u_int32_t *)(UART7_BASE_ADDR + 0x04))
#define rUFCON7				(*(volatile u_int32_t *)(UART7_BASE_ADDR + 0x08))
#define rUMCON7				(*(volatile u_int32_t *)(UART7_BASE_ADDR + 0x0C))
#define rUTRSTAT7			(*(volatile u_int32_t *)(UART7_BASE_ADDR + 0x10))
#define rUERSTAT7			(*(volatile u_int32_t *)(UART7_BASE_ADDR + 0x14))
#define rUFSTAT7			(*(volatile u_int32_t *)(UART7_BASE_ADDR + 0x18))
#define rUMSTAT7			(*(volatile u_int32_t *)(UART7_BASE_ADDR + 0x1C))
#define rUBRDIV7			(*(volatile u_int32_t *)(UART7_BASE_ADDR + 0x28))
#define rUABRCNT7			(*(volatile u_int32_t *)(UART7_BASE_ADDR + 0x2C))
#define rUTXH7				(*(volatile u_int32_t *)(UART7_BASE_ADDR + 0x20))
#define rURXH7				(*(volatile u_int32_t *)(UART7_BASE_ADDR + 0x24))

#define rULCON8				(*(volatile u_int32_t *)(UART8_BASE_ADDR + 0x00))
#define rUCON8				(*(volatile u_int32_t *)(UART8_BASE_ADDR + 0x04))
#define rUFCON8				(*(volatile u_int32_t *)(UART8_BASE_ADDR + 0x08))
#define rUMCON8				(*(volatile u_int32_t *)(UART8_BASE_ADDR + 0x0C))
#define rUTRSTAT8			(*(volatile u_int32_t *)(UART8_BASE_ADDR + 0x10))
#define rUERSTAT8			(*(volatile u_int32_t *)(UART8_BASE_ADDR + 0x14))
#define rUFSTAT8			(*(volatile u_int32_t *)(UART8_BASE_ADDR + 0x18))
#define rUMSTAT8			(*(volatile u_int32_t *)(UART8_BASE_ADDR + 0x1C))
#define rUBRDIV8			(*(volatile u_int32_t *)(UART8_BASE_ADDR + 0x28))
#define rUABRCNT8			(*(volatile u_int32_t *)(UART8_BASE_ADDR + 0x2C))
#define rUTXH8				(*(volatile u_int32_t *)(UART8_BASE_ADDR + 0x20))
#define rURXH8				(*(volatile u_int32_t *)(UART8_BASE_ADDR + 0x24))

#endif /* __SAMSUNG_UART_H */
