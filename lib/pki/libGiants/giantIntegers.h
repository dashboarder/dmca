/* Copyright (c) 1998-2007 Apple Inc.  All Rights Reserved.
 *
 * NOTICE: USE OF THE MATERIALS ACCOMPANYING THIS NOTICE IS SUBJECT
 * TO THE TERMS OF THE SIGNED "FAST ELLIPTIC ENCRYPTION (FEE) REFERENCE
 * SOURCE CODE EVALUATION AGREEMENT" BETWEEN APPLE COMPUTER, INC. AND THE
 * ORIGINAL LICENSEE THAT OBTAINED THESE MATERIALS FROM APPLE COMPUTER,
 * INC.  ANY USE OF THESE MATERIALS NOT PERMITTED BY SUCH AGREEMENT WILL
 * EXPOSE YOU TO LIABILITY.
 ***************************************************************************
 *
 * giantIntegers.h - large-integer arithmetic library.
 */

#ifndef	_GI_GIANTINTEGERS_H_
#define _GI_GIANTINTEGERS_H_

#include <libGiants/giantTypes.h>
#include <libGiants/giantDebug.h>

#ifdef __cplusplus
extern "C" {
#endif

/* platform-independent digit manipulation macros */

#define GIANT_BYTES_PER_DIGIT	(sizeof(giantDigit))
#define GIANT_BITS_PER_DIGIT	(8 * GIANT_BYTES_PER_DIGIT)
#define GIANT_DIGIT_MASK		((giantDigit)~0)
#define BYTES_TO_GIANT_DIGITS(x)	\
	((x + GIANT_BYTES_PER_DIGIT - 1) / GIANT_BYTES_PER_DIGIT)


/*
 * Static giant support; giantstruct->n[] no longer dynamically allocated, but
 * hard-coded to the max needed for a particular implementation. This max size
 * is a function of the prime size (easy to determine) and the max size of 
 * giant, in digits relative to the size of the prime, actually needed for the
 * math implemented in this library (which is going to be tricky to accurately
 * determine). We want to minimize the static size of a giant in order to 
 * minimize RAM usage. 
 */

#ifndef	GI_MAX_PRIME_SIZE_BITS
#error Please #define GI_MAX_PRIME_SIZE_BITS.
#endif

/* convert that to bytes and digits; these remain fixed */
#define MAX_PRIME_SIZE_BYTES	((GI_MAX_PRIME_SIZE_BITS + 7) / 8)
#define MAX_PRIME_SIZE_DIGITS	BYTES_TO_GIANT_DIGITS(MAX_PRIME_SIZE_BYTES)

/* 
 * The number of digits in a minimal giant, requiring only extra space for 
 * an add. 
 */
#define GIANT_LITTLE_DIGITS				(MAX_PRIME_SIZE_DIGITS + 1)

/* 
 * The number of digits in a max giant, used for mulg and its kin. 
 */
#define GIANT_BIG_DIGITS				(MAX_PRIME_SIZE_DIGITS * 2)

/*
 * The number of digits in a "triple giant", used sparingly in and 
 * around make_recip and modg_via_recip only.
 * The max observed required size seems to be when doing a mulg on 
 * two numbers, both of whose signs are s+1.
 */
#define GIANT_TRIPLE_DIGITS				((MAX_PRIME_SIZE_DIGITS * 2) + 2)

/* Basic giant type with giantDigit array allocated elsewhere */
typedef struct {
	gi_uint16 sign;			/* number of giantDigits */
	gi_uint16 capacity;		/* largest possible number of giantDigits */
	giantDigit *n;			/* n[0] is l.s. digit */
} giantStruct;

typedef giantStruct *giant;

/* 
 * Giant capable of being allocated on the stack, with capacity 
 * no greater than GIANT_LITTLE_DIGITS, i.e., a "little giant" or 
 * lGiant in the SFEE paper.
 *
 * Use base giantstruct g in any function call requiring a giant.
 */
typedef struct {
	giantStruct	g;
	giantDigit	_n_[GIANT_LITTLE_DIGITS];	
} lgiant;

/*
 * "Big giant" "subclass" of standard giant. Use base giantstruct g in 
 * any function call requiring a giant. 
 */
typedef struct {	
	giantStruct	g;
	giantDigit	_n_[GIANT_BIG_DIGITS];
} bigGiantStruct;

typedef bigGiantStruct bgiant;			/* for heap allocation */

/*
 * "Triple giant" "subclass" of standard giant. Used sparingly in and 
 * around make_recip only.

 */
typedef struct {	
	giantStruct	g;
	giantDigit	_n_[GIANT_TRIPLE_DIGITS];
} tripleGiantStruct;

typedef tripleGiantStruct tgiant;	

gi_uint16 bitlen(giant n);					/* Returns the bit-length n;
											 * e.g. n=7 returns 3. */
giantDigit bitval(giant n, gi_uint16 pos);
gi_uint16 giantNumBytes(giant n);
void gtog(giant src, giant dest);			/* Copies one giant to another */
void int_to_giant(gi_uint32 n, giant g);  	/* Gives a giant an int value */
GIReturn giant_to_int(giant g, gi_uint32 *i); /* Convent giant to int. */
gi_uint16 numtrailzeros(giant g);			/* # of trailing zero bits in g */
int gcompg(giant a, giant b);				/* Returns 1, 0, -1 as a>b, a=b, a<b */
void normal_addg(giant a, giant b);			/* b += a */
#define addg(a, b)	normal_addg(a, b)

void iaddg(giantDigit a, giant b);			/* b += a */
void imulg(giantDigit a, giant b);			/* b *= a */
void normal_subg(giant a, giant b);			/* b -= a. */
void gshiftleft(gi_uint16 bits, giant g);	/* Shift g left by bits, introducing
											 * zeros on the right. */
void gshiftright(gi_uint16 bits, giant g);	/* Shift g right by bits, losing bits
											 * on the right. */
// Right-shift z by s giantDigits 
void gshifltwordsright(gi_uint16 digits, giant g);
// shift left by s giantDigits 
void gshiftwordsleft(gi_uint16 digits, giant g);

void extractbits(gi_uint16 n, giant src, giant dest);
											/* dest becomes lowermost n bits of
											 * src.  Equivalent to
											 * dest = src % 2^n */

void grammarSquare(giant a);				/* g *= g. */
void grammarSquare_common(giant a, giant dest);
											/* dest := (a * a)
											 * dest is typically a bgiant */

#define gsquare(g) grammarSquare(g)

void mulg(giant a, giant b);				/* b *= a. */
void mulg_common(giant a, giant	b, giant dest);
											/* dest := a * b;
											 * dest is typically a bgiant */
void gtrimSign(giant g);					/* Adjust sign for possible leading
											 * (m.s.) zero digits */

void clearGiant(giant g);					/* zero a giant's data */

/* converted from trivial functions */

#if		GIANTS_DEBUG
/*
 * counter, acccumulates calls to (giantDigit x giantDigit) multiply. 
 */
extern unsigned numGiantDigitMuls;
#define INCR_DIGIT_MULS		numGiantDigitMuls++

/* 
 * Accumulate calls to mulg_common() 
 */
extern unsigned numMulgCommon;
#define INCR_NUM_MULGS		numMulgCommon++;

/* 
 * Accumulate calls to grammarSquare_common() 
 */
extern unsigned numGSquareCommon;
#define INCR_NUM_GSQUARE	numGSquareCommon++;


#else	/* GIANTS_DEBUG */

#define INCR_DIGIT_MULS
#define INCR_NUM_MULGS
#define INCR_NUM_GSQUARE

#endif	/* GIANTS_DEBUG */

/*
 * Functions which can be explicit C functions, macros, or static inlines
 * depending on the configuration. 
 */
#if		!defined(GI_INLINE_SMALL_FUNCS) && !defined(GI_MACRO_SMALL_FUNCTIONS)
#error	Please define GI_INLINE_SMALL_FUNCS and GI_MACRO_SMALL_FUNCTIONS.
#endif

#if		GI_INLINE_SMALL_FUNCS && GI_MACRO_SMALL_FUNCTIONS
#error	GI_INLINE_SMALL_FUNCS and GI_MACRO_SMALL_FUNCTIONS are mutually exclusive. 
#endif

#if		GI_INLINE_SMALL_FUNCS

/* common giant init */
static GI_INLINE void initGiant(
	giant g, 
	gi_uint16 capacity,
	giantDigit *digits)
{
	g->sign = 0;
	g->capacity = capacity;
	g->n = digits;
}

/* call this after declaring a local lgiant and before using it */
static GI_INLINE void localGiantAlloc(lgiant *g)
{
	initGiant(&g->g, GIANT_LITTLE_DIGITS, g->_n_);
}

/* ditto, for a bgiant */
static GI_INLINE void localBigGiantAlloc(bgiant *g)
{
	initGiant(&g->g, GIANT_BIG_DIGITS, g->_n_);
}

/* ditto, for a tgiant */
static GI_INLINE void localTriGiantAlloc(tgiant *g)
{
	initGiant(&g->g, GIANT_TRIPLE_DIGITS, g->_n_);
}

/* Returns whether g is 1 */
static GI_INLINE int isone(giant g)
{
	return ((g->sign == 1) && (g->n[0] == 1));
}

/* Returns whether g is zero */
static GI_INLINE int isZero(giant g)
{
	/* we're skipping the !trimmed check here... */
	return (g->sign == 0);
}
			
#elif	GI_MACRO_SMALL_FUNCTIONS

#define initGiant(gnt, cap, digs)		\
	(gnt)->sign = 0;					\
	(gnt)->capacity = cap;				\
	(gnt)->n = digs;

#define localGiantAlloc(lg)		initGiant(&((lg)->g), GIANT_LITTLE_DIGITS, (lg)->_n_)
#define localBigGiantAlloc(bg)	initGiant(&((bg)->g), GIANT_BIG_DIGITS,    (bg)->_n_)
#define localTriGiantAlloc(tg)	initGiant(&((tg)->g), GIANT_TRIPLE_DIGITS, (tg)->_n_)

#define isone(g)	(((g)->sign == 1) && ((g)->n[0] == 1))
#define isZero(g)	((g)->sign == 0)

#else

/*** regular C functions ***/

/* call this after declaring a local lgiant and before using it */
extern void localGiantAlloc(lgiant *g);

/* ditto, for a bgiant */
extern void localBigGiantAlloc(bgiant *g);

/* ditto, for a tgiant */
extern void localTriGiantAlloc(tgiant *g);

void initGiant(
	giant g, 
	gi_uint16 capacity,
	giantDigit *digits);

int isone(giant g);							/* Returns whether g is 1 */
int isZero(giant g);						/* Returns whether g is zero */

#endif	/* INLINE / MACRO */

#ifdef __cplusplus
}
#endif

#endif	/* _GI_GIANTINTEGERS_H_ */
