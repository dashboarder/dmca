#include "WMRConfig.h"
#include "ANDTypes.h"
#include "WMROAM.h"
#include "VFLBuffer.h"
#include "PPNMiscTypes.h"
#include "PPNFPartTypes.h"
#include "FPart.h"
#include "VFL.h"
#include "WMRBuf.h"
#include "WhimoryBoot.h"

#define kPPNFPartMinorVersion (1)

#define BYTES_PER_PAGE_META (mcxt->dev.lbas_per_page * mcxt->dev.lba_meta_bytes_buffer)

static PPNFPart_MainCxt *g_ppnfpartMain = NULL;
extern UInt32 g_num_keepout_blocks;

static BOOL32 _findSpecialBlock
    (PPNFPart_MainCxt *mcxt,
    UInt16 *special_block_cache_index,
    UInt16 type);

#if !AND_READONLY

static void
_setSpareFPartCxt
    (PPNFPart_MainCxt *mcxt,
    PPNFPart_Spare *spare,
    UInt16 vfl_special_type,
    UInt8 index)
{
    WMR_MEMSET(spare, 0, sizeof(PPNFPart_Spare));
    spare->spare_type = PPNFPART_SPECIAL_BLOCK_SPARE_TYPE;
    spare->vfl_special_type = vfl_special_type;
    spare->index = index;
}

static void
_copySpareFPartCxtToMeta
    (PPNFPart_MainCxt *mcxt,
    PPNFPart_Spare *spare,
    UInt8 *meta)
{
    UInt16 lba_idx;

    for (lba_idx = 0; lba_idx < mcxt->dev.lbas_per_page; lba_idx++)
    {
        WMR_MEMCPY(meta, spare, sizeof(PPNFPart_Spare));
        meta += mcxt->dev.lba_meta_bytes_buffer;
    }
}

#endif // !AND_READONLY

static void
_copySpareFPartCxtFromMeta
    (PPNFPart_MainCxt *mcxt,
    PPNFPart_Spare *spare,
    UInt8 *meta)
{
    WMR_MEMCPY(spare, meta, sizeof(PPNFPart_Spare));
}

static BOOL32
_ppnfpartInit
    (PPNFPart_MainCxt * mcxt,
    LowFuncTbl *pFILFunctions,
    UInt32 dwOptions)
{
    if (mcxt->initialized)
    {
        return TRUE32;
    }

    WMR_ASSERT(sizeof(PPNFPart_Spare) == 16);

    mcxt->fil = pFILFunctions;
    ppnMiscFillDevStruct(&mcxt->dev, mcxt->fil);
    mcxt->done_mapping = FALSE32;

    WMR_BufZone_Init(&mcxt->zone);
    mcxt->command = WMR_Buf_Alloc_ForDMA(&mcxt->zone, sizeof(PPNCommandStruct));
    WMR_BufZone_FinishedAllocs(&mcxt->zone);
    WMR_BufZone_Rebase(&mcxt->zone, (void **)&mcxt->command);
    WMR_BufZone_FinishedRebases(&mcxt->zone);

    mcxt->special_blocks = (SpecialBlockCacheEntryType*)WMR_MALLOC(AND_MAX_SPECIAL_BLOCKS *
                                                                   sizeof(SpecialBlockCacheEntryType));

    mcxt->special_blocks_idx = 0;
    mcxt->initialized = TRUE32;

    return TRUE32;
}

static void
_ppnfpartChangeCacheState
    (PPNFPart_MainCxt * mcxt,
    BOOL32 validate)
{
    if (validate)
    {
        mcxt->done_mapping = TRUE32;
    }
    else
    {
        WMR_MEMSET(mcxt->special_blocks, 0xFF, (AND_MAX_SPECIAL_BLOCKS * sizeof(SpecialBlockCacheEntryType)));
        mcxt->special_blocks_idx = 0;
        mcxt->done_mapping = FALSE32;
    }
}

static void
_ppnfpartClose
    (PPNFPart_MainCxt * mcxt)
{
    _ppnfpartChangeCacheState(mcxt, FALSE32);
    WMR_BufZone_Free(&mcxt->zone);
    WMR_FREE(mcxt->special_blocks, (AND_MAX_SPECIAL_BLOCKS * sizeof(SpecialBlockCacheEntryType)));
    WMR_MEMSET(mcxt, 0, sizeof(PPNFPart_MainCxt));
}

static UInt16
_countBlocksInHeader
    (PPNFPart_MainCxt *mcxt,
    SpecialBlockHeaderStruct *header)
{
    UInt16 block_address_count;

    for (block_address_count = 0;
         block_address_count < PPNFPART_MAX_SPECIAL_BLOCKS_PER_TYPE;
         block_address_count++)
    {
        if (header->block_addresses[block_address_count].bank == 0xFFFF)
        {
            break;
        }
    }
    return block_address_count;
}

static UInt16
_getMajorVersion
    (PPNFPart_MainCxt *mcxt,
    UInt32 version)
{
    return (UInt16)(version >> 16);
}

void
_initSpecialBlockHeader
    (SpecialBlockHeaderStruct *header,
    SpecialBlockCacheEntryType * blocks,
    UInt16 block_count,
    UInt32 data_size,
    UInt32 copy_number)
{
    UInt16 i;

    WMR_MEMSET(header, 0x00, sizeof(SpecialBlockHeaderStruct));
    WMR_MEMSET(header->block_addresses, 0xFF,
               (PPNFPART_MAX_SPECIAL_BLOCKS_PER_TYPE * sizeof(SpecialBlockAddress)));
    for (i = 0; i < block_count; i++)
    {
        header->block_addresses[i].bank = blocks[i].bank;
        header->block_addresses[i].block = blocks[i].block;
    }
    header->data_size = data_size;
    header->version = PPNFPART_STRUCTURE_VERSION;
    header->copy_number = copy_number;
}

static BOOL32
_findSpecialBlockInCache
    (PPNFPart_MainCxt *mcxt,
    UInt16 *special_block_cache_index,
    UInt16 type)
{
    UInt16 i;

    // try the cache first
    for (i = 0; i < mcxt->special_blocks_idx; i++)
    {
        if (andSBGetCode(mcxt->special_blocks[i].type) == andSBGetCode(type))
        {
            if (special_block_cache_index != NULL)
            {
                *special_block_cache_index = i;
            }
            return TRUE32;
        }
    }
    return FALSE32;
}

static UInt16
_countTypeOfBlocksInCache
    (PPNFPart_MainCxt *mcxt,
    UInt16 type)
{
    UInt16 i, count = 0;

    for (i = 0; i < mcxt->special_blocks_idx; i++)
    {
        if (andSBGetCode(mcxt->special_blocks[i].type) == andSBGetCode(type))
        {
            count++;
        }
    }
    return count;
}

static BOOL32
_isSpecialBlockInCache
    (PPNFPart_MainCxt *mcxt,
    UInt16 bank,
    UInt16 block,
    UInt16 type)
{
    UInt16 i;

    for (i = 0; i < mcxt->special_blocks_idx; i++)
    {
        if ((mcxt->special_blocks[i].bank == bank) &&
            (mcxt->special_blocks[i].block == block) &&
            (andSBGetCode(mcxt->special_blocks[i].type) == andSBGetCode(type)))
        {
            return TRUE32;
        }
    }
    return FALSE32;
}

