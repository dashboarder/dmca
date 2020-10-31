/*
 * Copyright (C) 2007-2009 Apple Inc. All rights reserved.
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
#include <platform.h>
#include <platform/clocks.h>
#include <platform/int.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <sys.h>
#include <sys/callout.h>

#include "timer.h"

/* 16-bit timers */
#define TIMER_A		0
#define TIMER_B		1
#define TIMER_C		2
#define TIMER_D		3

/* 32-bit timers */
#define TIMER_E		4
#define TIMER_F		5
#define TIMER_G		6
#define TIMER_H		7

/* 64-bit timer */
#define TIMER_64	8

#define	INVALID_TIMER	-1

#ifndef TIMER_DEADLINE_TIMER
# if TIMER_DEADLINE_USE16
#  define TIMER_DEADLINE_TIMER	TIMER_D
# else
#  define TIMER_DEADLINE_TIMER	TIMER_E
# endif
#endif

/* timer16/32 stuff */
typedef struct {
	volatile u_int32_t *tcon;
	volatile u_int32_t *tcmd;
	volatile u_int32_t *tdata0;
	volatile u_int32_t *tdata1;
	volatile u_int32_t *tpre;
	volatile u_int32_t *tcnt;
} timer_regs_t;

static const timer_regs_t timer_regs_list[] = {
	{ &rTACON, &rTACMD, &rTADATA0, &rTADATA1, &rTAPRE, &rTACNT },
	{ &rTBCON, &rTBCMD, &rTBDATA0, &rTBDATA1, &rTBPRE, &rTBCNT },
	{ &rTCCON, &rTCCMD, &rTCDATA0, &rTCDATA1, &rTCPRE, &rTCCNT },
	{ &rTDCON, &rTDCMD, &rTDDATA0, &rTDDATA1, &rTDPRE, &rTDCNT },
	{ &rTECON, &rTECMD, &rTEDATA0, &rTEDATA1, &rTEPRE, &rTECNT },
	{ &rTFCON, &rTFCMD, &rTFDATA0, &rTFDATA1, &rTFPRE, &rTFCNT },
	{ &rTGCON, &rTGCMD, &rTGDATA0, &rTGDATA1, &rTGPRE, &rTGCNT },
	{ &rTHCON, &rTHCMD, &rTHDATA0, &rTHDATA1, &rTHPRE, &rTHCNT }
};
static const timer_regs_t *timer_regs(int timer);

enum timer_clk {
	PCLK,
	NCLK,
	SLOWCLK,
};

typedef struct {
	/* configuration */
	enum timer_clk source_clk;
	uint32_t divider_bits;
	int prescalar;

	/* callout routine */
	void (*ovf_tick)(void);
	void (*mat0_tick)(void);
	void (*mat1_tick)(void);
} timer_status_t;

static timer_status_t timer_status[TIMER_H+1];

enum timer_mode {
	MODE_INTERVAL = 0,
	MODE_PWM = 0x2,
	MODE_PDM = 0x3,
	MODE_ONESHOT_PWM = 0x4,
	MODE_ONESHOT_PDM = 0x5,
	MODE_CAPTURE = 0x6,
};

static void setup_timer(int timer, uint32_t data0, uint32_t data1, enum timer_mode mode, bool countdown, bool countdown_start, bool start_polarity);
static void setup_timer_clk(int timer, enum timer_clk clk, int divider, int prescalar);
static void start_stop_timer(int timer, bool start);

static void timer_deadline(void);
static void (* timer_deadline_func)(void);

/* timer 64 stuff */
static u_int64_t read_timer64(void);

static int timer64_ticks_per_sec; // for system time
static uint64_t timer64_scale;

static void timer32_int(void *);
static void timer16_int(void *);

static u_int64_t read_timer64(void)
{
	u_int32_t low, high;

retry:
	high = rTM64_CNTH;
	low = rTM64_CNTL;
	if (high != rTM64_CNTH)
		goto retry;

	return (u_int64_t)high << 32 | low;
}

static u_int32_t read_timer(int timer)
{
	const timer_regs_t *regs = timer_regs(timer);

	return *regs->tcnt;
}

