// Copyright (C) 2008-2009 Apple, Inc. All rights reserved.
//
// This document is the property of Apple, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple, Inc.
//

#include "WMRFeatures.h"
#if WMR_BUILDING_IBOOT
#include "nandid.h"
#include <drivers/nand_device_tree.h>
#include <target/nand_spec_tables.h>
#include <sys.h>
#include <debug.h>
#else
#include "NandSpec.h"
#include "NandSpecTables.h"
#endif

#include "WMROAM.h"
#include "FIL.h"

// =============================================================================
// Preprocessor Definitions

#define WITH_NAND_DT_CONFIG (WITH_NAND_FILESYSTEM && WITH_DEVICETREE)

// =============================================================================
// Extern global variables

// =============================================================================
// Private Function Declarations

static BOOL32 isSameChipId(const NandChipId chipId1, const NandChipId chipId2);
static BOOL32 isSameConfigId(const NandConfigId* configId1, const NandConfigId* configId2);

static const NandGeometry* lookupNandGeometry(const NandChipId chipId);
static const NandTiming* lookupNandTiming(const NandConfigId* configId);
static const NandBoardSupport* lookupBoardSupport(const NandConfigId* configId);
static BOOL32 isValidChipId(const NandChipId chipId);
static BOOL32 getChipIdAndTotalCECountAndCEMap(const NandChipId* chipIds, NandChipId* foundChipId, UInt32* totalCECount, UInt32* ceMap);
static BOOL32 areChipIdsIdentical(const NandChipId* chipIds, NandChipId* chipId);
static UInt32 setBitCount(UInt32 x);
static BOOL32 checkCESymmetryAndGetPackageCnt(NandLandingMap *landingMap, UInt32 ceMap, UInt8* pkgCnt);

// =============================================================================
// Private Global Variables
#if WITH_NAND_DT_CONFIG
static NandInfo currentNandInfo;
static UInt32   currentCECount = 0;
static UInt32   currentCEBitmap = 0;
static UInt32   currentActiveDataBus = 0;
#endif //WITH_NAND_DT_CONFIG
static UInt32   currentCorrectableBits = 0;
static UInt32   currentRefreshThreshold = 0;
static BOOL32   ppnDevice = FALSE32;
static BOOL32   toggleDevice = FALSE32;
static BOOL32   _retire_on_invalid_refresh;
static void    *_ppn_feature_list = NULL;
static UInt32   _ppn_feature_size = 0;
static void    *_dbg_chipids_list = NULL;
static UInt32   _dbg_chipids_size = 0;

static UInt32 _dqsHalfCycleClocks;
static UInt32 _ceSetupClocks;
static UInt32 _ceHoldClocks;
static UInt32 _ADLClocks;
static UInt32 _WHRClocks;

static UInt32 _readPreClocks;
static UInt32 _readPostClocks;
static UInt32 _writePreClocks;
static UInt32 _writePostClocks;

static UInt32 _readSetupClocks;
static UInt32 _readHoldClocks;
static UInt32 _readDccycleClocks;
static UInt32 _writeSetupClocks;
static UInt32 _writeHoldClocks;

static UInt32 _regDQSTimingCtrl;

// =============================================================================
// Local constants


// =============================================================================
// Private Function Definitions

// compare the contents of a buffer to the given value, repeated
static BOOL32 comparePattern(const UInt8 *buffer, UInt8 value, UInt32 length)
{
    UInt32 idx;
    
    for (idx = 0; idx < length; ++idx)
    {
        if (buffer[idx] != value)
        {
            return FALSE32;
        }
    
    }

    return TRUE32;
}

static BOOL32 isSameChipId(const NandChipId chipId1,
                           const NandChipId chipId2)
{
    UInt32 idx;
    // Treat 0x00 as dont-care
    for (idx = 0; idx < COMPARE_NAND_CHIP_ID_SIZE; ++idx)
    {
        if (chipId1[idx] && chipId2[idx] && (chipId1[idx] != chipId2[idx]))
        {
            return FALSE32;
        }
    }
    return TRUE32;
}

