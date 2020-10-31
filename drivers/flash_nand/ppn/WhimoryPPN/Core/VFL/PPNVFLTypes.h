#ifndef _PPNVFL_TYPES_H_
#define _PPNVFL_TYPES_H_

#include "WMRConfig.h"
#include "FPart.h"
#include "VFL.h"
#include "FIL.h"
#include "PPN_FIL.h"
#include "WMRBuf.h"
#include "PPNMiscTypes.h"

/* defines */
#define PPNVFL_SCRUB_BLOCK_COUNT    (20)
#define PPNVFL_ROOM_FOR_BOOT_BLOCKS (20)
#define PPNVFL_MAX_SUPPORTED_BANKS  (128)
#define PPNVFL_NUM_OF_VFL_CXT_BLOCK_SETS (2)
#define PPNVFL_MAX_INFO_BLOCK_COUNT (64)
#define PPNVFL_MAX_SUPPORTED_CHANNELS (2)

// #define AND_SB_TYPE_DRIVER_SIGNATURE (AND_SB_BIT_SPEICAL_FLAG | AND_SB_BIT_PERSISTENT_FLAG |
// andSBSetHandler(AND_SB_HANDLER_SINGLE_VERSION) | andSBSetCode(AND_SB_CODE_DRIVER_SIGNATURE))

#define PPNVFL_CODE_VFL_AVAILABLE  (0xF0)
#define PPNVFL_CODE_VFL_INFO       (0xF1)
#define PPNVFL_CODE_GROWN_BB       (0xFD)
#define PPNVFL_CODE_INITIAL_BB     (0xFE)

#define PPNVFL_TYPE_AVAILABLE_MARK (AND_SB_BIT_SPEICAL_FLAG | andSBSetHandler(AND_SB_HANDLER_VFL_CXT) | \
                                    andSBSetCode(PPNVFL_CODE_VFL_AVAILABLE))

#define PPNVFL_TYPE_VFL_INFO       (AND_SB_BIT_SPEICAL_FLAG | andSBSetHandler(AND_SB_HANDLER_VFL_CXT) | \
                                    andSBSetCode(PPNVFL_CODE_VFL_INFO))
#define PPNVFL_TYPE_GROWN_BB       (AND_SB_BIT_SPEICAL_FLAG | andSBSetHandler(AND_SB_HANDLER_VFL_CXT) | \
                                    andSBSetCode(PPNVFL_CODE_GROWN_BB))
#define PPNVFL_TYPE_INITIAL_BB     (AND_SB_BIT_SPEICAL_FLAG | andSBSetHandler(AND_SB_HANDLER_VFL_CXT) | \
                                    andSBSetCode(PPNVFL_CODE_INITIAL_BB))

#define PPNVFL_SPARE_TYPE_CXT       (AND_SPARE_TYPE_REGION_VFL | 0x00)
#define PPNVFL_SPARE_TYPE_TST       (AND_SPARE_TYPE_REGION_VFL | 0x01)

#define PPNVFL_MIN_PAGE_SIZE        (8192)  // minimum page size in bytes

#define PPN_V2P_MIXED_MARK          (0x8000)    // use the 16 bit to mark a virtual block is made
                                                // of different blocks at different CAUs (not all the same offset)
#define PPN_V2P_UNMAPPED            (0x7FFF)    // indicator that this block entry is unmapped

#define SPECIAL_BLOCK_PRE_ALLOCATION_STRIPE   (8)

#define P2SB_RESERVED_VALUE         (0xFFFF)
#define P2V_RESERVED_VALUE          (0x00)

#define PPNVFL_CXT_LAST_BANKIDX     (0xFF)         // use this define for the index and bank of the last/summary index page
#define PPNVFL_CXT_IGNORE_BANKIDX   (0xF0)         // use this value to tell the read function to ignore the bank and index values

/* PPNVFL internal structures */

typedef struct
{
    UInt16 block;
    UInt16 bank;
} BlockAddress;

typedef struct
{
    UInt16 vbn;      // what vbn this blcok belongs to
    UInt8 offset;    // what offset in the vbn this block is used for if all the banks have blocks available
                     // this number should equal to the bank number this physical blcok is coming from
    UInt8 reserved;  // should be set to 0x00 at this point
} P2VType;

