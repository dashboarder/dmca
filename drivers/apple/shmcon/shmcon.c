/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <arch/arm/arm.h>
#include <drivers/power.h>
#include <drivers/shmcon.h>
#if WITH_CONSISTENT_DBG
#include <drivers/consistent_debug.h>
#endif
#include <lib/libc.h>
#include <platform.h>
#include <platform/memmap.h>
#include <sys.h>
#include <sys/task.h>

#ifndef COHERENT_JTAG
/* Default to coherent, should be true for most AP's */
#define COHERENT_JTAG 1
#endif

#ifndef SHARED_CONSOLE_BASE
#define SHARED_CONSOLE_BASE	PANIC_BASE
#define SHARED_CONSOLE_SIZE	PANIC_SIZE
#endif

#ifndef SHMCON_NAME
#define SHMCON_NAME		"AP-iBoot"
#endif

#define SHMCON_MAGIC 		'SHMC'
#define SHMCON_VERSION 		2
#define CBUF_IN  		0
#define CBUF_OUT 		1
#define INBUF_SIZE 		(SHARED_CONSOLE_SIZE / 16)
#define FULL_ALIGNMENT		(64)

#define FLAG_CACHELINE_32	1
#define FLAG_CACHELINE_64	2

/* Defines to clarify the master/slave fields' use as circular buffer pointers */
#define head_in		sidx[CBUF_IN]
#define tail_in		midx[CBUF_IN]
#define head_out	midx[CBUF_OUT]
#define tail_out	sidx[CBUF_OUT]

/* TODO: get from device tree/target */
#define NUM_CHILDREN		5

#define shmcon_barrier()	arm_memory_barrier()

#define WRAP_INCR(l, x) do{ (x)++; if((x) >= (l)) (x) = 0; } while(0)

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

struct shm_buffer_info {
	uint64_t 	base;
	uint32_t	unused;
	uint32_t	magic;
};

struct shmcon_header {
	uint32_t	magic;
	uint8_t		version;
	uint8_t		children;	/* number of child entries in child_ent */
	uint16_t	flags;
	uint64_t	buf_paddr[2];	/* Physical address for buffers (in, out) */
	uint32_t	buf_len[2];
	uint8_t		name[8];

	/* Slave-modified data - invalidate before read */
	uint32_t	sidx[2] __attribute__((aligned (FULL_ALIGNMENT)));	/* In head, out tail */

	/* Master-modified data - clean after write */
	uint32_t	midx[2] __attribute__((aligned (FULL_ALIGNMENT)));	/* In tail, out head */ 

	uint64_t	child[0];	/* Physical address of child header pointers */
};

static volatile struct shmcon_header *shmcon = NULL;
static uint8_t *shmbuf[2];

/* 
  * WARNING: This function invalidates all cache lines spanned
  * by the range provided.  It should only be used on memory
  * that can safely be invalidated up to cache line rounding size.
  */
static void cache_line_invalidate(void *addr, size_t size)
{
	uintptr_t address = (uintptr_t)addr;
	size_t shift;

	shift = address & ((1 << CPU_CACHELINE_SHIFT)-1);
	address -= shift;
	size += shift;
	platform_cache_operation(CACHE_INVALIDATE, (void *)address, ROUNDUP(size, CPU_CACHELINE_SIZE));
}

int shmcon_getc(int port, bool wait)
{
	int c;
	uint32_t head, tail, len;

	if (shmcon == NULL)
		return -1;

	tail = shmcon->tail_in;
	len = shmcon->buf_len[CBUF_IN];

#if !COHERENT_JTAG
	cache_line_invalidate((void *)&shmcon->head_in, sizeof(shmcon->head_in));
#endif
	head = shmcon->head_in;

	if (head == tail) {
		if (!wait)
			return -1;
		do {
			task_yield();
#if !COHERENT_JTAG
			cache_line_invalidate((void *)&shmcon->head_in, sizeof(shmcon->head_in));
#endif
			head = shmcon->head_in;
		} while (head == tail);
	}
#if !COHERENT_JTAG
	cache_line_invalidate(&shmbuf[CBUF_IN][tail], CPU_CACHELINE_SIZE);
#endif
	c = shmbuf[CBUF_IN][tail];
	shmcon_barrier();
	WRAP_INCR(len, tail);
	shmcon->tail_in = tail;
#if !COHERENT_JTAG
	platform_cache_operation(CACHE_CLEAN, (void *)&shmcon->tail_in, ROUNDUP(sizeof(shmcon->tail_in), CPU_CACHELINE_SIZE));
#endif
	return c;

}