static BOOL32 isSameConfigId(const NandConfigId*  configId1,
                             const NandConfigId*  configId2)
{
    if (0 == WMR_MEMCMP(configId1, configId2, sizeof(NandConfigId)))
    {
        return TRUE32;
    }
    else
    {
        return FALSE32;
    }
}

static const NandGeometry* lookupNandGeometry(const NandChipId chipId)
{
    UInt32 count = sizeof(_nandGeometryTable) / sizeof(_nandGeometryTable[0]);
    UInt32 idx;

    for (idx = 0; idx < count; idx++)
    {
        const NandGeometry* geometry = &_nandGeometryTable[idx];
        if (isSameChipId(chipId, geometry->chipId))
        {
            return geometry;
        }
    }

    return NULL;
}

#if AND_ENABLE_NAND_DESCRIPTION_TEXT
static const NandDescription* lookupNandDescription(const NandPackageId*  packageId)
{
    UInt32 idx;

    for (idx = 0; idx < _nandDescriptionTableSize; idx++)
    {
        const NandDescription* description = &_nandDescriptionTable[idx];
        if (isSamePackageId(packageId, &(description->packageId)))
        {
            return description;
        }
    }

    return NULL;
}
#endif

static const NandTiming* lookupNandTiming(const NandConfigId*  configId)
{
    UInt32 count = sizeof(_nandTimingTable) / sizeof(_nandTimingTable[0]);
    UInt32 idx;

    for (idx = 0; idx < count; idx++)
    {
        const NandTiming* timing = &_nandTimingTable[idx];
        if (isSameConfigId(configId, &(timing->configId)))
        {
            return timing;
        }
    }

    return NULL;
}

static const NandBoardSupport* lookupBoardSupport(const NandConfigId*  configId)
{
    UInt32 count = sizeof(_nandBoardSupportTable) / sizeof(_nandBoardSupportTable[0]);
    UInt32 idx;

    for (idx = 0; idx < count; idx++)
    {
        const NandBoardSupport* support = &_nandBoardSupportTable[idx];
        if (isSameConfigId(configId, &(support->configId)))
        {
            return support;
        }
    }

    return NULL;
}

static BOOL32 isValidChipId(NandChipId const chipId)
{
    if (comparePattern(&chipId[0], 0x00, NAND_CHIP_ID_SIZE) ||
        comparePattern(&chipId[0], 0xFF, NAND_CHIP_ID_SIZE))
    {
        return FALSE32;
    }
    else
    {
        return TRUE32;
    }
}

static BOOL32 getChipIdAndTotalCECountAndCEMap(const NandChipId* chipIds, NandChipId* foundChipId, UInt32* totalCECount, UInt32* ceMap)
{
    BOOL32 found = FALSE32;
    UInt32 i;

    *ceMap = 0;
    *totalCECount = 0;

    for (i = 0; i < NAND_MAX_CE_COUNT_TOTAL; i++)
    {
        if (isValidChipId(chipIds[i]))
        {
            if (!found)
            {
                WMR_MEMSET(foundChipId, 0, sizeof(NandChipId));
                WMR_MEMCPY(foundChipId, chipIds[i], VALID_NAND_CHIP_ID_SIZE);
                found = TRUE32;
            }
            (*totalCECount)++;
            (*ceMap) |= (1 << i);
        }
        
    }    

    return found;
}

static BOOL32 areChipIdsIdentical(const NandChipId* chipIds, NandChipId* chipId)
{
    UInt32 i;

    for (i = 0; i < NAND_MAX_CE_COUNT_TOTAL; i++)
    {
        if (isValidChipId(chipIds[i]) && (!isSameChipId(*chipId, chipIds[i])))
        {
            return FALSE32;
        }
    }

    return TRUE32;
}

