#ifndef _PPNFPART_TYPES_H_
#define _PPNFPART_TYPES_H_

#include "WMRConfig.h"
#include "PPNMiscTypes.h"
#include "WMRBufTypes.h"

/* defines */

#define PPNFPART_MAX_SPECIAL_BLOCKS_PER_TYPE (8)
#define PPNFPART_MAX_SPECIAL_BLOCKS_TYPE (20)

#define PPNFPART_SPECIAL_BLOCK_SPARE_TYPE (AND_SPARE_TYPE_REGION_FPART + 0x00)

// On NAND structures - Start

#define PPNFPART_STRUCTURE_VERSION (0x00000001) // use the two MSB as major - not forward compatible

// SpecialBlockHeaderStruct - used as a header in special blocks that are managed
// by PPNFPart
typedef struct
{
    struct
    {
        UInt16 bank;
        UInt16 block;
    } block_addresses[PPNFPART_MAX_SPECIAL_BLOCKS_PER_TYPE];
    UInt32 version;
    UInt32 data_size;
    UInt32 copy_number;
    UInt32 reserved[21]; // reserving bits and rounding the structure size to 128 bytes
} SpecialBlockHeaderStruct;

// PPNFPart_Spare - used in the metadata of any speical blcok
// managed by PPNFPart
typedef struct
{
    UInt8 spare_type;            // spare type
    UInt8 index;                 // for spcial blocks holding information that cross page boundary
    UInt16 vfl_special_type;     // shared type (between VFL and FPART) identifying this type of special block
    UInt8 reserved1[12];
} PPNFPart_Spare;

// On NAND structures - End

typedef struct
{
    UInt16 bank;
    UInt16 block;
    UInt16 type;
} SpecialBlockCacheEntryType;

/* PPNFPart internal structures */
typedef struct
{
    PPN_DeviceInfo dev;
    LowFuncTbl *fil;
    SpecialBlockCacheEntryType * special_blocks; // malloc to size AND_MAX_SPECIAL_BLOCKS in init
    UInt16 special_blocks_idx; // pointer to the next available entry in the special_blocks array
    BOOL32 done_mapping; // flag indicating the cache is up to date - no need to search in nand

    // reserve a single command struct for fpart to
    // do physical operations
    WMR_BufZone_t zone;
    PPNCommandStruct * command;
    BOOL32 initialized;
} PPNFPart_MainCxt; // hold all other pointers

#endif /* _PPNFPART_TYPES_H_ */
