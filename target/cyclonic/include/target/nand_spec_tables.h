
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
    {
        /* chipId            */ { 0x98, 0xE7, 0x94, 0x32 },
        /* blocksPerCS       */ 4100,
        /* pagesPerBlock     */ 128,
        /* dataBytesPerPage  */ 8192,
        /* spareBytesPerPage */ 448,
        /* eccPer512Bytes    */ 16,
        /* initialBBType     */ INIT_BBT_TOSHIBA_MLC,
        /* diesPerCS         */ 1
    }
};

// =============================================================================
// Nand Timing Table

static NandTiming _nandTimingTable[] =
{
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x98, 0xE7, 0x94, 0x32 },
                    /* ceCnt              */ 2
                }
            }
        },
        /* writeCycleNanosecs       */ 25,
        /* writeSetupNanosecs       */ 12,
        /* writeHoldNanosecs        */ 10,
        /* readCycleNanosecs        */ 25,
        /* readSetupNanosecs        */ 12,
        /* readHoldNanosecs         */ 10,
        /* readDelayNanosecs        */ 20,
        /* readValidNanosecs        */ 25
    }
};

#if (defined (AND_ENABLE_NAND_DESCRIPTION_TEXT) || AND_ENABLE_NAND_DESCRIPTION_TEXT)
// =============================================================================
// NandChipId Description
const NandChipIdDescription _nandChipIdDescriptionTable[] =
{
    {
        /* chipId            */ { 0x98, 0xE7, 0x94, 0x32 },
        /* die               */ "32nm",
        /* vendor_name       */ "Toshiba"
    }
};

// =============================================================================
// Nand Description Table

static const NandDescription _nandDescriptionTable[] =
{
    {
        /* packageId          */
        {
            /* chipId            */ { 0x98, 0xE7, 0x94, 0x32 },
            /* ceCnt             */ 2
        },
        /* appleName          */ "335S0700",
        /* vendor_part_num    */ "TH58NVG6D2FLA49",
        /* package_type       */ "DDP",
        /* package_size       */ "1.00mm LGA"
    }
};
const UInt32 _nandDescriptionTableSize = sizeof(_nandDescriptionTable) / sizeof(NandDescription);

#endif // (defined(AND_ENABLE_NAND_DESCRIPTION_TEXT) || AND_ENABLE_NAND_DESCRIPTION_TEXT)

// =============================================================================
// Nand Board Support Table

static NandBoardSupport _nandBoardSupportTable[] =
{
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x98, 0xE7, 0x94, 0x32 },
                    /* ceCnt     */ 2
                }
            }
        },
        /* vsType                */ FIL_VS_TOSHIBA_2P_EXT
    }
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
        0x00000303
    },

    /* useToggleMode          */ 0,
    /* useDiffDQSMode         */ 0,
    /* useDiffREMode          */ 0,
    /* useVref                */ 0
};

static NandFormat _nandFormatPpn =
{
    /* metaPerLogicalPage          */ 16,
    /* validMetaPerLogicalPage     */ 16,
    /* logicalPageSize             */ 0
};

static NandFormat _nandFormatRaw =
{
    /* metaPerLogicalPage          */ 12,
    /* validMetaPerLogicalPage     */ 10,
    /* logicalPageSize             */ 0
};

