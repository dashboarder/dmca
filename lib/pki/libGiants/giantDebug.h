/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * giantDebug.h - debugging support.
 *
 * Created Aug. 5 2005 by Doug Mitchell.
 */

#ifndef	_GIANT_DEBUG_H_
#define _GIANT_DEBUG_H_

#include <libGiants/giantPlatform.h>

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef	GIANTS_DEBUG
/* Should be defined in giantPlatform.h */
#error	Please defgine GIANTS_DEBUG.
#endif

#if	GIANTS_DEBUG

#include <assert.h>
#include <debug.h>
#include <stdlib.h>
#include <stdio.h>

#define dbgLog(str)		printf(str)

#define GIASSERT(s)		assert(s)

#else	/* GIANTS_DEBUG */

#define dbgLog(str)

#define GIASSERT(s)	

#endif	/* GIANTS_DEBUG */

#undef	NULL
#define NULL ((void *)0)

#if		GIANTS_DEBUG
void GIRaise(const char *str) __attribute((noreturn));
#else
#define GIRaise(s)
#endif

/* detect overflow og giant->n[] */
#define CHECK_GIANT_OFLOW(g)	GIASSERT((g)->sign <= (g)->capacity)

/* stack analysis support */
#if		GI_STACK_DEBUG

#include <stdio.h>

extern void *gInitSp;
extern void *giLowestSp;

extern void GI_SET_INIT_SP(void);
extern void *GI_GET_SP(void);

static inline void GI_CHECK_SP(const char *fcn)
{
	void *currSp = GI_GET_SP();
	if((giLowestSp == NULL) || (currSp < giLowestSp)) {
		giLowestSp = currSp;
		printf("%s: new min sp %p; stack used %d\n", fcn,
			giLowestSp,
			gInitSp ? (gInitSp - giLowestSp) : 0);
	}
}

/* this does some fine-grain stack logging regardless of giLowestSp */
#if		GI_STACK_LOG
static inline void GI_LOG_SP(const char *fcn)
{
	void *currSp = GI_GET_SP();
	printf("%s: sp %p; stack used %d\n", 
		fcn, currSp, 
		gInitSp ? (gInitSp - currSp) : 0);
}
#else
#define GI_LOG_SP(fcn)
#endif	/* GI_STACK_LOG */
#else	/* GI_STACK_DEBUG */

#define GI_SET_INIT_SP()
#define GI_GET_SP()
#define GI_CHECK_SP(fcn)
#define GI_LOG_SP(fcn)

#endif	/* GI_STACK_DEBUG */

#ifdef	__cplusplus
}
#endif

#endif	/* _GIANT_DEBUG_H_ */
