/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* Current implementation: Image4 Spec 1.0 */

#include <corecrypto/ccsha1.h>
#include <debug.h>
#include <drivers/aes.h>
#include <Img4Decode.h>
#include <lib/blockdev.h>
#include <lib/cksum.h>
#include <lib/image.h>
#include <lib/nonce.h>
#include <libDER/asn1Types.h>
#include <libDER/DER_Decode.h>
#include <platform.h>
#include <platform/chipid.h>
#include <sys/errno.h>
#include <sys/hash.h>
#include <sys/security.h>
#include <stdlib.h>
#include <string.h>
#include "image4_partial.h"
#include "image4_wrapper.h"
#include <lib/image4_spi.h>
#include <platform/breadcrumbs.h>

#if !WITH_CORECRYPTO
# error "libImg4Decode requires corecrypto integration"
#endif

#define AES_KEY_SIZE_BYTES_256	(32)

#define UNTAG(x)		(((x) >> 24) & 0xff),(((x) >> 16) & 0xff),(((x) >> 8) & 0xff),((x) & 0xff)

typedef enum matching_relation {
	REL_EQUAL,
	REL_GREATER_EQUAL
} matching_relation_t;

struct enviornment_properties {
	/* information passed from enviornment to image4 validation code */
	uint64_t	chip_id;
	uint64_t	unique_chip_id;
	uint64_t	chip_epoch;
	uint64_t	security_domain;
	uint64_t	board_id;
	bool		production_status;
	bool		security_mode;
	bool		local_boot;
	bool		verify_manifest_hash;
	bool		verify_nonce_hash;
	uint64_t	boot_nonce;
	uint8_t		boot_manifest_hash[HASH_OUTPUT_SIZE];
	bool		valid_boot_manifest_hash;
};

struct image_properties {
	/* information/actions passed by image4 validation code */
	bool		demote_production_status;
	bool		enable_keys;
	bool		allow_mix_n_match;
	bool		effective_production_status;
	bool		valid_effective_production_status;
	bool		effective_security_mode;
	bool		valid_effective_security_mode;
	uint8_t		manifest_hash[HASH_OUTPUT_SIZE];
	bool		valid_manifest;	
	bool		manifest_hash_verified;
	uint8_t		object_digest[HASH_OUTPUT_SIZE];
};

struct image4_info {
	struct list_node			node;
	struct blockdev				*bdev;			/* device the image is on */
	off_t					devOffset;		/* where on the bdev it is */
	struct image_info			image_info;		/* public image_info */
};

struct image4_wrapper_context {
	const Img4				*img4;			/* parsed image4, from libImage4Decode */
	const struct enviornment_properties	*env_properties;	/* info provided by enviornment */
	struct image_properties			*img_properties;	/* info gathered from parsed image */
};

/*
 * image4 callback mechanism used by some targets to copy image4 properties into the 
 * device tree. See image4_spi.h for additional documentation
 */
static struct {
	bool     capture_enabled;

	image4_start_capture_callback start_cb;
	image4_boolean_property_callback bool_cb;
	image4_integer_property_callback int_cb;
	image4_string_property_callback string_cb;
	image4_validity_callback validity_cb; 
} image4_callbacks;

struct image4_keybag {
	u_int32_t	kbSelector;
#define kImage4KeybagSelectorNoKey		(0)
#define kImage4KeybagSelectorChipUnique		(1)
#define kImage4KeybagSelectorChipUniqueDev	(2)
	u_int32_t	kbKeySize;
	u_int8_t	kbIVBytes[AES_BLOCK_SIZE];
	u_int8_t	kbKeyBytes[AES_KEY_SIZE_BYTES_256];
};

static struct list_node images = LIST_INITIAL_VALUE(images);

static int image4_load_copyobject(struct image_info *image_info, void *objectBuffer, size_t objectSize);
static int image4_validate_property_callback_interposer(uint32_t tag, const Img4Property *value, uint32_t propertyType, void *context);
static int image4_validate_property_callback(uint32_t tag, const Img4Property *value, uint32_t propertyType, void *context);
static int image4_load_decrypt_payload(Img4 *img4, void *payloadBuffer, size_t payloadSize);
static const Img4DecodeImplementation *image4_hash_init(void);

// chain_validation in libImg4Decode expects this to be declared by client
const struct ccdigest_info *sha1_digest_info;
const struct ccdigest_info *sha384_digest_info;

/*
 * image4_process_superblock
 *
 * Searches the given bdev looking for image4 images.  Returns a count of found
 * images.
 *
 * Image4 objects are in DER format. They are simply concatenated in
 * the blockdev, so we need to partially decode each object's header,
 * and repeatedly skip over them until we hit an EOC (end-of-content)
 * tag or fail to parse.
 */
