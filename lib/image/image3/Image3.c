/*
 * Copyright (C) 2007-2011, 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include "Image3.h"

/* KERNEL is defined when building the AppleMobileFileIntegrityKext */
#ifdef KERNEL

#include <machine/limits.h>
#include <string.h>

#else /* !KERNEL */

#ifdef IMAGE3_DEBUG
#include <fcntl.h>
#endif
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#endif /* !KERNEL */

#include <sys/errno.h>

#ifndef __linux__
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#if (BYTE_ORDER == LITTLE_ENDIAN)
# define HTOLE(x)	(x)
# define LETOH(x)	(x)
# define HTOLELL(x)	(x)
# define LETOHLL(x)	(x)
#else
# include <libkern/OSByteOrder.h>
# define HTOLE(x)	OSSwapHostToLittleInt32(x)
# define LETOH(x)	OSSwapLittleToHostInt32(x)
# define HTOLELL(x)	OSSwapHostToLittleInt64(x)
# define LETOHLL(x)	OSSwapLittleToHostInt64(x)
#endif

#ifndef offsetof
#if defined(__GNUC__)
#define offsetof(t, d) __builtin_offsetof(t, d)
#else
#error need offsetof definition
#endif
#endif

#ifdef IMAGE3_DEBUG
# ifdef KERNEL
#  include <libkern/libkern.h>
# else
#  include <stdio.h>
# endif
# define debug(fmt, args...)	printf("I3:%s: " fmt "\n", __FUNCTION__ , ##args)
# define UNTAG(x)		(((x) >> 24) & 0xff),(((x) >> 16) & 0xff),(((x) >> 8) & 0xff),((x) & 0xff)
#else
# define debug(fmt, args...)
#endif

#include "Image3Format.h"

#define IMAGE3_HASH_SIZE	CCSHA1_OUTPUT_SIZE	/* SHA-1 hash size */

/*
 * Internal handle state
 */
typedef struct _Image3InternalState {
	Image3ObjectHeader		*image;
	u_int32_t			flags;
# define kImage3ImageWasInstantiated	(1<<0)
# define kImage3ImageWasValidated	(1<<1)	/* validation has been performed */
# define kImage3ImageIsTrusted		(1<<2)	/* validation indicates image is trusted */
# define kImage3ImageWasCreated		(1<<16)
# define kImage3ImageIsSigned		(1<<17)	/* signature has been appended */
# define kImage3ImageWasAllocated	(1<<18)
	size_t				allocSize;

#ifdef IMAGE3_CREATOR
	int				cursor;
	int				lastTag;
#endif

	struct _Image3InternalState	*nestedImage;
} Image3InternalState;


/*
 * Discards an Image3 working object.
 */
extern void
image3Discard(
	Image3ObjectHandle *withHandle)
{
	if (withHandle && *withHandle) {
		if ((*withHandle)->image && ((*withHandle)->flags & kImage3ImageWasAllocated)) {
			debug("discarding image %p", (*withHandle)->image);
			image3Free((*withHandle)->image, (*withHandle)->allocSize);
		}
		debug("discarding handle %p", *withHandle);
		image3Free(*withHandle, sizeof(Image3InternalState));
		*withHandle = NULL;
	}
}

#ifdef IMAGE3_CREATOR

/*
 * Create a new Image3 working object.
 *
 * Returns:
 * 0		Success
 * ENOMEM	Unable to allocate memory
 */
int
image3InstantiateNew(
	Image3ObjectHandle *newHandle,
	size_t initialAllocation,
	u_int32_t imageType)
{
	Image3InternalState	*state;
	int			result;

	if ((state = image3Malloc(sizeof(*state))) == NULL)
		return(ENOMEM);

	if (initialAllocation < sizeof(Image3ObjectHeader))
		initialAllocation = sizeof(Image3ObjectHeader);

	if ((state->image = image3Malloc(initialAllocation)) == NULL) {
		image3Free(state, sizeof(*state));
		return(ENOMEM);
	}
	debug("allocated handle %p and initial buffer %p of %u bytes", state, state->image, (unsigned int)initialAllocation);
	
	memset(state->image, 0, sizeof(*state->image));
	state->image->ihMagic = HTOLE(kImage3HeaderMagic);
	state->image->ihType =  HTOLE(imageType);
	
	state->allocSize = initialAllocation;
	state->cursor = 0;
	state->lastTag = -1;
	state->flags = kImage3ImageWasAllocated;
	state->nestedImage = NULL;

	/*
	 * Because of the issue described in rdar://problem/6705064, we need to stop embedding a TYPE tag within leaf certificates.
	 * (Essentially, the TYPE tag in the embbeded cert has precendence over the TYPE tag in the parent image, causing the
	 * parent image to assume the type of embdded image; i.e., the parent image type "becomes" 'cert'.)  As a quick fix, we detect
	 * whether this new instantiation is for an embedded image by seeing if the requested imageType is for 'cert'. This
	 * saves plumbing in a new paramter/flag from the caller for now.	 
	 */
	if ((imageType != IMAGE_TYPE_EMBEDCERT) && ((result = image3SetTagUnsignedNumber(state,  kImage3TagTypeType, imageType)) != 0)) {
		image3Free(state->image, initialAllocation);
		image3Free(state, sizeof(*state));
		return(result);
	}

	*newHandle = state;

	return(0);
}
	
/*
 * Finalises an image.
 *
 * If signImage is set, hashes and signs the image.  Reservation length, if non-zero, indicates that the final signed image
 * will include tags not currently present in the image.  A partial digest is computed, but the size of the reservation is
 * required so that ihSignedLength can be set properly for inclusion in the partial hash state.  The partial digest is
 * constrained to conclude on a 64 byte boundary by adding the appropriate padding following the last tag in the signed portion
 * of a finalised image.
 *
 * Returns:
 * 0		Success
 */
