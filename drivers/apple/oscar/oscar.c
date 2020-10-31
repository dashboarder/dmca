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

#include <debug.h>
#include <drivers/oscar/oscar.h>
#include <drivers/power.h>
#include <drivers/uart.h>
#include <platform/memmap.h>
#include <sys.h>
#include <lib/env.h>
#include <sys/menu.h>
#include <sys/task.h>

#if defined(WITH_MENU) && WITH_MENU

static int	oscar_main(int argc, struct cmd_arg *args);
static int	oscar_validate(addr_t addres, size_t length);
static int	oscar_init(void);
static int	oscar_upload(addr_t addres, size_t length);
static void	oscar_console(void);

MENU_COMMAND_DEBUG(oscar, oscar_main, "Boot an attached Oscar coprocessor", NULL);


static int
oscar_main(int argc, struct cmd_arg *args) 
{
	addr_t addr = 0;
	size_t len = 0;

	addr = env_get_uint("loadaddr", (uintptr_t)DEFAULT_LOAD_ADDRESS);
	len = env_get_uint("filesize", 0);

	if (len == 0) {
		printf("must upload firmware first\n");
		return -1;
	}
	if (oscar_validate(addr, len) < 0) {
		return -1;
	}

	/* XXX sanity-check firmware header image bytes */

	if (oscar_init() != 0)
		return -1;

	if (oscar_upload(addr, len) != 0)
		return -1;

	oscar_console();

	return 0;
}

static int
oscar_validate(addr_t address, size_t length)
{
	/* image must be at least header + vector table long */
	if (length <= (16 + 64)) {
		printf("firmware too small\n");
		return -1;
	}

	/* check magic numbers */
	if ((*(uint8_t *)address & 0x3f) != 0x1a) {
		printf("bad firmware magic number\n");
		return -1;
	}

	unsigned crc_len = *(uint32_t *)(address + 4) * 4;
	if ((crc_len + 16) > length) {
		printf("firmware truncated (header says %d, received %zu)\n", crc_len + 128, length);
		return -1;
	}
	return 0;
}

static int
oscar_init(void)
{
	unsigned limit;

	/* DEBUG_EN should be high already */

	/* assert oscar reset */
	power_set_gpio(POWER_GPIO_OSCAR_RESET, 1 /* output */, 0 /* reset active low */);

	/* wait 100us */
	spin(100);

	/* configure serial port for upload */
	uart_hw_init_extended(OSCAR_SERIAL_PORT, OSCAR_ROM_BAUDRATE, 8, PARITY_NONE, 1);

	/* drain FIFO */
	for (limit = 0; limit < 1000; limit++)
		if (uart_getc(OSCAR_SERIAL_PORT, false) < 0)
			break;

	/* power should be on already */

	/* release oscar reset */
	power_set_gpio(POWER_GPIO_OSCAR_RESET, 1 /* output */, 1 /* reset inactive high */);

	/* spin sending 0x80 and waiting for 0x3f */
	for (limit = 0; limit < 100000; limit++) {

		/* this will stall once the FIFO is full, so we will be paced by the port */
		uart_putc(OSCAR_SERIAL_PORT, 0x80);

		/* look for a character */
		int c = uart_getc(OSCAR_SERIAL_PORT, false);
		if (c == 0x3f)
			return 0;
		if (c != -1)
			printf("oscar: ignoring 0x%02x\n", c);
	}
	printf("failed to get handshake from Oscar ROM\n");
	return -1;
}

static int
oscar_check(void)
{
	/*
	 * ROM sends Fx<cr><lf> if it's unhappy, where
	 * x is normally 0..5.
	 */
	int c = uart_getc(OSCAR_SERIAL_PORT, false);

	/* no response, but keep waiting */
	if (c == -1)
		return 0;

	switch (c) {
	case -1:
		/* no data */
	case 'O':
	case 'K':
	case '\r':
		/* the ROM sends these after a successful upload */
	default:
		return 0;	/* keep waiting */

	case 'X':
		/* app has started */
		return 1;

	case 'F':
		/* ROM error message */
		break;
	}

	/* decode the ROM error */
	int reason = -1;
	for (unsigned limit = 0; limit < 5000; limit++) {
		reason = uart_getc(OSCAR_SERIAL_PORT, false);
		if (reason != -1)
			break;
		task_yield();
	}
	printf("upload failed: ");
	switch (reason) {
	case '0':
		printf("invalid magic number\n");
		break;
	case '1':
		printf("invalid image size\n");
		break;
	case '2':
		printf("invalid header CRC\n");
		break;
	case '3':
		printf("invalid image CRC\n");
		break;
	case '4':
		printf("invalid image base address\n");
		break;
	case -1:
		printf("error code not received\n");
		break;
	default:
		/* not a recognised error, must be application data */
		return 1;
	}
	return -1;
}

static int 
oscar_upload(addr_t address, size_t length)
{
	const uint8_t *p = (const uint8_t *)address;

	printf("sending %zu bytes...\n", length);

	/* upload */
	while (length--) {

		uart_putc(OSCAR_SERIAL_PORT, *p++);

		if (oscar_check() < 0)
			return -1;
		task_yield();

	}

	/* wait a little while for a late error, and to let the firmware start */
	for (unsigned i = 0; i < 1000000; i++) {
		switch (oscar_check()) {
		case 0:
			break;
		case 1:
			goto done;
		default:
		case -1:
			return -1;
		}
		task_yield();
	}

	printf("timed out waiting for app handshake\n");
	return -1;

done:
	/* enable clk32 - cannot be done safely until firmware on Oscar2 disables the reset function */
	power_set_gpio(POWER_GPIO_OSCAR_RESET, 2 /* 32kHz */, 0);

	return 0;
}

static void
oscar_console(void)
{
	/* configure serial port for console */
	uart_hw_init_extended(OSCAR_SERIAL_PORT, OSCAR_CONSOLE_BAUDRATE, 8, PARITY_NONE, 1);

	printf("\nOscar console log - hit any key to exit.\n\n");

	for (;;) {
		task_yield();
		if (debug_getchar_nowait() != -1) {
			printf("\n\n");
			return;
		}
		int c = uart_getc(OSCAR_SERIAL_PORT, false);
		if (c != -1)
			putchar(c);
	}
}


#endif // defined(WITH_MENU) && WITH_MENU
