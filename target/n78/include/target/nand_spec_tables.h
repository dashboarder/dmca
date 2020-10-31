
// Copyright (C) 2008-2010 Apple, Inc. All rights reserved.
//
// This document is the property of Apple, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple, Inc.
//
// NOTE: This is a generated file and should not be hand-edited!
//

// =============================================================================
// Nand Geometry Table

static NandGeometry _nandGeometryTable[] =
{
};

// =============================================================================
// Nand Timing Table

static NandTiming _nandTimingTable[] =
{
};

#if (defined (AND_ENABLE_NAND_DESCRIPTION_TEXT) || AND_ENABLE_NAND_DESCRIPTION_TEXT)
// =============================================================================
// NandChipId Description
const NandChipIdDescription _nandChipIdDescriptionTable[] =
{
};

// =============================================================================
// Nand Description Table

static const NandDescription _nandDescriptionTable[] =
{
};
const UInt32 _nandDescriptionTableSize = sizeof(_nandDescriptionTable) / sizeof(NandDescription);

#endif // (defined(AND_ENABLE_NAND_DESCRIPTION_TEXT) || AND_ENABLE_NAND_DESCRIPTION_TEXT)

// =============================================================================
// Nand Board Support Table

static NandBoardSupport _nandBoardSupportTable[] =
{
};

// =============================================================================
// Nand Board Timing Table

static NandBoardInfo _nandBoardInfo =
{
    /* controlDstr            */ NAND_DRIVE_STRENGTH_DEFAULT,
    /* dataDstr               */ NAND_DRIVE_STRENGTH_DEFAULT,
    /* chipEnableDstr         */ NAND_DRIVE_STRENGTH_DEFAULT,

    /* socToNandRiseNanosecs  */ 10,
    /* socToNandFallNanosecs  */ 10,
    /* nandToSocRiseNanosecs  */ 10,
    /* nandToSocFallNanosecs  */ 10,

    /* landing map */
    {
        0x00000101
    },

    /* useToggleMode          */ TRUE32,
    /* useDiffDQSMode         */ FALSE32,
    /* useDiffREMode          */ FALSE32,
    /* useVref                */ TRUE32,
    /* useSingleHostChannel   */ FALSE32,
    /* allowSingleChipEnable  */ TRUE32
};

static NandFormat _nandFormatPpn =
{
    /* metaPerLogicalPage          */ 16,
    /* validMetaPerLogicalPage     */ 16,
    /* logicalPageSize             */ 4096
};

static NandFormat _nandFormatRaw =
{
    /* metaPerLogicalPage          */ 12,
    /* validMetaPerLogicalPage     */ 10,
    /* logicalPageSize             */ 0
};

