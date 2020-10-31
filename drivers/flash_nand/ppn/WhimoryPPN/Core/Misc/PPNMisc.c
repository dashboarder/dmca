#include "WMRConfig.h"
#include "ANDTypes.h"
#include "WMROAM.h"
#include "VFLBuffer.h"
#include "PPNMiscTypes.h"
#include "PPN_FIL.h"
#include "FIL.h"

/* print functions - for now i am keeping it old school */
#if (defined (AND_SIMULATOR) && AND_SIMULATOR)
#define     VFL_ERR_PRINT(x)            WMR_PANIC x
#else
#define     VFL_ERR_PRINT(x)            WMR_RTL_PRINT(x)
#endif
#define     VFL_WRN_PRINT(x)            WMR_RTL_PRINT(x)

#if defined (VFL_LOG_MSG_ON)
#define     VFL_LOG_PRINT(x)            WMR_DBG_PRINT(x)
#else
#define     VFL_LOG_PRINT(x)
#endif
#define VFL_INF_MSG_ON (0)
#if (defined (VFL_INF_MSG_ON) && VFL_INF_MSG_ON)
#define     VFL_INF_PRINT(x, y)         { if (x) { WMR_RTL_PRINT(y); } }
#else
#define     VFL_INF_PRINT(x, y)
#endif

#define BYTES_PER_PAGE_META (dev->lbas_per_page * dev->lba_meta_bytes_buffer)

#define PPNMISC_VERIFY_CONVERSION (0)

#if PPNMISC_VERIFY_CONVERSION
static BOOL32
ppnMiscTestConversionFunctions
    (PPN_DeviceInfo * dev);
#endif // PPNMISC_VERIFY_CONVERSION

/*
 * Name: ppnMiscGetCAUFromBank
 * Input: bank
 * Return value: CAU inside a chip enable
 */
UInt16
ppnMiscGetCAUFromBank
    (PPN_DeviceInfo * dev,
    UInt16 bank)
{
    return (UInt16)((bank / dev->num_channels) % dev->caus_per_ce);
}

/*
 * Name: ppnMiscGetChannelFromBank
 * Input: bank
 * Return value: channel this bank belongs to
 */
UInt16
ppnMiscGetChannelFromBank
    (PPN_DeviceInfo * dev,
    UInt16 bank)
{
    return (UInt16)(bank % dev->num_channels);
}

/*
 * ppnMiscGetCEIdxFromBank
 * Input: bank
 * Return value: chip enable index (location) within a channel
 */
UInt16
ppnMiscGetCEIdxFromBank
    (PPN_DeviceInfo * dev,
    UInt16 bank)
{
    return (UInt16)((bank / (dev->caus_per_ce * dev->num_channels)) % dev->ces_per_channel);
}

/*
 * Name: ppnMiscGetCEFromBank
 * Input: bank
 * Return value: physical chip enable value used by the FIL layer
 */
UInt16
ppnMiscGetCEFromBank
    (PPN_DeviceInfo * dev,
    UInt16 bank)
{
    UInt16 chip_enable_idx, channel;

    chip_enable_idx = ppnMiscGetCEIdxFromBank(dev, bank);
    channel = ppnMiscGetChannelFromBank(dev, bank);
    return dev->chip_enable_table[channel][chip_enable_idx];
}

/*
 * Name: ppnMiscGetCEIdxFromChipEnable
 * Input: channel, physical chip enable
 * Return value: chip enable index inside the specified channel
 */
UInt16
ppnMiscGetCEIdxFromChipEnable
    (PPN_DeviceInfo * dev,
    UInt16 channel,
    UInt16 chip_enable)
{
    UInt16 chip_enable_idx;

    WMR_ASSERT(channel < dev->num_channels);
    for (chip_enable_idx = 0; chip_enable_idx < dev->ces_per_channel; chip_enable_idx++)
    {
        if (dev->chip_enable_table[channel][chip_enable_idx] == chip_enable)
        {
            break;
        }
    }
    WMR_ASSERT(chip_enable_idx < dev->ces_per_channel);
    return chip_enable_idx;
}

/*
 * Name: ppnMiscConvertToPPNPageAddress
 * Input: cau, block, page offset, slc mode
 * Return value: physical ppn page address
 */