int
image4_process_superblock(void *sblock, struct blockdev *bdev, off_t offset, uint32_t imageOptions)
{
	uint8_t *buf = malloc(IMAGE4_ID_BYTES);
	int count;
	dprintf(DEBUG_SPEW, "image4_process_superblock\n");
	for (count = 0; count < IMAGE_MAX_COUNT; ++count) {
		struct image4_info *info;
		uint32_t type, size;
		off_t residual;
		uint32_t buf_bytes;

		// Truncate read size to the end of bdev.
		if ((uint64_t) offset >= bdev->total_len) {
			dprintf(DEBUG_SPEW, "End of bdev\n");
			break;
		}
		residual = bdev->total_len - offset;

		// Don't read partial headers
		if (residual < IMAGE4_ID_BYTES) {
			dprintf(DEBUG_SPEW, "End of bdev\n");
			break;
		}

		buf_bytes = residual;
		// Truncate read size to IMAGE4_ID_BYTES.
		if (buf_bytes > IMAGE4_ID_BYTES)
			buf_bytes = IMAGE4_ID_BYTES;

		if (sblock != NULL) {
			// If this is the first loop, grab bytes from the
			// block already provided by the caller.
			memcpy(buf, sblock, buf_bytes);
			sblock = NULL;
		} else {
			// Otherwise, fetch the next header from bdev.
			// Rely on bdev to detect out of range reads.
			int err = blockdev_read(bdev, buf, offset, buf_bytes);
			if (err != IMAGE4_ID_BYTES) {
				dprintf(DEBUG_SPEW,
					"Error reading image @0x%llx: %d\n",
					offset, err);
				break;
			}
		}

		// Check if it's an Image4 header and get the type.
		if (image4_get_partial(buf, buf_bytes, &type, &size) != 0) {
			dprintf(DEBUG_SPEW, "Not Image4 @ 0x%llx\n", offset);
			break;
		}

		// Images claiming to overflow their block device are up to no good
		if (size > (uint64_t)residual) {
			dprintf(DEBUG_SPEW, "Size overflows block device @ 0x%llx\n", offset);
			break;
		}

		// Prepare image4_info.
		info = malloc(sizeof(*info));
		info->bdev = bdev;
		info->devOffset = offset;
		info->image_info.imageLength = size;
		info->image_info.imageAllocation = size;
		info->image_info.imageType = type;
		info->image_info.imagePrivateMagic = IMAGE4_IMAGE_INFO_MAGIC;
		info->image_info.imageOptions = imageOptions;
		info->image_info.imagePrivate = info;

		// Append to the list
		list_add_tail(&images, &info->node);
		
		// Advance to the next image.
		offset += size;
	}
	free(buf);
	return count;
}

/*
 * image4_free_bdev
 *
 * Frees resources for any previously found images on the specified bdev.
 * Useful for starting afresh with a new image superblock.
 */
void
image4_free_bdev(struct blockdev *bdev)
{
	struct image4_info *img, *tmp;

	list_for_every_entry_safe(&images, img, tmp, struct image4_info, node) {
		if (img->image_info.imagePrivateMagic == IMAGE4_IMAGE_INFO_MAGIC &&
		    img->bdev == bdev) {
			list_delete(&img->node);
			free(img);
		}
	}
}

/*
 * image4_find
 *
 * Returns the image_info handle to the first image of the specified type.
 */
struct image_info *
image4_find(u_int32_t image_type)
{
	struct image4_info *image;

	list_for_every_entry(&images, image, struct image4_info, node) {
		if (image->image_info.imageType == image_type)
			return(&image->image_info);
	}

	return(NULL);
}

/*
 * image4_load
 *
 * Loads the image, with the payload ultimately placed at *load_addr and its
 * size in *load_len.
 *
 * It is the caller's responsibility to ensure that the load address is specified
 * and that the buffer is large enough (image_info.imageAllocation).
 */
