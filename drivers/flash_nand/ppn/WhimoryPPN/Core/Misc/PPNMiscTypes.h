#ifndef _PPN_MISC_TYPES_H_
#define _PPN_MISC_TYPES_H_

#include "WMRConfig.h"
#include "PPN_FIL.h"
#include "FIL.h"

#define PPN_SPECIAL_BLOCK_LIMIT(dev) ((dev)->blocks_per_cau - (((dev)->blocks_per_cau * 5) / 100))

/* PPNVFL internal structures */

typedef struct
{
    UInt32 num_channels;
    UInt32 ces_per_channel;
    UInt32 caus_per_ce;
    UInt32 num_of_banks;
    UInt32 blocks_per_cau;
    UInt32 pages_per_block_slc;
    UInt32 pages_per_block_mlc;
    UInt32 bits_per_cau_addr;
    UInt32 bits_per_page_addr;
    UInt32 bits_per_block_addr;
    UInt32 main_bytes_per_page;
    UInt32 main_bytes_per_lba;
    UInt32 pages_per_sublk;
    UInt32 lbas_per_page;
    UInt32 lba_meta_bytes_valid;
    UInt32 lba_meta_bytes_buffer;
    UInt32 max_pages_in_prep_per_ce;
    BOOL32 f_match_oddeven_blocks;
    UInt16 chip_enable_table[4][4]; // up to 4 channels and up to 4 chip enables per channel
    LowFuncTbl *fil;
} PPN_DeviceInfo;

typedef struct
{
    PPNCommandEntry entry[PPN_MAX_CES_PER_BUS][PPN_MAX_PAGES_PER_CE];
    PPNMemoryIndex mem_index[PPN_MAX_CES_PER_BUS][PPN_MAX_PAGES_PER_CE];
} PPNReorderStruct;

UInt16 ppnMiscGetCAUFromBank
    (PPN_DeviceInfo * dev,
    UInt16 bank);
UInt16 ppnMiscGetChannelFromBank
    (PPN_DeviceInfo * dev,
    UInt16 bank);
UInt16 ppnMiscGetCEIdxFromBank
    (PPN_DeviceInfo * dev,
    UInt16 bank);
UInt16 ppnMiscGetCEFromBank
    (PPN_DeviceInfo * dev,
    UInt16 bank);
UInt16 ppnMiscGetCEIdxFromChipEnable
    (PPN_DeviceInfo * dev,
    UInt16 channel,
    UInt16 chip_enable);
UInt32 ppnMiscConvertToPPNPageAddress
    (PPN_DeviceInfo * dev,
    UInt16 cau,
    UInt16 block,
    UInt16 page,
    BOOL32 slcAddress);
void ppnMiscConvertPhysicalAddressToBankBlockPage
    (PPN_DeviceInfo * dev,
    UInt16 channel,
    UInt16 chip_enable_idx,
    UInt32 page,
    UInt16 *bank,
    UInt16 *block,
    UInt16 *page_offset,
    BOOL32 *slc);
void ppnMiscConvertPhysicalAddressToCauBlockPage
    (PPN_DeviceInfo * dev,
    UInt32 page,
    UInt16 *cau,
    UInt16 *block,
    UInt16 *page_offset);
PPNStatusType ppnMiscSingleOperation
    (PPN_DeviceInfo * dev,
    PPNCommandStruct * command,
    PPNCommand command_type,
    PPNOptions options,
    UInt16 bank,
    UInt16 block,
    UInt16 page_offset,
    BOOL32 slc,
    void *data,
    void *meta);
void ppnMiscFillDevStruct
    (PPN_DeviceInfo * dev,
    LowFuncTbl *fil);
BOOL32 ppnMiscIsBlockGood
    (UInt8 *bbt,
    UInt16 block);
BOOL32
ppnMiscAddPhysicalAddressToCommandStructure
    (PPN_DeviceInfo * dev,
    PPNCommandStruct **commands,
    UInt16 channel,
    UInt16 chip_enable_idx,
    UInt16 cau,
    UInt16 block,
    UInt16 page_offset,
    BOOL32 slc,
    UInt8 pba_offset,
    UInt8 pba_count,
    UInt8 mem_index,
    UInt16 mem_offset);
void
ppnMiscInitCommandStructure
    (PPN_DeviceInfo * dev,
    PPNCommandStruct **commands,
    UInt16 num_of_command_structs,
    PPNCommand command,
    PPNOptions options);
BOOL32
ppnMiscAddMemoryToCommandStructure
    (PPN_DeviceInfo *dev,
    PPNCommandStruct **commands,
    UInt16 num_of_command_structs,
    void *data,
    void *meta,
    UInt8 num_of_lbas);

#ifndef AND_READONLY
BOOL32
ppnMiscTestSpecialBlock
    (PPN_DeviceInfo * dev,
    PPNCommandStruct * command,
    UInt16 bank,
    UInt16 block,
    UInt8 spareType);
#endif // AND_READONLY

void
ppnMiscReorderCommandStruct
    (PPN_DeviceInfo * dev,
    PPNCommandStruct **commands,
    UInt16 num_of_command_structs,
    PPNReorderStruct *reorder);

PPNStatusType
ppnMiscSingleLbaRead
    (PPN_DeviceInfo * dev,
    PPNCommandStruct * command,
    PPNCommand command_type,
    PPNOptions options,
    UInt16 ce,
    UInt16 cau,
    UInt16 block,
    UInt16 page_offset,
    UInt8 lba_offset,
    BOOL32 slc,
    void *data,
    void *meta);

#endif /* _PPN_MISC_TYPES_H_ */
