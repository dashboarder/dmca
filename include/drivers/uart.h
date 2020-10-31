/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __DRIVERS_UART_H
#define __DRIVERS_UART_H

#include <sys/types.h>

__BEGIN_DECLS

/*
 * High-level UART interface.
 */
int uart_init(void);

int uart_getc(int port, bool wait);
int uart_puts(int port, const char *s);
int uart_putc(int port, char c);

int uart_send_break(int port, bool enable);

/*
 * Low-level UART primitives required by the drivers/uart module.
 */
enum uart_parity {
	PARITY_NONE,
	PARITY_ODD,
	PARITY_EVEN
};
int uart_hw_init(u_int32_t port, u_int32_t baudrate);
int uart_hw_getc(u_int32_t port);
int uart_hw_putc(u_int32_t port, char c);

/* optional, backdoor for tweaking UART config */
int uart_hw_init_extended(u_int32_t port, u_int32_t baudrate, u_int32_t databits,
			  enum uart_parity parity, u_int32_t stopbits);
/* set interrupt mode on/off, explicit buffer size */
int uart_hw_set_rx_buf(u_int32_t port, bool interrupt, size_t buffer_len);

__END_DECLS

#endif /* __DRIVERS_UART_H */
