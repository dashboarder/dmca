/*
 * Copyright (C) 2009-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch.h>
#include <arch/arm/arm.h>
#include <debug.h>
#include <drivers/aic/aic.h>
#include <platform.h>
#include <platform/int.h>
#include <platform/timer.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <platform/soc/hwregbase.h>
#include <platform/soc/pmgr.h>
#include <sys/callout.h>

#if WITH_AIC_TIMERS
static void timer_deadline(void *arg);
static void (* timer_deadline_func)(void);
#endif

#if WITH_AIC_INTERRUPTS || WITH_AIC_TIMERS

static inline uint32_t _aic_read_reg(volatile uint32_t *reg)
{
	uint32_t val;
#if PLATFORM_VARIANT_IOP || AIC_VERSION > 0
	val = *reg;
#else
	/* An exclusive load is required for the CPU ID to be passed along */
	__asm__ volatile( "ldrex %0, [%1]" : "=r" (val) : "r" (reg));
#endif // PLATFORM_VARIANT_IOP || AIC_VERSION > 0
	return val;
}

static inline void _aic_write_reg(volatile uint32_t *reg, uint32_t val)
{
	*reg = val;
}

static void _aic_init()
{
#if AIC_VERSION > 0
	rAIC_GLB_CFG |= (AIC_GLBCFG_SYNC_ACG | AIC_GLBCFG_EIR_ACG | AIC_GLBCFG_REG_ACG);

	/* Clear the timeout values before setting them to avoid set bits from the default values */
	rAIC_GLB_CFG &= ~(AIC_GLBCFG_SEWT(AIC_GLBCFG_WT_MASK) | AIC_GLBCFG_AEWT(AIC_GLBCFG_WT_MASK));
	rAIC_GLB_CFG &= ~(AIC_GLBCFG_SIWT(AIC_GLBCFG_WT_MASK) | AIC_GLBCFG_AIWT(AIC_GLBCFG_WT_MASK));
	rAIC_GLB_CFG |= (AIC_GLBCFG_SEWT(AIC_GLBCFG_WT_64MICRO) | AIC_GLBCFG_AEWT(AIC_GLBCFG_WT_64MICRO));
	rAIC_GLB_CFG |= (AIC_GLBCFG_SIWT(AIC_GLBCFG_WT_64MICRO) | AIC_GLBCFG_AIWT(AIC_GLBCFG_WT_64MICRO));
#else
	rAIC_GLB_CFG |= AIC_GLBCFG_ACG;

	/* Clear the timeout values before setting them to avoid set bits from the default values */
	rAIC_GLB_CFG &= ~(AIC_GLBCFG_EWT(AIC_GLBCFG_WT_MASK) | AIC_GLBCFG_IWT(AIC_GLBCFG_WT_MASK));
	rAIC_GLB_CFG |= (AIC_GLBCFG_EWT(AIC_GLBCFG_WT_64MICRO) | AIC_GLBCFG_IWT(AIC_GLBCFG_WT_64MICRO));
#endif // AIC_VERSION > 0

	// Enable Interrupts
	rAIC_GLB_CFG |= AIC_GLBCFG_IEN;
}

#endif

#if WITH_AIC_INTERRUPTS

/*
 * Interrupt support
 */

extern void __arm_irq(void);

static struct interrupt_entry handlers[kAIC_NUM_INTS];

#if AIC_VERSION < 1
static uint32_t fiq_source = kAIC_INT_SPURIOUS;
#endif // AIC_VERSION < 1

static void _aic_mask_timer_int(bool mask)
{
#if AIC_VERSION < 1
	if (mask)
		_aic_write_reg(&rAIC_TMR_ST_SET, AIC_TMRST_TIM);
	else
		_aic_write_reg(&rAIC_TMR_ST_CLR, AIC_TMRST_TIM);
#endif // AIC_VERSION < 1
}