static const timer_regs_t *timer_regs(int timer)
{
	if (timer == TIMER_64)
		panic("timer_regs: asked for invalid timer registers\n");

	return &timer_regs_list[timer];
}

static void setup_timer_clk(int timer, enum timer_clk clk, int divider, int prescalar)
{
	if (clk == SLOWCLK) {
		timer_status[timer].source_clk = PCLK;
		timer_status[timer].divider_bits = 6;
	} else {
		timer_status[timer].source_clk = clk;
		switch (divider) {
			case 2:
				timer_status[timer].divider_bits = 0;
				break;
			case 4:
				timer_status[timer].divider_bits = 1;
				break;
			case 16:
				timer_status[timer].divider_bits = 2;
				break;
			case 64:
				timer_status[timer].divider_bits = 3;
				break;
			case 1:
				timer_status[timer].divider_bits = 4;
				break;
			default:
				dprintf(DEBUG_CRITICAL, "setup_timer_clk: invalid divider %d\n", divider);
		}
	}
	timer_status[timer].prescalar = prescalar;
}

static void setup_timer(int timer, uint32_t data0, uint32_t data1, enum timer_mode mode, bool countdown, bool countdown_start, bool start_polarity)
{
	start_stop_timer(timer, false);

	switch (timer) {
		case TIMER_A: case TIMER_B: case TIMER_C: case TIMER_D: {
			const timer_regs_t *regs = timer_regs(timer);

			/* setup timer (16 bit) */
			*regs->tcon = (start_polarity ? (1<<11) : 0) |
						  (mode << 3) | 
						  (timer_status[timer].divider_bits<<8) | ((timer_status[timer].source_clk == NCLK) ? (1<<6) : 0) |
						  (7<<12); // mode, upcount (no downcount feature), divider, clock source, enable all ints
			*regs->tpre = timer_status[timer].prescalar;
			*regs->tdata0 = data0;
			*regs->tdata1 = data1;
			*regs->tcmd = 2; // initialize the timer
			break;
		}
		case TIMER_E: case TIMER_F: case TIMER_G: case TIMER_H: {
			const timer_regs_t *regs = timer_regs(timer);

			/* setup timer (32 bit) */
			*regs->tcon = (countdown_start ? (1<<28) : 0) |
						  (countdown ? (1<<24) : 0) | 
						  (start_polarity ? (1<<11) : 0) | 
						  (mode << 3) | 
						  (timer_status[timer].divider_bits<<8) | ((timer_status[timer].source_clk == NCLK) ? (1<<6) : 0) |
						  (7<<12); // mode, downcount, pwm/pdm start bit, divider, clock source, enable all ints
			*regs->tpre = timer_status[timer].prescalar;
			*regs->tdata0 = data0;
			*regs->tdata1 = data1;
			*regs->tcmd = 2; // initialize the timer
			break;
		}
		default:
			panic("setup_timer: timer %d not supported\n", timer);
	}	
}

static void start_stop_timer(int timer, bool start)
{
	switch (timer) {
		case TIMER_A: case TIMER_B: case TIMER_C: case TIMER_D:
		case TIMER_E: case TIMER_F: case TIMER_G: case TIMER_H: {
			const timer_regs_t *regs = timer_regs(timer);
			if (start) 
				*regs->tcmd = 1; // start it
			else
				*regs->tcmd = 0; // stop it
			break;
		}
		case TIMER_64:
			if (start)
				rTM64_CON &= ~0xf;
			else
				rTM64_CON |= 0xa;
			break;
		default:
			panic("start_stop_timer: timer %d not supported\n", timer);
	}
}