int
image4_load(struct image_info *image_info, const u_int32_t *types, u_int32_t count, u_int32_t *actual, void **load_addr, size_t *load_len)
{
	int				result = -1;
	void				*objectBuffer = NULL;
	size_t				originalObjectSize = 0;
	size_t				correctedObjectSize = 0;
	Img4				img4;
	DERReturn			ret;
	DERItem				item;
	void				*payloadBuffer;
	size_t				payloadSize;
	bool				trustedImage = false; /* treat image as untrusted to start with */
	bool				matchedType;
	u_int32_t			actualType;
	u_int32_t			cnt;
	struct image4_wrapper_context	image4_wrapper_context;
	struct enviornment_properties	env_properties;
	struct image_properties		img_properties;
	const Img4DecodeImplementation *implementation;

	/* basic sanity on arguments */
	RELEASE_ASSERT(NULL != image_info);
	RELEASE_ASSERT(NULL != load_addr);
	RELEASE_ASSERT(NULL != load_len);

	/* refuse to operate on an object that is larger than the load buffer */
	if (image_info->imageAllocation > *load_len) {
		dprintf(DEBUG_INFO, "loaded image too large\n");
		platform_record_breadcrumb_int("image4_load_fail", 1);
		goto out;
	}

	/* refuse to operate on an object buffer at zero */
	if (NULL == *load_addr) {
		dprintf(DEBUG_INFO, "cannot load image with buffer at zero\n");
		platform_record_breadcrumb_int("image4_load_fail", kBCImg4NullPtr);
		goto out;
	}

	/* initialize locals */
	bzero((void *)&img_properties, sizeof(img_properties));

	/* initialize enviornment object */
	bzero((void *)&env_properties, sizeof(env_properties));
	env_properties.chip_epoch = (uint64_t)platform_get_hardware_epoch();
	env_properties.chip_id = (uint64_t)platform_get_chip_id();
	env_properties.board_id = (uint64_t)platform_get_board_id();
	env_properties.unique_chip_id = platform_get_ecid_id();
	env_properties.security_domain = (uint64_t)platform_get_security_domain();
	env_properties.production_status = platform_get_raw_production_mode();
	env_properties.security_mode = platform_get_secure_mode();
	env_properties.local_boot = ((image_info->imageOptions & IMAGE_OPTION_LOCAL_STORAGE) == IMAGE_OPTION_LOCAL_STORAGE);
	env_properties.verify_nonce_hash = ((image_info->imageOptions & IMAGE_OPTION_NEW_TRUST_CHAIN) == IMAGE_OPTION_NEW_TRUST_CHAIN);
	env_properties.verify_manifest_hash = !env_properties.verify_nonce_hash && platform_get_mix_n_match_prevention_status();
	env_properties.valid_boot_manifest_hash = (platform_get_boot_manifest_hash(env_properties.boot_manifest_hash) == 0);
	if (env_properties.verify_nonce_hash && !env_properties.local_boot)
		env_properties.boot_nonce = platform_get_nonce();

	/* setup for image4 library */
	implementation = image4_hash_init();

	/*
	 * Fetch the object into the supplied (presumed safe) buffer.
	 */
	objectBuffer = *load_addr;
	originalObjectSize = image_info->imageAllocation;
	if (image4_load_copyobject(image_info, objectBuffer, originalObjectSize))
		goto out;

	/* if we are just being asked to load the image whole, we're done here */
	if ((image_info->imageOptions & IMAGE_OPTION_JUST_LOAD) == IMAGE_OPTION_JUST_LOAD) {
		result = 0;
		goto out;
	}

	/* Correct image object length, libImg4Decode expects exact length passed from caller.
	 * If there are extra bytes at the end, erase them.
	 */
	if (image4_get_partial((const void *)objectBuffer, IMAGE4_ID_BYTES, NULL, (uint32_t *)&correctedObjectSize) != 0) {
		dprintf(DEBUG_INFO, "Error parsing Image4 header\n");
		platform_record_breadcrumb_int("image4_load_fail", kBCImg4NullPtr);
		goto out;
	}
	if (correctedObjectSize > originalObjectSize) {
		dprintf(DEBUG_INFO, "Truncated Image4: %u vs %u\n", (unsigned) correctedObjectSize, (unsigned) originalObjectSize);
		platform_record_breadcrumb_int("image4_load_fail", kBCImg4Truncated);
		goto out;
	}
	bzero((void *)(objectBuffer + correctedObjectSize), (originalObjectSize - correctedObjectSize));

	/*
	 * Instantiate the Image4 object.
	 */
	dprintf(DEBUG_SPEW, "instantiating image\n");
	ret = Img4DecodeInit((const DERByte *) objectBuffer,
			     (DERSize) correctedObjectSize,
			     &img4);
	if (ret != DR_Success) {
		dprintf(DEBUG_INFO, "image instantiation failed, ret: %d\n", (int)ret);
		platform_record_breadcrumb_int("image4_load_fail", kBCImg4DecodeInitFail);

		goto out;
	}

	/*
	 * Assume that the object's payload type is the actual type.
	 */
	Img4DecodeGetPayloadType(&img4, &actualType);

	dprintf(DEBUG_SPEW, "actualType = %x\n", actualType);
	if (image4_callbacks.start_cb && image4_callbacks.start_cb(actualType)) {
		// Start a capture -- clear out any existing captured state / validity
		if (!image4_callbacks.validity_cb)
			panic("image4_callbacks.validity_cb is NULL");
		image4_callbacks.validity_cb(false);
		image4_callbacks.capture_enabled = true;
	} else {
		image4_callbacks.capture_enabled = false;
	}

	/*
	 * If the caller provided a list of types,
	 * check if the actual type matches one of 
	 * the types the caller will accept.
	 */
	if (count != 0) {
		matchedType = false;
		for (cnt = 0; cnt < count; cnt++) {
			dprintf(DEBUG_SPEW, "types[%x] = %x\n", cnt, types[cnt]);
			if (types[cnt] == actualType) {
				matchedType = true;
				break;
			}
		}

		if (!matchedType) {
			dprintf(DEBUG_INFO, "image type not accepted by caller\n");
			platform_record_breadcrumb_int("image4_load_fail", kBCImg4ImageTypeMismatch);

			goto out;
		}
	}

	/*
	 * Check if image contains a manifest
	 */
	ret = Img4DecodeManifestExists(&img4, &img_properties.valid_manifest);
	if ((ret != DR_Success) || !img_properties.valid_manifest) {
		dprintf(DEBUG_INFO, "failed manifest exists check, ret: %d, contain_manifest: %d\n", (int)ret, img_properties.valid_manifest);
		platform_record_breadcrumb_int("image4_load_fail", kBCImg4ManifestInvalid);

		goto check_untrusted;
	}

        /* At this point, we expect img_properties to be updated with useful values */

	/*
	 * Save context info to be passed back to the callbacks from the library
	 */
	image4_wrapper_context.img4 = (const Img4 *) &img4;
	image4_wrapper_context.env_properties = (const struct enviornment_properties *) &env_properties;
	image4_wrapper_context.img_properties = &img_properties;

	/* 
	 * Image 4 object trust evaluation: steps 7 to 16 
	 */
	ret = Img4DecodePerformTrustEvaluatation(actualType,
						 &img4,
						 (Img4DecodeValidatePropertyCallback) image4_validate_property_callback_interposer,
						 implementation,
						 (void *) &image4_wrapper_context);
	if (ret != DR_Success) {
		dprintf(DEBUG_INFO, "trust evalulation failed, ret: %d\n", (int)ret);
		platform_record_breadcrumb_int("image4_load_fail", kBCImg4TrustEvalFail);

		goto check_untrusted;
	}

	/* 
	 * Save object manifest hash
	 */
	RELEASE_ASSERT(Img4DecodeCopyManifestDigest((const Img4 *) &img4, img_properties.manifest_hash, HASH_OUTPUT_SIZE, implementation) == DR_Success);

	/*
	 * Except first boot stage, each boot-stage is required to verify boot manifest hash if mix-n-match prevention is enforced by previous stage.
	 * If AllowMixNMatch tag found true, it will break the chain of enforcing mix-n-match prevention.
	 * If previous stage didn't passed a valid boot manifest hash, skip verifying manifest hash, and tag image untrusted.
	 * First stage: ROM, or iBoot when performing restore from recovery mode or LLB in DFU mode.
	 */
	if (env_properties.verify_manifest_hash && !img_properties.allow_mix_n_match) {
		if (!env_properties.valid_boot_manifest_hash || (0 != memcmp((const void *)env_properties.boot_manifest_hash, (const void *)img_properties.manifest_hash, HASH_OUTPUT_SIZE))) {
			dprintf(DEBUG_INFO, "invalid boot manifest hash, or boot manifest hash failed to match\n");
			platform_record_breadcrumb_int("image4_load_fail", kBCImg4BootManifestFail);

			goto check_untrusted;
		}

		img_properties.manifest_hash_verified = true;
	}

	/* 
	 * Check effective production status still matches current state of the device,
	 * if effective production status is valid and security mode
	 */
	if (img_properties.valid_effective_production_status) {
		if (img_properties.effective_production_status != security_get_effective_production_status(img_properties.demote_production_status)) {
			dprintf(DEBUG_INFO, "effective production status failed to match\n");
			platform_record_breadcrumb_int("image4_load_fail", kBCImg4ProdStatusMismatch);

			goto check_untrusted;
		}
	}

	/* 
	 * Check effective secure mode still matches current state of the device,
	 * if effective secure mode is valid
	 */
	if (img_properties.valid_effective_security_mode) {
		if (img_properties.effective_security_mode != platform_get_secure_mode()) {
			dprintf(DEBUG_INFO, "effective security mode failed to match\n");
			platform_record_breadcrumb_int("image4_load_fail", kBCImg4SecureModeMismatch);

			goto check_untrusted;
		}
	}

	/* Mark image as trusted now */
	trustedImage = true;

check_untrusted:
	/*
	 * The image is not signed or signature is invalid or a constraint is not met.
	 *
	 * If the image was created with 'enhanced security', this is fatal.  Otherwise
	 * tell the security system and see if it's OK for us to use the image.
	 *
	 * Note that the security call may adjust the state of the system in order
	 * to ensure that it is safe to run untrusted code before replying OK.
	 */
	if (!trustedImage) {
		/* Rollback image and enviornment properites updated by image validation callback */
		img_properties.enable_keys = false;
		img_properties.demote_production_status = false;
		img_properties.effective_production_status = false;
		img_properties.effective_security_mode = false;
		img_properties.allow_mix_n_match = false;
		img_properties.valid_effective_production_status = false;
		img_properties.valid_effective_security_mode = false;

#if WITH_UNTRUSTED_EXECUTION_ALLOWED
		/* Only ROM allows untrusted execution if boot-config with TEST_MODE is selected and part is fused non-secure ('S' bit cleared) */
		if (((image_info->imageOptions & IMAGE_OPTION_REQUIRE_TRUST) == 0) && !platform_get_secure_mode()) {
			dprintf(DEBUG_INFO, "untrusted execution allowed, continuing\n");
		}
		else 
#endif		
		{
			/* policy says that we aren't allowed to use un-trusted images */
			dprintf(DEBUG_INFO, "untrusted execution not allowed, exiting\n");
			goto out;
		}
	}

	/*
	 * Find the payload.
	 */
	dprintf(DEBUG_SPEW, "looking for payload\n");
	ret = Img4DecodeGetPayload(&img4, &item);
	if (ret != DR_Success) {
		dprintf(DEBUG_INFO, "Img4DecodeGetPayload failed: %d\n", (int) ret);
		platform_record_breadcrumb_int("image4_load_fail", kBCImg4PayloadDecodeFail);

		goto out;
	}
	payloadBuffer = item.data;
	payloadSize = item.length;

	/*
	 * Verify that the payload will fit within the output buffer.
	 */
	if (payloadSize > *load_len) {
		dprintf(DEBUG_INFO, "payload size exceeds buffer size\n");
		goto out;
	}

#if WITH_AES
	/*
	 * If keys are requested to be disabled (untrusted image or trusted image with EKEY set to false), 
	 * don't decrypt it.
	 */
	if (img_properties.enable_keys) {
		/*
		 * Decrypt the payload data
		 */
		if (0 != image4_load_decrypt_payload(&img4,
						     payloadBuffer,
						     payloadSize)) {
			/* the image is encrypted but we can't decrypt it */
			platform_record_breadcrumb_int("image4_load_fail", kBCImg4PayloadDecryptFail);

			goto out;
		}
	}
#endif /* WITH_AES */

	/*
	 * All done and OK; copy the payload to the base of the buffer and pass the actual
	 * payload size back to the caller.
	 */
	dprintf(DEBUG_SPEW, "copying payload %p->%p\n", payloadBuffer, objectBuffer);
	memcpy(objectBuffer, payloadBuffer, payloadSize);
	*load_len = payloadSize;
	result = 0;

	/*
	 * Clear remains of image4 from the memory.
	 * NOTE: Currently in ROM, iBoot first and second stage, image4 object as whole is loaded
	 * at final destination address. Its processed there, then payload from image4 is moved to 
	 * final destination. This assertion is added to catch if this changes in future, and a more
	 * exhaustive checks will be needed to guarantee image4 related information is cleared from the
	 * memory besides payload.
	 */
	RELEASE_ASSERT(objectBuffer < payloadBuffer);
	RELEASE_ASSERT(payloadSize < correctedObjectSize);
	bzero((uint8_t *)objectBuffer + payloadSize, correctedObjectSize - payloadSize);

	/*
	 * Disable keys if object never requested to enable them or image validation failed.
	 * Also, tell security system to flag GID and UID access.
	 */
	if (!img_properties.enable_keys)
		security_set_untrusted();

	/*
	 * If the image is trusted, and it has asserted that we should clear the
	 * production status flag, do that now.
	 */
	security_set_production_override(img_properties.demote_production_status);

	/* 
	 * Save current object manifest hash, it will be boot manifest hash for next stage.
	 * If untrusted (we failed to find a manifest or manifest is not valid), let next stage know manifest is not valid.
	 */
	if (trustedImage)
		security_set_boot_manifest_hash(img_properties.manifest_hash);
	else
		security_set_boot_manifest_hash(NULL);

	/*
	 * If ANMN is true and image is trusted, stop enforcing MixNMatch prevention.
	 * Otherwise, keep enforcing MixNMatch prevention:
	 * - if current enviornment requires nonce hash and image is trusted
	 * - if manifest hash is verified which will be true if enviornment started with VerifyManifestHash true, AMNM is false, and image is trusted.
	 */
	if (!img_properties.allow_mix_n_match &&
	    ((trustedImage && env_properties.verify_nonce_hash) || img_properties.manifest_hash_verified))
		security_set_mix_n_match_prevention_status(true);
	else
		security_set_mix_n_match_prevention_status(false);

	/*
	 * If didn't load from local storage, the fuses must be locked
	 * (For local storage, the platform can delay locking in some rare cases)
	 */
	if (!env_properties.local_boot)
		security_set_lock_fuses();

	/*
	 * Provide the actual type to the caller.
	 */
	if (actual != NULL) {
		*actual = actualType;
	}

out:
	/*
	 * If we failed, nuke the caller's pointers and clear the work buffer.
	 */
	if (result != 0) {
		dprintf(DEBUG_SPEW, "load failed, clearing object buffer\n");
		*load_addr = 0;
		*load_len = 0;
		if (objectBuffer != NULL) {
			memset(objectBuffer, 0, originalObjectSize);
		}
		/*
		 * If capturing properties, alert caller that the image is invalid
		 */
		if (image4_callbacks.capture_enabled)
			image4_callbacks.validity_cb(false);
	}
	else {
		if (image4_callbacks.capture_enabled)
			image4_callbacks.validity_cb(true);
	}
		
	return (result);
}