typedef struct
{
    UInt16 type;      // special block type
    UInt16 reserved;  // should be set to 0xFFFF at this point
} P2SpecialBlockType;

typedef union
{
    P2VType p2v;
    P2SpecialBlockType p2sb;
} P2BlockUsageType;

typedef struct
{
    UInt16 pbn;
    UInt8 bank;
} MixedEntryType;

// On NAND structures - Start

// PPNVFL_GeneralCxt - general vfl context
typedef struct
{
    UInt32 ftl_type;                        // FTL identifier
    BlockAddress scrub_list[PPNVFL_SCRUB_BLOCK_COUNT];
    UInt16 ftl_cxt_vbn[FTL_CXT_SECTION_SIZE];   // page address (FTL virtual addressing space) of FTL Cxt
    UInt16 num_of_ftl_sublk;                // the number of super blocks allocated for the FTL -
                                            // default id to give all to FTL
    UInt16 num_of_vfl_sublk;                // the total number of available super blocks
    UInt16 scrub_idx;
    UInt8 reserved[160];

    // start the vfl_blocks array at offset 256
    BlockAddress vfl_blocks[64];            // vfl blocks physical addresses
} PPNVFL_GeneralCxt;

// vfl blocks geometry example
// in this example redundancy of 4 and depth of 2 is used.
// the vfl is always made of two sets
// numbers are [s,r,d] where s - set (out of two), r - redundancy, d - depth

/*

   set 0:
   [0,0,0][0,0,1]
   [0,1,0][0,1,1]
   [0,1,0][0,1,1]
   [0,3,0][0,3,1]

   set 1:
   [1,0,0][1,0,1]
   [1,1,0][1,1,1]
   [1,1,0][1,1,1]
   [1,3,0][1,3,1]

   [s,r,d] is located in:
     (s * vfl_blocks_redundancy * vfl_blocks_depth) +
     (d * vfl_blocks_redundancy) + r

 */

// PPNVFL_BankCxt - per bank vfl context
typedef struct
{
    UInt16 vfl_map_pages[16];
} PPNVFL_BankCxt;

// PPNVFL_OnNANDCxt - a container structure (allowing space for expension)
// for general vfl and bank specific vfl data
typedef struct
{
    // generic info
    PPNVFL_GeneralCxt gen;
    // bank specific info
    PPNVFL_BankCxt banks[PPNVFL_MAX_SUPPORTED_BANKS];
} PPNVFL_OnNANDCxt;

typedef struct
{
    UInt8 spareType;            // spare type
    UInt8 bank;                 // bank number of the p2v mapping
    UInt8 idx;                  // page number of the p2v mapping
    UInt8 reserved1[1];         // reserved
    UInt32 cxt_age;             // context age 0xFFFFFFFF --> 0x0
    UInt8 reserved2[8];         // reserved
} PPNVFL_CxtSpare;
// On NAND structures - End

typedef struct
{
    UInt32 cxt_age;                         // context age 0xFFFFFFFF --> 0x0
    UInt16 pages_per_block_mapping;         // how many pages are needed for all block mapping
    UInt16 pbns_per_cxt_mapping_page;       // how many block mappings fit in a page
    UInt16 keepout;
    UInt16 vfl_blocks_redundancy;           // how many copies are used in parallel per one part of the vfl blocks pair
    UInt16 vfl_blocks_depth;                // how many blocks are used per copy
    UInt16 vfl_pages_per_set;               // how many pages we can write in a set (if depth is bigger than 1 this
                                            // number is depth * slc_page_per_block)
    UInt16 num_of_vfl_blocks;               // number of vfl blocks
    UInt16 next_cxt_block_offset;           // can be 0 or 1 two copies are written in two parallel blocks
    UInt16 next_cxt_page_offset;
    BOOL32 need_rewrite;
    BOOL32 search_for_cxt;
} PPNVFL_GeneralRAMCxt;

