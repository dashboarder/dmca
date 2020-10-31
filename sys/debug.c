/*
 * Copyright (C) 2007-2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <arch.h>
#include <sys.h>
#include <sys/task.h>
#include <debug.h>
#include <lib/cbuf.h>
#include <drivers/power.h>

#if WITH_HW_UART
# include <drivers/uart.h>
# if WITH_TARGET_CONFIG
#  include <target/uartconfig.h>
# elif WITH_PLATFORM_UARTCONFIG
#  include <platform/uartconfig.h>
# endif
#endif

#if APPLICATION_SECUREROM
# define DEBUG_QUEUE_LEN 256
#else
# define DEBUG_QUEUE_LEN 4096
#endif
static struct task_event debug_event;
static struct cbuf *debug_cbuf;

#if WITH_USB_MODE_RECOVERY

// Use USB serial debug, unless there's a dockfifo, because spew from that
// driver will deadlock us.
#if !defined(USE_USB_DEBUG) && !defined(WITH_HW_DOCKFIFO_BULK)
#define USE_USB_DEBUG 1
#endif

#include <drivers/usb/usb_public.h>

#endif // WITH_USB_MODE_RECOVERY

#if WITH_TBT_MODE_RECOVERY

#ifndef USE_TBT_DEBUG
#define USE_TBT_DEBUG 1
#endif

#include <drivers/thunderbolt/thunderboot.h>

#endif // WITH_TBT_MODE_RECOVERY

#if WITH_ARM_DCC
# include <arch/arm/arm.h>
#endif

#if WITH_SHM_CONSOLE
# include <drivers/shmcon.h>
#endif

#if WITH_HW_DOCKFIFO_UART
# include <drivers/dockfifo/dockfifo.h>
#endif

#if WITH_HW_DOCKCHANNEL_UART
# include <drivers/dockchannel/dockchannel.h>
#endif

int debug_getchar(void)
{
	ssize_t ret;
	char c;

	ASSERT(NULL != debug_cbuf);

	event_wait(&debug_event);
	ret = cbuf_read_char(debug_cbuf, &c);
	ASSERT(ret >= 0);

	return c;
}

int debug_getchar_nowait(void)
{
	ssize_t ret;
	char c;

	ASSERT(NULL != debug_cbuf);

	ret = cbuf_read_char(debug_cbuf, &c);
	if (ret < 1) {
		return -1;
	}

	return c;
}

int debug_pushchar(int c)
{
	ASSERT(NULL != debug_cbuf);
	return cbuf_write_char(debug_cbuf, c);
}

void debug_init(void)
{
	debug_cbuf = cbuf_create(DEBUG_QUEUE_LEN, &debug_event);
}


int DebugUartReady = 0;

void debug_enable_uarts(int debug_uarts)
{
    DebugUartReady |= debug_uarts;
}

void debug_putchar(int c)
{
#if USE_USB_DEBUG
    usb_serial_putchar(c);
#endif

#if USE_TBT_DEBUG
    thunderboot_putchar(c);
#endif
    
#if defined(DEBUG_SERIAL_PORT)
    if ((DebugUartReady & kPowerNVRAMiBootDebugIAPSerial) != 0) {
      if (c == '\n')
        uart_putc(DEBUG_SERIAL_PORT, '\r');
      uart_putc(DEBUG_SERIAL_PORT, c);
    }
#endif
    
#if defined(DEBUG_SERIAL_PORT2)
    if ((DebugUartReady & kPowerNVRAMiBootDebugAltSerial) != 0) {
      if (c == '\n')
        uart_putc(DEBUG_SERIAL_PORT2, '\r');
      uart_putc(DEBUG_SERIAL_PORT2, c);
    }
#endif

#if WITH_ARM_DCC
    // always output dcc
    arm_write_dcc_char(c);
#endif

#if WITH_SHM_CONSOLE
    if ((DebugUartReady & kPowerNVRAMiBootDebugJtag) != 0) {
      if (c == '\n')
        shmcon_putc(0, '\r');
      shmcon_putc(0, c);
    }
#endif

#if WITH_HW_DOCKFIFO_UART
    if ((DebugUartReady & kPowerNVRAMiBootDebugIAPSerial) != 0) {
        if (c == '\n')
            dockfifo_uart_putc('\r');
        dockfifo_uart_putc(c);
    }
#endif

#if WITH_HW_DOCKCHANNEL_UART
    if ((DebugUartReady & kPowerNVRAMiBootDebugIAPSerial) != 0) {
        if (c == '\n')
            dockchannel_uart_putc('\r');
        dockchannel_uart_putc(c);
    }
#endif

#if WITH_APPLICATION_PUTCHAR
    application_putchar(c);
#endif
    
}

int debug_run_script(const char *addr)
{
	char lastchar;

//	printf("running script at %p\n", addr);

	/* start stuffing data into the input queue at the specified address until we hit an EOF char */
	lastchar = 0;
	while ((*addr != 0x04) && (*addr != 0)) {
		debug_pushchar(*addr);
		lastchar = *addr;
		addr++;
	}
	if (lastchar != '\n')
		debug_pushchar('\n');

//	printf("finished running script (EOF at %p)\n", addr);
	return 0;
}

