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
#ifndef _IMAGE3_FORMAT_H_
#define _IMAGE3_FORMAT_H_

#include <drivers/sha1.h>

/*
 * All structures defined here are on-media formats.
 */
#pragma pack(push, 1)

/*
 * Fixed object header.
 */
typedef struct _Image3ObjectHeader {
	/* these fields are unsigned */
	u_int32_t	ihMagic;
#define kImage3HeaderMagic		'Img3'
	u_int32_t	ihSkipDistance;
	u_int32_t	ihBufferLength;

	/* fields below are signed */
	u_int32_t	ihSignedLength;
	u_int32_t	ihType;
	u_int8_t	ihBuffer[];
} Image3ObjectHeader;

/*
 * Generic tag header.
 */
typedef struct {
	u_int32_t	itTag;
	u_int32_t	itSkipDistance;
	u_int32_t	itBufferLength;
	u_int8_t	itBuffer[];
} Image3TagHeader;

#ifdef IMAGE3_CREATOR
/*
 * Partial hash types.
 */
typedef struct {
	/* number of valid bytes in the buffer */
	u_int32_t	masteredReservationLength;
	u_int32_t	masteredSignedLength;
	u_int8_t	sha1_state[CCSHA1_OUTPUT_SIZE];
} Image3PartialHash;
#endif /* IMAGE3_CREATOR */

/*
 * Utility tag types.
 */
typedef struct {
	/* number of valid bytes in the buffer */
	u_int32_t	stringLength;
	char		stringBytes[];
} Image3TagString;

typedef struct {
	union {
		u_int32_t	u32;
		int32_t		s32;
	} value;
} Image3TagNumber32;

typedef struct {
	union {
		u_int64_t	u64;
		int64_t		s64;
	} value;
} Image3TagNumber64;

typedef struct {
	union {
		Image3TagNumber32	n32;
		Image3TagNumber64	n64;
	} number;
} Image3TagNumber;

#ifdef IMAGE3_CREATOR
#define IMAGE_TYPE_EMBEDCERT	'cert'	/* ihType of image embedded in leaf cert */
#endif /* IMAGE3_CREATOR */

/*
 * Defined tags and their datastructures.
 */

#define	kImage3TagTypeData		'DATA'		/* opaque */
#define kImage3TagTypeSignedHash	'SHSH'		/* opaque */
#define	kImage3TagTypeCertificateChain	'CERT'		/* opaque */
#define	kImage3TagTypeVersion		'VERS'		/* string */
#define	kImage3TagTypeSecurityEpoch	'SEPO'		/* number */
#define	kImage3TagTypeSecurityDomain	'SDOM'		/* number */
#define kImage3TagTypeProductionStatus	'PROD'		/* number */
#define	kImage3TagTypeChipType		'CHIP'		/* number */
#define	kImage3TagTypeBoardType		'BORD'		/* number */
#define	kImage3TagTypeUniqueID		'ECID'		/* number */
#define	kImage3TagTypeRandomPad		'SALT'		/* number */
#define	kImage3TagTypeType		'TYPE'		/* number */
#define	kImage3TagTypeOverride		'OVRD'		/* bitmap */
#define kImage3TagTypeHardwareEpoch	'CEPO'		/* number */
#define kImage3TagTypeNonce		'NONC'		/* opaque */

#define kImage3TagTypeKeybag		'KBAG'
typedef struct {
	u_int32_t	kbSelector;
#define kImage3KeybagSelectorNoKey		(0)
#define kImage3KeybagSelectorChipUnique		(1)
#define kImage3KeybagSelectorChipUniqueDev	(2)
	u_int32_t	kbKeySize;
	u_int8_t	kbIVBytes[16];
	u_int8_t	kbKeyBytes[32];
} Image3TagKBAG;

#define kImage3SecurityDomainManufacturer	0
#define kImage3SecurityDomainDarwin		1
#define kImage3SecurityDomainRTXC		3

#define kImage3SecurityProductionStatus		1

#define kImage3OverrideProductionMode		1

/* restore structure packing */
#pragma pack(pop)

#endif /* _IMAGE3_FORMAT_H_ */
