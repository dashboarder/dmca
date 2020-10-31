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
#include <sys/task.h>
#include <sys.h>

#include <drivers/power.h>
#include <drivers/uart.h>

#include <target/uartconfig.h>

/*
 * Hardware-independent UART support.
 */

#define	UART_BAUDRATE	115200

static int uart_reader_task(void *arg);

/*
 * uart_init
 *
 * Initalise UARTs in the system.
 *
 * Starts the debug input polling threads as well.
 */
int
uart_init(void)
{
	u_int32_t	port;

	/* do base hardware init */
	for (port = 0; port < UARTS_COUNT; port++)
		uart_hw_init(port, UART_BAUDRATE);

#ifdef DEBUG_SERIAL_PORT
	/* regular console port */
	task_start(task_create("uart reader", uart_reader_task, (void *)DEBUG_SERIAL_PORT, 0x200));
#ifdef DEBUG_SERIAL_PORT2
	/* alternate console port */
	task_start(task_create("uart reader", uart_reader_task, (void *)DEBUG_SERIAL_PORT2, 0x200));
#endif
#endif

#if DEBUG_UART_ENABLE_DEFAULT
	/* enable console ports early */
	DebugUartReady |= kPowerNVRAMiBootDebugIAPSerial | kPowerNVRAMiBootDebugAltSerial;
#endif

	return(0);
}

/*
 * uart_getc
 *
 * Read a byte from the port, optionally waiting.
 * If not waiting, returns -1 if a byte is not available.
 */
int
uart_getc(int port, bool wait)
{
	int c;
	
	for (;;) {
		/* grab a byte */
		c = uart_hw_getc(port);

		/* if we got one, or we're not waiting, return */
		if ((-1 != c) || !wait)
			return(c);

		/* give up cycles */
		task_yield();
	}
}

/*
 * uart_puts
 *
 * Write a nul-terminated string to the port.
 */
int
uart_puts(int port, const char *s)
{
	while(*s)
		uart_putc(port, *(s++));
	return(0);
}

/*
 * uart_putc
 *
 * Write a byte to the port.
 */
int
uart_putc(int port, char c)
{
	/* spin until the byte can be sent */
	while (-1 == uart_hw_putc(port, c))
		task_yield();
	return(0);
}

/*
 * uart_debug_reader_task
 *
 * This helper task polls the configured serial port and
 * feeds bytes read from it into the console input queue.
 */
static int
uart_reader_task(void *arg)
{
	int		port = (int)arg;
	u_int8_t	port_mask;
	int		c;

	/* work out what the enable mask is for this port */
	switch (port) {
#if defined(DEBUG_SERIAL_PORT)
	case DEBUG_SERIAL_PORT:
		port_mask = kPowerNVRAMiBootDebugIAPSerial;
		break;
#if defined(DEBUG_SERIAL_PORT2)
	case DEBUG_SERIAL_PORT2:
		port_mask = kPowerNVRAMiBootDebugAltSerial;
		break;
#endif	
#endif
	default:
		ASSERT(false);
		return(0);
	}

	/* loop reading bytes */
	for(;;) {
		/* ask the port for a byte, waiting if required */
		c = uart_getc(port, true);

		/* if our port is enabled, feed it to the input queue */
		if (DebugUartReady & port_mask)
			debug_pushchar(c);

		/* yield to avoid hogging */
		task_yield();
	}
	return(0);
}

