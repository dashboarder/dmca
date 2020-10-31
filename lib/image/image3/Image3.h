/*
 * Copyright (C) 2007-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _IMAGE3_H_
#define _IMAGE3_H_

#include <stdint.h>

#include <sys/types.h>

#ifndef false
# include <stdbool.h>
#endif


/*
 * All Image3 operations use an Image3ObjectHandle to refer to the image being worked on.
 */
struct _Image3InternalState;
typedef struct _Image3InternalState	*Image3ObjectHandle;

/*
 * Discards an Image3 image.
 */
extern void
image3Discard(
	Image3ObjectHandle *withHandle);


#ifdef IMAGE3_CREATOR
/*
 * Create a new Image3 working object.
 *
 * Returns:
 * 0		Success
 * ENOMEM	Unable to allocate memory
 */
extern int
image3InstantiateNew(
	Image3ObjectHandle *newHandle,
	size_t initialAllocation,
	u_int32_t imageType);

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
extern int
image3Finalize(
	const Image3ObjectHandle withHandle,
	void **objectBuffer,
	size_t *objectSize,
	bool signImage,
	size_t reservationSize);

/*
 * Set a numeric tag.
 *
 * Returns:
 * 0		Success
 * ENOMEM	Unable to allocate memory
 */
extern int
image3SetTagSignedNumber(
	const Image3ObjectHandle withHandle,
	u_int32_t withTag,
	int64_t withValue);

extern int
image3SetTagUnsignedNumber(
	const Image3ObjectHandle withHandle,
	u_int32_t withTag,
	u_int64_t withValue);

/*
 * Set a string tag.
 *
 * Returns:
 * 0		Success
 * ENOMEM	Unable to allocate memory
 */
extern int
image3SetTagString(
	const Image3ObjectHandle withHandle,
	u_int32_t withTag,
	char *withValue);

/*
 * Set a structure tag.
 *
 * Note that the withAlignment field aligns the structure, not the tag.
 *
 * Returns:
 * 0		Success
 * ENOMEM	Unable to allocate memory
 */
extern int
image3SetTagStructure(
	const Image3ObjectHandle withHandle,
	u_int32_t withTag,
	const void *withValue,
	size_t withSize,
	int withAlignment);


/*
 * Advances the cursor with zero padding.
 *
 * Returns:
 * 0		Success
 * ENOMEM	Unable to allocate memory
 * EROFS	Image is read-only
 */
extern int
image3AdvanceCursorWithZeroPad(
					const Image3ObjectHandle withHandle,
					int withSize);

#endif /* IMAGE3_CREATOR */


/*
 * Take a data buffer that might contain an Image3 object and get a handle to it.
 *
 * Note that no security-relatd validation is performed by this operation.
 *
 * If IMAGE3_CREATOR is defined and the image is mutable, or if copyBuffer is
 * true, then the image buffer is copied.  Otherwise the image directly references
 * the supplied buffer.
 *
 * Returns:
 * 0		Success.
 * EINVAL	The contents of the buffer are malformed
 */
extern int
image3InstantiateFromBuffer(
	Image3ObjectHandle *newHandle,
	const void *fromBuffer,
	const size_t bufferSize,
	bool copyBuffer);

/*
 * Validate the signature on an Image3 object.
 *
 * Note that images from untrusted sources should be handled with *extreme caution*
 * until this operation has reported success.
 *
 * If the object was loaded from local storage where it was expected that it was
 * personalised for the device (normally the case) then the kImage3ValidateLocalStorage
 * option should be supplied.
 *
 * If the object's validity can only be asserted via the ticket API,
 * kImage3ValidateRequireTicket should be supplied.
 *
 * The result of the validation operation is cached, so it is safe to call this function
 * multiple times.  Once the initial validation operation is performed, the validation
 * options are ignored, so knowledge of the source of the object is not required in this
 * case.
 *
 * Returns:
 * 0		Success.
 * EINVAL	The contents of the buffer are malformed or otherwise cannot be validated.
 * EPERM	Validation failed, the image cannot be trusted.
 */
#define kImage3ValidateLocalStorage	(1<<0)
#define kImage3ValidateRequireTicket	(1<<2)

extern int
image3ValidateSignature(
	const Image3ObjectHandle withHandle,
	const u_int32_t expectedType,
	const u_int32_t validationOptions,
	bool *validatedWithEmbeddedSignature);

/*
 * Test for the presence of a tag in an image.
 *
 * Returns:
 * 0		Success
 * EINVAL	The buffer is malformed.
 * ENOENT	The tag was not found.
 */
extern int
image3TagIsPresent(
	const Image3ObjectHandle withHandle,
	const u_int32_t withTag);