static BOOL32
_addSingleSpecialBlockToCache
    (PPNFPart_MainCxt *mcxt,
    UInt16 bank,
    UInt16 block,
    UInt16 type)
{
    UInt16 i;

    // check if the block is in the list (and ignore the add call if it is)
    if (_isSpecialBlockInCache(mcxt, bank, block, type))
    {
        return TRUE32;
    }

    // make sure we have space
    if (mcxt->special_blocks_idx >= AND_MAX_SPECIAL_BLOCKS)
    {
        WMR_PRINT(ERROR, TEXT("[FPART:ERR] no more space in special blcok list index = %d (l:%d)\n"),
                       mcxt->special_blocks_idx, __LINE__);
        return FALSE32;
    }

    // add new block to the list
    WMR_ASSERT(bank < mcxt->dev.num_of_banks);
    WMR_ASSERT(block < mcxt->dev.blocks_per_cau);

    mcxt->special_blocks[mcxt->special_blocks_idx].bank = bank;
    mcxt->special_blocks[mcxt->special_blocks_idx].block = block;
    mcxt->special_blocks[mcxt->special_blocks_idx].type = type;
    mcxt->special_blocks_idx++;
    for (i = (mcxt->special_blocks_idx - 1); i > 0; i--)
    {
        SpecialBlockCacheEntryType temp_sb_entry;
        if (mcxt->special_blocks[i].type > mcxt->special_blocks[i - 1].type)
        {
            WMR_MEMCPY(&temp_sb_entry, &mcxt->special_blocks[i], sizeof(SpecialBlockCacheEntryType));
            WMR_MEMCPY(&mcxt->special_blocks[i], &mcxt->special_blocks[i - 1], sizeof(SpecialBlockCacheEntryType));
            WMR_MEMCPY(&mcxt->special_blocks[i - 1], &temp_sb_entry, sizeof(SpecialBlockCacheEntryType));
        }
        else if (mcxt->special_blocks[i].type == mcxt->special_blocks[i - 1].type)
        {
            if (mcxt->special_blocks[i].bank > mcxt->special_blocks[i - 1].bank)
            {
                WMR_MEMCPY(&temp_sb_entry, &mcxt->special_blocks[i], sizeof(SpecialBlockCacheEntryType));
                WMR_MEMCPY(&mcxt->special_blocks[i], &mcxt->special_blocks[i - 1], sizeof(SpecialBlockCacheEntryType));
                WMR_MEMCPY(&mcxt->special_blocks[i - 1], &temp_sb_entry, sizeof(SpecialBlockCacheEntryType));
            }
            else if (mcxt->special_blocks[i].bank == mcxt->special_blocks[i - 1].bank)
            {
                if (mcxt->special_blocks[i].block > mcxt->special_blocks[i - 1].block)
                {
                    WMR_MEMCPY(&temp_sb_entry, &mcxt->special_blocks[i], sizeof(SpecialBlockCacheEntryType));
                    WMR_MEMCPY(&mcxt->special_blocks[i], &mcxt->special_blocks[i - 1],
                               sizeof(SpecialBlockCacheEntryType));
                    WMR_MEMCPY(&mcxt->special_blocks[i - 1], &temp_sb_entry, sizeof(SpecialBlockCacheEntryType));
                }
            }
        }
    }
    return TRUE32;
}

#if !AND_READONLY

static void
_removeSingleSpecialBlockFromCache
    (PPNFPart_MainCxt *mcxt,
    UInt16 bank,
    UInt16 block)
{
    UInt16 i;
    BOOL32 found_block = FALSE32;

    // mcxt->special_blocks is an ordered list
    // removing an item from the list should cause the other items to be pulled in (and keep the order)

    for (i = 0; i < mcxt->special_blocks_idx; i++)
    {
        if ((bank == mcxt->special_blocks[i].bank) && (block == mcxt->special_blocks[i].block))
        {
            mcxt->special_blocks_idx--;
            if (i == mcxt->special_blocks_idx)
            {
                // if we remove the last item in the list then we are done
                break;
            }
            found_block = TRUE32;
        }
        if (found_block)
        {
            WMR_MEMCPY(&mcxt->special_blocks[i], &mcxt->special_blocks[i + 1], sizeof(SpecialBlockCacheEntryType));
        }
    }
}

#endif // !AND_READONLY

static BOOL32
_addSpecialBlocksToCache
    (PPNFPart_MainCxt *mcxt,
    SpecialBlockAddress * blocks,
    UInt16 block_count,
    UInt16 type)
{
    UInt16 i;

    for (i = 0; i < block_count; i++)
    {
        if (!_addSingleSpecialBlockToCache(mcxt, blocks[i].bank, blocks[i].block, type))
        {
            WMR_PRINT(ERROR, TEXT("[FPART:ERR] failed to add special block (bank:0x%X, block:0x%X, type:0x%X)(l:%d)\n"),
                           blocks[i].bank, blocks[i].block, type, __LINE__);
            return FALSE32;
        }
    }

    return TRUE32;
}

static BOOL32
_findSpecialBlock
    (PPNFPart_MainCxt *mcxt,
    UInt16 *special_block_cache_index,
    UInt16 type)
{
    Buffer *buf = NULL;
    UInt16 block, bank;
    PPNStatusType status;
    UInt8 **bbt = NULL;
    BOOL32 res = FALSE32;

    // try the cache first
    if (_findSpecialBlockInCache(mcxt, special_block_cache_index, type) == TRUE32)
    {
        return TRUE32;
    }

    if (mcxt->done_mapping == TRUE32)
    {
        return FALSE32;
    }

    buf = BUF_Get(BUF_MAIN_AND_SPARE);

    // read the BBTs
    bbt = (UInt8**)WMR_MALLOC(sizeof(UInt8*) * mcxt->dev.num_of_banks);
    WMR_MEMSET(bbt, 0, (sizeof(UInt8*) * mcxt->dev.num_of_banks));

    for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
    {
        bbt[bank] = (UInt8*)WMR_MALLOC(sizeof(UInt8) * ((mcxt->dev.blocks_per_cau >> 3) + 1));
        status = ppnMiscSingleOperation(&mcxt->dev, mcxt->command, PPN_COMMAND_CAUBBT,
                                        PPN_NO_OPTIONS, bank, 0, 0, FALSE32, buf->pData, NULL);
        if (status & PPN_OTHERS_STATUS_OP_FAILED)
        {
            goto clean_and_return;
        }
        WMR_MEMCPY(bbt[bank], buf->pData, (sizeof(UInt8) * ((mcxt->dev.blocks_per_cau >> 3) + 1)));
    }

    for (block = mcxt->dev.blocks_per_cau - 1; block >= PPN_SPECIAL_BLOCK_LIMIT(&mcxt->dev); block--)
    {
        for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
        {
            UInt16 page;

            status = 0;

            if (!ppnMiscIsBlockGood(bbt[bank], block))
            {
                continue;
            }

            for (page = 0; page < mcxt->dev.pages_per_block_slc &&
                 (!(status & PPN_READ_STATUS_CLEAN)); page++)
            {
                status = ppnMiscSingleOperation(&mcxt->dev, mcxt->command,
                                                PPN_COMMAND_READ, PPN_NO_OPTIONS,
                                                bank, block, page, TRUE32,
                                                buf->pData, buf->pSpare);
                if (!(status & PPN_READ_STATUS_INVALID_DATA))
                {
                    SpecialBlockHeaderStruct *temp_header = (SpecialBlockHeaderStruct*)buf->pData;
                    PPNFPart_Spare *temp_spare = (PPNFPart_Spare *)buf->pSpare;

                    // check that this is a special block we identify
                    if (temp_spare->spare_type != PPNFPART_SPECIAL_BLOCK_SPARE_TYPE)
                    {
                        // this is not a special block, skip to the next one
                        break;
                    }
                    // this is a special block, check if this page contains a header
                    else if (temp_spare->index == 0)
                    {
                        UInt16 idx, blocks_in_header = 0;
                        BOOL32 continue_on_header_error = FALSE32;
                        SpecialBlockAddress special_block_address[PPNFPART_MAX_SPECIAL_BLOCKS_PER_TYPE];

                        // verify that what we read makes sense
                        // check that the header makes sense
                        if (_getMajorVersion(mcxt, temp_header->version) >
                            _getMajorVersion(mcxt, PPNFPART_STRUCTURE_VERSION))
                        {
                            WMR_PRINT(ERROR, TEXT("[PPNFPART:ERR] special block header version does not match (0x%08X)\n"),
                                           temp_header->version);
                            continue_on_header_error = TRUE32;
                        }

                        blocks_in_header = _countBlocksInHeader(mcxt, temp_header);

                        for (idx = 0; ((idx < blocks_in_header) &&
                                       (!continue_on_header_error)); idx++)
                        {
                            if ((temp_header->block_addresses[idx].bank >= mcxt->dev.num_of_banks) ||
                                (temp_header->block_addresses[idx].block >= mcxt->dev.blocks_per_cau))
                            {
                                WMR_PRINT(ERROR, TEXT("[PPNFPART:ERR] SB header address is out of range (0x%04X, 0x)\n"),
                                               temp_header->block_addresses[idx].bank,
                                               temp_header->block_addresses[idx].block);
                                continue_on_header_error = TRUE32;
                            }
                        }

                        if (continue_on_header_error)
                        {
                            continue;
                        }

                        WMR_MEMSET(special_block_address, 0xFF,
                                   (PPNFPART_MAX_SPECIAL_BLOCKS_PER_TYPE * sizeof(SpecialBlockAddress)));

                        for (idx = 0; idx < blocks_in_header; idx++)
                        {
                            special_block_address[idx].bank = temp_header->block_addresses[idx].bank;
                            special_block_address[idx].block = temp_header->block_addresses[idx].block;
                        }

                        if (andSBGetCode(temp_spare->vfl_special_type) == andSBGetCode(type))
                        {
                            _addSpecialBlocksToCache(mcxt, special_block_address, blocks_in_header,
                                                     temp_spare->vfl_special_type);
                            if (special_block_cache_index != NULL)
                            {
                                _findSpecialBlockInCache(mcxt, special_block_cache_index, type);
                            }
                            res = TRUE32;
                            goto clean_and_return;
                        }

                        if (!_findSpecialBlockInCache(mcxt, NULL, temp_spare->vfl_special_type))
                        {
                            _addSpecialBlocksToCache(mcxt, special_block_address, blocks_in_header,
                                                     temp_spare->vfl_special_type);
                        }

                        // if we got here, the special block was properly processed
                        break;
                    }
                }
            }
        }
    }

    mcxt->done_mapping = TRUE32;

 clean_and_return:

    if (buf != NULL)
    {
        BUF_Release(buf);
    }

    for (bank = 0; (bbt != NULL) && (bank < mcxt->dev.num_of_banks); bank++)
    {
        if (bbt[bank] != NULL)
        {
            WMR_FREE(bbt[bank], (sizeof(UInt8) * ((mcxt->dev.blocks_per_cau >> 3) + 1)));
        }
    }

    if (bbt != NULL)
    {
        WMR_FREE(bbt, (sizeof(UInt8*) * mcxt->dev.num_of_banks));
    }

    return res;
}

