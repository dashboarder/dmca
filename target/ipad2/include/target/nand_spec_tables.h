
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
        /* chipId            */ { 0xEC, 0xD7, 0x94, 0x7A },
        /* blocksPerCS       */ 4152,
        /* pagesPerBlock     */ 128,
        /* dataBytesPerPage  */ 8192,
        /* spareBytesPerPage */ 640,
        /* eccPer512Bytes    */ 16,
        /* initialBBType     */ INIT_BBT_SAMSUNG_MLC_8K,
        /* diesPerCS         */ 1
    },
    {
        /* chipId            */ { 0xEC, 0xDE, 0xD5, 0x7A },
        /* blocksPerCS       */ 8304,
        /* pagesPerBlock     */ 128,
        /* dataBytesPerPage  */ 8192,
        /* spareBytesPerPage */ 640,
        /* eccPer512Bytes    */ 16,
        /* initialBBType     */ INIT_BBT_SAMSUNG_MLC_8K,
        /* diesPerCS         */ 2
    },
    {
        /* chipId            */ { 0xEC, 0xD7, 0x94, 0x72 },
        /* blocksPerCS       */ 4152,
        /* pagesPerBlock     */ 128,
        /* dataBytesPerPage  */ 8192,
        /* spareBytesPerPage */ 436,
        /* eccPer512Bytes    */ 12,
        /* initialBBType     */ INIT_BBT_SAMSUNG_MLC_8K,
        /* diesPerCS         */ 1
    },
    {
        /* chipId            */ { 0xEC, 0xDE, 0xD5, 0x72 },
        /* blocksPerCS       */ 8304,
        /* pagesPerBlock     */ 128,
        /* dataBytesPerPage  */ 8192,
        /* spareBytesPerPage */ 436,
        /* eccPer512Bytes    */ 12,
        /* initialBBType     */ INIT_BBT_SAMSUNG_MLC_8K,
        /* diesPerCS         */ 2
    },
    {
        /* chipId            */ { 0x98, 0xE7, 0x94, 0x32 },
        /* blocksPerCS       */ 4100,
        /* pagesPerBlock     */ 128,
        /* dataBytesPerPage  */ 8192,
        /* spareBytesPerPage */ 448,
        /* eccPer512Bytes    */ 16,
        /* initialBBType     */ INIT_BBT_TOSHIBA_MLC,
        /* diesPerCS         */ 1
    },
    {
        /* chipId            */ { 0x98, 0xEE, 0x95, 0x32 },
        /* blocksPerCS       */ 8200,
        /* pagesPerBlock     */ 128,
        /* dataBytesPerPage  */ 8192,
        /* spareBytesPerPage */ 448,
        /* eccPer512Bytes    */ 24,
        /* initialBBType     */ INIT_BBT_TOSHIBA_MLC,
        /* diesPerCS         */ 2
    },
    {
        /* chipId            */ { 0x2C, 0x88, 0x04, 0x4B },
        /* blocksPerCS       */ 4096,
        /* pagesPerBlock     */ 256,
        /* dataBytesPerPage  */ 8192,
        /* spareBytesPerPage */ 448,
        /* eccPer512Bytes    */ 24,
        /* initialBBType     */ INIT_BBT_MICRON_ONFI,
        /* diesPerCS         */ 1
    },
    {
        /* chipId            */ { 0x45, 0x48, 0x94, 0x32 },
        /* blocksPerCS       */ 4096,
        /* pagesPerBlock     */ 128,
        /* dataBytesPerPage  */ 8192,
        /* spareBytesPerPage */ 448,
        /* eccPer512Bytes    */ 8,
        /* initialBBType     */ INIT_BBT_SANDISK_MLC,
        /* diesPerCS         */ 1
    },
    {
        /* chipId            */ { 0x45, 0x68, 0x95, 0x32 },
        /* blocksPerCS       */ 8192,
        /* pagesPerBlock     */ 128,
        /* dataBytesPerPage  */ 8192,
        /* spareBytesPerPage */ 448,
        /* eccPer512Bytes    */ 8,
        /* initialBBType     */ INIT_BBT_SANDISK_MLC,
        /* diesPerCS         */ 2
    },
    {
        /* chipId            */ { 0x45, 0x2D, 0x94, 0x82 },
        /* blocksPerCS       */ 4096,
        /* pagesPerBlock     */ 256,
        /* dataBytesPerPage  */ 8192,
        /* spareBytesPerPage */ 640,
        /* eccPer512Bytes    */ 16,
        /* initialBBType     */ INIT_BBT_SANDISK_MLC,
        /* diesPerCS         */ 1
    },
    {
        /* chipId            */ { 0x45, 0x4D, 0x94, 0x82 },
        /* blocksPerCS       */ 4096,
        /* pagesPerBlock     */ 256,
        /* dataBytesPerPage  */ 8192,
        /* spareBytesPerPage */ 640,
        /* eccPer512Bytes    */ 16,
        /* initialBBType     */ INIT_BBT_SANDISK_MLC,
        /* diesPerCS         */ 1
    },
    {
        /* chipId            */ { 0x45, 0x29, 0x94, 0x32 },
        /* blocksPerCS       */ 4096,
        /* pagesPerBlock     */ 128,
        /* dataBytesPerPage  */ 8192,
        /* spareBytesPerPage */ 640,
        /* eccPer512Bytes    */ 16,
        /* initialBBType     */ INIT_BBT_SANDISK_MLC,
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
                    /* chipId             */ { 0xEC, 0xD7, 0x94, 0x7A },
                    /* ceCnt              */ 4
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
        /* readValidNanosecs        */ 15
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0xEC, 0xD7, 0x94, 0x7A },
                    /* ceCnt              */ 4
                },
                {
                    /* chipId             */ { 0xEC, 0xD7, 0x94, 0x7A },
                    /* ceCnt              */ 4
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
        /* readValidNanosecs        */ 15
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0xEC, 0xDE, 0xD5, 0x7A },
                    /* ceCnt              */ 4
                },
                {
                    /* chipId             */ { 0xEC, 0xDE, 0xD5, 0x7A },
                    /* ceCnt              */ 4
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
        /* readValidNanosecs        */ 15
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0xEC, 0xD7, 0x94, 0x72 },
                    /* ceCnt              */ 4
                }
            }
        },
        /* writeCycleNanosecs       */ 30,
        /* writeSetupNanosecs       */ 15,
        /* writeHoldNanosecs        */ 10,
        /* readCycleNanosecs        */ 30,
        /* readSetupNanosecs        */ 15,
        /* readHoldNanosecs         */ 10,
        /* readDelayNanosecs        */ 25,
        /* readValidNanosecs        */ 15
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0xEC, 0xD7, 0x94, 0x72 },
                    /* ceCnt              */ 4
                },
                {
                    /* chipId             */ { 0xEC, 0xD7, 0x94, 0x72 },
                    /* ceCnt              */ 4
                }
            }
        },
        /* writeCycleNanosecs       */ 30,
        /* writeSetupNanosecs       */ 15,
        /* writeHoldNanosecs        */ 10,
        /* readCycleNanosecs        */ 30,
        /* readSetupNanosecs        */ 15,
        /* readHoldNanosecs         */ 10,
        /* readDelayNanosecs        */ 25,
        /* readValidNanosecs        */ 15
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0xEC, 0xDE, 0xD5, 0x72 },
                    /* ceCnt              */ 4
                }
            }
        },
        /* writeCycleNanosecs       */ 30,
        /* writeSetupNanosecs       */ 15,
        /* writeHoldNanosecs        */ 10,
        /* readCycleNanosecs        */ 30,
        /* readSetupNanosecs        */ 15,
        /* readHoldNanosecs         */ 10,
        /* readDelayNanosecs        */ 25,
        /* readValidNanosecs        */ 15
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0xEC, 0xDE, 0xD5, 0x72 },
                    /* ceCnt              */ 4
                },
                {
                    /* chipId             */ { 0xEC, 0xDE, 0xD5, 0x72 },
                    /* ceCnt              */ 4
                }
            }
        },
        /* writeCycleNanosecs       */ 30,
        /* writeSetupNanosecs       */ 15,
        /* writeHoldNanosecs        */ 10,
        /* readCycleNanosecs        */ 30,
        /* readSetupNanosecs        */ 15,
        /* readHoldNanosecs         */ 10,
        /* readDelayNanosecs        */ 25,
        /* readValidNanosecs        */ 15
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0xEC, 0xDE, 0xD5, 0x72 },
                    /* ceCnt              */ 8
                }
            }
        },
        /* writeCycleNanosecs       */ 30,
        /* writeSetupNanosecs       */ 15,
        /* writeHoldNanosecs        */ 10,
        /* readCycleNanosecs        */ 30,
        /* readSetupNanosecs        */ 15,
        /* readHoldNanosecs         */ 10,
        /* readDelayNanosecs        */ 25,
        /* readValidNanosecs        */ 15
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0xEC, 0xDE, 0xD5, 0x72 },
                    /* ceCnt              */ 8
                },
                {
                    /* chipId             */ { 0xEC, 0xDE, 0xD5, 0x72 },
                    /* ceCnt              */ 8
                }
            }
        },
        /* writeCycleNanosecs       */ 30,
        /* writeSetupNanosecs       */ 15,
        /* writeHoldNanosecs        */ 10,
        /* readCycleNanosecs        */ 30,
        /* readSetupNanosecs        */ 15,
        /* readHoldNanosecs         */ 10,
        /* readDelayNanosecs        */ 25,
        /* readValidNanosecs        */ 15
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x98, 0xE7, 0x94, 0x32 },
                    /* ceCnt              */ 4
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
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x98, 0xE7, 0x94, 0x32 },
                    /* ceCnt              */ 4
                },
                {
                    /* chipId             */ { 0x98, 0xE7, 0x94, 0x32 },
                    /* ceCnt              */ 4
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
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x98, 0xEE, 0x95, 0x32 },
                    /* ceCnt              */ 4
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
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x98, 0xEE, 0x95, 0x32 },
                    /* ceCnt              */ 4
                },
                {
                    /* chipId             */ { 0x98, 0xEE, 0x95, 0x32 },
                    /* ceCnt              */ 4
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
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x98, 0xEE, 0x95, 0x32 },
                    /* ceCnt              */ 8
                },
                {
                    /* chipId             */ { 0x98, 0xEE, 0x95, 0x32 },
                    /* ceCnt              */ 8
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
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x2C, 0x88, 0x04, 0x4B },
                    /* ceCnt              */ 2
                }
            }
        },
        /* writeCycleNanosecs       */ 20,
        /* writeSetupNanosecs       */ 10,
        /* writeHoldNanosecs        */ 7,
        /* readCycleNanosecs        */ 20,
        /* readSetupNanosecs        */ 10,
        /* readHoldNanosecs         */ 7,
        /* readDelayNanosecs        */ 16,
        /* readValidNanosecs        */ 15
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x2C, 0x88, 0x04, 0x4B },
                    /* ceCnt              */ 2
                },
                {
                    /* chipId             */ { 0x2C, 0x88, 0x04, 0x4B },
                    /* ceCnt              */ 2
                }
            }
        },
        /* writeCycleNanosecs       */ 20,
        /* writeSetupNanosecs       */ 10,
        /* writeHoldNanosecs        */ 7,
        /* readCycleNanosecs        */ 20,
        /* readSetupNanosecs        */ 10,
        /* readHoldNanosecs         */ 7,
        /* readDelayNanosecs        */ 16,
        /* readValidNanosecs        */ 15
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x2C, 0x88, 0x04, 0x4B },
                    /* ceCnt              */ 4
                },
                {
                    /* chipId             */ { 0x2C, 0x88, 0x04, 0x4B },
                    /* ceCnt              */ 4
                }
            }
        },
        /* writeCycleNanosecs       */ 20,
        /* writeSetupNanosecs       */ 10,
        /* writeHoldNanosecs        */ 7,
        /* readCycleNanosecs        */ 20,
        /* readSetupNanosecs        */ 10,
        /* readHoldNanosecs         */ 7,
        /* readDelayNanosecs        */ 16,
        /* readValidNanosecs        */ 15
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x45, 0x2D, 0x94, 0x82 },
                    /* ceCnt              */ 2
                },
                {
                    /* chipId             */ { 0x45, 0x2D, 0x94, 0x82 },
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
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x45, 0x48, 0x94, 0x32 },
                    /* ceCnt              */ 4
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
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x45, 0x68, 0x95, 0x32 },
                    /* ceCnt              */ 4
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
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x45, 0x4D, 0x94, 0x82 },
                    /* ceCnt              */ 4
                },
                {
                    /* chipId             */ { 0x45, 0x4D, 0x94, 0x82 },
                    /* ceCnt              */ 4
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
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x45, 0x29, 0x94, 0x32 },
                    /* ceCnt              */ 2
                }
            }
        },
        /* writeCycleNanosecs       */ 20,
        /* writeSetupNanosecs       */ 12,
        /* writeHoldNanosecs        */ 10,
        /* readCycleNanosecs        */ 20,
        /* readSetupNanosecs        */ 12,
        /* readHoldNanosecs         */ 10,
        /* readDelayNanosecs        */ 20,
        /* readValidNanosecs        */ 25
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x45, 0x48, 0x94, 0x32 },
                    /* ceCnt              */ 4
                },
                {
                    /* chipId             */ { 0x45, 0x48, 0x94, 0x32 },
                    /* ceCnt              */ 4
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
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x45, 0x68, 0x95, 0x32 },
                    /* ceCnt              */ 4
                },
                {
                    /* chipId             */ { 0x45, 0x68, 0x95, 0x32 },
                    /* ceCnt              */ 4
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
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* chipId             */ { 0x45, 0x2D, 0x94, 0x82 },
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
        /* chipId            */ { 0xEC, 0xD7, 0x94, 0x7A },
        /* die               */ "27nm",
        /* vendor_name       */ "Samsung"
    },
    {
        /* chipId            */ { 0xEC, 0xDE, 0xD5, 0x7A },
        /* die               */ "27nm",
        /* vendor_name       */ "Samsung"
    },
    {
        /* chipId            */ { 0xEC, 0xD7, 0x94, 0x72 },
        /* die               */ "35nm",
        /* vendor_name       */ "Samsung"
    },
    {
        /* chipId            */ { 0xEC, 0xDE, 0xD5, 0x72 },
        /* die               */ "35nm",
        /* vendor_name       */ "Samsung"
    },
    {
        /* chipId            */ { 0x98, 0xE7, 0x94, 0x32 },
        /* die               */ "32nm",
        /* vendor_name       */ "Toshiba"
    },
    {
        /* chipId            */ { 0x98, 0xEE, 0x95, 0x32 },
        /* die               */ "32nm",
        /* vendor_name       */ "Toshiba"
    },
    {
        /* chipId            */ { 0x2C, 0x88, 0x04, 0x4B },
        /* die               */ "25nm",
        /* vendor_name       */ "Micron"
    },
    {
        /* chipId            */ { 0x45, 0x48, 0x94, 0x32 },
        /* die               */ "32nm",
        /* vendor_name       */ "Sandisk"
    },
    {
        /* chipId            */ { 0x45, 0x68, 0x95, 0x32 },
        /* die               */ "32nm",
        /* vendor_name       */ "Sandisk"
    },
    {
        /* chipId            */ { 0x45, 0x2D, 0x94, 0x82 },
        /* die               */ "24nm",
        /* vendor_name       */ "Sandisk"
    },
    {
        /* chipId            */ { 0x45, 0x4D, 0x94, 0x82 },
        /* die               */ "24nm",
        /* vendor_name       */ "Sandisk"
    },
    {
        /* chipId            */ { 0x45, 0x29, 0x94, 0x32 },
        /* die               */ "24nm",
        /* vendor_name       */ "Sandisk"
    }
};