/*
 * Fetch a numeric tag's value.
 *
 * Returns:
 * 0		Success
 * EINVAL	The buffer is malformed.
 * ENOENT	The tag was not found.
 */
extern int
image3GetTagSignedNumber(
	const Image3ObjectHandle withHandle,
	const u_int32_t withTag,
	int64_t *toNumber,
	int skipCount);

extern int
image3GetTagUnsignedNumber(
	const Image3ObjectHandle withHandle,
	const u_int32_t withTag,
	u_int64_t *toNumber,
	int skipCount);

/*
 * Fetch a string tag's value.
 *
 * Returns:
 * 0		Success.  *toBuffer contains an ASCIIZ string that must be freed by the caller.
 * EINVAL	The buffer is malformed.
 * ENOENT	The tag was not found.
 */
extern int
image3GetTagString(
	const Image3ObjectHandle withHandle,
	const u_int32_t withTag,
	char **toBuffer,
	int skipCount);

/*
 * Fetch a direct pointer to a structure tag.  Note that this points into the image itself
 * and should be used with some caution.
 *
 * The skipCount argument may be used to specify a number of matching tags to be ignored.
 *
 * Returns:
 * 0		Success.
 * EINVAL	The buffer is malformed.
 * ENOENT	The tag was not found.
 */
extern int
image3GetTagStruct(
	const Image3ObjectHandle withHandle,
	const u_int32_t withTag,
	void **structPtr,
	size_t *structSize,
	int skipCount);

/*
 * Compute the digest to be signed into the root ticket.  If the image has been previously
 * personalized with the old method (with a signature covering an added ECID tag), the digest
 * excludes the personalized region.
 *
 * Although the underlying image3 buffer is returned unmodified, this function may
 * temporarily modify the buffer when computing the hash.  The buffer must therefore be in a 
 * writable region of memory.

 */
extern int
image3GetPrePersonalizedDigest(
    const Image3ObjectHandle withHandle,
	uint8_t *hashBuffer,
    size_t hashBufferLength);

/*
 * Return the nested image's handle (if one exists).
 *
 * Note that the nested image is only discoverable if an image is authenticated.
 */
Image3ObjectHandle
image3GetNestedImage(const Image3ObjectHandle withHandle);


#ifdef IMAGE3_DEBUG
extern void
_image3PrintImage(Image3ObjectHandle withHandle);

#ifdef IMAGE3_UTILITY
int
_image3WriteData(Image3ObjectHandle withHandle, const char *path);
#endif /* IMAGE3_UTILITY */

#endif /* IMAGE3_DEBUG */

/*
 * Prototypes for external functions that must be supplied.
 */

/*
 * Generate a SHA-1 hash of the supplied buffer.
 */
extern void
image3SHA1Generate(void *dataBuffer, size_t dataSize, void *hashBuffer);

#ifdef IMAGE3_CREATOR
/*
 * Generate a partial SHA-1 hash of the supplied buffer.
 */
void
image3SHA1Partial(void *dataBuffer, size_t dataSize, void *hashBuffer);
#endif /* IMAGE3_CREATOR */

/*
 * Sign a hash.
 */
extern int
image3PKISignHash(
	void *hashBuffer,
	size_t hashSize,
	void **resultPointer,
	size_t *resultSize,
	void **certBlobPointer,
	size_t *certBlobSize);

/*
 * Verify the signature on a hash.
 *
 * Returns:
 *  0      - image is valid and can be trusted
 *  EPERM  - image is valid but cannot be trusted
 *  EINVAL - image is not valid, cannot be trusted
 */
extern int
image3PKIVerifyHash(
        void *hashBuffer,
	size_t hashSize,
	void *signedHashBuffer,
	size_t signedHashSize,
	void *certBlobBuffer,
	size_t certBlobSize,
	void **certCustomData,
	size_t *certCustomDataSize);

/*
 * Validate an image hash using the ticket database.
 *
 * Returns:
 *  0      - image is valid and can be trusted
 *  EPERM  - image is valid but cannot be trusted
 *  EINVAL - image is not valid, cannot be trusted
 */
extern int
image3TicketVerifyHash(
	void *hashBuffer,
	size_t hashSize,
	uint32_t imageType,
	uint32_t expectedType);

/*
 * Decrypt a buffer in-place using AES and a local unit-specific key.
 */
extern int
image3AESDecryptUsingLocalKey(void *buffer, size_t length);

/*
 * Encrypt a buffer in-place using AES and a local unit-specific key.
 */
extern void
image3AESEncryptUsingLocalKey(void *buffer, size_t length);

/*
 * Memory allocator
 */
extern void *
image3Malloc(size_t size);

/*
 * Memory de-allocator
 */
extern void
image3Free(void *ptr, size_t size);

#endif /* _IMAGE3_H_ */
