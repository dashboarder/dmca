/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <sys/linker_set.h>

#define IOP_MESSAGE_NO_WAIT		0
#define IOP_MESSAGE_WAIT_FOREVER	0xffffffff	/* actually a bit more than an hour */

extern void	iop_message_trace(const char *ident, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

/*
 * Magic for hooking up IOP functions.
 */
struct iop_function {
	const char	*function_name;
	int		(* entrypoint)(void *cfg);
	int		stack_allocation;
	int		control_channel;
};

#define IOP_FUNCTION(_function_name, _entrypoint, _stack_allocation, _control_channel) \
static const struct iop_function					\
__attribute__ ((used))							\
__iop_function_##_function_name = {					\
	#_function_name,						\
	_entrypoint,							\
	_stack_allocation,						\
	_control_channel						\
};									\
LINKER_SET_ENTRY(iop_function, __iop_function_##_function_name);

/*
 * IOP sleep/wake notifications
 */

struct iop_sleep_hook {
	void	( *func)(int mode);
};
#define IOP_SLEEP_MODE_SLEEPING	0
#define IOP_SLEEP_MODE_WAKING	1

#define IOP_SLEEP_HOOK(_name, _hook_function)				\
static const struct iop_sleep_hook					\
__attribute__ ((used))							\
__iop_sleep_hook_##_name = {						\
	_hook_function							\
};									\
LINKER_SET_ENTRY(iop_sleep_hook, __iop_sleep_hook_##_name);
