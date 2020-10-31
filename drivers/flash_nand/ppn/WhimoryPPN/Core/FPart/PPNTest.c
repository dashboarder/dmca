//Test file compiled only for simulator

static BOOL32
ppnMiscTestConversionFunctions
(PPN_DeviceInfo * dev);

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


