/*
 * Copyright (C) 2007-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <lib/Image2Format.h>
#include <lib/blockdev.h>
#include <lib/profile.h>
#include <lib/image.h>
#include <lib/cksum.h>
#include <sys/errno.h>
#include <sys/security.h>
#include <platform.h>
#include <drivers/aes.h>
#include <drivers/sha1.h>
#include <stdlib.h>
#include <platform/breadcrumbs.h>
#include <string.h>
#if WITH_PKI
#include <lib/pki.h>
#endif
#if WITH_TICKET
#include <lib/ticket.h>
#endif
#include "image3_wrapper.h"
#include "Image3Format.h"
#include "Image3.h"

#define UNTAG(x)		(((x) >> 24) & 0xff),(((x) >> 16) & 0xff),(((x) >> 8) & 0xff),((x) & 0xff)

/* some sanity checking of cross-domain constants */
#if kImage3SecurityDomainManufacturer != kPlatformSecurityDomainManufacturing
# error kImage3SecurityDomainManufacturer != kPlatformSecurityDomainManufacturing
#endif
#if kImage3SecurityDomainDarwin != kPlatformSecurityDomainDarwin
# error kImage3SecurityDomainDarwin != kPlatformSecurityDomainDarwin
#endif
#if kImage3KeybagSelectorChipUnique != IMAGE_KEYBAG_SELECTOR_PROD
# error kImage3KeybagSelectorChipUnique != IMAGE_KEYBAG_SELECTOR_PROD
#endif
#if kImage3KeybagSelectorChipUniqueDev != IMAGE_KEYBAG_SELECTOR_DEV
# error kImage3KeybagSelectorChipUniqueDev != IMAGE_KEYBAG_SELECTOR_DEV
#endif


struct image3_info {
	struct list_node	 node;

	struct blockdev		*bdev;		/* device the image is on */
	off_t			devOffset;	/* where on the bdev it is */

	Image3ObjectHandle	imageHandle;	/* if the image is loaded, its current handle */

	struct image_info 	image_info;	/* public image_info */
};

static struct list_node images = LIST_INITIAL_VALUE(images);

enum matching_relation {
	REL_EQUAL,
	REL_GREATER_EQUAL
};


static int
image3_load_copyobject(
	struct image_info *image_info, 
	void *objectBuffer, 
	size_t objectSize);

static int
image3_load_validate_signature(
	Image3ObjectHandle objectHandle,
	u_int32_t expectedType,
	bool requireTrust,
	uint32_t validationFlags,
	bool *untrustedImage,
	bool *imageIncludesConstraints);

static int
image3_load_validate_constraints(
	Image3ObjectHandle objectHandle,
	u_int32_t type,
	bool requireTrust,
	bool allowGreaterEpoch,
	bool *clearProduction,
	bool *untrustedImage);

static int
image3_load_decrypt_payload(
	Image3ObjectHandle objectHandle,
	void *payloadBuffer,
	size_t payloadSize,
	void *destinationBuffer,
	bool untrustedImage);

static int
verifyIntersectingBits(
	Image3ObjectHandle objectHandle,
	u_int32_t withTag,
	u_int64_t withBits,
	bool tagIsRequired,
	bool nestedImageOnly,
	bool *tagWasPresent);

static int
verifyNumberRelation(
	Image3ObjectHandle objectHandle,
	u_int32_t withTag,
	u_int64_t requiredValue,
	enum matching_relation relation,
	bool tagIsRequired,
	bool nestedImageOnly,
	bool *tagWasPresent);

static int
verifyMatchingBytes(
	Image3ObjectHandle objectHandle,
	u_int32_t withTag,
	const void *testBytes,
	size_t testBytesLength,
	bool tagIsRequired,
	bool *tagWasPresent);

/*
 * image3_process_superblock
 *
 * Searches the given bdev looking for image3 images.  Returns a count of found
 * images.
 *
 * We use the Image2 superblock format, but with Image3 object containers.
 */
