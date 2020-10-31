/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * rsaGiantKey.c 
 *
 * Created Aug. 9 2005 by Doug Mitchell.
 */

#include <libgRSA/rsaGiantKey.h>
#include <libGiants/giantIntegers.h>
#include <libGiants/giantDebug.h>

/* call this after declaring a local lgiant and before using it */
void localSmallGiantAlloc(rsaSgiant *g)
{
	initGiant(&g->g, RSA_SMALL_GIANT_DIGITS, g->_n_);
}

/* clear an RSA{Pub,Priv}GiantKey */
void rsaClearPubGKey(
	RSAPubGiantKey	*gKey)
{
	Bzero(gKey, sizeof(*gKey));
}

void rsaClearPrivGKey(
	RSAPrivGiantKey	*gKey)
{
	Bzero(gKey, sizeof(*gKey));
}

void rsaClearFullGiantKey(
	RSAFullGiantKey		*gKey)
{
	Bzero(gKey, sizeof(*gKey));
}