static UInt32 setBitCount(UInt32 x)
{
    // Count the ones
    UInt32 cnt = 0;

    while (x)
    {
        if (x & 1)
        {
            cnt++;
        }
        x = x >> 1;
    }
    return cnt;
}

static BOOL32 checkCESymmetryAndGetPackageCnt(NandLandingMap *landingMap, UInt32 ceMap, UInt8* pkgCnt)
{
    UInt32 mask;
    UInt32 i = 0;
    UInt32 cePerPkg = setBitCount((*landingMap)[0] & ceMap);
    
    *pkgCnt = 0;

    do
    {
        mask = (*landingMap)[i];

        if (mask & ceMap)
        {
            if (cePerPkg != setBitCount(mask & ceMap))
            {
                return FALSE32;
            }
            (*pkgCnt)++;
        }
        i++;
    }
    while (mask != 0);

    return TRUE32;
}

// =============================================================================
// Public Function Definitions
BOOL32 findNandInfo(const NandChipId*  chipIds,
                    NandInfo*          nandInfo,
                    const UInt32 numActiveDataBus)
{
    NandConfigId detectedConfigId;
    NandChipId chipId;
    UInt32 totalCECount;
    UInt32 ceMap;
    UInt32 i;

    nandInfo->boardInfo = &_nandBoardInfo;
    nandInfo->boardSupport = NULL;
    nandInfo->configTiming = NULL;
    nandInfo->format       = &_nandFormatRaw;

    WMR_MEMSET(&detectedConfigId, 0, sizeof(NandConfigId));

    if (!getChipIdAndTotalCECountAndCEMap(chipIds, &chipId, &totalCECount, &ceMap))
    {
        WMR_PRINT(INIT, "No NAND Detected\n");
        return FALSE32;
    }

    if (!areChipIdsIdentical(chipIds, &chipId))
    {
        WMR_PRINT(INIT, "Chip IDs not identical!\n");
        return FALSE32;
    }

    if (!checkCESymmetryAndGetPackageCnt(&_nandBoardInfo.landingMap, ceMap, &(detectedConfigId.packageCnt)))
    {
        WMR_PRINT(INIT, "Chip IDs not symmetrical!\n");
        return FALSE32;
    }

    detectedConfigId.connectedBusCnt = numActiveDataBus;

    nandInfo->geometry = lookupNandGeometry(chipId);

    if (!nandInfo->geometry)
    {
        WMR_PRINT(INIT, "Unknown Chip ID!\n");
        return FALSE32;
    }
    
    for (i = 0; i < detectedConfigId.packageCnt; i++)
    {
        WMR_MEMCPY(&(detectedConfigId.packageIds[i].chipId), nandInfo->geometry->chipId, sizeof(NandChipId));
        detectedConfigId.packageIds[i].ceCnt = totalCECount / detectedConfigId.packageCnt;
    }

    nandInfo->boardSupport = lookupBoardSupport(&detectedConfigId);
    nandInfo->configTiming = lookupNandTiming(&detectedConfigId);
    
    if (!nandInfo->boardSupport)
    {
        WMR_PRINT(INIT, "Board support not found!\n");
        return FALSE32;
    }

    if (!nandInfo->configTiming)
    {
        WMR_PRINT(INIT, "Nand timing not found!\n");
        return FALSE32;
    }

#if WITH_NAND_DT_CONFIG    
    currentNandInfo = *nandInfo;
    currentCECount = totalCECount;
    currentCEBitmap = ceMap;
    currentActiveDataBus = numActiveDataBus;
#endif //WITH_NAND_DT_CONFIG

    return TRUE32;
}

void setECCLevels(UInt32 numCorrectableBits, UInt32 refreshThresholdBits)
{
    currentCorrectableBits = numCorrectableBits;
    currentRefreshThreshold = refreshThresholdBits;
}

