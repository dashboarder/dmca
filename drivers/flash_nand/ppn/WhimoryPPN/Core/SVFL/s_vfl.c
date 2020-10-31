
#define AND_TRACE_LAYER VFL

#include "WMRConfig.h"
#include "ANDTypes.h"
#include "WMROAM.h"
#include "VFLBuffer.h"
#include "PPNMiscTypes.h"
#include "s_vfl_types.h"
#include "VFL.h"

#define kPPNVFLMinorVersion_ver1 (1) // first version
#define kPPNVFLMinorVersion_ver2 (2) // fixed rdar://9098529

#define kPPNVFLMinorVersion kPPNVFLMinorVersion_ver2

// _readPageOfASet - defines
#define RPS_STATUS_SPARE_TYPE_MISSMATCH    (1 << 1)
#define RPS_STATUS_BANK_MISSMATCH          (1 << 2)
#define RPS_STATUS_IDX_MISSMATCH           (1 << 3)

/* ----------- I/F commands - start ------------- */

static Int32  _ppnvflOpen
    (UInt32 dwKeepout,
    UInt32 minor_ver,
    UInt32 dwOptions);

/* ----------- I/F commands - end ------------- */

PPNVFL_MainCxt mcxt;
static VFLFailureDetails gstLastFailure;

#if WMR_EFI
ANDNandLayoutStruct p_layout;
UInt32 p_layout_size = sizeof(ANDNandLayoutStruct);
UInt32 stats_copy_from_meta = 0;
UInt32 stats_copy_from_data = 0;
UInt32 stats_offset_from_meta = 0;
UInt32 stats_offset_from_data = 0;
#endif 

// helper functions - local
#if !AND_READONLY
static BOOL32 _programCxtLastPage
    (Buffer * buf,
    BOOL32 retry);
static BOOL32 _copyVFLCxtToOtherPair
    (void);
#endif // !AND_READONLY
static BOOL32 _readVFLCxtPage
    (Buffer *buf,
    UInt16 bank,
    UInt16 vfl_idx);
static void
_copyFromOnNandCxtToRam
    (UInt8 *data);
#if !AND_READONLY
static void
_copyFromRamToOnNandCxt
    (UInt8 *data);
static BOOL32
_checkForBankMapPages
    (UInt16 vfl_set);
#endif // !AND_READONLY

typedef enum
{
    VFLCxtProgramFail = 0,
    VFLCxtProgramDone = 2,
    VFLCxtProgramRetry = 3,
} ProgramVFLCxtPageWrapStatus;

#if !AND_READONLY
static ProgramVFLCxtPageWrapStatus _programVFLCxtPageWrap
    (Buffer *buf,
    UInt16 bank,
    UInt16 vfl_idx);
static BOOL32 _programVFLCxtPage
    (Buffer *buf,
    UInt16 bank,
    UInt16 vfl_idx);
#endif // !AND_READONLY
static void
_convertVFLPageIdxToPhysicalPageBlockOffsets
    (UInt16 set,
    UInt16 vfl_page_idx,
    UInt16 *vfl_block_offset,
    UInt16 *vfl_page_offset);
#ifndef AND_READONLY
static BOOL32
_markBlockForRetirement
    (UInt16 bank,
    UInt16 block);
#endif // !AND_READONLY

#if !AND_READONLY

static void
_setVFLCxtSpare
    (UInt8 *meta,
    UInt8 bank,
    UInt8 vfl_idx)
{
    PPNVFL_CxtSpare spare;
    UInt16 lba_idx;

    WMR_MEMSET(&spare, 0x00, sizeof(PPNVFL_CxtSpare));
    spare.cxt_age = mcxt.r_gen.cxt_age;
    spare.bank = bank;
    spare.spareType = PPNVFL_SPARE_TYPE_CXT;
    spare.idx = vfl_idx;
    for (lba_idx = 0; lba_idx < mcxt.dev.lbas_per_page; lba_idx++)
    {
        WMR_MEMCPY(meta, &spare, sizeof(PPNVFL_CxtSpare));
    }
}

/*
 * Name: _ReportFailure
 * Input: mode, FailingCE, PhysicalPageAddress
 * Output: none
 * Return value: none
 */
static void
_ReportFailure
    (VFLFailureMode mode,
    UInt16 wFailingCE,
    UInt32 dwPhysicalPage)
{
    gstLastFailure.mode = mode;
    gstLastFailure.wCE[0] = wFailingCE;
    gstLastFailure.dwPhysicalPage = dwPhysicalPage;
}

#endif // !AND_READONLY

/* address translating functions - start */

static UInt8
_countVbnPhysicalBlocks
    (UInt16 vbn)
{
    UInt16 byte_idx, count;
    UInt32 bitmap_start_offset;

    if (vbn == mcxt.r_gen.cached_vbn)
    {
        return mcxt.r_gen.cached_vbn_good_blocks;
    }

    bitmap_start_offset = mcxt.r_gen.bytes_per_vbn_bitmap * vbn;

    for (byte_idx = 0, count = 0; byte_idx < mcxt.r_gen.bytes_per_vbn_bitmap; byte_idx++)
    {
        UInt8 bit_idx;
        for (bit_idx = 0; bit_idx < 8; bit_idx++)
        {
            if (mcxt.v2p_bitmap[bitmap_start_offset + byte_idx] & (1 << bit_idx))
            {
                mcxt.r_gen.cached_banks_v2p[count] = (byte_idx << 3) + bit_idx;
                count++;
            }
        }
    }

    mcxt.r_gen.cached_vbn = vbn;
    mcxt.r_gen.cached_vbn_good_blocks = count;
    return count;
}

static UInt8
_getVbnPhysicalBank
    (UInt16 vbn,
    UInt16 good_bank_offset)
{
    _countVbnPhysicalBlocks(vbn);
    WMR_ASSERT(vbn == mcxt.r_gen.cached_vbn);
    return mcxt.r_gen.cached_banks_v2p[good_bank_offset];
}

static UInt8
_getVbnGoodBankOffset
    (UInt16 vbn,
    UInt16 physical_bank)
{
    UInt16 byte_idx, good_bank_offset = 0;
    const UInt32 bitmap_start_offset = mcxt.r_gen.bytes_per_vbn_bitmap * vbn;
    BOOL32 done = FALSE32;

    for (byte_idx = 0;
         (byte_idx < mcxt.r_gen.bytes_per_vbn_bitmap) && (!done); byte_idx++)
    {
        UInt8 bit_idx;
        for (bit_idx = 0; (bit_idx < 8) && (!done); bit_idx++)
        {
            if (mcxt.v2p_bitmap[bitmap_start_offset + byte_idx] & (1 << bit_idx))
            {
                if (physical_bank == ((byte_idx << 3) + bit_idx))
                {
                    done = TRUE32;
                    break;
                }
                else
                {
                    good_bank_offset++;
                }
            }
        }
    }
    return good_bank_offset;
}

static void
_convertReadAddressToVba
    (UInt16 channel,
    UInt16 chip_enable_idx,
    const RowColLenAddressType *read_addr,
    UInt32 *vba,
    UInt16 *bank,
    UInt16 *block,
    UInt16 *page_offset,
    BOOL32 *slc)
{
    UInt8 good_bank_offset;

    ppnMiscConvertPhysicalAddressToBankBlockPage(&mcxt.dev, channel, chip_enable_idx,
                                                 read_addr->row, bank, block, page_offset, slc);
    good_bank_offset = _getVbnGoodBankOffset(*block, *bank);
    *vba = ((*block * mcxt.dev.pages_per_sublk) +
            (*page_offset * _countVbnPhysicalBlocks(*block)) + good_bank_offset);
    *vba *= mcxt.dev.lbas_per_page;
    *vba += (read_addr->column / (mcxt.dev.main_bytes_per_lba + mcxt.dev.lba_meta_bytes_buffer));
}

static void
_convertBankBlockToByteAndBitIdx
    (UInt16 bank,
    UInt16 pbn,
    UInt32 *byte_idx,
    UInt16 *bit_idx)
{
    *byte_idx = ((mcxt.r_gen.bytes_per_vbn_bitmap * pbn) +
                 ((bank >> 3) % mcxt.r_gen.bytes_per_vbn_bitmap));
    *bit_idx = bank % 8;
}

static void
_setBlockAsVbnInBitmap
    (UInt16 bank,
    UInt16 pbn)
{
    UInt32 byte_idx;
    UInt16 bit_idx;

    _convertBankBlockToByteAndBitIdx(bank, pbn, &byte_idx, &bit_idx);
    mcxt.v2p_bitmap[byte_idx] |= (1 << bit_idx);
}

static void
_markBlockToScrubInBitmap
    (UInt16 bank,
    UInt16 pbn)
{
    UInt32 byte_idx;
    UInt16 bit_idx;

    _convertBankBlockToByteAndBitIdx(bank, pbn, &byte_idx, &bit_idx);
    if (!(mcxt.v2p_scrub_bitmap[byte_idx] & (1 << bit_idx)))
    {
        mcxt.blocks_to_scrub_count++;
        mcxt.v2p_scrub_bitmap[byte_idx] |= (1 << bit_idx);
    }
}

#if !AND_READONLY

static void
_setBlockAsBadInBitmap
    (UInt16 bank,
    UInt16 pbn)
{
    UInt32 byte_idx;
    UInt16 bit_idx;

    _convertBankBlockToByteAndBitIdx(bank, pbn, &byte_idx, &bit_idx);
    mcxt.v2p_bitmap[byte_idx] &= (~(1 << bit_idx));
}

static void
_unmarkBlockToScrubInBitmap
    (UInt16 bank,
    UInt16 pbn)
{
    UInt32 byte_idx;
    UInt16 bit_idx;

    _convertBankBlockToByteAndBitIdx(bank, pbn, &byte_idx, &bit_idx);
    if (mcxt.v2p_scrub_bitmap[byte_idx] & (1 << bit_idx))
    {
        mcxt.blocks_to_scrub_count--;
        mcxt.v2p_scrub_bitmap[byte_idx] &= (~(1 << bit_idx));
    }
}

static BOOL32
_isBlockToScrubInBitmap
    (UInt16 bank,
    UInt16 pbn)
{
    UInt32 byte_idx;
    UInt16 bit_idx;

    _convertBankBlockToByteAndBitIdx(bank, pbn, &byte_idx, &bit_idx);

    return (mcxt.v2p_scrub_bitmap[byte_idx] & (1 << bit_idx)) ? TRUE32 : FALSE32;
}

#endif // !AND_READONLY

/*
 * Name: _convertVpnToBankBlockPage
 * Input: vpn
 * Output: bank, block, page offset
 * Return value: none
 */
static void
_convertVpnToBankBlockPage
    (UInt32 vpn,
    UInt16 *bank,
    UInt16 *block,
    UInt16 *page_offset)
{
    const UInt16 vbn = vpn / mcxt.dev.pages_per_sublk;
    const UInt16 physical_blocks_in_vbn = _countVbnPhysicalBlocks(vbn);
    const UInt16 page_offset_gen = vpn % mcxt.dev.pages_per_sublk;
    const UInt16 good_block_offset = page_offset_gen % physical_blocks_in_vbn;
    const UInt16 physical_bank_offset = _getVbnPhysicalBank(vbn, good_block_offset);
    const UInt16 page_offset_var = page_offset_gen / physical_blocks_in_vbn;

    WMR_ASSERT(physical_blocks_in_vbn <= mcxt.dev.num_of_banks);
    WMR_ASSERT(physical_bank_offset < mcxt.dev.num_of_banks);
    WMR_ASSERT(vbn < mcxt.dev.blocks_per_cau);
    WMR_ASSERT(page_offset_var < mcxt.dev.pages_per_block_mlc);

    if (bank != NULL)
    {
        *bank = physical_bank_offset;
    }
    if (block != NULL)
    {
        *block = vbn;
    }
    if (page_offset != NULL)
    {
        *page_offset = page_offset_var;
    }
}
/* address translating functions - end */

/*
 * Name: _printV2PMapping
 * Description: if WMR_PRINT_VFL_V2PMAPPING is enabled and VFL_INF_PRINT
 *              is working this function print the virtual to phsical map.
 * Return value: none
 */

// to enable this print you should replace 0 with a print type that is 
// enabled in WMR_PRINT_LEVEL
#define WMR_PRINT_VFLINF_CXT_BLOCK_MAPPING (0)

static void
_printVFLBlockMapping
    (UInt32 line)
{
#if (WMR_PRINT_VFLINF_CXT_BLOCK_MAPPING)
    UInt16 i, bank;

    WMR_PRINT(VFLINF_CXT_BLOCK_MAPPING,
                  "printing vfl block list cxt_age:0x%X (caller line:%d)\n", mcxt.r_gen.cxt_age, line);
    for (i = 0; i < mcxt.r_gen.num_of_vfl_blocks; i++)
    {
        WMR_PRINT(VFLINF_CXT_BLOCK_MAPPING,
                      "(0x%02X, 0x%04X, 0x%02X)\n", mcxt.gen.vfl_blocks[i].bank, mcxt.gen.vfl_blocks[i].block, mcxt.gen.vfl_blocks[i].features);
    }
    WMR_PRINT(VFLINF_CXT_BLOCK_MAPPING,
                  "printing mapping pages (caller line:%d)\n", line);
    for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
    {
        for (i = 0; i < mcxt.r_gen.pages_per_block_mapping; i++)
        {
            WMR_PRINT(VFLINF_CXT_BLOCK_MAPPING, "bank 0x%02X: 0x%04X \n", bank, mcxt.vfl_map_pages[bank][i]);
        }
    }
#endif // WMR_PRINT_VFLINF_CXT_BLOCK_MAPPING
}

/*
 * Name: _calcNumOfVFLSuBlk
 * Return value: number of VFL virtual blocks for this media
 */
static UInt32
_calcNumOfVFLSuBlk
    (void)
{
    return mcxt.dev.blocks_per_cau;
}

/* ppn commands functions - start */
/*
 * Name: _addAddressToCommandStructure
 * Output: pointer to commands structure
 * Input: vfl address to add, index in the global operation (coming from the FTL).
 * Return value: TRUE32 is successful, otehrwise - FALSE32
 */
static BOOL32
_addAddressToCommandStructure
    (PPNCommandStruct **commands,
    UInt32 vfl_address,
    UInt8 vba_offset,
    UInt8 vba_count,
    UInt8 mem_index,
    UInt16 mem_offset)
{
    UInt16 bank = 0, block = 0, cau = 0, chip_enable_idx = 0, channel = 0,
           page_offset = 0;

    // convert address from vfl to physical
    _convertVpnToBankBlockPage(vfl_address, &bank, &block, &page_offset);
    channel = ppnMiscGetChannelFromBank(&mcxt.dev, bank);
    chip_enable_idx = ppnMiscGetCEIdxFromBank(&mcxt.dev, bank);
    cau = ppnMiscGetCAUFromBank(&mcxt.dev, bank);
    return ppnMiscAddPhysicalAddressToCommandStructure(&mcxt.dev, mcxt.commands, channel, chip_enable_idx,
                                                       cau, block, page_offset, FALSE32, vba_offset, vba_count,
                                                       mem_index, mem_offset);
}

typedef enum
{
    InitOnly = 1UL << 0,
    FillOnly = 1UL << 1,
    PackOnly = 1UL << 2,
    InitAndFill = (InitOnly | FillOnly),
    InitFillPack = (InitOnly | FillOnly | PackOnly),
} FillSequentialAddressesInitType;

/*
 * Name: _fillSequentialAddressesToCommandStructure
 * Description: this function takes a start address and number of pages and
 *              generates commands structure to send to the FIL.
 * Output: pointer to commands structure
 * Input: start vfl address, number of addresses, command type,
 *        data pointer, meta pointer
 * Return value: TRUE32 is successful, otehrwise - FALSE32
 */
static BOOL32
_fillSequentialAddressesToCommandStructure
    (PPNCommandStruct **commands,
    UInt32 vba,
    UInt16 *num_vbas,
    PPNCommand command,
    void *data,
    void *meta,
    PPNOptions options,
    FillSequentialAddressesInitType init)
{
    UInt16 vba_idx;

    if (init & InitOnly)
    {
        ppnMiscInitCommandStructure(&mcxt.dev, mcxt.commands, mcxt.dev.num_channels, command, options);
    }

    if (init & FillOnly)
    {
        for (vba_idx = 0; (vba_idx < *num_vbas) && (init & FillOnly); )
        {
            const UInt32 curr_vba = vba + vba_idx;
            const UInt32 vpn = curr_vba / mcxt.dev.lbas_per_page;
            const UInt16 vba_offset = curr_vba % mcxt.dev.lbas_per_page;
            const UInt16 vba_count =
                WMR_MIN((UInt32)(*num_vbas - vba_idx), (UInt32)(mcxt.dev.lbas_per_page - vba_offset));

            if (_addAddressToCommandStructure(mcxt.commands, vpn, vba_offset, vba_count,
                                              mcxt.commands[0]->mem_buffers_count, vba_idx) == FALSE32)
            {
                *num_vbas = vba_idx;
                break;
            }

            vba_idx += vba_count;
        }
        ppnMiscAddMemoryToCommandStructure(&mcxt.dev, mcxt.commands, mcxt.dev.num_channels, data, meta, vba_idx);
    }

    if (init & PackOnly)
    {
        ppnMiscReorderCommandStruct(&mcxt.dev, mcxt.commands, mcxt.dev.num_channels, mcxt.reorder);
    }

    return TRUE32;
}

/* ppn commands functions - end */

/* block replacement functions - start */

#ifndef AND_READONLY

static BOOL32
_setBlockUsageInVFLCxt
    (UInt16 bank,
    UInt16 block,
    P2UsageType new_usage)
{
    P2UsageType *p2_block_usage_map;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
    const UInt16 vflcxt_in_page_offset = block % mcxt.r_gen.pbns_per_cxt_mapping_page;
    const UInt16 vflcxt_page_index = block / mcxt.r_gen.pbns_per_cxt_mapping_page;
    BOOL32 retry_program_cxt = FALSE32;

    p2_block_usage_map = (P2UsageType*)buf->pData;

    do
    {
        ProgramVFLCxtPageWrapStatus program_status;
        P2UsageType *on_vflcxt_block_usage = &p2_block_usage_map[vflcxt_in_page_offset];
        retry_program_cxt = FALSE32;
        // mark the block we are replacing as bad
        if (_readVFLCxtPage(buf, bank, vflcxt_page_index) == FALSE32)
        {
            WMR_PRINT(ERROR, "error reading vfl cxt (bank:0x%X, index:0x%X, block:0x%X)\n",
                           bank, vflcxt_page_index, block);
            goto return_error;
        }

        *on_vflcxt_block_usage = new_usage;

        program_status = _programVFLCxtPageWrap(buf, bank, vflcxt_page_index);
        if (program_status == VFLCxtProgramFail)
        {
            WMR_PRINT(ERROR, "error programming vfl cxt (bank:0x%X, index:0x%X)\n",
                           bank, vflcxt_page_index);
            goto return_error;
        }
        else if (program_status == VFLCxtProgramRetry)
        {
            retry_program_cxt = TRUE32;
        }
        else
        {
            retry_program_cxt = FALSE32;
        }
    }
    while (retry_program_cxt);

    BUF_Release(buf);
    return TRUE32;

 return_error:
    BUF_Release(buf);
    return FALSE32;
}