int
image3_process_superblock(Image2Superblock *sblock, struct blockdev *bdev, off_t offset, uint32_t imageOptions)
{
	int			imageCount;
	Image3ObjectHeader	objHeader;
	struct image3_info	*imageInfo;
	int			err;
	off_t			spaceBytes, cursor, residual;

	/*
	 * Validate the superblock
	 */

	/* check magic number and basic integrity */
	if (sblock->isMagic != kImage2SuperMagic) {
		dprintf(DEBUG_SPEW, "bad superblock magic\n");
		return(0);
	}
#if !IMAGE3_NO_CRC
	if (sblock->isCheck != crc32((unsigned char *)sblock, offsetof(Image2Superblock, isCheck))) {
		dprintf(DEBUG_SPEW, "superblock CRC mismatch\n");
		return(0);
	}
#endif

	/* validate isImageGranule and isImageAvail, assuming bdev->total_len and offset is trusted */
	RELEASE_ASSERT(offset >= 0);
	if ((uint64_t)offset > bdev->total_len) {
		dprintf(DEBUG_SPEW, "superblock outside blockdev\n");
		return(0);
	}
	spaceBytes = bdev->total_len - offset;
	if ((0 == sblock->isImageGranule) ||
	    (0 == sblock->isImageAvail)) {
		dprintf(DEBUG_SPEW, "granule/avail zero\n");
		return(0);
	}
	if (sblock->isImageGranule > spaceBytes) {
		dprintf(DEBUG_SPEW, "granule > allocated space\n");
		return(0);
	}
	if (sblock->isImageAvail > (spaceBytes / sblock->isImageGranule)) {
		dprintf(DEBUG_SPEW, "avail > allocated space\n");
		return(0);
	}

	/* validate isBootBlockSize */
	/* XXX 7561698 use of isImageOffset here is inconsistent with doc, seems to be just an adjunct to isBootBlockSize */
	if ((0 == sblock->isBootBlockSize) ||
	    (sblock->isBootBlockSize > sblock->isImageAvail) ||
	    (sblock->isImageOffset > (sblock->isImageAvail - sblock->isBootBlockSize))) {
		dprintf(DEBUG_SPEW, "bad bootblock size\n");
		return(0);
	}
	
	/*
	 * Compute the initial cursor value and residual byte count.
	 * XXX 7561698 this isn't how isImageOffset is documented to be used.
	 */
	cursor = ((off_t) sblock->isBootBlockSize + (off_t) sblock->isImageOffset) * (off_t) sblock->isImageGranule + offset;
	residual = (off_t) sblock->isImageAvail * (off_t) sblock->isImageGranule;

	/*
	 * Make  sure we don't try to go past the end of the blockdev. The code above is baroque
	 * and legacy enough that this additional check will just be a soft error.
	 */
	if (residual + cursor < cursor) {
		dprintf(DEBUG_SPEW, "overflow\n");
		return(0);
	}
	if (residual + cursor > spaceBytes) {
		residual = spaceBytes - cursor;
	}

	/*
	 * Scan the image space and build a list of visible images.
	 */
	for (imageCount = 0; imageCount < IMAGE_MAX_COUNT; imageCount++) {
		/* read the next image header */
		err = blockdev_read(bdev, &objHeader, cursor, sizeof(objHeader));
		if (err != sizeof(objHeader)) {
			dprintf(DEBUG_SPEW, "error reading image header @ 0x%llx\n", cursor);
			break;
		}

		/* sanity-check the header */
		if (objHeader.ihMagic != kImage3HeaderMagic)
			break;
		if (objHeader.ihSkipDistance > residual)
			break;
		if (objHeader.ihSkipDistance % sblock->isImageGranule)
			break;
		if ((uint32_t)(objHeader.ihBufferLength + sizeof(Image3ObjectHeader)) < objHeader.ihBufferLength)
			break; /* wraparound */
		if ((uint32_t)(objHeader.ihBufferLength + sizeof(Image3ObjectHeader)) > residual)
			break;

		/* it will do for now, make a note */
		imageInfo = malloc(sizeof(*imageInfo));
		
		/* build our private image info */
		imageInfo->bdev = bdev;
		imageInfo->devOffset = cursor;
		imageInfo->imageHandle = NULL;

		/* build the public image info */
		imageInfo->image_info.imageLength = objHeader.ihSignedLength;	/* overstated */
		imageInfo->image_info.imageAllocation = objHeader.ihBufferLength + sizeof(Image3ObjectHeader);
		imageInfo->image_info.imageType = objHeader.ihType;
		imageInfo->image_info.imagePrivateMagic = IMAGE3_IMAGE_INFO_MAGIC;
		imageInfo->image_info.imageOptions = (imageOptions & IMAGE_OPTION_MEMORY);
		imageInfo->image_info.imagePrivate = imageInfo;

		/* append to the list */
		list_add_tail(&images, &imageInfo->node);

		/* advance to the next image */
		if (objHeader.ihSkipDistance < objHeader.ihBufferLength) {
			imageCount++;   /* account for this image */
			break;	        /* image terminates normally */
                }
		cursor += objHeader.ihSkipDistance;
		residual -= objHeader.ihSkipDistance;
	}
	dprintf(DEBUG_SPEW, "%d images\n", imageCount);
	return(imageCount);
}

/*
 * image3_free_bdev
 *
 * Frees resources for any previously found images on the specified bdev.
 * Useful for starting afresh with a new image superblock.
 */
void
image3_free_bdev(struct blockdev *bdev)
{
	struct image3_info *img, *tmp;

	list_for_every_entry_safe(&images, img, tmp, struct image3_info, node) {
		if (img->image_info.imagePrivateMagic == IMAGE3_IMAGE_INFO_MAGIC &&
		    img->bdev == bdev) {
			list_delete(&img->node);
			free(img);
		}
	}
}

/*
 * image3_find
 *
 * Returns the image_info handle to the first image of the specified type.
 */
struct image_info *
image3_find(u_int32_t image_type)
{
	struct image3_info *image;

	list_for_every_entry(&images, image, struct image3_info, node) {
		if (image->image_info.imageType == image_type)
			return(&image->image_info);
	}

	return(NULL);
}

/*
 * image3_load
 *
 * Loads the image, with the DATA tag payload ultimately placed at *load_addr and its
 * size in *load_len.
 *
 * It is the caller's responsibility to ensure that the load address is specified
 * and that the buffer is large enough (image_info.imageAllocation).
 */