#if WITH_NAND_DT_CONFIG
void addNandProperty(DTNode *node, char *name, uint32_t value)
{
	uint32_t propSize = 0;
	void *propData = NULL;
	
    if (FindProperty(node, &name, &propData, &propSize) && 
        (propSize == sizeof(value)))
    {
        *(uint32_t*)propData = value;
    }
    else
    {
        WMR_PRINT(ERROR, "Failed to find NAND property: %s\n", name);
    }
}

static void fillPpnFeatures(DTNode *node)
{
    char *propName = "ppn-features";
    void *propData = NULL;
    UInt32 propSize = 0;
    UInt32 totalSize = sizeof(_ppn_feature_size) + _ppn_feature_size;
    UInt8 *cursor;

    if (!FindProperty(node, &propName, &propData, &propSize) ||
        (totalSize > propSize))
    {
        WMR_PANIC("expected device tree property %s of at least %u bytes", propName, (unsigned)totalSize);
    }

    cursor = propData;

    WMR_MEMCPY(cursor, &_ppn_feature_size, sizeof(_ppn_feature_size));
    cursor += sizeof(_ppn_feature_size);

    WMR_MEMCPY(cursor, _ppn_feature_list, _ppn_feature_size);
    cursor += _ppn_feature_size;
}

static void fillNandPpnProperties(DTNode *node)
{
    const NandPpn   *ppn = currentNandInfo.ppn;

    addNandProperty(node, "ppn-spec-version", ppn->specVersion);
    addNandProperty(node, "blocks-cau", ppn->blocksPerCau);
    addNandProperty(node, "caus-ce", ppn->causPerCe);
    addNandProperty(node, "slc-pages", ppn->slcPagesPerBlock);
    addNandProperty(node, "mlc-pages", ppn->mlcPagesPerBlock);
    addNandProperty(node, "match-oddeven-caus", ppn->matchOddEvenCaus);
    addNandProperty(node, "cau-bits", ppn->bitsPerCau);
    addNandProperty(node, "page-bits", ppn->bitsPerPage);
    addNandProperty(node, "block-bits", ppn->bitsPerBlock);
    addNandProperty(node, "max-transaction-size", ppn->maxTransactionSize);

    fillPpnFeatures(node);
}

static void fillDbgChipIds(DTNode *node)
{
    char *propName = "dbg-chipids";
    void *propData = NULL;
    UInt32 propSize = 0;
    UInt32 totalSize = _dbg_chipids_size;
    UInt8 *cursor;

    if (NULL == _dbg_chipids_list)
    {
        goto exit;
    }

    if (!FindProperty(node, &propName, &propData, &propSize) ||
        (totalSize > propSize))
    {
        WMR_PRINT(ERROR, "expected device tree property %s of at least %lu bytes", propName, totalSize);
        goto exit;
    }

    cursor = propData;

    WMR_MEMCPY(cursor, _dbg_chipids_list, _dbg_chipids_size);
    cursor += _dbg_chipids_size;

exit:

    return;
}