/*
 * image4_dump_list
 *
 * Print the list of currently-known images.
 */
void
image4_dump_list(bool detailed)
{
	struct image4_info *image;

	list_for_every_entry(&images, image, struct image4_info, node) {

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
 * image4_check_magic
 *
 * Verifies img4 header
 */
static int
image4_check_magic(void *buf, size_t len)
{
	const DERByte *der = (const DERByte *) buf;
	uint32_t der_size;
	if (len < IMAGE4_ID_BYTES) {
		dprintf(DEBUG_INFO, "Runt header size: %u\n", (unsigned) len);
		return -1;
	}
	if (image4_get_partial(der, IMAGE4_ID_BYTES, NULL, &der_size) != 0) {
		dprintf(DEBUG_INFO, "Error parsing Image4 header\n");
		return -1;
	}
	if (der_size > len) {
		dprintf(DEBUG_INFO, "Truncated Image4: %u vs %u\n",
			(unsigned) der_size, (unsigned) len);
		return -1;
	}
	return 0;
}

/*
 * image4_load_copyobject
 *
 * Fetch an image4 object from its current location into the supplied object buffer.
 */
static int
image4_load_copyobject(struct image_info *image_info, void *objectBuffer, size_t objectSize)
{
	struct image4_info	*image;
	int 			result;

	RELEASE_ASSERT(NULL != objectBuffer);

	dprintf(DEBUG_SPEW, "loading image %p with buffer %p/%x\n", image_info, objectBuffer, (unsigned)objectSize);
	
	switch (image_info->imagePrivateMagic) {
	case IMAGE4_IMAGE_INFO_MAGIC:
		/*
		 * Fetch the image data from the bdev into the caller-supplied
		 * scratch buffer.
		 */
		dprintf(DEBUG_SPEW, "reading image from bdev\n");
		image = (struct image4_info *)(image_info->imagePrivate);
		result = blockdev_read(image->bdev,
				       objectBuffer,
				       image->devOffset,
				       objectSize);
		if (result != (int)objectSize) {
			dprintf(DEBUG_CRITICAL, "blockdev read failed with %d\n", result);
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
		if (image4_check_magic(image_info->imagePrivate, objectSize) != 0) {
			dprintf(DEBUG_INFO, "image not image4\n");
			return(-1);
		}
		if (objectBuffer != image_info->imagePrivate) {
			dprintf(DEBUG_SPEW, "reading image from %p/%x\n", image_info->imagePrivate, (unsigned) objectSize);
			memmove(objectBuffer, image_info->imagePrivate, objectSize);
		}
		break;

	default:
		dprintf(DEBUG_INFO, "unrecognised image source type %x\n", image_info->imagePrivateMagic);
		return(-1);
	}

	return(0);
}

/* image4_verify_number_relation
 *
 * Verifies equal or greater equal relation for Number properties
 * returns:: DR_Success (0): success, -1: doesn't match required_value, DR_xxx: Sequence decode error
 *
 */
static int
image4_verify_number_relation(matching_relation_t relation, DERTag tag, const Img4Property *tag_value_item, uint64_t required_value)
{
	DERReturn ret = DR_Success;
	uint64_t tag_val = 0;

	if ((ret = Img4DecodeGetPropertyInteger64(tag_value_item, tag, &tag_val)) != DR_Success) {
		dprintf(DEBUG_INFO, "tag type not integer, ret: %d\n", (int)ret);
		goto out;
	}

	switch (relation) {
		case REL_EQUAL:
			if (tag_val != required_value)
				ret = -1;
			break;
		case REL_GREATER_EQUAL:
			if (tag_val < required_value)
				ret = -1;
			break;
	}

out:
	return ret;
}

/* image4_verify_bool_relation
 *
 * Verifies equal relation for Boolean properties
 * returns:: DR_Success (0): success, -1: doesn't match required_value, DR_xxx: Sequence decode error
 *
 */
static int
image4_verify_boolean_relation(DERTag tag, const Img4Property *tag_value_item, bool required_value)
{
	DERReturn ret = DR_Success;
	bool tag_val = false;

	if ((ret = Img4DecodeGetPropertyBoolean(tag_value_item, tag, &tag_val)) != DR_Success) {
		dprintf(DEBUG_INFO, "tag type not boolean, ret: %d\n", (int)ret);
		goto out;
	}

	if (tag_val != required_value)
		ret = -1;

out:
	return ret;
}

/* image4_verify_matching_bytes
 *
 * Verifies bytes matches for Data properties
 * returns:: DR_Success (0): success, -1: doesn't match required_value, DR_xxx: Sequence decode error
 *
 */
static int
image4_verify_matching_bytes(matching_relation_t relation, DERTag tag, const Img4Property *tag_value_item, const void *required_bytes, size_t required_length)
{
	DERReturn ret = -1;
	uint8_t *data;
	uint32_t length;

	if (Img4DecodeGetPropertyData(tag_value_item, tag, &data, &length) != DR_Success) {
		dprintf(DEBUG_INFO, "tag type not data, ret: %d\n", (int)ret);
		goto out;
	}

	if (length != required_length) {
		goto out;
	}

	switch(relation) {
		case REL_EQUAL:
		if (memcmp(data, required_bytes, required_length) != 0)
			goto out;
		break;
		
		case REL_GREATER_EQUAL:
		if (memcmp(data, required_bytes, required_length) < 0)
			goto out;
		break;
	}

	ret = DR_Success;

out:
	return ret;
}

/*
 * image4_validate_property_callback_interposer
 *
 * To support property capturing, libImg4Decode calls this adapter, which either
 * directly calls the real image4_validate_property_callback, or captures the
 * value and then calls image4_validate_property_callback.
 */
static int
image4_validate_property_callback_interposer(uint32_t tag, const Img4Property *value, uint32_t propertyType, void *context)
{
	/* Captures are only enabled on targets that call
	 * image4_register_property_capture_callbacks, and the registered type is
	 * equal to the actualType of the image.
	 */
	if (image4_callbacks.capture_enabled)
	{
		switch(value->type) {
			case ASN1_BOOLEAN:
			if (image4_callbacks.bool_cb) {
				bool tag_val;
				if (Img4DecodeGetPropertyBoolean(value, tag, &tag_val) == DR_Success) {
					image4_callbacks.bool_cb(tag, propertyType == kImg4ObjectProperty, tag_val);
				}
			}
			break;
			case ASN1_INTEGER:
			if (image4_callbacks.int_cb) {
				uint64_t tag_val;
				if (Img4DecodeGetPropertyInteger64(value, tag, &tag_val) == DR_Success) {
					image4_callbacks.int_cb(tag, propertyType == kImg4ObjectProperty, tag_val);
				}
			}
			break;
			case ASN1_OCTET_STRING:
			if (image4_callbacks.string_cb) {
				uint8_t *data;
				uint32_t length;
				if (Img4DecodeGetPropertyData(value, tag, &data, &length) == DR_Success) {
					image4_callbacks.string_cb(tag, propertyType == kImg4ObjectProperty, data, length);
				}
			}
			break;
			default:
				panic("Unknown ASN1 type %d\n", value->type);
		}
	}


	return image4_validate_property_callback(tag, value, propertyType, context);
}
/*
 * image4_validate_property_callback
 *
 * libImg4Decode will call this function to validate various
 * properties in the enviornment
 */
static int
image4_validate_property_callback(uint32_t tag, const Img4Property *value, uint32_t propertyType, void *context)
{
	// <rdar://problem/11960612> ValidatePropertyCallback return type should be DERReturn
	// Once this is fixed return more precise errors and for malformed images don't bother executing.
	int ret = 0;
	const struct image4_wrapper_context *image4_wrapper_context = (const struct image4_wrapper_context *)context;
	const struct enviornment_properties *env_properties = image4_wrapper_context->env_properties;
	struct image_properties *img_properties = image4_wrapper_context->img_properties;
	const Img4 *img4 = image4_wrapper_context->img4;
	/* setup for image4 library */
	const Img4DecodeImplementation *implementation = image4_hash_init();

	switch (propertyType) {
		case kImg4ManifestProperty:
			switch (tag) {
				/* Properties require Equal relation */
				case kImg4Tag_ECID:			/*  Unique Chip ID (Number) */
					ret = image4_verify_number_relation(REL_EQUAL,
									    tag,
									    value,
									    env_properties->unique_chip_id);
					break;
				case kImg4Tag_CHIP:			/*  Chip ID (Number) */
					ret = image4_verify_number_relation(REL_EQUAL,
									    tag,
						 			    value,
									    env_properties->chip_id);
					break;
				case kImg4Tag_BORD:			/*  Board ID (Number) */
					ret = image4_verify_number_relation(REL_EQUAL,
									    tag,
									    value,
									    env_properties->board_id);
					break;
				case kImg4Tag_SDOM:			/*  Security Domain (Number) */
					ret = image4_verify_number_relation(REL_EQUAL,
									    tag,
						 			    value,
									    env_properties->security_domain);
					break;

				/* Properties require Greater Equal relation */
				case kImg4Tag_CEPO:			/*  Certificate Epoch (Number) */
					ret = image4_verify_number_relation(REL_GREATER_EQUAL,
									    tag,
						 			    value,
								 	    env_properties->chip_epoch);
					break;

				/* Properties require an action */
				case kImg4Tag_AMNM:			/* Allow Mix-n-Match (Boolean) */
					ret = image4_verify_boolean_relation(tag, value, true);
					if (ret == 0)
						img_properties->allow_mix_n_match = true;
					else if (ret == -1)
						ret = 0;
					break;

				/* Properties (Boolean) require must match relation */
				case kImg4Tag_CPRO:			/*  Certificate Production Status (Boolean) */
					ret = image4_verify_boolean_relation(tag, value, env_properties->production_status);
					break;
				case kImg4Tag_CSEC:			/*  Certificate Security Mode (Boolean) */
					ret = image4_verify_boolean_relation(tag, value, env_properties->security_mode);
					break;

				/* Properties (Data) require must match relation */
				case kImg4Tag_BNCH:			/*  Boot Nonce Hash (Data) */
				{
					/* 
					 * Default say ok for boot-nonce hash, since its in manifest so it will be called for every object.
					 * Only verify, when a stage requires verification (in ROM, and in iBoot when doing restore from recovery mode.)
					 */
					if(env_properties->verify_nonce_hash) {
						uint64_t nonce;
						uint8_t nonce_hash[HASH_OUTPUT_SIZE];

						if (env_properties->local_boot)  {
							DERItem 	tag_data;

							/* Expects a Boot-nonce in RestoreInfo section */
							if ((ret = Img4DecodeGetRestoreInfoData(img4,
												kImg4Tag_BNCN,
												&tag_data.data,
												&tag_data.length)) == DR_Success) {
								if (tag_data.length != sizeof(nonce)) {
									dprintf(DEBUG_INFO, "BNCN tag_data length mismatch\n");
									goto out;
								}
								memcpy((void *)&nonce, (const void *)tag_data.data, (size_t)tag_data.length);
							}
							else {
								dprintf(DEBUG_INFO, "failed to find RestoreInfo, ret: %d\n", (int)ret);
								goto out;
							}
						}
						else {
							nonce = env_properties->boot_nonce;
						}

						hash_calculate((const void *)&nonce, sizeof(nonce), (void *)nonce_hash, sizeof(nonce_hash));

						// The boot nonce hash (BNCH) is truncated to NONCE_HASH_OUTPUT_SIZE bytes.
						ret = image4_verify_matching_bytes(REL_EQUAL, tag, value, (const void *)&nonce_hash, NONCE_HASH_OUTPUT_SIZE);
					}
					break;
				}

				/* Ignore rest */
				default:
					dprintf(DEBUG_INFO, "Ignoring unknown tag 0x%08x\n", tag);
					break;
			}
		break;
		
		case kImg4ObjectProperty:
			switch (tag) {
				/* Properties require an action */
				case kImg4Tag_DPRO:			/* Demote Production Status (Boolean) */
				{
					ret = image4_verify_boolean_relation(tag, value, true);
					if(ret == 0)
						img_properties->demote_production_status = true;
					else if (ret == -1)
						ret = 0;
					break;
				}
				case kImg4Tag_EKEY:			/* Enable Keys (Boolean) */
					ret = image4_verify_boolean_relation(tag, value, true);
					if (ret == 0)
						img_properties->enable_keys = true;
					else if (ret == -1)
						ret = 0;
					break;
				case kImg4Tag_EPRO:			/* Effective Production Status (Boolean) */
					img_properties->valid_effective_production_status = true;
					ret = image4_verify_boolean_relation(tag, value, true);
					/* true: production, false: development */
					if (ret == 0)
						img_properties->effective_production_status = true;
					else if (ret == -1)
						ret = 0;
					break;
				case kImg4Tag_ESEC:			/* Effective Security Mode (Boolean) */
					img_properties->valid_effective_security_mode = true;
					ret = image4_verify_boolean_relation(tag, value, true);
					/* true: secure, false: non-secure */
					if (ret == 0)
						img_properties->effective_security_mode = true;
					else if (ret == -1)
						ret = 0;
					break;

				/* Properties require must match relation */
				case kImg4Tag_DGST:			/*  Object Digest (Data) */
				{
					if ((ret = Img4DecodeCopyPayloadDigest(img4, img_properties->object_digest, HASH_OUTPUT_SIZE, implementation)) != DR_Success) {
						dprintf(DEBUG_INFO, "failed to find PayloadHash, ret: %d\n", (int)ret);
						goto out;
					}
					ret = image4_verify_matching_bytes(REL_EQUAL, tag, value, (const void *)img_properties->object_digest, HASH_OUTPUT_SIZE);
					break;
				}

				/* Ignore rest */
				default:
					dprintf(DEBUG_INFO, "Ignoring unknown tag 0x%08x\n", tag);
					break;
			}
		break;
	}

out:
	if (ret != 0)
		dprintf(DEBUG_INFO, "failed to validate tag 0x%08x, ret:%d\n", tag, ret);

	return ret;
}

void image4_register_property_capture_callbacks(image4_start_capture_callback start_cb, image4_boolean_property_callback bool_cb, 
			image4_integer_property_callback int_cb, image4_string_property_callback string_cb,
			image4_validity_callback validity)
{
	if (start_cb && !validity)
	{
		// For security reasons, if the caller is interested in any callbacks for an image type, a validity callback must be provided.
		panic("image4_register_property_capture_callbacks: When capturing properties, you must provide a validity callback\n");
	}
	else
	{

		image4_callbacks.start_cb = start_cb;
		image4_callbacks.bool_cb = bool_cb;
		image4_callbacks.int_cb = int_cb;
		image4_callbacks.string_cb = string_cb;
		image4_callbacks.validity_cb = validity;
	}

	image4_callbacks.capture_enabled = false;
	if (image4_callbacks.validity_cb)
		image4_callbacks.validity_cb(false);

	dprintf(DEBUG_SPEW, "image4 callbacks registered.\n");

}

static const Img4DecodeImplementation *image4_hash_init(void)
{
#if WITH_SHA2_384
	sha384_digest_info = ccsha384_di();
	return &kImg4DecodeSecureBootRsa4kSha384;
#else	// SHA1
	sha1_digest_info = sha1_get_ccsha1_ccdigest_info();
	return &kImg4DecodeSecureBootRsa1kSha1;
#endif
}

#if WITH_AES
/*
 * image4_load_decrypt_payload
 *
 * Decrypt the payload contents if they are encrypted and we can find a key
 * that matches.
 */
/* Reference Keybag Schema from image4 DER object :

	  65  116:     OCTET STRING, encapsulates {
	    <30 72>
	  67  114:       SEQUENCE {
	    <30 37>
	  69   55:         SEQUENCE {
	    <02 01>
	  71    1:           INTEGER 1
	    <04 10>
	  74   16:           OCTET STRING
	         :             48 BA A6 12 92 44 61 67 03 72 2D 59 FA 8D 59 3B
	    <04 20>
	  92   32:           OCTET STRING
	         :             30 44 52 34 7F 48 8E BF 7A 63 95 BA 54 6A EE 43
	         :             87 4A 2B 99 8C 1F 24 7F 64 51 4C E3 0C 6C 4C 52
	         :           }
	    <30 37>
	 126   55:         SEQUENCE {
	    <02 01>
	 128    1:           INTEGER 2
	    <04 10>
	 131   16:           OCTET STRING
	         :             90 F0 C5 19 3D 08 EA 7D 9B 5E F5 8B 31 EE 7F 9D
	    <04 20>
	 149   32:           OCTET STRING
	         :             06 5F 6B 72 92 56 ED FF A2 B5 07 9C 9B E3 A6 40
	         :             D1 B8 FD 87 90 80 BA 89 5C E6 84 84 03 9E DF 24
	         :           }
	         :         }
	         :       }
	         :     }
 */
struct ivkey {
	u_int8_t IV[AES_BLOCK_SIZE];
	u_int8_t Key[AES_KEY_SIZE_BYTES_256];
} __packed;

static int
image4_load_decrypt_payload(Img4 *img4, void *payloadBuffer, size_t payloadSize)
{
	int			result;
	DERReturn		ret;
	DERItem			keybagDER;
	DERDecodedInfo		keybagSequenceInfo;
	DERSequence		keybagSequence;
	u_int32_t		platformKeyOpts;
	u_int32_t		keybagKeyOpts;
	size_t			keybagKeySize;
	struct ivkey		keybagIVKey;
	struct image4_keybag	keyBag;
	DERDecodedInfo 		keyDER;
	DERSequence 		keySequence;
	DERDecodedInfo		info;

	/* basic sanity on arguments */
	RELEASE_ASSERT(NULL != img4);

	/* initialize locals */
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
	dprintf(DEBUG_INFO, "checking for encrypted payload\n");

	ret = Img4DecodeGetPayloadKeybag(img4, &keybagDER);
	if ((keybagDER.data == NULL) || (keybagDER.length == 0) || (ret == DR_ParamErr)) {
		/* there are no keybags, payload is not encrypted */
		result = 0;
		dprintf(DEBUG_INFO, "Unencrypted payload\n");
		goto out;
	}
	else if (ret != DR_Success) {
		dprintf(DEBUG_INFO, "failed to find a valid keybag for this system, ret: %d\n", (int) ret);
		goto out;
	}

	if ((DERDecodeItem(&keybagDER, &keybagSequenceInfo) != DR_Success) ||
	    (DERDecodeSeqContentInit(&keybagSequenceInfo.content, &keybagSequence) != DR_Success)) {
		dprintf(DEBUG_INFO, "PayloadKeybag SEQUENCE header malformed\n");
		goto out;
	}

	for (;;) {
		if ((DERDecodeSeqNext(&keybagSequence, &keyDER) != DR_Success) ||
		    (DERDecodeSeqContentInit(&keyDER.content, &keySequence) != DR_Success)) {
			dprintf(DEBUG_INFO, "PayloadKeybag key SEQUENCE header malformed\n");
			goto out;
		}

		if ((DERDecodeSeqNext(&keySequence, &info) != DR_Success) ||
		    (DERParseInteger(&info.content, &keyBag.kbSelector) != DR_Success)) {
			dprintf(DEBUG_INFO, "PayloadKeybag key bad KeySelector\n");
			goto out;
		}

		if (DERDecodeSeqNext(&keySequence, &info) != DR_Success) {
			dprintf(DEBUG_INFO, "PayloadKeybag key bad IV\n");
			goto out;
		}
		if (info.content.length != AES_BLOCK_SIZE) {
			dprintf(DEBUG_INFO, "IV wrong size: %u but expected %u\n",
				(unsigned) info.content.length, (unsigned) AES_BLOCK_SIZE);
			goto out;
		}
		memcpy(&keyBag.kbIVBytes, info.content.data, sizeof(keyBag.kbIVBytes));

		if (DERDecodeSeqNext(&keySequence, &info) != DR_Success) {
			dprintf(DEBUG_INFO, "PayloadKeybag key bad key\n");
			goto out;
		}
		keyBag.kbKeySize = info.content.length * 8;
		if ((keyBag.kbKeySize != 128) && (keyBag.kbKeySize != 192) && (keyBag.kbKeySize != 256)) {
			dprintf(DEBUG_INFO, "Unsupported key size: %d\n", keyBag.kbKeySize);
			goto out;
		}
		memcpy((void *)&keyBag.kbKeyBytes, (const void *)info.content.data, sizeof(keyBag.kbKeyBytes));

		/* work out how big the key is going to be */
		keybagKeyOpts = AES_KEY_TYPE_USER;
		switch (keyBag.kbKeySize) {
		case 128 : keybagKeyOpts |= AES_KEY_SIZE_128; break;
		case 192 : keybagKeyOpts |= AES_KEY_SIZE_192; break;
		case 256 : keybagKeyOpts |= AES_KEY_SIZE_256; break;
		default:
			/* not a valid AES key size, bail */
			dprintf(DEBUG_INFO, "AES key size %d not supported/valid\n", keyBag.kbKeySize);
			goto out;
		}
		keybagKeySize = keyBag.kbKeySize / 8;
		if (!(keybagKeySize <= sizeof(keybagIVKey.Key)))
			goto out;

		/* copy the IV from the keybag */
		memcpy(&keybagIVKey.IV, keyBag.kbIVBytes, sizeof(keybagIVKey.IV));

		/* copy the key from the keybag */
		memcpy(&keybagIVKey.Key, keyBag.kbKeyBytes, keybagKeySize);

		switch (keyBag.kbSelector) {
		case kImage4KeybagSelectorChipUnique:
		case kImage4KeybagSelectorChipUniqueDev:
			/* ask platform for key details */
			platformKeyOpts = 0;
			if (platform_translate_key_selector(keyBag.kbSelector, &platformKeyOpts) != 0) {
				/* we don't recognise this key, spin and try again */
				dprintf(DEBUG_INFO, "key selector %d not recognised\n", keyBag.kbSelector);
				continue;
			}
			dprintf(DEBUG_INFO, "using key selector %d\n", keyBag.kbSelector);
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
		default:
			/* 
			 * Unknown keybag-selector, bail
			 */
			dprintf(DEBUG_INFO, "unknown keybag-selector\n");
			goto out;
		}

		/*
		 * Decrypt the payload.
		 * 
		 * Pad payload size to multiple of 16 bytes (AES_BLOCK_SIZE), if its not padded already.
		 */
		RELEASE_ASSERT((payloadSize % AES_BLOCK_SIZE) == 0); 
		dprintf(DEBUG_INFO, "AES operation 0x%zx bytes\n", payloadSize);
		result = aes_cbc_decrypt(payloadBuffer,
					 payloadBuffer,
					 payloadSize,
					 keybagKeyOpts,
					 &keybagIVKey.Key,
					 &keybagIVKey.IV);

		/* clear the iv & key from memory */
		memset(&keybagIVKey, 0, sizeof(keybagIVKey));

		if (result)
			dprintf(DEBUG_INFO, "cannot decrypt image - unexpected AES failure");
		goto out;
	}

out:
	return (result);
}
#endif