static void _aic_mask_int_internal(uint32_t vector)
{
	if (vector == kAIC_VEC_IPI) {
		/* Not maskable */
	} else if (vector == kAIC_VEC_TMR) {
		_aic_mask_timer_int(true);
	} else if (vector == kAIC_VEC_SW_TMR) {
		/* Not maskable */
	} else {
		uint32_t reg = AIC_SRC_TO_EIR(vector);
		rAIC_EIR_MASK_SET(reg) = AIC_SRC_TO_MASK(vector);
	}
}

static void _aic_unmask_int_internal(uint32_t vector)
{
	if (vector == kAIC_VEC_IPI) {
		/* Not maskable */
	} else if (vector == kAIC_VEC_TMR) {
		_aic_mask_timer_int(false);
	} else {
		/* if AIC client requested interrupt state is not masked, unmask it */
		if (!handlers[vector].int_mask_requested) {
			uint32_t reg = AIC_SRC_TO_EIR(vector);
			rAIC_EIR_MASK_CLR(reg) = AIC_SRC_TO_MASK(vector);
		}
	}
}

int interrupt_init(void)
{
	_aic_init();

#if !PLATFORM_VARIANT_IOP
	/* mask everything */
	interrupt_mask_all();
#endif

	/* also enable it */
	exit_critical_section();

	return 0;
}

void interrupt_mask_all(void)
{
	uint32_t eir;

	/* mask everything */
	for (eir = 0; eir < kAIC_NUM_EIRS; eir++) {
		rAIC_EIR_MASK_SET(eir) = 0xffffffff;
	}
	/* XXX timers? */
}

void mask_int(uint32_t vector)
{
	/* save AIC client requested interrupt mask state */
	handlers[vector].int_mask_requested = true;
	_aic_mask_int_internal(vector);
}

void unmask_int(uint32_t vector)
{
	/* clear AIC client requested interrupt mask state */
	handlers[vector].int_mask_requested = false;
	_aic_unmask_int_internal(vector);
}


void set_int_type(uint32_t vector, int type)
{
	enter_critical_section();

#if AIC_VERSION < 1
	uint32_t reg;

	if (type & INT_TYPE_FIQ) {
		reg = _aic_read_reg(&rAIC_TMR_CFG);
		reg &= ~kAIC_TMRCFG_FSL_MASK;
		if ((reg & (AIC_TMRCFG_IMD|AIC_TMRCFG_EMD|AIC_TMRCFG_SMD)) !=
		    (AIC_TMRCFG_IMD|AIC_TMRCFG_EMD|AIC_TMRCFG_SMD))
			panic("Can't support multiple timer FIQ sources\n");
		if (vector == kAIC_VEC_TMR) {
			reg &= ~AIC_TMRCFG_IMD;
			_aic_write_reg(&rAIC_TMR_CFG, reg | AIC_TMRCFG_FSL(kAIC_TMRCFGFSL_PVT));
			fiq_source = kAIC_INT_PVT_TMR;
			arm_enable_fiqs();
		} else if (vector == kAIC_VEC_SW_TMR) {
			reg &= ~AIC_TMRCFG_SMD;
			_aic_write_reg(&rAIC_TMR_CFG, reg | AIC_TMRCFG_FSL(kAIC_TMRCFGFSL_SGT));
			fiq_source = kAIC_INT_SW_TMR;
			arm_enable_fiqs();
		}
	} else {
		if (vector == kAIC_VEC_TMR) {
			_aic_write_reg(&rAIC_TMR_CFG, _aic_read_reg(&rAIC_TMR_CFG) | AIC_TMRCFG_IMD);
		} else if (vector == kAIC_VEC_SW_TMR) {
			_aic_write_reg(&rAIC_TMR_CFG, _aic_read_reg(&rAIC_TMR_CFG) | AIC_TMRCFG_SMD);
		}
	}
#endif // AIC_VERSION < 1

	if (type & INT_TYPE_EDGE)
		panic("set_int_type: edge not supported\n");

	exit_critical_section();
}

