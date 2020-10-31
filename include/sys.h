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

/**
 * \file	sys.h
 */

/**
 * \defgroup	API	Application Programming Interfaces
 * \defgroup	SPI	System Private Interfaces
 */

#ifndef __SYS_H
#define __SYS_H

#include <compiler.h>
#include <sys/types.h>
#include <sys/linker_set.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <non_posix_types.h>

__BEGIN_DECLS

/**
 * Formats and displays the supplied message, then stops the system.
 *
 * The message is formatted by printf, and all formats are supported.
 *
 * \ingroup	API
 */
#if NO_PANIC_STRINGS
# define panic(x...) _panic("", "")
#elif TERSE_PANIC_STRINGS
# define panic(x...) _panic("", "%llx:%d", DEBUG_FILE_HASH, __LINE__)
#else
# define panic(x...) _panic(__func__, x)
#endif

extern char *gPanicStr;                 /**< pointer to panic string if panicking \ingroup API*/
extern const char *gPanicFunc;          /**< pointer to function name if panicking \ingroup API*/

/** \implements panic \ingroup SPI */
void _panic(const char *func, const char *fmt, ...) __noreturn;

/** performs minimal system quiesce and then stops the system \ingroup SPI */
void halt(void) __noreturn;

/**
 * \def		PANIC_HOOK
 * Panic hooks
 *
 * Invoked when the system panics.  Hook functions should avoid use of
 * system facilities, in particular anything involving interrupts or
 * task functions.
 *
 * \note panic hooks are invoked one at a time in no particular order.
 * \ingroup	API
 */
#if WITH_PANIC_HOOKS
struct panic_hook
{
        void (*func)(void *arg);        /**< the called function */
        void *arg;                      /**< passed to the called function */
};

