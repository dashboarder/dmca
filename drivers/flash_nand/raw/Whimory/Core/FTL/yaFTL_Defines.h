/*
 * Copyright (c) 2008-2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef  __YAFTL_DEFINES_H__
#define  __YAFTL_DEFINES_H__


#define YAFTLDBG_INIT       (1 << 0)
#define YAFTLDBG_FORMAT     (1 << 1)
#define YAFTLDBG_READ       (1 << 2)
#define YAFTLDBG_WRITE      (1 << 3)
#define YAFTLDBG_ERASE      (1 << 4)
#define YAFTLDBG_MISC       (1 << 5)
#define YAFTLDBG_ERROR      (1 << 6)
#define YAFTLDBG_OPEN       (1 << 7)
#define YAFTLDBG_INFO       (1 << 8)

//#define YAFTL_DEBUG         (YAFTLDBG_INIT | YAFTLDBG_ERROR | YAFTLDBG_INFO)
#define YAFTL_DEBUG         (YAFTLDBG_ERROR)
#ifdef YAFTL_DEBUG
#define YAFTL_ENABLE_DEBUG_PRINT (0xff)

# define debug(fac, fmt, args ...)                   \
    do { \
        if (YAFTLDBG_ ## fac & YAFTL_DEBUG & YAFTL_ENABLE_DEBUG_PRINT) { \
            _WMR_PRINT("yaFTL::%s(l:%d): " fmt "\n", __FUNCTION__, __LINE__, ## args); \
        }                               \
    } while (0)
#else
# define debug(fac, fmt, args ...)   do { } while (0)
#endif

#define FREE_BLK_TRH                (5)     /* number of units we require to be avaiable at any time */
#define INDEX_AREA                  (1)     /* block belongs to Index area */
#define DATA_AREA                   (2)     /* block belongs to data area */
#define FREE_I_BLOCK_TRS            (2)     /* minimum free blocks within index area */
#define DEFAULT_INDEX_CACHE_SIZE    (10)    /* index cached pages */
#define INDEX_BLOCKS_RATIO          (3)     /* ratio of index area in flash to required index area by flash size */
#define DEFAULT_EXPO_RATIO          (99)    /* percentage of data area exposed to file system */
#define CXT_ERASE_LIMIT             (30)


#define IC_DIRTY_PAGE 1
#define IC_BUSY_PAGE 2
#define FREE_CACHE_PAGE 0xffff

#define BLOCK_FREE                  (0xFF)  /* block marked as free */
#define BLOCK_ALLOCATED             (1)     /* block marked as allocated for data area */
#define BLOCK_CURRENT               (4)     /* block marked as current in data area */

#define BLOCK_GC                    (8)     /* block in data area goes through GC process */

#define BLOCK_I_FREE                (0xFF)  /* block marked as free */
#define BLOCK_I_CURRENT             (32)    /* block marked as current index block */
#define BLOCK_I_ALLOCATED           (64)    /* block marked as idex block */
#define BLOCK_I_GC                  (128)   /* block in index area goes through GC process */
#define BLOCK_TO_BE_ERASED          (0x1)  /* erase block before use . make sure the block is not erased before current block is closed  */
#define BLOCK_TO_BE_ERASED_ALIGNED_DATA  (0x2)  /* erase block before use . make sure the block is not erased before current block is closed  */
#define BLOCK_TO_BE_ERASED_ALIGNED_INDEX  (0x4)  /* erase block before use . make sure the block is not erased before current block is closed  */
#define BLOCK_READ_SCATTER          (0x8)        /* mark a block as being used in multi-read operation */  
#define BLOCK_TO_BE_MOVED           (0x80)  /* use it as an idication that block is to be refreshed after full mount is complete due to UECCs */

#define ERASE_IDLE_LIMIT            (5)     /* number of free blocks to be erased in idle time */

#define BLOCK_CTX_CNTR              (2)     /* block marked as CXT */
#define BLOCK_CTX_CURRENT           (16)    /* block marked as current CXT */
#define INDEX_PAGE_MARK             (0x80000000)     /* to be defined */

#define YAFTL_READ_DISTURB_LIMIT    YAFTL_RC_THRESHOLD /* read disturb limit */

#define MIN_RESTORE_BUFFER          (0x10000)
#define MAX_RESTORE_BUFFER          (0x200000)
#define YAFTL_WEARLEVEL_PERIOD      30
#define ERASE_COUNT_GAP_LIMIT       100
#define ERASE_COUNT_GAP_UPPER_LIMIT 500
#define STATIC_WEARLEVEL_TRIGGER    1000
#define NAND_MOUNT_INDEX_SIZE       1000
#define YAFTL_WRITE_MAX_STRIPES     32
#define YAFTL_INDEX_CACHE_COUNTER_LIMIT 0xFFFF
#define PAGES_READ_SUB_COUNTER_LIMIT YAFTL_RC_MULTIPLIER

// GCZONE_DOUBLEUPTO defines the point up to which we will multiply banks to.
// It is similar to setting a minimum of 16, but works with non-pow-2 configs.
#define GCZONE_DOUBLEUPTO 16
#define YAFTL_GC_RATIO_SCALAR 4

// Maximum number of TOC pages/block to support
#define MAX_TOC_PAGES 4
#define YAFTL_BTOCCACHE_SIZE_PER 2
#define YAFTL_BTOCCACHE_SRCSIZE  2

// make the function pointers in vfl struct look like the old vfl functions
#ifndef AND_READONLY
#define VFL_Format                  (yaFTL_VFLFunctions.Format)
#define VFL_Write                   (yaFTL_VFLFunctions.WriteSinglePage)
#define VFL_Erase                   (yaFTL_VFLFunctions.Erase)
#define VFL_ChangeFTLCxtVbn         (yaFTL_VFLFunctions.ChangeFTLCxtVbn)
#define VFL_WriteMultiplePagesInVb  (yaFTL_VFLFunctions.WriteMultiplePagesInVb)
#endif // ! AND_READONLY
#define VFL_Init                    (yaFTL_VFLFunctions.Init)
#define VFL_Open                    (yaFTL_VFLFunctions.Open)
#define VFL_Close                   (yaFTL_VFLFunctions.Close)
#define VFL_ReadMultiplePagesInVb   (yaFTL_VFLFunctions.ReadMultiplePagesInVb)
#define VFL_ReadScatteredPagesInVb  (yaFTL_VFLFunctions.ReadScatteredPagesInVb)
#define VFL_Read                    (yaFTL_VFLFunctions.ReadSinglePage)
#define VFL_GetFTLCxtVbn            (yaFTL_VFLFunctions.GetFTLCxtVbn)
//#define	VFL_GetAddress				(yaFTL_VFLFunctions.GetAddress)
#define VFL_GetDeviceInfo           (yaFTL_VFLFunctions.GetDeviceInfo)
#define VFL_GetStruct               (yaFTL_VFLFunctions.GetStruct)
#define VFL_SetStruct               (yaFTL_VFLFunctions.SetStruct)

#endif   // ----- #ifndef _YAFTL_DEFINES_H__INC  -----

