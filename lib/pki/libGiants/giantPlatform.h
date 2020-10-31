/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * giantPlatform.h - platform specific type and routines, must be ported to 
 *				   each platform on	which this library runs. 
 */

#ifndef	_GIANT_PLATFORM_H_
#define _GIANT_PLATFORM_H_

/*
 * This is defined true only for development and testing.
 * For production code, leave it zero and define your platform 
 * in this file.
 */

#define GI_PLATFORM_EMULATE		1
#if		GI_PLATFORM_EMULATE
#include <libGiants/giantPlatformEmulation.h>
#else

/* be sure to #define NDEBUG for deployment builds */
#define NDEBUG
#ifdef	NDEBUG
#define GIANTS_DEBUG	0
#else
#define GIANTS_DEBUG	1
#endif

/* 
 * Machine-dependent word-size related integer types. 
 *
 * The following integer types must be defined in this module:
 *
 * gi_uint8		-- unsigned 8 bit (typically "unsigned char")
 * gi_uint16	-- unsigned 16 bit, platform/compiler dependent
 * gi_uint32	-- unsigned 32-bit integer, platform/compiler dependent
 * gi_size		-- unsigned, at *least* 16 bits. Could be larger if that's
 *				   more efficient for the platform. For example on a 32-bit
 *				   processor this should be an unsigned 32 bit integer. 
 *				   Generally used inside of functions as counters, indices,
 *				   etc. Function parameters which must be portable to 
 *				   small processors are expressed as gi_uint16 or 
 *				   gi_uint32. 
 * giantDigit	-- see below
 * giantDouble	-- two giant digits, the result of a giantDigit multiply
 *
 * giantDigit is the largest native word size for which 
 * a (giantDigit x giantDigit) hardware multiply (into a giantDouble,
 * somehow) is available. This is a crucial data type; the large integer
 * artithmetic package uses data types called 'giants' which are mainly 
 * comprised of arrays of giantDigits. Most of the CPU cycles spent in this
 * library are involved in performing arithmetic on these arrays of 
 * giantDigits. 
 *
 * Our assumption is that the following all always fit into a gi_uint16:
 *
 * number of bits per giant digit
 * number of giant digits in the max base prime in this module
 * number of bits in the max base prime in this module
 *
 * Note we also assume the existence of a 16-bit integer type; arithmetic 
 * involving this type doesn't have to be particularly fast but it has to be
 * usable as a function argument and in simple assignments. Thus 8-bit 
 * platforms can run this code as long as there is a C type which compiles
 * to 16 bits. 
 *
 * These preprocessor symbols also need to be defined for each platform:
 *
 * GI_GIANT_LOG2_BITS_PER_DIGIT 
 *		The log to base 2 of the size of giantDigit. There
 *		is no straightforward way for the C preprocessor to figure this one 
 *		out so you have to specify it. For 16-bit giantDigits the value of 
 *		this symbol is 4; for 32-bit giantDigits the value is 5. 
 *
 * GI_GIANT_PORT_INLINE
 *		1 : we don't have a platform-specific giant arithmetic module; use the 
 *			inlines in giantPort_Generic.h
 *		0 : we have a platform-specific giant arithmetic module
 *
 * GI_INLINE_SMALL_FUNCS
 *		1 : Use 'static inline' for a select set of small (1-3 line) functions. 
 *		    Trade more code for faster runtime and less stack usage. The specific 
 *		    effect of this flag wil be nighly dependent on your compiler; in some 
 *		    cases you might even see smaller code size when this flag is true.
 *		0 : Disable this feature, use actual C functions.
 *
 * GI_MACRO_SMALL_FUNCTIONS
 *
 *		Like GI_INLINE_SMALL_FUNCS, except causes small functions to be implemented
 *		as macros. For platforms that don't support the 'inline' keyword. Mutually
 *		exclusive with GI_INLINE_SMALL_FUNCS.
 *		1 : Use #defined macros for the functions in question. 
 *		0 : Disable; use actual C functions.
 *
 *  
 * GI_SMALL_CRT_ENABLE
 *
 *		This flag involves a tradeoff between code size and stack usage in the 
 *		giantMod.c module. Setting this flag true increases code size and
 *		decreases stack usage. 
 * 
 *		On a G4, the tradeoff is that setting this flag saves 384 bytes of 
 *		stack space, at the cost of 1407 extra bytes of code.
 *
 *		Note: this flag has no effect if GI_MODG_ENABLE is 0.
 *
 *		0 : disable the small-CRT functions to save code space.
 *		1 : enable to the small-CRT functions to save stack space.
 *
 * GI_HAVE_MEMCPY
 *		1 : This platform has memcpy() and we want to use it as the implementation
 *			of Memcpy()
 *		0 : We've provided our own Memcpy()
 *
 * GI_HAVE_BZERO
 *		1 : This platform has bzero() and we want to use it as the implementation
 *			of Bzero()
 *		0 : We've provided our own Bzero()
 *
 * GI_INLINE
 *		Used where the 'inline' keyword would normally be used. #define to 'inline' 
 *      if your compiler provides this functionality, else #define it to nothing or
 *		some other compiler-specific value.
 *
 * GI_MODG_ENABLE
 *
 *		When nonzero, enables the modular arithmetic functions found in 
 *		giantMod.[ch]. Some applications such as SFEE do not need these functions.
 *		Other applications, like RSA and BBS, do in fact need them.
 *
 *		1 : Enable compilation of giantMod.[ch]
 *		0 : Disable compilation of giantMod.[ch]
 *
 * GI_PRIME_TEST_ENABLE
 *
 *		When nonzero, enables the logic in giantPrime.c used to determine
 *		whether a given giant is prime. Used in RSA key generation.
 *
 *		1 : Enable compilation of giantPrime.[ch]
 *		0 : Disable compilation of giantPrime.[ch]
 *
 * GI_INVERSE_MOD_ENABLE
 *
 *		When nonzero, enables the ginverseMod() function in giantMod.[ch]. Used 
 *		for RSA key generation and for BBS.
 *
 * 		1 : Enable compilation of ginverseMod()
 *		0 : Disable compilation of ginverseMod()
 *
 *
 * Optimization flags
 * ------------------
 *
 * The following symbols have to be #defined to 0 or 1. See the comments below
 * for detailed descritpions of the effects of these flags.
 *
 * GI_GSQUARE_ENABLE
 * GI_OPTIMIZED_MULG
 * GI_ADDG_INLINE
 */
 