static BOOL32
_ppnvflAllocateSpecialBlock
    (SpecialBlockAddress *chosen_block,
    UInt16 type)
{
    UInt16 bank, i;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
    P2UsageType *p2_block_usage_map = (P2UsageType*)buf->pData;
    BOOL32 retry_allocating = FALSE32;
    UInt8 failed_banks_marks[PPNVFL_MAX_SUPPORTED_BANKS];

    WMR_MEMSET(failed_banks_marks, 0, sizeof(failed_banks_marks));
    
    do
    {
        UInt16 vfl_idx, min_special_blocks = 0xFFFF;;
        UInt16 total_special_blocks = 0;
        chosen_block->bank = PPNVFL_INVALID_BANK_BLOCK;
        chosen_block->block = PPNVFL_INVALID_BANK_BLOCK;
        retry_allocating = FALSE32;
        
        // choose a bank
        for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
        {
            const UInt16 specials_in_bank = mcxt.r_banks[bank].num_special_blocks;

            total_special_blocks += specials_in_bank;

            if ((specials_in_bank < min_special_blocks) &&
                (mcxt.r_banks[bank].num_available != 0) &&
                (!(failed_banks_marks[bank])))
            {
                chosen_block->bank = bank;
                min_special_blocks = specials_in_bank;
            }
        }
        
        if (chosen_block->bank == PPNVFL_INVALID_BANK_BLOCK)
        {
#if AND_SIMULATOR
            WMR_SIM_EXIT("Out of available blocks");
#endif // AND_SIMULATOR
            WMR_PRINT(ERROR, "failed finding a bank to allocate special block\n");
            goto return_error;
        }
        else if (AND_MAX_SPECIAL_BLOCKS < total_special_blocks)
        {
            WMR_PRINT(ERROR, "total special blocks has exceeded the max allowed\n");
            goto return_error;
        }
        
        bank = chosen_block->bank;

        // chose a block
        for (vfl_idx = mcxt.r_gen.pages_per_block_mapping;
             (vfl_idx > 0) && (!retry_allocating) && (chosen_block->block == PPNVFL_INVALID_BANK_BLOCK); )
        {
            UInt16 cxt_blocks_offset;
            UInt16 blocks_in_cxt_page;
            vfl_idx--;
            cxt_blocks_offset = vfl_idx * mcxt.r_gen.pbns_per_cxt_mapping_page;
            blocks_in_cxt_page = WMR_MIN(mcxt.r_gen.pbns_per_cxt_mapping_page,
                                         (mcxt.dev.blocks_per_cau - cxt_blocks_offset));
            if (_readVFLCxtPage(buf, bank,
                                vfl_idx) == FALSE32)
            {
                WMR_PRINT(ERROR, "failed _readVFLCxtPage!\n");
                goto return_error;
            }
            for (i = blocks_in_cxt_page; ((i > 0) && (i + cxt_blocks_offset > PPN_SPECIAL_BLOCK_LIMIT(&mcxt.dev)) && 
                                          (!retry_allocating) && (chosen_block->block == PPNVFL_INVALID_BANK_BLOCK)); )
            {
                i--;
                if (p2_block_usage_map[i] == PPNVFL_TYPE_AVAILABLE_MARK)
                {
                    ProgramVFLCxtPageWrapStatus program_status;
                    UInt16 potential_block = i + cxt_blocks_offset;
                    if (!ppnMiscTestSpecialBlock(&mcxt.dev, mcxt.commands[0], bank, potential_block, PPNVFL_SPARE_TYPE_TST))
                    {
                        WMR_PRINT(VFLWARN, "failed testing block (bank:0x%X, block:0x%X) for sb allocation\n",
                                       bank, potential_block);

                        p2_block_usage_map[i] = PPNVFL_TYPE_GROWN_BB;
                    }
                    else
                    {
                        chosen_block->block = potential_block;
                        p2_block_usage_map[i] = type;
                    }
                    // program the update back to the NAND
                    program_status = _programVFLCxtPageWrap(buf, bank, vfl_idx);
                    if (program_status == VFLCxtProgramFail)
                    {
                        WMR_PRINT(ERROR, "error programming vfl cxt (bank:0x%X, i:0x%X)\n", bank, i);
                        goto return_error;
                    }
                    else if (program_status == VFLCxtProgramRetry)
                    {
                        retry_allocating = TRUE32;
                    }
                    else
                    {
                        // mark the change in the VFL RAM structures
                        mcxt.r_banks[bank].num_available--;
                        if (p2_block_usage_map[i] == PPNVFL_TYPE_GROWN_BB)
                        {
                            mcxt.r_banks[bank].num_grown_bad++;
                            retry_allocating = TRUE32;
                        }
                        else
                        {
                            mcxt.r_banks[bank].num_special_blocks++;
                        }
                    }
                }
            }
        }
        if ((!retry_allocating) && (chosen_block->block == PPNVFL_INVALID_BANK_BLOCK))
        {
            failed_banks_marks[bank] = 1;
            retry_allocating = TRUE32;
        }
    }
    while (retry_allocating);

    if (chosen_block->block == PPNVFL_INVALID_BANK_BLOCK)
    {
        WMR_PRINT(ERROR, "failed to find a new special block (bank:0x%X)\n", bank);
        goto return_error;
    }

    if (_programCxtLastPage(buf, TRUE32) == FALSE32)
    {
        WMR_PRINT(ERROR, "error programming last vfl cxt (bank:0x%X)\n", bank);
        goto return_error;
    }

    if (!mcxt.fpart->AllocateSpecialBlockType(chosen_block, 1, type))
    {
        WMR_PRINT(ERROR, "fpart failed to allocate special block with type %p\n", type);
        goto return_error;
    }
    BUF_Release(buf);
    return TRUE32;

 return_error:
    BUF_Release(buf);
    return FALSE32;
}

static BOOL32
_removeBadBlock
    (UInt16 bank,
    UInt16 block)
{
    Buffer *buf = NULL;

    if (!_setBlockUsageInVFLCxt(bank, block, PPNVFL_TYPE_GROWN_BB))
    {
        WMR_PRINT(ERROR, "failed to mark block as bad (bank:0x%X, block:0x%X)!\n", bank, block);
        goto return_error;
    }
    buf = BUF_Get(BUF_MAIN_AND_SPARE);

    // remove the block from the scrub bitmap if it is marked
    _unmarkBlockToScrubInBitmap(bank, block);

    if (_programCxtLastPage(buf, TRUE32) == FALSE32)
    {
        WMR_PRINT(ERROR, "error programming last vfl cxt (bank:0x%X, index:0x%X)\n", bank, block);
        goto return_error;
    }

    WMR_PRINT(VFLWARN, "done removing (bank:0x%X, block:0x%X)!\n", bank, block);

    BUF_Release(buf);
    _setBlockAsBadInBitmap(bank, block);
    mcxt.r_banks[bank].num_grown_bad++;
    mcxt.r_banks[bank].num_p2v--;
    return TRUE32;

 return_error:
    if (buf != NULL)
    {
        BUF_Release(buf);
    }
    WMR_PANIC("_replaceBadBlock() fail bank:0x%04X block:0x%04X\n", bank, block);
    return FALSE32;
}

static BOOL32 
_chooseReplacementBank
    (UInt16 *new_bank)
{
    UInt16 bank, max_available, min_cxt_blocks;
    
    *new_bank = PPNVFL_INVALID_BANK_BLOCK;
    
    // chose a bank to work with
    // pick the bank with maximum available blocks and minimum cxt blocks
    max_available = 0;
    min_cxt_blocks = 0xFFFF;
    for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
    {
        if (mcxt.r_banks[bank].num_available)
        {
            if (*new_bank == PPNVFL_INVALID_BANK_BLOCK)
            {
                *new_bank = bank;
                min_cxt_blocks = mcxt.r_banks[bank].num_cxt;
                max_available = mcxt.r_banks[bank].num_available;
            }
            else if (mcxt.r_banks[bank].num_cxt < min_cxt_blocks)
            {
                min_cxt_blocks = mcxt.r_banks[bank].num_cxt;
                max_available = mcxt.r_banks[bank].num_available;
                *new_bank = bank;
            }
            else if (mcxt.r_banks[bank].num_cxt == min_cxt_blocks)
            {
                if (mcxt.r_banks[bank].num_available > max_available)
                {
                    min_cxt_blocks = mcxt.r_banks[bank].num_cxt;
                    max_available = mcxt.r_banks[bank].num_available;
                    *new_bank = bank;
                }
            }
        }
    }
    if (*new_bank == PPNVFL_INVALID_BANK_BLOCK)
    {
        return FALSE32;
    }
    return TRUE32;
}

static BOOL32 
_chooseReplacementBlock
    (UInt16 new_bank,
    UInt16 *new_block,
    UInt16 *new_block_index,
    UInt16 *new_block_cxt_page,
    Buffer *buf)
{
    UInt16 i, j, num_failed_blocks = 0;
    const UInt16 blocks_in_cxt_page = mcxt.r_gen.pbns_per_cxt_mapping_page;
    P2UsageType *p2_block_usage_map = (P2UsageType*)buf->pData;;


    // pick a replacement block
    i = mcxt.r_gen.pages_per_block_mapping;
    *new_block = PPNVFL_INVALID_BANK_BLOCK;
    do
    {
        i--;
        if (_readVFLCxtPage(buf, new_bank, i) == FALSE32)
        {
            WMR_PRINT(ERROR, "failed _readVFLCxtPage!\n");
            break;
        }
        j = blocks_in_cxt_page;
        do
        {
            UInt16 potential_new_block;
            
            j--;
            potential_new_block = j + (i * blocks_in_cxt_page);
            
            if (p2_block_usage_map[j] == PPNVFL_TYPE_AVAILABLE_MARK)
            {
                // test the block - do not mark blocks as bad if they fail, just continue
                if (ppnMiscTestSpecialBlock(&mcxt.dev, mcxt.commands[0], new_bank, potential_new_block, PPNVFL_SPARE_TYPE_TST))
                {
                    *new_block = potential_new_block;
                    *new_block_index = j;
                    *new_block_cxt_page = i;
                    break;
                }
                else
                {
                    WMR_PRINT(VFLWARN, "failed testing replacement block (bank:0x%X, block:0x%X)!\n",
                              new_bank, potential_new_block);
                    num_failed_blocks++;
                }
            }
        }
        while (j != 0);
    }
    while ((i != 0) && (*new_block == PPNVFL_INVALID_BANK_BLOCK));    

    // verify a block was found
    if (*new_block == PPNVFL_INVALID_BANK_BLOCK)
    {
        WMR_ASSERT(num_failed_blocks == mcxt.r_banks[new_bank].num_available);
        mcxt.r_banks[new_bank].num_available = 0;
        WMR_PRINT(VFLWARN, "failed searching for replacement block\n");
        return FALSE32;
    }
    return TRUE32;
}

static BOOL32
_replaceErasedCxtBlock
    (UInt16 vfl_index)
{
    UInt16 new_bank = PPNVFL_INVALID_BANK_BLOCK, new_block_index = 0xFFFF, new_block_cxt_page = 0xFFFF;
    UInt16 bank, i, new_block = PPNVFL_INVALID_BANK_BLOCK;
    P2UsageType *p2_block_usage_map = NULL;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
    const UInt16 blocks_in_cxt_page = mcxt.r_gen.pbns_per_cxt_mapping_page;
    const UInt16 replaced_block = mcxt.gen.vfl_blocks[vfl_index].block;
    const UInt16 replaced_bank = mcxt.gen.vfl_blocks[vfl_index].bank;
    const UInt16 replaced_index = replaced_block % blocks_in_cxt_page;
    const UInt16 replaced_cxt_page = replaced_block / blocks_in_cxt_page;
    const UInt16 blocks_per_vfl_set = (mcxt.r_gen.num_of_vfl_blocks >> 1);
    const UInt16 set_mapping_offset = mcxt.r_gen.next_cxt_block_offset * blocks_per_vfl_set;

    p2_block_usage_map = (P2UsageType*)buf->pData;

    _printVFLBlockMapping(__LINE__);
    WMR_ASSERT(!_checkForBankMapPages(mcxt.r_gen.next_cxt_block_offset));
    
    while (_chooseReplacementBank(&new_bank))
    {

        if (_chooseReplacementBlock(new_bank, &new_block, &new_block_index, &new_block_cxt_page, buf))
        {
            break;
        }
    }

    if (new_bank == PPNVFL_INVALID_BANK_BLOCK)
    {
#if AND_SIMULATOR
        WMR_SIM_EXIT("Out of replacement blocks");
#endif // AND_SIMULATOR
        WMR_PRINT(ERROR, "failed finding a bank to provide vfl cxt blocks!\n");
        goto return_error;
    }
    
    mcxt.gen.vfl_blocks[vfl_index].block = new_block;
    mcxt.gen.vfl_blocks[vfl_index].bank = new_bank;
    _unsetVFLCxtScrubMark(mcxt.gen.vfl_blocks[vfl_index]);

    _printVFLBlockMapping(__LINE__);
    // erase original - ignore status
    ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_ERASE,
                           PPN_OPTIONS_IGNORE_BAD_BLOCK, replaced_bank, replaced_block, 0,
                           TRUE32, NULL, NULL);

    // erase current vfl set - fail replacement operation if we get an erase error
    for (i = 0; i < blocks_per_vfl_set; i++)
    {
        PPNStatusType status;

        status = ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_ERASE,
                                        PPN_NO_OPTIONS,
                                        mcxt.gen.vfl_blocks[set_mapping_offset + i].bank,
                                        mcxt.gen.vfl_blocks[set_mapping_offset + i].block,
                                        0, TRUE32, NULL, NULL);
        if (status & (PPN_ERASE_STATUS_RETIRE | PPN_ERASE_STATUS_FAIL | PPN_PROGRAM_STATUS_GEB))
        {
            WMR_PRINT(QUAL_FATAL, "failed erase new vfl block (bank:0x%04X, block: 0x%04X) with status 0x%02X\n",
                           mcxt.gen.vfl_blocks[set_mapping_offset + i].bank,
                           mcxt.gen.vfl_blocks[set_mapping_offset + i].block, status);
            goto return_error;
        }
    }
    // program last cxt page
    mcxt.r_gen.next_cxt_page_offset = 0;
    _copyFromRamToOnNandCxt(buf->pData);
    mcxt.r_gen.cxt_age--;
    mcxt.r_gen.need_rewrite = FALSE32;

    if (!_programVFLCxtPage(buf, PPNVFL_CXT_LAST_BANKIDX, PPNVFL_CXT_LAST_BANKIDX))
    {
        WMR_PRINT(ERROR, "_programVFLCxtPage failed while allocating new vfl block (bank:0x%04X, block: 0x%04X)\n",
                       new_bank, new_block);
        goto return_error;
    }

    for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
    {
        for (i = 0; i < mcxt.r_gen.pages_per_block_mapping; i++)
        {
            if (_readVFLCxtPage(buf, bank, i) == FALSE32)
            {
                WMR_PRINT(ERROR, "failed _readVFLCxtPage !\n");
                goto return_error;
            }

            if (new_block_cxt_page == i && new_bank == bank)
            {
                p2_block_usage_map[new_block_index] = PPNVFL_TYPE_VFL_INFO;
                mcxt.r_banks[bank].num_available--;
                mcxt.r_banks[bank].num_cxt++;
            }

            if (replaced_cxt_page == i && replaced_bank == bank)
            {
                p2_block_usage_map[replaced_index] = PPNVFL_TYPE_GROWN_BB;
                mcxt.r_banks[bank].num_cxt--;
                mcxt.r_banks[bank].num_grown_bad++;
            }
            if (!_programVFLCxtPage(buf, bank, i))
            {
                WMR_PRINT(ERROR, "_programVFLCxtPage failed with the new vfl block (bank:0x%04X, block: 0x%04X)\n",
                               new_bank, new_block);
                goto return_error;
            }
        }
    }

    // program last cxt page
    _copyFromRamToOnNandCxt(buf->pData);

    if (!_programVFLCxtPage(buf, PPNVFL_CXT_LAST_BANKIDX, PPNVFL_CXT_LAST_BANKIDX))
    {
        WMR_PRINT(ERROR, "_programVFLCxtPage failed with the new vfl block (bank:0x%04X, block: 0x%04X)\n",
                       new_bank, new_block);
        goto return_error;
    }

    WMR_PRINT(VFLWARN, "replacing cxt_idx 0x%X, org (bank:0x%X, block:0x%X), new (bank:0x%X, block:0x%X)!\n",
                   vfl_index, replaced_bank, replaced_block, new_bank, new_block);

    // erase the old vfl cxt set
    for (i = 0; i < blocks_per_vfl_set; i++)
    {
        const UInt16 old_set_location = (mcxt.r_gen.next_cxt_block_offset == 1 ? 0 : 1);
        const UInt16 old_set_mapping_offset = old_set_location * blocks_per_vfl_set;
        
        if (_isVFLCxtBlockMarkedScrub(mcxt.gen.vfl_blocks[old_set_mapping_offset + i]))
        {
            // erase bad block and ignore status
            ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_ERASE,
                                   PPN_OPTIONS_IGNORE_BAD_BLOCK,
                                   mcxt.gen.vfl_blocks[old_set_mapping_offset + i].bank,
                                   mcxt.gen.vfl_blocks[old_set_mapping_offset + i].block,
                                   0,
                                   TRUE32, NULL, NULL);
        }
        else
        {
            PPNStatusType status;

            status = ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_ERASE,
                                            PPN_NO_OPTIONS,
                                            mcxt.gen.vfl_blocks[old_set_mapping_offset + i].bank,
                                            mcxt.gen.vfl_blocks[old_set_mapping_offset + i].block,
                                            0,
                                            TRUE32, NULL, NULL);

            if (status & PPN_ERASE_STATUS_RETIRE)
            {
                WMR_PRINT(QUAL, "failed erase vflcxt block (bank:0x%04X, block: 0x%04X) with retire status\n",
                               mcxt.gen.vfl_blocks[old_set_mapping_offset + i].bank,
                               mcxt.gen.vfl_blocks[old_set_mapping_offset + i].block);
                _setVFLCxtScrubMark(mcxt.gen.vfl_blocks[old_set_mapping_offset + i]);
                if (!_programVFLCxtPage(buf, PPNVFL_CXT_LAST_BANKIDX, PPNVFL_CXT_LAST_BANKIDX))
                {
                    WMR_PRINT(ERROR, "_programVFLCxtPage failed while allocating new vfl block (bank:0x%04X, block: 0x%04X)\n",
                              new_bank, new_block);
                    goto return_error;
                }
            }
            else if (status & PPN_ERASE_STATUS_FAIL)
            {
                WMR_PRINT(QUAL_FATAL, "failed erase vflcxt block (bank:0x%04X, block: 0x%04X) with no retire status\n",
                               mcxt.gen.vfl_blocks[old_set_mapping_offset + i].bank,
                               mcxt.gen.vfl_blocks[old_set_mapping_offset + i].block);
                goto return_error;
            }
        }
    }

    BUF_Release(buf);
    return TRUE32;

 return_error:
    BUF_Release(buf);
    return FALSE32;
}

#endif // AND_READONLY

#ifndef AND_READONLY

static BOOL32
_markBlockForRetirement
    (UInt16 bank,
    UInt16 block)
{
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
    BOOL32 ret_val;

    _markBlockToScrubInBitmap(bank, block);
    ret_val = _setBlockUsageInVFLCxt(bank, block, PPNVFL_P2V_SCRUB_BIT);

    if (ret_val)
    {
        ret_val = _programCxtLastPage(buf, TRUE32);
    }

    BUF_Release(buf);

    return ret_val;
}

#endif // AND_READONLY

/* block replacement functions - end */

/*
 * Name: _analyzeSpansReadStatuses
 * Description: this function analyzes the read operation statuses (used for multipage read)
 * Input: number of pages read, mark for scrub
 * Output: need refresh flag, operation result flag, clean flag
 * Return value: none
 */