int timer_init(u_int32_t timer)
{
	dprintf(DEBUG_SPEW, "timers_init()\n");

#ifndef TIMER_NO_INIT
	clock_gate(CLK_TIMER, true);

	/* stop all timers */
	start_stop_timer(TIMER_A, false);
	start_stop_timer(TIMER_B, false);
	start_stop_timer(TIMER_C, false);
	start_stop_timer(TIMER_D, false);
	start_stop_timer(TIMER_E, false);
	start_stop_timer(TIMER_F, false);
	start_stop_timer(TIMER_G, false);
	start_stop_timer(TIMER_H, false);
	start_stop_timer(TIMER_64, false);

	setup_timer_clk(TIMER_A, NCLK, 2, 0);
	setup_timer_clk(TIMER_B, NCLK, 2, 0);
	setup_timer_clk(TIMER_C, NCLK, 2, 0);
	setup_timer_clk(TIMER_D, NCLK, 2, 0);
	setup_timer_clk(TIMER_E, NCLK, 2, 0);
	setup_timer_clk(TIMER_F, NCLK, 2, 0);
	setup_timer_clk(TIMER_G, NCLK, 2, 0);
	setup_timer_clk(TIMER_H, NCLK, 2, 0);

	/* set up the 64-bit timer */
	rTM64_CON = 0xa; // disable it
	rTM64_DATA0L = 0xffffffff;
	rTM64_DATA0H = 0xffffffff;
	rTM64_DATA1L = 0xffffffff;
	rTM64_DATA1H = 0xffffffff;
	rTM64_CON = (3<<15) | (1 << 4); // set nclk source, prescalar at /2, enable

	/* calculate how fast timer64 is running */
	timer64_ticks_per_sec = clock_get_frequency(CLK_NCLK) / 2;
#else
# ifndef TIMER_TICK_RATE
#  error TIMER_TICK_RATE not defined but we did not init the timebase
# endif
	timer64_ticks_per_sec = TIMER_TICK_RATE;
#endif

	// Precompute a 44.20 fixed-point representation of the microsecond to ticks conversion rate.
	// This limits the maximum number of microseconds that can be represented to 44 bits worth of ticks.
	timer64_scale = ((uint64_t)timer64_ticks_per_sec << 20) / (1000 * 1000);

	dprintf(DEBUG_SPEW, "timer64 running at %u Hz, will wrap around in %llu secs\n", 
			timer64_ticks_per_sec, 0xffffffffffffffffULL / timer64_ticks_per_sec);

	dprintf(DEBUG_SPEW, "using 64-bit timer for system time\n");

	/* install interrupt handler */
#if TIMER_DEADLINE_USE16
	install_int_handler(INT_TIMERIRQ, &timer16_int, (void *)1);
	unmask_int(INT_TIMERIRQ);
#else
	install_int_handler(INT_TIMERFIQ, &timer32_int, (void *)1);
	unmask_int(INT_TIMERFIQ);
#endif
	
	return 0;
}

void timer_gpio(int timer, int level)
{
	setup_timer(timer, 0, 0, MODE_PWM, false, false, level);
}

static void timer_handle_int(int timer, u_int32_t status)
{
	if (status & (1<<2)) {
		// overflow
		if (timer_status[timer].ovf_tick)
			timer_status[timer].ovf_tick();
	}
	if (status & (1<<1)) {
		// int1
		if (timer_status[timer].mat1_tick)
			timer_status[timer].mat1_tick();
	}
	if (status & (1<<0)) {
		// int0
		if (timer_status[timer].mat0_tick)
			timer_status[timer].mat0_tick();
	} 
}

static void timer16_int(void *arg)
{
	u_int32_t tcon;

	tcon = rTACON;
	if (tcon & (7<<16)) {
		timer_handle_int(TIMER_A, tcon >> 16);
		rTACON |= tcon & (7<<16);
	}
	tcon = rTBCON;
	if (tcon & (7<<16)) {
		timer_handle_int(TIMER_B, tcon >> 16);
		rTBCON |= tcon & (7<<16);
	}
	tcon = rTCCON;
	if (tcon & (7<<16)) {
		timer_handle_int(TIMER_C, tcon >> 16);
		rTCCON |= tcon & (7<<16);
	}
	tcon = rTDCON;
	if (tcon & (7<<16)) {
		timer_handle_int(TIMER_D, tcon >> 16);
		rTDCON |= tcon & (7<<16);
	}
}