int install_int_handler(uint32_t vector, int_handler handler, void *arg)
{
	ASSERT(vector < kAIC_NUM_INTS);

	enter_critical_section();

	handlers[vector].handler = handler;
	handlers[vector].arg = arg;

	if (vector < kAIC_MAX_EXTID) {
		rAIC_EIR_DEST(vector) = 1 << AIC_CPU_ID;
	}

	exit_critical_section();

	return 0;
}

static uint32_t spurious = 0;

#define SPURIOUS_DEBUG 0
#define PLATFORM_IRQ_SLICES	((PLATFORM_IRQ_COUNT + 31) / 32)

void platform_irq(void)
{
#if SPURIOUS_DEBUG
	uint32_t unmasked[PLATFORM_IRQ_SLICES];
	uint32_t i;

	for (i = 0; i < PLATFORM_IRQ_SLICES; i++) {
		unmasked[i] = rAIC_EIR_INT_RO(i) & ~rAIC_EIR_MASK_CLR(i);
	}
#endif
	uint32_t vector = _aic_read_reg(&rAIC_IACK);
	uint32_t source;

	if (vector == kAIC_INT_SPURIOUS) {
		/* Do some extra checking to be sure IACK is behaving
		 * as expected.  We don't really expect spurious
		 * interrupts in this environment, and certainly want
		 * to see a non-spurious IACK any time a source is
		 * active. */
#if SPURIOUS_DEBUG
		for (i = 0; i < PLATFORM_IRQ_SLICES; i++) {
			if (unmasked[i])
				panic("IACK=0 but sources are pending\n");
		}
		printf("spurious\n");
#endif
		spurious++;
		return;
	}
	while (vector != kAIC_INT_SPURIOUS) {
		if (AIC_INT_EXT(vector)) {
			source = AIC_INT_EXTID(vector);
		} else if (AIC_INT_IPI(vector)) {
			panic("Unexpected IPI\n");
#if AIC_VERSION < 1
		} else if (vector == kAIC_INT_PVT_TMR) {
			source = kAIC_VEC_TMR;
		} else if (vector == kAIC_INT_SW_TMR) {
			_aic_write_reg(&rAIC_TMR_ST_CLR, AIC_TMRST_SGT);
			source = kAIC_VEC_SW_TMR;
#endif // AIC_VERSION < 1
		} else {
			panic("Unrecognized IRQ vector: %x\n", vector);
		}
		if (handlers[source].handler)
			handlers[source].handler(handlers[source].arg);
		_aic_unmask_int_internal(source);
		vector = _aic_read_reg(&rAIC_IACK);
	}
}

#if AIC_VERSION < 1

void platform_fiq(void)
{
	uint32_t source;

	if (fiq_source == kAIC_INT_SW_TMR) {
		_aic_write_reg(&rAIC_TMR_ST_CLR, AIC_TMRST_SGT);
		source = kAIC_VEC_SW_TMR;
	} else if (fiq_source == kAIC_INT_PVT_TMR) {
		source = kAIC_VEC_TMR;
	} else
		panic("Unexpected FIQ\n");
	if (handlers[source].handler)
		handlers[source].handler(handlers[source].arg);
	_aic_unmask_int_internal(source);
}

#endif // AIC_VERSION < 1

#if AIC_VERSION == 2

void platform_fiq(void)
{
	/* clear FIQ and disable decrementer */
	_aic_write_reg(&rAIC_TMR_CFG, ~AIC_TMRCFG_EN);
	_aic_write_reg(&rAIC_TMR_ISR, AIC_TMRISR_PCT);

	/* if we have a callback, invoke it now */
	if (timer_deadline_func) {
		timer_deadline_func();
	}
}

#endif // AIC_VERSION == 2

#if PLATFORM_VARIANT_IOP

void interrupt_generate_ipc(uint32_t vector)
{
	uint32_t reg = AIC_SRC_TO_EIR(vector);
	rAIC_EIR_SW_SET(reg) = AIC_SRC_TO_MASK(vector);
}