int
image3_load(struct image_info *image_info, const u_int32_t *types, u_int32_t count, u_int32_t *actual, void **load_addr, size_t *load_len)
{
	int			result;
	Image3ObjectHandle	objectHandle;
	void			*objectBuffer;
	size_t			objectSize;
	void			*payloadBuffer;
	size_t			payloadSize;
	bool			fromLocalStorage;
	bool			untrustedImage;
	bool			imageIncludesConstraints;
	bool			clearProduction;
	u_int32_t		validationFlags;
	u_int32_t		type;
	u_int32_t		actualType;
	u_int32_t		cnt;
	bool			matchedType;

	/* basic sanity on arguments */
	RELEASE_ASSERT(NULL != image_info);
	RELEASE_ASSERT(NULL != load_addr);
	RELEASE_ASSERT(NULL != load_len);
	RELEASE_ASSERT(0 != *load_len);

	untrustedImage = false;
	clearProduction = false;
	result = -1;
	objectHandle = 0;
	objectBuffer = *load_addr;
	objectSize = image_info->imageAllocation;

	/* refuse to operate on an object that is larger than the load buffer */
	if (objectSize > *load_len) {
		dprintf(DEBUG_INFO, "loaded image too large\n");
		platform_record_breadcrumb_int("image3_fail", kBCImg3TooLarge);

		goto out;
	}

	/* refuse to operate on an object buffer at zero */
	if (NULL == objectBuffer) {
		dprintf(DEBUG_INFO, "cannot load image with buffer at zero\n");
		platform_record_breadcrumb_int("image3_fail", kBCImg3NullPtr);
		goto out;
	}

	/*
	 * Fetch the object into the supplied (presumed safe) buffer.
	 */
	if (image3_load_copyobject(image_info, objectBuffer, objectSize))
		goto out;

	/* if we are just being asked to load the image whole, we're done here */
	if (image_info->imageOptions & IMAGE_OPTION_JUST_LOAD) {
		result = 0;
		goto out;
	}

	/*
	 * Instantiate the Image3 object.
	 */
	dprintf(DEBUG_SPEW, "instantiating image\n");
	if (0 != image3InstantiateFromBuffer(
		    &objectHandle,
		    objectBuffer,
		    objectSize,
		    false /* copy */)) {
		dprintf(DEBUG_INFO, "image instantiation failed\n");
		platform_record_breadcrumb_int("image3_fail", kBCImg3InstantiationFailed);
		goto out;
	}

	/*
	 * Assume that the object's type hint is the actual type.
	 */
	actualType = (*(Image3ObjectHeader **)objectHandle)->ihType;

	/*
	 * If the caller provided a list of types,
	 * check if the actual type matches one of 
	 * the types the caller will accept.
	 */
	if (count != 0) {
		matchedType = false;
		for (cnt = 0; cnt < count; cnt++) {
			if (types[cnt] == actualType) {
				matchedType = true;
				break;
			}
		}

		if (!matchedType) {
			dprintf(DEBUG_INFO, "image type not accepted by caller\n");
			platform_record_breadcrumb_int("image3_fail", kBCImg3UnacceptedType);
			goto out;
		}

		type = actualType;
	} else {
		type = IMAGE_TYPE_ANY;
	}

	/*
	 * Validate the image, either using the ticket library or with the
	 * included signature. If both local storage and memory are specified,
	 * local storage wins
	 */	
	fromLocalStorage =
		(image_info->imagePrivateMagic == IMAGE3_IMAGE_INFO_MAGIC &&
		 !(image_info->imageOptions & IMAGE_OPTION_MEMORY)) ||
		(image_info->imageOptions & IMAGE_OPTION_LOCAL_STORAGE);

	validationFlags = (fromLocalStorage ? kImage3ValidateLocalStorage : 0);
#if WITH_TICKET
	validationFlags |= (ticket_validation_required() ? kImage3ValidateRequireTicket	: 0);
#endif

	imageIncludesConstraints = false;

	if (0 != image3_load_validate_signature(objectHandle,
						type,
						(image_info->imageOptions & IMAGE_OPTION_REQUIRE_TRUST),
						validationFlags,
						&untrustedImage,
						&imageIncludesConstraints)) {
		/* signature validation failed and we are not willing to accept an untrusted image */
		platform_record_breadcrumb_int("image3_fail", kBCImg3SignatureValidationFail);
		goto out;
	}

	/*
	 * Apply additional constraints to the image that may further
	 * reject trust, if they can be provided by the image.
	 */
	if (imageIncludesConstraints) {
		if (0 != image3_load_validate_constraints(objectHandle,
							  type,
							  (image_info->imageOptions & IMAGE_OPTION_REQUIRE_TRUST),
							  (image_info->imageOptions & IMAGE_OPTION_GREATER_EPOCH),
							  &clearProduction,
							  &untrustedImage)) {
			/* constraint validation failed and we are not willing to accept an invalid image */
			dprintf(DEBUG_INFO, "image is not valid for this device\n");
			platform_record_breadcrumb_int("image3_fail", kBCImg3ConstraintValidationFail);
			goto out;
		}
	}
		
	/*
	 * By this point, the image is required to have been validated by either the ticket or 
	 * image3 signature.
	 */

	/*
	 * Find the payload.
	 */
	dprintf(DEBUG_SPEW, "looking for payload\n");
	payloadSize = 0;	/* any size is OK */
	if (0 != image3GetTagStruct(
			objectHandle,
			kImage3TagTypeData,
			&payloadBuffer,
			&payloadSize,
			0)) {
		dprintf(DEBUG_INFO, "image3GetTagStruct failed for payload lookup\n");
		platform_record_breadcrumb_int("image3_fail", kBCImg3GetTagStructFailed);
		goto out;
	}

#if WITH_AES
	/*
	 * Decrypt the payload data if required. Whether or not the payload is encrypted,
	 * on success it will be moved to the base of the buffer.
	 */
	if (0 != image3_load_decrypt_payload(objectHandle, 
					     payloadBuffer,
					     payloadSize,
					     objectBuffer,
					     untrustedImage)) {
		/* the image is encrypted but we can't decrypt it */
		platform_record_breadcrumb_int("image3_fail", kBCImg3DecryptFailed);
		goto out;
	}
#else
	memmove(objectBuffer, payloadBuffer, payloadSize);
#endif /* WITH_AES */
										    

	/*
	 * All done and OK; pass the actual payload size back to the caller.
	 */
	*load_len = payloadSize;
	result = 0;

	/*
	 * If the image is trusted, and it has asserted that we should clear the
	 * production status flag, do that now.
	 */
	if ((false == untrustedImage) && 
	    (true == clearProduction))
		security_set_production_override(true);

	/*
	 * Provide the actual type to the caller.
	 */
	if (actual != NULL) {
		*actual = actualType;
	}

out:
	/* clean up */
	if (objectHandle) {
		image3Discard(&objectHandle);
		objectHandle = 0;
	}

	/*
	 * If we failed, nuke the caller's pointers and clear the work buffer.
	 */
	if (result != 0) {
		dprintf(DEBUG_SPEW, "load failed, clearing object buffer\n");
		memset(*load_addr, 0, *load_len);
		*load_addr = 0;
		*load_len = 0;
	}
		
	return(result);
}

