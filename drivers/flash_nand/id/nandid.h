// Copyright (C) 2008-2009 Apple, Inc. All rights reserved.
//
// This document is the property of Apple, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple, Inc.
//

#include "ANDTypes.h"
#include "WMRTypes.h"
#include "WMRFeatures.h"
#if WMR_BUILDING_IBOOT
#include <lib/devicetree.h>
#endif

#ifndef _NANDID_H_
#define _NANDID_H_

// =============================================================================
// Preprocessor Constants

#define NAND_CHIP_ID_SIZE              (8)
#define VALID_NAND_CHIP_ID_SIZE        (6)
#define COMPARE_NAND_CHIP_ID_SIZE      (4)
#define LEGACY_NAND_CHIP_ID_SIZE       (4)

#define NAND_MAX_CE_COUNT_PER_PACKAGE  (8)
#define NAND_MAX_PACKAGES_PER_CONFIG   (2)
#define NAND_MAX_CE_COUNT_TOTAL        (NAND_MAX_PACKAGES_PER_CONFIG * NAND_MAX_CE_COUNT_PER_PACKAGE)

#define NAND_MAX_APPLE_NAME_SIZE       (20)
#define NAND_MAX_VENDOR_NAME_SIZE      (20)
#define NAND_MAX_PACKAGE_TYPE_SIZE     (20)
#define NAND_MAX_PACKAGE_SIZE_SIZE     (20)

// =============================================================================
// Drive Strength

typedef UInt8 NandDriveStrength;

#define NAND_DRIVE_STRENGTH_DEFAULT  ((NandDriveStrength)0)

#define NAND_DRIVE_STRENGTH_X1       ((NandDriveStrength)1)
#define NAND_DRIVE_STRENGTH_X2       ((NandDriveStrength)2)
#define NAND_DRIVE_STRENGTH_X3       ((NandDriveStrength)3)
#define NAND_DRIVE_STRENGTH_X4       ((NandDriveStrength)4)

// =============================================================================
// Nand Chip Id

typedef UInt8 NandChipId[NAND_CHIP_ID_SIZE];

// =============================================================================
// Nand Package Id

struct NandPackageIdStruct
{
    // Unique identifier for this struct - start
    NandChipId chipId;
    UInt8 ceCnt;
    // Unique identifier for this struct - end
};

typedef struct NandPackageIdStruct NandPackageId;

// =============================================================================
// Nand Config Id

struct NandConfigIdStruct
{
    // Unique identifier for this struct - start
    UInt8 connectedBusCnt;
    UInt8 packageCnt;
    NandPackageId packageIds[NAND_MAX_PACKAGES_PER_CONFIG];
    // Unique identifier for this struct - end
};

typedef struct NandConfigIdStruct NandConfigId;

// =============================================================================
// Nand Geometry

struct NandGeometryStruct
{
    // Unique identifier for this struct - start
    NandChipId chipId;
    // Unique identifier for this struct - end

    UInt16 blocksPerCS;
    UInt16 pagesPerBlock;
    UInt16 dataBytesPerPage;
    UInt16 spareBytesPerPage;

    UInt8 eccPer512Bytes;
    CheckInitialBadType initialBBType;

    UInt16 diesPerCS;
};

typedef struct NandGeometryStruct NandGeometry;

#if (defined (AND_ENABLE_NAND_DESCRIPTION_TEXT) || AND_ENABLE_NAND_DESCRIPTION_TEXT)
// =============================================================================
// Nand Description

struct NandDescriptionStruct
{
    // Unique identifier for this struct - start
    NandPackageId packageId;
    // Unique identifier for this struct - end

    char appleName[NAND_MAX_APPLE_NAME_SIZE];
    char vendorPartNumber[NAND_MAX_VENDOR_NAME_SIZE];
    char packageType[NAND_MAX_PACKAGE_TYPE_SIZE];
    char packageSize[NAND_MAX_PACKAGE_SIZE_SIZE];
};

typedef struct NandDescriptionStruct NandDescription;

// =============================================================================
// ChipId Description

struct NandChipIdDescriptionStruct
{
    // Unique identifier for this struct - start
    NandChipId chipId;
    // Unique identifier for this struct - end

    char die[NAND_MAX_VENDOR_NAME_SIZE];
    char vendor[NAND_MAX_VENDOR_NAME_SIZE];
};

typedef struct NandChipIdDescriptionStruct NandChipIdDescription;
#endif // (defined(AND_ENABLE_NAND_DESCRIPTION_TEXT) || AND_ENABLE_NAND_DESCRIPTION_TEXT)

// =============================================================================
// Nand Timing

struct NandTimingStruct
{
    // Unique identifier for this struct - start
    NandConfigId configId;
    // Unique identifier for this struct - end

