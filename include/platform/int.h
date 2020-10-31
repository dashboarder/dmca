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
#ifndef __PLATFORM_INT_H
#define __PLATFORM_INT_H

#include <sys/types.h>

__BEGIN_DECLS

/* interrupt controller */
int interrupt_init(void);
void interrupt_mask_all(void);
void unmask_int(uint32_t vector);
void mask_int(uint32_t vector);

/* optional for platforms requiring polling masked interrupts */
bool test_int(uint32_t vector);

int eint_init(void);
void eint_mask_all(void);
void unmask_eint(uint32_t eint, uint32_t vector);
void mask_eint(uint32_t eint, uint32_t vector);
void eint_trig(uint32_t eint, uint32_t vector, uint32_t trig);
void eint_pol(uint32_t eint, uint32_t vector, uint32_t trig);

#define INT_TYPE_IRQ 0
#define INT_TYPE_FIQ 1
#define INT_TYPE_LEVEL   0
#define INT_TYPE_EDGE    2

void set_int_type(uint32_t vector, int type);
typedef void (*int_handler)(void *arg);
int install_int_handler(uint32_t vector, int_handler handler, void *arg);
int install_eint_handler(uint32_t eint, uint32_t vector, int_handler handler, void *arg);

struct interrupt_entry {
	int_handler handler;
	void *arg;
	bool edge;
	bool int_mask_requested;	/* interrupt controller client requested interrupt mask state */
};

/* interprocessor interrupts */
void interrupt_generate_ipc(uint32_t vector);
void interrupt_clear_ipc(uint32_t vector);

/* interrupt routines */
void platform_irq(void);
void platform_fiq(void);

__END_DECLS

#endif