static void
_analyzeSpansReadStatuses
    (UInt16 vba_index_start,
    UInt16 vba_index_count,
    VFLReadStatusType *result_flags,
    BOOL32 boolMarkForScrub,
    VFL_ReadSpans_t *s)
{
    UInt16 channel;

    // look at the status
    for (channel = 0; channel < mcxt.dev.num_channels; channel++)
    {
        UInt16 i;
        PPNCommandStruct *command = mcxt.commands[channel];

        // locate the block that need to be replaced
        for (i = 0; i < command->num_pages; i++)
        {
            PPNStatusType current_status = mcxt.commands[channel]->entry[i].status;
            VFLReadStatusType current_result = 0;
            BOOL32 call_ftl_callback = FALSE32, retire_block = FALSE32;

            switch (current_status)
            {
                case (PPN_READ_STATUS_VALID):
                case (PPN_READ_STATUS_VALID | PPN_READ_STATUS_REFRESH):
                case (PPN_READ_STATUS_VALID | PPN_READ_STATUS_RETIRE):
                case (PPN_READ_STATUS_INVALID_DATA | PPN_READ_STATUS_REFRESH | PPN_READ_STATUS_VALID):
                case (PPN_READ_STATUS_INVALID_DATA | PPN_READ_STATUS_RETIRE | PPN_READ_STATUS_VALID):
                case (PPN_READ_STATUS_INVALID_DATA | PPN_READ_STATUS_CLEAN | PPN_READ_STATUS_VALID):
                case (PPN_READ_STATUS_INVALID_DATA | PPN_READ_STATUS_GEB | PPN_READ_STATUS_VALID):
                case (PPN_READ_STATUS_INVALID_DATA | PPN_READ_STATUS_WRONG_BITS_PER_CELL | PPN_READ_STATUS_VALID):
                    break;

                default:
                    current_result |= VFL_READ_STATUS_UNIDENTIFIED;

                    if (WMR_I_CAN_HAZ_DEBUGGER())
                    {
                        // avoid the callback in case we have unrecognized status
                        // and connected to debugger
                        s->cbOpt = 0;
                        s->cb = NULL;
                    }

                    break;
            }
            if (!(current_result & VFL_READ_STATUS_UNIDENTIFIED))
            {
                if (current_status & PPN_READ_STATUS_INVALID_DATA &&
                    (!(current_status & PPN_READ_STATUS_CLEAN)))
                {
                    current_result |= VFL_READ_STATUS_UECC;
                }
                if (current_status & PPN_READ_STATUS_WRONG_BITS_PER_CELL)
                {
                    current_result |= VFL_READ_STATUS_UECC;
                }
                if (current_status & PPN_READ_STATUS_CLEAN)
                {
                    current_result |= VFL_READ_STATUS_CLEAN;
                }
                if (current_status & PPN_READ_STATUS_REFRESH)
                {
                    current_result |= VFL_READ_STATUS_REFRESH;
                }
                if (current_status & PPN_READ_STATUS_RETIRE)
                {
                    current_result |= VFL_READ_STATUS_RETIRE;
                }
                if (!(current_status & PPN_READ_STATUS_INVALID_DATA))
                {
                    current_result |= VFL_READ_STATUS_VALID_DATA;
                }
            }
            *result_flags |= current_result;
            retire_block = ((boolMarkForScrub && 
                             (current_status & (PPN_READ_STATUS_RETIRE | PPN_READ_STATUS_WRONG_BITS_PER_CELL)))
                            ? TRUE32 : FALSE32);
            call_ftl_callback = ((s != NULL) && (current_result & s->cbOpt) && (s->cb != NULL)) ? TRUE32 : FALSE32;

            if (retire_block || call_ftl_callback)
            {
                UInt16 vba_idx, bank, block, page_offset;
                BOOL32 slc;
                UInt32 vba;
                UInt8 *meta, *page_meta;
                const PPNCommandEntry *entry = &command->entry[i];
                const PPNMemoryIndex *memoryIndex = &command->mem_index[i];
                const PageAddressType page = mcxt.commands[channel]->entry[i].addr.row;
                const UInt16 chip_enable_idx = mcxt.commands[channel]->entry[i].ceIdx;
                const UInt16 column_addr = mcxt.commands[channel]->entry[i].addr.column; 

                _convertReadAddressToVba(channel, entry->ceIdx, &entry->addr,
                                         &vba, &bank, &block, &page_offset, &slc);

                if (retire_block)
                {
#ifndef AND_READONLY
                    _markBlockForRetirement(bank, block);
#endif // ! AND_READONLY

                    WMR_PRINT(VFLWARN, "reporting a read error status = 0x%X vba:0x%X result:0x%X\n",
                                   current_status, vba, current_result);
                    WMR_PRINT(VFLWARN, "bank:0x%X, block:0x%X, page_offset:0x%X, slc:0x%X\n",
                                   bank, block, page_offset, slc);
                    WMR_PRINT(VFLWARN, "channel:0x%X, chip_enable_idx:0x%X, page_address:0x%X, column: 0x%X\n",
                                   channel, chip_enable_idx, page, column_addr);
                }

                meta = (UInt8*)command->mem_buffers[memoryIndex->idx].meta;
                page_meta = &meta[memoryIndex->offset * mcxt.dev.lba_meta_bytes_buffer];
                for (vba_idx = 0; vba_idx < entry->lbas; vba_idx++)
                {
                    UInt8 *curr_meta = &page_meta[vba_idx * mcxt.dev.lba_meta_bytes_buffer];
                    UInt32 curr_vba = vba + vba_idx;

                    if (retire_block)
                    {
                        WMR_PRINT(VFLWARN, "retiring vba:0x%08X status:0x%02X result:0x%08X "
                                            "meta: %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X\n",
                                       curr_vba, current_status, current_result,
                                       curr_meta[0], curr_meta[1], curr_meta[2], curr_meta[3],
                                       curr_meta[4], curr_meta[5], curr_meta[6], curr_meta[7],
                                       curr_meta[8], curr_meta[9], curr_meta[10], curr_meta[11],
                                       curr_meta[12], curr_meta[13], curr_meta[14], curr_meta[15]);
                    }

                    if (call_ftl_callback)
                    {
                        s->cb(curr_vba, current_result, curr_meta);
                    }
                }
            }
        }
    }

    if ((*result_flags &
         (VFL_READ_STATUS_REFRESH | VFL_READ_STATUS_RETIRE |
          VFL_READ_STATUS_UECC | VFL_READ_STATUS_UNIDENTIFIED)) &&
        WMR_I_CAN_HAZ_DEBUGGER())
    {
        WMR_PRINT(VFLWARN, "Unexpected PPN read results flags:0x%X\n", *result_flags);
        WMR_PRINT(VFLWARN, "st vba      ba blk  ofs s ch ce  len  col  row       meta \n");
        for (channel = 0; channel < mcxt.dev.num_channels; channel++)
        {
            UInt16 i;

            // locate the block that need to be replaced
            for (i = 0; i < mcxt.commands[channel]->num_pages; i++)
            {
                const PPNCommandStruct *command = mcxt.commands[channel];
                const PPNCommandEntry *entry = &command->entry[i];
                const PPNMemoryIndex *memoryIndex = &command->mem_index[i];
                const PPNStatusType current_status = entry->status;
                const UInt16 chip_enable_idx = entry->ceIdx;
                const UInt8 *meta = (UInt8*)command->mem_buffers[memoryIndex->idx].meta;
                const UInt8 *page_meta = &meta[memoryIndex->offset * mcxt.dev.lba_meta_bytes_buffer];
                UInt16 bank, block, page_offset, vba_idx;
                BOOL32 slc;
                UInt32 vba;

                _convertReadAddressToVba(channel, chip_enable_idx, &entry->addr,
                                         &vba, &bank, &block, &page_offset, &slc);

                for (vba_idx = 0; vba_idx < entry->lbas; vba_idx++)
                {
                    const UInt8 *curr_meta = &page_meta[vba_idx * mcxt.dev.lba_meta_bytes_buffer];
                    UInt32 curr_vba = vba + vba_idx;
                    WMR_PRINT(VFLWARN, "%02X %08X %02X %04X %03X %01X %02X %02X"
                                       "  %04X %04X %08X"
                                       "  %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X\n",
                                   current_status, curr_vba, bank, block, page_offset, slc, channel, chip_enable_idx,
                                   entry->addr.length, entry->addr.column, entry->addr.row,
                                   curr_meta[0], curr_meta[1], curr_meta[2], curr_meta[3],
                                   curr_meta[4], curr_meta[5], curr_meta[6], curr_meta[7],
                                   curr_meta[8], curr_meta[9], curr_meta[10], curr_meta[11],
                                   curr_meta[12], curr_meta[13], curr_meta[14], curr_meta[15]);
                }
            }
        }
    }

    if (!(*result_flags))
    {
        WMR_PANIC("result flags: 0x%02x\n", (UInt32) * result_flags);
    }
    
    if(*result_flags & (VFL_READ_STATUS_UNIDENTIFIED | VFL_READ_STATUS_REFRESH | 
                        VFL_READ_STATUS_RETIRE | VFL_READ_STATUS_UECC))
    {
        if (WMR_I_CAN_HAZ_DEBUGGER() && (*result_flags & VFL_READ_STATUS_UNIDENTIFIED))
        {
            WMR_PANIC("Read operation contained an unidentified status\n", (UInt32) * result_flags);
        }
        
        if(boolMarkForScrub)
        {
            WMR_PRINT(QUAL, "Errors in PPN read results flags:0x%X\n", *result_flags);
        }
        WMR_PRINT(UECC_PANIC, "Errors in PPN read: results flags:0x%X, boolMarkForScrub: %d\n", *result_flags, boolMarkForScrub);
    }
}

static void
_copyMapPagesFromOnNandCxtToRam
    (UInt8 *data)
{
    UInt16 bank;
    PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)data;

    for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
    {
        WMR_MEMCPY(mcxt.vfl_map_pages[bank], on_nand->banks[bank].vfl_map_pages,
                   mcxt.r_gen.pages_per_block_mapping * sizeof(UInt16));
    }
}

static void
_copyFromOnNandCxtToRam
    (UInt8 *data)
{
    PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)data;

    // copy gen
    WMR_MEMCPY(&mcxt.gen, &on_nand->gen, sizeof(PPNVFL_GeneralCxt));
    // copy vfl map locations
    _copyMapPagesFromOnNandCxtToRam(data);
}

#if !AND_READONLY

static void
_copyFromRamToOnNandCxt
    (UInt8 *data)
{
    UInt16 bank;
    PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)data;

    WMR_MEMSET(data, 0, mcxt.dev.main_bytes_per_page);
    // copy gen
    WMR_MEMCPY(&on_nand->gen, &mcxt.gen, sizeof(PPNVFL_GeneralCxt));

    for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
    {
        WMR_MEMCPY(on_nand->banks[bank].vfl_map_pages, mcxt.vfl_map_pages[bank],
                   mcxt.r_gen.pages_per_block_mapping * sizeof(UInt16));
    }
}

#endif // !AND_READONLY

static UInt16
_div2RoundUp
    (UInt16 in)
{
    return (((in) & 0x1) == 1) ? (((in) >> 1) + 1) : ((in) >> 1);
}

static Int32
_readPageOfASet
    (UInt16 set,
    UInt16 cxt_page_offset,
    Buffer *buf,
    UInt8 expected_bank,
    UInt8 expected_idx)
{
    UInt16 redundant_copy;
    PPNStatusType status;
    UInt16 vfl_block_offset = 0;
    UInt16 vfl_page_offset = 0;
    BOOL32 clean = TRUE32;
    const BOOL32 original_need_rewrite = mcxt.r_gen.need_rewrite;

    _convertVFLPageIdxToPhysicalPageBlockOffsets(set, cxt_page_offset,
                                                 &vfl_block_offset, &vfl_page_offset);

    for (redundant_copy = 0; redundant_copy < mcxt.r_gen.vfl_blocks_redundancy; redundant_copy++)
    {
        const UInt16 bank = mcxt.gen.vfl_blocks[vfl_block_offset + redundant_copy].bank;
        const UInt16 block = mcxt.gen.vfl_blocks[vfl_block_offset + redundant_copy].block;
        PPNVFL_CxtSpare *cxt_spare = (PPNVFL_CxtSpare*) buf->pSpare;

        status = ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_READ, PPN_NO_OPTIONS,
                                        bank, block, vfl_page_offset, TRUE32, buf->pData, buf->pSpare);

        // check if this is not the first copy or if the copy is marked to be moved
        if (((redundant_copy != 0) ||
             (status & (PPN_READ_STATUS_REFRESH | PPN_READ_STATUS_RETIRE))) &&
            (set == mcxt.r_gen.next_cxt_block_offset))
        {
            // avoid the prints when searching through clean pages...
            // print only if we ran into none clean pages
            if (!clean)
            {
                WMR_PRINT(VFLWARN, "read a copy of the vfl cxt with status 0x%02X"
                          " set:%01d, copy:%01d, page_offset:0x%03X type:0x%02X\n",
                          status, set, redundant_copy, cxt_page_offset, cxt_spare->spareType);
            }
            
            mcxt.r_gen.need_rewrite = TRUE32;
        }

        // if this is not initial search for cxt we should take the indication
        // for replacement from the device
        if ((status & PPN_READ_STATUS_RETIRE) && (!mcxt.r_gen.search_for_cxt))
        {
            _setVFLCxtScrubMark(mcxt.gen.vfl_blocks[vfl_block_offset + redundant_copy]);
        }

        if (!(status & PPN_READ_STATUS_INVALID_DATA))
        {
            UInt8 rps_status = 0;

            if (cxt_spare->spareType != PPNVFL_SPARE_TYPE_CXT)
            {
                rps_status |= RPS_STATUS_SPARE_TYPE_MISSMATCH;
            }
            
            if ((expected_bank != cxt_spare->bank) && (expected_bank != PPNVFL_CXT_IGNORE_BANKIDX))
            {
                rps_status |= RPS_STATUS_BANK_MISSMATCH;
            }

            if ((expected_idx != cxt_spare->idx) && (expected_idx != PPNVFL_CXT_IGNORE_BANKIDX))
            {
                rps_status |= RPS_STATUS_IDX_MISSMATCH;
            }

            if (mcxt.r_gen.need_rewrite && (!original_need_rewrite))
            {
                WMR_PRINT(VFLWARN, "failed reading the first copy of the vfl cxt "
                                    "(succeeded reading another copy)\n");
            }

            if (rps_status == 0)
            {
                return ANDErrorCodeOk;                
            }
            else
            {
                WMR_PRINT(ERROR, "problem reading a vfl cxt copy"
                          " rps_status:0x%02X, set:%01d, copy:%01d, page_offset:0x%03X"
                          " type:0x%02X bank:0x%02X idx:0x%02X\n",
                          rps_status, set, redundant_copy, cxt_page_offset, 
                          cxt_spare->spareType, cxt_spare->bank, cxt_spare->idx);
                mcxt.r_gen.need_rewrite = TRUE32;
            }
        }

        if (!(status & PPN_READ_STATUS_CLEAN))
        {
            clean = FALSE32;
        }
    }

    if (clean)
    {
        mcxt.r_gen.need_rewrite = original_need_rewrite;
        return ANDErrorCodeCleanOk;
    }

    WMR_PRINT(VFLWARN, "failed to read a single vfl cxt copy (UECC)\n");
    mcxt.r_gen.need_rewrite = TRUE32;
    return ANDErrorCodeUserDataErr;
}

static Int32
_readSingleRedundancyPageOfASet
    (UInt16 set,
    UInt16 cxt_page_offset,
    UInt16 redundant_copy,
    Buffer *buf)
{
    PPNStatusType status;
    UInt16 vfl_block_offset = 0, vfl_page_offset = 0, bank = PPNVFL_INVALID_BANK_BLOCK, block = PPNVFL_INVALID_BANK_BLOCK;

    _convertVFLPageIdxToPhysicalPageBlockOffsets(set, cxt_page_offset,
                                                 &vfl_block_offset, &vfl_page_offset);

    bank = mcxt.gen.vfl_blocks[vfl_block_offset + redundant_copy].bank;
    block = mcxt.gen.vfl_blocks[vfl_block_offset + redundant_copy].block;

    status = ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_READ, PPN_NO_OPTIONS,
                                    bank, block, vfl_page_offset, TRUE32, buf->pData, buf->pSpare);

    // check if this is not the first copy or if the copy is marked to be moved
    if ((status & (PPN_READ_STATUS_REFRESH | PPN_READ_STATUS_RETIRE)) &&
        (set == mcxt.r_gen.next_cxt_block_offset))
    {
        WMR_PRINT(VFLWARN, "_readSingleRedundancyPageOfASet got status:0x%02X from "
                            "redundant_copy:0x%04X set:0x%04X cxt_page_offset:0x%04X\n",
                       status, redundant_copy, set, cxt_page_offset);
        mcxt.r_gen.need_rewrite = TRUE32;
    }

    // if this is not initial search for cxt we should take the indication
    // for replacement from the device
    if ((status & PPN_READ_STATUS_RETIRE) && (!mcxt.r_gen.search_for_cxt))
    {
        _setVFLCxtScrubMark(mcxt.gen.vfl_blocks[vfl_block_offset + redundant_copy]);
    }

    if (!(status & PPN_READ_STATUS_INVALID_DATA))
    {
        return ANDErrorCodeOk;
    }

    if (status & PPN_READ_STATUS_CLEAN)
    {
        return ANDErrorCodeCleanOk;
    }

    return ANDErrorCodeUserDataErr;
}

/* vfl cxt management functions - start */
/*
 * Name: _findOnNANDCxt
 * Description: this function find the current cxt per bank and update the information
 *              in ram.
 * Input: bank
 * Return value: TRUE32 is successful, FALSE32 otherwise.
 */

static BOOL32
_findOnNANDCxt
    (void)
{
    Buffer *buf, *buf2;
    UInt16 bank = ~0, page, start_page, mid_page, end_page, block_idx, vfl_cxt_idx, cxt_blocks_idx;
    PPNStatusType status;
    PPNVFL_CxtSpare spare0, spare1;
    BOOL32 found_valid_cxt;

    buf = BUF_Get(BUF_MAIN_AND_SPARE);
    buf2 = BUF_Get(BUF_MAIN_AND_SPARE);

    // find the first written VFLCxt
    found_valid_cxt = FALSE32;
    for (block_idx = (mcxt.dev.blocks_per_cau - 1); ((!found_valid_cxt) && (block_idx != 0)); block_idx--)
    {
        for (bank = 0; ((!found_valid_cxt) && (bank < mcxt.dev.num_of_banks)); bank++)
        {
            PPNVFL_CxtSpare *cxt_spare = (PPNVFL_CxtSpare*)buf->pSpare;
            status = ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_READ,
                                            PPN_NO_OPTIONS, bank, block_idx, 0, TRUE32,
                                            buf->pData, buf->pSpare);
            if ((!(status & PPN_READ_STATUS_INVALID_DATA)) &&
                cxt_spare->spareType == PPNVFL_SPARE_TYPE_CXT &&
                cxt_spare->idx == PPNVFL_CXT_LAST_BANKIDX)
            {
                PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)buf->pData;
                WMR_MEMCPY(mcxt.gen.vfl_blocks, on_nand->gen.vfl_blocks,
                           (sizeof(VFLCxtBlockAddress) * mcxt.r_gen.num_of_vfl_blocks));
                mcxt.r_gen.cxt_age = cxt_spare->cxt_age;
                found_valid_cxt = TRUE32;
                break;
            }
        }
    }
    if (!found_valid_cxt)
    {
        WMR_PRINT(ERROR, "could not find a single copy of VFLCxt\n");
        goto error_return;
    }

    // agree on the location of the VFLCxt blocks - find the latest blocks and see that they all point to the same VFL
    // blocks
    found_valid_cxt = FALSE32;
    do
    {
        for (vfl_cxt_idx = 0; vfl_cxt_idx < mcxt.r_gen.num_of_vfl_blocks; vfl_cxt_idx++)
        {
            PPNVFL_CxtSpare *cxt_spare = (PPNVFL_CxtSpare*)buf->pSpare;
            status = ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_READ,
                                            PPN_NO_OPTIONS,
                                            mcxt.gen.vfl_blocks[vfl_cxt_idx].bank,
                                            mcxt.gen.vfl_blocks[vfl_cxt_idx].block,
                                            0, TRUE32, buf->pData, buf->pSpare);
            if ((!(status & PPN_READ_STATUS_INVALID_DATA)) &&
                cxt_spare->spareType == PPNVFL_SPARE_TYPE_CXT &&
                cxt_spare->idx == PPNVFL_CXT_LAST_BANKIDX &&
                mcxt.r_gen.cxt_age > cxt_spare->cxt_age)
            {
                PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)buf->pData;
                WMR_MEMCPY(mcxt.gen.vfl_blocks, on_nand->gen.vfl_blocks,
                           (sizeof(VFLCxtBlockAddress) * mcxt.r_gen.num_of_vfl_blocks));
                mcxt.r_gen.cxt_age = cxt_spare->cxt_age;
                break;
            }
        }
        if (vfl_cxt_idx == mcxt.r_gen.num_of_vfl_blocks)
        {
            found_valid_cxt = TRUE32;
        }
    }
    while (!found_valid_cxt);

    // use spare0 for the 1st pair and spare1 for the 2nd
    WMR_MEMSET(&spare0, 0, sizeof(PPNVFL_CxtSpare));
    WMR_MEMSET(&spare1, 0, sizeof(PPNVFL_CxtSpare));

    // read the 1st set
    if (_readPageOfASet(0, 0, buf, PPNVFL_CXT_LAST_BANKIDX, PPNVFL_CXT_LAST_BANKIDX) == ANDErrorCodeOk)
    {
        WMR_MEMCPY(&spare0, buf->pSpare, sizeof(PPNVFL_CxtSpare));
    }

    // read the 2nd set
    if (_readPageOfASet(1, 0, buf2, PPNVFL_CXT_LAST_BANKIDX, PPNVFL_CXT_LAST_BANKIDX) == ANDErrorCodeOk)
    {
        WMR_MEMCPY(&spare1, buf2->pSpare, sizeof(PPNVFL_CxtSpare));
    }

    // make a decision which pair of VFLCxt blcoks is current one and use it
    if (spare0.spareType == PPNVFL_SPARE_TYPE_CXT && spare1.spareType == PPNVFL_SPARE_TYPE_CXT)
    {
        WMR_ASSERT(spare0.cxt_age != spare1.cxt_age);
        
        // assume that if we have two valid copies the older one is good
        // as soon as we have a good good set in a new block we should erase the
        // old block
        if ((UInt32)spare0.cxt_age < (UInt32)spare1.cxt_age)
        {
            mcxt.r_gen.next_cxt_block_offset = 0;
            WMR_PRINT(INF, "_findOnNANDCxt() bank:0x%04X AGE0 0x%X, age1 0x%X, type0 0x%X, type1 0x%X\n",
                           bank, spare0.cxt_age, spare1.cxt_age, spare0.spareType, spare1.spareType);
        }
        else
        {
            mcxt.r_gen.next_cxt_block_offset = 1;
            WMR_PRINT(INF, "_findOnNANDCxt() bank:0x%04X age0 0x%X, AGE1 0x%X, type0 0x%X, type1 0x%X\n",
                           bank, spare0.cxt_age, spare1.cxt_age, spare0.spareType, spare1.spareType);
        }
    }
    else if (spare0.spareType == PPNVFL_SPARE_TYPE_CXT)
    {
        mcxt.r_gen.next_cxt_block_offset = 0;
    }
    else if (spare1.spareType == PPNVFL_SPARE_TYPE_CXT)
    {
        mcxt.r_gen.next_cxt_block_offset = 1;
    }
    else
    {
        WMR_PRINT(INF, "could not find a single copy of VFLCxt (bank:0x%X)\n",bank);
        goto error_return;
    }

    // make sure the block list is up to date
    if (mcxt.r_gen.next_cxt_block_offset)
    {
        PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)buf2->pData;
        WMR_MEMCPY(mcxt.gen.vfl_blocks, on_nand->gen.vfl_blocks,
                   (sizeof(VFLCxtBlockAddress) * mcxt.r_gen.num_of_vfl_blocks));
    }
    else
    {
        PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)buf->pData;
        WMR_MEMCPY(mcxt.gen.vfl_blocks, on_nand->gen.vfl_blocks,
                   (sizeof(VFLCxtBlockAddress) * mcxt.r_gen.num_of_vfl_blocks));
    }

    // search for the latest version of the VFLCxt inside the current VFLCxt block pair
    start_page = 1; // a set with a single page is not a valid set
    end_page = mcxt.r_gen.vfl_pages_per_set - 1;
    found_valid_cxt = FALSE32;

    while (end_page >= start_page)
    {
        PPNVFL_CxtSpare *spare = (PPNVFL_CxtSpare*)buf->pSpare;
        mid_page = _div2RoundUp(end_page + start_page);
        Int32 read_status = _readPageOfASet(mcxt.r_gen.next_cxt_block_offset, mid_page, 
                                            buf, PPNVFL_CXT_IGNORE_BANKIDX, PPNVFL_CXT_IGNORE_BANKIDX);

        if (read_status == ANDErrorCodeOk)
        {
            if (start_page == end_page)
            {
                if (spare->idx == PPNVFL_CXT_LAST_BANKIDX)
                {
                    mcxt.r_gen.cxt_age = spare->cxt_age;
                    mcxt.r_gen.next_cxt_page_offset = start_page + 1;

                    _copyFromOnNandCxtToRam(buf->pData);

                    // verify the last page we wrote is valid
                    if (_readSingleRedundancyPageOfASet(mcxt.r_gen.next_cxt_block_offset, start_page,
                                                        (mcxt.r_gen.vfl_blocks_redundancy - 1), buf) != ANDErrorCodeOk)
                    {
                        mcxt.r_gen.need_rewrite = TRUE32;
                        WMR_PRINT(VFLWARN, "could not find vfl cxt in the last redundancy copy\n");
                    }

                    found_valid_cxt = TRUE32;
                    break;
                }
                else
                {
                    mcxt.r_gen.need_rewrite = TRUE32;
                    WMR_PRINT(VFLWARN, "could not find cxt in bubble search\n");
                    break;
                }
            }
            else
            {
                start_page = mid_page;
                continue;
            }
        }

        if (read_status == ANDErrorCodeCleanOk)
        {
            end_page = mid_page - 1;
            continue;
        }

        WMR_PRINT(VFLWARN, "deteteced vfl cxt pages with user data errors\n");
        end_page = mid_page;
        mcxt.r_gen.need_rewrite = TRUE32;
        break;
    }

    for (cxt_blocks_idx = 0; (cxt_blocks_idx < 2) && (!found_valid_cxt); cxt_blocks_idx++)
    {
        for (page = (mcxt.r_gen.vfl_pages_per_set - 1); !found_valid_cxt; page--)
        {
            PPNVFL_CxtSpare *spare = (PPNVFL_CxtSpare*)buf->pSpare;

            Int32 read_status = _readPageOfASet(mcxt.r_gen.next_cxt_block_offset, page, buf, 
                                                PPNVFL_CXT_IGNORE_BANKIDX, PPNVFL_CXT_IGNORE_BANKIDX);

            if (read_status == ANDErrorCodeCleanOk)
            {
                continue;
            }

            WMR_PRINT(VFLWARN, "Reading cxt next_cxt_block_offset:0x%04X, page:0x%04X read_status:0x%08X\n",
                           mcxt.r_gen.next_cxt_block_offset, page, read_status);
            WMR_PRINT(VFLWARN, "spareType:0x%02X, bank:0x%02X, idx:0x%02X cxt_age:0x%08X\n",
                           spare->spareType, spare->bank, spare->idx, spare->cxt_age);

            if ((read_status == ANDErrorCodeOk) && (spare->spareType == PPNVFL_SPARE_TYPE_CXT) &&
                (spare->idx == PPNVFL_CXT_LAST_BANKIDX))
            {
                mcxt.r_gen.cxt_age = spare->cxt_age;
                _copyFromOnNandCxtToRam(buf->pData);
                found_valid_cxt = TRUE32;

                if (0 == page)
                {
                    // if the latest summary page we found is in page 0 the current block does not 
                    // contain a full set we should copy to it from the other block
                    mcxt.r_gen.next_cxt_block_offset = (mcxt.r_gen.next_cxt_block_offset == 1 ? 0 : 1);
                }

                break;
            }

            if (page == 0)
            {
                break;
            }
        }

        if (!found_valid_cxt)
        {
            // switch to the other cxt set
            mcxt.r_gen.next_cxt_block_offset = (mcxt.r_gen.next_cxt_block_offset == 1 ? 0 : 1);
        }
    }

    if (!found_valid_cxt)
    {
        WMR_PRINT(ERROR, "unable to find vfl cxt copy on the device\n");
        goto error_return;
    }

    if (mcxt.gen.num_of_ftl_sublk > mcxt.gen.num_of_vfl_sublk)
    {
        WMR_PRINT(ERROR, "FTLSuBlks(=%d) > VFLSuBlks(=%d)\n",
                       mcxt.gen.num_of_ftl_sublk, mcxt.gen.num_of_vfl_sublk);
        goto error_return;
    }

    WMR_PRINT(INF, "_findOnNANDCxt(0x%04X) block 0x%X, page 0x%X, age 0x%X\n",
                   bank, mcxt.r_gen.next_cxt_block_offset, mcxt.r_gen.next_cxt_page_offset,
                   mcxt.r_gen.cxt_age);

    // initialize cached mapping
    mcxt.r_gen.cached_vbn = 0xFFFF;
    mcxt.r_gen.cached_vbn_good_blocks = 0;
    WMR_MEMSET(mcxt.r_gen.cached_banks_v2p, 0xFF, sizeof(mcxt.r_gen.cached_banks_v2p));

    BUF_Release(buf);
    BUF_Release(buf2);
    return TRUE32;

 error_return:
    BUF_Release(buf);
    BUF_Release(buf2);
    return FALSE32;
}