int shmcon_putc(int port, char c)
{
	uint32_t head, len;

	if (shmcon == NULL)
		return -1;

	head = shmcon->head_out;
	len = shmcon->buf_len[CBUF_OUT];

	shmbuf[CBUF_OUT][head] = c;
	shmcon_barrier();
#if !COHERENT_JTAG
	platform_cache_operation(CACHE_CLEAN, &shmbuf[CBUF_OUT][head], CPU_CACHELINE_SIZE);
#endif
	WRAP_INCR(len, head);
	shmcon->head_out = head;
#if !COHERENT_JTAG
	platform_cache_operation(CACHE_CLEAN, (void *)&shmcon->head_out, ROUNDUP(sizeof(shmcon->head_out), CPU_CACHELINE_SIZE));
#endif
	return 0;
}

int shmcon_puts(int port, const char *s)
{
	for ( ; *s; s++)
		if (shmcon_putc(port, *s))
			break;
	return 0;
}

static int shmcon_reader_task(void *arg)
{
	for ( ; ; ) {
		int c;

		c = shmcon_getc(0, true);
		if (((DebugUartReady & kPowerNVRAMiBootDebugJtag) != 0) && (c >= 0)) {
			debug_pushchar((char)c);
		}
	}
	return 0;
}

int shmcon_init(void)
{
	volatile struct shm_buffer_info	*end;
	size_t				i, header_size;
	uintptr_t			buffer_base, buffer_end;
	uint32_t			num_children = NUM_CHILDREN;

	if (shmcon != NULL)
		return 0;

	shmcon = (struct shmcon_header *)SHARED_CONSOLE_BASE;
	header_size = sizeof(*shmcon) + (num_children * sizeof(shmcon->child[0]));
	buffer_base = ROUNDUP((uintptr_t)(shmcon) + header_size, CPU_CACHELINE_SIZE);
	buffer_end  = SHARED_CONSOLE_BASE + SHARED_CONSOLE_SIZE - (sizeof(*end));

	shmcon->magic = 0;
	shmcon_barrier();
	platform_cache_operation(CACHE_CLEAN, (void *)&shmcon->magic,
		ROUNDUP(sizeof(shmcon->magic), CPU_CACHELINE_SIZE));
	shmcon->buf_len[CBUF_IN] = INBUF_SIZE;
	shmcon->buf_paddr[CBUF_IN]  = buffer_base;
	shmcon->buf_paddr[CBUF_OUT] = ROUNDUP(buffer_base + INBUF_SIZE, CPU_CACHELINE_SIZE);
	for (i = 0; i < 2; i++) {
		shmcon->midx[i] = 0;
		shmcon->sidx[i] = 0;
		shmbuf[i] = (uint8_t *)(uintptr_t)shmcon->buf_paddr[i];
	}
	shmcon->buf_len[CBUF_OUT] = SHARED_CONSOLE_SIZE - ((uintptr_t)shmbuf[CBUF_OUT] - SHARED_CONSOLE_BASE)
		- (sizeof(struct shm_buffer_info));
	memset((void *)shmcon->name, ' ', sizeof(shmcon->name));
	memcpy((void *)shmcon->name, SHMCON_NAME, MIN(sizeof(shmcon->name), strlen(SHMCON_NAME)));
	cache_line_invalidate(shmbuf[CBUF_IN], INBUF_SIZE);
	shmcon->version = SHMCON_VERSION;
	shmcon->children = num_children;
	for (i = 0; i < num_children; i++)
		shmcon->child[i] = 0;
#if COHERENT_JTAG
	shmcon->flags = 0;
#else
	shmcon->flags = (CPU_CACHELINE_SIZE == 32) ? (FLAG_CACHELINE_32) : (FLAG_CACHELINE_64);
#endif
	shmcon_barrier();
	shmcon->magic = SHMCON_MAGIC;
	platform_cache_operation(CACHE_CLEAN, (void *)shmcon, ROUNDUP(header_size, CPU_CACHELINE_SIZE));
	end = (volatile struct shm_buffer_info *)buffer_end;
	end->base = SHARED_CONSOLE_BASE;
	end->unused = 0;
	shmcon_barrier();
	end->magic = SHMCON_MAGIC;
	platform_cache_operation(CACHE_CLEAN, (void *)end, ROUNDUP(sizeof(struct shm_buffer_info), CPU_CACHELINE_SIZE));

#if DEBUG_UART_ENABLE_DEFAULT
	DebugUartReady |= kPowerNVRAMiBootDebugJtag;
#endif	

	task_start(task_create("shmcon reader", shmcon_reader_task, (void *)NULL, 0x200));

#ifdef WITH_CONSISTENT_DBG
	dbg_record_header_t hdr;
	hdr.length = SHARED_CONSOLE_SIZE;
	hdr.physaddr = SHARED_CONSOLE_BASE;
	hdr.record_id = kDbgIdConsoleHeaderAP;
	dbg_record_header_t *dbghdr = consistent_debug_register_header(hdr);
	if (!dbghdr) {
		dprintf(DEBUG_SPEW, "Unable to allocate consistent debug header for console.\n");
	}
#endif
	return 0;
}

int shmcon_set_child(uint64_t phys_address, uint32_t num)
{
	if (shmcon == NULL || num >= shmcon->children)
		return -1;

	shmcon->child[num] = phys_address;
	return 0;
}

