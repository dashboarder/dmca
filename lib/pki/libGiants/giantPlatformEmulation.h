/* Copyright (c) 2004,2006-2007 Apple Inc.  All Rights Reserved.
 *
 * NOTICE: USE OF THE MATERIALS ACCOMPANYING THIS NOTICE IS SUBJECT
 * TO THE TERMS OF THE SIGNED "FAST ELLIPTIC ENCRYPTION (FEE) REFERENCE
 * SOURCE CODE EVALUATION AGREEMENT" BETWEEN APPLE COMPUTER, INC. AND THE
 * ORIGINAL LICENSEE THAT OBTAINED THESE MATERIALS FROM APPLE COMPUTER,
 * INC.  ANY USE OF THESE MATERIALS NOT PERMITTED BY SUCH AGREEMENT WILL
 * EXPOSE YOU TO LIABILITY.
 ***************************************************************************
 *
 * giantPlatformEmulation.h - #defines for platform emulation.
 *	
 * This file is NOT used in production code. 
 */

#ifndef	_GIANT_PLATFORM_EMULATION_H_
#define _GIANT_PLATFORM_EMULATION_H_

#define NDEBUG
#ifdef	NDEBUG
#define GIANTS_DEBUG	0
#else
#define GIANTS_DEBUG	1
#endif

/*
 * Emulation targets:
 *
 * #define _FEE_CUSTOM_XXX_CONFIG_ to 1 and fill in these blanks if
 * you want to generate a test configuration which is not strictly related
 * to the machine you're building for - e.g., build a 16-bit giant package on PPC.
 */

/* this one for emulating 16-bit CPU on Mac OS X / PPC */
#define _FEE_CUSTOM_16_CONFIG_		1
/* this one for emulating 8-bit CPU on Mac OS X / PPC */
#define _FEE_CUSTOM_8_CONFIG_		0

#if		_FEE_CUSTOM_16_CONFIG_

/* 16-bit word size emulated on Mac OS X */
#include <stdint.h>

typedef unsigned char	gi_uint8;
typedef uint16_t		gi_uint16;
typedef uint32_t		gi_uint32;
typedef uint16_t		gi_size;
typedef uint16_t		giantDigit;	
typedef uint32_t		giantDouble;		/* two giant digits */

#define GI_GIANT_LOG2_BITS_PER_DIGIT	4

#elif		_FEE_CUSTOM_8_CONFIG_

/* 8-bit word size emulated on Mac OS X */
#include <stdint.h>

typedef unsigned char	gi_uint8;
typedef uint16_t		gi_uint16;
typedef uint32_t		gi_uint32;
typedef uint16_t		gi_size;
typedef uint8_t			giantDigit;	
typedef uint16_t		giantDouble;		/* two giant digits */

#define GI_GIANT_LOG2_BITS_PER_DIGIT	3

#elif	defined(__APPLE__) && defined(__ppc__)

/* 
 * Mac OS X on 32-bit PPC
 */
#include <stdint.h>

typedef unsigned char	gi_uint8;
typedef uint16_t		gi_uint16;
typedef uint32_t		gi_uint32;
typedef uint32_t		gi_size;
typedef uint32_t		giantDigit;	
typedef uint64_t		giantDouble;		/* two giant digits */

#define GI_GIANT_LOG2_BITS_PER_DIGIT	5

#elif	defined(__APPLE__) && defined(__i386__)

/* 
 * Mac OS X on 32-bit Pentium
 */
#include <stdint.h>

typedef unsigned char	gi_uint8;
typedef uint16_t		gi_uint16;
typedef uint32_t		gi_uint32;
typedef uint32_t		gi_size;
typedef uint32_t		giantDigit;	
typedef uint64_t		giantDouble;		/* two giant digits */

#define GI_GIANT_LOG2_BITS_PER_DIGIT	5

#else
/* 
 * Other target goes here. 
 */
#error Platform dependent work needed
#endif

/* 
 * Maximum size in bits of base integer in this package. For BBS, this is the 
 * size of the public 'n' parameter. 
 */
#define GI_MAX_PRIME_SIZE_BITS		4096

/****
 **** Optimization flags
 ****/

/* 
 * There are two implementations of some core arithmetic routines. 
 * Which one is faster depends highly on your implementation of the
 * routines declared in giantPortCommon.h and implemented in either
 * giantPort_Generic.h (as static inlines) or elsewhere (in C or 
 * possibly assembly language). Performance also depends on your 
 * platform's giantDouble performance. If giantDouble arithmetic 
 * is fast then use GI_ADDG_INLINE. Otherwise try !GI_ADDG_INLINE, 
 * measure the two, and pick the best one. 
 */
#define GI_ADDG_INLINE				1

/* 
 * true  : we don't have a platform-specific giant arithmetic module
 * false : we do 
 */
#define GI_GIANT_PORT_INLINE		1

/*
 * GI_INLINE_SMALL_FUNCS
 *
 * When true, uses inlines instead of a small number of small functions.
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

/* enable/disable giantPrime.[ch] */
#define GI_PRIME_TEST_ENABLE		1

/* enable/disable ginverseMod() */
#define GI_INVERSE_MOD_ENABLE		1

#endif	/* _GIANT_PLATFORM_EMULATION_H_ */