/*
 * Name: _readVFLCxtPage
 * Description: reads a single VFL Cxt page (either mapping or VFLCxt struct)
 * Input: buffer, bank, index (what page to read).
 * Return value: TRUE32 is successful, FALSE32 otherwise.
 */

static BOOL32
_readVFLCxtPage
    (Buffer *buf,
    UInt16 bank,
    UInt16 vfl_idx)
{
    PPNVFL_CxtSpare *spare = (PPNVFL_CxtSpare*)buf->pSpare;
    Int32 read_status;
    const UInt16 set = ((mcxt.vfl_map_pages[bank][vfl_idx] & 0x8000) >> 15);
    const UInt16 cxt_page_offset = (mcxt.vfl_map_pages[bank][vfl_idx] & 0x7FFF);

    WMR_PRINT(INF, "readVFLCxtPage(0x%04X, 0x%04X)\n", bank, vfl_idx);
    read_status = _readPageOfASet(set, cxt_page_offset, buf, bank, vfl_idx);

    if ((read_status != ANDErrorCodeOk) ||
        ((spare->spareType != PPNVFL_SPARE_TYPE_CXT) ||
         (spare->idx != (UInt8)vfl_idx)))
    {
        WMR_PRINT(ERROR, "Error reading both cxt copies status 0x%X spareType 0x%02X index 0x%02X requested index 0x%02X\n",
                       read_status, spare->spareType, spare->idx, (UInt8)vfl_idx);
        return FALSE32;
    }
    return TRUE32;
}

static void
_convertVFLPageIdxToPhysicalPageBlockOffsets
    (UInt16 set,
    UInt16 vfl_page_idx,
    UInt16 *vfl_block_offset,
    UInt16 *vfl_page_offset)
{
    const UInt16 vfl_block_depth = (vfl_page_idx / mcxt.dev.pages_per_block_slc);

    *vfl_block_offset = ((set * mcxt.r_gen.vfl_blocks_redundancy * mcxt.r_gen.vfl_blocks_depth) +
                         (vfl_block_depth * mcxt.r_gen.vfl_blocks_redundancy));
    *vfl_page_offset = vfl_page_idx % mcxt.dev.pages_per_block_slc;
}
/*
 * Name: _programVFLCxtPage
 * Description: programs a single VFL Cxt page (either mapping or VFLCxt struct)
 * Input: buffer, bank, index (what page to read).
 * Return value: TRUE32 is successful, FALSE32 otherwise.
 */

#ifndef AND_READONLY

static BOOL32
_programVFLCxtPage
    (Buffer *buf,
    UInt16 bank,
    UInt16 vfl_idx)
{
    PPNStatusType status;
    UInt16 redundant_copy, set = mcxt.r_gen.next_cxt_block_offset;
    UInt16 vfl_block_offset, vfl_page_offset;

    WMR_PRINT(INF, "programVFLCxtPage(0x%04X, 0x%04X) page offset - 0x%X, set - 0x%X\n",
                   bank, vfl_idx, mcxt.r_gen.next_cxt_page_offset, set);

    WMR_ASSERT(mcxt.r_gen.next_cxt_page_offset != 0 || ((PPNVFL_CXT_LAST_BANKIDX == bank) && (PPNVFL_CXT_LAST_BANKIDX == vfl_idx)));
    
    _setVFLCxtSpare(buf->pSpare, bank, vfl_idx);

    _convertVFLPageIdxToPhysicalPageBlockOffsets(set, mcxt.r_gen.next_cxt_page_offset,
                                                 &vfl_block_offset, &vfl_page_offset);

    for (redundant_copy = 0; ((redundant_copy < mcxt.r_gen.vfl_blocks_redundancy) &&
                              (!mcxt.r_gen.need_rewrite)); redundant_copy++)
    {
        status = ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_PROGRAM,
                                        PPN_NO_OPTIONS,
                                        mcxt.gen.vfl_blocks[vfl_block_offset + redundant_copy].bank,
                                        mcxt.gen.vfl_blocks[vfl_block_offset + redundant_copy].block,
                                        vfl_page_offset, TRUE32, buf->pData, buf->pSpare);
        if (status & (PPN_PROGRAM_STATUS_FAIL | PPN_PROGRAM_STATUS_GEB))
        {
            PPNVFL_CxtSpare * spare = (PPNVFL_CxtSpare*) buf->pSpare;
            mcxt.r_gen.need_rewrite = TRUE32;

            // leaving this print as warning so EFI does not stop till the next one
            WMR_PRINT(VFLWARN, "status 0x%X marking vflcxt block for scrubbing(bank:0x%X, block:0x%X)\n",
                           status,
                           mcxt.gen.vfl_blocks[vfl_block_offset + redundant_copy].bank,
                           mcxt.gen.vfl_blocks[vfl_block_offset + redundant_copy].block);
            WMR_PRINT(QUAL, "spare->bank 0x%X, spare->idx 0x%X, spare->cxt_age 0x%X\n",
                           spare->bank, spare->idx, spare->cxt_age);
            _setVFLCxtScrubMark(mcxt.gen.vfl_blocks[vfl_block_offset + redundant_copy]);
        }
    }
    if (mcxt.r_gen.need_rewrite)
    {
        WMR_PRINT(VFLWARN, "failed _programVFLCxtPage mcxt.r_gen.need_rewrite is true\n");
        return FALSE32;
    }
    if (vfl_idx != PPNVFL_CXT_LAST_BANKIDX)
    {
        mcxt.vfl_map_pages[bank][vfl_idx] = (mcxt.r_gen.next_cxt_page_offset |
                                              ((mcxt.r_gen.next_cxt_block_offset & 0x0001) << 15));
    }

    mcxt.r_gen.next_cxt_page_offset++;
    mcxt.r_gen.cxt_age--;

    return TRUE32;
}

static ProgramVFLCxtPageWrapStatus
_programVFLCxtPageWrap
    (Buffer *buf,
    UInt16 bank,
    UInt16 vfl_idx)
{
    WMR_PRINT(INF, "programVFLCxtPageWrap(0x%04X, 0x%04X)\n", bank, vfl_idx);
    if (mcxt.r_gen.next_cxt_page_offset == mcxt.r_gen.vfl_pages_per_set ||
        mcxt.r_gen.need_rewrite)
    {
        WMR_PRINT(VFLWARN, "Changing PPNVFL Cxt blocks pair ()\n");
        if (_copyVFLCxtToOtherPair() == FALSE32)
        {
            return VFLCxtProgramFail;
        }
        // if this is the last page, assume the copy function wrote it
        if (vfl_idx == PPNVFL_CXT_LAST_BANKIDX)
        {
            return VFLCxtProgramDone;
        }
        return VFLCxtProgramRetry;
    }

    return _programVFLCxtPage(buf, bank, vfl_idx) ? VFLCxtProgramDone : VFLCxtProgramRetry;
}

static BOOL32
_checkForVFLBlocksInScrubList
    (UInt16 new_set_location,
    BOOL32 *replace_blocks_array,
    BOOL32 *replace_in_new_set)
{
    const UInt16 blocks_per_vfl_set = (mcxt.r_gen.num_of_vfl_blocks >> 1);
    const UInt16 new_set_mapping_offset = new_set_location * blocks_per_vfl_set;
    UInt16 idx;

    for (idx = 0; idx < blocks_per_vfl_set; idx++)
    {
        if (_isVFLCxtBlockMarkedScrub(mcxt.gen.vfl_blocks[new_set_mapping_offset + idx]))
        {
            WMR_PRINT(VFLWARN, "vfl cxt block replacement - found a vfl cxt block in the scrub list (bank: 0x%X, block: 0x%X, idx: 0x%X)\n",
                           mcxt.gen.vfl_blocks[new_set_mapping_offset + idx].bank,
                           mcxt.gen.vfl_blocks[new_set_mapping_offset + idx].block, idx);
            if (*replace_in_new_set)
            {
                WMR_PRINT(ERROR, "more than one block to replace in the vfl cxt\n");
                return FALSE32;
            }
            replace_blocks_array[idx] = TRUE32;
            *replace_in_new_set = TRUE32;
        }
    }

    return TRUE32;
}

static BOOL32
_checkForBankMapPages
    (UInt16 vfl_set)
{
    UInt16 bank, i;
    BOOL32 found_page_maps_from_set = FALSE32;
    
    for (bank = 0; (bank < mcxt.dev.num_of_banks) && (!found_page_maps_from_set); bank++)
    {
        for (i = 0; (i < mcxt.r_gen.pages_per_block_mapping) && (!found_page_maps_from_set); i++)
        {
            const UInt16 this_page_set = ((mcxt.vfl_map_pages[bank][i] & 0x8000) >> 15);
            if (this_page_set == vfl_set)
            {
                found_page_maps_from_set = TRUE32;
            }
        }
    }
    return found_page_maps_from_set;
}


// _copyVFLCxtToOtherPair - Use this function to copy the current vfl cxt from the pair it
// resides in to the other pair (either due to program failure or due to lack of space)
static BOOL32
_copyVFLCxtToOtherPair
    (void)
{
    UInt16 idx, bank;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
    Buffer *buf_mcxt_copy = BUF_Get(BUF_MAIN_AND_SPARE);
    PPNStatusType status;
    BOOL32 replace_blocks[PPNVFL_MAX_INFO_BLOCK_COUNT >> 1] = { FALSE32 };
    BOOL32 replace_in_new_set = FALSE32;
    const UInt16 blocks_per_vfl_set = (mcxt.r_gen.num_of_vfl_blocks >> 1);
    const UInt16 new_set_location = (mcxt.r_gen.next_cxt_block_offset == 1 ? 0 : 1);
    const UInt16 new_set_mapping_offset = new_set_location * blocks_per_vfl_set;

    WMR_PRINT(VFLWARN, "_copyVFLCxtToOtherPair - original cxt set 0x%x, age 0x%X\n",
              mcxt.r_gen.next_cxt_block_offset, mcxt.r_gen.cxt_age);

    if (buf == NULL || buf_mcxt_copy == NULL)
    {
        WMR_PRINT(ERROR, "failed allocating buf\n");
        goto return_error;
    }

    // make a copy of the vfl map pages (case things go bad)
    _copyFromRamToOnNandCxt(buf_mcxt_copy->pData);

    // check if there are replacement in the new pair
    if (!_checkForVFLBlocksInScrubList(new_set_location, replace_blocks, &replace_in_new_set))
    {
        goto return_error;
    }
    
    // check if there are any maps pointing to the set we are about to erase
    if (_checkForBankMapPages(new_set_location))
    {
        goto return_error;
    }
    

    mcxt.r_gen.need_rewrite = FALSE32;
    // erase the set of blocks we want to use
    if (!replace_in_new_set)
    {
        for (idx = 0; idx < blocks_per_vfl_set; idx++)
        {
            status = ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_ERASE,
                                            PPN_NO_OPTIONS,
                                            mcxt.gen.vfl_blocks[new_set_mapping_offset + idx].bank,
                                            mcxt.gen.vfl_blocks[new_set_mapping_offset + idx].block,
                                            0,
                                            TRUE32, NULL, NULL);

            if (status & PPN_ERASE_STATUS_RETIRE)
            {
                WMR_PRINT(QUAL, "failed erase vflcxt block (bank:0x%04X, block: 0x%04X) with retire status\n",
                               mcxt.gen.vfl_blocks[new_set_mapping_offset + idx].bank,
                               mcxt.gen.vfl_blocks[new_set_mapping_offset + idx].block);
                replace_blocks[idx] = TRUE32;
                replace_in_new_set = TRUE32;
            }
            else if (status & PPN_ERASE_STATUS_FAIL)
            {
                WMR_PRINT(QUAL_FATAL, "failed erase vflcxt block (bank:0x%04X, block: 0x%04X) with no retire status\n",
                               mcxt.gen.vfl_blocks[new_set_mapping_offset + idx].bank,
                               mcxt.gen.vfl_blocks[new_set_mapping_offset + idx].block);
                goto return_error;
            }
        }
    }
    mcxt.r_gen.next_cxt_block_offset = new_set_location;
    mcxt.r_gen.next_cxt_page_offset = 0;

    if ((!replace_in_new_set) && (!_programCxtLastPage(buf, FALSE32)))
    {
        WMR_PRINT(VFLWARN, "failed _programCxtLastPage inside _copyVFLCxtToOtherPair()\n");
        if (!_checkForVFLBlocksInScrubList(new_set_location, replace_blocks, &replace_in_new_set))
        {
            goto return_error;
        }
        if (!replace_in_new_set)
        {
            WMR_PRINT(ERROR, "failed _programVFLCxtPage but no blocks in scrub list\n");
            goto return_error;
        }
    }

    for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
    {
        for (idx = 0; ((!replace_in_new_set) &&
                       (idx < mcxt.r_gen.pages_per_block_mapping)); idx++)
        {
            if (!_readVFLCxtPage(buf, bank, idx))
            {
                WMR_PRINT(ERROR, "failed _readVFLCxtPage inside _copyVFLCxtToOtherPair ()\n");
                goto return_error;
            }
            if (!_programVFLCxtPage(buf, bank, idx))
            {
                WMR_PRINT(VFLWARN, "failed _programVFLCxtPage inside _copyVFLCxtToOtherPair ()\n");
                if (!_checkForVFLBlocksInScrubList(new_set_location, replace_blocks, &replace_in_new_set))
                {
                    goto return_error;
                }
                if (!replace_in_new_set)
                {
                    WMR_PRINT(ERROR, "failed _programVFLCxtPage but no blocks in scrub list\n");
                    goto return_error;
                }
            }
        }
    }
    if ((!replace_in_new_set) && (!_programCxtLastPage(buf, FALSE32)))
    {
        WMR_PRINT(VFLWARN, "failed _programCxtLastPage inside _copyVFLCxtToOtherPair()\n");
        if (!_checkForVFLBlocksInScrubList(new_set_location, replace_blocks, &replace_in_new_set))
        {
            goto return_error;
        }
        if (!replace_in_new_set)
        {
            WMR_PRINT(ERROR, "failed _programVFLCxtPage but no blocks in scrub list\n");
            goto return_error;
        }
    }

    if (replace_in_new_set)
    {
        UInt16 replacement_count = 0;
        
        for (idx = 0; idx < blocks_per_vfl_set; idx++)
        {
            if (replace_blocks[idx])
            {
                replacement_count++;
            }
            WMR_ASSERT(replacement_count <= 1); // allow only one block replacement
        }        
        for (idx = 0; idx < blocks_per_vfl_set; idx++)
        {
            if (replace_blocks[idx])
            {
                // revert the map (case the error happen while programming the new set and mapping is off)
                _copyMapPagesFromOnNandCxtToRam(buf_mcxt_copy->pData);

                BUF_Release(buf);
                buf = NULL;
                BUF_Release(buf_mcxt_copy);
                buf_mcxt_copy = NULL;
                // handle errors in working with the new pair
                if (!_replaceErasedCxtBlock((new_set_mapping_offset + idx)))
                {
                    WMR_PRINT(ERROR, "need to replace flag is set but nothing is marked for replacement (idx:0x%X)\n", idx);
                    goto return_error;
                }
                break;
            }
        }
        if (idx == blocks_per_vfl_set)
        {
            WMR_PRINT(ERROR, "need to replace flag is set but nothing is marked for replacement (idx:0x%X)\n", idx);
            goto return_error;
        }
    }

    // check if we need to replace a block from the previous set, if so, replace it here

    if (buf != NULL)
    {
        BUF_Release(buf);
    }
    if (buf_mcxt_copy != NULL)
    {
        BUF_Release(buf_mcxt_copy);
    }
    return TRUE32;

 return_error:
    if (buf != NULL)
    {
        BUF_Release(buf);
    }
    if (buf_mcxt_copy != NULL)
    {
        BUF_Release(buf_mcxt_copy);
    }
    return FALSE32;
}

static BOOL32
_programCxtLastPage
    (Buffer * buf,
    BOOL32 retry)
{
    ProgramVFLCxtPageWrapStatus program_status;

    do
    {
        _copyFromRamToOnNandCxt(buf->pData);
        program_status = _programVFLCxtPageWrap(buf, PPNVFL_CXT_LAST_BANKIDX, PPNVFL_CXT_LAST_BANKIDX);
    }
    while ((program_status == VFLCxtProgramRetry) && retry);

    return (program_status != VFLCxtProgramDone) ? FALSE32 : TRUE32;
}

static BOOL32
_setNumOfFTLSuBlks
    (UInt16 num_of_ftl_sublk)
{
    BOOL32 res;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);

    WMR_PRINT(INF, "_setNumOfFTLSuBlks (0x%04X)\n", num_of_ftl_sublk);
    if (num_of_ftl_sublk > mcxt.gen.num_of_vfl_sublk)
    {
        WMR_PANIC("[PPNVFL:WRN] failed setting the number of ftl blocks "
                  "num_of_vfl_sublk:0x%04X < num_of_ftl_sublk:0x%04X\n",
                  num_of_ftl_sublk, mcxt.gen.num_of_vfl_sublk);
    }
    mcxt.gen.num_of_ftl_sublk = num_of_ftl_sublk;
    mcxt.gen.exported_lba_no = 0xffffffff;
    res = _programCxtLastPage(buf, TRUE32);
    BUF_Release(buf);

    return res;
}

static BOOL32
_setNumOfExportedLbas
    (UInt32 num_of_exported_lbas)
{
    BOOL32 res;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);

    mcxt.gen.exported_lba_no = num_of_exported_lbas;

    res = _programCxtLastPage(buf, TRUE32);
    BUF_Release(buf);

    return res;
}

static BOOL32
_setL2vPoolSize
    (UInt32 exported_l2v_pool_size)
{
    BOOL32 res;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);

    mcxt.gen.exported_l2v_pool_size = exported_l2v_pool_size;

    res = _programCxtLastPage(buf, TRUE32);
    BUF_Release(buf);

    return res;
}

/* vfl cxt management functions - end */

#endif // AND_READONLY

/* vfl I/F implementation - start */
/*
 * Name: _ppnvflInit
 * Description: this function initialize the memory and structures for ppnvfl
 * Input: fpart function table
 * Return value: ANDErrorCodeOk if successfull
 */