/* These define a 16-bit word size emulated on Mac OS X */
#include <stdint.h>

typedef unsigned char	gi_uint8;
typedef uint16_t		gi_uint16;
typedef uint32_t		gi_uint32;
typedef uint16_t		gi_size;
typedef uint16_t		giantDigit;	
typedef uint32_t		giantDouble;		/* two giant digits */

#define GI_GIANT_LOG2_BITS_PER_DIGIT	4

/* 
 * Maximum size in bits of base integer in this package. For BBS, this is the 
 * size of the public 'n' parameter. For asymmetric encryption, it's the
 * size of the base prime. 
 *
 * Altering this has no effect on performance, only on memory usage.
 * Specifically it affects the amount of stack space used (since all 
 * giantIntegers in this package are allocated on the stack). 
 */
#define GI_MAX_PRIME_SIZE_BITS		4096


/****
 **** Optimization flags
 ****/

/* 
 * Sample measurements of the effects of these flags were
 * taken on an 800 MHz G4, with a 16 bit giantDigit, running 
 * the bbsProtocol test with 1024 bit 'n' and 10000 iterations. 
 * Your mileage will certainly vary; the only way to know for 
 * sure which is the best value for a particular optimizatiaon 
 * flag is to measure a real-world case on your platform and then
 * make the size-vs.-speed tradeoff decision yourself. 
 *
 * However, in the meantime, all of the functionality in this 
 * library does work correctly for any value of all of these 
 * flags. 
 */
 
/*
 * GI_GSQUARE_ENABLE
 *
 * Set nonzero to disable optimized grammarSquare(). 
 * Setting nonzero increases code size and increases performance.
 *
 * Setting this true has the following effects (on 800 MHz G4):
 *
 * -- adds 384 bytes of code 
 * -- 7% speedup on private side
 * -- 7% speedup on public side
 */
#define GI_GSQUARE_ENABLE			1