int
image3Finalize(
	const Image3ObjectHandle withHandle,
	void **objectBuffer,
	size_t *objectSize,
	bool signImage,
	size_t reservationSize)
{
	Image3ObjectHeader	*hdr;
	Image3PartialHash	partialHash;	
	char			normalHash[IMAGE3_HASH_SIZE];
	void			*hashBuffer = NULL;
	size_t			hashBufferSize;	
	size_t			hashSize;
	int				result;
	void			*signedHash;
	size_t			signedHashSize;
	void			*certChain;
	size_t			certChainSize;
	size_t			paddingSize;
	bool			createPartialHash = reservationSize != 0;

	debug("finalising %p", withHandle);
	hdr = withHandle->image;

	if (signImage) {
		/* pad the image as required to reach a partial hash boundary of 64 bytes */
		paddingSize = (64 - ((sizeof(*hdr) - offsetof(Image3ObjectHeader, ihSignedLength) + withHandle->cursor) % 64)) % 64;
		if ((result = image3AdvanceCursorWithZeroPad(
				withHandle,
				paddingSize)) != 0) {
			debug("failed to pad image to partial hashing boundary");
			return(result);
		}
	
		/* update header pointer as the padding above may have reallocated it */
		hdr = withHandle->image;

		/* sign up to this point, remembering that eventual signed length will cover the reserved area */
		hdr->ihSignedLength = HTOLE(withHandle->cursor + reservationSize);
		debug("will sign %u bytes for image %p", LETOH(hdr->ihSignedLength), withHandle);

		/* generate hash */
		hashSize = sizeof(*hdr) - offsetof(Image3ObjectHeader, ihSignedLength) + LETOH(hdr->ihSignedLength) - reservationSize;
		if (createPartialHash) {
			partialHash.masteredReservationLength = HTOLE(reservationSize);
			partialHash.masteredSignedLength = HTOLE(LETOH(hdr->ihSignedLength) - reservationSize);
			
			hashBuffer = &partialHash;
			hashBufferSize = sizeof(Image3PartialHash);
			image3SHA1Partial(&hdr->ihSignedLength, hashSize, partialHash.sha1_state);
		}
		else {
			hashBuffer = normalHash;
			hashBufferSize = IMAGE3_HASH_SIZE;
		image3SHA1Generate(&hdr->ihSignedLength, hashSize, hashBuffer);
		}
		
		/*
		 * sign hash & get cert chain; note well that this is the code path for all cases that update the signed length whether
		 * or not a signature is applied.  When computing hashes, for example, execution passes into image3PKISignHash and then
		 * terminates before returning.  Also, in cases supporting authorized installations, the image is finalized, but no actual
		 * hash or cert tags are appended.
		*/
		result = image3PKISignHash(
			hashBuffer,
			hashBufferSize,
			&signedHash,
			&signedHashSize,
			&certChain,
			&certChainSize);
		if (result) {
			debug("hash signing failed");
			return(result);
		}
		debug("signed hash size %u @ %p", (unsigned int)signedHashSize, signedHash);

		/* append tags containing signed hash and cert chain if provided */
		if (signedHashSize) {
			if ((result = image3SetTagStructure(
					withHandle,
					kImage3TagTypeSignedHash,
					signedHash,
					signedHashSize,
					0)) != 0) {
				debug("failed to append signed hash tag");
				return(result);
			}
		}
		if (certChainSize) {
			if ((result = image3SetTagStructure(
					withHandle,
					kImage3TagTypeCertificateChain,
					certChain,
					certChainSize,
					0)) != 0) {
				debug("failed to append cert chain tag");
				return(result);
			}
		}

		/* mark the image as signed and thus immutable */
		withHandle->flags |= kImage3ImageIsSigned;

		/* update header pointer as the tags above may have reallocated it */
		hdr = withHandle->image;
	}		
		
	/* fill out the remaining header fields */
	hdr->ihBufferLength = HTOLE(withHandle->cursor);
	hdr->ihSkipDistance = HTOLE(sizeof(*hdr) + withHandle->cursor);

	/* hand back the buffer and its final size */
	*objectBuffer = hdr;
	*objectSize = LETOH(hdr->ihSkipDistance);

	return(0);
}

/*
 * Set a numeric tag.
 *
 * Returns:
 * 0		Success
 * ENOMEM	Unable to allocate memory
 */
int
image3SetTagSignedNumber(
	const Image3ObjectHandle withHandle,
	u_int32_t withTag,
	int64_t withValue)
{
	Image3TagNumber32	num32;
	Image3TagNumber64	num64;
	void			*num;
	int64_t			tmp;
	size_t			size;

	debug("setting tag %c%c%c%c to %lld", UNTAG(withTag), withValue);

	tmp = (withValue > 0) ? -withValue : withValue;

	if ((tmp >> 32) >= -1) {
		size = sizeof(num32);
		num32.value.s32 = HTOLE(withValue);
		num = (void *)&num32;
	} else {
		size = sizeof(num64);
		num64.value.s64 = HTOLELL(withValue);
		num = (void *)&num64;
	}

	return(image3SetTagStructure(withHandle, withTag, num, sizeof(num), 0));
}
	
int
image3SetTagUnsignedNumber(
	const Image3ObjectHandle withHandle,
	u_int32_t withTag,
	u_int64_t withValue)
{
	Image3TagNumber32	num32;
	Image3TagNumber64	num64;
	void			*num;
	size_t			size;

	debug("setting tag %c%c%c%c to %llu", UNTAG(withTag), withValue);

	if ((withValue >> 32) == 0) {
		size = sizeof(num32);
		num32.value.u32 = HTOLE(withValue);
		num = (void *)&num32;
	} else {
		size = sizeof(num64);
		num64.value.u64 = HTOLELL(withValue);
		num = (void *)&num64;
	}

	return(image3SetTagStructure(withHandle, withTag, num, size, 0));
}
	