static Int32
_ppnvflInit
    (FPartFunctions * pFPartFunctions)
{
    UInt32 i;
    UInt16 pages_per_vfl_cxt_copy; // including the cxt and mapping

    if (mcxt.initialized)
    {
        return ANDErrorCodeOk;
    }

    // Format checks:
    WMR_ASSERT(sizeof(PPNVFL_CxtSpare) == 16);
    WMR_ASSERT(WMR_OFFSETOF(PPNVFL_GeneralCxt, vfl_blocks) == 256);

    mcxt.fpart = pFPartFunctions;
    mcxt.fil = mcxt.fpart->GetLowFuncTbl();

    ppnMiscFillDevStruct(&mcxt.dev, mcxt.fil);
    mcxt.single_command = (PPNCommandStruct*)mcxt.fpart->GetSingleCommandStruct();

    // allocating the memory for the CAUs structures
    // this code may later on move to open
    if (mcxt.dev.num_channels == 0 || mcxt.dev.ces_per_channel == 0 || mcxt.dev.caus_per_ce == 0)
    {
        WMR_PRINT(ERROR, "_ppnvflInit num_channels 0x%X ces_per_channel 0x%X caus_per_ce 0x%X\n",
                       mcxt.dev.num_channels, mcxt.dev.ces_per_channel, mcxt.dev.caus_per_ce);
    }

    mcxt.r_gen.pages_per_block_mapping = 1;
    i = mcxt.dev.blocks_per_cau * sizeof(P2UsageType);
    while (i > mcxt.dev.main_bytes_per_page)
    {
        i -= mcxt.dev.main_bytes_per_page;
        mcxt.r_gen.pages_per_block_mapping++;
    }
    mcxt.r_gen.pbns_per_cxt_mapping_page = mcxt.dev.main_bytes_per_page / sizeof(P2UsageType);

    // calculate vfl cxt on nand - depth, redundancy and pages per set
    mcxt.r_gen.vfl_blocks_redundancy = 4;
    // make sure that we have at least twice as many entries as needed for a single copy of vflcxt and
    // all the mapping
    pages_per_vfl_cxt_copy = 1 + (mcxt.r_gen.pages_per_block_mapping * mcxt.dev.num_of_banks);
    mcxt.r_gen.vfl_blocks_depth = 1;
    while ((UInt16)(mcxt.r_gen.vfl_blocks_depth * (UInt16)mcxt.dev.pages_per_block_slc) <
           (UInt32)(pages_per_vfl_cxt_copy * PPNVFL_NUM_OF_VFL_CXT_BLOCK_SETS))
    {
        mcxt.r_gen.vfl_blocks_depth++;
    }
    mcxt.r_gen.vfl_pages_per_set = (mcxt.r_gen.vfl_blocks_depth * mcxt.dev.pages_per_block_slc);
    mcxt.r_gen.num_of_vfl_blocks = (mcxt.r_gen.vfl_blocks_depth * mcxt.r_gen.vfl_blocks_redundancy *
                                     PPNVFL_NUM_OF_VFL_CXT_BLOCK_SETS);

    mcxt.gen.num_of_vfl_sublk = _calcNumOfVFLSuBlk();
    mcxt.gen.num_of_ftl_sublk = mcxt.gen.num_of_vfl_sublk;
    mcxt.gen.exported_lba_no = 0xffffffff;
    mcxt.gen.exported_l2v_pool_size = 0xffffffff;
    mcxt.r_gen.cxt_age = 0xFFFFFFFF;
    mcxt.r_gen.bytes_per_vbn_bitmap = (mcxt.dev.num_of_banks >> 3) + (mcxt.dev.num_of_banks % 8 ? 1 : 0);
    mcxt.v2p_bitmap_size = mcxt.r_gen.bytes_per_vbn_bitmap * mcxt.gen.num_of_vfl_sublk;
    mcxt.vfl_map_pages = (UInt16**)WMR_MALLOC(sizeof(UInt16*) * mcxt.dev.num_of_banks);
    mcxt.vfl_map_pages[0] = (UInt16*)WMR_MALLOC(sizeof(UInt16) * mcxt.dev.num_of_banks *
                                                 mcxt.r_gen.pages_per_block_mapping);
    for (i = 1; i < mcxt.dev.num_of_banks; i++)
    {
        mcxt.vfl_map_pages[i] = &mcxt.vfl_map_pages[0][mcxt.r_gen.pages_per_block_mapping * i];
    }
    mcxt.r_banks = (PPNVFL_BankRAMCxt*)WMR_MALLOC(sizeof(PPNVFL_BankRAMCxt) * mcxt.dev.num_of_banks);

    mcxt.v2p_bitmap = (UInt8*)WMR_MALLOC(mcxt.v2p_bitmap_size); // bit per block
    WMR_MEMSET(mcxt.v2p_bitmap, 0, mcxt.v2p_bitmap_size); // mark all blocks as bad
    mcxt.v2p_scrub_bitmap = (UInt8*)WMR_MALLOC(mcxt.v2p_bitmap_size); // bit per block
    WMR_MEMSET(mcxt.v2p_scrub_bitmap, 0, mcxt.v2p_bitmap_size); // mark all blocks as bad

    if (mcxt.dev.ces_per_channel > 1)
    {
        mcxt.reorder = (PPNReorderStruct*)WMR_MALLOC(sizeof(PPNReorderStruct));
    }
    else
    {
        mcxt.reorder = NULL;
    }

    // allocate memory for the commands
    WMR_ASSERT(mcxt.dev.num_of_banks <= PPNVFL_MAX_SUPPORTED_BANKS);
    WMR_BufZone_Init(&mcxt.bufzone);
    for (i = 0; i < mcxt.dev.num_channels; i++)
    {
        mcxt.commands[i] = (PPNCommandStruct *)WMR_Buf_Alloc_ForDMA(&mcxt.bufzone, sizeof(PPNCommandStruct));
    }
    WMR_BufZone_FinishedAllocs(&mcxt.bufzone);

    for (i = 0; i < mcxt.dev.num_channels; i++)
    {
        WMR_BufZone_Rebase(&mcxt.bufzone, (void **)&mcxt.commands[i]);
        WMR_ASSERT(mcxt.commands[i] != NULL);
    }
    WMR_BufZone_FinishedRebases(&mcxt.bufzone);

#ifdef AND_COLLECT_STATISTICS
    WMR_MEMSET(&mcxt.stat, 0, sizeof(SVFLStatistics));
#endif

    mcxt.initialized = TRUE32;
    return ANDErrorCodeOk;
}

static BOOL32
_isVFLCxtBlock
    (UInt16 bank,
    UInt16 block)
{
    UInt16 vfl_info_idx;
    const UInt8 bank_uint8 = (UInt8) bank;

    for (vfl_info_idx = 0; vfl_info_idx < mcxt.r_gen.num_of_vfl_blocks; vfl_info_idx++)
    {
        if ((mcxt.gen.vfl_blocks[vfl_info_idx].bank == bank_uint8) &&
            (mcxt.gen.vfl_blocks[vfl_info_idx].block == block))
        {
            return TRUE32;
        }
    }
    return FALSE32;
}

/*
 * Name: _ppnvflOpen
 * Description: reads the vfl structures from nand and build the ram version
 * Input:
 * Return value: ANDErrorCodeOk if successful.
 */

static Int32
_ppnvflOpen
    (UInt32 dwKeepout,
    UInt32 minor_ver,
    UInt32 dwOptions)
{
    Buffer * buf = NULL;
    UInt16 bank, j;
    SpecialBlockAddress *special_blocks = NULL;
    UInt16 *special_blocks_types = NULL;
    UInt16 total_special_blocks = 0, total_cxt_blocks = 0, total_nand_boot_blocks = 0;
    Int32 ret_val = ANDErrorCodeHwErr;
#if WMR_EFI
    const UInt8 num_sectors = (mcxt.dev.main_bytes_per_page) / 1024;
#endif

    buf = BUF_Get(BUF_MAIN_AND_SPARE);
    // populate the General and CAU structures

    special_blocks = (SpecialBlockAddress *)WMR_MALLOC(AND_MAX_SPECIAL_BLOCKS *
                                                       sizeof(SpecialBlockAddress));
    special_blocks_types = (UInt16*)WMR_MALLOC(AND_MAX_SPECIAL_BLOCKS * sizeof(UInt16));

    WMR_MEMSET(mcxt.v2p_bitmap, 0, mcxt.v2p_bitmap_size); // mark all blocks as bad
    mcxt.blocks_to_scrub_count = 0;
    WMR_MEMSET(mcxt.v2p_scrub_bitmap, 0, mcxt.v2p_bitmap_size); // mark all blocks as bad

    mcxt.r_gen.cxt_age = 0xFFFFFFFF;
    mcxt.r_gen.keepout = dwKeepout;
    mcxt.r_gen.options = dwOptions;

    WMR_MEMSET(mcxt.vfl_map_pages[0], 0, (mcxt.dev.num_of_banks *
                                           mcxt.r_gen.pages_per_block_mapping * sizeof(UInt16)));
    WMR_MEMSET(mcxt.r_banks, 0, (sizeof(PPNVFL_BankRAMCxt) * mcxt.dev.num_of_banks));

    mcxt.r_gen.search_for_cxt = TRUE32;

    if (_findOnNANDCxt() == FALSE32)
    {
        WMR_PRINT(ERROR, "Error _findOnNANDCxt()\n");
        BUF_Release(buf);
        return ANDErrorCodeHwErr;
    }

    if (!mcxt.r_gen.need_rewrite)
    {
        mcxt.r_gen.search_for_cxt = FALSE32;
    }

    for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
    {
        // read the mapping and fill the v2p maps
        for (j = 0; j < mcxt.r_gen.pages_per_block_mapping; j++)
        {
            UInt16 block, startOffset, count;
            P2UsageType *p2_block_usage_map = (P2UsageType*)buf->pData;
            if (_readVFLCxtPage(buf, bank, j) == FALSE32)
            {
                WMR_PRINT(ERROR, "Error _readVFLCxtPage(bank=%d, %d)\n", bank, j);
                BUF_Release(buf);
                return ANDErrorCodeHwErr;
            }
            startOffset = j * (mcxt.r_gen.pbns_per_cxt_mapping_page);
            count = WMR_MIN((mcxt.dev.blocks_per_cau - startOffset),
                            mcxt.r_gen.pbns_per_cxt_mapping_page);
            for (block = 0; block < count; block++)
            {
                UInt16 pbn = block + startOffset;
                if (!(p2_block_usage_map[block] & AND_SB_BIT_SPEICAL_FLAG))
                {
                    mcxt.r_banks[bank].num_p2v++;
                    _setBlockAsVbnInBitmap(bank, pbn);
                    if (p2_block_usage_map[block] & PPNVFL_P2V_SCRUB_BIT)
                    {
                        _markBlockToScrubInBitmap(bank, pbn);
                    }
                }
                // count bad blocks (initial and grown and special blocks)
                else
                {
                    switch (p2_block_usage_map[block])
                    {
                        case PPNVFL_TYPE_AVAILABLE_MARK:
                            mcxt.r_banks[bank].num_available++;
                            break;

                        case PPNVFL_TYPE_GROWN_BB:
                            mcxt.r_banks[bank].num_grown_bad++;
                            break;

                        case PPNVFL_TYPE_INITIAL_BB:
                            mcxt.r_banks[bank].num_initial_bad++;
                            break;

                        case PPNVFL_TYPE_VFL_INFO:
                            mcxt.r_banks[bank].num_cxt++;
                            WMR_ASSERT(_isVFLCxtBlock(bank, pbn));
                            total_cxt_blocks++;
                            break;

                        default:
                        {
                            if (andSBGetHandler(p2_block_usage_map[block]) != AND_SB_HANDLER_VFL_CXT)
                            {
                                if (AND_MAX_SPECIAL_BLOCKS <= total_special_blocks)
                                {
                                    WMR_PRINT(ERROR, "total special blocks has exceeded the max allowed\n");
                                    ret_val = ANDErrorCodeHwErr;
                                    goto exit;
                                }
                                mcxt.r_banks[bank].num_special_blocks++;
                                special_blocks[total_special_blocks].bank = bank;
                                special_blocks[total_special_blocks].block = pbn;
                                special_blocks_types[total_special_blocks] = p2_block_usage_map[block];
                                if(special_blocks_types[total_special_blocks] == AND_SB_TYPE_NAND_BOOT)
                                {
                                    total_nand_boot_blocks++;
                                }
                                total_special_blocks++;
                                break;
                            }
                            else
                            {
                                WMR_PRINT(ERROR, "unknown block marking: 0x%08x\n", p2_block_usage_map[block]);
                                ret_val = ANDErrorCodeHwErr;
                                goto exit;
                            }
                        }
                    }
                }
            }
        }
    }

    WMR_ASSERT(total_cxt_blocks == mcxt.r_gen.num_of_vfl_blocks);
    mcxt.r_gen.cxt_age--;
    if((BOOT_BLOCK_ALLOCATION_COUNT(&mcxt)) && (total_nand_boot_blocks > BOOT_BLOCK_ALLOCATION_COUNT(&mcxt)))
    {
        WMR_PRINT(ALWAYS, "!!!!!! total nand boot blocks exceeded max allowed %d > %d !!!!!! \n", total_nand_boot_blocks, BOOT_BLOCK_ALLOCATION_COUNT(&mcxt));
    }

    if (total_special_blocks != 0)
    {
        UInt16 sb_idx;

        mcxt.fpart->ChangeCacheState(FALSE32);
        // send the list to FPart
        for (sb_idx = 0; sb_idx < total_special_blocks; sb_idx++)
        {
            if (!mcxt.fpart->AllocateSpecialBlockType(&special_blocks[sb_idx], 1, special_blocks_types[sb_idx]))
            {
                WMR_PRINT(ERROR, "fpart failed to allocate special block with type %p\n",
                               special_blocks_types[sb_idx]);
                ret_val = ANDErrorCodeHwErr;
                goto exit;
            }
        }
        mcxt.fpart->ChangeCacheState(TRUE32);
    }

    ret_val = ANDErrorCodeOk;

    _printVFLBlockMapping(__LINE__);
#ifndef AND_READONLY
    if (mcxt.r_gen.need_rewrite)
    {
        if (!_copyVFLCxtToOtherPair())
        {
            // we failed to copy the vfl cxt we have not functional cxt
            WMR_PRINT(ERROR, "failed _copyVFLCxtToOtherPair after need_rewrite = TRUE\n");
            ret_val = ANDErrorCodeHwErr;
            goto exit;             
        }
        mcxt.r_gen.search_for_cxt = FALSE32;
        _printVFLBlockMapping(__LINE__);
    }
#endif

#if WMR_EFI
    //Bitflip info is always in the first few bytes on the bus - So need to check if those were data or meta

    if(!FIL_GetStruct(AND_STRUCT_FIL_NAND_LAYOUT, &p_layout, &p_layout_size))
    {
        WMR_PRINT(ERROR, "Could not get NAND layout for sector stats manipulation\n");
        ret_val = ANDErrorCodeHwErr;
        goto exit;
    }
    if(p_layout.dwMetaSegmentIndex == 0) //meta first, then data
    {
        //bitflips overflow from meta into data
        if(num_sectors > p_layout.pastrSegments[0].dwLength) 
        {
            stats_copy_from_meta = p_layout.pastrSegments[0].dwLength;
            stats_copy_from_data = num_sectors - stats_copy_from_meta;
            stats_offset_from_meta = 0;
            stats_offset_from_data = stats_copy_from_meta;
        }
        else
        {
            stats_copy_from_meta = num_sectors;
        }
    }
    else //data first, then meta. Assume that there are no more than 'sector size' number of sectors
    {
        stats_copy_from_data = num_sectors;
    }
#endif // WMR_EFI

 exit:
    if (NULL != buf)
    {
        BUF_Release(buf);
    }
    if (NULL != special_blocks)
    {
        WMR_FREE(special_blocks, (AND_MAX_SPECIAL_BLOCKS * sizeof(SpecialBlockAddress)));
    }
    if (NULL != special_blocks_types)
    {
        WMR_FREE(special_blocks_types, (AND_MAX_SPECIAL_BLOCKS * sizeof(UInt16)));
    }

    mcxt.r_gen.active = FALSE32;

    return ret_val;
}

#ifndef AND_READONLY

/* this function fills the vfl cxt block list */
static BOOL32
_fillVFLCxtBlockList
    (P2UsageType **on_nand_block_map)
{
    UInt16 vfl_info_idx, pbn, bank;

    pbn = mcxt.dev.blocks_per_cau - 1;

    for (vfl_info_idx = 0; ((vfl_info_idx < mcxt.r_gen.num_of_vfl_blocks) &&
                            (pbn != 0)); pbn--)
    {
        for (bank = 0; ((bank < mcxt.dev.num_of_banks) &&
                        (vfl_info_idx < mcxt.r_gen.num_of_vfl_blocks)); bank++)
        {
            if (on_nand_block_map[bank][pbn] == PPNVFL_TYPE_AVAILABLE_MARK)
            {
                if (ppnMiscTestSpecialBlock(&mcxt.dev, mcxt.commands[0], bank, pbn, PPNVFL_SPARE_TYPE_TST))
                {
                    on_nand_block_map[bank][pbn] = PPNVFL_TYPE_VFL_INFO;
                    mcxt.gen.vfl_blocks[vfl_info_idx].bank = bank;
                    mcxt.gen.vfl_blocks[vfl_info_idx].block = pbn;
                    mcxt.gen.vfl_blocks[vfl_info_idx].features = 0;
                    vfl_info_idx++;
                }
                else
                {
                    on_nand_block_map[bank][pbn] = PPNVFL_TYPE_GROWN_BB;
                }
            }
        }
    }

    if ((vfl_info_idx < mcxt.r_gen.num_of_vfl_blocks) || (pbn == 0))
    {
        WMR_PRINT(ERROR, "failed allocating vfl info blocks \n");
        goto return_error;
    }

    return TRUE32;

 return_error:
    return FALSE32;
}

/*
 * Name: _calcFormatNumOfVFLUserBlks
 * Return value: number of VFL virtual blocks for this media
 */
static UInt32
_calcFormatNumOfVFLUserBlks
(void)
{
    UInt32 shift, result, above, under, closest_power_of_two;

    shift = WMR_LOG2(mcxt.dev.blocks_per_cau);    

    above = mcxt.dev.blocks_per_cau - (1 << shift);
    under = (1 << (shift + 1)) - mcxt.dev.blocks_per_cau;
    
    if (under < above)
    {
        shift = shift + 1;
    }
    closest_power_of_two = (1 << shift);
    result = mcxt.dev.num_of_banks * PPNVFL_USER_BLKS_PER_256B * (closest_power_of_two >> 8);
    return result;
}

static BOOL32
_programFormatVFLCxt
    (P2UsageType **on_nand_block_map,
    Buffer *buf)
{
    UInt16 bank, i, vfl_info_idx;
    PPNStatusType status;

    for (vfl_info_idx = 0; vfl_info_idx < mcxt.r_gen.num_of_vfl_blocks; vfl_info_idx++)
    {
        status = ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_ERASE,
                                        PPN_NO_OPTIONS,
                                        mcxt.gen.vfl_blocks[vfl_info_idx].bank,
                                        mcxt.gen.vfl_blocks[vfl_info_idx].block,
                                        0, TRUE32, NULL, NULL);
        if (status & PPN_ERASE_STATUS_FAIL)
        {
            // we should not get here - vfl blocks were tested
            _setVFLCxtScrubMark(mcxt.gen.vfl_blocks[vfl_info_idx]);
            WMR_PRINT(QUAL_FATAL, "format fail erasing info block fail!\n");
            goto return_error;
        }
    }

    mcxt.r_gen.next_cxt_block_offset = 0;
    mcxt.r_gen.next_cxt_page_offset = 0;
    mcxt.r_gen.need_rewrite = FALSE32;

    if (!_programCxtLastPage(buf, TRUE32))
    {
        WMR_PRINT(VFLWARN, "_ppnvflFormat - failed _programCxtLastPage()\n");
        goto return_error;
    }

    for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
    {
        for (i = 0; i < mcxt.r_gen.pages_per_block_mapping; i++)
        {
            UInt16 start_offset = i * mcxt.r_gen.pbns_per_cxt_mapping_page;
            UInt16 size_to_copy = WMR_MIN(mcxt.dev.main_bytes_per_page,
                                          ((mcxt.dev.blocks_per_cau - start_offset) * sizeof(P2UsageType)));
            WMR_MEMSET(buf->pData, 0xFF, mcxt.dev.main_bytes_per_page);
            WMR_MEMCPY(buf->pData, &on_nand_block_map[bank][start_offset], size_to_copy);
            if (!_programVFLCxtPage(buf, bank, i))
            {
                WMR_PRINT(VFLWARN, "_ppnvflFormat - failed _programCxtLastPage()\n");
                goto return_error;
            }
        }
    }

    if (!_programCxtLastPage(buf, TRUE32))
    {
        WMR_PRINT(VFLWARN, "_ppnvflFormat - failed _programCxtLastPage()\n");
        goto return_error;
    }

    return TRUE32;

 return_error:

    // mark the good vfl blocks as available
    // mark the grown bad as grown bad    
    for (vfl_info_idx = 0; vfl_info_idx < mcxt.r_gen.num_of_vfl_blocks; vfl_info_idx++)
    {
        const UInt8 curr_bank = mcxt.gen.vfl_blocks[vfl_info_idx].bank;
        const UInt16 curr_block = mcxt.gen.vfl_blocks[vfl_info_idx].block;

        if (_isVFLCxtBlockMarkedScrub(mcxt.gen.vfl_blocks[vfl_info_idx]))
        {
            on_nand_block_map[curr_bank][curr_block] = PPNVFL_TYPE_GROWN_BB;
            _unsetVFLCxtScrubMark(mcxt.gen.vfl_blocks[vfl_info_idx]);
            ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_ERASE,
                                            PPN_OPTIONS_IGNORE_BAD_BLOCK, curr_bank, curr_block,
                                            0, TRUE32, NULL, NULL);
        }
        else
        {
            on_nand_block_map[curr_bank][curr_block] = PPNVFL_TYPE_AVAILABLE_MARK;
        }
    }

    return FALSE32;
}