    // write cycle
    UInt8 writeCycleNanosecs;
    UInt8 writeSetupNanosecs;
    UInt8 writeHoldNanosecs;

    // read cycle
    UInt8 readCycleNanosecs;
    UInt8 readSetupNanosecs;
    UInt8 readHoldNanosecs;

    // read window
    UInt8 readDelayNanosecs;
    UInt8 readValidNanosecs;
};

typedef struct NandTimingStruct NandTiming;

// =============================================================================
// Nand Support

struct NandBoardSupportStruct
{
    // Unique identifier for this struct - start
    NandConfigId configId;
    // Unique identifier for this struct - end

    VendorSpecificType vsType;
};

typedef struct NandBoardSupportStruct NandBoardSupport;

// =============================================================================
// Board Package/Landing Map

typedef UInt32 NandLandingMask;

typedef NandLandingMask NandLandingMap[NAND_MAX_PACKAGES_PER_CONFIG + 1];

// =============================================================================
// Nand Board Timing

struct NandBoardInfoStruct
{
    NandDriveStrength controlDstr;
    NandDriveStrength dataDstr;
    NandDriveStrength chipEnableDstr;
    
    UInt8 socToNandRiseNanosecs;
    UInt8 socToNandFallNanosecs;
    UInt8 nandToSocRiseNanosecs;
    UInt8 nandToSocFallNanosecs;
    
    NandLandingMap landingMap;
    BOOL32 useToggleMode;
    BOOL32 useDiffDQSMode;
    BOOL32 useDiffREMode;
    BOOL32 useVref;
    BOOL32 useSingleHostChannel;
    BOOL32 allowSingleChipEnable;
};

typedef struct NandBoardInfoStruct NandBoardInfo;

struct NandPpnStruct
{
    UInt32    specVersion;
    UInt32    blocksPerCau;
    UInt32    causPerCe;
    UInt32    slcPagesPerBlock;
    UInt32    mlcPagesPerBlock;
    UInt32    matchOddEvenCaus;
    UInt32    bitsPerCau;
    UInt32    bitsPerPage;
    UInt32    bitsPerBlock;
    UInt32    maxTransactionSize;
};

typedef struct NandPpnStruct NandPpn;

struct NandFormatStruct
{
    UInt32    metaPerLogicalPage;
    UInt32    validMetaPerLogicalPage;
    UInt32    logicalPageSize;
};
typedef struct NandFormatStruct NandFormat;

// =============================================================================
// Nand Info

struct NandInfoStruct
{
    const NandBoardSupport*  boardSupport;
    const NandBoardInfo*     boardInfo;
    const NandTiming*        configTiming;
    const NandGeometry*      geometry;
    const NandPpn*           ppn;
    const NandFormat*        format;
};

typedef struct NandInfoStruct NandInfo;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// =============================================================================
// Public Function Declarations

BOOL32 findNandInfo(const NandChipId*    chipIds,
                    NandInfo*            nandInfo,
                    UInt32 numActiveDataBus);

void setNandIdPpnConfig(const NandChipId*  chipIds,
                        NandInfo*          nandInfo,
                        UInt32             busses,
                        UInt32             ces);
    
void reportFMCTimingValues(UInt32 readSetupClocks,
                           UInt32 readHoldClocks,
                           UInt32 readDccycleClocks,
                           UInt32 writeSetupClocks,
                           UInt32 writeHoldClocks);

void reportToggleModeFMCTimingValues(UInt32 dqsHalfCycleClocks,
                                     UInt32 ceSetupClocks,
                                     UInt32 ceHoldClocks,
                                     UInt32 adlClocks,
                                     UInt32 whrClocks,
                                     UInt32 readPreClocks,
                                     UInt32 readPostClocks,
                                     UInt32 writePreClocks,
                                     UInt32 writePostClocks,
                                     UInt32 regDQSTimingCtrl);

void setToggleMode();

void setPPNOptions(BOOL32 retire_on_invalid_refresh);

void reportPpnFeatures(void *list, UInt32 size);

void reportDbgChipIds(void *list, UInt32 size);

void setECCLevels(UInt32 numCorrectableBits,
                  UInt32 refreshThresholdBits);
                    
BOOL32 checkPpnLandingMap(UInt32 validCEMap);

const NandFormat * getPPNFormat(void);

BOOL32 targetSupportsToggle(void);
BOOL32 targetSupportsDiffDQS(void);
BOOL32 targetSupportsDiffRE(void);
BOOL32 targetSupportsVREF(void);
BOOL32 targetSupportsSHC(void);
BOOL32 targetSupportsSingleCE(void);

// =============================================================================
// Public Variables

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // _NANDID_H_