/*
 * image3_load_copyobject
 *
 * Fetch an image3 object from its current location into the supplied object buffer.
 */
static int
image3_load_copyobject(struct image_info *image_info, void *objectBuffer, size_t objectSize)
{
	struct image3_info	*image;
	int 			result;

	PROFILE_ENTER('ILC');

	RELEASE_ASSERT(NULL != objectBuffer);
	RELEASE_ASSERT(0 != objectSize);

	dprintf(DEBUG_SPEW, "loading image %p with buffer %p/%x\n", image_info, objectBuffer, (unsigned)objectSize);
	
	switch (image_info->imagePrivateMagic) {
	case IMAGE3_IMAGE_INFO_MAGIC:
		/*
		 * Fetch the image data from the bdev into the caller-supplied
		 * scratch buffer.
		 */
		dprintf(DEBUG_SPEW, "reading image from bdev\n");
		image = (struct image3_info *)(image_info->imagePrivate);
		result = blockdev_read(image->bdev,
				       objectBuffer,
				       image->devOffset,
				       objectSize);
		if (result != (int)objectSize) {
			dprintf(DEBUG_CRITICAL, "blockdev read failed with %d\n", result);
			platform_record_breadcrumb_int("image3_fail", kBCImg3BDEVReadFail);
			return(-1);
		}
		break;

	case IMAGE_MEMORY_INFO_MAGIC:
		/*
		 * Copy the image data from the memory buffer into the
		 * caller-supplied scratch buffer.
		 *
		 * If the caller does not supply a buffer, the image is assumed to be in
		 * 'trusted' memory already and is left where it is.
		 */
		if (((Image3ObjectHeader *)image_info->imagePrivate)->ihMagic != kImage3HeaderMagic) {
			dprintf(DEBUG_INFO, "image magic %x not image3\n",
			    ((Image3ObjectHeader *)image_info->imagePrivate)->ihMagic);
			platform_record_breadcrumb_int("image3_fail", kBCImg3BadMagic);
			return(-1);
		}
		if (objectBuffer != image_info->imagePrivate) {
			dprintf(DEBUG_SPEW, "reading image from %p/%x\n", image_info->imagePrivate, (unsigned) objectSize);
			memmove(objectBuffer, image_info->imagePrivate, objectSize);
		}
		break;

	default:
		dprintf(DEBUG_INFO, "unrecognised image source type %x\n", image_info->imagePrivateMagic);
		platform_record_breadcrumb_int("image3_fail", kBCImg3UnknownPrivateMagic);
		return(-1);
	}

	PROFILE_EXIT('ILC');

	return(0);
}

/*
 * image3_load_validate_signature
 *
 * Validate the signature on the object (if present).
 *
 * If the image was created from memory and the supplier didn't assert it was from
 * local storage, the signature won't have been personalised.
 */
static int
image3_load_validate_signature(Image3ObjectHandle objectHandle,
			       uint32_t expectedType,
			       bool requireTrust,
			       uint32_t validationFlags,
			       bool *untrustedImage,
			       bool *imageIncludesConstraints)
{
	int	result;

	PROFILE_ENTER('ILV');

	RELEASE_ASSERT(NULL != objectHandle);
	RELEASE_ASSERT(NULL != untrustedImage);

	/* try to validate the signature */
	result = image3ValidateSignature(objectHandle, expectedType, validationFlags, imageIncludesConstraints);

	switch (result) {
	case 0:
		/* image has a signature and we believe it's valid */
		dprintf(DEBUG_SPEW, "image trusted\n");
		break;
		
	case EPERM:
		/*
		 * The image is not signed or signature is invalid.
		 *
		 * If the image was created with 'enhanced security', this is fatal.  Otherwise
		 * tell the security system and see if it's OK for us to use the image.
		 *
		 * Note that the security call may adjust the state of the system in order
		 * to ensure that it is safe to run untrusted code before replying OK.
		 */
		dprintf(DEBUG_SPEW, "image not trusted\n");
		if (requireTrust || !security_validate_image(kSecurityImageUntrusted)) {
			/* policy says that we aren't allowed to use un-trusted images */
			dprintf(DEBUG_INFO, "image validation failed and untrusted images are not permitted\n");
			return(-1);
		}

		/*
		 * Security system says it's OK to load untrusted images, and the client
		 * hasn't explicitly asked for a trusted image, so let our caller know that
		 * this image is *not* trusted, but return success.
		 */
		*untrustedImage = true;
		dprintf(DEBUG_INFO, "untrusted image loaded\n");
		break;
		
	default:
		/* something went wrong */
		dprintf(DEBUG_INFO, "image validation returned %d\n", result);
		return(-1);
	}

	PROFILE_EXIT('ILV');

	return(0);
}

/*
 * Validate that constraints supplied by the image are met.
 *
 * This function currently requires that the image be considered trusted on entry.
 */						  