/*
 * Name: _ppnvflFormat
 * Description: implements format function for the vfl I/F
 * Input:
 * Return value: ANDErrorCodeOk if successful.
 */
static Int32
_ppnvflFormat
    (UInt32 dwKeepout,
    UInt32 dwOptions)
{
    UInt16 bank, pbn, i, pre_mapped_special_blocks;
    Buffer *buf = NULL;
    P2UsageType **on_nand_block_map = NULL;
    PPNStatusType status;
    UInt16 sb_allocation_count = 0;
    UInt16 *special_blocks_types = NULL;
    SpecialBlockAddress *special_blocks = NULL;
    UInt32 sizeof_special_blocks_types = 0, sizeof_special_blocks = 0, vbn_allocation_count = 0;
    const UInt32 min_num_of_user_blocks = _calcFormatNumOfVFLUserBlks(); // minimum number of physical blocks that
                                                                         // needs to be available for ftls
    UInt32 sb_pre_alloc;


    // Init the general structure
    mcxt.r_gen.cxt_age = 0xFFFFFFFF;
    mcxt.r_gen.keepout = dwKeepout;
    mcxt.r_gen.options = dwOptions;
    mcxt.gen.ftl_type = 0xFFFFFFFF;
    mcxt.gen.exported_lba_no = 0xFFFFFFFF;
    mcxt.gen.exported_l2v_pool_size = 0xffffffff;

    WMR_PRINT(ALWAYS, "dwKeepout:%d, dwOptions:0x%X\n", dwKeepout, dwOptions);
    
    // before we start - make sure the PPN configuration here make sense to this vfl
    if (mcxt.dev.ces_per_channel > PPN_MAX_CES_PER_BUS)
    {
        WMR_PRINT(ERROR, "PPNVFL does not support configurations with more than %d CEs/Channel (found %d)!\n",
                       PPN_MAX_CES_PER_BUS, mcxt.dev.ces_per_channel);
        goto return_error;
    }
    if (mcxt.dev.main_bytes_per_page < PPNVFL_MIN_PAGE_SIZE)
    {
        WMR_PRINT(ERROR, "PPNVFL does not support configurations with less than %d bytes per page (found %d)!\n",
                       PPNVFL_MIN_PAGE_SIZE, mcxt.dev.main_bytes_per_page);
        goto return_error;
    }
    if (mcxt.dev.num_of_banks > PPNVFL_MAX_SUPPORTED_BANKS)
    {
        WMR_PRINT(ERROR, "PPNVFL does not support configurations with more than %d banks (found %d)!\n",
                       PPNVFL_MAX_SUPPORTED_BANKS, mcxt.dev.num_of_banks);
        goto return_error;
    }

    buf = BUF_Get(BUF_MAIN_AND_SPARE);
    if (buf == NULL)
    {
        WMR_PRINT(ERROR, "BUF_Get fail!\n");
        return ANDErrorCodeHwErr;
    }

    // start - 1. allocate memory to hold phisycal to virtual mapping
    on_nand_block_map = (P2UsageType**)WMR_MALLOC(sizeof(P2UsageType*) * mcxt.dev.num_of_banks);
    if (on_nand_block_map == NULL)
    {
        WMR_PRINT(ERROR, "allocation fail!\n");
        goto return_error;
    }
    on_nand_block_map[0] = (P2UsageType*)WMR_MALLOC(mcxt.dev.blocks_per_cau * mcxt.dev.num_of_banks *
                                                    sizeof(P2UsageType));
    if (on_nand_block_map[0] == NULL)
    {
        WMR_PRINT(ERROR, "allocation fail!\n");
        goto return_error;
    }
    // assign the rest of the pointers
    for (bank = 1; bank < mcxt.dev.num_of_banks; bank++)
    {
        on_nand_block_map[bank] = &(on_nand_block_map[0][(bank * mcxt.dev.blocks_per_cau)]);
    }
    // end   - 1. allocate memory to hold phisycal to virtual mapping - end

    // start - 2. read the bbts
    for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
    {
        status = ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_CAUBBT,
                                        PPN_NO_OPTIONS, bank, 0, 0, TRUE32, buf->pData, NULL);
        if (status & PPN_OTHERS_STATUS_OP_FAILED)
        {
            WMR_PRINT(ERROR, "ReadCAUBBT fail!\n");
            goto return_error;
        }

        for (pbn = 0; pbn < mcxt.dev.blocks_per_cau; pbn++)
        {
            if (ppnMiscIsBlockGood(buf->pData, pbn) == FALSE32)
            {
                on_nand_block_map[bank][pbn] = PPNVFL_TYPE_INITIAL_BB;
                WMR_PRINT(ALWAYS, "Initial Bad Block bank: 0x%04x block 0x%04x\n", bank, pbn);
            }
            else
            {
                on_nand_block_map[bank][pbn] = PPNVFL_TYPE_AVAILABLE_MARK;
            }
        }
    }
    // end   - 2. read the bbts

    // start - 3. mark special blocks
    // start - 3.1 mark pre allocated special blocks
    if (mcxt.fpart->MapSpecialBlocks(NULL, NULL, &pre_mapped_special_blocks) == TRUE32)
    {
        sizeof_special_blocks_types = pre_mapped_special_blocks * sizeof(UInt16);
        sizeof_special_blocks = pre_mapped_special_blocks * sizeof(SpecialBlockAddress);
        
        special_blocks_types = (UInt16*)WMR_MALLOC(sizeof_special_blocks_types);
        special_blocks = (SpecialBlockAddress*)WMR_MALLOC(sizeof_special_blocks);

        if (mcxt.fpart->MapSpecialBlocks(special_blocks, special_blocks_types,
                                         &pre_mapped_special_blocks) == FALSE32)
        {
            WMR_PRINT(ERROR, "MapSpecialBlocks fail after success??? \n");
            goto return_error;
        }
        for (i = 0; i < pre_mapped_special_blocks; i++)
        {
            bank = special_blocks[i].bank;
            pbn = special_blocks[i].block;
            on_nand_block_map[bank][pbn] = special_blocks_types[i];
        }
    }
    // end   - 3.1 mark pre allocated special blocks

    // start - 3.2 mark block zero
    // start - 3.3 mark nand boot blocks
    for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
    {
        if (ppnMiscGetCAUFromBank(&mcxt.dev, bank) == 0)
        {
            on_nand_block_map[bank][0] = AND_SB_TYPE_BLOCK_ZERO;
            // support for legacy nand-boot area
            if (0 == (WMR_INIT_USE_KEEPOUT_AS_BORROW & mcxt.r_gen.options))
            {
                for (pbn = 0; pbn < mcxt.r_gen.keepout; pbn++)
                {
                    if (on_nand_block_map[bank][pbn] == PPNVFL_TYPE_AVAILABLE_MARK)
                    {
                        on_nand_block_map[bank][pbn] = AND_SB_TYPE_KEEPOUT_BOOT;
                    }
                }
            }
        }
    }

    // end   - 3.2 mark block zero
    // end   - 3.3 mark nand boot blocks

    // start - 3.4 mark vfl context blocks
    WMR_MEMSET(mcxt.gen.vfl_blocks, 0xFF, (sizeof(mcxt.gen.vfl_blocks[0]) * mcxt.r_gen.num_of_vfl_blocks));
    if (!_fillVFLCxtBlockList(on_nand_block_map))
    {
        goto return_error;
    }
    // end   - 3.4 mark vfl context blocks

    // start - 3.5 generate a bar for the perfect striping (make sure we put special blocks on top)

    sb_allocation_count = 0;
    sb_pre_alloc = SPECIAL_BLOCK_PRE_ALLOCATION_COUNT(&mcxt);
    for (pbn = mcxt.dev.blocks_per_cau - 1;
         ((pbn >= PPN_SPECIAL_BLOCK_LIMIT(&mcxt.dev)) && (sb_allocation_count < sb_pre_alloc));
         pbn--)
    {
        for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
        {
            if (on_nand_block_map[bank][pbn] == PPNVFL_TYPE_AVAILABLE_MARK)
            {
                if (sb_pre_alloc > sb_allocation_count)
                {
                    sb_allocation_count++;
                }
                else
                {
                    // we have enough sb, allocate rest of stripe for vbn
                    on_nand_block_map[bank][pbn] = PPNVFL_TYPE_VBN;
                    vbn_allocation_count++;
                }
            }
        }
    }
    
    if (pbn < PPN_SPECIAL_BLOCK_LIMIT(&mcxt.dev))
    {
        WMR_PRINT(ERROR, "_ppnvflFormat - unable to reserve special block stripe - 0x%X\n", pbn);
        goto return_error;
    }

    mcxt.gen.num_of_vfl_sublk = pbn + 1;
    mcxt.gen.num_of_ftl_sublk = pbn + 1;
    // end   - 3.5 generate a bar for the perfect striping (make sure we put special blocks on top)

    // end   - 3. mark special blocks

    // start - 4. allocate virtual blocks - start
    // start - 4.1. pick the best aligned blocks first

    for (pbn = 1; pbn < mcxt.gen.num_of_vfl_sublk; pbn++)
    {
        for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
        {
            if (on_nand_block_map[bank][pbn] == PPNVFL_TYPE_AVAILABLE_MARK)
            {
                on_nand_block_map[bank][pbn] = PPNVFL_TYPE_VBN;
                vbn_allocation_count++;
            }
        }
    }

    if (PPNVFL_TYPE_VBN == on_nand_block_map[mcxt.dev.num_of_banks - 1][pbn])
    {
        mcxt.gen.num_of_vfl_sublk = pbn + 1;
        mcxt.gen.num_of_ftl_sublk = pbn + 1;
    }

    if (vbn_allocation_count < min_num_of_user_blocks)
    {
        WMR_PRINT(ERROR, "_ppnvflFormat - unable to reserve enough blocks "
                         "for ftl - actual 0x%X, expected 0x%X\n",
                       vbn_allocation_count, min_num_of_user_blocks);
        goto return_error;
    }

    // end - 4.1. pick the best aligned blocks first
    // end   - 4. allocate virtual blocks - end

    // start - 5. program the context blocks
    // start - 5.1. erase all the vfl context blocks
    // start - 5.2. program the cxt blocks
    while (!_programFormatVFLCxt(on_nand_block_map, buf))
    {
        WMR_PRINT(VFLWARN, "_programFormatVFLCxt failed, reallocating vflcxt blocks\n");

        if (!_fillVFLCxtBlockList(on_nand_block_map))
        {
            goto return_error;
        }
    }
    // end   - 5.1. erase all the vfl context blocks
    // end   - 5.2. program the cxt blocks
    // end   - 5. program the context blocks

    // start - 6. clean up the mess
    // free temp allocations done in this function
    WMR_FREE(on_nand_block_map[0], mcxt.dev.blocks_per_cau * mcxt.dev.num_of_banks * sizeof(P2UsageType));
    WMR_FREE(on_nand_block_map, (sizeof(P2UsageType*) * mcxt.dev.num_of_banks));
    
    if (special_blocks_types != NULL)
    {
        WMR_FREE(special_blocks_types, sizeof_special_blocks_types);
    }
    
    if (special_blocks != NULL)
    {
        WMR_FREE(special_blocks, sizeof_special_blocks);
    }
    
    BUF_Release(buf);
    // end   - 6. clean up the mess

    return ANDErrorCodeOk;

 return_error:

    // start - 6. clean up the mess
    // free temp allocations done in this function
    if (on_nand_block_map != NULL && on_nand_block_map[0] != NULL)
    {
        WMR_FREE(on_nand_block_map[0], mcxt.dev.blocks_per_cau * mcxt.dev.num_of_banks * sizeof(P2UsageType));
    }
    
    if (on_nand_block_map != NULL)
    {
        WMR_FREE(on_nand_block_map, (sizeof(P2UsageType*) * mcxt.dev.num_of_banks));
    }

    if (special_blocks_types != NULL)
    {
        WMR_FREE(special_blocks_types, sizeof_special_blocks_types);
    }
    
    if (special_blocks != NULL)
    {
        WMR_FREE(special_blocks, sizeof_special_blocks);
    }

    BUF_Release(buf);
    // end   - 6. clean up the mess

    return ANDErrorCodeHwErr;
}

#endif // !AND_READONLY

/*
 * Name: _bitSwapBuffer
 * Description: swaps the order of the bits on every byte in the given buffer
 * Input: buffer and length, no alignment requirement
 * Output: bit-swapped buffer
 */
static void
_bitSwapBuffer
    (void *buffer,
    UInt32 length)
{
    UInt8 *cursor = (UInt8*)buffer;

    while (length--)
    {
        UInt8 byte = *cursor;
        *cursor = ((byte & 0x01) << 7) |
                  ((byte & 0x02) << 5) |
                  ((byte & 0x04) << 3) |
                  ((byte & 0x08) << 1) |
                  ((byte & 0x10) >> 1) |
                  ((byte & 0x20) >> 3) |
                  ((byte & 0x40) >> 5) |
                  ((byte & 0x80) >> 7);
        cursor++;
    }
}

/*
 * Name: _ppnvflGetStruct
 * Description: implements GetStruct function for the vfl I/F
 * Input: struct type
 * Output: buffer, buffer size
 * Return value: TRUE32 if successful, otherwise - FALSE32
 */
static BOOL32
_ppnvflGetStruct
    (UInt32 dwStructType,
    void *pvoidStructBuffer,
    UInt32 *pdwStructSize)
{
    BOOL32 boolRes = FALSE32;

    switch (dwStructType & AND_STRUCT_INDEX_MASK)
    {
        case AND_STRUCT_VFL_GET_TYPE:
        {
            UInt8 bVFLType = VFL_TYPE_SWISSPPNVFL;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &bVFLType,
                                      sizeof(bVFLType));
            break;
        }

#ifdef AND_COLLECT_STATISTICS
        case AND_STRUCT_VFL_STATISTICS_SIZE:
        {
            UInt32 dwStatSize = sizeof(SVFLStatistics);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwStatSize,
                                      sizeof(dwStatSize));
            break;
        }

        case AND_STRUCT_VFL_STATISTICS:
        {
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &mcxt.stat,
                                      sizeof(mcxt.stat));
            break;
        }
#endif // AND_COLLECT_STATISTICS

        case AND_STRUCT_VFL_FILSTATISTICS:
        {
            boolRes = FIL_GetStruct(AND_STRUCT_FIL_STATISTICS, pvoidStructBuffer,
                                    pdwStructSize);
            break;
        }

        case AND_STRUCT_VFL_NUM_OF_FTL_SUBLKS:
        {
            UInt16 dwNumFTLSuBlks = mcxt.gen.num_of_ftl_sublk;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwNumFTLSuBlks,
                                      sizeof(dwNumFTLSuBlks));
            break;
        }

        case AND_STRUCT_VFL_EXPORTED_LBA_NO:
        {
            UInt32 dwNumOfExportedLbas = mcxt.gen.exported_lba_no;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwNumOfExportedLbas,
                                      sizeof(dwNumOfExportedLbas));
            break;
        }

        case AND_STRUCT_VFL_EXPORTED_L2V_POOL:
        {
            UInt32 dwL2vPoolSize = mcxt.gen.exported_l2v_pool_size;
#ifndef AND_READONLY
            UInt32 vfl_capacity_in_kb = mcxt.gen.num_of_vfl_sublk * mcxt.dev.pages_per_sublk * (mcxt.dev.main_bytes_per_page/1024);

            if ((AND_L2V_POOL_SIZE_UNSPECIFIED == dwL2vPoolSize) || ((LARGE_DISK_THRESHOLD_KB < vfl_capacity_in_kb) && (LARGE_DISK_L2V_NODEPOOL_MEM >  dwL2vPoolSize )))
            {
                if(AND_L2V_POOL_SIZE_UNSPECIFIED == dwL2vPoolSize)
                {
                    dwL2vPoolSize = mcxt.fil->GetDeviceInfo(AND_DEVINFO_L2V_POOL_SIZE);
                }
                if (LARGE_DISK_THRESHOLD_KB < vfl_capacity_in_kb)
                {
                    dwL2vPoolSize = WMR_MAX(dwL2vPoolSize, LARGE_DISK_L2V_NODEPOOL_MEM);
                }
                if (AND_L2V_POOL_SIZE_UNSPECIFIED != dwL2vPoolSize)
                {
                    _setL2vPoolSize(dwL2vPoolSize);
                }
            }
#endif // AND_READONLY
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwL2vPoolSize,
                                      sizeof(dwL2vPoolSize));
            break;
        }

        case AND_STRUCT_VFL_NUM_OF_SUBLKS:
        {
            UInt16 dwNumVFLSuBlks = mcxt.gen.num_of_vfl_sublk;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwNumVFLSuBlks,
                                      sizeof(dwNumVFLSuBlks));
            break;
        }

        case AND_STRUCT_VFL_BYTES_PER_PAGE:
        {
            UInt16 wPageSize = mcxt.dev.main_bytes_per_page;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wPageSize,
                                      sizeof(wPageSize));
            break;
        }

        case AND_STRUCT_VFL_PAGES_PER_SUBLK:
        {
            UInt16 wPagesPerSuBlk = mcxt.dev.pages_per_sublk;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wPagesPerSuBlk,
                                      sizeof(wPagesPerSuBlk));
            break;
        }

        case AND_STRUCT_VFL_NUM_CHANNELS:
        {
            UInt32 dwNumChannels = mcxt.dev.num_channels;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwNumChannels,
                                      sizeof(dwNumChannels));
            break;
        }

        case AND_STRUCT_VFL_CES_PER_CHANNEL:
        {
            UInt32 dwCesPerChannel = mcxt.dev.ces_per_channel;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwCesPerChannel,
                                      sizeof(dwCesPerChannel));
            break;
        }

        case AND_STRUCT_VFL_CAUS_PER_CE:
        {
            UInt32 dwCausPerCe = mcxt.dev.caus_per_ce;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwCausPerCe,
                                      sizeof(dwCausPerCe));
            break;
        }
            
        case AND_STRUCT_VFL_MAX_TRANS_IN_PAGES:
        {
            UInt32 maxTransactionSizeinPages = (mcxt.dev.ces_per_channel) * (mcxt.dev.num_channels) * (PPN_MAX_PAGES_PER_CE);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &maxTransactionSizeinPages,
                                      sizeof(maxTransactionSizeinPages));
            break;
        }

        case AND_STRUCT_VFL_NAND_TYPE:
        {
            UInt32 nand_type = NAND_TYPE_PPN;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &nand_type,
                                      sizeof(nand_type));
            break;
        }

        case AND_STRUCT_VFL_BYTES_PER_META:
        {
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &mcxt.dev.lba_meta_bytes_buffer,
                                      sizeof(mcxt.dev.lba_meta_bytes_buffer));
            break;
        }

        case AND_STRUCT_VFL_VALID_BYTES_PER_META:
        {
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &mcxt.dev.lba_meta_bytes_valid,
                                      sizeof(mcxt.dev.lba_meta_bytes_valid));
            break;
        }

        case AND_STRUCT_VFL_PUBLIC_FBBT:
            // Fall Through
        case AND_STRUCT_VFL_FACTORY_BBT:
        {
            UInt8 cau;
            UInt8 ce;
            UInt8  *pData;
            Buffer *buf;
            const UInt32 cau_bbt_size = (mcxt.dev.blocks_per_cau / 8) + (mcxt.dev.blocks_per_cau % 8 ? 1 : 0);
            const UInt32 total_ces = mcxt.dev.ces_per_channel * mcxt.dev.num_channels;
            const UInt32 total_caus = mcxt.dev.caus_per_ce * total_ces;
            const UInt32 required_buffer_size = cau_bbt_size * total_caus;

            if (NULL == pdwStructSize)
            {
                // Can't do anything without a size
                boolRes = FALSE32;
            }
            else if (NULL == pvoidStructBuffer)
            {
                // Just report the size
                *pdwStructSize = required_buffer_size;
                boolRes = TRUE32;
            }
            else if (*pdwStructSize < required_buffer_size)
            {
                // Caller is asking for actual data with
                // too small of a buffer
                boolRes = FALSE32;
            }
            else if (AND_STRUCT_VFL_PUBLIC_FBBT == (dwStructType & AND_STRUCT_INDEX_MASK))
            {
                *pdwStructSize = required_buffer_size;

                // Mark all blocks bad
                WMR_MEMSET(pvoidStructBuffer, 0, *pdwStructSize);

                for (ce = 0; ce < total_ces; ce++)
                {
                    const UInt32 ce_bbt_size = cau_bbt_size * mcxt.dev.caus_per_ce;
                    const UInt32 ce_bbt_idx = ce * ce_bbt_size;
                    UInt8 *ce_bbt_ptr = ((UInt8 *)pvoidStructBuffer) + ce_bbt_idx;

                    // Mark block 0 good
                    ce_bbt_ptr[0] = 0x01;
                }

                boolRes = TRUE32;
            }
            else
            {
                *pdwStructSize = required_buffer_size;
                boolRes = TRUE32;
                pData = (UInt8 *)pvoidStructBuffer;
                buf = BUF_Get(BUF_MAIN_AND_SPARE);
                WMR_ASSERT(buf != NULL);

                for (ce = 0; ce < total_ces; ce++)
                {
                    for (cau = 0; cau < mcxt.dev.caus_per_ce; cau++)
                    {
                        ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_CAUBBT,
                                               PPN_NO_OPTIONS,
                                               ((cau * total_ces) + ce),
                                               0, 0, TRUE32, buf->pData, NULL);
                        WMR_MEMCPY(pData, buf->pData, cau_bbt_size);
                        // Consumers expect opposite bit-order
                        _bitSwapBuffer(pData, cau_bbt_size);
                        pData += cau_bbt_size;
                    }
                }
                BUF_Release(buf);
            }

            break;
        }

        case AND_STRUCT_VFL_CORRECTABLE_BITS:
        {
            UInt16 correctableBits = (UInt16)mcxt.fil->GetDeviceInfo(AND_DEVINFO_CORRECTABLE_SIZE);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &correctableBits, sizeof(correctableBits));
            break;
        }

        case AND_STRUCT_VFL_CORRECTABLE_SIZE:
        {
            UInt16 correctableSize = (UInt16)mcxt.fil->GetDeviceInfo(AND_DEVINFO_CORRECTABLE_SIZE);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &correctableSize, sizeof(correctableSize));
            break;
        }

        case AND_STRUCT_VFL_PAGES_PER_BLOCK:
        {
            UInt16 pagesPerBlock = (UInt16)mcxt.fil->GetDeviceInfo(AND_DEVINFO_PAGES_PER_BLOCK);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &pagesPerBlock, sizeof(pagesPerBlock));
            break;
        }

        case AND_STRUCT_VFL_DEVICEINFO_SIZE:
        {
            UInt32 deviceInfoSize = sizeof(mcxt.dev);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &deviceInfoSize, sizeof(deviceInfoSize));
            break;
        }

        case AND_STRUCT_VFL_DEVICEINFO:
        {
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &mcxt.dev, sizeof(mcxt.dev));
            break;
        }

        case AND_STRUCT_VFL_GETADDRESS:
        {
            UInt16 bank, block, page_offset, cau;
            UInt32 vba, vpn;
            ANDAddressStruct *addrStruct = (ANDAddressStruct*)pvoidStructBuffer;
            *pdwStructSize = sizeof(ANDAddressStruct);
            vba = addrStruct->dwVpn;
            vpn = vba / mcxt.dev.lbas_per_page;
            _convertVpnToBankBlockPage(vpn, &bank, &block, &page_offset);
            cau = ppnMiscGetCAUFromBank(&mcxt.dev, bank);
            addrStruct->dwPpn = ppnMiscConvertToPPNPageAddress(&mcxt.dev, cau, block, page_offset, FALSE32);
            addrStruct->dwCS = (UInt32)ppnMiscGetCEFromBank(&mcxt.dev, bank);
            addrStruct->dwColumn = vba % mcxt.dev.lbas_per_page;
            addrStruct->dwCau = cau;
            boolRes = TRUE32;
            break;
        }