UInt32
ppnMiscConvertToPPNPageAddress
    (PPN_DeviceInfo * dev,
    UInt16 cau,
    UInt16 block,
    UInt16 page,
    BOOL32 slcAddress)
{
    UInt32 ppnPageAddr;

    ppnPageAddr = (UInt32)page;
    ppnPageAddr |= (UInt32)((UInt32)block << dev->bits_per_page_addr);
    ppnPageAddr |= (UInt32)((UInt32)cau << (dev->bits_per_page_addr + dev->bits_per_block_addr));
    ppnPageAddr |= (UInt32)((UInt32)(slcAddress ? 1 : 0) <<
                            (dev->bits_per_page_addr + dev->bits_per_block_addr + dev->bits_per_cau_addr));

    return ppnPageAddr;
}

/*
 * Name: ppnMiscConvertPhysicalAddressToCauBlockPage
 * Input: physcial page address (row address)
 * Output: cau, block, page offset
 * Return value: none
 */
void
ppnMiscConvertPhysicalAddressToCauBlockPage
    (PPN_DeviceInfo * dev,
    UInt32 page,
    UInt16 *cau,
    UInt16 *block,
    UInt16 *page_offset)
{
    UInt32 mask;

    // get page offset
    mask = (1UL << dev->bits_per_page_addr) - 1;
    *page_offset = (UInt16)(page & mask);

    // get block
    mask = (1UL << dev->bits_per_block_addr) - 1;
    page >>= dev->bits_per_page_addr;
    *block = (UInt16)(page & mask);

    // get cau
    mask = (1UL << dev->bits_per_cau_addr) - 1;
    page >>= dev->bits_per_block_addr;
    *cau = (UInt16)(page & mask);
}

/*
 * Name: ppnMiscConvertPhysicalAddressToBankBlockPage
 * Input: channel, physical chip enable, physcial page address
 * Output: bank, block, page offset, slc mode
 * Return value: none
 */
void
ppnMiscConvertPhysicalAddressToBankBlockPage
    (PPN_DeviceInfo * dev,
    UInt16 channel,
    UInt16 chip_enable_idx,
    UInt32 page,
    UInt16 *bank,
    UInt16 *block,
    UInt16 *page_offset,
    BOOL32 *slc)
{
    UInt16 cau;
    UInt32 mask;

    // get page offset
    mask = (1UL << dev->bits_per_page_addr) - 1;
    *page_offset = (UInt16)(page & mask);

    // get block
    mask = (1UL << dev->bits_per_block_addr) - 1;
    page >>= dev->bits_per_page_addr;
    *block = (UInt16)(page & mask);

    // get cau
    mask = (1UL << dev->bits_per_cau_addr) - 1;
    page >>= dev->bits_per_block_addr;
    cau = (UInt16)(page & mask);

    // get SLC bit
    page >>= dev->bits_per_cau_addr;
    *slc = (page & 1) ? TRUE32 : FALSE32;

    *bank = ((chip_enable_idx * dev->num_channels * dev->caus_per_ce) +
             (cau * dev->num_channels) + channel);
}

/*
 * Name: ppnMiscSingleOperation
 * Description: this function prepares a command for a low level operation.
 * Input: command type, bank, block, page offset, slc mode, data pointer,
 *        meta pointer
 *        data pointer, meta pointer
 * Return value: operation status
 */