typedef struct
{
    UInt16 num_special_blocks;
    UInt16 num_grown_bad;
    UInt16 num_initial_bad;
    UInt16 num_available;
    UInt16 num_cxt;
    UInt16 num_p2v;
} PPNVFL_BankRAMCxt;

#ifdef AND_COLLECT_STATISTICS
typedef struct
{
    UInt64 ddwPagesWrittenCnt;
    UInt64 ddwPagesReadCnt;
    UInt64 ddwBlocksErasedCnt;
    UInt64 ddwSingleWriteCallCnt;
    UInt64 ddwSingleReadCallCnt;
    UInt64 ddwSequentialReadCallCnt;
    UInt64 ddwScatteredReadCallCnt;
    UInt64 ddwMultipleWriteCallCnt;
    UInt64 ddwEraseCallCnt;
    UInt64 ddwEraseTimeoutFailCnt;
    UInt64 ddwBurnInCode;
} PPNVFLStatistics;

#define VFL_STATISTICS_DESCREPTION { \
        "ddwPagesWrittenCnt", \
        "ddwPagesReadCnt", \
        "ddwBlocksErasedCnt", \
        "ddwSingleWriteCallCnt", \
        "ddwSingleReadCallCnt", \
        "ddwSequetialReadCallCnt", \
        "ddwScatteredReadCallCnt", \
        "ddwMultipleWriteCallCnt", \
        "ddwEraseCallCnt", \
        "ddwEraseTimeoutFailCnt", \
        "ddwBurnInCode", \
}
#endif /* AND_COLLECT_STATISTICS */

typedef struct
{
    PPNVFL_GeneralRAMCxt r_gen;
    PPN_DeviceInfo dev;
    PPNVFL_GeneralCxt gen;
    UInt16 **vfl_map_pages;
    PPNVFL_BankRAMCxt *r_banks;
    FPartFunctions *fpart;
    LowFuncTbl *fil;
#ifdef AND_COLLECT_STATISTICS
    PPNVFLStatistics stat;
#endif
    UInt16 *v2p_main;   // main virtual to physical table
    // use one entry to vbn assuming no replacement block or mark
    // an entry in the v2p_mixed
    MixedEntryType *v2p_mixed;
    UInt16 mixed_entries;

    PPNCommandStruct *commands[PPNVFL_MAX_SUPPORTED_CHANNELS];
    PPNCommandStruct *single_command;
    WMR_BufZone_t bufzone;

    PPNReorderStruct *reorder;

    UInt16 **chip_enable_table; // chip_enable_table[x][y] is the physical chip enable of
    // chip enable number y in channel x

    BOOL32 initialized;
    UInt32 format_options;
} PPNVFL_MainCxt; // hold all other pointers

#define PPNVFL_EXPORT_META_VERSION 0x00000001
#define PPNVFL_MAX_SUPPORTED_BANKS_EXPORT (128) // intentionally seperating the export
                                               // define from other defines that are used
                                               // for on-NAND structures
#if (PPNVFL_MAX_SUPPORTED_BANKS_EXPORT < PPNVFL_MAX_SUPPORTED_BANKS)
#error PPNVFL_MAX_SUPPORTED_BANKS_EXPORT is less than PPNVFL_MAX_SUPPORTED_BANKS
#endif // PPNVFL_MAX_SUPPORTED_BANKS_EXPORT < PPNVFL_MAX_SUPPORTED_BANKS

typedef struct
{
    UInt32 version; // make sure this value chanegs if new fields are added or the structure changes
    struct
    {
        UInt32 num_of_channels;
        UInt32 ces_per_channel;
        UInt32 caus_per_ce;
    } dev_info;
    struct
    {
        struct
        {
            UInt32 chip_enable;
            UInt16 chip_enable_idx;
            UInt16 channel;
            UInt16 cau;
        } geometry;
        UInt16 num_special_blocks;
        UInt16 num_grown_bad;
        UInt16 num_initial_bad;
        UInt16 num_available;
        UInt16 num_cxt;
        UInt16 num_p2v;
    } per_bank_data[PPNVFL_MAX_SUPPORTED_BANKS_EXPORT];
} PPNVFLMetaDataExport;

#endif /* _PPNVFL_TYPES_H_ */