#ifdef AND_COLLECT_STATISTICS
        case AND_STRUCT_VFL_BURNIN_CODE:
        {
            UInt32 code = (UInt32)mcxt.stat.ddwBurnInCode;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &code, sizeof(code));
            break;
        }
#endif //AND_COLLECT_STATISTICS

        case AND_STRUCT_VFL_LAST_ERROR:
        {
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &gstLastFailure, sizeof(gstLastFailure));
            break;
        }

        case AND_STRUCT_VFL_VFLMETA:
        {
            const UInt32 chip_enable_idx = dwStructType & AND_STRUCT_CS_MASK;

            if ((NULL == pvoidStructBuffer) &&
                (NULL != pdwStructSize))
            {
                *pdwStructSize = sizeof(PPNVFLMetaDataExport);
                boolRes = TRUE32;
            }
            else if ((NULL != pvoidStructBuffer) &&
                     (NULL != pdwStructSize) &&
                     (*pdwStructSize >= sizeof(PPNVFLMetaDataExport)) &&
                     (chip_enable_idx < (mcxt.dev.num_channels * mcxt.dev.ces_per_channel)))
            {
                PPNVFLMetaDataExport *exported_meta = (PPNVFLMetaDataExport*)pvoidStructBuffer;
                UInt16 bank;
                WMR_MEMSET(pvoidStructBuffer, 0xFF, sizeof(PPNVFLMetaDataExport));
                exported_meta->version = PPNVFL_EXPORT_META_VERSION;
                exported_meta->dev_info.num_of_channels = mcxt.dev.num_channels;
                exported_meta->dev_info.ces_per_channel = mcxt.dev.ces_per_channel;
                exported_meta->dev_info.caus_per_ce = mcxt.dev.caus_per_ce;
                for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
                {
                    exported_meta->per_bank_data[bank].geometry.chip_enable = ppnMiscGetCEFromBank(&mcxt.dev, bank);
                    exported_meta->per_bank_data[bank].geometry.chip_enable_idx = ppnMiscGetCEIdxFromBank(&mcxt.dev,
                                                                                                          bank);
                    exported_meta->per_bank_data[bank].geometry.channel = ppnMiscGetChannelFromBank(&mcxt.dev, bank);
                    exported_meta->per_bank_data[bank].geometry.cau = ppnMiscGetCAUFromBank(&mcxt.dev, bank);
                    exported_meta->per_bank_data[bank].num_special_blocks = mcxt.r_banks[bank].num_special_blocks;
                    exported_meta->per_bank_data[bank].num_grown_bad = mcxt.r_banks[bank].num_grown_bad;
                    exported_meta->per_bank_data[bank].num_initial_bad = mcxt.r_banks[bank].num_initial_bad;
                    exported_meta->per_bank_data[bank].num_available = mcxt.r_banks[bank].num_available;
                    exported_meta->per_bank_data[bank].num_cxt = mcxt.r_banks[bank].num_cxt;
                    exported_meta->per_bank_data[bank].num_p2v = mcxt.r_banks[bank].num_p2v;
                }
                *pdwStructSize = sizeof(PPNVFLMetaDataExport);
                boolRes = TRUE32;
            }
            break;
        }

#if AND_SUPPORT_BLOCK_BORROWING
        case AND_STRUCT_VFL_FIND_BORROWED_BLOCKS:
        {
            if (NULL != pdwStructSize)
            {
                BorrowBlockAddress *pastrBlocks = (BorrowBlockAddress*)pvoidStructBuffer;
                SpecialBlockAddress special_blocks[AND_MAX_SPECIAL_BLOCKS];
                UInt16 i, count, max_count, types[AND_MAX_SPECIAL_BLOCKS];


                mcxt.fpart->MapSpecialBlocks(special_blocks, types, &count);
                max_count = WMR_MIN(count, (*pdwStructSize / sizeof(pastrBlocks[0])));
                WMR_PRINT(ALWAYS, "Checking borrowed blocks - count: %d max_count: %d\n", count, max_count);
                for (i = 0; i < max_count; i++)
                {
                    UInt16 channel;
                    UInt16 ceIdx;
                    UInt16 cau;
                    UInt32 ppn_page;
                    channel = ppnMiscGetChannelFromBank(&mcxt.dev, special_blocks[i].bank);
                    ceIdx = ppnMiscGetCEIdxFromBank(&mcxt.dev, special_blocks[i].bank);
                    pastrBlocks[i].dwPhysicalCE = channel + ceIdx * mcxt.dev.num_channels;
                    cau = ppnMiscGetCAUFromBank(&mcxt.dev, special_blocks[i].bank);
                    ppn_page = ppnMiscConvertToPPNPageAddress(&mcxt.dev, cau, special_blocks[i].block, 0, FALSE32);
                    pastrBlocks[i].dwPhysicalBlock = (ppn_page >> mcxt.dev.bits_per_page_addr);
                    pastrBlocks[i].wType = types[i];
                }
                *pdwStructSize = max_count * sizeof(pastrBlocks[0]);
                boolRes = TRUE32;
            }
            break;
        }
#endif

        default:
            WMR_PANIC("_ppnvflGetStruct not handling dwStructType 0x%08x", dwStructType);
            WMR_PRINT(VFLWARN, "_ppnvflGetStruct 0x%X is not identified as VFL data struct identifier!\n", dwStructType);
            boolRes = FALSE32;
    }
    return boolRes;
}

/*
 * Name: _ppnvflGetDeviceInfo
 * Description: implements GetDeviceInfo function for the vfl I/F
 * Input: type
 * Return value: info per the requested information type
 */
static UInt32
_ppnvflGetDeviceInfo
    (UInt32 dwParamType)
{
    UInt32 dwRetVal = 0xFFFFFFFF;

    switch (dwParamType)
    {
        case AND_DEVINFO_PAGES_PER_SUBLK:
        {
            dwRetVal = (UInt32)mcxt.dev.pages_per_sublk;
            break;
        }

        case AND_DEVINFO_BYTES_PER_PAGE:
        {
            dwRetVal = (UInt32)mcxt.dev.main_bytes_per_page;
            break;
        }

        case AND_DEVINFO_NUM_OF_BANKS:
        {
            dwRetVal = (UInt32)mcxt.dev.num_of_banks;
            break;
        }

        case AND_DEVINFO_NUM_OF_USER_SUBLK:
        {
            dwRetVal = (UInt32)mcxt.gen.num_of_ftl_sublk;
            break;
        }

        case AND_DEVINFO_FTL_TYPE:
        {
            dwRetVal = (UInt32)mcxt.gen.ftl_type;
            break;
        }

        case AND_DEVINFO_FIL_LBAS_PER_PAGE:
        {
            dwRetVal = (UInt32)mcxt.dev.lbas_per_page;
            break;
        }

        case AND_DEVINFO_FIL_META_VALID_BYTES:
        {
            dwRetVal = (UInt32)mcxt.dev.lba_meta_bytes_valid;
            break;
        }

        case AND_DEVINFO_FIL_META_BUFFER_BYTES:
        {
            dwRetVal = (UInt32)mcxt.dev.lba_meta_bytes_buffer;
            break;
        }

        case AND_DEVINFO_VFL_AVAILABLE_VBAS:
        {
            UInt32 num_of_vbas = 0;
            UInt8 bank;
            for (bank = 0; bank < mcxt.dev.num_of_banks; bank++)
            {
                num_of_vbas += (mcxt.r_banks[bank].num_p2v * mcxt.dev.lbas_per_page *
                                mcxt.dev.pages_per_block_mlc);
            }
            dwRetVal = num_of_vbas;
            break;
        }

        case AND_DEVINFO_BLOCKS_PER_CAU:
        {
            dwRetVal = (UInt32)mcxt.dev.blocks_per_cau;
            break;
        }

        // pass-through to FIL
        case AND_DEVINFO_STREAM_BUFFER_MAX:
        {
            dwRetVal = mcxt.fil->GetDeviceInfo(dwParamType);
            break;
        }

        default:
        {
            WMR_PANIC("dwParamType not supported (0x%08x)\n", dwParamType);
            break;
        }
    }
    return dwRetVal;
}

static VFLReadStatusType
_ppnvflReadMultipleVbas
    (UInt32 vba,
    UInt16 count,
    UInt8 *data,
    UInt8 *meta,
    BOOL32 bDisableWhitening,
    BOOL32 bMarkECCForScrub)
{
    UInt16 vba_idx;
    VFLReadStatusType op_status = 0;

    WMR_PRINT(LOG, "ReadMultipleVbas(0x%08X, 0x%04X)\n", vba, count);

#ifdef AND_COLLECT_STATISTICS
    mcxt.stat.ddwVbasReadCnt += count;
    mcxt.stat.ddwSequentialReadCallCnt++;
#endif

    for (vba_idx = 0; vba_idx < count; )
    {
        UInt16 vbas_added = count - vba_idx;
        UInt8 *curr_data = &data[vba_idx * mcxt.dev.main_bytes_per_lba];
        UInt8 *curr_meta = &meta[vba_idx * mcxt.dev.lba_meta_bytes_buffer];
        UInt32 curr_vba = vba + vba_idx;

        _fillSequentialAddressesToCommandStructure(mcxt.commands, curr_vba, &vbas_added,
                                                   PPN_COMMAND_READ, curr_data, curr_meta, 
                                                   (bMarkECCForScrub ? 0 : PPN_OPTIONS_IGNORE_BAD_BLOCK),
                                                   InitFillPack);

        mcxt.fil->PerformCommandList(mcxt.commands, mcxt.dev.num_channels);

        _analyzeSpansReadStatuses(0, vbas_added, &op_status, bMarkECCForScrub, NULL);
        vba_idx += vbas_added;
    }

    return op_status;
}

#ifndef AND_READONLY

/*
 * Name: _ppnvflProgramMultipleVbas
 * Description: implements multi page program for the vfl I/F
 * Input: start virtual page address, number of page to program,
 *        data buffer, meta buffer
 * ignored: replace block on fail, whitening
 * Return value: TRUE32 if successful, otherwise FALSE32
 */
static BOOL32
_ppnvflProgramMultipleVbas
    (UInt32 vba,
    UInt16 count,
    UInt8 * data,
    UInt8 * meta,
    BOOL32 boolReplaceBlockOnFail,
    BOOL32 bDisableWhitening)
{
    UInt16 channel, i;
    BOOL32 res = TRUE32, last_failure_updated = FALSE32;
    UInt16 vba_idx;
    UInt16 phyCE;

    WMR_TRACE_4(ProgramMultipleVbas, START, vba, count, boolReplaceBlockOnFail, bDisableWhitening);

    WMR_ASSERT(!(vba % mcxt.dev.lbas_per_page));
    WMR_ASSERT(!(count % mcxt.dev.lbas_per_page));
    WMR_ASSERT(!mcxt.r_gen.active);
    mcxt.r_gen.active = TRUE32;

    WMR_PRINT(LOG, "ProgramMultipleVbas(0x%08X, 0x%04X)\n", vba, count);
#ifdef AND_COLLECT_STATISTICS
    mcxt.stat.ddwVbasProgrammedCnt += count;
    mcxt.stat.ddwSequentialProgramCallCnt++;
#endif

    for (vba_idx = 0; vba_idx < count && res; )
    {
        UInt16 vbas_added = count - vba_idx;
        UInt8 *curr_meta = &meta[vba_idx * mcxt.dev.lba_meta_bytes_buffer];

        _fillSequentialAddressesToCommandStructure(mcxt.commands, (vba + vba_idx),
                                                   &vbas_added, PPN_COMMAND_PROGRAM,
                                                   &data[vba_idx * mcxt.dev.main_bytes_per_lba],
                                                   curr_meta,
                                                   PPN_NO_OPTIONS, InitFillPack);
        mcxt.fil->PerformCommandList(mcxt.commands, mcxt.dev.num_channels);

        // handle program errors
        for (channel = 0; channel < mcxt.dev.num_channels; channel++)
        {
            PPNCommandStruct    *command = mcxt.commands[channel];

            for (i = 0; ((command->page_status_summary & PPN_PROGRAM_STATUS_FAIL) &&
                         (i < command->num_pages)); i++)
            {
                if (command->entry[i].status & PPN_PROGRAM_STATUS_FAIL)
                {
                    UInt16 bank, block, page_offset;
                    BOOL32 slc;

                    ppnMiscConvertPhysicalAddressToBankBlockPage(&mcxt.dev, channel,
                                                                 command->entry[i].ceIdx,
                                                                 command->entry[i].addr.row, &bank, &block,
                                                                 &page_offset, &slc);

                    phyCE = ppnMiscGetCEFromBank(&mcxt.dev,bank);
                    if (last_failure_updated == FALSE32)
                    { 
                        _ReportFailure(VFLFailWrite, phyCE, command->entry[i].addr.row);
                        last_failure_updated = TRUE32;
                    }

                    if (boolReplaceBlockOnFail == TRUE32)
                    {
                        WMR_PRINT(QUAL, "program fail -> retire "
                                            "vba:0x%X bank:0x%X, block:0x%X page_offset:0x%X "
                                            "phy CE: %d, row addr: 0x%X\n",
                                       vba, bank, block, page_offset, phyCE, command->entry[i].addr.row);
                        _markBlockForRetirement(bank, block);
                    }
                    else
                    {
                        WMR_PRINT(QUAL, "program fail -> do NOT retire "
                                            "vba:0x%X bank:0x%X, block:0x%X page_offset:0x%X\n"
                                            "phy CE: %d, row addr: 0x%X\n",
                                       vba, bank, block, page_offset, phyCE, command->entry[i].addr.row);
                    }
                    res = FALSE32;
                }
            }
        }
        vba_idx += vbas_added;
    }

    WMR_TRACE_1(ProgramMultipleVbas, END, res);

    mcxt.r_gen.active = FALSE32;
    return res;
}

/*
 * Name: _ppnvflErase
 * Description: implements erase single virtual block for the vfl I/F
 * Input: virtual block address, replace block on fail
 * Return value: ANDErrorCodeOk if successful, otherwise ANDErrorWriteFailureErr
 */
static Int32
_ppnvflErase
    (UInt16 wVbn,
    BOOL32 bReplaceOnFail)
{
    UInt32 vba;
    UInt16 bank, channel, vbas_to_erase;
    BOOL32 last_failure_updated = FALSE32;
    UInt16 phyCE;
    ANDStatus status = ANDErrorCodeOk;

    WMR_PRINT(LOG, "Erase (0x%04X)\n", wVbn);

    WMR_ASSERT(!mcxt.r_gen.active);
    mcxt.r_gen.active = TRUE32;

#ifdef AND_COLLECT_STATISTICS
    mcxt.stat.ddwVBlocksErasedCnt++;
    mcxt.stat.ddwEraseCallCnt++;
#endif

    for (bank = 0; (bank < mcxt.dev.num_of_banks) && (mcxt.blocks_to_scrub_count != 0); bank++)
    {
        if (_isBlockToScrubInBitmap(bank, wVbn))
        {
            if (!_removeBadBlock(bank, wVbn))
            {
                status = ANDErrorWriteFailureErr;
                goto exit;
            }
            // invalidate the vbn cache
            mcxt.r_gen.cached_vbn = 0xFFFF;
        }
    }

    vbas_to_erase = _countVbnPhysicalBlocks(wVbn) * mcxt.dev.lbas_per_page;
    vba = wVbn * mcxt.dev.pages_per_sublk * mcxt.dev.lbas_per_page;
    _fillSequentialAddressesToCommandStructure(mcxt.commands, vba, &vbas_to_erase,
                                               PPN_COMMAND_ERASE, NULL, NULL, PPN_NO_OPTIONS, InitFillPack);
    mcxt.fil->PerformCommandList(mcxt.commands, mcxt.dev.num_channels);

    for (channel = 0; channel < mcxt.dev.num_channels; channel++)
    {
        PPNCommandStruct    *command = mcxt.commands[channel];
        UInt16 page_idx;

        for (page_idx = 0; ((command->page_status_summary & PPN_ERASE_STATUS_RETIRE) &&
                            (page_idx < command->num_pages)); page_idx++)
        {
            // invalidate the vbn cache
            mcxt.r_gen.cached_vbn = PPNVFL_INVALID_BANK_BLOCK;

            if (command->entry[page_idx].status & PPN_ERASE_STATUS_RETIRE)
            {
                UInt16 block, page_offset;
                BOOL32 slc;

                ppnMiscConvertPhysicalAddressToBankBlockPage(&mcxt.dev, channel,
                                                             command->entry[page_idx].ceIdx,
                                                             command->entry[page_idx].addr.row, &bank, &block,
                                                             &page_offset,
                                                             &slc);

                WMR_PRINT(QUAL, "failed to erase a block - channel: 0x%02X, ceIdx:0x%02X, "
                                "page_addr: 0x%08X, bank: 0x%02X, block: 0x%04X, slc:%01d\n", 
                          channel, command->entry[page_idx].ceIdx, command->entry[page_idx].addr.row, 
                          bank, block, slc);

                if (last_failure_updated == FALSE32)
                {
                    phyCE = ppnMiscGetCEFromBank(&mcxt.dev,bank);
                    _ReportFailure(VFLFailErase, phyCE,
                                   command->entry[page_idx].addr.row);
                    last_failure_updated = TRUE32;
                }

                // replace a bad block if the caller asked us to do so
                if (bReplaceOnFail == TRUE32)
                {
                    if (!_removeBadBlock(bank, block))
                    {
                        WMR_PRINT(ERROR, "error removing a block\n");
                        status = ANDErrorWriteFailureErr;
                        goto exit;
                    }
                }
                else
                {
                    status = ANDErrorWriteFailureErr;
                    goto exit;
                }
            }
        }
    }

exit:
    mcxt.r_gen.active = FALSE32;
    return status;
}

#endif // AND_READONLY

/*
 * Name: _ppnvflSetStruct
 * Description: implements SetStruct for the vfl I/F
 * Input: type, buffer, buffer size
 * Return value: TRUE32 if successful, otherwise FALSE32
 */
static BOOL32
_ppnvflSetStruct
    (UInt32 dwStructType,
    void * pvoidStructBuffer,
    UInt32 dwStructSize)
{
    BOOL32 result = FALSE32;

    switch (dwStructType & AND_STRUCT_INDEX_MASK)
    {
#ifndef AND_READONLY
        case AND_STRUCT_VFL_FTLTYPE:
        {
            Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
            mcxt.gen.ftl_type = (*((UInt32*)pvoidStructBuffer));

            result = _programCxtLastPage(buf, TRUE32);
            BUF_Release(buf);
        }
        break;

        case AND_STRUCT_VFL_EXPORTED_LBA_NO:
        {
            result = _setNumOfExportedLbas(*((UInt32*)pvoidStructBuffer));
        }
        break;

        case AND_STRUCT_VFL_EXPORTED_L2V_POOL:
        {
            result = _setL2vPoolSize(*((UInt32*)pvoidStructBuffer));
        }
        break;

        case AND_STRUCT_VFL_NUM_OF_FTL_SUBLKS:
        {
            result = _setNumOfFTLSuBlks(*((UInt16*)pvoidStructBuffer));
        }
        break;
#endif // AND_READONLY

#ifdef AND_COLLECT_STATISTICS
        case AND_STRUCT_VFL_BURNIN_CODE:
        {
            if (NULL != pvoidStructBuffer)
            {
                mcxt.stat.ddwBurnInCode = *((UInt32*)pvoidStructBuffer);
                result = TRUE32;
            }
        }
        break;

        case AND_STRUCT_VFL_STATISTICS:
        {
            if ((NULL != pvoidStructBuffer))
            {
                WMR_MEMCPY(&mcxt.stat, pvoidStructBuffer, WMR_MIN(sizeof(SVFLStatistics), dwStructSize));
                result = TRUE32;
            }
        }
        break;

        case AND_STRUCT_VFL_FILSTATISTICS:
        {
            result = FIL_SetStruct(AND_STRUCT_FIL_STATISTICS, pvoidStructBuffer, dwStructSize);
        }
        break;
#endif //AND_COLLECT_STATISTICS

        default:
        {
            WMR_PRINT(ERROR, "_ppnvflSetStruct 0x%X is not identified is VFL data struct identifier!\n", dwStructType);
        }
    }
    return result;
}

