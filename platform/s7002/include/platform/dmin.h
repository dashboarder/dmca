/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

 /*
  * Dmin.h is provided by the diags team, defining the 'DMin' key's format
  * The contents below are copy-pasted verbatim from header files supplied by Diags.
  */


#ifndef _DMin_h_
#define _DMin_h_

/* vim: set ts=4 sw=4 softtabstop=4 noexpandtab */

#include <stdint.h>

#ifndef MULTICHARACTER_LITERAL
#	define MULTICHARACTER_LITERAL(a,b,c,d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))
#endif

enum {
	kEnclosureNone = 			0,
	kEnclosureAlumLight6063 = 	1,
	kEnclosureAlumDark6063 = 	2,
	kEnclosureAlumLight7003 = 	3,
	kEnclosureAlumDark7003 = 	4,
	kEnclosureSUSMirror = 		5,
	kEnclosureSUSBrushed = 		6,
	kEnclosureSUSBlackPVD = 	7,
	kEnclosureMaverickYellow = 	8,
	kEnclosureMaverickRose = 	9,
	kEnclosureZirconiaWhite = 	10,
	kEnclosureGlass = 			11,
};
typedef uint32_t EnclosureType_t; // goes with above enum

enum {
	kFCMTypeNone = 				0,
	kFCMTypeOnyx = 				1,
	kFCMTypeGlass = 			2,
};
typedef uint32_t FCMType_t; 	// goes with above

enum {
	kBCMWindowTypeNone = 		0,
	kBCMWindowTypeOnyx = 		1,
	kBCMWindowTypeGlass = 		2,
};
typedef uint32_t BCMWindowType_t;

enum {
	kBCMLensTypeNone = 			0,
	kBCMLensTypeNormal =		1,
	kBCMLensTypeFresnel = 		2,
};
typedef uint32_t BCMLensType_t;

enum {
	kBCMLEDTypeNone = 			0,
	kBCMLEDTypeNormal = 		1,
	kBCMLEDTypeNew = 			2,
};
typedef uint32_t BCMPTLEDType_t;

enum {
	kBCMPlatinumTypeNone = 		0,
	kBCMPlatinumTypeProtoN_34 = 1,
	kBCMPlatinumTypeProtoC = 	2,
};
typedef uint32_t BCMPTSensorType_t;

/* Version 2 information - added Lisa Bits */

enum {
	kLisaOpAmpNone =		0,
	kLisaOpAmpMAX4321 =		1,
	kLisaOpAmpLMV693 =		2,
	kLisaOpAmpLMV881 =		3,
};
typedef uint32_t LisaOpAmpType_t;

enum {
	kLisaSensorICNone =	0,
	kLisaSensorICA0 =	1,
	kLisaSensorICB0 =	2,
};
typedef uint32_t LisaSensorICType_t;

enum {
	kLisaWheelNone =				0,
	kLisaWheelLF31 =				1,
	kLisaWheelLF31LowDiffusion =	2,
	kLisaWheelLF31HighStripeVar =	3,
	kLisaWheelLF31pSLaser =		4,
	kLisaWheelLF45 = 				5,
	kLisaWheelSantek =			6,
	kLisaWheelNisseiBlack =		7,
	kLisaWheelNissei =			8,
	kLisaWheelHeptagonGrind =		9,
	kLisaWheelHeptagonFormed =	10
};
typedef uint32_t LisaWheelType_t;

enum {
	kLisaKnurlNone =	0,
	kLisaKnurlProtoN =	1,
};
typedef uint32_t LisaKnurlType_t;

/* Pack the structure, do not allow compiler padding */
typedef struct __attribute__((packed)) {
	EnclosureType_t topEnclosure;		// See enum at the top
	EnclosureType_t botEnclosure;		// Same values here
	FCMType_t fcmType;					// Onyx or Glass
	uint32_t fcmARCoated;				// 0x00000000 = no AR coating, 0x00000001 = AR coating
	uint32_t fcmInkColor;				// 0x00RRGGBB (little endian, 32-bit word, just like DClr)
	BCMWindowType_t bcmWindow;			// Onyx or Glass
	BCMLensType_t bcmLens;				// Normal or Fresnel
	BCMPTSensorType_t bcmPTSensorType;	// 3 or 4
	BCMPTLEDType_t bcmPTLEDType;		// Normal or "New"
} DMin_v1_t;

typedef struct __attribute__((packed)) {
	EnclosureType_t topEnclosure;		// See enum at the top
	EnclosureType_t botEnclosure;		// Same values here
	FCMType_t fcmType;					// Onyx or Glass
	uint32_t fcmARCoated;				// 0x00000000 = no AR coating, 0x00000001 = AR coating
	uint32_t fcmInkColor;				// 0x00RRGGBB (little endian, 32-bit word, just like DClr)
	BCMWindowType_t bcmWindow;			// Onyx or Glass
	BCMLensType_t bcmLens;				// Normal or Fresnel
	BCMPTSensorType_t bcmPTSensorType;	// 3 or 4
	BCMPTLEDType_t bcmPTLEDType;		// Normal or "New"
	
	// v2 bits
	LisaOpAmpType_t lisaOAType;
	LisaSensorICType_t lisaSensorIC;
	LisaWheelType_t lisaEncoderWheel;
	LisaKnurlType_t lisaKnurl;
} DMin_v2_t;

#define DMIN_VERSION_1	0x00000001
#define DMIN_VERSION_2	0x00000002

typedef struct {
	uint32_t version;				// Set to DMIN_VERSION_2
	union {
		DMin_v1_t v1;				// if version == DMIN_VERSION_1, use this struct
		DMin_v2_t v2;				// if version == DMIN_VERSION_2, use this struct
	};	// Anonymous union to bridge between v1 and any later version
}DMin_t;

#define SYSCFG_DEVICE_MATERIAL_INFO_SIZE	(sizeof(DMin_t))	/* v1 = 40 bytes, v2 = 56 bytes */
#define SYSCFG_DEVICE_MATERIAL_INFO			MULTICHARACTER_LITERAL('D','M','i','n')

#endif /* _DMin_h_ */