static void timer32_int(void *arg)
{
	u_int32_t tm32_status, tm32_ack, status, cnt;

	tm32_status = rTM32_INT;
	tm32_ack = 0;
	cnt = 0;

#if TIMER_VERSION != 0
	status = (tm32_status >> (cnt * 8)) & 0x7;
	timer_handle_int(TIMER_H, status);
	tm32_ack |= (status << (cnt * 8));
	cnt++;
#endif
	status = (tm32_status >> (cnt * 8)) & 0x7;
	timer_handle_int(TIMER_G, status);
	tm32_ack |= (status << (cnt * 8));
	cnt++;
	
	status = (tm32_status >> (cnt * 8)) & 0x7;
	timer_handle_int(TIMER_F, status);
	tm32_ack |= (status << (cnt * 8));
	cnt++;
	
	status = (tm32_status >> (cnt * 8)) & 0x7;
	timer_handle_int(TIMER_E, status);
	tm32_ack |= (status << (cnt * 8));
	cnt++;

	rTM32_INT = tm32_ack;
}

static void timer_deadline(void)
{

//	printf("deadline interrupt at %llu\n", read_timer64());
	/* stop the timer */
	start_stop_timer(TIMER_DEADLINE_TIMER, false);

	/* call the handler */
	if (timer_deadline_func)
		timer_deadline_func();
}

u_int64_t timer_get_ticks()
{
	return read_timer64();
}

u_int32_t timer_get_tick_rate(void)
{
	return timer64_ticks_per_sec;
}

utime_t timer_ticks_to_usecs(uint64_t ticks)
{
	return (ticks * 1000 * 1000) / timer64_ticks_per_sec;
}

uint64_t timer_usecs_to_ticks(utime_t usecs)
{
	return (usecs * timer64_scale) >> 20;
}

void timer_stop_all(void)
{
#ifndef TIMER_NO_INIT
	/* stop all timers */
	start_stop_timer(TIMER_A, false);
	start_stop_timer(TIMER_B, false);
	start_stop_timer(TIMER_C, false);
	start_stop_timer(TIMER_D, false);
	start_stop_timer(TIMER_E, false);
	start_stop_timer(TIMER_F, false);
	start_stop_timer(TIMER_G, false);
	start_stop_timer(TIMER_H, false);
	start_stop_timer(TIMER_64, false);
#endif
}

void timer_deadline_enter(uint64_t deadline, void (* callback)(void))
{
	uint64_t	ticks;
	uint32_t	decr;

	timer_deadline_func = callback;
	if (callback != NULL) {
		/* convert absolute deadline to relative time */
		ticks = read_timer64();
		if (deadline <= ticks) {
//			printf("setting deadline %llu before current time %llu\n", deadline, ticks);
			deadline = 1;
		} else {
//			printf("setting deadline %llu (0x%llx away)\n", deadline, deadline - ticks);
			deadline -= ticks;
		}

		/* clamp the deadline to our maximum */
#if TIMER_DEADLINE_USE16
		decr = (deadline > 0xffff) ? 0xffff : deadline;
#else
		decr = (deadline > 0xffffffff) ? 0xffffffff : deadline;
#endif
//		printf("  decr 0x%x\n", decr);
		
		/*
		 * Start the timer in interval mode.  We will stop it when we handle the
		 * interrupt, so it doesn't really matter as long as we get an interrupt after
		 * (decr) ticks or so.
		 */
		timer_status[TIMER_DEADLINE_TIMER].mat0_tick = timer_deadline;
		setup_timer(TIMER_DEADLINE_TIMER, decr, 0, MODE_INTERVAL, false, false, 0);
		start_stop_timer(TIMER_DEADLINE_TIMER, true);
	} else {
		/* stop the timer */
		start_stop_timer(TIMER_DEADLINE_TIMER, false);
	}
}

u_int32_t timer_get_entropy(void)
{
        return arch_get_entropy(&rTM64_CNTL);
}

void wdt_enable(void)
{
	rWDTCON = (1 << 20) | (15 << 16) | (4 << 12) | (0xA << 8);
}

void wdt_chip_reset(void)
{
	rWDTCON = (1 << 20) | (0 << 16) | (0 << 15) | (0 << 12) | (0 << 8) | (0 << 0);
}