// =============================================================================
// Nand Description Table

static const NandDescription _nandDescriptionTable[] =
{
    {
        /* packageId          */
        {
            /* chipId            */ { 0xEC, 0xD7, 0x94, 0x7A },
            /* ceCnt             */ 4
        },
        /* appleName          */ "TBD",
        /* vendor_part_num    */ "K9HDG08U5A-L",
        /* package_type       */ "QDP",
        /* package_size       */ "1.00mm LGA"
    },
    {
        /* packageId          */
        {
            /* chipId            */ { 0xEC, 0xDE, 0xD5, 0x7A },
            /* ceCnt             */ 4
        },
        /* appleName          */ "335S0791",
        /* vendor_part_num    */ "K9PFG08U5A-L",
        /* package_type       */ "ODP",
        /* package_size       */ "1.00mm LGA"
    },
    {
        /* packageId          */
        {
            /* chipId            */ { 0xEC, 0xD7, 0x94, 0x72 },
            /* ceCnt             */ 4
        },
        /* appleName          */ "335S0682",
        /* vendor_part_num    */ "K9HDG08U5M-LCB0",
        /* package_type       */ "QDP",
        /* package_size       */ "1.00mm LGA"
    },
    {
        /* packageId          */
        {
            /* chipId            */ { 0xEC, 0xDE, 0xD5, 0x72 },
            /* ceCnt             */ 4
        },
        /* appleName          */ "335S0665",
        /* vendor_part_num    */ "K9PEG08U5M-LCB0",
        /* package_type       */ "ODP",
        /* package_size       */ "1.00mm LGA"
    },
    {
        /* packageId          */
        {
            /* chipId            */ { 0xEC, 0xDE, 0xD5, 0x72 },
            /* ceCnt             */ 8
        },
        /* appleName          */ "335S0707",
        /* vendor_part_num    */ "K9UHG08U8M-LCB0",
        /* package_type       */ "16DP",
        /* package_size       */ "1.40mm LGA"
    },
    {
        /* packageId          */
        {
            /* chipId            */ { 0x98, 0xE7, 0x94, 0x32 },
            /* ceCnt             */ 4
        },
        /* appleName          */ "335S0701",
        /* vendor_part_num    */ "TH58NVG7D2FLA89",
        /* package_type       */ "QDP",
        /* package_size       */ "1.00mm LGA"
    },
    {
        /* packageId          */
        {
            /* chipId            */ { 0x98, 0xEE, 0x95, 0x32 },
            /* ceCnt             */ 4
        },
        /* appleName          */ "335S0702",
        /* vendor_part_num    */ "TH58NVG8D2FLA89",
        /* package_type       */ "ODP",
        /* package_size       */ "1.00mm LGA"
    },
    {
        /* packageId          */
        {
            /* chipId            */ { 0x98, 0xEE, 0x95, 0x32 },
            /* ceCnt             */ 8
        },
        /* appleName          */ "335S0708",
        /* vendor_part_num    */ "TH58NVG9D2FLAB7",
        /* package_type       */ "16DP",
        /* package_size       */ "1.40mm LGA"
    },
    {
        /* packageId          */
        {
            /* chipId            */ { 0x2C, 0x88, 0x04, 0x4B },
            /* ceCnt             */ 2
        },
        /* appleName          */ "335S0766",
        /* vendor_part_num    */ "MT29F128G08CEAAAC5",
        /* package_type       */ "DDP",
        /* package_size       */ "1.00mm LGA"
    },
    {
        /* packageId          */
        {
            /* chipId            */ { 0x2C, 0x88, 0x04, 0x4B },
            /* ceCnt             */ 4
        },
        /* appleName          */ "335S0767",
        /* vendor_part_num    */ "MT29F256G08CMAAAC5",
        /* package_type       */ "QDP",
        /* package_size       */ "1.00mm LGA"
    },
    {
        /* packageId          */
        {
            /* chipId            */ { 0x45, 0x48, 0x94, 0x32 },
            /* ceCnt             */ 4
        },
        /* appleName          */ "335S0721",
        /* vendor_part_num    */ "SDZNNMCHER-016G",
        /* package_type       */ "QDP",
        /* package_size       */ "1mm LGA"
    },
    {
        /* packageId          */
        {
            /* chipId            */ { 0x45, 0x68, 0x95, 0x32 },
            /* ceCnt             */ 4
        },
        /* appleName          */ "335S0722",
        /* vendor_part_num    */ "SDZNNMDHER-032G",
        /* package_type       */ "ODP",
        /* package_size       */ "1mm LGA"
    },
    {
        /* packageId          */
        {
            /* chipId            */ { 0x45, 0x2D, 0x94, 0x82 },
            /* ceCnt             */ 2
        },
        /* appleName          */ "TBD",
        /* vendor_part_num    */ "SDZNOQBHER-016G",
        /* package_type       */ "DDP",
        /* package_size       */ "1mm LGA"
    },
    {
        /* packageId          */
        {
            /* chipId            */ { 0x45, 0x4D, 0x94, 0x82 },
            /* ceCnt             */ 4
        },
        /* appleName          */ "TBD",
        /* vendor_part_num    */ "SDZNOQCHER-032G",
        /* package_type       */ "QDP",
        /* package_size       */ "1mm LGA"
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
                    /* deviceId  */ { 0xEC, 0xD7, 0x94, 0x7A },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_HYNIX_2P
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0xEC, 0xD7, 0x94, 0x7A },
                    /* ceCnt     */ 4
                },
                {
                    /* deviceId  */ { 0xEC, 0xD7, 0x94, 0x7A },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_HYNIX_2P
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0xEC, 0xDE, 0xD5, 0x7A },
                    /* ceCnt     */ 4
                },
                {
                    /* deviceId  */ { 0xEC, 0xDE, 0xD5, 0x7A },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_HYNIX_2P
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0xEC, 0xD7, 0x94, 0x72 },
                    /* ceCnt     */ 4
                },
                {
                    /* deviceId  */ { 0xEC, 0xD7, 0x94, 0x72 },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_HYNIX_2P
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0xEC, 0xD7, 0x94, 0x72 },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_HYNIX_2P
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0xEC, 0xDE, 0xD5, 0x72 },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_HYNIX_2P
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0xEC, 0xDE, 0xD5, 0x72 },
                    /* ceCnt     */ 4
                },
                {
                    /* deviceId  */ { 0xEC, 0xDE, 0xD5, 0x72 },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_HYNIX_2P
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0xEC, 0xDE, 0xD5, 0x72 },
                    /* ceCnt     */ 8
                },
                {
                    /* deviceId  */ { 0xEC, 0xDE, 0xD5, 0x72 },
                    /* ceCnt     */ 8
                }
            }
        },
        /* vsType                */ FIL_VS_SIMPLE
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0xEC, 0xDE, 0xD5, 0x72 },
                    /* ceCnt     */ 8
                }
            }
        },
        /* vsType                */ FIL_VS_HYNIX_2P
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x98, 0xEE, 0x95, 0x32 },
                    /* ceCnt     */ 8
                },
                {
                    /* deviceId  */ { 0x98, 0xEE, 0x95, 0x32 },
                    /* ceCnt     */ 8
                }
            }
        },
        /* vsType                */ FIL_VS_SIMPLE
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x98, 0xE7, 0x94, 0x32 },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_TOSHIBA_2P_EXT
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x98, 0xE7, 0x94, 0x32 },
                    /* ceCnt     */ 4
                },
                {
                    /* deviceId  */ { 0x98, 0xE7, 0x94, 0x32 },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_TOSHIBA_2P_EXT
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x98, 0xEE, 0x95, 0x32 },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_TOSHIBA_2P_EXT
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x98, 0xEE, 0x95, 0x32 },
                    /* ceCnt     */ 4
                },
                {
                    /* deviceId  */ { 0x98, 0xEE, 0x95, 0x32 },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_TOSHIBA_2P_EXT
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x2C, 0x88, 0x04, 0x4B },
                    /* ceCnt     */ 2
                }
            }
        },
        /* vsType                */ FIL_VS_ONFI_2P
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x2C, 0x88, 0x04, 0x4B },
                    /* ceCnt     */ 2
                },
                {
                    /* deviceId  */ { 0x2C, 0x88, 0x04, 0x4B },
                    /* ceCnt     */ 2
                }
            }
        },
        /* vsType                */ FIL_VS_ONFI_2P
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x2C, 0x88, 0x04, 0x4B },
                    /* ceCnt     */ 4
                },
                {
                    /* deviceId  */ { 0x2C, 0x88, 0x04, 0x4B },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_ONFI_2P
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x45, 0x4D, 0x94, 0x82 },
                    /* ceCnt     */ 4
                },
                {
                    /* deviceId  */ { 0x45, 0x4D, 0x94, 0x82 },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_TOSHIBA_2P_EXT
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x45, 0x29, 0x94, 0x32 },
                    /* ceCnt     */ 2
                }
            }
        },
        /* vsType                */ FIL_VS_TOSHIBA_2P_EXT
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x45, 0x2D, 0x94, 0x82 },
                    /* ceCnt     */ 2
                }
            }
        },
        /* vsType                */ FIL_VS_TOSHIBA_2P_EXT
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x45, 0x48, 0x94, 0x32 },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_TOSHIBA_2P_EXT
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x45, 0x2D, 0x94, 0x82 },
                    /* ceCnt     */ 2
                },
                {
                    /* deviceId  */ { 0x45, 0x2D, 0x94, 0x82 },
                    /* ceCnt     */ 2
                }
            }
        },
        /* vsType                */ FIL_VS_TOSHIBA_2P_EXT
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x45, 0x48, 0x94, 0x32 },
                    /* ceCnt     */ 4
                },
                {
                    /* deviceId  */ { 0x45, 0x48, 0x94, 0x32 },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_TOSHIBA_2P_EXT
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 1,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x45, 0x68, 0x95, 0x32 },
                    /* ceCnt     */ 4
                }
            }
        },
        /* vsType                */ FIL_VS_TOSHIBA_2P_EXT
    },
    {
        /* configId              */
        {
            /* connectedBusCnt   */ 2,
            /* packageCnt        */ 2,
            /* packageIds[]      */
            {
                {
                    /* deviceId  */ { 0x45, 0x68, 0x95, 0x32 },
                    /* ceCnt     */ 4
                },
                {
                    /* deviceId  */ { 0x45, 0x68, 0x95, 0x32 },
                    /* ceCnt     */ 4
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

    /* socToNandRiseNanosecs  */ 3,
    /* socToNandFallNanosecs  */ 3,
    /* nandToSocRiseNanosecs  */ 5,
    /* nandToSocFallNanosecs  */ 5,

    /* landing map */
    {
        0x00003333,
        0x0000CCCC
    },

    /* useToggleMode          */ FALSE32,
    /* useDiffDQSMode         */ FALSE32,
    /* useDiffREMode          */ FALSE32,
    /* useVref                */ FALSE32,
    /* useSingleHostChannel   */ FALSE32,
    /* allowSingleChipEnable  */ FALSE32
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