PPNStatusType
ppnMiscSingleOperation
    (PPN_DeviceInfo * dev,
    PPNCommandStruct * command,
    PPNCommand command_type,
    PPNOptions options,
    UInt16 bank,
    UInt16 block,
    UInt16 page_offset,
    BOOL32 slc,
    void *data,
    void *meta)
{
    UInt16 cau = 0, channel = 0, i = 0, chip_enable_idx = 0;
    UInt8 status;
    UInt32 row_addr;

    channel = ppnMiscGetChannelFromBank(dev, bank);
    cau = ppnMiscGetCAUFromBank(dev, bank);
    chip_enable_idx = ppnMiscGetCEIdxFromBank(dev, bank);
    
    WMR_MEMSET(command, 0, sizeof(PPNCommandStruct));

    command->num_pages = 1;
    command->lbas = dev->lbas_per_page;
    command->command = command_type;
    command->options = options;
    command->mem_buffers[0].data = data;
    command->mem_buffers[0].meta = meta;
    command->mem_buffers[0].num_of_lbas = dev->lbas_per_page; // change if this is a small read
    row_addr = ppnMiscConvertToPPNPageAddress(dev, cau, block, page_offset, slc);

    switch (command_type)
    {
        case PPN_COMMAND_READ:
        {
            command->entry[0].addr.row = row_addr;
            command->entry[0].addr.column = 0;
            command->entry[0].addr.length =
                (dev->lba_meta_bytes_buffer + dev->main_bytes_per_lba) * dev->lbas_per_page;
            command->mem_index[0].offset = 0;
            command->mem_index[0].idx = 0; // change for sub page reads
            command->entry[0].lbas = dev->lbas_per_page; // change for sub page reads
            break;
        }

#ifndef AND_READONLY
        case PPN_COMMAND_PROGRAM:
        case PPN_COMMAND_ERASE:
        {
            command->entry[0].addr.row = row_addr;
            command->mem_index[0].offset = 0;
            command->mem_index[0].idx = 0;
            command->entry[0].lbas = dev->lbas_per_page;
            break;
        }
#endif // AND_READONLY

        default:
        {
            command->entry[0].addr.row = row_addr;
            command->mem_index[0].offset = 0;
            command->mem_index[0].idx = 0;
            command->entry[0].lbas = dev->lbas_per_page;
            break;
        }
    }

    command->entry[0].ceIdx = chip_enable_idx;
    for (i = 0; i < dev->ces_per_channel; i++)
    {
        command->ceInfo[i].ce = dev->chip_enable_table[channel][i];
        command->ceInfo[i].pages = 0;
    }
    command->ceInfo[chip_enable_idx].pages = 1;

    command->context.device_info = NULL;
    command->context.handle = NULL;
    command->context.bus_num = channel;

    dev->fil->PerformCommandList(&command, 1);
    status = command->page_status_summary;

    return status;
}

/*
 * Name: ppnMiscSingleLbaRead
 * Description: this function prepares a command for a low level single LBA read.
 * Input: command type, ce, cau, block, page offset, slc mode, data pointer,
 *        meta pointer
 * Return value: operation status
 */
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
    void *meta)
{
    UInt16 channel = 0, i = 0, chip_enable_idx = 0;
    UInt8 status;
    UInt32 row_addr;

    chip_enable_idx = ce % dev->ces_per_channel;
    channel = ce / dev->ces_per_channel;

    WMR_MEMSET(command, 0, sizeof(PPNCommandStruct));

    command->num_pages = 1;
    command->lbas = 1;
    command->command = command_type;
    command->options = options;
    command->mem_buffers[0].data = data;
    command->mem_buffers[0].meta = meta;
    command->mem_buffers[0].num_of_lbas = 1;
    row_addr = ppnMiscConvertToPPNPageAddress(dev, cau, block, page_offset, slc);

    switch (command_type)
    {
        case PPN_COMMAND_READ:
        {
            command->entry[0].addr.row = row_addr;
            command->entry[0].addr.column = lba_offset * (dev->main_bytes_per_lba + dev->lba_meta_bytes_buffer);
            command->entry[0].addr.length =
                (dev->lba_meta_bytes_buffer + dev->main_bytes_per_lba);
            command->mem_index[0].offset = 0;
            command->mem_index[0].idx = command->mem_buffers_count;
            command->entry[0].lbas = 1;
            break;
        }
        default:
        {
            VFL_ERR_PRINT((TEXT("PPNMISC:ERR] currently only read supported: (l:%d)\n"), __LINE__));
            return 0; 
        }
    }

    command->entry[0].ceIdx = chip_enable_idx;
    for (i = 0; i < dev->ces_per_channel; i++)
    {
        command->ceInfo[i].ce = dev->chip_enable_table[channel][i];
        command->ceInfo[i].pages = 0;
    }
    command->ceInfo[chip_enable_idx].pages = 1;

    command->context.device_info = NULL;
    command->context.handle = NULL;
    command->context.bus_num = channel;

    dev->fil->PerformCommandList(&command, 1);
    status = command->page_status_summary;

    return status;
}

/*
 * Name: ppnMiscFillDevStruct
 * Description: this function fills the device information struct for vfl and fpart.
 * Input: fil function struct
 * Return value: none
 */
