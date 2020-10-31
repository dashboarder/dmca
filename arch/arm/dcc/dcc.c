/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch.h>
#include <debug.h>
#include <sys.h>
#include <sys/task.h>
#include <lib/cbuf.h>

#define DSCR_WDTR_FULL  (1L<<29)
#define DSCR_RDTR_FULL  (1L<<30)

#if ARM_DCC_SYNCHRONOUS
static bool dcc_wait_tx = true;
#else
static bool dcc_wait_tx;
#endif

#ifndef DCC_TX_BUFFER_SIZE
# define DCC_TX_BUFFER_SIZE 256
#endif

static struct cbuf *dcc_output_cbuf;

extern u_int32_t read_dtr(void);
extern void write_dtr(u_int32_t c);
extern u_int32_t read_dscr(void);

void arm_write_dcc_char(int c)
{
	if (dcc_wait_tx) {
		while (read_dscr() & DSCR_WDTR_FULL);
		write_dtr(c);
	} else {
		if (dcc_output_cbuf != 0)
		   	cbuf_write_char(dcc_output_cbuf, c);
	}
}

int arm_read_dcc_char(void)
{
	while (!(read_dscr() & DSCR_RDTR_FULL)); 

	return read_dtr(); 
}

int arm_read_dcc_char_nowait(void)
{
	if (!(read_dscr() & DSCR_RDTR_FULL))
		return -1;

	return read_dtr();
}

static int dcc_read_task(void *arg)
{
	for(;;) {
		int c;

		c = arm_read_dcc_char_nowait();
		if (c >= 0) {
			if (c != '\r')
				debug_pushchar(c);
		}
		task_sleep(1 * 1000);
	}
	return 0;
}

static int dcc_write_task(void *arg)
{
	for(;;) {
		ssize_t ret;
		char c;

		ret = cbuf_read_char(dcc_output_cbuf, &c);
		if (ret == 1) {
   			while(read_dscr() & DSCR_WDTR_FULL)
				 task_yield();
		    write_dtr(c);
		}
		task_sleep(1 * 1000);
	}
	return 0;
}

int arm_dcc_init(void)
{
	if (arm_read_dcc_char_nowait() >= 0)
	   	dcc_wait_tx = true;

	if (!dcc_wait_tx) {
		dcc_output_cbuf = cbuf_create(DCC_TX_BUFFER_SIZE, NULL);
		task_start(task_create("dcc write", &dcc_write_task, NULL, 256));
	}

	task_start(task_create("dcc read", &dcc_read_task, NULL, 512));

	return 0;
}