/*
 * Set a string tag.
 *
 * Returns:
 * 0		Success
 * ENOMEM	Unable to allocate memory
 */
int
image3SetTagString(
	const Image3ObjectHandle withHandle,
	u_int32_t withTag,
	char *withValue)
{
	Image3TagString	*str;
	size_t		len;
	int		result;

	debug("setting tag %c%c%c%c to '%s'", UNTAG(withTag), withValue);
	len = strlen(withValue);

	/* allocate the structure with room for the string */
	if ((str = (Image3TagString *)image3Malloc(len + sizeof(*str))) == NULL)
		return(ENOMEM);

	str->stringLength = len;
	memcpy(str->stringBytes, withValue, len);

	result = image3SetTagStructure(withHandle, withTag, (void *)str, len + sizeof(*str), 0);

	image3Free(str, len + sizeof(*str));
	return(result);
}	

/*
 * Set a structure tag.
 *
 * Returns:
 * 0		Success
 * ENOMEM	Unable to allocate memory
 * EROFS	Image is read-only
 */
int
image3SetTagStructure(
	const Image3ObjectHandle withHandle,
	u_int32_t withTag,
	const void *withValue,
	size_t withSize,
	int withAlignment)
{
	Image3ObjectHeader	*newHeader;
	size_t			needAlloc;
	Image3TagHeader		*tagHeader;
	u_int32_t		dataCursor;
	size_t			remainderSize;

	debug("setting structure tag %c%c%c%c of %u bytes aligned %d for image %p cursor %u",
	    UNTAG(withTag), (unsigned int)withSize, withAlignment, withHandle, withHandle->cursor);
	
	/* once an image is signed, it's immutable */
	if (withHandle->flags & kImage3ImageIsSigned)
		return(EROFS);

	/*
	 * Align the cursor as required.  Note that this is fairly hairy, as we have to take the
	 * image header into account as well, since alignment is relative to the base of the
	 * image...
	 */
	if (withAlignment) {
		/* forward-align the cursor */
		dataCursor = withHandle->cursor + sizeof(*tagHeader) + sizeof(Image3ObjectHeader);
		dataCursor += (withAlignment - (withHandle->cursor % withAlignment)) % withAlignment;
		withHandle->cursor = dataCursor - sizeof(*tagHeader) - sizeof(Image3ObjectHeader);

		/* fix up the skip distance from the previous tag */
		if (withHandle->lastTag != -1) {
			tagHeader = (Image3TagHeader *)&withHandle->image->ihBuffer[withHandle->lastTag];
			tagHeader->itSkipDistance = HTOLE(withHandle->cursor - withHandle->lastTag);
		}
		debug("doing aligned insertion with tag at %u and data at %u", withHandle->cursor, dataCursor);
	}

	/* calculate remainder, we want to pad payload size to a multiple of 16 to facilitate AES */
	if ((withSize % 16) == 0) {
		remainderSize = 0;
	}
	else {
		remainderSize = 16 - (withSize % 16);
	}
	debug("remainder size is 0x%x", remainderSize, withTag);

	/* check to see whether there's room in the buffer and re-allocate to suit */
	needAlloc = sizeof(*newHeader) + withHandle->cursor + withSize + remainderSize + sizeof(*tagHeader);
	if (needAlloc > withHandle->allocSize) {
		debug("need allocation of %u but current allocation only good for %u",
		    (unsigned int)needAlloc, (unsigned int)withHandle->allocSize);
		if ((newHeader = realloc(withHandle->image, needAlloc)) == NULL) {
			debug("failed to reallocate buffer to add tag");
			return(ENOMEM);
		}
		debug("image buffer moved %p -> %p", withHandle->image, newHeader);
		debug("oldsize was 0x%x, newsize is 0x%x, newsize - oldsize is 0x%x",
				withHandle->allocSize, needAlloc, needAlloc - withHandle->allocSize);
		withHandle->image = newHeader;
		withHandle->allocSize = needAlloc;
	}

	/* populate the new tag. itBufferLength is intentionally short by remainderSize. */
	tagHeader = (Image3TagHeader *)&withHandle->image->ihBuffer[withHandle->cursor];
	tagHeader->itTag = HTOLE(withTag);
	tagHeader->itBufferLength = HTOLE(withSize);

	/* pad payload size to a multiple of 16 to facilitate AES */
	tagHeader->itSkipDistance = HTOLE(withSize + remainderSize + sizeof(*tagHeader));

	/* and pad skip distance to a multiple of 4 to keep headers aligned */
	if (LETOH(tagHeader->itSkipDistance) & 3) {
		debug("padding skip distance %u to %u",
		    LETOH(tagHeader->itSkipDistance),
		    LETOH(tagHeader->itSkipDistance) + 4 - (LETOH(tagHeader->itSkipDistance) & 3));
		tagHeader->itSkipDistance = HTOLE(LETOH(tagHeader->itSkipDistance) + 4 - (LETOH(tagHeader->itSkipDistance) & 3));
	}
		

	debug("copying %u bytes of data %p -> %p", (unsigned int)withSize, withValue, tagHeader->itBuffer);
	memcpy(tagHeader->itBuffer, withValue, withSize);
	debug("padding with %u bytes of zeroes at %p", (unsigned int)remainderSize, tagHeader->itBuffer, tagHeader->itBuffer + withSize);
	memset(tagHeader->itBuffer + withSize, 0, remainderSize);

	/* move the cursor */
	withHandle->lastTag = withHandle->cursor;
	withHandle->cursor += LETOH(tagHeader->itSkipDistance);

	debug("new cursor is %u", withHandle->cursor);

	/* update this so that _image3PrintImage works */
	withHandle->image->ihBufferLength = HTOLE(withHandle->cursor);

	return(0);
}

