#ifndef _PPNVFL_TYPES_H_
#define _PPNVFL_TYPES_H_

#include "WMRConfig.h"
#include "FPart.h"
#include "VFL.h"
#include "FIL.h"
#include "PPN_FIL.h"
#include "WMRBuf.h"
#include "PPNMiscTypes.h"
#include "WMROAM.h"

/* defines */
#define PPNVFL_ROOM_FOR_BOOT_BLOCKS (20)
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

#define PPNVFL_TYPE_VBN            (0)

#define PPNVFL_SPARE_TYPE_CXT       (AND_SPARE_TYPE_REGION_VFL | 0x00)
#define PPNVFL_SPARE_TYPE_TST       (AND_SPARE_TYPE_REGION_VFL | 0x01)

#define PPNVFL_MIN_PAGE_SIZE        (8192)  // minimum page size in bytes

#define PPN_V2P_MIXED_MARK          (0x8000)    // use bit 15 to mark a virtual block is made
                                                // of different blocks at different CAUs (not all the same offset)

#define PPNVFL_P2V_SCRUB_BIT        (1 << 0)   // use bit 0 in the p2v mapping to indicate a block should be retired

#define BOOT_BLOCK_ALLOCATION_COUNT(m)          (0 != (WMR_INIT_USE_KEEPOUT_AS_BORROW & (m)->r_gen.options) ? \
                                                 (m)->r_gen.keepout : 0)

#define SPECIAL_BLOCK_PRE_ALLOCATION_COUNT(m)   (BOOT_BLOCK_ALLOCATION_COUNT(m) + 11)
                                                // number of pre allocated special blocks
                                                // 64 BFN (worst case), 1 Signature,
                                                // 2 Unique Info, 2 Diags, 6 Spare

#define P2SB_RESERVED_VALUE         (0xFFFF)
#define P2V_RESERVED_VALUE          (0x00)

#define PPNVFL_CXT_LAST_BANKIDX     (0xFF)         // use this define for the index and bank of the last/summary index page
#define PPNVFL_CXT_IGNORE_BANKIDX   (0xF0)         // use this value to tell the read function to ignore the bank and index values
#define PPNVFL_INVALID_BANK_BLOCK   (0xFFFF)       // use to mark unasinged value or invliad assignment

/* PPNVFL internal structures */

typedef struct
{
    UInt16 block;
    UInt8 bank;
    UInt8 features; // bit fields...
} VFLCxtBlockAddress;

#define S_VFL_CXT_BLOCK_SCRUB_BIT (1 << 7)

#define _isVFLCxtBlockMarkedScrub(_blk_addr)  (_blk_addr.features & S_VFL_CXT_BLOCK_SCRUB_BIT)
#define _setVFLCxtScrubMark(_blk_addr)  { _blk_addr.features |= S_VFL_CXT_BLOCK_SCRUB_BIT; }
#define _unsetVFLCxtScrubMark(_blk_addr)  { _blk_addr.features &= (~S_VFL_CXT_BLOCK_SCRUB_BIT); }

WMR_CASSERT((sizeof(VFLCxtBlockAddress) == 4), sizeof_VFLCxtBlockAddress);

typedef UInt16 P2UsageType;

// On NAND structures - Start

// IMPORTANT: the following defines are used in on nand structures - be careful changing it
#define PPNVFL_MAX_SUPPORTED_BANKS  (128)
#define PPNVFL_SCRUB_BLOCK_COUNT    (20)

// PPNVFL_GeneralCxt - general vfl context
typedef struct
{
    UInt32 ftl_type;                        // FTL identifier
    UInt32 exported_lba_no;                 // Number of LBAs exported by a TL
    UInt32 exported_l2v_pool_size;          // l2v tree allocated size in bytes
    UInt16 num_of_ftl_sublk;                // the number of super blocks allocated for the FTL -
                                            // default id to give all to FTL
    UInt16 num_of_vfl_sublk;                // the total number of available super blocks
    UInt8 reserved[0xF0];

    // start the vfl_bloock array in offset 256
    VFLCxtBlockAddress vfl_blocks[64];            // vfl blocks physical addresses
} PPNVFL_GeneralCxt;

WMR_CASSERT((sizeof(PPNVFL_GeneralCxt) == 512), sizeof_PPNVFL_GeneralCxt);

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

WMR_CASSERT((sizeof(PPNVFL_BankCxt) == 32), sizeof_PPNVFL_BankCxt);

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
    UInt32 options;
    UInt16 vfl_blocks_redundancy;           // how many copies are used in parallel per one part of the vfl blocks pair
    UInt16 vfl_blocks_depth;                // how many blocks are used per copy
    UInt16 vfl_pages_per_set;               // how many pages we can write in a set (if depth is bigger than 1 this
                                            // number is depth * slc_page_per_block)
    UInt16 num_of_vfl_blocks;               // number of vfl blocks
    UInt16 next_cxt_block_offset;           // can be 0 or 1 two copies are written in two parallel blocks
    UInt16 next_cxt_page_offset;
    BOOL32 need_rewrite;
    UInt8 bytes_per_vbn_bitmap;
    BOOL32 search_for_cxt;

    // cached mapping
    UInt16 cached_vbn;
    UInt16 cached_vbn_good_blocks;
    UInt8 cached_banks_v2p[PPNVFL_MAX_SUPPORTED_BANKS];
    
    BOOL32 active;
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
    UInt64 ddwVbasProgrammedCnt;
    UInt64 ddwVbasReadCnt;
    UInt64 ddwVBlocksErasedCnt;
    UInt64 ddwSequentialReadCallCnt;
    UInt64 ddwScatteredReadCallCnt;
    UInt64 ddwSpansReadInitCallCnt;
    UInt64 ddwSpansReadAddCallCnt;
    UInt64 ddwSpansReadExecCallCnt;
    UInt64 ddwSequentialProgramCallCnt;
    UInt64 ddwEraseCallCnt;
    UInt64 ddwBurnInCode;
} SVFLStatistics;

#define SVFL_STATISTICS_DESCRIPTION { \
        "ddwVbasProgrammedCnt", \
        "ddwVbasReadCnt", \
        "ddwVBlocksErasedCnt", \
        "ddwSequentialReadCallCnt", \
        "ddwScatteredReadCallCnt", \
        "ddwSpansReadInitCallCnt", \
        "ddwSpansReadAddCallCnt", \
        "ddwSpansReadExecCallCnt", \
        "ddwSequentialProgramCallCnt", \
        "ddwEraseCallCnt", \
        "ddwBurnInCode" \
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
    SVFLStatistics stat;
#endif

    UInt8 *v2p_bitmap;
    UInt8 *v2p_scrub_bitmap;
    UInt16 blocks_to_scrub_count;
    UInt32 v2p_bitmap_size;

    PPNCommandStruct *commands[PPNVFL_MAX_SUPPORTED_CHANNELS];
    PPNCommandStruct *single_command;
    WMR_BufZone_t bufzone;

    UInt16 **chip_enable_table; // chip_enable_table[x][y] is the physical chip enable of
    // chip enable number y in channel x

    PPNReorderStruct *reorder;

    BOOL32 initialized;
} PPNVFL_MainCxt; // hold all other pointers

#define PPNVFL_EXPORT_META_VERSION 0x00000004
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