void interrupt_clear_ipc(uint32_t vector)
{
	uint32_t reg = AIC_SRC_TO_EIR(vector);
	rAIC_EIR_SW_CLR(reg) = AIC_SRC_TO_MASK(vector);
}

#endif

#endif

#if WITH_AIC_TIMERS
/*
 * Timer support; currently assumes that the banked register block works
 * (XXX no it doesn't - see top of file)
 */

#define DECR_MAX_COUNT	(0xffffffffULL)

int timer_init(uint32_t timer)
{
	/* Allow only banked access to CPU's own timer */
	if (timer != 0) return -1;

	/* Default to disabled and IRQ for all modes */
	_aic_write_reg(&rAIC_TMR_CFG, AIC_TMRCFG_IMD|AIC_TMRCFG_EMD|AIC_TMRCFG_SMD);

	/* install our interrupt handler */
	install_int_handler(kAIC_VEC_TMR, (int_handler)&timer_deadline, NULL);
	unmask_int(kAIC_VEC_TMR);

	return 0;
}

void timer_stop_all(void)
{
	/* Even though it says "all", only stop the dedicated timer */
	_aic_write_reg(&rAIC_TMR_CFG, _aic_read_reg(&rAIC_TMR_CFG) & ~AIC_TMRCFG_EN);
}

void timer_deadline_enter(uint64_t deadline, void (* func)(void))
{
	uint64_t	ticks;
	uint32_t	decr;
	
	timer_deadline_func = func;
	if (func) {
//		printf("installing deadline %p\n", func);
		
		/* convert absolute deadline to relative time */
		ticks = timer_get_ticks();
		if (deadline < ticks) {
//			printf(" %llu too soon (it is now %llu)\n", deadline, ticks);
#if AIC_VERSION == 2
			deadline = 1;
#else // AIC_VERSION == 2
			deadline = 0;
#endif
		} else {
			deadline -= ticks;
		}

		/* clamp the deadline to our maximum, which is about 178 seconds */
		decr = (deadline > DECR_MAX_COUNT) ? DECR_MAX_COUNT : deadline;
//		printf(" using decrementer count %u\n", decr);

		/*
		 * Reset the decrementer by programming the max count, clearing
		 * any pending interrupt status, then programming the desired count.
		 */
		_aic_write_reg(&rAIC_TMR_CNT, DECR_MAX_COUNT);
		_aic_write_reg(&rAIC_TMR_CFG, _aic_read_reg(&rAIC_TMR_CFG) | AIC_TMRCFG_EN);
		_aic_write_reg(&rAIC_TMR_ISR, AIC_TMRISR_PCT);
		_aic_write_reg(&rAIC_TMR_CNT, decr);
	} else {
		/* no deadline, disable the decrementer */
		_aic_write_reg(&rAIC_TMR_CFG, _aic_read_reg(&rAIC_TMR_CFG) & ~AIC_TMRCFG_EN);
		_aic_write_reg(&rAIC_TMR_ISR, _aic_read_reg(&rAIC_TMR_ISR) | AIC_TMRISR_PCT);
	}
}

static void timer_deadline(void *arg)
{
	/* acknowledge the interrupt and disable the timer */
	_aic_write_reg(&rAIC_TMR_CFG, _aic_read_reg(&rAIC_TMR_CFG) & ~AIC_TMRCFG_EN);
	_aic_write_reg(&rAIC_TMR_ISR, _aic_read_reg(&rAIC_TMR_ISR) | AIC_TMRISR_PCT);

	/* if we have a callback, invoke it now */
	if (timer_deadline_func)
		timer_deadline_func();
}

#endif // WITH_AIC_TIMERS

/*
 * The rest are defined for all targets that include the AIC driver
 */