void fillNandConfigProperties(DTNode *node)
{
    uint32_t propSize = 0;
    void *propData = NULL;
    char *propName = NULL;

    if (0 == currentCECount) {
        fillDbgChipIds(node);
        return;
    }

    // Configuration
    addNandProperty(node, "#ce", currentCECount);
    addNandProperty(node, "ce-bitmap", currentCEBitmap);

    propName = "device-readid";
    if (FindProperty(node, &propName, &propData, &propSize) && 
        (propSize == NAND_CHIP_ID_SIZE))
    {
        WMR_MEMCPY(propData, currentNandInfo.geometry->chipId, NAND_CHIP_ID_SIZE);
    }

    // Geometry
    addNandProperty(node, "#databus", currentActiveDataBus);
    addNandProperty(node, "#ce-blocks", currentNandInfo.geometry->blocksPerCS);
    addNandProperty(node, "#block-pages", currentNandInfo.geometry->pagesPerBlock);
    addNandProperty(node, "#page-bytes", currentNandInfo.geometry->dataBytesPerPage);
    addNandProperty(node, "#spare-bytes", currentNandInfo.geometry->spareBytesPerPage);
    addNandProperty(node, "#die-ce", currentNandInfo.geometry->diesPerCS);
    
    // Formatting
    addNandProperty(node, "bbt-format", currentNandInfo.geometry->initialBBType);
    addNandProperty(node, "vendor-type", currentNandInfo.boardSupport->vsType);
    addNandProperty(node, "ecc-threshold", currentRefreshThreshold);
    addNandProperty(node, "ecc-correctable", currentCorrectableBits);
    
    // Bus Timing
    addNandProperty(node, "read-setup-clks", _readSetupClocks);
    addNandProperty(node, "read-hold-clks", _readHoldClocks);
    addNandProperty(node, "read-dccycle-clks", _readDccycleClocks);
    addNandProperty(node, "write-setup-clks", _writeSetupClocks);
    addNandProperty(node, "write-hold-clks", _writeHoldClocks);

    // NAND Format parameters
    addNandProperty(node, "meta-per-logical-page", currentNandInfo.format->metaPerLogicalPage);
    addNandProperty(node, "valid-meta-per-logical-page", currentNandInfo.format->validMetaPerLogicalPage);
    if (currentNandInfo.format->logicalPageSize == 0)
    {
        // Use native page size
        UInt32 pageSize = currentNandInfo.geometry->dataBytesPerPage;
        addNandProperty(node, "logical-page-size", pageSize);
    }
    else
    {
        addNandProperty(node, "logical-page-size", currentNandInfo.format->logicalPageSize);
    }

    if (toggleDevice)
    {
        addNandProperty(node, "toggle-device",  1);
        addNandProperty(node, "enable-diff-dqs",  _nandBoardInfo.useDiffDQSMode);
        addNandProperty(node, "enable-diff-re",  _nandBoardInfo.useDiffREMode);
        addNandProperty(node, "enable-vref",  _nandBoardInfo.useVref);

        addNandProperty(node, "dqs-half-cycle-clks", _dqsHalfCycleClocks);

        addNandProperty(node, "ce-setup-clks", _ceSetupClocks);
        addNandProperty(node, "ce-hold-clks", _ceHoldClocks);
        addNandProperty(node, "adl-clks", _ADLClocks);
        addNandProperty(node, "whr-clks", _WHRClocks);

        addNandProperty(node, "read-pre-clks", _readPreClocks);
        addNandProperty(node, "read-post-clks", _readPostClocks);
        addNandProperty(node, "write-pre-clks", _writePreClocks);
        addNandProperty(node, "write-post-clks", _writePostClocks);

        addNandProperty(node, "reg-dqs-delay", _regDQSTimingCtrl);
    }
    else
    {
        addNandProperty(node, "toggle-device",  0);
    }

    if (ppnDevice == TRUE32)
    {
        addNandProperty(node, "ppn-device", 1);
        fillNandPpnProperties(node);
    }
    else
    {
        addNandProperty(node, "ppn-device", 0);
    }

    addNandProperty(node, "retire-on-invalid-refresh", _retire_on_invalid_refresh);

}

#endif //WITH_NAND_CONFIG

void reportToggleModeFMCTimingValues(UInt32 dqsHalfCycleClocks,
                                     UInt32 ceSetupClocks,
                                     UInt32 ceHoldClocks,
                                     UInt32 adlClocks,
                                     UInt32 whrClocks,
                                     UInt32 readPreClocks,
                                     UInt32 readPostClocks,
                                     UInt32 writePreClocks,
                                     UInt32 writePostClocks,
                                     UInt32 regDQSTimingCtrl)
{
    _dqsHalfCycleClocks = dqsHalfCycleClocks;
    _ceSetupClocks    = ceSetupClocks;
    _ceHoldClocks     = ceHoldClocks;
    _ADLClocks        = adlClocks;
    _WHRClocks        = whrClocks;
    _readPreClocks    = readPreClocks;
    _readPostClocks   = readPostClocks;
    _writePreClocks   = writePreClocks;
    _writePostClocks  = writePostClocks;
    _regDQSTimingCtrl = regDQSTimingCtrl;
}

