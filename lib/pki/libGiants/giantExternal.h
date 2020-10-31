/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * giantExternal.h - convert between giant integer and external representation
 *
 * Created Aug. 5 2005 by Doug Mitchell.
 */

#ifndef	_GIANT_EXTERNAL_H_
#define _GIANT_EXTERNAL_H_

#include <libGiants/giantIntegers.h>
#include <libGiants/giantTypes.h>

#ifdef	__cplusplus
extern "C" {
#endif

/* 
 * Max size in bytes needed to serialize a giant of given number of digits. 
 */
#define MAX_SERIALIZED_GIANT_SIZE(digits)	(GIANT_BYTES_PER_DIGIT * digits)

/* 
 * Precalculate actual size in bytes needed to serialize a given giant.
 */
gi_uint16 serializeGiantBytes(const giant g);

/*
 * serialize, deserialize giants's n[] to/from byte stream.
 * First byte of byte stream is the m.s. byte of the resulting giant,
 * regardless of the size of giantDigit. The resulting byte stream
 * is suitable for use as the content of a DER-encoded integer. 
 *
 * No assumption is made about the alignment of cp.
 *
 * The numBytes argument indicates the space available on input;
 * on return it indicates the actual number of bytes written. 
 */
GIReturn serializeGiant(
	const giant g,
	gi_uint8 *cp,
	gi_uint16 *numBytes);		/* IN/OUT */
	
/* 
 * Initialize a giant with specified data, which may well have come from 
 * serializeGiant.
 */
GIReturn deserializeGiant(
	giant g,
	const gi_uint8 *cp,
	gi_uint16 numBytes);

#ifdef	__cplusplus
}
#endif

#endif	/* _GIANT_EXTERNAL_H_ */