static int
image3_load_validate_constraints(Image3ObjectHandle objectHandle,
				 u_int32_t type,
				 bool requireTrust,
				 bool allowGreaterEpoch,
				 bool *clearProduction,
				 bool *untrustedImage)
{
	u_int64_t nonce;
	u_int8_t nonce_hash[20];

	PROFILE_ENTER('IVC');
	
	RELEASE_ASSERT(NULL != objectHandle);
	RELEASE_ASSERT(NULL != clearProduction);
	RELEASE_ASSERT(NULL != untrustedImage);

	/*
	 * Validate that the image has been signed with a certificate that meets
	 * the following constraints:
	 *     Security Domain of the cert matches the platform
	 *     Production Mode of the cert is allowed by the platform
	 *     Chip ID of the cert matches the platform
	 */

	/*
	 * Verify that the image's cert meets the platform's security domain constraints.
	 *
	 * Images must be signed for the security domain advertised by the platform.
	 */
	dprintf(DEBUG_SPEW, "verifying platform constraints\n");
	/* tag must be present in this case */
	if (0 != verifyNumberRelation(
		    objectHandle,
		    kImage3TagTypeSecurityDomain,
		    platform_get_security_domain(),
		    REL_EQUAL,
		    true /* tagIsRequired */,
		    true /* nestedImageOnly */,
		    NULL /* tagWasPresent */)) {

		dprintf(DEBUG_SPEW, "invalid security domain in certificate\n");
		goto verify_untrusted;
	}

	/*
	 * Verify that the image's cert meets the platform's production status constraints.
	 *
	 * If we are in production mode, the image's cert must have a production status tag
	 * and the tag's value must be correct.
	 */
	if (platform_get_raw_production_mode()) {
		if (0 != verifyNumberRelation(
			    objectHandle,
			    kImage3TagTypeProductionStatus,
			    kImage3SecurityProductionStatus,
			    REL_EQUAL,
			    true /* tagIsRequired */,
			    true /* nestedImageOnly */,
			    NULL /* tagWasPresent */)) {
 
			dprintf(DEBUG_SPEW, "invalid production status in certificate\n");
			goto verify_untrusted;
		}

		/*
		 * If the client is willing to consider an override of the device's production
		 * status, check for an override tag.
		 */
		if (0 != verifyIntersectingBits(
			    objectHandle,
			    kImage3TagTypeOverride,
			    kImage3OverrideProductionMode,
			    false /* tagIsRequired */,
			    true /* nestedImageOnly */,
			    clearProduction /* tagWasPresent */)) {

			dprintf(DEBUG_SPEW, "invalid override tag in certificate\n");
			goto verify_untrusted;
		}
	}
	
	/*
	 * Verify that the image's cert meets the platform's chip id constraints.
	 *
	 * Images must be signed for the chip id advertised by the platform.
	 * 
	 */
	if (0 != verifyNumberRelation(
		    objectHandle,
		    kImage3TagTypeChipType,
		    platform_get_chip_id(),
		    REL_EQUAL,
		    true /* tagIsRequired */,
		    true /* nestedImageOnly */,
		    NULL /* tagWasPresent */)) {

		dprintf(DEBUG_SPEW, "invalid chip type in certificate\n");
		goto verify_untrusted;
	}

	/*
	 * Verify that the image's type matches the requested
	 * type and that type is not IMAGE_TYPE_ANY.
	 */
	if ((type != IMAGE_TYPE_ANY) &&
	    (0 != verifyNumberRelation(
		    objectHandle,
		    kImage3TagTypeType,
		    type,
		    REL_EQUAL,
		    true /* tagIsRequired */,
		    false /* !nestedImageOnly */,
		    NULL /* tagWasPresent */))) {

		/*
		 * If the security system says it's OK to execute untrusted images,
		 * we say it's OK to execute the wrong type of image, too.
		 */
		dprintf(DEBUG_SPEW, "invalid image type\n");
		goto verify_untrusted;
	}

	/*
	 * Validate that the image meets the following optional constraints:
	 *     Security Epoch of the image must match the platform
	 *     Hardware Epoch of the image's cert must be valid on the platform
	 *     Board ID of the image much match the platform
	 *     ECID of the image must match the platform
	 */

	/*
	 * Verify that the image is compatible with the current epoch.
	 *
	 * If the image is created with explicit permission for epoch mismatch on the
	 * later side, be permissive, otherwise require an exact match.
	 */
	if (0 != verifyNumberRelation(
		    objectHandle,
		    kImage3TagTypeSecurityEpoch,
		    platform_get_security_epoch(),
		    (allowGreaterEpoch ? REL_GREATER_EQUAL : REL_EQUAL),
		    false /* !tagIsRequired */,
		    false /* !nestedImageOnly */,
		    NULL /* tagWasPresent */)) {

		dprintf(DEBUG_SPEW, "invalid security epoch\n");
		goto verify_untrusted;
	}

	/*
	 * Verify that the certificate is compatible with the current hardware epoch.
	 *
	 * The image must be signed with a certificate that has an epoch greater
	 * than or equal to the chip's minimum epoch.
	 */
	if (0 != verifyNumberRelation(
		    objectHandle,
		    kImage3TagTypeHardwareEpoch,
		    platform_get_hardware_epoch(),
		    REL_GREATER_EQUAL,
		    false /* !tagIsRequired */,
		    true /* nestedImageOnly */,
		    NULL /* tagWasPresent*/)) {

		dprintf(DEBUG_SPEW, "invalid hardware epoch\n");
		goto verify_untrusted;
	}

	/*
	 * Look for and check the current chip, board, ECID and nonce against any list supplied with the image.
	 */
	if (0 != verifyNumberRelation(
		    objectHandle,
		    kImage3TagTypeBoardType,
		    platform_get_board_id(),
		    REL_EQUAL,
		    false /* !tagIsRequired */,
		    false /* !nestedImageOnly */,
		    NULL /* tagWasPresent */)) {

		/*
		 * If the security system says it's OK to execute untrusted images,
		 * we say it's OK to execute images tagged for the wrong board too.
		 */
		dprintf(DEBUG_SPEW, "invalid board type\n");
		goto verify_untrusted;
	}

	if (0 != verifyNumberRelation(
		    objectHandle,
		    kImage3TagTypeUniqueID,
		    platform_get_ecid_id(),
		    REL_EQUAL,
		    platform_get_ecid_image_personalization_required() /* tagIsRequired? */,
		    false /* !nestedImageOnly */,
		    NULL /* tagWasPresent */)) {

		dprintf(DEBUG_SPEW, "invalid unique chip id\n");
		goto verify_untrusted;
	}

	nonce = platform_get_nonce();
	sha1_calculate((const void *)&nonce, sizeof(nonce), (void *)nonce_hash);

	if (0 != verifyMatchingBytes(
		    objectHandle,
		    kImage3TagTypeNonce,
		    nonce_hash,
		    sizeof(nonce_hash),
		    false /* !tagIsRequired */,
		    NULL /* tagWasPresent */)) {

		dprintf(DEBUG_SPEW, "invalid nonce\n");
		goto verify_untrusted;
	}

	PROFILE_EXIT('IVC');

	return(0);

verify_untrusted:
	/*
	 * A constraint has not been met.
	 *
	 * If the image was created with 'enhanced security', this is fatal.  Otherwise
	 * tell the security system and see if it's OK for us to use the image.
	 *
	 * Note that the security call may adjust the state of the system in order
	 * to ensure that it is safe to run untrusted code before replying OK.
	 */
	if (requireTrust || !security_validate_image(kSecurityImageUntrusted)) {
		/* policy says that we aren't allowed to use un-trusted images */
		dprintf(DEBUG_INFO, "image validation failed and untrusted images are not permitted\n");
		return(-1);
	}
	dprintf(DEBUG_CRITICAL, "image validation failed but untrusted images are permitted\n");
	*untrustedImage = true;

	return(0);
}