/*
 * Advances the cursor with zero padding.
 *
 * Returns:
 * 0		Success
 * ENOMEM	Unable to allocate memory
 * EROFS	Image is read-only
 */
int
image3AdvanceCursorWithZeroPad(
					  const Image3ObjectHandle withHandle,
					  int withSize)
{
	Image3ObjectHeader	*newHeader;
	size_t			needAlloc;
	Image3TagHeader		*tagHeader;
	size_t			previousCursor;
	
	debug("advancing cursor by %u bytes for image %p cursor %u",
		  (unsigned int)withSize, withHandle, withHandle->cursor);
	
	/* once an image is signed, it's immutable */
	if (withHandle->flags & kImage3ImageIsSigned)
		return(EROFS);

	/* advance the cursor */
	previousCursor = withHandle->cursor;
	withHandle->cursor += withSize;
	debug("new cursor is %u", withHandle->cursor);
		
	/* fix up the skip distance from the previous tag */
	if (withHandle->lastTag != -1) {
		tagHeader = (Image3TagHeader *)&withHandle->image->ihBuffer[withHandle->lastTag];
		tagHeader->itSkipDistance = HTOLE(withHandle->cursor - withHandle->lastTag);
	}
	
	
	/* check to see whether there's room in the buffer and re-allocate to suit */
	needAlloc = sizeof(*newHeader) + withHandle->cursor;
	if (needAlloc > withHandle->allocSize) {
		debug("need allocation of %u but current allocation only good for %u",
			  (unsigned int)needAlloc, (unsigned int)withHandle->allocSize);
		if ((newHeader = realloc(withHandle->image, needAlloc)) == NULL) {
			debug("failed to reallocate buffer to add tag");
			return(ENOMEM);
		}
		debug("image buffer moved %p -> %p", withHandle->image, newHeader);
		withHandle->image = newHeader;
		withHandle->allocSize = needAlloc;
	}
	
	/* zero pad */
	bzero(&withHandle->image->ihBuffer[previousCursor], withHandle->cursor - previousCursor);
	
	/* update this so that _image3PrintImage works */
	withHandle->image->ihBufferLength = HTOLE(withHandle->cursor);
	
	return(0);
}

#endif /* IMAGE3_CREATOR */

/*
 * Take a data buffer that might contain an Image3 object and get a handle to it.
 *
 * Note that no security validation is performed by this operation.
 *
 * Returns:
 * 0		Success
 * ENOMEM	Could not allocate handle or buffer
 * EINVAL	The contents of the buffer are malformed
 */
int
image3InstantiateFromBuffer(
	Image3ObjectHandle *newHandle,
	const void *fromBuffer,
	const size_t bufferSize,
	bool copyBuffer)
{
	Image3ObjectHeader		*hdr;
	Image3InternalState		*state;

	debug("instantiating from buffer %p size %u", fromBuffer, (unsigned int)bufferSize);

	/* assume we have a header and do some basic sanity checks */
	hdr = (Image3ObjectHeader *)fromBuffer;
	if (bufferSize < sizeof(*hdr)) {
		debug("buffer size %llu too small for header size %llu", (unsigned long long)bufferSize, (unsigned long long)sizeof(*hdr));
		return(EINVAL);		/* buffer too small to really contain header */
	}
	if (LETOH(hdr->ihMagic) != kImage3HeaderMagic) {
		debug("bad magic 0x%08x expecting 0x%08x", LETOH(hdr->ihMagic), kImage3HeaderMagic);
		return(EINVAL);		/* magic must match */
	}
	if (LETOH(hdr->ihBufferLength) > (bufferSize - sizeof(*hdr))) {
		debug("header length %llu too large for buffer length %llu",
		    (unsigned long long)(LETOH(hdr->ihBufferLength) + sizeof(*hdr)), (unsigned long long)bufferSize);
		return(EINVAL);		/* container is too big for buffer */
	}
	if (LETOH(hdr->ihSignedLength) > LETOH(hdr->ihBufferLength)) {
		debug("signed legnth %u too large for buffer length %u",
		    LETOH(hdr->ihSignedLength), LETOH(hdr->ihBufferLength));
		return(EINVAL);		/* signed length is too large */
	}
	if ((LETOH(hdr->ihBufferLength) + sizeof(*hdr)) > LETOH(hdr->ihSkipDistance)) {
		debug("skip distance %llu too short for buffer length %llu",
		    (unsigned long long)(LETOH(hdr->ihBufferLength) + sizeof(*hdr)), (unsigned long long)LETOH(hdr->ihSkipDistance));
		return(EINVAL);		/* skip distance falls into container */
	}
		
	/* the buffer looks OK, allocate our state */
	if ((state = image3Malloc(sizeof(*state))) == NULL) {
		debug("failed to allocate state");
		return(ENOMEM);
	}

	state->flags = kImage3ImageWasInstantiated;
	state->nestedImage = NULL;
	
	if (LETOH(hdr->ihSignedLength) > 0) {
		debug("image claims to be signed, marking immutable");
		state->flags |= kImage3ImageIsSigned;
	} else {
#ifdef IMAGE3_CREATOR
		Image3TagHeader	*tagHeader;
		u_int32_t	tagCursor;

		/* since we have to be able to realloc, copy to an allocated buffer */
		copyBuffer = true;

		/*
		 * To get lastTag set correctly, we have to walk the tags in the image.
		 *
		 * Note that this is only done in read/write tools and only if the image
		 * is not signed - in this case we are not concerned with security.
		 */
		state->cursor = LETOH(hdr->ihBufferLength);
		state->lastTag = -1;
		if (LETOH(hdr->ihBufferLength) > 0) {
			tagCursor = 0;
	
			for (;;) {
				tagHeader = (Image3TagHeader *)&hdr->ihBuffer[tagCursor];

				/* if the tag points past the end of the image, it's corrupt */
				/* if the skip distance is to small, it's corrupt */
				if ((tagCursor + LETOH(tagHeader->itSkipDistance)) > LETOH(hdr->ihBufferLength)) {
					image3Free(state, sizeof(*state));
					debug("tag skip distance %u moves cursor %u outside buffer length %u",
					    LETOH(tagHeader->itSkipDistance), tagCursor, LETOH(hdr->ihBufferLength));
					return(EINVAL);
				}
				if ((LETOH(tagHeader->itSkipDistance) < sizeof(*tagHeader))) {
					image3Free(state, sizeof(*state));
					debug("tag skip distance %u outside too small", LETOH(tagHeader->itSkipDistance));
					return(EINVAL);
				}

				/* if the tag points precisely to the end of the image, we're done */
				if ((tagCursor + LETOH(tagHeader->itSkipDistance)) == LETOH(hdr->ihBufferLength)) {
					state->lastTag = tagCursor;
					break;
				}

				/* skip to the next tag */
				tagCursor += LETOH(tagHeader->itSkipDistance);
			}
		}
#endif
	}

	/* if we were asked, or forced, copy the buffer */
	if (copyBuffer) {
		debug("image mutable or copy requested");
		state->allocSize = LETOH(hdr->ihBufferLength) + sizeof(*hdr);
		if ((state->image = image3Malloc(state->allocSize)) == NULL) {
			image3Free(state, sizeof(*state));
			debug("failed to allocate memory for image copy");
			return(ENOMEM);
		}
		memcpy(state->image, fromBuffer, state->allocSize);
		state->flags |= kImage3ImageWasAllocated;
	} else {
		state->image = hdr;
		state->allocSize = bufferSize;
	}
	
	*newHandle = state;
	return(0);
}

