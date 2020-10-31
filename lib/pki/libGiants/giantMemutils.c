/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * giantMemutils.c - trivial implementation of memcpy and bzero, for platforms 
 *				   without libc versions of those functions.
 *
 * Created Aug. 8 2005 by Doug Mitchell.
 */
 
#include <libGiants/giantPlatform.h>

#ifndef	GI_HAVE_MEMCPY
#error Please define GI_HAVE_MEMCPY.
#endif

#ifndef	GI_HAVE_BZERO
#error Please define GI_HAVE_BZERO.
#endif

#if !GI_HAVE_MEMCPY

void *Memcpy(void *dst, const void *src, gi_uint16 len)
{
	gi_size dex;
	unsigned char *dstp = (unsigned char *)dst;
	const unsigned char *srcp = (const unsigned char *)src;
	
	for(dex=0; dex<len; dex++) {
		*dstp++ = *srcp++;
	}
	return dst;
}

#endif

#if !GI_HAVE_BZERO

void Bzero(void *b, gi_uint16 len)
{
	gi_size dex;
	unsigned char *dstp = (unsigned char *)b;
	
	for(dex=0; dex<len; dex++) {
		*dstp++ = 0;
	}
}

#endif