uint64_t aic_get_ticks(void)
{
	uint32_t time_lo, time_hi;

	/* read the timebase */
	do {
		time_hi = rAIC_TIME_HI;
		time_lo = rAIC_TIME_LO;
	} while (time_hi != rAIC_TIME_HI);

	return ((uint64_t)time_hi << 32) | time_lo;
}

void aic_spin(uint32_t usecs)
{
	uint32_t delta_time = usecs * (OSC_FREQ / 1000000);
	uint32_t start_time = rAIC_TIME_LO;

	while ((rAIC_TIME_LO - start_time) < delta_time);
}

#if WITH_AIC_TIMERS

#ifdef OSC_FREQ

utime_t timer_ticks_to_usecs(uint64_t ticks)
{
#if (OSC_FREQ % 1000000) == 0
	return ticks / (OSC_FREQ / 1000000);
#else
#error "The code below has overflow issues. Make sure you really want to use it"
	return (ticks * 1000 * 1000) / OSC_FREQ;
#endif
}

uint64_t timer_usecs_to_ticks(utime_t usecs)
{
#if (OSC_FREQ % 1000000) == 0
	return usecs * (OSC_FREQ / 1000000);
#else
#error "The code below has overflow issues. Make sure you really want to use it"
	uint64_t timer_scale = ((uint64_t)(OSC_FREQ) << 20) / (1000 * 1000);
	return (ticks * timer_scale) >> 20;
#endif
}

uint32_t timer_get_tick_rate(void)
{
	return OSC_FREQ;
}

#endif // OSC_FREQ

uint64_t timer_get_ticks(void)
{
	return aic_get_ticks();
}

#endif  // WITH_AIC_TIMERS

uint32_t timer_get_entropy(void)
{
	return arch_get_entropy(&rAIC_TIME_LO);
}

#ifndef PMGR_WDG_VERSION
#error "PMGR_WDG_VERSION not defined"
#endif

#if PMGR_WDG_VERSION < 1
void wdt_chip_reset(void)
{
	rPMGR_WDOG_CTL = 0;		// Turn off any pending events
	rPMGR_WDOG_RST = 1;		// Set counter for immediate reset
	rPMGR_WDOG_TMR = 0x80000000;	// Set counter to a safe value
	rPMGR_WDOG_CTL = 4;		// Enable watchdog reset
	rPMGR_WDOG_TMR = 0;		// Reset counter to zero
}

void wdt_enable(void)
{
	rPMGR_WDOG_CTL = 0;		// Turn off any pending events
	rPMGR_WDOG_RST = 120*OSC_FREQ;	// Set counter for two minute reset
	rPMGR_WDOG_TMR = 0x80000000;	// Set counter to a safe value
	rPMGR_WDOG_CTL = 0x0C;		// Enable watchdog system reset
	rPMGR_WDOG_TMR = 0;		// Reset counter to zero
}
#else
void wdt_chip_reset(void)
{
	rPMGR_CHIP_WDOG_CTL = 0;
	rPMGR_CHIP_WDOG_RST_CNT = 1;
	rPMGR_CHIP_WDOG_TMR = 0x80000000;
	rPMGR_CHIP_WDOG_CTL = 4;
	rPMGR_CHIP_WDOG_TMR = 0;
}

void wdt_system_reset(void)
{
	rPMGR_SYS_WDOG_CTL = 0;		// Turn off watchdog
	rPMGR_SYS_WDOG_RST_CNT = 1;	// Set counter for immediate reset
	rPMGR_SYS_WDOG_TMR = 0x80000000;// Set up-counter to safe value
	rPMGR_SYS_WDOG_CTL = 4;		// Turn on interrupt
	rPMGR_SYS_WDOG_TMR = 0;		// Reset counter to zero
}

void wdt_enable(void)
{
	rPMGR_SYS_WDOG_CTL = 0;
	rPMGR_SYS_WDOG_RST_CNT = 120 * OSC_FREQ;
	rPMGR_SYS_WDOG_TMR = 0x80000000;
	rPMGR_SYS_WDOG_CTL = 4;
}
#endif