/*
 * Validate the signature on an Image3 object.
 *
 * Note that images from untrusted sources should be handled with *extreme caution*
 * until this operation has reported success.
 *
 * We assume that the ihBufferLength and ihSignedLength fields in the image header
 * have already been sanity-checked.
 *
 * If the object was loaded from local storage where it was expected that it was
 * personalised for the device (normally the case) then the kImage3ValidateLocalStorage
 * option should be supplied.
 *
 * Returns:
 * 0		Success.
 * EINVAL	The contents of the buffer are malformed
 * ENOMEM	Could not allocate working memory
 * EPERM	Security validation failed
 * Other errors may be returned by the PKI or AES infrastructure. 
 *
 * Note to auditors:
 * -----------------
 * This code is used on images that have been processed by image3InstantiateFromBuffer.
 * As such, it makes the assumption that header fields that are validated there may be
 * trusted.  In particular, ihBufferLength is trusted to lie within the caller's buffer,
 * and ihSignedLength is trusted to be less than or equal to ihBufferLength.
 */
int
image3ValidateSignature(
	Image3ObjectHandle withHandle,
	u_int32_t expectedType,
	u_int32_t validationOptions,
	bool *validatedWithEmbeddedSignature)
{
	Image3ObjectHeader	*hdr;
	Image3TagHeader		*hashTag;
	Image3TagHeader		*certTag;
	u_int32_t		tagCursor;
	char			hashBuffer[IMAGE3_HASH_SIZE];
	size_t			hashSize;
	void			*certCustomData;
	size_t			certCustomDataLength;
	int			result;
	uint8_t			*base;
	size_t			len;

	debug("validating signature on image %p", withHandle);

	/* do we already know whether the image is trusted? */
	if (withHandle->flags & kImage3ImageWasValidated) {
		debug("returning cached result %d", (withHandle->flags & kImage3ImageIsTrusted) ? 0 : EPERM);
		return((withHandle->flags & kImage3ImageIsTrusted) ? 0 : EPERM);
	}

	/* image can be altered below, no second chances */
	withHandle->flags |= kImage3ImageWasValidated;

	/* if the image is not signed, it's not trusted */
	if (!(withHandle->flags & kImage3ImageIsSigned)) {
		debug("image is not signed, cannot be considered trusted");
		return(EPERM);
	}
	
	hdr = withHandle->image;

	/* verify that the image is large enough to contain at least one tag header */
	if (LETOH(hdr->ihBufferLength) < sizeof(*hashTag)) {
		debug("image too small to be signed");
		return(EINVAL);
	}

	/*
	 * Indirect (ticket) image signature validation.
	 */

	/*
	 * If the image has been personalised by the addition of an ECID in
	 * the signed range, we need to back that out before generating the
	 * hash for ticket purposes.
	 *
	 * This is transitional, and it can eventually be removed out once 
	 * ticketting is the norm.
	 *
	 * In the case of ticket validation after backing out an ECID tag, we
	 * need to zero out the ECID tag after validation to prevent leakage
	 * of untrusted data. If ticket validation fails and we fall back to
	 * embedded image signature validation, we have to keep the ECID tag 
	 */
	uint32_t savedLength;
	Image3TagHeader *ecidTag;
	uint32_t rewindCount;

	/* save the signed length - we might overwrite it here, but we'll need
	   to restore it if ticket validation fails and we fall back to embedded
	   image signature validation */
	savedLength = LETOH(hdr->ihSignedLength);

	/* look backwards to see if the last thing in the signed range is an ECID tag */
	tagCursor = savedLength;
	if (tagCursor > (sizeof(Image3TagHeader) + sizeof(Image3TagNumber64))) {

		/* back up to the highest possible address at which we might find the ECID tag */
		tagCursor -= sizeof(Image3TagHeader) + sizeof(Image3TagNumber64);

		/* due to padding, we may have to go back up to 64 bytes further to find the ECID tag */
		rewindCount = 0;
		do {
			ecidTag = (Image3TagHeader *)&hdr->ihBuffer[tagCursor];
			if ((ecidTag->itTag == kImage3TagTypeUniqueID) &&
			    (ecidTag->itBufferLength == sizeof(Image3TagNumber64))) {
				/* this looks like a valid ECID tag - back the signed length up to avoid it */
				hdr->ihSignedLength = HTOLE(tagCursor);
				debug("backed signed length up from %u to %u\n", savedLength, tagCursor);
				break;
			}
			/* check whether we can safely rewind further */
			if (tagCursor < 4)
				break;
			tagCursor -= 4;
			rewindCount += 4;
		} while (rewindCount <= 64);
	}

	/* generate the hash that we expect to find in the ticket */
	hashSize = sizeof(*hdr) - offsetof(Image3ObjectHeader, ihSignedLength) + LETOH(hdr->ihSignedLength);
	image3SHA1Generate(&hdr->ihSignedLength, hashSize, hashBuffer);
	
	/* ask the ticket validator for an opinion on this image */
	result = image3TicketVerifyHash(hashBuffer, sizeof(hashBuffer), LETOH(hdr->ihType), expectedType);

	if (result == 0) {
		debug("ticket says this image is trusted");
		goto out_trusted;
	}
	if (result == EPERM) {
		debug("ticket says this image is untrusted");
		return(EPERM);
	}
	debug("ticket does not like this image");

	/*
	 * Embedded image signature validation.
	 */

	/* verify that the caller is willing to have us validate with the embedded signature */
	if (validationOptions & kImage3ValidateRequireTicket) {
		debug("image requires ticket validation");
		return(EINVAL);
	}

	/* restore the signed length to include the ECID tag again */
	hdr->ihSignedLength = HTOLE(savedLength);

	/* verify that the hash tag lies within the image buffer */
	tagCursor = LETOH(hdr->ihSignedLength);
	if (tagCursor > (LETOH(hdr->ihBufferLength) - sizeof(*hashTag))) {
		debug("hash tag overflows buffer");
		return(EINVAL);
	}
	/* verify that the hash tag's buffer does not overflow the image buffer */
	hashTag = (Image3TagHeader *)&hdr->ihBuffer[tagCursor];
	if (LETOH(hashTag->itBufferLength) > (LETOH(hdr->ihBufferLength) - tagCursor - sizeof(*hashTag))) {
		debug("buffer too small to contain signed hash tag payload");
		return(EINVAL);
	}
	if (LETOH(hashTag->itTag) != kImage3TagTypeSignedHash) {
		debug("buffer does not contain signed hash tag at required location");
		return(EINVAL);
	}

	/* move the tag cursor to the location we expect to find the cert chain */
	tagCursor += LETOH(hashTag->itSkipDistance);

	/* check cursort against lower legal bound */
	if (tagCursor < LETOH(hdr->ihSignedLength)) {
		debug("skip distance on signed hash tag wraps cursor");
		return(EINVAL);
	}

	/* verify that the cert tag lies within the buffer */
	if (tagCursor > (LETOH(hdr->ihBufferLength) - sizeof(*certTag))) {
		debug("cert tag overflows buffer");
		return(EINVAL);
	}

	/* verify that the cert tag's buffer does not overflow the image buffer */
	certTag = (Image3TagHeader *)&hdr->ihBuffer[tagCursor];
	if (LETOH(certTag->itBufferLength) > (LETOH(hdr->ihBufferLength) - tagCursor - sizeof(*certTag))) {
		debug("buffer too small to contain cert chain tag payload");
		return(EINVAL);
	}
	if (LETOH(certTag->itTag) != kImage3TagTypeCertificateChain) {
		debug("buffer does not contain cert chain tag at required location");
		return(EINVAL);
	}

	/* fix up the buffer length to precisely describe the image including signature and no more */
	if ((tagCursor + sizeof(*certTag) + LETOH(certTag->itBufferLength)) != LETOH(hdr->ihBufferLength)) {
		debug("correcting buffer length from %u to %u",
		    LETOH(hdr->ihBufferLength), (unsigned int)(tagCursor + sizeof(*certTag) + LETOH(certTag->itBufferLength)));
		hdr->ihBufferLength = HTOLE(tagCursor + sizeof(*certTag) + LETOH(certTag->itBufferLength));
	}
	
	/* hash the signed portions of the buffer */
	hashSize = sizeof(*hdr) - offsetof(Image3ObjectHeader, ihSignedLength) + LETOH(hdr->ihSignedLength);
	image3SHA1Generate(&hdr->ihSignedLength, hashSize, hashBuffer);

	/* if the image has come from local storage, the signed hash needs to be decrypted */
	if (validationOptions & kImage3ValidateLocalStorage) {
                if (0 != (LETOH(hashTag->itBufferLength) % 16)) {
                        debug("signed hash buffer length invalid for AES decryption");
                        return(EINVAL);
                }
		debug("decrypting signed hash");
		image3AESDecryptUsingLocalKey(hashTag->itBuffer, LETOH(hashTag->itBufferLength));
	}

	/* call the PKI interface to validate the signature and compare the hash */
	certCustomData = NULL;
	certCustomDataLength = 0;
	result = image3PKIVerifyHash(
		hashBuffer,
		IMAGE3_HASH_SIZE,
		hashTag->itBuffer,
		LETOH(hashTag->itBufferLength),
		certTag->itBuffer,
		LETOH(certTag->itBufferLength),
		&certCustomData,
		&certCustomDataLength);

	/* clear the possibly decrypted signature from memory */
	memset(hashTag->itBuffer, 0, LETOH(hashTag->itBufferLength));

	if (result) {
		debug("PKI verification failed (%d)", result);
		return(result);
	}
	debug("PKI verification passed...");
	
	/* if the cert validation returned custom data, it should be an image3 image as well */
	if (certCustomData) {
		result = image3InstantiateFromBuffer(
			&withHandle->nestedImage,
			certCustomData,
			certCustomDataLength,
			true /* copyBuffer */);
		if (result) {
			debug("failed to instantiate image from certificate custom data");
			return(result);
		}

		/* inherit valid/trusted flags from the parent image - not validated separately */
		withHandle->nestedImage->flags |=
		    kImage3ImageWasValidated | kImage3ImageIsTrusted | kImage3ImageIsSigned;
	}
	if (validatedWithEmbeddedSignature)
		*validatedWithEmbeddedSignature = true;

out_trusted:	
	withHandle->flags |= kImage3ImageIsTrusted;

	/*
	 * Strip the signature and cert chain (and anything that might follow them) out of the buffer 
	 * to prevent any extra data hanging around.
	 */
	base = (uint8_t *)hdr + sizeof(*hdr) + LETOH(hdr->ihSignedLength);
	len = (uint8_t *)hdr + withHandle->allocSize - base;

	memset(base, 0, len);

	hdr->ihBufferLength = HTOLE(LETOH(hdr->ihSignedLength));
	
	return(0);
}