#if WITH_AES
/*
 * image3_load_decrypt_payload
 *
 * Decrypt the payload contents if they are encrypted and we can find a key
 * that matches.
 */
struct ivkey {
	u_int8_t IV[16];
	u_int8_t Key[32];
} __packed;

static int
image3_load_decrypt_payload(Image3ObjectHandle objectHandle,
			    void *payloadBuffer,
			    size_t payloadSize,
			    void *destinationBuffer,
			    bool untrustedImage)
{
	int			result;
	Image3TagKBAG		*keyBag;
	u_int32_t		platformKeyOpts;
	size_t			paddedPayloadSize;
	size_t			keybagSize;
	u_int32_t		keybagKeyOpts;
	size_t			keybagKeySize;
	struct ivkey		keybagIVKey;

	PROFILE_ENTER('IDP');

	/*
	 * Note that we cannot assert that the payload size is clean % 16.
	 * The image3 generation code guarantees that tags are padded such
	 * that there is always enough space for data items to be decrypted
	 * without touching data outside the tag, and that any data in a
	 * tag that's been encyrypted is padded out to suit, however the payload
	 * size that is visible is the actual byte size of the data.
	 */
	RELEASE_ASSERT(NULL != payloadBuffer);
	RELEASE_ASSERT(0 != payloadSize);

	/* initialise state */
	result = -1;
	memset(&keybagIVKey, 0, sizeof(keybagIVKey));

	/*
	 * Look for payload encryption keybags, see if there is one for us.
	 *
	 * If there is, unpack the key and use it and the decryption to move
	 * the payload to the head of the buffer.  If there are keybags but not one
	 * for us, that's an error.
	 *
	 * If there are no keybags or the image was untrusted, just copy the image directly.
	 */
	dprintf(DEBUG_SPEW, "checking for encrypted payload\n");
	for (int keybagIndex = 0; ; keybagIndex++) {

		/*
		 * Fetch the next keybag.
		 */
		keybagSize = sizeof(*keyBag);
		result = image3GetTagStruct(
			objectHandle,
			kImage3TagTypeKeybag,
			(void *)&keyBag,
			&keybagSize,
			keybagIndex);
		if (result == ENOENT) {
			if (keybagIndex == 0) {
				/* there are no keybags, payload is not encrypted; just move the
				   payload to the destination and return */
				memmove(destinationBuffer, payloadBuffer, payloadSize);
				result = 0;
				goto out;
			}
			/* there were keybags but not one we recognised */
			dprintf(DEBUG_INFO, "failed to find a keybag for this system\n");
			goto out;
		}

		/*
		 * If the image is untrusted but encrypted, refuse to decrypt it.
		 */
		if (untrustedImage) {
			dprintf(DEBUG_INFO, "encrypted image found but we do not trust it\n");
			goto out;
		}

		/* work out how big the key is going to be */
		keybagKeyOpts = AES_KEY_TYPE_USER;
		switch (keyBag->kbKeySize) {
		case 128 : keybagKeyOpts |= AES_KEY_SIZE_128; break;
		case 192 : keybagKeyOpts |= AES_KEY_SIZE_192; break;
		case 256 : keybagKeyOpts |= AES_KEY_SIZE_256; break;
		default:
			/* not a valid AES key size, bail */
			dprintf(DEBUG_INFO, "AES key size %d not supported/valid\n", keyBag->kbKeySize);
			goto out;
		}
		keybagKeySize = keyBag->kbKeySize / 8;
		RELEASE_ASSERT(keybagKeySize <= sizeof(keybagIVKey.Key));

		/* copy the IV from the keybag */
		memcpy(&keybagIVKey.IV, keyBag->kbIVBytes, sizeof(keybagIVKey.IV));

		/* copy the key from the keybag */
		memcpy(&keybagIVKey.Key, keyBag->kbKeyBytes, keybagKeySize);

		switch (keyBag->kbSelector) {
		case kImage3KeybagSelectorNoKey:
			/* key is not encrypted, just use it as-is */
			dprintf(DEBUG_SPEW, "using non-wrapped AES key\n");
			break;
		default:
			/* ask platform for key details */
			platformKeyOpts = 0;
			if (platform_translate_key_selector(keyBag->kbSelector, &platformKeyOpts) != 0) {
				/* we don't recognise this key, spin and try again */
				dprintf(DEBUG_SPEW, "key selector %d not recognised\n", keyBag->kbSelector);
				continue;
			}
			dprintf(DEBUG_SPEW, "using key selector %d\n", keyBag->kbSelector);
			if (aes_cbc_decrypt(&keybagIVKey,
					    &keybagIVKey,
					    sizeof(keybagIVKey.IV) + keybagKeySize,
					    platformKeyOpts,
					    0,
					    0)) {
				dprintf(DEBUG_INFO, "cannot decrypt image - hardware AES keys disabled\n");
				goto out;
			}
			break;
		}

		/*
		 * Decrypt the payload.
		 *
		 * Note that if the claimed payload size is not mod16, the buffer
		 * has been padded by the image3 generator.  Note also that at this
		 * point we trust the container.
		 */
		if ((payloadSize % 16) != 0) {
			paddedPayloadSize = payloadSize + (16 - (payloadSize % 16));
		} else {
			paddedPayloadSize = payloadSize;
		}
		dprintf(DEBUG_SPEW, "AES operation 0x%zx bytes\n", paddedPayloadSize);
		result = aes_cbc_decrypt(payloadBuffer,
					 destinationBuffer,
					 paddedPayloadSize,
					 keybagKeyOpts,
					 &keybagIVKey.Key,
					 &keybagIVKey.IV);

		/* clear the iv & key from memory */
		memset(&keybagIVKey, 0, sizeof(keybagIVKey));

		if (result)
			dprintf(DEBUG_INFO, "cannot decrypt image - unexpected AES failure");
		goto out;
	}
	dprintf(DEBUG_INFO, "could not find a compatible keybag\n");
out:
	PROFILE_EXIT('IDP');
	return(result);
}
#endif /* WITH_AES */