void
ppnMiscFillDevStruct
    (PPN_DeviceInfo * dev,
    LowFuncTbl *fil)
{
    UInt16 channel, chip_enable_idx;

    dev->fil = fil;
    dev->num_channels = fil->GetDeviceInfo(AND_DEVINFO_NUM_OF_CHANNELS);
    dev->ces_per_channel = fil->GetDeviceInfo(AND_DEVINFO_NUM_OF_CES_PER_CHANNEL);
    dev->caus_per_ce = fil->GetDeviceInfo(AND_DEVINFO_CAUS_PER_CE);
    dev->blocks_per_cau = fil->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CAU);
    dev->pages_per_block_slc = fil->GetDeviceInfo(AND_DEVINFO_SLC_PAGES_PER_BLOCK);
    dev->pages_per_block_mlc = fil->GetDeviceInfo(AND_DEVINFO_MLC_PAGES_PER_BLOCK);
    dev->f_match_oddeven_blocks = (BOOL32)fil->GetDeviceInfo(AND_DEVINFO_MATCH_ODDEVEN_CAUS);
    dev->bits_per_cau_addr = fil->GetDeviceInfo(AND_DEVINFO_BITS_PER_CAU_ADDRESS);
    dev->bits_per_page_addr = fil->GetDeviceInfo(AND_DEVINFO_BITS_PER_PAGE_ADDRESS);
    dev->bits_per_block_addr = fil->GetDeviceInfo(AND_DEVINFO_BITS_PER_BLOCK_ADDRESS);
    dev->main_bytes_per_page = fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    dev->num_of_banks = dev->num_channels * dev->ces_per_channel * dev->caus_per_ce;
    dev->lbas_per_page = fil->GetDeviceInfo(AND_DEVINFO_FIL_LBAS_PER_PAGE);
    dev->main_bytes_per_lba = dev->main_bytes_per_page / dev->lbas_per_page;
    dev->lba_meta_bytes_buffer = fil->GetDeviceInfo(AND_DEVINFO_FIL_META_BUFFER_BYTES);
    dev->lba_meta_bytes_valid = fil->GetDeviceInfo(AND_DEVINFO_FIL_META_VALID_BYTES);
    dev->max_pages_in_prep_per_ce = fil->GetDeviceInfo(AND_DEVINFO_FIL_PREP_BUFFER_ENTRIES);
    //    dev->f_match_oddeven_blocks = (fil->GetDeviceInfo(AND_DEVINFO_FIL_MATCH_ODD_EVEN_BLKS) ?
    //                                   TRUE32 : FALSE32);

//    WMR_PRINT(ALWAYS, "num_of_banks = %d (num_channels = %d, ces_per_channel = %d, caus_per_ce = %d\n",
//              dev->num_of_banks, dev->num_channels, dev->ces_per_channel, dev->caus_per_ce);

    dev->pages_per_sublk = dev->num_of_banks * dev->pages_per_block_mlc;
    for (channel = 0; channel < dev->num_channels; channel++)
    {
        for (chip_enable_idx = 0; chip_enable_idx < dev->ces_per_channel; chip_enable_idx++)
        {
            PPNChipEnableStruct chip_enable_struct;
            UInt32 size;
            chip_enable_struct.channel = channel;
            chip_enable_struct.chip_enable_idx = chip_enable_idx;
            FIL_GetStruct(AND_STRUCT_FIL_GET_CE_INFO, &chip_enable_struct, &size);
            dev->chip_enable_table[channel][chip_enable_idx] = chip_enable_struct.physical_chip_enable;
        }
    }

#if PPNMISC_VERIFY_CONVERSION
    // debug code - verify that the conversion code works correctly
    ppnMiscTestConversionFunctions(dev);
#endif // PPNMISC_VERIFY_CONVERSION
}

/*
 * Name: ppnMiscIsBlockGood
 * Description: this function takes the bad block table bitmap and checks if a
 *              block is initally marked bad or not.
 * Input: bad block table bitmap (as read from the PPN device), block
 * Return value: none
 */
BOOL32
ppnMiscIsBlockGood
    (UInt8 *bbt,
    UInt16 block)
{
    UInt16 byte;
    UInt8 bit;

    byte = block >> 3;
    bit = 0x80 >> (block & 0x0007);
    return bbt[byte] & bit ? TRUE32 : FALSE32;
}

#ifndef AND_READONLY