/*
 * Test for the presence of a tag in an image.
 *
 * Returns:
 * 0		Success
 * EINVAL	The buffer is malformed.
 * ENOENT	The tag was not found.
 */
int
image3TagIsPresent(
	const Image3ObjectHandle withHandle,
	const u_int32_t withTag)
{
	void	*p;

	return(image3GetTagStruct(withHandle, withTag, &p, NULL, 0));
}

/*
 * Fetch a numeric tag's value.
 *
 * Returns:
 * 0		Success
 * EINVAL	The buffer is malformed.
 * ENOENT	The tag was not found.
 */
int
image3GetTagSignedNumber(
	const Image3ObjectHandle withHandle,
	const u_int32_t withTag,
	int64_t *toNumber,
	int skipCount)
{
	Image3TagNumber	*num;
	size_t		size;
	int		result;

	size = 0;
	if ((result = image3GetTagStruct(withHandle, withTag, (void **)&num, &size, skipCount)) == 0) {
		switch (size) {
			case sizeof(Image3TagNumber32) :
				*toNumber = LETOH(num->number.n32.value.s32);
				break;

			case sizeof(Image3TagNumber64) :
				*toNumber = LETOHLL(num->number.n64.value.s64);
				break;

			default :
				result = EINVAL;
				break;
		}
	}

	return(result);
}

