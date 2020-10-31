/*
 * Copyright (C) 2008,2014-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#if !APPLICATION_SECUREROM

#include <lib/mib.h>
#include <platform.h>

// Default MIB Application/Product
MIB_CONSTANT_WEAK(kMIBTargetApplicationProduct,		kOIDTypeUInt32,  0);

// Default MIB build configuration constants
MIB_CONSTANT_WEAK(kMIBTargetWithHwAsp,			kOIDTypeBoolean, false);
MIB_CONSTANT_WEAK(kMIBTargetWithEffaceable,		kOIDTypeBoolean, false);
MIB_CONSTANT_WEAK(kMIBTargetWithFileSystem,		kOIDTypeBoolean, false);
MIB_CONSTANT_WEAK(kMIBTargetWithHwFlashNand,		kOIDTypeBoolean, false);
MIB_CONSTANT_WEAK(kMIBTargetWithHwFlashNor,		kOIDTypeBoolean, false);
MIB_CONSTANT_WEAK(kMIBTargetWithHwNvme,			kOIDTypeBoolean, false);
MIB_CONSTANT_WEAK(kMIBTargetWithLegacyPanicLogs,	kOIDTypeBoolean, false);

// Default MIB target paint constants
MIB_CONSTANT_WEAK(kMIBTargetDisplayGammaTableCount,	kOIDTypeUInt32, 0);
MIB_CONSTANT_PTR_WEAK(kMIBTargetDisplayGammaTablePtr,	kOIDTypeStruct, NULL);
MIB_CONSTANT_WEAK(kMIBTargetPanicOnUnknownDclrVersion,	kOIDTypeBoolean, true);
MIB_CONSTANT_WEAK(kMIBTargetPictureScale,		kOIDTypeUInt32, 1);

// Default MIB NOR configuration
MIB_CONSTANT_WEAK(kMIBTargetNvramNorBankCount,		kOIDTypeUInt32,	2);
MIB_CONSTANT_WEAK(kMIBTargetNvramNorBankSize,		kOIDTypeSize,	0x2000);
MIB_CONSTANT_WEAK(kMIBTargetNvramNorOffset,		kOIDTypeInt32, -(2 * 0x2000));

// Default MIB Image3 constants
// Delete this and it's definition in mib.base when all Image3 targets are deprecated.
MIB_CONSTANT_WEAK(kMIBTargetAcceptGlobalTicket,		kOIDTypeBoolean, false);

//Default Color Manager Data
MIB_CONSTANT_WEAK(kMIBTargetDisplayCMEngammaTablePtr,	kOIDTypeStruct, NULL);
MIB_CONSTANT_WEAK(kMIBTargetDisplayCMEngammaTableCount,	kOIDTypeUInt32, 0);
MIB_CONSTANT_WEAK(kMIBTargetDisplayCMDegammaTablePtr,	kOIDTypeStruct, NULL);
MIB_CONSTANT_WEAK(kMIBTargetDisplayCMDegammaTableCount,	kOIDTypeUInt32, 0);
MIB_CONSTANT_WEAK(kMIBTargetDisplayCMMatrixTablePtr,	kOIDTypeStruct, NULL);
MIB_CONSTANT_WEAK(kMIBTargetDisplayCMMatrixTableCount,	kOIDTypeUInt32, 0);

#endif	// !APPLICATION_SECUREROM

