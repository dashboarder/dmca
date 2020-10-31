/*
 * Copyright (C) 2006-2011 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <debug.h>
#include <arch.h>
#include <compiler.h>
#include <lib/env.h>
#include <lib/heap.h>
#include <lib/mib.h>
#include <lib/random.h>
#include <platform/memmap.h>
#include <sys.h>
#include <sys/task.h>
#include <sys/security.h>
#include <sys/callout.h>

/* these need to be non-static and initialised for applications that patch them */
#define HEAP_SIZE_UNPATCHED	((size_t)-1)
void		*heap_base = (void *)-1;
size_t		heap_size = HEAP_SIZE_UNPATCHED;

MIB_CONSTANT_PTR(kMIBBuildBanner, kOIDTypeString|kOIDDataIndirect, (void *)&build_banner_string);
MIB_CONSTANT_PTR(kMIBBuildStyle,  kOIDTypeString|kOIDDataIndirect, (void *)&build_style_string);
MIB_CONSTANT_PTR(kMIBBuildTag,    kOIDTypeString|kOIDDataIndirect, (void *)&build_tag_string);

/* called from each binary's main() routine to initialize basic system services */
void sys_init(void)
{
	/*
	 * Initialise the heap.
	 *
	 * If HEAP_END is defined, we assume that we are going to
	 * configure the heap size using it.
	 * 
	 * If HEAP_BASE is defined, we respect the definition.  Otherwise
	 * we start the heap at the end of the __DATA segment.
	 *
	 * If HEAP_END is not defined, someone may have patched heap_base/
	 * heap_end, so we check and create a heap with their values if they
	 * have.
	 */
#ifdef HEAP_END
# ifdef HEAP_BASE
	/* we have a defined base for the heap */
	heap_base = (void *)HEAP_BASE;
# else
	/* create the heap at the end of the __DATA segment */
	heap_base = __segment_end(__DATA);
# endif
	/* compute the heap size*/
	heap_size = HEAP_END - (uintptr_t)heap_base;
#endif
	/* if we have computed (or had patched in) a heap size, create the heap */
	if (heap_size != HEAP_SIZE_UNPATCHED) {
#if WITH_RANDOM
		bool random_heap_cookie = mib_get_bool(kMIBTargetRandomHeapCookie);

		if (random_heap_cookie) {
			uint8_t heap_cookie[HEAP_COOKIE_SIZE];

			random_get_bytes_noheap(heap_cookie, sizeof(heap_cookie), heap_base, heap_size);
			heap_set_cookie(heap_cookie);
		}
#endif

		heap_add_chunk(heap_base, heap_size, true);
	}

	/* initialize security */
#if !WITH_NO_SECURITY
	security_init(false);
#endif

#if WITH_ENV
	/* populate the environment with some default values */
	sys_setup_default_environment();
#endif

	/* initialize the tasking system */
	task_init();

	/* after tasks are initialized, do some more cpu init (if needed) */
	arch_cpu_init_posttasks();

	/* initialize the debug i/o queue */
	debug_init();
}

