/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * giantDebug.c - debugging support
 *
 * Created Aug. 5 2005 by Doug Mitchell.
 */

#include <libGiants/giantDebug.h>
#include <libGiants/giantTypes.h>

#if		GIANTS_DEBUG

void GIRaise(const char *str)
{
	printf("Fatal libGiants library error: %s\n", str);
	exit(1);
}


#endif	/* GIANTS_DEBUG */

#if		GI_STACK_DEBUG

void *gInitSp;
void *giLowestSp;

void GI_SET_INIT_SP(void)
{
	gInitSp = GI_GET_SP();
	giLowestSp = NULL;
}

#ifdef	__ppc__


void *GI_GET_SP(void)
{
	void *currsp;
	asm("mr %0,r1" : "=r"(currsp) : );
	return currsp;
}

#else
#ifdef	__i386__
void *GI_GET_SP(void)
{
	register void *sp __asm__("esp");
	return sp;
}
#else
#ifdef  __arm__
void *GI_GET_SP(void)
{
        register void *sp __asm__("r13");
        return sp;
}
#else
#error Need a platform-dependent GI_GET_SP()
#endif
#endif
#endif
#endif	/* GI_STACK_DEBUG */