/* 
 * GI_OPTIMIZED_MULG
 *
 * Use old unoptimized, but smaller, mulg if this is zero.
 * Setting nonzero increases code size and increases performance.
 *
 * Setting this true has the following effects (on 800 MHz G4):
 *
 * -- adds 168 bytes of code 
 * -- 20% speedup on private side
 * -- 18% speedup on public side
 */

#define GI_OPTIMIZED_MULG			1

/* 
 * GI_ADDG_INLINE
 *
 * There are two implementations of some core arithmetic routines. 
 * Which one is faster depends highly on your implementation of the
 * routines declared in giantPortCommon.h and implemented in either
 * giantPort_Generic.h (as static inlines) or elsewhere (in C or 
 * possibly assembly language). Performance also depends on your 
 * platform's giantDouble performance. If giantDouble arithmetic 
 * is fast then use GI_ADDG_INLINE. Otherwise try !GI_ADDG_INLINE, 
 * measure the two, and pick the best one. 
 *
 * Setting this false has the following effects (on 800 MHz G4):
 *
 * -- increases code size 192 bytes 
 * -- elliptic operations (SFEE) slow down by 50-100%
 */
#define GI_ADDG_INLINE				1

/* 
 * GI_GIANT_PORT_INLINE
 *
 * true  : we don't have a platform-specific giant arithmetic module
 * false : we do 
 *
 * If GI_ADDG_INLINE is true, setting this true saves a small amount
 * of code with no measurable performance impact. 
 *
 * If GI_ADDG_INLINE is false, setting GI_GIANT_PORT_INLINE to
 * true has the following effects:
 *
 * -- adds 168 bytes of code
 * -- 2% speedup on private side
 * -- 1% speedup on public side
 */
#define GI_GIANT_PORT_INLINE		0

/*
 * GI_INLINE_SMALL_FUNCS
 *
 * When true, uses macros instead of a small number of small functions.
 * This is a tradeff of speed and less stack usage vs. code size.
 */
#define GI_INLINE_SMALL_FUNCS		1

/*
 * GI_MACRO_SMALL_FUNCTIONS
 *
 * Like GI_INLINE_SMALL_FUNCS, except causes small functions to be implemented
 * as macros. For platforms that don't support the 'inline' keyword. 
 */
#define GI_MACRO_SMALL_FUNCTIONS	0

/* 
 * GI_SMALL_CRT_ENABLE
 *
 * This flag involves a tradeoff between code size and stack usage in the 
 * giantMod.c module. Setting this flag true increases code size and
 * decreases stack usage. 
 * 
 * On a G4, the tradeoff is that setting this flag saves 384 bytes of 
 * stack space, at the cost of 1407 extra bytes of code.
 *
 * Note: this flag has no effect if GI_MODG_ENABLE is 0.
 */
#define GI_SMALL_CRT_ENABLE			1


/****
 **** Library implementation flags
 ****/

/* standard library functions available here */
#define GI_HAVE_MEMCPY				1
#define GI_HAVE_BZERO				1

/* enable/disable modular arithmetic */
#define GI_MODG_ENABLE				1

/* enable/disable giantPrime.[ch] - requires GI_MODG_ENABLE! */
#define GI_PRIME_TEST_ENABLE		1

/* enable/disable ginverseMod() */
#define GI_INVERSE_MOD_ENABLE		1

#endif	/* GI_PLATFORM_EMULATE */

#pragma mark --- Declarations of functions with platform-specific implementations ---

/*
 * You should not have to modify the rest of this file.
 */
 
#if		GI_HAVE_MEMCPY || GI_HAVE_BZERO
#include <string.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This platform supports inline keywords.
 */
#define GI_INLINE		inline

#if		GI_HAVE_MEMCPY

#define Memcpy(d, s, l)		memcpy(d, s, l)

#else	/* !GI_HAVE_MEMCPY */

/*
 * memcpy() equivalent
 */
extern void *Memcpy(void *dst, const void *src, gi_uint16 len);

#endif	/* GI_HAVE_MEMCPY */

#if		GI_HAVE_BZERO

#define Bzero(b, l)		bzero(b, l)

#else	/* !GI_HAVE_BZERO */

/*
 * bzero() equivalent
 */
extern void Bzero(void *b, gi_uint16 len);

#endif	/* GI_HAVE_BZERO */

#ifdef __cplusplus
}
#endif

#endif	/*_GIANT_PLATFORM_H_*/
