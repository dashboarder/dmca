/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * giantTypes.h - Standard typedefs
 *
 * Created Aug. 5 2005 by Doug Mitchell.
 */

#ifndef	_GIANT_TYPES_H_
#define _GIANT_TYPES_H_

#include <libGiants/giantPlatform.h>

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Standard return codes.
 */
typedef enum {
	GR_Success = 0,
	GR_IllegalArg,			/* illegal argument */
	GR_Unimplemented,		/* unimplemented function */
	GR_Internal,			/* internal library error */
	GR_Overflow,			/* caller error, buffer/giant overflow */
} GIReturn;
 
/*
 * We administer the namespace for other libraries' return codes here, so
 * all can share a common space. 
 */
#define SFEE_RETURN_BASE		1000	/* libSFEE */
#define RSA_RETURN_BASE			2000	/* libgRSA */
#define LIBCERT_RETURN_BASE		3000	/* libCert */
#define DH_RETURN_BASE			4000	/* libgDH */

#ifdef	__cplusplus
}
#endif

#endif	/* _GIANT_TYPES_H_ */