/*
 * Verify that if the image (and optionally the signing cert's embedded image) has the specified
 * tag, that there is some intersection between its value and the supplied bits.
 */
static int
verifyIntersectingBits(
	Image3ObjectHandle objectHandle,
	u_int32_t withTag,
	u_int64_t withBits,
	bool tagIsRequired,
	bool nestedImageOnly,
	bool *tagWasPresent)
{
	Image3ObjectHandle	nestedObjectHandle;
	u_int64_t		value;
	int			result;
	int			nestedResult;
	bool			wasPresent;
	bool			nestedWasPresent;

	result     = 0;
	wasPresent = false;

	/*
	 * Check against the nested object handle first if one exists by recursively calling ourself.
	 *  Since tagIsRequired is not set, nestedResult will be 0 if the tag was not present or the relation is valid.
	 *  nestedResult will only be EPERM if the tag was present but the relation was not valid.
	 *  nestedWasPresent will indicate if the tag was present.
	 */
	if ((nestedObjectHandle = image3GetNestedImage(objectHandle))) {
		nestedResult = verifyIntersectingBits(nestedObjectHandle, withTag, withBits, false, false, &nestedWasPresent);
	} else {
		nestedResult     = 0;
		nestedWasPresent = false;
	}

	if (nestedImageOnly)
		goto out;

	/* check the image */
	switch ((result = image3GetTagUnsignedNumber(
			objectHandle,
			withTag,
			&value, 0))) {
	case ENOENT:
		result = 0;
		break;
		
	case 0:
		wasPresent = true;
		/* there are stipulated domains, ensure that we intersect */
		if (!(value & withBits)) {
			dprintf(DEBUG_INFO, "tag %c%c%c%c value 0x%x missing required bits 0x%x\n",
				UNTAG(withTag), (unsigned) value, (unsigned) withBits);
			result = EPERM;
		}
		break;
		
	default:
		/* something bad happened */
		dprintf(DEBUG_INFO, "unexpected error %d while attempting to find tag %c%c%c%c\n", result, UNTAG(withTag));
		break;
	}
out:
	/* let nestedResult override result */
	if (nestedResult != 0)
		result = nestedResult;

	/* merge wasPresent with nestedWasPresent */
	wasPresent |= nestedWasPresent;

	/* if requested, provide wasPresent back to the caller */
	if (tagWasPresent)
		*tagWasPresent = wasPresent;

	/* if no tags of the specified type, permission depends on the tag requirement */
	if (tagIsRequired && !wasPresent)
		result = EPERM;

	return(result);
}

/*
 * Verify that, if any tags matching withTag are present, that one of them has the testValue.
 *
 * In the case where the nested image is present, the same applies to both.
 */
static int
verifyNumberRelation(
	Image3ObjectHandle objectHandle,
	u_int32_t withTag,
	u_int64_t testValue,
	enum matching_relation relation,
	bool tagIsRequired,
	bool nestedImageOnly,
	bool *tagWasPresent)
{
	Image3ObjectHandle	nestedObjectHandle;
	u_int64_t		value;
	int			result;
	int			nestedResult;
	int			tagInstance;
	bool			wasPresent;
	bool			nestedWasPresent;

	result     = 0;
	wasPresent = false;

	/*
	 * Check against the nested object handle first if one exists by recursively calling ourself.
	 *  Since tagIsRequired is not set, nestedResult will be 0 if the tag was not present or the relation is valid.
	 *  nestedResult will only be EPERM if the tag was present but the relation was not valid.
	 *  nestedWasPresent will indicate if the tag was present.
	 */
	if ((nestedObjectHandle = image3GetNestedImage(objectHandle))) {
		nestedResult = verifyNumberRelation(nestedObjectHandle, withTag, testValue, relation, false, false, &nestedWasPresent);
	} else {
		nestedResult     = 0;
		nestedWasPresent = false;
	}

	if (nestedImageOnly)
		goto out;

	/*
	 * Iterate over instances of the tag, looking for a match.
	 */
	wasPresent = false;
	for (tagInstance = 0; ; tagInstance++) {
		switch ((result = image3GetTagUnsignedNumber(
				objectHandle,
				withTag,
				&value,
				tagInstance))) {
		case ENOENT:
			if (tagInstance == 0) {
				result = 0;
			} else {
				/* no matching tags of the specified type, permission is denied */
				result = EPERM;
			}
			goto out;

		case 0:
			wasPresent = true;
			/* if the value matches, we are done */
			switch (relation) {
			case REL_EQUAL:
				if (value == testValue)
					goto out;
				break;
			case REL_GREATER_EQUAL:
				if (value >= testValue)
					goto out;
				break;
			default:
				/* whatever */
				break;
			}
			break;

		default:
			dprintf(DEBUG_INFO, "unexpected error %d while searching for tag %c%c%c%c instance %d\n",
			    result, UNTAG(withTag), tagInstance);
			goto out;
		}
	}
out:
	/* let nestedResult override result */
	if (nestedResult != 0)
		result = nestedResult;

	/* merge wasPresent with nestedWasPresent */
	wasPresent |= nestedWasPresent;

	/* If requested, provide wasPresent back to the caller */
	if (tagWasPresent)
		*tagWasPresent = wasPresent;

	/* if no tags of the specified type, permission depends on the tag requirement */
	if (tagIsRequired && !wasPresent)
		result = EPERM;

	return(result);
}