BOOL32
ppnMiscTestSpecialBlock
    (PPN_DeviceInfo * dev,
    PPNCommandStruct * command,
    UInt16 bank,
    UInt16 block,
    UInt8 spareType)
{
    // try the block (erase, program, read) twice
    UInt16 page_offset, i;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
    BOOL32 slc = TRUE32;
    BOOL32 pages;

    for (i = 0; i < 2; i++)
    {
        PPNStatusType status;

        // alternate between slc and mlc
        slc = (slc ? FALSE32 : TRUE32);
        pages = (slc ? dev->pages_per_block_slc : dev->pages_per_block_mlc);

        // erase
        status = ppnMiscSingleOperation(dev, command, PPN_COMMAND_ERASE, PPN_OPTIONS_IGNORE_BAD_BLOCK,
                                        bank, block, 0, slc, NULL, NULL);
        if (status & (PPN_ERASE_STATUS_RETIRE | PPN_ERASE_STATUS_FAIL))
        {
            VFL_WRN_PRINT((TEXT("PPNMISC:WRN] testing special block - failed erasing (0x%X, 0x%X) (l:%d)\n"), bank,
                           block, __LINE__));
            goto return_error;
        }

        // program
        for (page_offset = 0; page_offset < pages; page_offset++)
        {
            WMR_MEMSET(buf->pData, (UInt8)page_offset, dev->main_bytes_per_page);
            WMR_MEMSET(buf->pSpare, spareType, BYTES_PER_PAGE_META);
            status = ppnMiscSingleOperation(dev, command, PPN_COMMAND_PROGRAM, PPN_NO_OPTIONS,
                                            bank, block, page_offset, slc, buf->pData, buf->pSpare);
            if (status & (PPN_PROGRAM_STATUS_FAIL))
            {
                VFL_WRN_PRINT((TEXT(
                                   "PPNMISC:WRN] testing special block - failed programming (0x%X, 0x%X, 0x%X) (l:%d)\n"),
                               bank, block,
                               page_offset, __LINE__));
                goto return_error;
            }
        }
        // verify
        for (page_offset = 0; page_offset < pages; page_offset++)
        {
            WMR_MEMSET(buf->pData, 0, dev->main_bytes_per_page);
            WMR_MEMSET(buf->pSpare, 0, BYTES_PER_PAGE_META);
            status = ppnMiscSingleOperation(dev, command, PPN_COMMAND_READ, PPN_NO_OPTIONS, bank, block, page_offset,
                                            slc, buf->pData, buf->pSpare);
            if (status & (PPN_READ_STATUS_INVALID_DATA | PPN_READ_STATUS_REFRESH | PPN_READ_STATUS_RETIRE))
            {
                VFL_WRN_PRINT((TEXT("PPNMISC:WRN] testing special block - failed reading (0x%X, 0x%X, 0x%X) (l:%d)\n"),
                               bank, block, page_offset, __LINE__));
                goto return_error;
            }
            if (buf->pData[0] != (UInt8)page_offset || buf->pSpare[0] != spareType)
            {
                VFL_WRN_PRINT((TEXT("PPNMISC:WRN] testing special block - failed verifying (0x%X, 0x%X, 0x%X) (l:%d)\n"),
                               bank, block, page_offset, __LINE__));
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
    UInt16 mem_offset)
{
    UInt16 idx = 0;
    UInt32 page_address = 0;
    PPNCommandStruct *command;

    page_address = ppnMiscConvertToPPNPageAddress(dev, cau, block, page_offset, FALSE32);
    command = commands[channel];
    // check for buffer overflow
    if ((command->num_pages >= PPN_MAX_PAGES) ||
        (command->ceInfo[chip_enable_idx].pages >= PPN_MAX_PAGES_PER_CE) ||
        (command->ceInfo[chip_enable_idx].pages >= dev->max_pages_in_prep_per_ce))
    {
        return FALSE32;
    }

    // use idx as a temp variable for index in this specific channel
    idx = commands[channel]->num_pages;
    command->entry[idx].ceIdx = chip_enable_idx;
    switch (command->command)
    {
        case PPN_COMMAND_READ:
        {
            command->entry[idx].addr.row = page_address;
            command->entry[idx].addr.column = pba_offset * (dev->main_bytes_per_lba + dev->lba_meta_bytes_buffer);
            command->entry[idx].addr.length = pba_count * (dev->main_bytes_per_lba + dev->lba_meta_bytes_buffer);
            command->mem_index[idx].offset = mem_offset;
            command->mem_index[idx].idx = mem_index;
            command->entry[idx].lbas = pba_count;
            break;
        }

#ifndef AND_READONLY
        case PPN_COMMAND_PROGRAM:
        case PPN_COMMAND_ERASE:
        {
            command->entry[idx].addr.row = page_address;
            command->mem_index[idx].offset = mem_offset;
            command->mem_index[idx].idx = mem_index;
            command->entry[idx].lbas = dev->lbas_per_page;
            break;
        }
#endif // AND_READONLY

        default:
        {
            command->entry[idx].addr.row = page_address;
            command->mem_index[idx].offset = mem_offset;
            command->mem_index[idx].idx = mem_index;
            command->entry[idx].lbas = dev->lbas_per_page;
            break;
        }
    }
    commands[channel]->lbas += command->entry[idx].lbas;
    command->num_pages++;

    // use idx as a temp variable for index in prep buffer
    command->ceInfo[chip_enable_idx].pages++;

    return TRUE32;
}

/*
 * Name: ppnMiscAddMemoryToCommandStructure
 * Output: pointer to commands structure
 * Input: command type, data pointer, meta pointer.
 * Return value: TRUE32 for success otherwise FALSE32
 */
BOOL32
ppnMiscAddMemoryToCommandStructure
    (PPN_DeviceInfo *dev,
    PPNCommandStruct **commands,
    UInt16 num_of_command_structs,
    void *data,
    void *meta,
    UInt8 num_of_lbas)
{
    UInt16 struct_idx;

    for (struct_idx = 0; struct_idx < num_of_command_structs; struct_idx++)
    {
        PPNCommandStruct *command = commands[struct_idx];
        PPNMemoryEntry *mem = &command->mem_buffers[command->mem_buffers_count];

        if (command->mem_buffers_count >= PPN_MAX_MEM_BUFFERS)
        {
            return FALSE32;
        }

        mem->data = data;
        mem->meta = meta;
        mem->num_of_lbas = num_of_lbas;
        command->mem_buffers_count++;
    }
    return TRUE32;
}

/*
 * Name: ppnMiscInitCommandStructure
 * Output: pointer to commands structure
 * Input: command type, data pointer, meta pointer.
 * Return value: none
 */
void
ppnMiscInitCommandStructure
    (PPN_DeviceInfo * dev,
    PPNCommandStruct **commands,
    UInt16 num_of_command_structs,
    PPNCommand command,
    PPNOptions options)
{
    UInt16 i, j;

    for (i = 0; i < num_of_command_structs; i++)
    {
        WMR_MEMSET(commands[i], 0, sizeof(PPNCommandStruct));
        // initilaze the command structure fields
        commands[i]->options = options;
        commands[i]->num_pages = 0;
        commands[i]->lbas = 0;
        commands[i]->command = command;
        commands[i]->mem_buffers[0].data = NULL;
        commands[i]->mem_buffers[0].meta = NULL;
        commands[i]->mem_buffers_count = 0;
        for (j = 0; j < dev->ces_per_channel; j++)
        {
            commands[i]->ceInfo[j].ce = dev->chip_enable_table[i][j];
            commands[i]->ceInfo[j].pages = 0;
        }
        commands[i]->context.device_info = NULL;
        commands[i]->context.handle = NULL;
        commands[i]->context.bus_num = i;
    }
}

#if PPNMISC_VERIFY_CONVERSION

static BOOL32
_verifyAnAddress
    (PPN_DeviceInfo * dev,
    UInt16 bank,
    UInt16 block,
    UInt16 page_offset,
    BOOL32 slc)
{
    UInt16 chip_enable_idx, channel, cau;
    UInt32 page_address = 0, line_failed = 0;
    UInt16 temp_bank, temp_block, temp_page_offset;
    BOOL32 temp_slc;

    // convert to a physical address
    cau = ppnMiscGetCAUFromBank(dev, bank);
    if (!(cau < dev->caus_per_ce))
    {
        line_failed = __LINE__;
        goto return_error;
    }
    page_address = ppnMiscConvertToPPNPageAddress(dev, cau, block, page_offset, slc);
    chip_enable_idx = ppnMiscGetCEIdxFromBank(dev, bank);
    if (!(chip_enable_idx < dev->ces_per_channel))
    {
        line_failed = __LINE__;
        goto return_error;
    }
    channel = ppnMiscGetChannelFromBank(dev, bank);
    if (!(channel < dev->num_channels))
    {
        line_failed = __LINE__;
        goto return_error;
    }

    // convert back
    ppnMiscConvertPhysicalAddressToBankBlockPage(dev, channel, chip_enable_idx, page_address,
                                                 &temp_bank, &temp_block, &temp_page_offset, &temp_slc);
    if (!((temp_bank == bank) && (temp_block == block) && (temp_page_offset == page_offset) && (slc == temp_slc)))
    {
        line_failed = __LINE__;
        goto return_error;
    }

    return TRUE32;

 return_error:
    VFL_WRN_PRINT((TEXT("PPNMISC:ERR] fail address conversion: (l:%d)\n"), line_failed));
    VFL_WRN_PRINT((TEXT("PPNMISC:ERR]     bank:0x%X, block:0x%X, page_offset:0x%X, slc:0x%X\n"),
                   bank, block, page_offset, slc));
    VFL_WRN_PRINT((TEXT("PPNMISC:ERR]     channel:0x%X, chip_enable_idx:0x%X, cau:0x%X\n"),
                   channel, chip_enable_idx, cau));
    VFL_WRN_PRINT((TEXT("PPNMISC:ERR]     temp_bank:0x%X, temp_block:0x%X, temp_page_offset:0x%X, temp_slc:0x%X\n"),
                   temp_bank, temp_block, temp_page_offset, temp_slc));
    return FALSE32;
}

static BOOL32
ppnMiscTestConversionFunctions
    (PPN_DeviceInfo * dev)
{
    UInt16 bank, block, page_offset;
    UInt32 counter = 0;

    for (bank = 0; bank < dev->num_of_banks; bank++)
    {
        for (block = 0; block < dev->blocks_per_cau; block++)
        {
            for (page_offset = 0; page_offset < dev->pages_per_block_mlc; page_offset++)
            {
                counter++;
                if (!(_verifyAnAddress(dev, bank, block, page_offset, TRUE32)))
                {
                    VFL_ERR_PRINT((TEXT("PPNMISC:ERR] fail address conversion test: (l:%d)\n"), __LINE__));
                    return FALSE32;
                }
            }
            for (page_offset = 0; page_offset < dev->pages_per_block_slc; page_offset++)
            {
                if (!(_verifyAnAddress(dev, bank, block, page_offset, TRUE32)))
                {
                    VFL_ERR_PRINT((TEXT("PPNMISC:ERR] fail address conversion test: (l:%d)\n"), __LINE__));
                    return FALSE32;
                }
            }
        }
    }

    return counter;
}

#endif // PPNMISC_VERIFY_CONVERSION

void
ppnMiscReorderCommandStruct
    (PPN_DeviceInfo * dev,
    PPNCommandStruct **commands,
    UInt16 num_of_command_structs,
    PPNReorderStruct *reorder)
{
    UInt16 channel;

    // if there is only one ce per channel no need to reorder
    if (dev->ces_per_channel == 1)
    {
        return;
    }

#if !AND_READONLY
    if (PPN_COMMAND_PROGRAM == (*commands)->command)
    {
        return;
    }
#endif // !AND_READONLY

    for (channel = 0; channel < num_of_command_structs; channel++)
    {
        PPNCommandStruct *command = commands[channel];
        UInt16 ce, page_idx;
        UInt16 ce_page_count[PPN_MAX_CES_PER_BUS] = {0};

        if (1 >= command->num_pages)
        {
            continue;
        }

        for (page_idx = 0; page_idx < command->num_pages; page_idx++)
        {
            const PPNCommandEntry *entry = &command->entry[page_idx];
            const PPNMemoryIndex *memIndex = &command->mem_index[page_idx];
            reorder->entry[entry->ceIdx][ce_page_count[entry->ceIdx]] = *entry;
            reorder->mem_index[entry->ceIdx][ce_page_count[entry->ceIdx]] = *memIndex;
            ce_page_count[entry->ceIdx]++;
        }

        page_idx = 0;
        for (ce = 0; ce < dev->ces_per_channel; ce++)
        {
            const PPNCommandEntry *entry = reorder->entry[ce];
            const PPNMemoryIndex *memIndex = reorder->mem_index[ce];
            WMR_MEMCPY(&command->entry[page_idx], entry, ce_page_count[ce] * sizeof(*entry));
            WMR_MEMCPY(&command->mem_index[page_idx], memIndex, ce_page_count[ce] * sizeof(*memIndex));
            command->ceInfo[ce].offset = page_idx;
            page_idx += ce_page_count[ce];
        }
    }
}
