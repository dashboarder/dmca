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
#include <arch.h>
#include <debug.h>
#include <lib/libc.h>
#include <platform.h>
#include <platform/int.h>
#include <sys.h>

#include <arch/arm/arm.h>

#if WITH_HW_EDGEIC
#include <drivers/edgeic.h>
#endif

#include "pl192vic.h"

#define MAX_INTS (VICS_COUNT * 32)
static struct interrupt_entry handlers[MAX_INTS];

extern void __arm_irq(void);

#if SUPPORT_SLEEP
struct vic_state {
	u_int32_t	enabled;
	u_int32_t	select;
};

static struct vic_state vic_saved_state[VICS_COUNT];

# define SAVE_ENABLED(_v, _i, _s)					\
do {									\
	if (_s) {							\
		vic_saved_state[(_v)].enabled |= (1 << (_i));		\
	} else {							\
		vic_saved_state[(_v)].enabled &= ~(1<< (_i));		\
	}								\
} while(0)

# define SAVE_SELECT(_v, _i, _s)					\
do {									\
	if (_s) {							\
		vic_saved_state[(_v)].select |= (1 << (_i));		\
	} else {							\
		vic_saved_state[(_v)].select &= ~(1<< (_i));		\
	}								\
} while(0)

#else
# define SAVE_ENABLED(_v, _i, _s)
# define SAVE_SELECT(_v, _i, _s)
#endif /* SUPPORT_SLEEP */

int interrupt_init(void)
{
	u_int32_t vic, vec;

#if WITH_HW_EDGEIC
	/* clear the pending int source block */
	edgeic_reset();
#endif

	for (vic = 0; vic < VICS_COUNT; vic++) {
		/* mask everything */
		rVICINTENCLEAR(vic) = 0xffffffff;

		/* set all vectors as int */
		rVICINTSELECT(vic) = 0;

		/* unmask all priority levels */
		rVICSWPRIORITYMASK(vic) = 0xffff;
		
		/* set up all the vector addresses with the vector number */
		for(vec = 0; vec < 32; vec++) {
			rVICVECTADDR(vic, vec) = vic * 32 + vec;
		}

#if SUPPORT_SLEEP
		/* restore saved state, or initialize to all off */
		rVICINTENABLE(vic) = vic_saved_state[vic].enabled;
		rVICINTSELECT(vic) = vic_saved_state[vic].select;

		/* if any vectors are FIQs, unmask FIQ */
		if (vic_saved_state[vic].select != 0)
			arm_enable_fiqs();
#endif
	}
	/* also enable it */
	exit_critical_section();
	return 0;
}

void interrupt_mask_all(void)
{
	u_int32_t vic;

	/* mask everything */
	for (vic = 0; vic < VICS_COUNT; vic++) {
		rVICINTENCLEAR(vic) = 0xffffffff;
	}
}

void mask_int(u_int32_t vector)
{
	u_int32_t vic = vector / 32;
	u_int32_t vec = vector % 32;

	ASSERT(vic < VICS_COUNT);

	enter_critical_section();

	rVICINTENCLEAR(vic) |= 1 << vec;

	SAVE_ENABLED(vic, vec, false);
	
	exit_critical_section();
}

void unmask_int(u_int32_t vector)
{
	u_int32_t vic = vector / 32;
	u_int32_t vec = vector % 32;

	ASSERT(vic < VICS_COUNT);

	enter_critical_section();

	rVICINTENABLE(vic) |= 1 << vec;

	SAVE_ENABLED(vic, vec, true);
	
	exit_critical_section();
}


