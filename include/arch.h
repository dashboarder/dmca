/*
 * Copyright (C) 2007-2010 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __ARCH_H
#define __ARCH_H

#include <sys/types.h>
#include <compiler.h>

__BEGIN_DECLS

struct arch_task;
struct task;

int arch_cpu_init(bool resume);
int arch_cpu_init_posttasks(void);
int arch_cpu_quiesce(void);

void arch_halt(void);
void arch_spin(void) __attribute__((noreturn));

u_int32_t arch_get_noncached_address(u_int32_t address);

u_int32_t arch_enable_ints(void);
u_int32_t arch_disable_ints(void);
u_int32_t arch_restore_ints(u_int32_t state);

/*
 * arch_task_create
 *
 * Initialises the architecture-specific fields in the given task
 * such that it can be switched to and run.
 */
int arch_task_create(struct task *); // used internally by task_create()

/*
 * arch_task_context_switch
 *
 * Switches execution from the current task to a new task.  This can be used to
 * save the current state by switching from and to the same task.
 */
void arch_task_context_switch(struct arch_task *old, struct arch_task *new_task);

/*
 * arch_task_context_restore
 *
 * Switches execution to a previously saved state.
 */
void arch_task_context_restore(struct arch_task *restore_context);

/*
 * Prints a backtrace for the current execution context.
 */
void arch_backtrace_current_task(void *stack_base, size_t stack_len);

/*
 * Prints a backtrace for the given task.
 */
void arch_backtrace_task(struct arch_task *arch_task, void *stack_base, size_t stack_len);

#if WITH_VFP
/*
 * arch_task_fp_enable
 *
 * Enable floating point support for the current task.
 */
bool arch_task_fp_enable(bool enable);
#endif

/*
 * Collects  entropy by reading a counter across
 * an asynchronous boundary.  The returned word
 * will ideally contain at least one bit of entropy.
 */
u_int32_t arch_get_entropy(volatile u_int32_t *counter_address);

extern const char *build_banner_string;
extern const char *build_style_string;
extern const char *build_tag_string;

extern void _text_start __segment_start_sym(__TEXT);
extern void _text_end __segment_end_sym(__TEXT);
extern void _data_start __segment_start_sym(__DATA);
extern void _data_end __segment_end_sym(__DATA);

__END_DECLS

#endif