int
image3GetTagUnsignedNumber(
	const Image3ObjectHandle withHandle,
	const u_int32_t withTag,
	u_int64_t *toNumber,
	int skipCount)
{
	Image3TagNumber	*num;
	size_t		size;
	int		result;

	size = 0;
	if ((result = image3GetTagStruct(withHandle, withTag, (void **)&num, &size, skipCount)) == 0) {
		switch (size) {
			case sizeof(Image3TagNumber32) :
				*toNumber = LETOH(num->number.n32.value.u32);
				break;

			case sizeof(Image3TagNumber64) :
				*toNumber = LETOHLL(num->number.n64.value.u64);
				break;

			default :
				result = EINVAL;
				break;
		}
	}

	return(result);
}

/*
 * Fetch a string tag's value.
 *
 * Returns:
 * 0		Success.  *toBuffer contains an ASCIIZ string that must be freed by the caller.
 * EINVAL	The buffer is malformed.
 * ENOMEM	Not enough memory available to allocate a copy of the string.
 * ENOENT	The tag was not found.
 */
int
image3GetTagString(
	const Image3ObjectHandle withHandle,
	const u_int32_t withTag,
	char **toBuffer,
	int skipCount)
{
	Image3TagString	*str;
	size_t		size;
	char		*buf;
	int		result;

	size = 0;
	if ((result = image3GetTagStruct(withHandle, withTag, (void **)&str, &size, skipCount)) == 0) {
		/* validate string structure */
		if (size < sizeof(*str))				/* malformed */
			return(EINVAL);
		if ((size - sizeof(*str)) != LETOH(str->stringLength))	/* claimed length overflows allocation */
			return(EINVAL);

		/* make a copy of the string */
		if ((buf = image3Malloc(LETOH(str->stringLength) + 1)) == NULL) {
			result = ENOMEM;
		} else {
			/* copy valid bytes, guarantee NUL termination */
			memcpy(buf, str->stringBytes, str->stringLength);
			buf[str->stringLength] = '\0';
			*toBuffer = buf;
		}
	}
	return(result);
}

/*
 * Find a tag by name.
 *
 * The leading (skipCount) matching tags will be ignored.
 *
 * The tag 0xffffffff is a wildcard with matches any tag.
 *
 * Returns:
 * 0		Success
 * ENOENT	The tag was not found.
 */