void set_int_type(uint32_t vector, int type)
{
	u_int32_t vic = vector / 32;
	u_int32_t vec = vector % 32;
	bool edge;

	ASSERT(vic < VICS_COUNT);

	enter_critical_section();

	if (type & INT_TYPE_FIQ) {
		rVICINTSELECT(vic) |= (1 << vec);
		SAVE_SELECT(vic, vec, true);
		arm_enable_fiqs();
	} else {
		rVICINTSELECT(vic) &= ~(1 << vec);
		SAVE_SELECT(vic, vec, false);
	}

	edge = (type & INT_TYPE_EDGE) != 0;
#if WITH_HW_EDGEIC
	handlers[vector].edge = edge;
	edgeic_select_edge(vector, edge);
#else
	if (edge)
		panic("set_int_type: edge not supported\n");
#endif

	exit_critical_section();
}

int install_int_handler(uint32_t vector, int_handler handler, void *arg)
{
	ASSERT(vector < MAX_INTS);

	enter_critical_section();

	handlers[vector].handler = handler;
	handlers[vector].arg = arg;

	exit_critical_section();

	return 0;
}

void platform_irq(void)
{
	u_int32_t vic, vector;

	for (vic = 0; vic < VICS_COUNT; vic++) {
		if (rVICIRQSTATUS(vic) != 0) {
			break;
		}
	}
	if (vic >= VICS_COUNT) {
		return;  // false alarm
	}

	vector = rVICADDR(vic);
//	printf("irq: %d\n", vector);

	if (vector < MAX_INTS ) {

		if (handlers[vector].edge) {
#if WITH_HW_EDGEIC
			edgeic_clear_interrupt(vector);
#else
			panic("platform_irq: edge not supported\n");
#endif
		}
		if (handlers[vector].handler)
			handlers[vector].handler(handlers[vector].arg);
	} else {
		void (*crap_isr)(void);
		crap_isr = (void (*)(void) )vector;
		crap_isr();
	}

	/* EOI the interrupt controller */
	rVICADDR(vic) = 1;
}

void platform_fiq(void)
{
	u_int32_t vic, pending = 0, vector = 0;

	for (vic = 0; vic < VICS_COUNT; vic++) {
		pending = rVICFIQSTATUS(vic);
		if (pending != 0) break;
		vector += 32;
	}

	if (pending == 0) {
		return;
	}

	vector += __builtin_ctz(pending);
//	printf("fiq: 0x%x %d\n", pending, vector);

	if (handlers[vector].handler)
		handlers[vector].handler(handlers[vector].arg);
}

#ifdef IPC_VIC_BASE_ADDR
/*
 * Ring our partner's doorbell.
 */
void interrupt_generate_ipc(u_int32_t vector)
{
	u_int32_t vic = vector / 32;
	u_int32_t vec = vector % 32;
	
	rVICIPCSOFTINT(vic) = (1 << vec);
}

/*
 * Clear the doorbell interrupt from our partner.
 */
void interrupt_clear_ipc(u_int32_t vector)
{
	u_int32_t vic = vector / 32;
	u_int32_t vec = vector % 32;
	
	rVICSOFTINTCLEAR(vic) = (1 << vec);
}
#endif

static void
do_int_panic(void *arg __unused)
{
	int vic, vec;
	for (vic = 0; vic < VICS_COUNT; vic++) {
		u_int32_t en = rVICINTENABLE(vic);
		u_int32_t stat = rVICIRQSTATUS(vic);

		printf("rVICINTENABLE(%d) = 0x%08x\n", vic, en);
		printf("rVICIRQSTATUS(%d) = 0x%08x\n", vic, stat);
		
		for (vec = 0; vec < 32; vec++) {
			const int vector = (vic * 32) + vec;
			const struct interrupt_entry *entry = &handlers[vector];
			const bool enabled = en & (1 << vec);
			const bool requested = stat & (1 << vec);
			// Print if it's enabled, requested or has a handler
			if (enabled || requested || (NULL != entry->handler)) {
				printf("vector[%d]: %s:%d - h=%p\n", vector,
					(enabled ? "unmsk" : "mask"),
					(requested ? 1 : 0),
					entry->handler);
			}
		}
	}
}

#if APPLICATION_EMBEDDEDIOP
PANIC_HOOK(irq, do_int_panic, NULL);
#endif