/*
 * Verify that, if any tags matching withTag are present, that one of them has the testBytes.
 *
 */
static int
verifyMatchingBytes(
	Image3ObjectHandle objectHandle,
	u_int32_t withTag,
	const void *testBytes,
	size_t testBytesLength,
	bool tagIsRequired,
	bool *tagWasPresent)
{
	void			*bytes;
	int			result;
	int			tagInstance;
	bool			wasPresent;

	result     = 0;
	wasPresent = false;
	bytes = NULL;

	/*
	 * Iterate over instances of the tag, looking for a match.
	 */
	wasPresent = false;
	for (tagInstance = 0; ; tagInstance++) {
		switch ((result = image3GetTagStruct(
				objectHandle,
				withTag,
				(void **)&bytes,
				(size_t *)&testBytesLength,
				tagInstance))) {
		case ENOENT:
			if (tagInstance == 0) {
				result = 0;
			} else {
				/* no matching tags of the specified type, permission is denied */
				result = EPERM;
			}
			goto out;

		case 0:
			wasPresent = true;
			/* if the value matches, we are done */
			if (memcmp(testBytes, (const void *)bytes, testBytesLength) == 0)
				goto out;
			break;

		default:
			dprintf(DEBUG_INFO, "unexpected error %d while searching for tag %c%c%c%c instance %d\n",
			    result, UNTAG(withTag), tagInstance);
			goto out;
		}
	}
out:
	/* If requested, provide wasPresent back to the caller */
	if (tagWasPresent)
		*tagWasPresent = wasPresent;

	/* if no tags of the specified type, permission depends on the tag requirement */
	if (tagIsRequired && !wasPresent)
		result = EPERM;

	return(result);
}

/*
 * image3_dump_list
 *
 * Print the list of currently-known images.
 */
void
image3_dump_list(bool detailed)
{
	struct image3_info *image;

	list_for_every_entry(&images, image, struct image3_info, node) {

		printf("image %p: bdev %p type %c%c%c%c offset 0x%llx",
		    image,
		    image->bdev,
		    UNTAG(image->image_info.imageType),
		    image->devOffset);
		if (image->image_info.imageLength > 0) {
			printf(" len 0x%x",
			    image->image_info.imageLength);
		}
		printf("\n");
	}
}

/*
 * Support for the Image3 core library.
 */
int
image3AESDecryptUsingLocalKey(void *buffer, size_t length)
{
#if WITH_AES
	unsigned char	derivedSeed[16] = {0xdb, 0x1f, 0x5b, 0x33, 0x60, 0x6c, 0x5f, 0x1c,
					   0x19, 0x34, 0xaa, 0x66, 0x58, 0x9c, 0x06, 0x61};
	unsigned char	derivedKey[16];

	/* derive the key */
	if (aes_cbc_encrypt(derivedSeed, derivedKey, 16, AES_KEY_SIZE_128 | AES_KEY_TYPE_UID0, NULL, NULL))
		return(-1);

	/* and decrypt with it */
	if (aes_cbc_decrypt(buffer, buffer, length, AES_KEY_SIZE_128 | AES_KEY_TYPE_USER, derivedKey, NULL))
		return(-1);

	/* clear the derived key from memory */
	memset(derivedKey, 0, sizeof(derivedKey));
	return(0);
#else
	/* with no AES/local keys, we have no personalisation */
	dprintf(DEBUG_INFO, "image3: no AES, cannot de-personalise\n");
	return(-1);
#endif
}

void
image3SHA1Generate(void *dataBuffer, size_t dataSize, void *hashBuffer)
{
	sha1_calculate(dataBuffer, dataSize, hashBuffer);
}

int
image3PKIVerifyHash(
        void *hashBuffer,
	size_t hashSize,
	void *signedHashBuffer,
	size_t signedHashSize,
	void *certBlobBuffer,
	size_t certBlobSize,
	void **certCustomData,
	size_t *certCustomDataSize)
{
	int result = EPERM;

#if WITH_PKI
	if (!verify_signature_with_chain(certBlobBuffer, certBlobSize,
	       	  			 signedHashBuffer, signedHashSize, hashBuffer, hashSize,
					 certCustomData, certCustomDataSize, NULL, NULL))
		result = 0;
#else
	/* with no PKI, a signed image cannot be trusted */
	dprintf(DEBUG_INFO, "image3: cannot verify hash, no PKI support\n");
#endif

	return(result);
}

int
image3TicketVerifyHash(void *hashBuffer, size_t hashSize, uint32_t imageType, uint32_t expectedType)
{
	bool	untrusted = false;
	bool	result = false;

#if WITH_TICKET
	result = ticket_validate_image3(imageType, expectedType, hashBuffer, hashSize, &untrusted);
#endif

	if (result && !untrusted)
		return(0);

	if (result && untrusted)
		return(EPERM);

	return(EINVAL);
}
	
/*
 * Memory allocator
 *
 * Use calloc to guarantee zero-filled memory
 */
void *
image3Malloc(size_t size)
{
	return calloc(size, 1);
}

/*
 * Memory de-allocator
 */
void
image3Free(void *ptr, size_t size __unused)
{
	free(ptr);
}