# define PANIC_HOOK(_name, _func, _arg)                      \
        static const struct panic_hook __attribute__((used)) \
        __panic_hook_ ## _name = {                           \
                _func,                                       \
                _arg                                         \
        };                                                   \
        LINKER_SET_ENTRY(panic_hooks, __panic_hook_ ## _name)
#else
# define PANIC_HOOK(_name, _func, _arg) \
        struct hack
#endif

/* debug i/o routines */
extern int DebugUartReady;              /**< bits set indicate UARTs ready for debug use \ingroup SPI */

/**
 * \defgroup	debug	Debugging Support
 * \ingroup	SPI
 */

/**
 * initialise debug output
 *
 * \ingroup	debug
 */
void debug_init(void);

/**
 * enable debug output via UART
 *
 * \param[in]	debug_uarts	the UART channels to enable
 * \ingroup	debug
 */
void debug_enable_uarts(int debug_uarts);

/**
 * emit one character to the debug console
 *
 * \param[in]	c		the character to be emitted
 * \ingroup	debug
 */
void debug_putchar(int c);

/**
 * get one character from the debug console
 *
 * \return the character read
 * \ingroup	debug
 */
int debug_getchar(void);

/**
 * get one character from the debug console without waiting
 *
 * \return the character read or -1 if no character is waiting
 * \ingroup	debug
 */
int debug_getchar_nowait(void);

/**
 * push a character into the input queue
 *
 * \param[in]	c		the character to be pushed
 * \ingroup	debug
 */
int debug_pushchar(int c);

/**
 * run a canned script
 *
 * \param[in]	addr		the address of a NUL-terminated buffer containing the script
 * \ingroup	debug
 */
int debug_run_script(const char *addr);

#if WITH_APPLICATION_PUTCHAR
/**
 * passes a character being output to the debug console on to the application
 *
 * \note This function is implemented by the application.
 * \param[in]	c		the character being output
 * \ingroup	debug
 */
void application_putchar(int c);
#endif

/**
 * \defgroup	critical_sections Critical Sections
 * \ingroup	API
 */

/**
 * \fn		enter_critical_section
 * Enters a critical section, disabling interrupts if they are enabled.
 * \ingroup	critical_sections
 */
/**
 * \fn		exit_critical_section
 * Exits a critical section, re-enableing interrupts if this is the outermost nested section.
 * \ingroup	critical_sections
 */
#if DEBUG_CRITICAL_SECTIONS
# define enter_critical_section()       _enter_critical_section(__FUNCTION__)
# define exit_critical_section()        _exit_critical_section(__FUNCTION__)
void _enter_critical_section(const char *caller);       /**< \implements enter_critical_section \ingroup SPI */
void _exit_critical_section(const char *caller);        /**< \implements exit_critical_section \ingroup SPI */
#else
void enter_critical_section(void);
void exit_critical_section(void);
#endif

/* the following are only used in interrupt handler glue */
void _irq_enter_critical_section(void); /**< \ingroup SPI */
void _irq_exit_critical_section(void);  /**< \ingroup SPI */

/**
 * \defgroup	time	Time
 * \ingroup	API
 */

/**
 * \return	system uptime in microseconds.
 * \ingroup	time
 */
utime_t system_time(void);

/**
 * pause for a specified period
 *
 * \note	the system may enter a power-saving mode while waiting
 *
 * \param[in]	usecs		time to wait before returning
 * \ingroup	time
 */
void spin(utime_t usecs);

/**
 * test whether the specified time interval has elapsed
 *
 * \return	true if the interval has elapsed
 * \param[in]	start_time	the value returned by system_time at the start of the interval
 * \param[in]	timeout		the interval duration in microseconds
 * \ingroup	time
 */
bool time_has_elapsed(utime_t start_time, utime_t timeout);

/**
 * \return	calendar time in microseconds since midnight, January 1, 1970
 * \ingroup	time
 */
utime_t calendar_time(void);

/* system initialization */
void sys_init(void);                            /**< \ingroup SPI */
void sys_setup_default_environment(void);       /**< \ingroup SPI */
void sys_load_environment(void);                /**< \ingroup SPI */

/**
 * Initialize stack cookies.
 *
 * \note        Must be called from a stack frame that will never return
 *              by conventional methods (i.e., return statement or falling
 *              through the final closing brace).
 * \ingroup	SPI
 */
void sys_init_stack_cookie(void);

/**
 * \defgroup	memory_mapping	Memory Mapping
 * \ingroup	API
 */

/**
 * Structure describing static memory mappings.
 *
 * This must be defined by the platform.  The array is terminated
 * by an entry with a length of zero.
 *
 * If a mapping does not exist with a particular attribute,
 * set the field to MAP_NO_ENTRY.
 */
struct mem_static_map_entry {
        addr_t          cached_base;
        addr_t          uncached_base;
        addr_t          physical_base;
        size_t		size;
};

#define MAP_NO_ENTRY    (~(uint32_t)0)  /**< \ingroup memory_mapping */
#define MAP_FAILED      ((void *)~0)    /**< \ingroup memory_mapping */

/** platform-defined memory mappings */
extern struct mem_static_map_entry mem_static_map_entries[];

/**
 * Finds the static cached mapping for a pointer.
 *
 * \return	cached mapping
 * \retval      MAP_FAILED      no cached mapping exists
 * \ingroup	memory_mapping
 */
void *mem_static_map_cached(uintptr_t ptr);

/**
 * Finds the static uncached mapping for a pointer.
 *
 * \return	uncached mapping
 * \retval      MAP_FAILED      no uncached mapping exists
 * \ingroup	memory_mapping
 */
void *mem_static_map_uncached(uintptr_t ptr);

/**
 * Finds the physical address as seen by a bus-master device for a pointer.
 *
 * \return	the physical address the pointer refers to
 * \retval      MAP_NO_ENTRY    there is no physical mapping for this pointer
 * \ingroup	memory_mapping
 */
uintptr_t mem_static_map_physical(uintptr_t ptr);

__END_DECLS

#endif