void setToggleMode()
{
    toggleDevice = TRUE32;
}


void reportFMCTimingValues(UInt32 readSetupClocks,
                           UInt32 readHoldClocks,
                           UInt32 readDccycleClocks,
                           UInt32 writeSetupClocks,
                           UInt32 writeHoldClocks)
{
    _readSetupClocks = readSetupClocks;
    _readHoldClocks = readHoldClocks;
    _readDccycleClocks = readDccycleClocks;
    _writeSetupClocks =  writeSetupClocks;
    _writeHoldClocks = writeHoldClocks;
}

void setNandIdPpnConfig(const NandChipId*  chipIds,
                        NandInfo*          nandInfo,
                        UInt32             busses,
                        UInt32             ces)
{
#if WITH_NAND_DT_CONFIG
    currentActiveDataBus = busses;
    currentNandInfo = *nandInfo;

    getChipIdAndTotalCECountAndCEMap(chipIds, (NandChipId*)&currentNandInfo.geometry->chipId, &currentCECount, &currentCEBitmap);
    WMR_ASSERT(ces == currentCECount);
#endif //WITH_NAND_DT_CONFIG    	
    nandInfo->format = &_nandFormatPpn;
    ppnDevice = TRUE32;
}

void setPPNOptions(BOOL32 retire_on_invalid_refresh)
{
    _retire_on_invalid_refresh = retire_on_invalid_refresh;
}

void reportPpnFeatures(void *list, UInt32 size)
{
    _ppn_feature_list = list;
    _ppn_feature_size = size;
}

void reportDbgChipIds(void *list, UInt32 size)
{
    _dbg_chipids_list = list;
    _dbg_chipids_size = size;
}

// Bitmap of valid CEs (all CEs are identical to CE0) gets cross-checked
// against landing map from nand table
// If a valid CE bit is set on an invalid landing bit, returns error
BOOL32 checkPpnLandingMap(UInt32 validCEMap)
{
    NandLandingMap *landingMap;
    UInt32 i = 0;
    UInt32 current;
    UInt32 intersect, remainder;

    landingMap = &_nandBoardInfo.landingMap;
    remainder = validCEMap;

    do {
        current = (*landingMap)[i];
        intersect = remainder & current;
        remainder &= ~intersect;
        i++;
    } while ((current != 0) && (remainder != 0));

    if (remainder != 0)
    {
        WMR_PRINT(ERROR, "Valid CE bit detected on an invalid landing bit!\n");
        return FALSE32;
    }
    return TRUE32;
}

const NandFormat * getPPNFormat(void)
{
    return &_nandFormatPpn;
}

BOOL32 targetSupportsToggle(void)
{
    return _nandBoardInfo.useToggleMode ? TRUE32 : FALSE32;
}

BOOL32 targetSupportsDiffDQS(void)
{
    return _nandBoardInfo.useDiffDQSMode ? TRUE32 : FALSE32;
}

BOOL32 targetSupportsDiffRE(void)
{
    return _nandBoardInfo.useDiffREMode ? TRUE32 : FALSE32;
}

BOOL32 targetSupportsVREF(void)
{
    return _nandBoardInfo.useVref ? TRUE32 : FALSE32;
}

BOOL32 targetSupportsSHC(void)
{
    return _nandBoardInfo.useSingleHostChannel;
}

BOOL32 targetSupportsSingleCE(void)
{
    return _nandBoardInfo.allowSingleChipEnable;
}

// Run only NAND chip identification
int flash_nand_id(void)
{
	OAM_Init();

	if (FIL_SUCCESS != FIL_Init())
		return -1;
		
	return 0;
}