// _readSpecialBlockSingleVersionFromSingleRedundantCopy - reads a single version type special block
// from one of the redundant blocks.

static BOOL32
_readSpecialBlockSingleVersionFromSingleRedundantCopy
    (PPNFPart_MainCxt *mcxt,
    void *data,
    UInt32 data_size,
    SpecialBlockCacheEntryType *sb_address,
    UInt32 *copy_number) // increased by one every time the speical block is written
{
    Buffer *buf;
    UInt16 valid_pages;
    PPNStatusType status;
    UInt8  *data_uint8;
    UInt32 pages_per_copy = 1, size_to_copy = 0, number_of_copies = mcxt->dev.pages_per_block_slc;
    PPNFPart_Spare spare;

    data_uint8 = (UInt8*)data;

    buf = BUF_Get(BUF_MAIN_AND_SPARE);

    for (valid_pages = 0; valid_pages < pages_per_copy; valid_pages++)
    {
        BOOL32 data_read = FALSE32;
        UInt16 copy_to_read_from;
        for (copy_to_read_from = 0; (copy_to_read_from < number_of_copies &&
                                     !data_read); copy_to_read_from++)
        {
            UInt16 page_offset = (copy_to_read_from * pages_per_copy) + valid_pages;

            status = ppnMiscSingleOperation(&mcxt->dev, mcxt->command,
                                            PPN_COMMAND_READ, PPN_NO_OPTIONS,
                                            sb_address->bank,
                                            sb_address->block,
                                            page_offset,
                                            TRUE32, buf->pData, buf->pSpare);
            if (!(status & PPN_READ_STATUS_INVALID_DATA))
            {
                UInt32 data_offset, data_to_copy;
                _copySpareFPartCxtFromMeta(mcxt, &spare, buf->pSpare);
                if (valid_pages == 0)
                {
                    SpecialBlockHeaderStruct *header = (SpecialBlockHeaderStruct *)buf->pData;

                    // check that this is really what we are looking for
                    if (spare.vfl_special_type == sb_address->type &&
                        spare.spare_type == PPNFPART_SPECIAL_BLOCK_SPARE_TYPE &&
                        spare.index == 0)
                    {
                        if (copy_number != NULL)
                        {
                            *copy_number = header->copy_number;
                        }
                        if (data_uint8 != NULL)
                        {
                            size_to_copy = WMR_MIN(header->data_size, data_size);
                        }
                        else
                        {
                            size_to_copy = 0;
                        }
                        pages_per_copy = (((size_to_copy + sizeof(SpecialBlockHeaderStruct)) /
                                           mcxt->dev.main_bytes_per_page) +
                                          (((size_to_copy + sizeof(SpecialBlockHeaderStruct)) %
                                            mcxt->dev.main_bytes_per_page) ? 1 : 0));
                        number_of_copies = mcxt->dev.pages_per_block_slc / pages_per_copy;
                    }
                    else
                    {
                        continue;
                    }
                    data_to_copy = WMR_MIN((mcxt->dev.main_bytes_per_page -
                                            sizeof(SpecialBlockHeaderStruct)),
                                           size_to_copy);
                    data_offset = sizeof(SpecialBlockHeaderStruct);

                    if (data_to_copy && data_uint8 != NULL)
                    {
                        WMR_MEMCPY(&data_uint8[0], &buf->pData[data_offset], data_to_copy);
                    }
                    data_read = TRUE32;
                    break;
                }
                else
                {
                    if (spare.vfl_special_type == sb_address->type &&
                        spare.spare_type == PPNFPART_SPECIAL_BLOCK_SPARE_TYPE &&
                        spare.index == (UInt8)valid_pages)
                    {
                        data_offset = ((valid_pages * mcxt->dev.main_bytes_per_page) -
                                       sizeof(SpecialBlockHeaderStruct));
                        data_to_copy = WMR_MIN(mcxt->dev.main_bytes_per_page,
                                               (size_to_copy - data_offset));

                        WMR_MEMCPY(&data_uint8[data_offset], buf->pData, data_to_copy);
                        data_read = TRUE32;
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
        }
        if (!data_read)
        {
            goto return_error;
        }
    }
    BUF_Release(buf);
    return TRUE32;

 return_error:
    BUF_Release(buf);
    return FALSE32;
}

static BOOL32
_readSpecialBlockSingleVersion
    (PPNFPart_MainCxt *mcxt,
    void *data,
    UInt32 data_size,
    UInt16 special_block_cache_index,
    UInt32 *copy_number) // increased by one every time the speical block is written
{
    UInt16 sb_count, sb_type, block_idx, known_good_index = 0xFFFF;
    UInt32 local_copy_number = 0, curr_copy_number = 0;
    SpecialBlockCacheEntryType *sb_addresses;
    BOOL32 read_valid_copy = FALSE32;

    sb_type = mcxt->special_blocks[special_block_cache_index].type;
    sb_count = _countTypeOfBlocksInCache(mcxt, sb_type);
    sb_addresses = &mcxt->special_blocks[special_block_cache_index];

    // loop through the different redundat copies to find the latest version
    for (block_idx = 0; block_idx < sb_count; block_idx++)
    {
        BOOL32 status_reading_from_this_block = FALSE32;
        if (read_valid_copy)
        {
            status_reading_from_this_block =
                _readSpecialBlockSingleVersionFromSingleRedundantCopy(mcxt, NULL, 0,
                                                                      &sb_addresses[block_idx],
                                                                      &local_copy_number);
        }
        else
        {
            status_reading_from_this_block =
                _readSpecialBlockSingleVersionFromSingleRedundantCopy(mcxt, data, data_size,
                                                                      &sb_addresses[block_idx],
                                                                      &local_copy_number);
        }

        if (status_reading_from_this_block)
        {
            if (!read_valid_copy)
            {
                read_valid_copy = TRUE32;
                known_good_index = block_idx;
                curr_copy_number = local_copy_number;
            }
            else
            {
                if (local_copy_number != curr_copy_number)
                {
                    WMR_PRINT(ALWAYS, TEXT(
                                       "[PPNFPART:WRN] special block with more than one version (type:0x%04X) (line:%d)!\n"),
                                   sb_type, __LINE__);
                }
                if (local_copy_number > curr_copy_number)
                {
                    if (_readSpecialBlockSingleVersionFromSingleRedundantCopy(mcxt, data, data_size,
                                                                              &sb_addresses[block_idx],
                                                                              &local_copy_number))
                    {
                        WMR_PRINT(ALWAYS, TEXT(
                                           "[PPNFPART:WRN] successfully copied new SB version (new:0x08X, old:0x08X, type:0x%04X) (line:%d)!\n"),
                                       local_copy_number, curr_copy_number, sb_type, __LINE__);
                        curr_copy_number = local_copy_number;
                        known_good_index = block_idx;
                    }
                    else
                    {
                        WMR_PRINT(ALWAYS, TEXT(
                                           "[PPNFPART:WRN] failed to copy new SB version (new:0x08X, old:0x08X, type:0x%04X) (line:%d)!\n"),
                                       local_copy_number, curr_copy_number, sb_type, __LINE__);

                        // read the copy of the SB we read successfully earlier
                        if (!_readSpecialBlockSingleVersionFromSingleRedundantCopy(mcxt, data, data_size,
                                                                                   &sb_addresses[known_good_index],
                                                                                   &local_copy_number))
                        {
                            // if a good copy became bad, fail here
                            WMR_PRINT(ERROR, TEXT(
                                               "[PPNFPART:ERR] failed to read SB from known good copy (type:0x%04X) (line:%d)!\n"),
                                           sb_type, __LINE__);
                            return FALSE32;
                        }
                    }
                }
            }
        }
        else
        {
            WMR_PRINT(ALWAYS, TEXT(
                               "[PPNFPART:WRN] failed to read from assigned SB copy (bank:0x%04X, block:0x%04X, type:0x%04X) (line:%d)!\n"),
                           sb_addresses[block_idx].bank, sb_addresses[block_idx].block, sb_type, __LINE__);
        }
    }
    if (read_valid_copy && (copy_number != NULL))
    {
        *copy_number = local_copy_number;
    }
    return read_valid_copy;
}

#ifndef AND_READONLY
static BOOL32
_writeSpecialBlockSingleVersion
    (PPNFPart_MainCxt *mcxt,
    void *data,
    UInt32 data_size,
    UInt16 special_block_cache_index)
{
    const UInt16 pages_per_copy = (((data_size + sizeof(SpecialBlockHeaderStruct)) /
                                    mcxt->dev.main_bytes_per_page) +
                                   (((data_size + sizeof(SpecialBlockHeaderStruct)) %
                                     mcxt->dev.main_bytes_per_page) ? 1 : 0));
    const UInt16 redundancy_per_block = (mcxt->dev.pages_per_block_slc / pages_per_copy);
    Buffer *buf;
    UInt16 sb_count, page_offset, sb_type, replace_block_index = 0xFFFF;
    UInt8  *data_uint8;
    SpecialBlockCacheEntryType *sb_addresses;
    SpecialBlockHeaderStruct header;
    PPNFPart_Spare spare;
    UInt32 copy_number = 0;
    BOOL32 read_previous_copy_status, replace_bad_block = FALSE32;
    BOOL32 done_writing_per_copy[PPNFPART_MAX_SPECIAL_BLOCKS_PER_TYPE] = { FALSE32 };

    data_uint8 = (UInt8*)data;
    buf = BUF_Get(BUF_MAIN_AND_SPARE);
    sb_type = mcxt->special_blocks[special_block_cache_index].type;
    sb_count = _countTypeOfBlocksInCache(mcxt, sb_type);
    sb_addresses = &mcxt->special_blocks[special_block_cache_index];

    // try to read the previous version of the sb
    read_previous_copy_status = _readSpecialBlockSingleVersion(mcxt, NULL, 0, special_block_cache_index, &copy_number);
    if (!read_previous_copy_status)
    {
        copy_number = 0;
    }
    else
    {
        copy_number++;
    }
    WMR_PRINT(ALWAYS, "Writing special block type:0x%04X copy_number:0x%08X\n", sb_type, copy_number);
    // prepare the header
    _initSpecialBlockHeader(&header, sb_addresses, sb_count, data_size, copy_number);

    do
    {
        UInt32 sb_start_index = 0, sb_copy = 0;
        if (replace_bad_block)
        {
            SpecialBlockAddress bad_block, new_block;
            VFLFunctions *_vfl;
            PPNStatusType status;

            bad_block.bank = sb_addresses[replace_block_index].bank;
            bad_block.block = sb_addresses[replace_block_index].block;

            // erase the bad block and verify no stale data
            ppnMiscSingleOperation(&mcxt->dev, mcxt->command, PPN_COMMAND_ERASE,
                                   PPN_NO_OPTIONS, bad_block.bank, bad_block.block,
                                   0, TRUE32, NULL, NULL);

            for (page_offset = 0; page_offset < mcxt->dev.pages_per_block_slc; page_offset++)
            {
                PPNFPart_Spare *spare = (PPNFPart_Spare*)buf->pSpare;
                ((PPNFPart_Spare*)buf->pSpare)->spare_type = PPNFPART_SPECIAL_BLOCK_SPARE_TYPE;
                status = ppnMiscSingleOperation(&mcxt->dev, mcxt->command, PPN_COMMAND_READ,
                                                PPN_NO_OPTIONS, bad_block.bank, bad_block.block,
                                                page_offset, TRUE32, buf->pData, buf->pSpare);
                if ((!(status & PPN_READ_STATUS_INVALID_DATA)) &&
                    (spare->spare_type == PPNFPART_SPECIAL_BLOCK_SPARE_TYPE))
                {
                    WMR_PRINT(ERROR, TEXT(
                                       "[PPNFPART:ERR] after erasing a failed block twice it has stale valid data (bank:0x%X, block:0x%X) (line:%d)!\n"),
                                   bad_block.bank, bad_block.block, __LINE__);
                    goto return_error;
                }
            }

            // allocate replacement block, mark this block as grown bad
            _vfl = WMR_PPN_GetVFL();
            if (_vfl == NULL)
            {
                WMR_PRINT(ERROR, TEXT("[PPNFPART:ERR] failed to find vfl (bank:0x%X, block:0x%X) fail (line:%d)!\n"),
                               bad_block.bank, bad_block.block, __LINE__);
                goto return_error;
            }
            if (!_vfl->MarkBlockAsGrownBad(&bad_block))
            {
                WMR_PRINT(ERROR, TEXT(
                                   "[PPNFPART:ERR] failed to mark block as grown bad (bank:0x%X, block:0x%X) fail (line:%d)!\n"),
                               bad_block.bank, bad_block.block, __LINE__);
                goto return_error;
            }
            _removeSingleSpecialBlockFromCache(mcxt, bad_block.bank, bad_block.block);
            if (!_vfl->AllocateSpecialBlock(&new_block, sb_type))
            {
                WMR_PRINT(ERROR, TEXT(
                                   "[PPNFPART:ERR] failed to mark block as grown bad (bank:0x%X, block:0x%X) fail (line:%d)!\n"),
                               bad_block.bank, bad_block.block, __LINE__);
                goto return_error;
            }
            sb_addresses[replace_block_index].bank = new_block.bank;
            sb_addresses[replace_block_index].block = new_block.block;
            sb_start_index = replace_block_index;
            replace_block_index = 0xFFFF;
            replace_bad_block = FALSE32;
            copy_number++; // change the copy number
            _initSpecialBlockHeader(&header, sb_addresses, sb_count, data_size, copy_number);
            WMR_MEMSET(done_writing_per_copy, 0, sizeof(BOOL32) * PPNFPART_MAX_SPECIAL_BLOCKS_PER_TYPE);
        }

        for (sb_copy = sb_start_index; (!done_writing_per_copy[sb_copy]) && (!replace_bad_block);
             sb_copy = ((sb_copy + 1) % sb_count))
        {
            UInt16 i, j;
            PPNStatusType status;
            status = ppnMiscSingleOperation(&mcxt->dev, mcxt->command, PPN_COMMAND_ERASE, PPN_NO_OPTIONS,
                                            sb_addresses[sb_copy].bank, sb_addresses[sb_copy].block,
                                            0, TRUE32, NULL, NULL);
            if (status & PPN_ERASE_STATUS_FAIL)
            {
                WMR_PRINT(ALWAYS, TEXT("[PPNFPART:WRN] erasing special block (bank:0x%X, block:0x%X) fail (line:%d)!\n"),
                               sb_addresses[sb_copy].bank, sb_addresses[sb_copy].block, __LINE__);

                replace_block_index = sb_copy;
                replace_bad_block = TRUE32;
                break;
            }

            page_offset = 0;
            for (i = 0; (i < redundancy_per_block) && (!replace_bad_block); i++)
            {
                for (j = 0; (j < pages_per_copy) && (!replace_bad_block); j++)
                {
                    UInt16 data_offset = (j == 0 ? 0 : ((j * mcxt->dev.main_bytes_per_page) -
                                                        sizeof(SpecialBlockHeaderStruct)));
                    UInt16 data_to_copy = (j == 0 ?
                                           WMR_MIN((mcxt->dev.main_bytes_per_page - sizeof(SpecialBlockHeaderStruct)),
                                                   data_size) :
                                           WMR_MIN(mcxt->dev.main_bytes_per_page,
                                                   (data_size - data_offset)));

                    WMR_MEMSET(buf->pData, 0x00, mcxt->dev.main_bytes_per_page);
                    if (j == 0)
                    {
                        WMR_MEMCPY(buf->pData, &header, sizeof(SpecialBlockHeaderStruct));
                        WMR_MEMCPY(&buf->pData[sizeof(SpecialBlockHeaderStruct)], data, data_to_copy);
                    }
                    else
                    {
                        WMR_MEMCPY(buf->pData, &data_uint8[data_offset], data_to_copy);
                    }

                    _setSpareFPartCxt(mcxt, &spare, sb_type, (UInt8)j);
                    _copySpareFPartCxtToMeta(mcxt, &spare, buf->pSpare);

                    status = ppnMiscSingleOperation(&mcxt->dev, mcxt->command,
                                                    PPN_COMMAND_PROGRAM, PPN_NO_OPTIONS,
                                                    sb_addresses[sb_copy].bank,
                                                    sb_addresses[sb_copy].block,
                                                    page_offset, TRUE32,
                                                    buf->pData, buf->pSpare);
                    if (status & PPN_PROGRAM_STATUS_FAIL)
                    {
                        WMR_PRINT(ALWAYS, TEXT(
                                           "[PPNFPART:WRN] programming special block (bank:0x%X, block:0x%X) fail (line:%d)!\n"),
                                       sb_addresses[i].bank, sb_addresses[i].block, __LINE__);
                        replace_block_index = sb_copy;
                        replace_bad_block = TRUE32;
                    }
                    page_offset++;
                }
            }

            // program unused pages in the block
            while ((page_offset < mcxt->dev.pages_per_block_slc) && (!replace_bad_block))
            {
                WMR_MEMSET(buf->pData, 0x00, mcxt->dev.main_bytes_per_page);
                _setSpareFPartCxt(mcxt, &spare, sb_type, 0xFF);
                _copySpareFPartCxtToMeta(mcxt, &spare, buf->pSpare);
                status = ppnMiscSingleOperation(&mcxt->dev, mcxt->command,
                                                PPN_COMMAND_PROGRAM, PPN_NO_OPTIONS,
                                                sb_addresses[sb_copy].bank,
                                                sb_addresses[sb_copy].block,
                                                page_offset, TRUE32,
                                                buf->pData, buf->pSpare);
                if (status & PPN_PROGRAM_STATUS_FAIL)
                {
                    WMR_PRINT(ALWAYS, TEXT(
                                       "[PPNFPART:WRN] programming special block (bank:0x%X, block:0x%X) fail (line:%d)!\n"),
                                   sb_addresses[sb_copy].bank, sb_addresses[sb_copy].block, __LINE__);
                    replace_block_index = sb_copy;
                    replace_bad_block = TRUE32;
                }
                page_offset++;
            }

            if (!replace_bad_block)
            {
                done_writing_per_copy[sb_copy] = TRUE32;
            }
        }
    }
    while (replace_bad_block);

    BUF_Release(buf);
    return TRUE32;

 return_error:
    BUF_Release(buf);
    return FALSE32;
}
#endif // ! AND_READONLY

static BOOL32
_ppnfpartReadSpecialBlock
    (PPNFPart_MainCxt *mcxt,
    void *data,
    UInt32 data_size,
    UInt16 type)
{
    UInt16 special_block_cache_index;
    BOOL32 res = FALSE32;

    // find the block addresses
    if (_findSpecialBlock(mcxt, &special_block_cache_index, type) == FALSE32)
    {
        WMR_PRINT(ALWAYS, TEXT("[PPNVFL:WRN] Read: failed to locate special blocks (type = 0x%X) (line:%d)!\n"),
                       type, __LINE__);
        return FALSE32;
    }
    switch (andSBGetHandler(mcxt->special_blocks[special_block_cache_index].type))
    {
        case AND_SB_HANDLER_SINGLE_VERSION:
            res = _readSpecialBlockSingleVersion(mcxt, data, data_size, special_block_cache_index, NULL);
            break;

        default:
            WMR_PRINT(ERROR, TEXT("[PPNVFL:ERR] no support for the special block handler (type = 0x%X) (line:%d)!\n"),
                           type, __LINE__);
            break;
    }
    return res;
}

#ifndef AND_READONLY
static BOOL32
_ppnfpartWriteSpecialBlock
    (PPNFPart_MainCxt *mcxt,
    void *data,
    UInt32 data_size,
    UInt16 type)
{
    UInt16 special_block_cache_index;
    BOOL32 res = FALSE32;

    // find the block addresses
    if (!_findSpecialBlock(mcxt, &special_block_cache_index, type))
    {
        WMR_PRINT(ERROR, TEXT("[PPNVFL:ERR] Write: failed to locate special blocks (type = 0x%X) (line:%d)!\n"),
                       type, __LINE__);
        return FALSE32;
    }
    switch (andSBGetHandler(mcxt->special_blocks[special_block_cache_index].type))
    {
        case AND_SB_HANDLER_SINGLE_VERSION:
            res = _writeSpecialBlockSingleVersion(mcxt, data, data_size, special_block_cache_index);
            break;

        default:
            WMR_PRINT(ERROR, TEXT("[PPNVFL:ERR] no support for the special block handler (type = 0x%X) (line:%d)!\n"),
                           type, __LINE__);
            break;
    }
    return res;
}
#endif // !AND_READONLY

#if !AND_FPART_ONLY
static BOOL32
_ppnfpartMapSpecialBlocks
    (PPNFPart_MainCxt *mcxt,
    SpecialBlockAddress *blocks,
    UInt16 *types,
    UInt16 *count)
{
    UInt16 i;

    // look for none existing type (marking all the types we find on the way)
    _findSpecialBlock(mcxt, NULL, 0);

    if (count != NULL && mcxt->special_blocks_idx != 0)
    {
        *count = mcxt->special_blocks_idx;
    }
    else
    {
        return FALSE32;
    }

    if (count != NULL && blocks != NULL && types != NULL)
    {
        for (i = 0; i < mcxt->special_blocks_idx; i++)
        {
            blocks[i].bank = mcxt->special_blocks[i].bank;
            blocks[i].block = mcxt->special_blocks[i].block;
            types[i] = mcxt->special_blocks[i].type;
        }
    }

    return TRUE32;
}
#endif //!AND_FPART_ONLY

#if (!AND_FPART_ONLY) || (!defined(AND_READONLY))
static BOOL32
_ppnfpartAllocateSpecialBlockType
    (PPNFPart_MainCxt *mcxt,
    SpecialBlockAddress *blocks,
    UInt16 blocks_count,
    UInt16 type)
{
    return _addSpecialBlocksToCache(mcxt, blocks, blocks_count, type);
}

static BOOL32
_ppnfpartIsSpecialBlockTypeAllocated
    (PPNFPart_MainCxt *mcxt,
    UInt16 type)
{
    return _findSpecialBlock(mcxt, NULL, type);
}
#endif //(!AND_FPART_ONLY) || (!defined(AND_READONLY))

#ifndef AND_READONLY

#define DEFAULT_PERSISTENT_BLOCK_DATA_SIZE (8192)
static BOOL32
_WriteDefaultPersistentSpecialBlock
    (PPNFPart_MainCxt * mcxt,
    VFLFunctions *vfl,
    UInt16 special_block_type)
{
    BOOL32 retval = TRUE32;
    void *buffer = NULL;

    if (!_ppnfpartIsSpecialBlockTypeAllocated(mcxt, special_block_type))
    {
        SpecialBlockAddress chosen_blocks[2];

        // Need to allocate a constant size buffer,
        // indpendent of physical page size
        buffer = WMR_MALLOC(DEFAULT_PERSISTENT_BLOCK_DATA_SIZE);
        WMR_ASSERT(buffer != NULL);

        WMR_PRINT(INIT, "allocating 0x%04x\n", (UInt32)special_block_type);

        if (!vfl->AllocateSpecialBlock(&chosen_blocks[0], special_block_type) ||
            !vfl->AllocateSpecialBlock(&chosen_blocks[1], special_block_type))
        {
            WMR_PRINT(ERROR, "VFL Failed to allocate special block\n");
            retval = FALSE32;
            goto done;
        }

        if (!_ppnfpartAllocateSpecialBlockType(mcxt, chosen_blocks, 2, special_block_type))
        {
            WMR_PRINT(ERROR, "Failed to assign special blocks to FPart\n");
            retval = FALSE32;
            goto done;
        }

        WMR_MEMSET(buffer, 0xFF, DEFAULT_PERSISTENT_BLOCK_DATA_SIZE);

        if (!_ppnfpartWriteSpecialBlock(mcxt, buffer, DEFAULT_PERSISTENT_BLOCK_DATA_SIZE, special_block_type))
        {
            retval = FALSE32;
            goto done;
        }
    }

 done:
    if (NULL != buffer)
    {
        WMR_FREE(buffer, DEFAULT_PERSISTENT_BLOCK_DATA_SIZE);
    }
    return retval;
}

static BOOL32
_ppnfpartFormat
    (PPNFPart_MainCxt *mcxt,
    VFLFunctions *vfl,
    UInt32 format_options)
{
    if ((format_options & FPART_INIT_OPTION_DEV_UNIQUE) &&
        !_WriteDefaultPersistentSpecialBlock(mcxt, vfl, AND_SB_TYPE_UNIQUE_INFO))
    {
        return FALSE32;
    }

    if ((format_options & FPART_INIT_OPTION_DIAG_CTRL) &&
        !_WriteDefaultPersistentSpecialBlock(mcxt, vfl, AND_SB_TYPE_DIAGNOSTICS))
    {
        return FALSE32;
    }

    return TRUE32;
}

#if AND_SUPPORT_NEURALIZE
static Int32
_ppnfpartNeuralize
    (PPNFPart_MainCxt *mcxt)
{
    UInt16 i = 0, j = 0;
    Buffer *buf;

    buf = BUF_Get(BUF_MAIN_AND_SPARE);
    for (i = 0; i < mcxt->dev.num_of_banks; i++)
    {
        PPNStatusType status;

        // get the original BBT
        status = ppnMiscSingleOperation(&mcxt->dev, mcxt->command, PPN_COMMAND_CAUBBT,
                                        PPN_NO_OPTIONS, i, 0, 0, FALSE32, buf->pData, NULL);
        if (status & PPN_OTHERS_STATUS_OP_FAILED)
        {
            WMR_PRINT(ERROR, "Unable to fetch BBT\n");
            BUF_Release(buf);
            return ANDErrorCodeHwErr;
        }

        for (j = 0; j < mcxt->dev.blocks_per_cau; j++)
        {
            if (ppnMiscIsBlockGood(buf->pData, j) == TRUE32)
            {
                status = ppnMiscSingleOperation(&mcxt->dev, mcxt->command, PPN_COMMAND_ERASE,
                                                PPN_NO_OPTIONS, i, j, 0, FALSE32, NULL, NULL);
            }
        }
    }
    BUF_Release(buf);
    return ANDErrorCodeOk;
}
#endif //AND_SUPPORT_NEURALIZE

#endif //!AND_READONLY

static BOOL32
_g_ppnfpartInit
    (LowFuncTbl *pFILFunctions,
    UInt32 dwOptions)
{
    if (g_ppnfpartMain == NULL)
    {
        g_ppnfpartMain = (PPNFPart_MainCxt*)WMR_MALLOC(sizeof(PPNFPart_MainCxt));
        if (g_ppnfpartMain == NULL)
        {
            WMR_PRINT(ERROR, TEXT("[PPNFPART:ERR] failed allocating g_ppnfpartMain (line:%d))\n"), __LINE__);
            return FALSE32;
        }
        WMR_MEMSET(g_ppnfpartMain, 0, sizeof(PPNFPart_MainCxt));
    }
    return _ppnfpartInit(g_ppnfpartMain, pFILFunctions, dwOptions);
}

BOOL32
_g_ppnfpartReadSpecialBlock
    (void *data,
    UInt32 data_size,
    UInt16 type)
{
    return _ppnfpartReadSpecialBlock(g_ppnfpartMain, data, data_size, type);
}

static BOOL32
_g_ppnfpartReadDeviceUniqueInfo
    (UInt8 * data, UInt32 data_size)
{
    return _ppnfpartReadSpecialBlock(g_ppnfpartMain, data, data_size, AND_SB_TYPE_UNIQUE_INFO);
}

static BOOL32
_g_ppnfpartReadDiagnosticInfo
    (UInt8 * data, UInt32 data_size)
{
    return _ppnfpartReadSpecialBlock(g_ppnfpartMain, data, data_size, AND_SB_TYPE_DIAGNOSTICS);
}

#ifndef AND_READONLY
static BOOL32
_g_ppnfpartWriteSpecialBlock
    (void *data,
    UInt32 data_size,
    UInt16 type)
{
    return _ppnfpartWriteSpecialBlock(g_ppnfpartMain, data, data_size, type);
}

static BOOL32
_g_ppnfpartWriteDeviceUniqueInfo
    (UInt8 *data,
    UInt32 data_size)
{
    return _ppnfpartWriteSpecialBlock(g_ppnfpartMain, data, data_size, AND_SB_TYPE_UNIQUE_INFO);
}

static BOOL32
_g_ppnfpartWriteDiagnosticInfo
    (UInt8 *data,
    UInt32 data_size)
{
    return _ppnfpartWriteSpecialBlock(g_ppnfpartMain, data, data_size, AND_SB_TYPE_DIAGNOSTICS);
}
#endif // !AND_READONLY

#if (!defined(AND_READONLY) && AND_SUPPORT_NEURALIZE)
static BOOL32
_g_ppnfpartNeuralize
    (void)
{
    return _ppnfpartNeuralize(g_ppnfpartMain);
}
#endif //!AND_READONLY && AND_SUPPORT_NEURALIZE

#if !AND_FPART_ONLY
static BOOL32
_g_ppnfpartMapSpecialBlocks
    (SpecialBlockAddress *blocks,
    UInt16 *types,
    UInt16 *count)
{
    return _ppnfpartMapSpecialBlocks(g_ppnfpartMain, blocks, types, count);
}

static BOOL32
_g_ppnfpartAllocateSpecialBlockType
    (SpecialBlockAddress *blocks,
    UInt16 blocks_count,
    UInt16 type)
{
    return _ppnfpartAllocateSpecialBlockType(g_ppnfpartMain, blocks, blocks_count, type);
}

static BOOL32
_g_ppnfpartIsSpecialBlockTypeAllocated
    (UInt16 type)
{
    return _ppnfpartIsSpecialBlockTypeAllocated(g_ppnfpartMain, type);
}
#endif //!AND_FPART_ONLY

#ifndef AND_READONLY
static UInt32
_g_ppnfpartFormat
    (VFLFunctions *vfl,
    UInt32 format_options)
{
    return _ppnfpartFormat(g_ppnfpartMain, vfl, format_options);
}
#endif //!AND_READONLY

static UInt32
_g_ppnfpartGetMinorVersion
    (void)
{
    return kPPNFPartMinorVersion;
}

static void
_g_ppnfpartClose
    (void)
{
    if (g_ppnfpartMain != NULL)
    {
        _ppnfpartClose(g_ppnfpartMain);
        WMR_FREE(g_ppnfpartMain, sizeof(PPNFPart_MainCxt));
        g_ppnfpartMain = NULL;
    }
}

#if !AND_FPART_ONLY
static void
_g_ppnfpartChangeCacheState
    (BOOL32 validate)
{
    _ppnfpartChangeCacheState(g_ppnfpartMain, validate);
}
#endif //!AND_FPART_ONLY

void *
_g_ppnfpartGetSingleCommandStruct
    (void)
{
    if (g_ppnfpartMain == NULL)
    {
        return NULL;
    }
    return g_ppnfpartMain->command;
}

void
PPNFPart_Register
    (FPartFunctions * pFPartFunctions)
{
    WMR_MEMSET(pFPartFunctions, 0, sizeof(FPartFunctions));
    pFPartFunctions->Init = _g_ppnfpartInit;
    pFPartFunctions->GetLowFuncTbl = FIL_GetFuncTbl;
    pFPartFunctions->ReadSpecialBlock = _g_ppnfpartReadSpecialBlock;
    pFPartFunctions->Close = _g_ppnfpartClose;
    pFPartFunctions->GetMinorVersion = _g_ppnfpartGetMinorVersion;
    pFPartFunctions->ReadDeviceUniqueInfo = _g_ppnfpartReadDeviceUniqueInfo;
    pFPartFunctions->ReadDiagnosticInfo = _g_ppnfpartReadDiagnosticInfo;
#if !AND_FPART_ONLY
    pFPartFunctions->ChangeCacheState = _g_ppnfpartChangeCacheState;
    pFPartFunctions->MapSpecialBlocks = _g_ppnfpartMapSpecialBlocks;
    pFPartFunctions->IsSpecialBlockTypeAllocated = _g_ppnfpartIsSpecialBlockTypeAllocated;
    pFPartFunctions->AllocateSpecialBlockType = _g_ppnfpartAllocateSpecialBlockType;
#endif //!AND_FPART_ONLY
#ifndef AND_READONLY
    pFPartFunctions->Format = _g_ppnfpartFormat;
#if AND_SUPPORT_NEURALIZE
    pFPartFunctions->Neuralize = _g_ppnfpartNeuralize;
#endif //AND_SUPPORT_NEURALIZE
    pFPartFunctions->WriteSpecialBlock = _g_ppnfpartWriteSpecialBlock;
    pFPartFunctions->WriteDeviceUniqueInfo = _g_ppnfpartWriteDeviceUniqueInfo;
    pFPartFunctions->WriteDiagnosticInfo = _g_ppnfpartWriteDiagnosticInfo;
#endif // !AND_READONLY
    pFPartFunctions->GetSingleCommandStruct = _g_ppnfpartGetSingleCommandStruct;
}

static FPartFunctions _ppnfpart_functions;

FPartFunctions *
PPN_FPart_GetFunctions
    (void)
{
    PPNFPart_Register(&_ppnfpart_functions);
    return &_ppnfpart_functions;
}

//////////////
// start - hack to switch units from vanilla to swiss
//////////////

// _readSpecialBlockSingleVersionFromSingleRedundantCopy - reads a single version type special block
// from one of the redundant blocks.

static BOOL32
oldReadSpecialBlockSingleVersionFromSingleRedundantCopy
    (PPNFPart_MainCxt *mcxt,
    void *data,
    UInt32 data_size,
    SpecialBlockCacheEntryType *sb_address,
    UInt32 *copy_number) // increased by one every time the speical block is written
{
    Buffer *org_buf, *buf;
    UInt16 valid_pages;
    PPNStatusType status;
    UInt8  *data_uint8;
    UInt32 pages_per_copy = 1, size_to_copy = 0, number_of_copies = mcxt->dev.pages_per_block_slc;
    PPNFPart_Spare spare;

    data_uint8 = (UInt8*)data;

    org_buf = BUF_Get(BUF_MAIN_AND_SPARE);
    buf = BUF_Get(BUF_MAIN_AND_SPARE);

    for (valid_pages = 0; valid_pages < pages_per_copy; valid_pages++)
    {
        BOOL32 data_read = FALSE32;
        UInt16 copy_to_read_from;
        for (copy_to_read_from = 0; (copy_to_read_from < number_of_copies &&
                                     !data_read); copy_to_read_from++)
        {
            UInt16 page_offset = (copy_to_read_from * pages_per_copy) + valid_pages;

            status = ppnMiscSingleOperation(&mcxt->dev, mcxt->command,
                                            PPN_COMMAND_READ, PPN_NO_OPTIONS,
                                            sb_address->bank,
                                            sb_address->block,
                                            page_offset,
                                            TRUE32, org_buf->pData, org_buf->pSpare);

            WMR_MEMCPY(&buf->pData[0], &org_buf->pData[0], (5 * 1024));
            WMR_MEMCPY(&buf->pData[(5 * 1024)], &org_buf->pSpare[16], 16);
            WMR_MEMCPY(&buf->pData[(5 * 1024) + 16], &org_buf->pData[(5 * 1024)], ((3 * 1024) - 16));
            WMR_MEMCPY(&buf->pSpare[0], &org_buf->pSpare[0], 16);
            WMR_MEMCPY(&buf->pSpare[16], &org_buf->pSpare[0], 16);
            if (!(status & PPN_READ_STATUS_INVALID_DATA))
            {
                UInt32 data_offset, data_to_copy;
                _copySpareFPartCxtFromMeta(mcxt, &spare, buf->pSpare);
                if (valid_pages == 0)
                {
                    SpecialBlockHeaderStruct *header = (SpecialBlockHeaderStruct *)buf->pData;

                    // check that this is really what we are looking for
                    if (spare.vfl_special_type == sb_address->type &&
                        spare.spare_type == PPNFPART_SPECIAL_BLOCK_SPARE_TYPE &&
                        spare.index == 0)
                    {
                        if (copy_number != NULL)
                        {
                            *copy_number = header->copy_number;
                        }
                        if (data_uint8 != NULL)
                        {
                            size_to_copy = WMR_MIN(header->data_size, data_size);
                        }
                        else
                        {
                            size_to_copy = 0;
                        }
                        pages_per_copy = (((size_to_copy + sizeof(SpecialBlockHeaderStruct)) /
                                           mcxt->dev.main_bytes_per_page) +
                                          (((size_to_copy + sizeof(SpecialBlockHeaderStruct)) %
                                            mcxt->dev.main_bytes_per_page) ? 1 : 0));
                        number_of_copies = mcxt->dev.pages_per_block_slc / pages_per_copy;
                    }
                    else
                    {
                        continue;
                    }
                    data_to_copy = WMR_MIN((mcxt->dev.main_bytes_per_page -
                                            sizeof(SpecialBlockHeaderStruct)),
                                           size_to_copy);
                    data_offset = sizeof(SpecialBlockHeaderStruct);

                    if (data_to_copy && data_uint8 != NULL)
                    {
                        WMR_MEMCPY(&data_uint8[0], &buf->pData[data_offset], data_to_copy);
                    }
                    data_read = TRUE32;
                    break;
                }
                else
                {
                    if (spare.vfl_special_type == sb_address->type &&
                        spare.spare_type == PPNFPART_SPECIAL_BLOCK_SPARE_TYPE &&
                        spare.index == (UInt8)valid_pages)
                    {
                        data_offset = ((valid_pages * mcxt->dev.main_bytes_per_page) -
                                       sizeof(SpecialBlockHeaderStruct));
                        data_to_copy = WMR_MIN(mcxt->dev.main_bytes_per_page,
                                               (size_to_copy - data_offset));

                        WMR_MEMCPY(&data_uint8[data_offset], buf->pData, data_to_copy);
                        data_read = TRUE32;
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
        }
        if (!data_read)
        {
            goto return_error;
        }
    }
    BUF_Release(buf);
    BUF_Release(org_buf);
    return TRUE32;

 return_error:
    BUF_Release(buf);
    BUF_Release(org_buf);
    return FALSE32;
}

static BOOL32
oldReadSpecialBlockSingleVersion
    (PPNFPart_MainCxt *mcxt,
    void *data,
    UInt32 data_size,
    UInt16 special_block_cache_index,
    UInt32 *copy_number) // increased by one every time the speical block is written
{
    UInt16 sb_count, sb_type, block_idx, known_good_index = 0xFFFF;
    UInt32 local_copy_number = 0, curr_copy_number = 0;
    SpecialBlockCacheEntryType *sb_addresses;
    BOOL32 read_valid_copy = FALSE32;

    sb_type = mcxt->special_blocks[special_block_cache_index].type;
    sb_count = _countTypeOfBlocksInCache(mcxt, sb_type);
    sb_addresses = &mcxt->special_blocks[special_block_cache_index];

    // loop through the different redundat copies to find the latest version
    for (block_idx = 0; block_idx < sb_count; block_idx++)
    {
        BOOL32 status_reading_from_this_block = FALSE32;
        if (read_valid_copy)
        {
            status_reading_from_this_block =
                oldReadSpecialBlockSingleVersionFromSingleRedundantCopy(mcxt, NULL, 0,
                                                                        &sb_addresses[block_idx],
                                                                        &local_copy_number);
        }
        else
        {
            status_reading_from_this_block =
                oldReadSpecialBlockSingleVersionFromSingleRedundantCopy(mcxt, data, data_size,
                                                                        &sb_addresses[block_idx],
                                                                        &local_copy_number);
        }

        if (status_reading_from_this_block)
        {
            if (!read_valid_copy)
            {
                read_valid_copy = TRUE32;
                known_good_index = block_idx;
                curr_copy_number = local_copy_number;
            }
            else
            {
                if (local_copy_number != curr_copy_number)
                {
                    WMR_PRINT(ALWAYS, TEXT(
                                       "[PPNFPART:WRN] special block with more than one version (type:0x%04X) (line:%d)!\n"),
                                   sb_type, __LINE__);
                }
                if (local_copy_number > curr_copy_number)
                {
                    if (oldReadSpecialBlockSingleVersionFromSingleRedundantCopy(mcxt, data, data_size,
                                                                                &sb_addresses[block_idx],
                                                                                &local_copy_number))
                    {
                        WMR_PRINT(ALWAYS, TEXT(
                                           "[PPNFPART:WRN] successfully copied new SB version (new:0x08X, old:0x08X, type:0x%04X) (line:%d)!\n"),
                                       local_copy_number, curr_copy_number, sb_type, __LINE__);
                        curr_copy_number = local_copy_number;
                        known_good_index = block_idx;
                    }
                    else
                    {
                        WMR_PRINT(ALWAYS, TEXT(
                                           "[PPNFPART:WRN] failed to copy new SB version (new:0x08X, old:0x08X, type:0x%04X) (line:%d)!\n"),
                                       local_copy_number, curr_copy_number, sb_type, __LINE__);

                        // read the copy of the SB we read successfully earlier
                        if (!oldReadSpecialBlockSingleVersionFromSingleRedundantCopy(mcxt, data, data_size,
                                                                                     &sb_addresses[known_good_index],
                                                                                     &local_copy_number))
                        {
                            // if a good copy became bad, fail here
                            WMR_PRINT(ERROR, TEXT(
                                               "[PPNFPART:ERR] failed to read SB from known good copy (type:0x%04X) (line:%d)!\n"),
                                           sb_type, __LINE__);
                            return FALSE32;
                        }
                    }
                }
            }
        }
        else
        {
            WMR_PRINT(ALWAYS, TEXT(
                               "[PPNFPART:WRN] failed to read from assigned SB copy (bank:0x%04X, block:0x%04X, type:0x%04X) (line:%d)!\n"),
                           sb_addresses[block_idx].bank, sb_addresses[block_idx].block, sb_type, __LINE__);
        }
    }
    if (read_valid_copy && (copy_number != NULL))
    {
        *copy_number = local_copy_number;
    }
    return read_valid_copy;
}

BOOL32
ppnfpartOldReadSpecialBlock
    (void *data,
    UInt32 data_size,
    UInt16 type)
{
    UInt16 special_block_cache_index;
    BOOL32 res = FALSE32;
    PPNFPart_MainCxt *mcxt = g_ppnfpartMain;

    // find the block addresses
    if (_findSpecialBlock(mcxt, &special_block_cache_index, type) == FALSE32)
    {
        WMR_PRINT(ALWAYS, TEXT("[PPNVFL:WRN] Read: failed to locate special blocks (type = 0x%X) (line:%d)!\n"),
                       type, __LINE__);
        return FALSE32;
    }
    switch (andSBGetHandler(mcxt->special_blocks[special_block_cache_index].type))
    {
        case AND_SB_HANDLER_SINGLE_VERSION:
            res = oldReadSpecialBlockSingleVersion(mcxt, data, data_size, special_block_cache_index, NULL);
            break;

        default:
            WMR_PRINT(ERROR, TEXT("[PPNVFL:ERR] no support for the special block handler (type = 0x%X) (line:%d)!\n"),
                           type, __LINE__);
            break;
    }
    return res;
}

//////////////
// end - hack to switch units from vanilla to swiss
//////////////