/*
 * Name: _ppnvflClose
 * Description: implements close for the vfl I/F
 * Input:
 * Return value: none
 */
static void
_ppnvflClose
    (void)
{
    if (mcxt.v2p_bitmap)
    {
        WMR_FREE(mcxt.v2p_bitmap, mcxt.v2p_bitmap_size);
        mcxt.v2p_bitmap = NULL;
    }

    if (mcxt.v2p_scrub_bitmap)
    {
        WMR_FREE(mcxt.v2p_scrub_bitmap, mcxt.v2p_bitmap_size);
        mcxt.v2p_scrub_bitmap = NULL;
    }

    if (mcxt.vfl_map_pages[0])
    {
        WMR_FREE(mcxt.vfl_map_pages[0], (sizeof(UInt16) * mcxt.dev.num_of_banks * mcxt.r_gen.pages_per_block_mapping));
        mcxt.vfl_map_pages[0] = NULL;
    }

    if (mcxt.vfl_map_pages)
    {
        WMR_FREE(mcxt.vfl_map_pages, (sizeof(UInt16*) * mcxt.dev.num_of_banks));
        mcxt.vfl_map_pages = NULL;
    }

    if (mcxt.r_banks)
    {
        WMR_FREE(mcxt.r_banks, (sizeof(PPNVFL_BankRAMCxt) * mcxt.dev.num_of_banks));
        mcxt.r_banks = NULL;
    }

    if (mcxt.reorder)
    {
        WMR_FREE(mcxt.reorder, sizeof(PPNReorderStruct));
        mcxt.reorder = NULL;
    }

    if (mcxt.initialized)
    {
        WMR_BufZone_Free(&mcxt.bufzone);
    }
    WMR_MEMSET(&mcxt, 0, sizeof(PPNVFL_MainCxt));
}

#ifndef AND_READONLY
#if (defined(AND_SUPPORT_BLOCK_BORROWING) && AND_SUPPORT_BLOCK_BORROWING)
static BOOL32
_ppnvflBorrowSpareBlock
    (UInt32 *pdwPhysicalCE,
    UInt32 *pdwPhysicalBlock,
    UInt16 wType)
{
    BOOL32 res;
    SpecialBlockAddress special_block;

    res = _ppnvflAllocateSpecialBlock(&special_block, wType);
    if (res)
    {
        UInt16 channel;
        UInt16 ceIdx;
        UInt16 cau;
        UInt32 ppn_page;
        channel = ppnMiscGetChannelFromBank(&mcxt.dev, special_block.bank);
        ceIdx = ppnMiscGetCEIdxFromBank(&mcxt.dev, special_block.bank);
        *pdwPhysicalCE = channel + ceIdx * mcxt.dev.num_channels;
        cau = ppnMiscGetCAUFromBank(&mcxt.dev, special_block.bank);
        ppn_page = ppnMiscConvertToPPNPageAddress(&mcxt.dev, cau, special_block.block, 0, FALSE32);
        *pdwPhysicalBlock = (ppn_page >> mcxt.dev.bits_per_page_addr);
    }
    return res;
}
#endif // AND_SUPPORT_BLOCK_BORROWING

static BOOL32
_ppnvflMarkBlockAsGrownBad
    (SpecialBlockAddress *block)
{
    P2UsageType block_usage;
    BOOL32 result;

    block_usage = PPNVFL_TYPE_GROWN_BB;
    mcxt.r_banks[block->bank].num_special_blocks--;
    mcxt.r_banks[block->bank].num_grown_bad++;
    result = _setBlockUsageInVFLCxt(block->bank, block->block, block_usage);

    if (result)
    {
        Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);

        result = _programCxtLastPage(buf, TRUE32);
        BUF_Release(buf);
    }

    return result;
}
#endif // AND_READONLY

static UInt16
_ppnvflGetVbasPerVb
    (UInt16 wVbn)
{
    return _countVbnPhysicalBlocks(wVbn) * mcxt.dev.pages_per_block_mlc * mcxt.dev.lbas_per_page;
}

static void
_ppnvflReadSpansInit
    (VFL_ReadSpans_t *s,
    VFL_ReadSpansCB_t cb,
    UInt32 cbOpt,
    BOOL32 bDisableWhitening,
    BOOL32 bMarkECCForScrub)
{
#ifdef AND_COLLECT_STATISTICS
    mcxt.stat.ddwSpansReadInitCallCnt++;
#endif

    WMR_ASSERT(!mcxt.r_gen.active);

    s->bDisableWhitening = bDisableWhitening;
    s->bMarkECCForScrub = bMarkECCForScrub;
    s->len = 0;
    s->cb = cb;
    s->cbOpt = cbOpt;
    s->op_status = 0;
}

static VFLReadStatusType
_ppnvflReadSpans
    (VFL_ReadSpans_t *s)
{
    UInt16 sp_idx;
    UInt16 vba_index_start = 0, vba_index_count = 0;
    
    WMR_ASSERT(!mcxt.r_gen.active);
    mcxt.r_gen.active = TRUE32;

#ifdef AND_COLLECT_STATISTICS
    mcxt.stat.ddwSpansReadExecCallCnt++;
#endif

    for (sp_idx = 0; sp_idx < s->len; sp_idx++)
    {
        UInt16 vba_idx;
        const UInt32 vba = s->vba[sp_idx];
        const UInt16 count = s->count[sp_idx];
        UInt8* data = s->data[sp_idx];
        UInt8* meta = s->meta[sp_idx];
#ifdef AND_COLLECT_STATISTICS
        mcxt.stat.ddwVbasReadCnt += count;
#endif

        WMR_ASSERT((data != NULL) && (meta != NULL));
        WMR_PRINT(LOG, "ReadSpans[%02d](0x%08X, 0x%04X)\n", sp_idx, vba, count);

        for (vba_idx = 0; vba_idx < s->count[sp_idx]; )
        {
            UInt16 vbas_added = count - vba_idx;
            UInt8 *curr_data = &data[vba_idx * mcxt.dev.main_bytes_per_lba];
            UInt8 *curr_meta = &meta[vba_idx * mcxt.dev.lba_meta_bytes_buffer];
            UInt32 curr_vba = vba + vba_idx;

            if (vba_index_count == 0)
            {
                _fillSequentialAddressesToCommandStructure(mcxt.commands, curr_vba, &vbas_added,
                                                           PPN_COMMAND_READ, curr_data, curr_meta, 
                                                           (s->bMarkECCForScrub ? 0 : PPN_OPTIONS_IGNORE_BAD_BLOCK),
                                                           InitAndFill);
            }
            else
            {
                _fillSequentialAddressesToCommandStructure(mcxt.commands, curr_vba, &vbas_added,
                                                           PPN_COMMAND_READ, curr_data, curr_meta, 
                                                           (s->bMarkECCForScrub ? 0 : PPN_OPTIONS_IGNORE_BAD_BLOCK),
                                                           FillOnly);
            }

            vba_idx += vbas_added;
            if (0 != vbas_added)
            {
                vba_index_count += vbas_added;
            }
            else
            {
                _fillSequentialAddressesToCommandStructure(mcxt.commands, 0, NULL,
                                                           PPN_COMMAND_NONE, NULL, NULL, 
                                                           (s->bMarkECCForScrub ? 0 : PPN_OPTIONS_IGNORE_BAD_BLOCK),
                                                           PackOnly);
                mcxt.fil->PerformCommandList(mcxt.commands, mcxt.dev.num_channels);
                _analyzeSpansReadStatuses(vba_index_start, vba_index_count, &s->op_status,
                                          s->bMarkECCForScrub, s);
                vba_index_start += vba_index_count;
                vba_index_count = 0;
            }
        }
    }

    if (vba_index_count != 0)
    {
        _fillSequentialAddressesToCommandStructure(mcxt.commands, 0, NULL,
                                                    PPN_COMMAND_NONE, NULL, NULL, 
                                                   (s->bMarkECCForScrub ? 0 : PPN_OPTIONS_IGNORE_BAD_BLOCK),
                                                   PackOnly);
        mcxt.fil->PerformCommandList(mcxt.commands, mcxt.dev.num_channels);
        _analyzeSpansReadStatuses(vba_index_start, vba_index_count, &s->op_status, s->bMarkECCForScrub, s);
    }

    mcxt.r_gen.active = FALSE32;
    return s->op_status;
}

static void
_ppnvflReadSpansAdd
    (VFL_ReadSpans_t *s,
    UInt32 vba,
    UInt32 count,
    UInt8 *data,
    UInt8 *meta)
{
    WMR_PRINT(LOG, "ReadSpansAdd (0x%08X, 0x%04X)\n", vba, count);
    WMR_ASSERT(vba < (mcxt.dev.pages_per_sublk * mcxt.dev.blocks_per_cau * mcxt.dev.lbas_per_page));
    WMR_ASSERT(!mcxt.r_gen.active);

#ifdef AND_COLLECT_STATISTICS
    mcxt.stat.ddwSpansReadAddCallCnt++;
#endif

    if (s->len >= VFL_SPANS_MAX)
    {
        _ppnvflReadSpans(s);
        s->len = 0;
    }

    WMR_ASSERT((data != NULL) && (meta != NULL));

    s->vba[s->len] = vba;
    s->count[s->len] = count;
    s->data[s->len] = data;
    s->meta[s->len] = meta;
    s->len++;
}

/* vfl I/F implementation - end */

static UInt32
_ppnvflGetMinorVersion
    (void)
{
    return kPPNVFLMinorVersion;
}

#if WMR_EFI
// Page APIs, implemented for EFI/Bonfire
static BOOL32
_ppnvflWriteMultiplePagesInVb
    (UInt32 dwVpn,
    UInt16 wNumPagesToWrite,
    UInt8 * pbaData,
    UInt8 * pbaSpare,
    BOOL32 boolReplaceBlockOnFail,
    BOOL32 bDisableWhitening)
{
    UInt32 vba = dwVpn * mcxt.dev.lbas_per_page;
    UInt32 vbaCount = wNumPagesToWrite * mcxt.dev.lbas_per_page;

    return _ppnvflProgramMultipleVbas(vba, vbaCount, pbaData, pbaSpare, boolReplaceBlockOnFail,
                                      bDisableWhitening);
}
#endif // WMR_EFI

#if WMR_EFI
/*
 * Name: _analyzeMultiScatteredReadStatuses
 * Description: this function analyzes the read operation statuses (used for multipage read)
 * Input: number of pages read, mark for scrub
 * Output: need refresh flag, operation result flag, clean flag
 * Return value: none
 */
static UInt32
_analyzeMultiScatteredReadStatuses
    (UInt32 *result_flags,
     UInt16 bank)
{
    BOOL32 last_failure_updated = FALSE32;
    UInt32 current_result = 0;
    UInt32 read_status = VFL_READ_STATUS_UNIDENTIFIED;

    PPNStatusType current_status = mcxt.single_command->entry[0].status;
    UInt16 phyCE;
    
    switch (current_status)
    {
        case (PPN_READ_STATUS_VALID):
        case (PPN_READ_STATUS_VALID | PPN_READ_STATUS_REFRESH):
        case (PPN_READ_STATUS_VALID | PPN_READ_STATUS_RETIRE):
        case (PPN_READ_STATUS_INVALID_DATA | PPN_READ_STATUS_REFRESH | PPN_READ_STATUS_VALID):
        case (PPN_READ_STATUS_INVALID_DATA | PPN_READ_STATUS_RETIRE | PPN_READ_STATUS_VALID):
        case (PPN_READ_STATUS_INVALID_DATA | PPN_READ_STATUS_CLEAN | PPN_READ_STATUS_VALID):
        case (PPN_READ_STATUS_INVALID_DATA | PPN_READ_STATUS_GEB | PPN_READ_STATUS_VALID):
            break;

        default:
            current_result |= VFL_READ_STATUS_UNIDENTIFIED;
            break;
    }
    if (!(current_result & VFL_READ_STATUS_UNIDENTIFIED))
    {
        if (current_status & PPN_READ_STATUS_INVALID_DATA &&
            (!(current_status & PPN_READ_STATUS_CLEAN)))
        {
            current_result |= VFL_READ_STATUS_UECC;
        }
        if (current_status & PPN_READ_STATUS_CLEAN)
        {
            current_result |= VFL_READ_STATUS_CLEAN;
            read_status = VFL_READ_STATUS_CLEAN;
        }
        if (current_status & PPN_READ_STATUS_REFRESH)
        {
            current_result |= VFL_READ_STATUS_REFRESH;
        }
        if (current_status & PPN_READ_STATUS_RETIRE)
        {
            current_result |= VFL_READ_STATUS_RETIRE;
        }
        if (!(current_status & PPN_READ_STATUS_INVALID_DATA))
        {
            current_result |= VFL_READ_STATUS_VALID_DATA;
            read_status = VFL_READ_STATUS_VALID_DATA;
        }
    }
    *result_flags |= current_result;
    if ((!last_failure_updated) && (current_result & VFL_READ_STATUS_UECC))
    {
        last_failure_updated = TRUE32;
        phyCE = ppnMiscGetCEFromBank(&mcxt.dev,bank);
        _ReportFailure(VFLFailUECC, phyCE, mcxt.single_command->entry[0].addr.row);
        read_status = VFL_READ_STATUS_UECC;
    }
    else if (current_result & VFL_READ_STATUS_REFRESH)
    {
        phyCE = ppnMiscGetCEFromBank(&mcxt.dev,bank);
        _ReportFailure(VFLFailRefresh, phyCE, mcxt.single_command->entry[0].addr.row);
    }
    return read_status;
}
#endif // WMR_EFI

#if WMR_EFI
// Page APIs, implemented for EFI/Bonfire
static BOOL32
_ppnvflReadScatteredPagesInVb
    (UInt32 * padwVpn,
    UInt16 wNumPagesToRead,
    UInt8 * pbaData,
    UInt8 * pbaSpare,
    BOOL32 * pboolNeedRefresh,
    UInt8 * pabSectorStats,
    BOOL32 bDisableWhitening,
    Int32 *actualStatus)
{
    UInt16 vpn_idx;
    BOOL32 res = FALSE32;
    const UInt8 num_sectors = (mcxt.dev.main_bytes_per_page) / 1024;   //Number of kilobyte sectors
    UInt32 op_status = 0;
    UInt32 read_status;

    WMR_PRINT(LOG, "ReadScatteredPagesInVb(0x%04X)\n", wNumPagesToRead);

    // mark all the sectors as not valid
    if (pabSectorStats != NULL)
    {
        WMR_MEMSET(pabSectorStats, 0xFD, (num_sectors * wNumPagesToRead));
    }
    
    for (vpn_idx = 0; vpn_idx < wNumPagesToRead; vpn_idx++)
    {
        UInt32 vpn = padwVpn[vpn_idx];
        UInt16 bank, block, page_offset;
        UInt8 *data = &pbaData[vpn_idx * mcxt.dev.main_bytes_per_page];
        UInt8 *meta = &pbaSpare[vpn_idx * mcxt.dev.lba_meta_bytes_valid * mcxt.dev.lbas_per_page];
        UInt8 *stats;
        UInt8 *stats_meta = NULL;
        UInt8 *stats_data = NULL;

        // convert to physical address
        _convertVpnToBankBlockPage(vpn, &bank, &block, &page_offset);

        if (pabSectorStats != NULL)
        {
            ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_READ,
                                   PPN_OPTIONS_REPORT_HEALTH,
                                   bank, block, page_offset, FALSE32, data, meta);
            // use the right offset in the stats buffer (leave swiss holes where relevant)
            stats = &pabSectorStats[vpn_idx * num_sectors];
            read_status = _analyzeMultiScatteredReadStatuses(&op_status, bank);

            if(stats_copy_from_data)
            {
                stats_data = stats + stats_offset_from_data;
            }
            if(stats_copy_from_meta)
            {
                stats_meta = stats + stats_offset_from_meta;
            }

            if (read_status & VFL_READ_STATUS_VALID_DATA)
            {
                if(stats_data)
                {
                    WMR_MEMCPY(stats_data, data, stats_copy_from_data);
                }
                if(stats_meta)
                {
                    WMR_MEMCPY(stats_meta, meta, stats_copy_from_meta);
                }
            }
            else if (read_status & VFL_READ_STATUS_CLEAN)
            {
                if(stats_data)
                {
                    WMR_MEMSET(stats_data, 0xFE, stats_copy_from_data);
                }
                if(stats_meta)
                {
                    WMR_MEMSET(stats_meta, 0xFE, stats_copy_from_meta);
                }
            }
            else if (read_status & VFL_READ_STATUS_UECC)
            {
                if(stats_data)
                {
                    WMR_MEMSET(stats_data, 0xFF, stats_copy_from_data);
                }
                if(stats_meta)
                {
                    WMR_MEMSET(stats_meta, 0xFF, stats_copy_from_meta);
                }
            }   
        }
        else
        {
            ppnMiscSingleOperation(&mcxt.dev, mcxt.single_command, PPN_COMMAND_READ,
                                   PPN_NO_OPTIONS,
                                   bank, block, page_offset, FALSE32, data, meta);
            read_status = _analyzeMultiScatteredReadStatuses(&op_status, bank);
        }       
    }

    if (actualStatus != NULL)
    {
        if (op_status & VFL_READ_STATUS_UNIDENTIFIED)
        {
            WMR_PRINT(ERROR, "op_status 0x%02x\n", op_status);
            *actualStatus = ANDErrorCodeHwErr;
        }
        else if((op_status & VFL_READ_STATUS_CLEAN) && (!(op_status & VFL_READ_STATUS_VALID_DATA))&& (!(op_status & VFL_READ_STATUS_UECC)))
        {
            *actualStatus = ANDErrorCodeCleanOk;
            res = TRUE32;
        }
        else if ((op_status & VFL_READ_STATUS_UECC) ||
                 ((op_status & VFL_READ_STATUS_CLEAN) && (!(op_status & VFL_READ_STATUS_VALID_DATA))))
        {
            *actualStatus = ANDErrorCodeUserDataErr;
        }
        else if (op_status & VFL_READ_STATUS_VALID_DATA)
        {
            *actualStatus = ANDErrorCodeOk;
            res = TRUE32;
        }
        else
        {
            // don't panic, per rdar://8234566
            WMR_PRINT(ERROR, "op_status 0x%02x\n", op_status);
            *actualStatus = ANDErrorCodeHwErr;
        }
    }
    if (pboolNeedRefresh != NULL)
    {
        if (op_status & (VFL_READ_STATUS_REFRESH | VFL_READ_STATUS_RETIRE | VFL_READ_STATUS_UECC))
        {
            *pboolNeedRefresh = TRUE32;
        }
        else
        {
            *pboolNeedRefresh = FALSE32;
        }
    }

    return res;
}
#endif // WMR_EFI

void
SwissVFL_Register
    (VFLFunctions * pVFL_Functions)
{
    WMR_MEMSET(pVFL_Functions, 0, sizeof(VFLFunctions));
#ifndef AND_READONLY
    pVFL_Functions->Format = _ppnvflFormat;
    pVFL_Functions->Erase = _ppnvflErase;
#if (defined(AND_SUPPORT_BLOCK_BORROWING) && AND_SUPPORT_BLOCK_BORROWING)
    pVFL_Functions->BorrowSpareBlock = _ppnvflBorrowSpareBlock;
#endif // AND_SUPPORT_BLOCK_BORROWING
    pVFL_Functions->ProgramMultipleVbas = _ppnvflProgramMultipleVbas;
#endif // ! AND_READONLY
    pVFL_Functions->Init = _ppnvflInit;
    pVFL_Functions->Open = _ppnvflOpen;
    pVFL_Functions->Close = _ppnvflClose;
    pVFL_Functions->ReadMultipleVbas = _ppnvflReadMultipleVbas;
    //	pVFL_Functions->GetAddress = VFL_GetAddress;
    pVFL_Functions->GetStruct = _ppnvflGetStruct;
    pVFL_Functions->SetStruct = _ppnvflSetStruct;
    pVFL_Functions->GetDeviceInfo = _ppnvflGetDeviceInfo;
#ifndef AND_READONLY
    pVFL_Functions->AllocateSpecialBlock = _ppnvflAllocateSpecialBlock;
    pVFL_Functions->MarkBlockAsGrownBad = _ppnvflMarkBlockAsGrownBad;
#endif
    pVFL_Functions->GetMinorVersion = _ppnvflGetMinorVersion;
    pVFL_Functions->GetVbasPerVb = _ppnvflGetVbasPerVb;
    pVFL_Functions->ReadSpansInit = _ppnvflReadSpansInit;
    pVFL_Functions->ReadSpansAdd = _ppnvflReadSpansAdd;
    pVFL_Functions->ReadSpans = _ppnvflReadSpans;

#if WMR_EFI
    // Page APIs, for EFI/Bonfire ONLY
    pVFL_Functions->WriteMultiplePagesInVb = _ppnvflWriteMultiplePagesInVb;
    pVFL_Functions->ReadScatteredPagesInVb = _ppnvflReadScatteredPagesInVb;
#endif
}

static VFLFunctions _ppnvfl_functions;

VFLFunctions *
PPN_SVFL_GetFunctions
    (void)
{
    SwissVFL_Register(&_ppnvfl_functions);
    return &_ppnvfl_functions;
}
/* global calls for the ppnvfl (using the declared global main vfl cxt struct - end */
