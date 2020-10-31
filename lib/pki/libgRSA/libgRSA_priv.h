/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * libgRSA_priv.h - private libRSA routines.
 *
 * Created Aug. 10 2005 by Doug Mitchell.
 */

#ifndef	_LIBGRSA_RSA_PRIV_H_
#define _LIBGRSA_RSA_PRIV_H_

#include <libgRSA/libgRSA.h>
#include <libGiants/giantIntegers.h>
#include <libDER/libDER.h>

#ifdef __cplusplus
extern "C" {
#endif

/* make sure NDEBUG is #define for deployment build! */
#define NDEBUG
#ifdef	NDEBUG

#define rsaDebug(s)
#define rsaDebug1(s, a)
#define rsaDebug2(s, a, b)
#define RSAASSERT(s)

#else	/* !NDEBUG */

#include <stdio.h>
#include <assert.h>

#define rsaDebug(s)			printf(s)
#define rsaDebug1(s, a)		printf(s, a)
#define rsaDebug2(s, a, b)	printf(s, a, b)

#define RSAASSERT(s)		assert(s)


#endif	/* !NDEBUG */

/* convert a libGiants GIReturn to our own RSAStatus */
extern RSAStatus giantStatusToRSA(GIReturn grtn);

/* convert a libDER DERReturn to our own RSAStatus */
extern RSAStatus derStatusToRSA(DERReturn drtn);

/* convert an RSAStatus to a string (disabled for NDEBUG builds) */
extern const char *rsaStatusStr(RSAStatus rrtn);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBGRSA_RSA_PRIV_H_ */