static int
_image3FindTag(
	const Image3ObjectHandle withHandle,
	const u_int32_t withTag,
	Image3TagHeader **forHeader,
	size_t *structSize,
	int skipCount)
{
	Image3TagHeader	*thdr;
	u_int8_t	*cursor, *bound, *next, *tagEnd;

	/* our starting point */
	cursor = &withHandle->image->ihBuffer[0];
	
	/* find the upper boundary of the tag buffer */
	bound = cursor + LETOH(withHandle->image->ihBufferLength);
	debug("scanning for tags between %p and %p", cursor, bound);

	/* do not permit the buffer to wrap */
	if (bound < cursor) {
		debug("integer wrap computing upper bound");
		return(EINVAL);
	}

	/* iterate, making sure that the header fits within the upper boundary */
	while (cursor < bound) {
		/* consider the tag at cursor */
		thdr = (Image3TagHeader *)cursor;
		
		/* do not permit the header to wrap */
		if ((cursor + sizeof(*thdr)) < cursor) {
			debug("integer wrap between cursor and tag header end");
			return(EINVAL);
		}

		/* if any part of the header is outside the boundary, the buffer is malformed */
		if ((cursor + sizeof(*thdr)) > bound) {
			debug("tag header outside search boundary");
			return(EINVAL);
		}
	
		/* if the tag data wraps or crosses the boundary, the buffer is malformed */
		tagEnd = cursor + sizeof(*thdr) + LETOH(thdr->itBufferLength);
		if ((tagEnd < (cursor + sizeof(*thdr))) || (tagEnd > bound)) {
			debug("tag data violates search boundaries or wraps");
			return(EINVAL);
		}
	
		/* compare tags */
		if ((withTag == 0xffffffff) || (LETOH(thdr->itTag) == withTag)) {
			if (skipCount == 0) {
				if (structSize) {
					/* caller-specified size does not match tag data size */
					if (*structSize && (*structSize != LETOH(thdr->itBufferLength))) {
						debug("tag specifies size %u but caller requires %u for tag 0x%x",
						    (unsigned int)*structSize, LETOH(thdr->itBufferLength), LETOH(thdr->itTag));
						return(EINVAL);
					}

					/* tell caller data size */
					*structSize = LETOH(thdr->itBufferLength);
				}
				debug("found tag at %p", thdr);
				*forHeader = thdr;
				return(0);
			}
			skipCount--;
		}
		
		/* validate the skip distance */
		if (LETOH(thdr->itSkipDistance) < (sizeof(*thdr) + LETOH(thdr->itBufferLength))) {
			debug("skip distance %u less than the sum of header size and buffer data %u",
			    LETOH(thdr->itSkipDistance), (unsigned int)(sizeof(*thdr) + LETOH(thdr->itBufferLength)));
			return(EINVAL);
		}
		
		/* find the next header */
		next = cursor + LETOH(thdr->itSkipDistance);

		/* do not permit arithmetic wrap */
		if (next < cursor) {
			debug("integer wrap advancing cursor");
			return(EINVAL);
		}

		cursor = next;
	}
	return(ENOENT);
}

/*
 * Fetch a direct pointer to a structure tag.  Note that this points into the object itself
 * and should be used with some caution.
 *
 * Returns:
 * 0		Success.
 * EINVAL	The buffer is malformed.
 * ENOENT	The tag was not found.
 */
int
image3GetTagStruct(
	const Image3ObjectHandle withHandle,
	const u_int32_t withTag,
	void **structPtr,
	size_t *structSize,
	int skipCount)
{
	int	result;
	Image3TagHeader	*thdr;

	if ((result = _image3FindTag(withHandle, withTag, &thdr, structSize, skipCount)) != 0)
		return(result);

	*structPtr = (void *)&thdr->itBuffer;
	return(0);
}

/*
 * Return the nested image's handle (if one exists).
 */
Image3ObjectHandle
image3GetNestedImage(const Image3ObjectHandle withHandle)
{
	return(withHandle->nestedImage);
}

#ifdef IMAGE3_DEBUG
void
_image3PrintImage(Image3ObjectHandle withHandle)
{
	int			result;
	int			index;
	unsigned int		i, j;
	Image3TagHeader		*thdr;

	printf("image skip distance: 0x%x\n", LETOH(withHandle->image->ihSkipDistance));
	printf("image buffer length: 0x%x\n", LETOH(withHandle->image->ihBufferLength));
	printf("image signed length: 0x%x\n\n", LETOH(withHandle->image->ihSignedLength));
	
	for (index = 0; ; index++) {
		result = _image3FindTag(withHandle, 0xffffffff, &thdr, NULL, index);
		if (result != 0) {
			if (result != ENOENT)
				debug("image scan terminated unexpectedly");
			break;
		}
		printf("tag:           %c%c%c%c\n", UNTAG(LETOH(thdr->itTag)));
		printf("skip distance: 0x%x\n", LETOH(thdr->itSkipDistance));
		printf("buffer length: 0x%x\n", LETOH(thdr->itBufferLength));
		printf("---------------------\n");

		for (i = 0; i < LETOH(thdr->itBufferLength); i += 16) {
			printf("%08x: ", i);
			for (j = 0; (j < 16) && ((i + j) < LETOH(thdr->itBufferLength)); j++)
				printf(" %02x", (unsigned int)thdr->itBuffer[i + j]);
			printf("\n");
		}
	}
}

#ifdef IMAGE3_UTILITY

int
_image3WriteData(Image3ObjectHandle withHandle, const char *path)
{
	int	fd;
	int	result = 0;
	Image3TagHeader		*thdr;

	fd = open(path, O_CREAT|O_RDWR|O_TRUNC, 0644);

	if (fd < 0) {
		perror("open failed");
		return fd;
	}

	/*
	 * Check to see if the image is encrypted.  If so, we don't
	 * decrypt it here, so there is nothing we can do
	 */
	result = _image3FindTag(withHandle, kImage3TagTypeKeybag, &thdr, NULL, 0);	
	if (result == 0) {
		debug("image is encrypted, no going back");
		result = -1;
		goto out;	
	}

	result = _image3FindTag(withHandle, kImage3TagTypeData, &thdr, NULL, 0);
	if (result != 0) {
		debug("couldn't find data section");
		goto out;
	}

	write(fd, thdr->itBuffer, thdr->itBufferLength);

	printf("Wrote %s\n", path);

	close(fd);

out:
	return (result);
}

#endif /* IMAGE3_UTILITY */

#endif /* IMAGE3_DEBUG */
