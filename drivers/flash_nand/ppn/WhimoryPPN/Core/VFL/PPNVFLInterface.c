#include "WMRConfig.h"
#include "ANDTypes.h"
#include "WMROAM.h"
#include "VFLBuffer.h"
#include "PPNMiscTypes.h"
#include "PPNVFLTypes.h"

#define kPPNVFLMinorVersion (1)

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

// read result flags - 32 Bit
#define PPNVFL_READ_RES_REFRESH      (1 << 0)
#define PPNVFL_READ_RES_RETIRE       (1 << 1)
#define PPNVFL_READ_RES_UECC         (1 << 2) // UECC = invalid data TRUE and clean FALSE
#define PPNVFL_READ_RES_CLEAN        (1 << 3) // UECC = invalid data TRUE and clean TRUE
#define PPNVFL_READ_RES_VALID_DATA   (1 << 4) // at least one of the pages had valid data
#define PPNVFL_READ_RES_UNIDENTIFIED (1 << 5) // at least one status was something we do not expect

#define PPNVFL_CONTEXT_BLOCK_LIMIT(dev) (((dev)->blocks_per_cau * 5) / 100)

#define SECTOR_SIZE 1024

// _readPageOfASet - defines
#define RPS_STATUS_SPARE_TYPE_MISSMATCH    (1 << 1)
#define RPS_STATUS_BANK_MISSMATCH          (1 << 2)
#define RPS_STATUS_IDX_MISSMATCH           (1 << 3)

/* ----------- I/F commands - start ------------- */

#ifndef AND_READONLY
static Int32 _g_ppnvflFormat
    (UInt32 dwKeepout,
    UInt32 dwOptions);
static Int32 _g_ppnvflWriteSinglePage
    (UInt32 nVpn,
    Buffer *pBuf,
    BOOL32 boolReplaceBlockOnFail,
    BOOL32 bDisableWhitening);
static Int32 _g_ppnvflErase
    (UInt16 wVbn,
    BOOL32 bReplaceOnFail);
static Int32 _g_ppnvflChangeFTLCxtVbn
    (UInt16 *aFTLCxtVbn);
static BOOL32 _g_ppnvflWriteMultiplePagesInVb
    (UInt32 dwVpn,
    UInt16 wNumPagesToWrite,
    UInt8 * pbaData,
    UInt8 * pbaSpare,
    BOOL32 boolReplaceBlockOnFail,
    BOOL32 bDisableWhitening);
#endif // ! AND_READONLY
static Int32 _g_ppnvflInit
    (FPartFunctions * pFPartFunctions);
static Int32 _g_ppnvflOpen
    (UInt32 dwKeepout,
    UInt32 minor_ver,
    UInt32 dwOptions);
static void  _g_ppnvflClose
    (void);
static BOOL32 _g_ppnvflReadScatteredPagesInVb
    (UInt32 * padwVpn,
    UInt16 wNumPagesToRead,
    UInt8 * pbaData,
    UInt8 * pbaSpare,
    BOOL32 * pboolNeedRefresh,
    UInt8 * pabSectorStats,
    BOOL32 bDisableWhitening,
    Int32 *actualStatus);
static Int32 _g_ppnvflReadSinglePage
    (UInt32 nVpn,
    Buffer *pBuf,
    BOOL32 bCleanCheck,
    BOOL32 bMarkECCForScrub,
    BOOL32 * pboolNeedRefresh,
    UInt8 * pabSectorStats,
    BOOL32 bDisableWhitening);
static UInt16* _g_ppnvflGetFTLCxtVbn
    (void);
static BOOL32 _g_ppnvflGetStruct
    (UInt32 dwStructType,
    void * pvoidStructBuffer,
    UInt32 * pdwStructSize);
static BOOL32 _g_ppnvflSetStruct
    (UInt32 dwStructType,
    void * pvoidStructBuffer,
    UInt32 dwStructSize);
static UInt32 _g_ppnvflGetDeviceInfo
    (UInt32 dwParamType);
static Int32  _ppnvflOpen
    (PPNVFL_MainCxt * mcxt,
    UInt32 dwKeepout,
    UInt32 minor_ver,
    UInt32 dwOptions);

/* ----------- I/F commands - end ------------- */

static PPNVFL_MainCxt *g_ppnvflMain = NULL;
static VFLFailureDetails gstLastFailure;

// helper functions - local
#if !AND_READONLY
static BOOL32 _programCxtLastPage
    (PPNVFL_MainCxt * mcxt,
    Buffer * buf,
    BOOL32 retry);
static BOOL32 _copyVFLCxtToOtherPair
    (PPNVFL_MainCxt * mcxt);
#endif // !AND_READONLY
static BOOL32 _readVFLCxtPage
    (PPNVFL_MainCxt * mcxt,
    Buffer *buf,
    UInt16 bank,
    UInt16 vfl_idx);
static void
_copyFromOnNandCxtToRam
    (PPNVFL_MainCxt * mcxt,
    UInt8 *data);
#if !AND_READONLY
static void
_copyFromRamToOnNandCxt
    (PPNVFL_MainCxt * mcxt,
    UInt8 *data);
#endif // !AND_READONLY

typedef enum
{
    VFLCxtProgramFail = 0,
    VFLCxtProgramDone = 2,
    VFLCxtProgramRetry = 3,
} ProgramVFLCxtPageWrapStatus;

#if !AND_READONLY
static ProgramVFLCxtPageWrapStatus _programVFLCxtPageWrap
    (PPNVFL_MainCxt * mcxt,
    Buffer *buf,
    UInt16 bank,
    UInt16 vfl_idx);
static BOOL32 _programVFLCxtPage
    (PPNVFL_MainCxt * mcxt,
    Buffer *buf,
    UInt16 bank,
    UInt16 vfl_idx);
#endif // !AND_READONLY
static void
_convertVFLPageIdxToPhysicalPageBlockOffsets
    (PPNVFL_MainCxt * mcxt,
    UInt16 set,
    UInt16 vfl_page_idx,
    UInt16 *vfl_block_offset,
    UInt16 *vfl_page_offset);

#ifndef AND_READONLY
static BOOL32
_markBlockForReplacement
    (PPNVFL_MainCxt * mcxt,
    UInt16 bank,
    UInt16 block);
#endif // ! AND_READONLY

#if !AND_READONLY

static void
_setVFLCxtSpare
    (PPNVFL_MainCxt * mcxt,
    UInt8 *meta,
    UInt8 bank,
    UInt8 vfl_idx)
{
    PPNVFL_CxtSpare *spare = (PPNVFL_CxtSpare*)meta;

    WMR_MEMSET(spare, 0x00, sizeof(PPNVFL_CxtSpare));
    spare->cxt_age = mcxt->r_gen.cxt_age;
    spare->bank = bank;
    spare->spareType = PPNVFL_SPARE_TYPE_CXT;
    spare->idx = vfl_idx;
}

#endif // !AND_READONLY

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

/* address translating functions - start */

static UInt32
_getMixedArrayEntryOffset
    (PPNVFL_MainCxt * mcxt,
    UInt16 vbn,
    UInt16 mixed_array_offset)
{
    const UInt32 v2p_mixed_entry_uint32 = mcxt->v2p_main[vbn] & (~PPN_V2P_MIXED_MARK);
    const UInt32 num_of_banks_uint32 = mcxt->dev.num_of_banks;
    const UInt32 mixed_array_offset_uint32 = mixed_array_offset;

    return (v2p_mixed_entry_uint32 * num_of_banks_uint32) + mixed_array_offset_uint32;
}

/*
 * Name: _convertVpnToBankBlockPage
 * Input: vpn
 * Output: bank, block, page offset
 * Return value: none
 */
static void
_convertVpnToBankBlockPage
    (PPNVFL_MainCxt * mcxt,
    UInt32 vpn,
    UInt16 *bank,
    UInt16 *block,
    UInt16 *page_offset)
{
    UInt16 vbn = vpn / mcxt->dev.pages_per_sublk;
    UInt16 mixed_array_offset = vpn % mcxt->dev.num_of_banks;

    if (mcxt->v2p_main[vbn] & PPN_V2P_MIXED_MARK)
    {
        UInt32 mixed_index = _getMixedArrayEntryOffset(mcxt, vbn, mixed_array_offset);
        *block = mcxt->v2p_mixed[mixed_index].pbn;
        *bank = mcxt->v2p_mixed[mixed_index].bank;
    }
    else
    {
        *block = mcxt->v2p_main[vbn];
        *bank = mixed_array_offset;
    }
    *page_offset = (vpn % mcxt->dev.pages_per_sublk) / mcxt->dev.num_of_banks;
}
/* address translating functions - end */

/*
 * Name: _printV2PMapping
 * Description: if PPNVFL_PRINT_V2PMAPPING is enabled and VFL_INF_PRINT
 *              is working this function print the virtual to phsical map.
 * Return value: none
 */

#define PPNVFL_PRINT_V2PMAPPING (1)

static void
_printV2PMapping
    (PPNVFL_MainCxt * mcxt)
{
#if (defined(PPNVFL_PRINT_V2PMAPPING) && PPNVFL_PRINT_V2PMAPPING)
    UInt16 vbn;
    UInt32 vpn;

    VFL_INF_PRINT(1, (TEXT("_printV2PMapping()\n")));
    for (vbn = 0; vbn < mcxt->gen.num_of_vfl_sublk; vbn++)
    {
        UInt16 bank_idx;
        vpn = (UInt32)vbn * (UInt32)mcxt->dev.pages_per_sublk;
        VFL_INF_PRINT(1, (TEXT("%04X : "), vbn));
        for (bank_idx = 0; bank_idx < mcxt->dev.num_of_banks; bank_idx++)
        {
            UInt16 bank, block, page_offset;
            _convertVpnToBankBlockPage(mcxt, (vpn + bank_idx), &bank, &block, &page_offset);
            VFL_INF_PRINT(1, (TEXT("[%02X, %04X] "), bank, block));
        }
        VFL_INF_PRINT(1, (TEXT("\n")));
    }
#endif
}

#define PPNVFL_PRINT_CXT_BLOCK_MAPPING (1)

static void
_printVFLBlockMapping
    (PPNVFL_MainCxt * mcxt,
    UInt32 line)
{
    UInt16 i, bank;

    VFL_INF_PRINT(PPNVFL_PRINT_CXT_BLOCK_MAPPING,
                  (TEXT("[PPNVFL:INF] printing vfl block list (caller line:%d)\n"), line));
    for (i = 0; i < mcxt->r_gen.num_of_vfl_blocks; i++)
    {
        VFL_INF_PRINT(PPNVFL_PRINT_CXT_BLOCK_MAPPING,
                      (TEXT("(0x%04X, 0x%04X)"), mcxt->gen.vfl_blocks[i].bank, mcxt->gen.vfl_blocks[i].block));
    }
    VFL_INF_PRINT(PPNVFL_PRINT_CXT_BLOCK_MAPPING, (TEXT("\n")));
    VFL_INF_PRINT(PPNVFL_PRINT_CXT_BLOCK_MAPPING,
                  (TEXT("[PPNVFL:INF] printing mapping pages (caller line:%d)\n"), line));
    for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
    {
        VFL_INF_PRINT(PPNVFL_PRINT_CXT_BLOCK_MAPPING,
                      (TEXT("bank 0x%02X: "), bank));
        for (i = 0; i < mcxt->r_gen.pages_per_block_mapping; i++)
        {
            VFL_INF_PRINT(PPNVFL_PRINT_CXT_BLOCK_MAPPING,
                          (TEXT("0x%04X "), mcxt->vfl_map_pages[bank][i]));
        }
        VFL_INF_PRINT(PPNVFL_PRINT_CXT_BLOCK_MAPPING, (TEXT("\n")));
    }
}
/*
 * Name: callPrintV2PMapping
 * Description: this function is a none static version of _printV2PMapping
 * Return value: none
 */
void
callPrintV2PMapping
    (void)
{
    _printV2PMapping(g_ppnvflMain);
}

/*
 * Name: _calcNumOfVFLSuBlk
 * Return value: number of VFL virtual blocks for this media
 */
static UInt32
_calcNumOfVFLSuBlk
    (PPNVFL_MainCxt * mcxt)
{
    UInt32 temp, shift;

    temp = mcxt->dev.blocks_per_cau + 100; // making sure we have the right bit set
    shift = 0;
    while (temp >>= 1)
    {
        shift++;
    }
    return PPNVFL_USER_BLKS_PER_256B * (1 << (shift - 8));
}

/* ppn commands functions - start */
/*
 * Name: _addAddressToCommandStructure
 * Output: pointer to commands structure
 * Input: vfl address to add, index in the global operation (coming from the FTL).
 * Return value: TRUE32 is successful, otehrwise - FALSE32
 */
#define PPNVFL_PRINT_ADD_ADDRESS (1)
static BOOL32
_addAddressToCommandStructure
    (PPNVFL_MainCxt * mcxt,
    PPNCommandStruct **commands,
    UInt32 vfl_address,
    UInt8 vba_offset,
    UInt8 vba_count,
    UInt8 mem_index,
    UInt16 mem_offset)
{
    UInt16 bank = 0, block = 0, cau = 0, chip_enable_idx = 0, channel = 0,
           page_offset = 0;

    // convert address from vfl to physical
    _convertVpnToBankBlockPage(mcxt, vfl_address, &bank, &block, &page_offset);
    channel = ppnMiscGetChannelFromBank(&mcxt->dev, bank);
    chip_enable_idx = ppnMiscGetCEIdxFromBank(&mcxt->dev, bank);
    cau = ppnMiscGetCAUFromBank(&mcxt->dev, bank);
    return ppnMiscAddPhysicalAddressToCommandStructure(&mcxt->dev, mcxt->commands, channel, chip_enable_idx,
                                                      cau, block, page_offset, FALSE32, vba_offset, vba_count,
                                                      mem_index, mem_offset);
}

/*
 * Name: _fillScatteredAddressesToCommandStructure
 * Description: this function takes an array of scattered addresses and
 *              generates commands structure to send to the FIL.
 * Output: pointer to commands structure
 * Input: array of vfl addresses, number of addresses, command type,
 *        data pointer, meta pointer
 * Return value: TRUE32 is successful, otehrwise - FALSE32
 */
static BOOL32
_fillScatteredAddressesToCommandStructure
    (PPNVFL_MainCxt * mcxt,
    PPNCommandStruct **commands,
    UInt32 *vfl_addresses,
    UInt16 *num_addresses,
    PPNCommand command,
    void *data,
    void *meta,
    PPNOptions options)
{
    UInt16 addr_idx;

    ppnMiscInitCommandStructure(&mcxt->dev, mcxt->commands, mcxt->dev.num_channels, command, options);
    for (addr_idx = 0; addr_idx < *num_addresses; addr_idx++)
    {
        if (_addAddressToCommandStructure(mcxt, mcxt->commands, vfl_addresses[addr_idx], 0, 1,
            mcxt->commands[0]->mem_buffers_count, addr_idx) == FALSE32)
        {
            *num_addresses = addr_idx;
            break;
        }
    }
    ppnMiscAddMemoryToCommandStructure(&mcxt->dev, mcxt->commands, mcxt->dev.num_channels, data, meta, addr_idx);
    ppnMiscReorderCommandStruct(&mcxt->dev, mcxt->commands, mcxt->dev.num_channels, mcxt->reorder);

    return TRUE32;
}

#if !AND_READONLY

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
    (PPNVFL_MainCxt * mcxt,
    PPNCommandStruct **commands,
    UInt32 vfl_address,
    UInt16 *num_addresses,
    PPNCommand command,
    void *data,
    void *meta,
    PPNOptions options)
{
    UInt16 addr_idx;

    ppnMiscInitCommandStructure(&mcxt->dev, mcxt->commands, mcxt->dev.num_channels, command, options);
    for (addr_idx = 0; addr_idx < *num_addresses; addr_idx++)
    {
        if (_addAddressToCommandStructure(mcxt, mcxt->commands, (vfl_address + addr_idx), 0, 1,
                                        mcxt->commands[0]->mem_buffers_count, addr_idx) == FALSE32)
        {
            *num_addresses = addr_idx;
            break;
        }
    }
    ppnMiscAddMemoryToCommandStructure(&mcxt->dev, mcxt->commands, mcxt->dev.num_channels, data, meta, addr_idx);
    ppnMiscReorderCommandStruct(&mcxt->dev, mcxt->commands, mcxt->dev.num_channels, mcxt->reorder);

    return TRUE32;
}

#endif // !AND_READONLY

/* ppn commands functions - end */

/* vfl mixed list functions - start */
static UInt32
_getV2PMixedArraySize
    (PPNVFL_MainCxt * mcxt)
{
    return sizeof(MixedEntryType) * (mcxt->dev.blocks_per_cau - mcxt->gen.num_of_vfl_sublk) *
           mcxt->dev.num_of_banks * mcxt->dev.num_of_banks;
}

/*
 * Name: _isMixedBlock
 * Description: this function checks if a virtual block has blocks from
 *              different offsets in different banks or not.
 * Input: virtual block address
 * Return value: TRUE32 if the block is mixed, otherwise FALSE32
 */
BOOL32
_isMixedBlock
    (PPNVFL_MainCxt * mcxt,
    UInt16 vbn)
{
    WMR_ASSERT(mcxt->v2p_main[vbn] != 0xFFFF);

    if (mcxt->v2p_main[vbn] & PPN_V2P_MIXED_MARK)
    {
        return TRUE32;
    }
    return FALSE32;
}

/*
 * Name: _setMixedBlockEntry
 * Description: this function adds/changes a block in an existing
 *              mixed entry.
 * Input: vbn, bank, block in the bank
 * Return value: TRUE32 if successful, otherwise FALSE32
 */
static BOOL32
_setMixedBlockEntry
    (PPNVFL_MainCxt * mcxt,
    UInt16 vbn,
    UInt16 mixed_array_offset,
    UInt16 bank,
    UInt16 pbn)
{
    const UInt32 entry_offset = _getMixedArrayEntryOffset(mcxt, vbn, mixed_array_offset);

    if (_isMixedBlock(mcxt, vbn) == FALSE32)
    {
        return FALSE32;
    }

    mcxt->v2p_mixed[entry_offset].pbn = pbn;
    mcxt->v2p_mixed[entry_offset].bank = bank;
    return TRUE32;
}

/*
 * Name: _addNewMixedBlockEntry
 * Description: this function generates a new mixed block entry.
 * Input: vbn, bank, block in the bank
 * Return value: TRUE32 if successful, otherwise FALSE32
 */
static BOOL32
_addNewMixedBlockEntry
    (PPNVFL_MainCxt * mcxt,
    UInt16 vbn,
    UInt16 mixed_array_offset,
    UInt16 bank,
    UInt16 pbn)
{
    const UInt16 main_pbn = mcxt->v2p_main[vbn];
    const UInt16 mixed_entry = mcxt->mixed_entries;
    UInt32 entry_offset;
    UInt16 i;

    if (_isMixedBlock(mcxt, vbn) == TRUE32)
    {
        return FALSE32;
    }

    mcxt->v2p_main[vbn] = mixed_entry | PPN_V2P_MIXED_MARK;
    mcxt->mixed_entries++;
    entry_offset = _getMixedArrayEntryOffset(mcxt, vbn, 0);
    for (i = 0; i < mcxt->dev.num_of_banks; i++)
    {
        mcxt->v2p_mixed[entry_offset + i].pbn = main_pbn;
        mcxt->v2p_mixed[entry_offset + i].bank = i;
    }
    return _setMixedBlockEntry(mcxt, vbn, mixed_array_offset, bank, pbn);
}

/* vfl mixed list functions - end */

/* block replacement functions - start */

#ifndef AND_READONLY

#define PPNVFL_CHECK_REPLACEMENT_BLOCK_WITH_REOPEN (0)
/* debug code - compare vfl cxt before and after opening vfl */
BOOL32
_testVFLCxtAfterOpenCycle
    (PPNVFL_MainCxt * mcxt)
{
    UInt16 temp_bank;
    PPNVFL_MainCxt main_cxt_copy;
    PPNVFL_BankRAMCxt *r_banks_copy;
    UInt16 *v2p_main_copy;
    MixedEntryType *v2p_mixed_copy;
    BOOL32 res = TRUE32;
    UInt16 i;

    r_banks_copy = (PPNVFL_BankRAMCxt*)WMR_MALLOC(mcxt->dev.num_of_banks *
                                                  sizeof(PPNVFL_BankRAMCxt));
    v2p_main_copy = (UInt16*)WMR_MALLOC(sizeof(UInt16) * mcxt->gen.num_of_vfl_sublk);
    v2p_mixed_copy = (MixedEntryType*)WMR_MALLOC(_getV2PMixedArraySize(mcxt));
    WMR_MEMCPY(&main_cxt_copy, mcxt, sizeof(PPNVFL_MainCxt));
    WMR_MEMCPY(r_banks_copy, mcxt->r_banks, (sizeof(PPNVFL_BankRAMCxt) * mcxt->dev.num_of_banks));
    WMR_MEMCPY(v2p_main_copy, mcxt->v2p_main, (sizeof(UInt16) * mcxt->gen.num_of_vfl_sublk));
    WMR_MEMCPY(v2p_mixed_copy, mcxt->v2p_mixed, _getV2PMixedArraySize(mcxt));

    ppnMiscFillDevStruct(&mcxt->dev, mcxt->fil);
    _ppnvflOpen(mcxt, mcxt->r_gen.keepout, 0, 0);
    // compare banks and main info before and after
    if (WMR_MEMCMP(&main_cxt_copy, mcxt, sizeof(PPNVFL_MainCxt)) != 0)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] problem comparing main (line:%d)\n"), __LINE__));
        res = FALSE32;
    }

    for (temp_bank = 0; temp_bank < mcxt->dev.num_of_banks; temp_bank++)
    {
        if (WMR_MEMCMP(&r_banks_copy[temp_bank], &mcxt->r_banks[temp_bank], sizeof(PPNVFL_BankRAMCxt)) != 0)
        {
            VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] problem comparing bank 0x%X RAM(line:%d)\n"), temp_bank, __LINE__));
            res = FALSE32;
        }
    }

    for (i = 0; i < mcxt->gen.num_of_vfl_sublk; i++)
    {
        if ((mcxt->v2p_main[i] & PPN_V2P_MIXED_MARK) != (v2p_main_copy[i] & PPN_V2P_MIXED_MARK))
        {
            VFL_ERR_PRINT((TEXT(
                               "[PPNVFL:ERR] problem comparing v2p_main[0x%04X] org = 0x%04X reopen = 0x%04X (line:%d)\n"),
                           i, mcxt->v2p_main[i] != v2p_main_copy[i], __LINE__));
            res = FALSE32;
        }
        else if (mcxt->v2p_main[i] & PPN_V2P_MIXED_MARK)
        {
            UInt16 j;
            const UInt16 idx_new = mcxt->v2p_main[i] & (~PPN_V2P_MIXED_MARK);
            const UInt16 idx_org = v2p_main_copy[i] & (~PPN_V2P_MIXED_MARK);
            for (j = 0; j < mcxt->dev.num_of_banks; j++)
            {
                const UInt32 entry_offset_new = ((UInt32)idx_new * (UInt32)mcxt->dev.num_of_banks) + (UInt32)j;
                const UInt32 entry_offset_org = ((UInt32)idx_org * (UInt32)mcxt->dev.num_of_banks) + (UInt32)j;
                if (WMR_MEMCMP(&mcxt->v2p_mixed[entry_offset_new],
                               &v2p_mixed_copy[entry_offset_org],
                               sizeof(MixedEntryType)))
                {
                    VFL_ERR_PRINT((TEXT(
                                       "[PPNVFL:ERR] problem comparing v2p_mixed[0x%X][0x%X] org = 0x%04X reopen = 0x%04X (line:%d)\n"),
                                   i, j, mcxt->v2p_mixed[entry_offset_new], v2p_mixed_copy[entry_offset_org], __LINE__));
                    res = FALSE32;
                }
            }
        }
        else if (mcxt->v2p_main[i] != v2p_main_copy[i])
        {
            VFL_ERR_PRINT((TEXT(
                               "[PPNVFL:ERR] problem comparing v2p_main[0x%04X] org = 0x%04X reopen = 0x%04X (line:%d)\n"),
                           i, mcxt->v2p_main[i] != v2p_main_copy[i], __LINE__));
            res = FALSE32;
        }
    }

    WMR_FREE(r_banks_copy, (mcxt->dev.num_of_banks *
                            sizeof(PPNVFL_BankRAMCxt)));
    WMR_FREE(v2p_main_copy, (sizeof(UInt16) * mcxt->gen.num_of_vfl_sublk));
    WMR_FREE(v2p_mixed_copy, (_getV2PMixedArraySize(mcxt)));
    return res;
}

static BOOL32
_setBlockUsageInVFLCxt
    (PPNVFL_MainCxt *mcxt,
    UInt16 bank,
    UInt16 block,
    P2BlockUsageType *new_usage,
    P2BlockUsageType *old_usage)
{
    P2BlockUsageType *p2_block_usage_map;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
    const UInt16 vflcxt_in_page_offset = block % mcxt->r_gen.pbns_per_cxt_mapping_page;
    const UInt16 vflcxt_page_index = block / mcxt->r_gen.pbns_per_cxt_mapping_page;
    BOOL32 retry_program_cxt = FALSE32;

    p2_block_usage_map = (P2BlockUsageType*)buf->pData;

    do
    {
        ProgramVFLCxtPageWrapStatus program_status;
        P2BlockUsageType *on_vflcxt_block_usage = &p2_block_usage_map[vflcxt_in_page_offset];
        retry_program_cxt = FALSE32;
        // mark the block we are replacing as bad
        if (_readVFLCxtPage(mcxt, buf, bank, vflcxt_page_index) == FALSE32)
        {
            VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] error reading vfl cxt (bank:0x%X, index:0x%X, block:0x%X) (line:%d)"),
                           bank, vflcxt_page_index, block, __LINE__));
            goto return_error;
        }

        if ((old_usage != NULL) && WMR_MEMCMP(old_usage, on_vflcxt_block_usage, sizeof(P2BlockUsageType)))
        {
            VFL_ERR_PRINT((TEXT(
                               "[PPNVFL:ERR] old_usage is different than written (bank:0x%X, block:0x%X, old usage:0x%08X, written type: 0x%X) (line:%d)"),
                           bank, block, *((UInt32*)old_usage), *((UInt32*)on_vflcxt_block_usage), __LINE__));
            goto return_error;
        }
        on_vflcxt_block_usage->p2sb.type = PPNVFL_TYPE_GROWN_BB;
        on_vflcxt_block_usage->p2sb.reserved = P2SB_RESERVED_VALUE;

        program_status = _programVFLCxtPageWrap(mcxt, buf, bank, vflcxt_page_index);
        if (program_status == VFLCxtProgramFail)
        {
            VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] error programming vfl cxt (bank:0x%X, index:0x%X) (line:%d)"),
                           bank, vflcxt_page_index, __LINE__));
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

#define PPNVFL_PRINT_SPEICAL_BLOCK_ALLOC_SUMMARY
static BOOL32
_ppnvflAllocateSpecialBlock
    (PPNVFL_MainCxt * mcxt,
    SpecialBlockAddress *chosen_block,
    UInt16 type)
{
    UInt16 bank, i;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
    P2BlockUsageType *p2_block_usage_map = (P2BlockUsageType*)buf->pData;
    BOOL32 retry_allocating = FALSE32;
    UInt32 failed_banks_mask = 0;

#if (defined(PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_BEFORE) && PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_BEFORE)
    _printV2PMapping(mcxt);
#endif

    do
    {
        UInt16 vfl_idx, min_special_blocks = 0xFFFF;;
        UInt16 total_special_blocks = 0;
        chosen_block->bank = 0xFFFF;
        chosen_block->block = 0xFFFF;
        retry_allocating = FALSE32;
        // choose a bank
        for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
        {
            const UInt16 specials_in_bank = mcxt->r_banks[bank].num_special_blocks;
            total_special_blocks += specials_in_bank;
            if (specials_in_bank < min_special_blocks &&
                mcxt->r_banks[bank].num_available != 0 &&
                !(failed_banks_mask & (1UL << bank)))
            {
                chosen_block->bank = bank;
                min_special_blocks = specials_in_bank;
            }
        }
        if (chosen_block->bank == 0xFFFF)
        {
#if AND_SIMULATOR
            WMR_SIM_EXIT("Out of available blocks");
#endif // AND_SIMULATOR
            VFL_WRN_PRINT((TEXT("[PPNVFL:WRN] failed finding a bank to allocate special block (line:%d)\n"), __LINE__));
            goto return_error;
        }
        else if (AND_MAX_SPECIAL_BLOCKS < total_special_blocks)
        {
            VFL_ERR_PRINT((TEXT("total special blocks has exceeded the max allowed\n")));
            goto return_error;
        }
        bank = chosen_block->bank;

        // chose a block
        for (vfl_idx = mcxt->r_gen.pages_per_block_mapping;
             (vfl_idx > 0) && (!retry_allocating) && (chosen_block->block == 0xFFFF); )
        {
            UInt16 cxt_blocks_offset;
            UInt16 blocks_in_cxt_page;
            vfl_idx--;
            cxt_blocks_offset = vfl_idx * mcxt->r_gen.pbns_per_cxt_mapping_page;
            blocks_in_cxt_page = WMR_MIN(mcxt->r_gen.pbns_per_cxt_mapping_page,
                                         (mcxt->dev.blocks_per_cau - cxt_blocks_offset));
            if (_readVFLCxtPage(mcxt, buf, bank,
                                vfl_idx) == FALSE32)
            {
                VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failed _readVFLCxtPage (line:%d)!\n"), __LINE__));
                goto return_error;
            }
            for (i = blocks_in_cxt_page; (i > 0) && (i + cxt_blocks_offset > PPN_SPECIAL_BLOCK_LIMIT(&mcxt->dev)) && (!retry_allocating) && (chosen_block->block == 0xFFFF); )
            {
                i--;
                if (p2_block_usage_map[i].p2sb.type == PPNVFL_TYPE_AVAILABLE_MARK)
                {
                    ProgramVFLCxtPageWrapStatus program_status;
                    UInt16 potential_block = i + cxt_blocks_offset;
                    if (!ppnMiscTestSpecialBlock(&mcxt->dev, mcxt->commands[0], bank, potential_block, PPNVFL_SPARE_TYPE_TST))
                    {
                        VFL_WRN_PRINT((TEXT(
                                           "[PPNVFL:WRN] failed testing block (bank:0x%X, block:0x%X) for sb allocation (line:%d)"),
                                       bank, potential_block, __LINE__));

                        p2_block_usage_map[i].p2sb.type = PPNVFL_TYPE_GROWN_BB;
                        p2_block_usage_map[i].p2sb.reserved = P2SB_RESERVED_VALUE;
                    }
                    else
                    {
                        chosen_block->block = potential_block;
                        p2_block_usage_map[i].p2sb.type = type;
                        p2_block_usage_map[i].p2sb.reserved = P2SB_RESERVED_VALUE;
                    }
                    // program the update back to the NAND
                    program_status = _programVFLCxtPageWrap(mcxt, buf, bank, vfl_idx);
                    if (program_status == VFLCxtProgramFail)
                    {
                        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] error programming vfl cxt (bank:0x%X, i:0x%X) (line:%d)"),
                                       bank, i, __LINE__));
                        goto return_error;
                    }
                    else if (program_status == VFLCxtProgramRetry)
                    {
                        retry_allocating = TRUE32;
                    }
                    else
                    {
                        // mark the change in the VFL RAM structures
                        mcxt->r_banks[bank].num_available--;
                        if (p2_block_usage_map[i].p2sb.type == PPNVFL_TYPE_GROWN_BB)
                        {
                            mcxt->r_banks[bank].num_grown_bad++;
                            retry_allocating = TRUE32;
                        }
                        else
                        {
                            mcxt->r_banks[bank].num_special_blocks++;
                        }
                    }
                }
            }
        }
        if ((!retry_allocating) && (chosen_block->block == 0xFFFF))
        {
            failed_banks_mask |= 1UL << bank;
            retry_allocating = TRUE32;
        }
    }
    while (retry_allocating);

    if (chosen_block->block == 0xFFFF)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failed to find a new special block (bank:0x%X)(line:%d)!\n"),
                       bank, __LINE__));
        goto return_error;
    }

    if (_programCxtLastPage(mcxt, buf, TRUE32) == FALSE32)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] error programming last vfl cxt (bank:0x%X) (line:%d)"),
                       bank, __LINE__));
        goto return_error;
    }

#if (defined(PPNVFL_CHECK_REPLACEMENT_BLOCK_WITH_REOPEN) && PPNVFL_CHECK_REPLACEMENT_BLOCK_WITH_REOPEN)
#if (defined(PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_AFTER) && PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_AFTER)
    _printV2PMapping(mcxt);
#endif // PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_AFTER
    _testVFLCxtAfterOpenCycle(mcxt);
#endif // PPNVFL_CHECK_REPLACEMENT_BLOCK_WITH_REOPEN
    if (!mcxt->fpart->AllocateSpecialBlockType(chosen_block, 1, type))
    {
        VFL_ERR_PRINT((TEXT("fpart failed to allocate special block with type %p\n"), type));
        goto return_error;
    }
    BUF_Release(buf);
    return TRUE32;

 return_error:
    BUF_Release(buf);
    return FALSE32;
}

#define PPNVFL_PRINT_BLOCK_REPLACEMENT (1)
#if (defined(PPNVFL_PRINT_BLOCK_REPLACEMENT) && PPNVFL_PRINT_BLOCK_REPLACEMENT)
#define PPNVFL_PRINT_BLOCK_REPLACEMENT_SUMMARY (1)
#define PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_BEFORE (0)
#define PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_AFTER (0)
#else
#define PPNVFL_PRINT_BLOCK_REPLACEMENT_SUMMARY (0)
#define PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_BEFORE (0)
#define PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_AFTER (0)
#endif

static BOOL32
_replaceBadBlock
    (PPNVFL_MainCxt * mcxt,
    UInt16 replaced_bank,
    UInt16 replaced_block,
    UInt16 vbn,
    UInt16 mixed_array_offset)
{
    UInt16 i, j, new_block = 0xFFFF, new_bank = replaced_bank;
    P2BlockUsageType *p2_block_usage_map;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
    const UInt16 blocks_in_cxt_page = mcxt->r_gen.pbns_per_cxt_mapping_page;
    BOOL32 retry_program_cxt = FALSE32;
    P2BlockUsageType old_usage, new_usage;

    p2_block_usage_map = (P2BlockUsageType*)buf->pData;

#if (defined(PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_BEFORE) && PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_BEFORE)
    _printV2PMapping(mcxt);
#endif

    do
    {
        ProgramVFLCxtPageWrapStatus program_status;
        // search for an available block
        // read the cxt pages search for replacement blocks

        retry_program_cxt = FALSE32;
        if (mcxt->r_banks[new_bank].num_available == 0)
        {
            UInt16 max_avail_bank = 0xFFFF, max_avail_count = 0, bank;

            VFL_WRN_PRINT((TEXT(
                               "[PPNVFL:WRN] can't find available blocks in the original bank - look elsewhere (bank:0x%X, block:0x%X)(line:%d)!\n"),
                           replaced_bank, replaced_block, __LINE__));
            for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
            {
                if (max_avail_count < mcxt->r_banks[bank].num_available)
                {
                    max_avail_count = mcxt->r_banks[bank].num_available;
                    max_avail_bank = bank;
                }
            }
            if (max_avail_bank == 0xFFFF)
            {
#if (defined(AND_SIMULATOR) && AND_SIMULATOR)
                WMR_SIM_EXIT("Out of replacement blocks");
#else // defined(AND_SIMULATOR) && AND_SIMULATOR
                VFL_ERR_PRINT((TEXT(
                                   "[PPNVFL:ERR] failed - no replacement blocks in any of the banks (bank:0x%X, block:0x%X)(line:%d)!\n"),
                               replaced_bank, replaced_block, __LINE__));
#endif // defined(AND_SIMULATOR) && AND_SIMULATOR
                goto return_error;
            }
            new_bank = max_avail_bank;
        }
        for (i = 0; ((!retry_program_cxt) && (i < mcxt->r_gen.pages_per_block_mapping) &&
                     (new_block == 0xFFFF)); i++)
        {
            if (_readVFLCxtPage(mcxt, buf, new_bank, i) == FALSE32)
            {
                VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failed _readVFLCxtPage (line:%d)!\n"), __LINE__));
                goto return_error;
            }
            for (j = 0; j < blocks_in_cxt_page; j++)
            {
                if (p2_block_usage_map[j].p2sb.type == PPNVFL_TYPE_AVAILABLE_MARK)
                {
                    UInt16 potential_new_block = j + (i * blocks_in_cxt_page);
                    p2_block_usage_map[j].p2v.vbn = vbn;
                    p2_block_usage_map[j].p2v.offset = (UInt8)mixed_array_offset;
                    p2_block_usage_map[j].p2v.reserved = P2V_RESERVED_VALUE;

                    // program the update back to the NAND
                    program_status = _programVFLCxtPageWrap(mcxt, buf, new_bank, i);
                    if (program_status == VFLCxtProgramFail)
                    {
                        VFL_ERR_PRINT((TEXT(
                                           "[PPNVFL:ERR] error programming vfl cxt (bank:0x%X, i:0x%X, j:0x%X) (line:%d)"),
                                       new_bank, i, j, __LINE__));
                        goto return_error;
                    }
                    else if (program_status == VFLCxtProgramRetry)
                    {
                        retry_program_cxt = TRUE32;
                        break;
                    }
                    new_block = potential_new_block;
                    // mark the change in the VFL RAM structures
                    if (_isMixedBlock(mcxt, vbn) == TRUE32)
                    {
                        _setMixedBlockEntry(mcxt, vbn, mixed_array_offset, new_bank, new_block);
                    }
                    else
                    {
                        _addNewMixedBlockEntry(mcxt, vbn, mixed_array_offset, new_bank, new_block);
                    }
                    mcxt->r_banks[new_bank].num_available--;
                    mcxt->r_banks[new_bank].num_p2v++;
                    mcxt->r_banks[replaced_bank].num_p2v--;
                    mcxt->r_banks[replaced_bank].num_grown_bad++;
                    VFL_INF_PRINT(PPNVFL_PRINT_BLOCK_REPLACEMENT_SUMMARY,
                                  (TEXT(
                                       "[PPNVFL:INF] block replacement (vbn:0x%X, array_offset:0x%X, new_bank:0x%X, new_block:0x%X) (l:%d)\n"),
                                   vbn, mixed_array_offset, new_bank, new_block, __LINE__));
                    break;
                }
            }
        }

        if ((!retry_program_cxt) && (new_block == 0xFFFF))
        {
            VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failed to find replacement block (bank:0x%X, block:0x%X)(line:%d)!\n"),
                           replaced_bank, replaced_block, __LINE__));
            goto return_error;
        }
    }
    while (retry_program_cxt);

    BUF_Release(buf);
    buf = NULL;
    old_usage.p2v.vbn = vbn;
    old_usage.p2v.offset = (UInt8)mixed_array_offset;
    old_usage.p2v.reserved = P2V_RESERVED_VALUE;
    new_usage.p2sb.type = PPNVFL_TYPE_GROWN_BB;
    new_usage.p2sb.reserved = P2SB_RESERVED_VALUE;
    if (!_setBlockUsageInVFLCxt(mcxt, replaced_bank, replaced_block, &new_usage, &old_usage))
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failed to mark block as bad (bank:0x%X, block:0x%X)(line:%d)!\n"),
                       replaced_bank, replaced_block, __LINE__));
        goto return_error;
    }
    buf = BUF_Get(BUF_MAIN_AND_SPARE);

    // find the block we replaced in the scrub list and remove it
    for (i = 0; i < mcxt->gen.scrub_idx; i++)
    {
        if (replaced_block == mcxt->gen.scrub_list[i].block &&
            replaced_bank == mcxt->gen.scrub_list[i].bank)
        {
            if (i != mcxt->gen.scrub_idx - 1)
            {
                // move the block in the last scrub index to the place of the one
                // we are replacing so we can substruct scrub_idx
                mcxt->gen.scrub_list[i].block = mcxt->gen.scrub_list[mcxt->gen.scrub_idx - 1].block;
                mcxt->gen.scrub_list[i].bank = mcxt->gen.scrub_list[mcxt->gen.scrub_idx - 1].bank;
            }
            mcxt->gen.scrub_idx--;
            break;
        }
    }

    if (_programCxtLastPage(mcxt, buf, TRUE32) == FALSE32)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] error programming last vfl cxt (bank:0x%X, index:0x%X) (line:%d)"),
                       replaced_bank, replaced_block, __LINE__));
        goto return_error;
    }

    VFL_WRN_PRINT((TEXT(
                       "[PPNVFL:WRN] replacing vbn 0x%X, org (bank:0x%X, block:0x%X), new (bank:0x%X, block:0x%X) (line:%d)!\n"),
                   vbn, replaced_bank, replaced_block, new_bank, new_block, __LINE__));
#if (defined(PPNVFL_CHECK_REPLACEMENT_BLOCK_WITH_REOPEN) && PPNVFL_CHECK_REPLACEMENT_BLOCK_WITH_REOPEN)
    _testVFLCxtAfterOpenCycle(mcxt);
#if (defined(PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_AFTER) && PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_AFTER)
    _printV2PMapping(mcxt);
#endif // PPNVFL_PRINT_BLOCK_REPLACEMENT_MAPPING_AFTER
#endif // PPNVFL_CHECK_REPLACEMENT_BLOCK_WITH_REOPEN
    BUF_Release(buf);
    return TRUE32;

 return_error:
    if (buf != NULL)
    {
        BUF_Release(buf);
    }
    WMR_PANIC("_replaceBadBlock() fail bank 0x%x block 0x%x vbn 0x%x offset 0x%x\n", replaced_bank, replaced_block, vbn,
              mixed_array_offset);
    return FALSE32;
}

static BOOL32
_replaceErasedCxtBlock
    (PPNVFL_MainCxt * mcxt,
    UInt16 vfl_index)
{
    UInt16 i, j, new_block = 0xFFFF, new_bank = 0xFFFF, new_block_index = 0xFFFF, new_block_cxt_page = 0xFFFF;
    UInt16 bank, max_available, min_cxt_blocks;
    P2BlockUsageType *p2_block_usage_map = NULL;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
    const UInt16 blocks_in_cxt_page = mcxt->r_gen.pbns_per_cxt_mapping_page;
    const UInt16 replaced_block = mcxt->gen.vfl_blocks[vfl_index].block;
    const UInt16 replaced_bank = mcxt->gen.vfl_blocks[vfl_index].bank;
    const UInt16 replaced_index = replaced_block % blocks_in_cxt_page;
    const UInt16 replaced_cxt_page = replaced_block / blocks_in_cxt_page;
    const UInt16 blocks_per_vfl_set = (mcxt->r_gen.num_of_vfl_blocks >> 1);
    const UInt16 set_mapping_offset = mcxt->r_gen.next_cxt_block_offset * blocks_per_vfl_set;

    PPNStatusType status;

    p2_block_usage_map = (P2BlockUsageType*)buf->pData;

    _printVFLBlockMapping(mcxt, __LINE__);

    // chose a bank to work with
    // pick the bank with maximum available blocks and minimum cxt blocks
    max_available = 0;
    min_cxt_blocks = 0xFFFF;
    for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
    {
        if (mcxt->r_banks[bank].num_available)
        {
            if (new_bank == 0xFFFF)
            {
                new_bank = bank;
                min_cxt_blocks = mcxt->r_banks[bank].num_cxt;
                max_available = mcxt->r_banks[bank].num_available;
            }
            else if (mcxt->r_banks[bank].num_cxt < min_cxt_blocks)
            {
                min_cxt_blocks = mcxt->r_banks[bank].num_cxt;
                max_available = mcxt->r_banks[bank].num_available;
                new_bank = bank;
            }
            else if (mcxt->r_banks[bank].num_cxt == min_cxt_blocks)
            {
                if (mcxt->r_banks[bank].num_available > max_available)
                {
                    min_cxt_blocks = mcxt->r_banks[bank].num_cxt;
                    max_available = mcxt->r_banks[bank].num_available;
                    new_bank = bank;
                }
            }
        }
    }

    // verify a bank was chosen
    if (new_bank == 0xFFFF)
    {
#if AND_SIMULATOR
        WMR_SIM_EXIT("Out of replacement blocks");
#endif // AND_SIMULATOR
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failed finding a bank to provide vfl cxt blocks (line:%d)!\n"),
                       __LINE__));
        goto return_error;
    }

    // pick a replacement block
    for (i = 0; (i < mcxt->r_gen.pages_per_block_mapping) && (new_block == 0xFFFF); i++)
    {
        if (_readVFLCxtPage(mcxt, buf, new_bank, i) == FALSE32)
        {
            VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failed _readVFLCxtPage (line:%d)!\n"), __LINE__));
            goto return_error;
        }
        for (j = 0; j < blocks_in_cxt_page; j++)
        {
            UInt16 potential_new_block = j + (i * blocks_in_cxt_page);
            if (p2_block_usage_map[j].p2sb.type == PPNVFL_TYPE_AVAILABLE_MARK)
            {
                // test the block - do not mark blocks as bad if they fail, just continue
                if (ppnMiscTestSpecialBlock(&mcxt->dev, mcxt->commands[0], new_bank, potential_new_block, PPNVFL_SPARE_TYPE_TST))
                {
                    new_block = potential_new_block;
                    new_block_index = j;
                    new_block_cxt_page = i;
                    mcxt->gen.vfl_blocks[vfl_index].block = new_block;
                    mcxt->gen.vfl_blocks[vfl_index].bank = new_bank;
                    break;
                }
                else
                {
                    VFL_WRN_PRINT((TEXT(
                                       "[PPNVFL:WRN] failed testing replacement block (bank:0x%X, block:0x%X)(line:%d)!\n"),
                                   new_bank, potential_new_block, __LINE__));
                }
            }
        }
    }

    // verify a block was found
    if (new_block == 0xFFFF)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failed searching for replacement block (bank:0x%X, block:0x%X)(line:%d)!\n"),
                       replaced_bank, replaced_block, __LINE__));
        goto return_error;
    }

    _printVFLBlockMapping(mcxt, __LINE__);
    // erase original - ignore status
    ppnMiscSingleOperation(&mcxt->dev, mcxt->single_command, PPN_COMMAND_ERASE,
                           PPN_OPTIONS_IGNORE_BAD_BLOCK, replaced_bank, replaced_block, 0,
                           TRUE32, NULL, NULL);

    // erase current vfl set - fail replacement operation if we get an erase error
    for (i = 0; i < blocks_per_vfl_set; i++)
    {
        status = ppnMiscSingleOperation(&mcxt->dev, mcxt->single_command, PPN_COMMAND_ERASE,
                                        PPN_NO_OPTIONS,
                                        mcxt->gen.vfl_blocks[set_mapping_offset + i].bank,
                                        mcxt->gen.vfl_blocks[set_mapping_offset + i].block,
                                        0, TRUE32, NULL, NULL);
        if (status & (PPN_ERASE_STATUS_RETIRE | PPN_ERASE_STATUS_FAIL | PPN_PROGRAM_STATUS_GEB))
        {
            WMR_PRINT(QUAL_FATAL,
                      "failed erase new vfl block (bank:0x%04X, block: 0x%04X) with status 0x%02X\n",
                      mcxt->gen.vfl_blocks[set_mapping_offset + i].bank,
                      mcxt->gen.vfl_blocks[set_mapping_offset + i].block, status);
            goto return_error;
        }
    }
    // program last cxt page
    mcxt->r_gen.next_cxt_page_offset = 0;
    _copyFromRamToOnNandCxt(mcxt, buf->pData);
    mcxt->r_gen.cxt_age--;
    mcxt->r_gen.need_rewrite = FALSE32;

    if (!_programVFLCxtPage(mcxt, buf, PPNVFL_CXT_LAST_BANKIDX, PPNVFL_CXT_LAST_BANKIDX))
    {
        VFL_ERR_PRINT((TEXT(
                           "[PPNVFL:ERR] _programVFLCxtPage failed while allocating new vfl block (bank:0x%04X, block: 0x%04X) (line:%d)\n"),
                       new_bank, new_block, __LINE__));
        goto return_error;
    }

    for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
    {
        for (i = 0; i < mcxt->r_gen.pages_per_block_mapping; i++)
        {
            if (_readVFLCxtPage(mcxt, buf, bank, i) == FALSE32)
            {
                VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failed _readVFLCxtPage (line:%d)!\n"), __LINE__));
                goto return_error;
            }

            if (new_block_cxt_page == i && new_bank == bank)
            {
                p2_block_usage_map[new_block_index].p2sb.type = PPNVFL_TYPE_VFL_INFO;
                p2_block_usage_map[new_block_index].p2sb.reserved = P2SB_RESERVED_VALUE;
                mcxt->r_banks[bank].num_available--;
                mcxt->r_banks[bank].num_cxt++;
            }

            if (replaced_cxt_page == i && replaced_bank == bank)
            {
                p2_block_usage_map[replaced_index].p2sb.type = PPNVFL_TYPE_GROWN_BB;
                p2_block_usage_map[replaced_index].p2sb.reserved = P2SB_RESERVED_VALUE;
                mcxt->r_banks[bank].num_cxt--;
                mcxt->r_banks[bank].num_grown_bad++;
            }
            if (!_programVFLCxtPage(mcxt, buf, bank, i))
            {
                VFL_ERR_PRINT((TEXT(
                                   "[PPNVFL:ERR] _programVFLCxtPage failed with the new vfl block (bank:0x%04X, block: 0x%04X) (line:%d)\n"),
                               new_bank, new_block, __LINE__));
                goto return_error;
            }
        }
    }

    // check if the original block is in the scrub list, if so remove it
    for (i = 0; i < mcxt->gen.scrub_idx; i++)
    {
        if (replaced_block == mcxt->gen.scrub_list[i].block && replaced_bank == mcxt->gen.scrub_list[i].bank)
        {
            if (i != mcxt->gen.scrub_idx - 1)
            {
                // move the block in the last scrub index to the place of the one
                // we are replacing so we can substruct scrub_idx
                mcxt->gen.scrub_list[i].bank = mcxt->gen.scrub_list[mcxt->gen.scrub_idx - 1].bank;
                mcxt->gen.scrub_list[i].block = mcxt->gen.scrub_list[mcxt->gen.scrub_idx - 1].block;
            }
            mcxt->gen.scrub_idx--;
            break;
        }
    }

    // program last cxt page
    _copyFromRamToOnNandCxt(mcxt, buf->pData);

    if (!_programVFLCxtPage(mcxt, buf, PPNVFL_CXT_LAST_BANKIDX, PPNVFL_CXT_LAST_BANKIDX))
    {
        VFL_ERR_PRINT((TEXT(
                           "[PPNVFL:ERR] _programVFLCxtPage failed with the new vfl block (bank:0x%04X, block: 0x%04X) (line:%d)\n"),
                       new_bank, new_block, __LINE__));
        goto return_error;
    }

    VFL_WRN_PRINT((TEXT(
                       "[PPNVFL:WRN] replacing cxt_idx 0x%X, org (bank:0x%X, block:0x%X), new (bank:0x%X, block:0x%X) (line:%d)!\n"),
                   vfl_index, replaced_bank, replaced_block, new_bank, new_block, __LINE__));

    // erase the set of blocks we want to use
    for (i = 0; i < blocks_per_vfl_set; i++)
    {
        const UInt16 old_set_location = (mcxt->r_gen.next_cxt_block_offset == 1 ? 0 : 1);
        const UInt16 old_set_mapping_offset = old_set_location * blocks_per_vfl_set;
        status = ppnMiscSingleOperation(&mcxt->dev, mcxt->single_command, PPN_COMMAND_ERASE,
                                        PPN_NO_OPTIONS,
                                        mcxt->gen.vfl_blocks[old_set_mapping_offset + i].bank,
                                        mcxt->gen.vfl_blocks[old_set_mapping_offset + i].block,
                                        0,
                                        TRUE32, NULL, NULL);

        if (status & PPN_ERASE_STATUS_RETIRE)
        {
            VFL_WRN_PRINT((TEXT(
                               "[PPNVFL:WRN] failed erase vflcxt block (bank:0x%04X, block: 0x%04X) with retire status (line:%d)\n"),
                           mcxt->gen.vfl_blocks[old_set_mapping_offset + i].bank,
                           mcxt->gen.vfl_blocks[old_set_mapping_offset + i].block, __LINE__));
            if (!_markBlockForReplacement(mcxt, mcxt->gen.vfl_blocks[old_set_mapping_offset + i].bank,
                                          mcxt->gen.vfl_blocks[old_set_mapping_offset + i].block))
            {
                VFL_ERR_PRINT((TEXT(
                                   "[PPNVFL:ERR] failed marking vflcxt block (bank:0x%04X, block: 0x%04X) for scrub (line:%d)\n"),
                               mcxt->gen.vfl_blocks[old_set_mapping_offset + i].bank,
                               mcxt->gen.vfl_blocks[old_set_mapping_offset + i].block, __LINE__));
            }
        }
        else if (status & PPN_ERASE_STATUS_FAIL)
        {
            VFL_ERR_PRINT((TEXT(
                               "[PPNVFL:ERR] failed erase vflcxt block (bank:0x%04X, block: 0x%04X) with no retire status (line:%d)\n"),
                           mcxt->gen.vfl_blocks[old_set_mapping_offset + i].bank,
                           mcxt->gen.vfl_blocks[old_set_mapping_offset + i].block, __LINE__));
            goto return_error;
        }
    }

    BUF_Release(buf);
    return TRUE32;

 return_error:
    BUF_Release(buf);
    return FALSE32;
}

#endif // ! AND_READONLY

static BOOL32
_markBlockForReplacementNoProgram
    (PPNVFL_MainCxt * mcxt,
    UInt16 bank,
    UInt16 block)
{
    UInt16 i;

    // check if the block is allready in the scrub list
    for (i = 0; i < mcxt->gen.scrub_idx; i++)
    {
        if (mcxt->gen.scrub_list[i].block == block && mcxt->gen.scrub_list[i].bank == bank)
        {
            return TRUE32;
        }
    }

    if (mcxt->gen.scrub_idx >= PPNVFL_SCRUB_BLOCK_COUNT)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR]  scrub list is full bank 0x%X (line:%d)!\n"),
                       bank, __LINE__));
        return FALSE32;
    }

    mcxt->gen.scrub_list[mcxt->gen.scrub_idx].block = block;
    mcxt->gen.scrub_list[mcxt->gen.scrub_idx].bank = bank;
    mcxt->gen.scrub_idx++;

    return TRUE32;
}

#ifndef AND_READONLY

static BOOL32
_markBlockForReplacement
    (PPNVFL_MainCxt * mcxt,
    UInt16 bank,
    UInt16 block)
{
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
    BOOL32 ret_val;

    if (_markBlockForReplacementNoProgram(mcxt, bank, block) == FALSE32)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR]  fail _markBlockForReplacementNoProgram (bank:0x%X) (line:%d)!\n"),
                       bank, __LINE__));
        BUF_Release(buf);
        return FALSE32;
    }

    ret_val = _programCxtLastPage(mcxt, buf, TRUE32);
    BUF_Release(buf);

    return ret_val;
}
#endif // AND_READONLY
/* block replacement functions - end */

/*
 * Name: _analyzeMultiReadStatuses
 * Description: this function analyzes the read operation statuses (used for multipage read)
 * Input: number of pages read, mark for scrub
 * Output: need refresh flag, operation result flag, clean flag
 * Return value: none
 */
void
_analyzeMultiReadStatuses
    (PPNVFL_MainCxt * mcxt,
    UInt32 *vpn_array,
    UInt8 *spare,
    UInt32 *result_flags,
    BOOL32 boolMarkForScrub,
    UInt8 *pabSectorStats)
{
    UInt16 channel;
    UInt16 phyCE;
    BOOL32 last_failure_updated = FALSE32;
    UInt32 stats_offset;
    const UInt8 num_sectors = (mcxt->dev.main_bytes_per_page) / SECTOR_SIZE;   //Number of kilobyte sector

    // look at the status
    for (channel = 0; channel < mcxt->dev.num_channels; channel++)
    {
        UInt16 i;

        // locate the block that need to be replaced
        for (i = 0; i < mcxt->commands[channel]->num_pages; i++)
        {
            PPNStatusType current_status = mcxt->commands[channel]->entry[i].status;
            UInt32 current_result = 0, actions = 0;
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
                    current_result |= PPNVFL_READ_RES_UNIDENTIFIED;
                    break;
            }
            if (!(current_result & PPNVFL_READ_RES_UNIDENTIFIED))
            {
                if (current_status & PPN_READ_STATUS_INVALID_DATA &&
                    (!(current_status & PPN_READ_STATUS_CLEAN)))
                {
                    current_result |= PPNVFL_READ_RES_UECC;
                    if(pabSectorStats)
                    {
                        stats_offset = (mcxt->commands[channel]->mem_index[i].offset) * num_sectors;
                        WMR_MEMSET(pabSectorStats + stats_offset, 0xFF, num_sectors);
                    }
                }
                if (current_status & PPN_READ_STATUS_CLEAN)
                {
                    current_result |= PPNVFL_READ_RES_CLEAN;
                    if(pabSectorStats)
                    {
                        stats_offset = (mcxt->commands[channel]->mem_index[i].offset) * num_sectors;
                        WMR_MEMSET(pabSectorStats + stats_offset, 0xFE, num_sectors);
                    }
                }
                if (current_status & PPN_READ_STATUS_REFRESH)
                {
                    current_result |= PPNVFL_READ_RES_REFRESH;
                }
                if (current_status & PPN_READ_STATUS_RETIRE)
                {
                    current_result |= PPNVFL_READ_RES_RETIRE;
                }
                if (!(current_status & PPN_READ_STATUS_INVALID_DATA))
                {
                    current_result |= PPNVFL_READ_RES_VALID_DATA;
                }
            }
            *result_flags |= current_result;
            if (current_result &
                (PPNVFL_READ_RES_REFRESH | PPNVFL_READ_RES_RETIRE |
                 PPNVFL_READ_RES_UECC | PPNVFL_READ_RES_UNIDENTIFIED))
            {
                const UInt16 chip_enable_idx = mcxt->commands[channel]->entry[i].ceIdx;
                const PageAddressType page = mcxt->commands[channel]->entry[i].addr.row;
                UInt16 bank, block, page_offset;
                BOOL32 slc;
                UInt16 transaction_index = mcxt->commands[channel]->mem_index[i].offset;
                UInt32 vpn = (vpn_array == NULL ? 0xFFFFFFFF : vpn_array[transaction_index]);
                UInt8 *curr_spare = &spare[transaction_index * mcxt->dev.lba_meta_bytes_buffer];

                // reverse the address to bank, block, page offset, slc type address
                ppnMiscConvertPhysicalAddressToBankBlockPage(&mcxt->dev, channel,
                                                             chip_enable_idx, page, &bank, &block,
                                                             &page_offset, &slc);

                if ((!last_failure_updated) && (current_result & PPNVFL_READ_RES_UECC))
                {
                    actions |= 1;
                    last_failure_updated = TRUE32;
                    phyCE = ppnMiscGetCEFromBank(&mcxt->dev,bank);
                    _ReportFailure(VFLFailUECC, phyCE, page);
                }
                else if (current_result & VFL_READ_STATUS_REFRESH)
                {
                    phyCE = ppnMiscGetCEFromBank(&mcxt->dev,bank);
                    _ReportFailure(VFLFailRefresh, phyCE, page);
                }

#ifndef AND_READONLY
                if (boolMarkForScrub && (current_status & PPN_READ_STATUS_RETIRE))
                {
                    _markBlockForReplacement(mcxt, bank, block);
                    actions |= 2;
                }
#endif // ! AND_READONLY

                VFL_WRN_PRINT((TEXT(
                                   "[PPNVFL:WRN] reporting a read error status = 0x%X actions:0x%X vpn:0x%X result:0x%X (l:%d)\n"),
                               current_status, actions, vpn, current_result, __LINE__));
                VFL_WRN_PRINT((TEXT("[PPNVFL:WRN] bank:0x%X, block:0x%X, page_offset:0x%X, slc:0x%X\n"),
                               bank, block, page_offset, slc));
                VFL_WRN_PRINT((TEXT("[PPNVFL:WRN] channel:0x%X, chip_enable_idx:0x%X, page_address:0x%X\n"),
                               channel, chip_enable_idx, page));
                VFL_WRN_PRINT((TEXT(
                                   "[PPNVFL:WRN] meta: %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X\n"),
                               curr_spare[0], curr_spare[1], curr_spare[2], curr_spare[3],
                               curr_spare[4], curr_spare[5], curr_spare[6], curr_spare[7],
                               curr_spare[8], curr_spare[9], curr_spare[10], curr_spare[11],
                               curr_spare[12], curr_spare[13], curr_spare[14], curr_spare[15]));
            }
        }
    }

    if ((*result_flags &
         (PPNVFL_READ_RES_REFRESH | PPNVFL_READ_RES_RETIRE |
          PPNVFL_READ_RES_UECC | PPNVFL_READ_RES_UNIDENTIFIED)) ||
        ((*result_flags & PPNVFL_READ_RES_CLEAN) &&
         (*result_flags & PPNVFL_READ_RES_VALID_DATA)))
    {
        VFL_WRN_PRINT((TEXT("[PPNVFL:WRN] *result_flags:0x%X (line:%d)\n"), *result_flags, __LINE__));
        VFL_WRN_PRINT((TEXT(
                           "[PPNVFL:WRN] st vpn      ba blk  ofs s ch ce  len  col  row       meta (line:%d)\n"),
                       __LINE__));
        for (channel = 0; channel < mcxt->dev.num_channels; channel++)
        {
            UInt16 i;

            // locate the block that need to be replaced
            for (i = 0; i < mcxt->commands[channel]->num_pages; i++)
            {
                const PPNCommandEntry *entry = &mcxt->commands[channel]->entry[i];
                const PPNStatusType current_status = entry->status;
                const UInt16 chip_enable_idx = entry->ceIdx;
                const PageAddressType page = entry->addr.row;
                const UInt16 transaction_index = mcxt->commands[channel]->mem_index[i].offset;
                const UInt8 *curr_spare = &spare[transaction_index * mcxt->dev.lba_meta_bytes_buffer];
                const UInt32 vpn = (vpn_array == NULL ? 0xFFFFFFFF : vpn_array[transaction_index]);
                UInt16 bank, block, page_offset;
                BOOL32 slc;

                // reverse the address to bank, block, page offset, slc type address
                ppnMiscConvertPhysicalAddressToBankBlockPage(&mcxt->dev, channel,
                                                             chip_enable_idx, page, &bank, &block,
                                                             &page_offset, &slc);

                VFL_WRN_PRINT((TEXT(
                                   "[PPNVFL:WRN] %02X %08X %02X %04X %03X %01X %02X %02X"
                                   "  %04X %04X %08X"
                                   "  %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X\n"),
                               current_status, vpn, bank, block, page_offset, slc, channel, chip_enable_idx,
                               entry->addr.length, entry->addr.column, entry->addr.row,
                               curr_spare[0], curr_spare[1], curr_spare[2], curr_spare[3],
                               curr_spare[4], curr_spare[5], curr_spare[6], curr_spare[7],
                               curr_spare[8], curr_spare[9], curr_spare[10], curr_spare[11],
                               curr_spare[12], curr_spare[13], curr_spare[14], curr_spare[15]));
            }
        }
    }
    if (!(*result_flags))
    {
        WMR_PANIC("result flags: 0x%02x\n", *result_flags);
    }
}

static void
_copyMapPagesFromOnNandCxtToRam
    (PPNVFL_MainCxt * mcxt,
    UInt8 *data)
{
    UInt16 bank;
    PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)data;

    for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
    {
        WMR_MEMCPY(mcxt->vfl_map_pages[bank], on_nand->banks[bank].vfl_map_pages,
                   mcxt->r_gen.pages_per_block_mapping * sizeof(UInt16));
    }
}

static void
_copyFromOnNandCxtToRam
    (PPNVFL_MainCxt * mcxt,
    UInt8 *data)
{
    PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)data;

    // copy gen
    WMR_MEMCPY(&mcxt->gen, &on_nand->gen, sizeof(PPNVFL_GeneralCxt));
    // copy vfl map locations
    _copyMapPagesFromOnNandCxtToRam(mcxt, data);
}

#if !AND_READONLY

static void
_copyFromRamToOnNandCxt
    (PPNVFL_MainCxt * mcxt,
    UInt8 *data)
{
    UInt16 bank;
    PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)data;

    WMR_MEMSET(data, 0, mcxt->dev.main_bytes_per_page);
    // copy gen
    WMR_MEMCPY(&on_nand->gen, &mcxt->gen, sizeof(PPNVFL_GeneralCxt));

    for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
    {
        WMR_MEMCPY(on_nand->banks[bank].vfl_map_pages, mcxt->vfl_map_pages[bank],
                   mcxt->r_gen.pages_per_block_mapping * sizeof(UInt16));
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
    (PPNVFL_MainCxt * mcxt,
    UInt16 set,
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
    const BOOL32 original_need_rewrite = mcxt->r_gen.need_rewrite;

    _convertVFLPageIdxToPhysicalPageBlockOffsets(mcxt, set, cxt_page_offset,
                                                 &vfl_block_offset, &vfl_page_offset);

    for (redundant_copy = 0; redundant_copy < mcxt->r_gen.vfl_blocks_redundancy; redundant_copy++)
    {
        const UInt16 bank = mcxt->gen.vfl_blocks[vfl_block_offset + redundant_copy].bank;
        const UInt16 block = mcxt->gen.vfl_blocks[vfl_block_offset + redundant_copy].block;
        PPNVFL_CxtSpare *cxt_spare = (PPNVFL_CxtSpare*) buf->pSpare;

        status = ppnMiscSingleOperation(&mcxt->dev, mcxt->single_command, PPN_COMMAND_READ, PPN_NO_OPTIONS,
                                        bank, block, vfl_page_offset, TRUE32, buf->pData, buf->pSpare);

        // check if this is not the first copy or if the copy is marked to be moved
        if (((redundant_copy != 0) ||
             (status & (PPN_READ_STATUS_REFRESH | PPN_READ_STATUS_RETIRE))) &&
            (set == mcxt->r_gen.next_cxt_block_offset))
        {
            // avoid the prints when searching through clean pages...
            // print only if we ran into none clean pages
            if (!clean)
            {
                VFL_WRN_PRINT((TEXT("[PPNVFL:WRN] read a copy of the vfl cxt with status 0x%02X"
                          " set:%01d, copy:%01d, page_offset:0x%03X type:0x%02X (line:%d)\n"),
                          status, set, redundant_copy, cxt_page_offset, cxt_spare->spareType, __LINE__));
            }
            
            mcxt->r_gen.need_rewrite = TRUE32;
        }

        // if this is not initial search for cxt we should take the indication
        // for replacement from the device
#ifndef AND_READONLY
        if ((status & PPN_READ_STATUS_RETIRE) && (!mcxt->r_gen.search_for_cxt))
        {
            _markBlockForReplacementNoProgram(mcxt, bank, block);
        }
#endif // ! AND_READONLY

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

            if (mcxt->r_gen.need_rewrite && (!original_need_rewrite))
            {
                VFL_WRN_PRINT((TEXT("[PPNVFL:WRN] failed reading the first copy of the vfl cxt "
                                    "(succeeded reading another copy) (line:%d)\n"), __LINE__));
            }

            if (rps_status == 0)
            {
                return ANDErrorCodeOk;                
            }
            else
            {
                WMR_PRINT(VFLWARN, "problem reading a vfl cxt copy"
                          " rps_status:0x%02X, set:%01d, copy:%01d, page_offset:0x%03X"
                          " type:0x%02X bank:0x%02X idx:0x%02X\n",
                          rps_status, set, redundant_copy, cxt_page_offset, 
                          cxt_spare->spareType, cxt_spare->bank, cxt_spare->idx);
                mcxt->r_gen.need_rewrite = TRUE32;
            }
        }

        if (!(status & PPN_READ_STATUS_CLEAN))
        {
            clean = FALSE32;
        }
    }

    if (clean)
    {
        mcxt->r_gen.need_rewrite = original_need_rewrite;
        return ANDErrorCodeCleanOk;
    }

    VFL_WRN_PRINT((TEXT("[PPNVFL:WRN] failed to read a single vfl cxt copy (UECC) (line:%d)\n"), __LINE__));
    mcxt->r_gen.need_rewrite = TRUE32;
    return ANDErrorCodeUserDataErr;
}

static Int32
_readSingleRedundancyPageOfASet
    (PPNVFL_MainCxt *mcxt,
    UInt16 set,
    UInt16 cxt_page_offset,
    UInt16 redundant_copy,
    Buffer *buf)
{
    PPNStatusType status;
    UInt16 vfl_block_offset = 0, vfl_page_offset = 0, bank = 0xFFFF, block = 0xFFFF;

    _convertVFLPageIdxToPhysicalPageBlockOffsets(mcxt, set, cxt_page_offset,
                                                 &vfl_block_offset, &vfl_page_offset);

    bank = mcxt->gen.vfl_blocks[vfl_block_offset + redundant_copy].bank;
    block = mcxt->gen.vfl_blocks[vfl_block_offset + redundant_copy].block;

    status = ppnMiscSingleOperation(&mcxt->dev, mcxt->single_command, PPN_COMMAND_READ,
                                    PPN_NO_OPTIONS, bank, block, vfl_page_offset, TRUE32, buf->pData, buf->pSpare);

    // check if this is not the first copy or if the copy is marked to be moved
    if ((status & (PPN_READ_STATUS_REFRESH | PPN_READ_STATUS_RETIRE)) &&
        (set == mcxt->r_gen.next_cxt_block_offset))
    {
        VFL_WRN_PRINT((TEXT("[PPNVFL:WRN] _readSingleRedundancyPageOfASet got status:0x%02X from "
                            "redundant_copy:0x%04X set:0x%04X cxt_page_offset:0x%04X (line:%d)\n"),
                       status, redundant_copy, set, cxt_page_offset, __LINE__));
        mcxt->r_gen.need_rewrite = TRUE32;
    }

    // if this is not initial search for cxt we should take the indication
    // for replacement from the device
    if ((status & PPN_READ_STATUS_RETIRE) && (!mcxt->r_gen.search_for_cxt))
    {
        _markBlockForReplacementNoProgram(mcxt, bank, block);
    }

    if (!(status & PPN_READ_STATUS_INVALID_DATA))
    {
        PPNVFL_CxtSpare *cxt_spare = (PPNVFL_CxtSpare*) buf->pSpare;
        if (cxt_spare->spareType == PPNVFL_SPARE_TYPE_CXT)
        {
            return ANDErrorCodeOk;
        }
        else
        {
            VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] _readSingleRedundancyPageOfASet got ok status "
                                "but wrong sparetype 0x%02X (line:%d)\n"),
                           cxt_spare->spareType, __LINE__));
            return ANDErrorCodeUserDataErr;
        }
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
#define PPNVFL_PRINT_FINDVFLCXT_SUMMARY (1)
#define PPNVFL_PRINT_FINDVFLCXT_MORE_INFO (1)

static BOOL32
_findOnNANDCxt
    (PPNVFL_MainCxt * mcxt)
{
    Buffer *buf, *buf2;
    UInt16 bank, page, start_page, mid_page, end_page, block_idx, vfl_cxt_idx, cxt_blocks_idx;
    PPNStatusType status;
    PPNVFL_CxtSpare spare0, spare1;
    BOOL32 found_valid_cxt;

    buf = BUF_Get(BUF_MAIN_AND_SPARE);
    buf2 = BUF_Get(BUF_MAIN_AND_SPARE);

    // find the first written VFLCxt
    found_valid_cxt = FALSE32;
    for (block_idx = 0; ((!found_valid_cxt) && (block_idx < mcxt->dev.blocks_per_cau)); block_idx++)
    {
        if (PPNVFL_CONTEXT_BLOCK_LIMIT(&mcxt->dev) == block_idx)
        {
            // we don't want to panic, we just want to complain loudly for debug purposes
            WMR_PRINT(ERROR, "VFL context not found yet... %lu of %lu sublks checked...\n",
                      block_idx, mcxt->dev.blocks_per_cau);
        }
        for (bank = 0; ((!found_valid_cxt) && (bank < mcxt->dev.num_of_banks)); bank++)
        {
            UInt16 keepout = (ppnMiscGetCAUFromBank(&mcxt->dev, bank) ? 0 :
                              WMR_MAX(mcxt->r_gen.keepout, 1));
            PPNVFL_CxtSpare *cxt_spare = (PPNVFL_CxtSpare*)buf->pSpare;
            status = ppnMiscSingleOperation(&mcxt->dev, mcxt->single_command, PPN_COMMAND_READ, PPN_NO_OPTIONS,
                                            bank, (block_idx + keepout), 0, TRUE32, buf->pData, buf->pSpare);
            if ((!(status & PPN_READ_STATUS_INVALID_DATA)) &&
                cxt_spare->spareType == PPNVFL_SPARE_TYPE_CXT &&
                cxt_spare->idx == PPNVFL_CXT_LAST_BANKIDX)
            {
                PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)buf->pData;
                WMR_MEMCPY(mcxt->gen.vfl_blocks, on_nand->gen.vfl_blocks,
                           (sizeof(BlockAddress) * mcxt->r_gen.num_of_vfl_blocks));
                mcxt->r_gen.cxt_age = cxt_spare->cxt_age;
                found_valid_cxt = TRUE32;
                break;
            }
        }
    }
    if (!found_valid_cxt)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] could not find a single copy of VFLCxt (l:%d)\n"), __LINE__));
        goto error_return;
    }

    // agree on the location of the VFLCxt blocks - find the latest blocks and see that they all point to the same VFL
    // blocks
    found_valid_cxt = FALSE32;
    do
    {
        for (vfl_cxt_idx = 0; vfl_cxt_idx < mcxt->r_gen.num_of_vfl_blocks; vfl_cxt_idx++)
        {
            PPNVFL_CxtSpare *cxt_spare = (PPNVFL_CxtSpare*)buf->pSpare;
            status = ppnMiscSingleOperation(&mcxt->dev, mcxt->single_command, PPN_COMMAND_READ,
                                            PPN_NO_OPTIONS,
                                            mcxt->gen.vfl_blocks[vfl_cxt_idx].bank,
                                            mcxt->gen.vfl_blocks[vfl_cxt_idx].block,
                                            0, TRUE32, buf->pData, buf->pSpare);
            if ((!(status & PPN_READ_STATUS_INVALID_DATA)) &&
                cxt_spare->spareType == PPNVFL_SPARE_TYPE_CXT &&
                cxt_spare->idx == 0xFF &&
                mcxt->r_gen.cxt_age > cxt_spare->cxt_age)
            {
                PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)buf->pData;
                WMR_MEMCPY(mcxt->gen.vfl_blocks, on_nand->gen.vfl_blocks,
                           (sizeof(BlockAddress) * mcxt->r_gen.num_of_vfl_blocks));
                mcxt->r_gen.cxt_age = cxt_spare->cxt_age;
                break;
            }
        }
        if (vfl_cxt_idx == mcxt->r_gen.num_of_vfl_blocks)
        {
            found_valid_cxt = TRUE32;
        }
    }
    while (!found_valid_cxt);

    // use spare0 for the 1st pair and spare1 for the 2nd
    WMR_MEMSET(&spare0, 0, sizeof(PPNVFL_CxtSpare));
    WMR_MEMSET(&spare1, 0, sizeof(PPNVFL_CxtSpare));

    // read the 1st set
    if (_readPageOfASet(mcxt, 0, 0, buf, PPNVFL_CXT_IGNORE_BANKIDX, PPNVFL_CXT_LAST_BANKIDX) == ANDErrorCodeOk)
    {
        WMR_MEMCPY(&spare0, buf->pSpare, sizeof(PPNVFL_CxtSpare));
    }

    // read the 2nd set
    if (_readPageOfASet(mcxt, 1, 0, buf2, PPNVFL_CXT_IGNORE_BANKIDX, PPNVFL_CXT_LAST_BANKIDX) == ANDErrorCodeOk)
    {
        WMR_MEMCPY(&spare1, buf2->pSpare, sizeof(PPNVFL_CxtSpare));
    }

    // make a decision which pair of VFLCxt blcoks is current one and use it
    if (spare0.spareType == PPNVFL_SPARE_TYPE_CXT && spare1.spareType == PPNVFL_SPARE_TYPE_CXT)
    {
        // assume that if we have two valid copies the older one is good
        // as soon as we have a good good set in a new block we should erase the
        // old block
        if ((UInt32)spare0.cxt_age < (UInt32)spare1.cxt_age)
        {
            mcxt->r_gen.next_cxt_block_offset = 0;
            VFL_INF_PRINT(PPNVFL_PRINT_FINDVFLCXT_MORE_INFO,
                          (TEXT(
                               "[PPNVFL:INF] _findOnNANDCxt(0x%04X) AGE0 0x%X, age1 0x%X, type0 0x%X, type1 0x%X (line:%d)\n"),
                           bank, spare0.cxt_age, spare1.cxt_age, spare0.spareType, spare1.spareType, __LINE__));
        }
        else
        {
            mcxt->r_gen.next_cxt_block_offset = 1;
            VFL_INF_PRINT(PPNVFL_PRINT_FINDVFLCXT_MORE_INFO,
                          (TEXT(
                               "[PPNVFL:INF] _findOnNANDCxt(0x%04X) age0 0x%X, AGE1 0x%X, type0 0x%X, type1 0x%X (line:%d)\n"),
                           bank, spare0.cxt_age, spare1.cxt_age, spare0.spareType, spare1.spareType, __LINE__));
        }
    }
    else if (spare0.spareType == PPNVFL_SPARE_TYPE_CXT)
    {
        mcxt->r_gen.next_cxt_block_offset = 0;
    }
    else if (spare1.spareType == PPNVFL_SPARE_TYPE_CXT)
    {
        mcxt->r_gen.next_cxt_block_offset = 1;
    }
    else
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] could not find a single copy of VFLCxt (bank:0x%X) (l:%d)\n"),
                       bank, __LINE__));
        goto error_return;
    }

    // make sure the block list is up to date
    if (mcxt->r_gen.next_cxt_block_offset)
    {
        PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)buf2->pData;
        WMR_MEMCPY(mcxt->gen.vfl_blocks, on_nand->gen.vfl_blocks,
                   (sizeof(BlockAddress) * mcxt->r_gen.num_of_vfl_blocks));
    }
    else
    {
        PPNVFL_OnNANDCxt * on_nand = (PPNVFL_OnNANDCxt*)buf->pData;
        WMR_MEMCPY(mcxt->gen.vfl_blocks, on_nand->gen.vfl_blocks,
                   (sizeof(BlockAddress) * mcxt->r_gen.num_of_vfl_blocks));
    }

    // search for the latest version of the VFLCxt inside the current VFLCxt block pair
    start_page = 0;
    end_page = mcxt->r_gen.vfl_pages_per_set - 1;
    found_valid_cxt = FALSE32;

    while (end_page >= start_page)
    {
        PPNVFL_CxtSpare *spare = (PPNVFL_CxtSpare*)buf->pSpare;
        mid_page = _div2RoundUp(end_page + start_page);
        Int32 read_status = _readPageOfASet(mcxt, mcxt->r_gen.next_cxt_block_offset, mid_page, 
                                            buf, PPNVFL_CXT_IGNORE_BANKIDX, PPNVFL_CXT_IGNORE_BANKIDX);

        if (read_status == ANDErrorCodeOk)
        {
            if (start_page == end_page)
            {
                if (spare->idx == PPNVFL_CXT_LAST_BANKIDX)
                {
                    mcxt->r_gen.cxt_age = spare->cxt_age;
                    mcxt->r_gen.next_cxt_page_offset = start_page + 1;

                    _copyFromOnNandCxtToRam(mcxt, buf->pData);

                    // verify the last page we wrote is valid
                    if (_readSingleRedundancyPageOfASet(mcxt, mcxt->r_gen.next_cxt_block_offset, start_page,
                                                        (mcxt->r_gen.vfl_blocks_redundancy - 1), buf) != ANDErrorCodeOk)
                    {
                        mcxt->r_gen.need_rewrite = TRUE32;
                        VFL_WRN_PRINT((TEXT(
                                           "[PPNVFL:WRN] could not find vfl cxt in the last redundancy copy (line:%d)\n"),
                                       __LINE__));
                    }

                    found_valid_cxt = TRUE32;
                    break;
                }
                else
                {
                    mcxt->r_gen.need_rewrite = TRUE32;
                    VFL_WRN_PRINT((TEXT("[PPNVFL:WRN] could not find cxt in bubble search (line:%d)\n"), __LINE__));
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

        VFL_WRN_PRINT((TEXT("[PPNVFL:WRN] deteteced vfl cxt pages with user data errors (l:%d)\n"), __LINE__));
        end_page = mid_page;
        mcxt->r_gen.need_rewrite = TRUE32;
        break;
    }

    for (cxt_blocks_idx = 0; (cxt_blocks_idx < 2) && (!found_valid_cxt); cxt_blocks_idx++)
    {
        for (page = (mcxt->r_gen.vfl_pages_per_set - 1); (page != 0) && (!found_valid_cxt); page--)
        {
            PPNVFL_CxtSpare *spare = (PPNVFL_CxtSpare*)buf->pSpare;

            Int32 read_status = _readPageOfASet(mcxt, mcxt->r_gen.next_cxt_block_offset, page, buf,
                                                PPNVFL_CXT_IGNORE_BANKIDX, PPNVFL_CXT_LAST_BANKIDX);

            if (read_status == ANDErrorCodeCleanOk)
            {
                continue;
            }

            VFL_WRN_PRINT((TEXT(
                               "[PPNVFL:WRN] Reading cxt next_cxt_block_offset:0x%04X, page:0x%04X read_status:0x%08X (line:%d)\n"),
                           mcxt->r_gen.next_cxt_block_offset, page, read_status, __LINE__));
            VFL_WRN_PRINT((TEXT("[PPNVFL:WRN] spareType:0x%02X, bank:0x%02X, idx:0x%02X cxt_age:0x%08X (line:%d)\n"),
                           spare->spareType, spare->bank, spare->idx, spare->cxt_age, __LINE__));

            if (read_status == ANDErrorCodeOk)
            {
                mcxt->r_gen.cxt_age = spare->cxt_age;
                _copyFromOnNandCxtToRam(mcxt, buf->pData);
                found_valid_cxt = TRUE32;
                break;
            }
        }
        if (!found_valid_cxt)
        {
            // switch to the other cxt set
            mcxt->r_gen.next_cxt_block_offset = (mcxt->r_gen.next_cxt_block_offset == 1 ? 0 : 1);
        }
    }

    if (!found_valid_cxt)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] unable to find vfl cxt copy on the device (line:%d)\n"), __LINE__));
        goto error_return;
    }

    if (mcxt->gen.num_of_ftl_sublk > mcxt->gen.num_of_vfl_sublk)
    {
        VFL_ERR_PRINT((TEXT("FTLSuBlks(=%d) > VFLSuBlks(=%d)\n"), mcxt->gen.num_of_ftl_sublk, mcxt->gen.num_of_vfl_sublk));
        goto error_return;
    }

    VFL_INF_PRINT(PPNVFL_PRINT_FINDVFLCXT_SUMMARY,
                  (TEXT("[PPNVFL:INF] _findOnNANDCxt(0x%04X) block 0x%X, page 0x%X, age 0x%X(line:%d)\n"),
                   bank, mcxt->r_gen.next_cxt_block_offset, mcxt->r_gen.next_cxt_page_offset,
                   mcxt->r_gen.cxt_age, __LINE__));
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
#define PPNVFL_PRINT_READVFLCXT_SUMMARY (0)

static BOOL32
_readVFLCxtPage
    (PPNVFL_MainCxt * mcxt,
    Buffer *buf,
    UInt16 bank,
    UInt16 vfl_idx)
{
    PPNVFL_CxtSpare *spare = (PPNVFL_CxtSpare*)buf->pSpare;
    Int32 read_status;
    const UInt16 set = ((mcxt->vfl_map_pages[bank][vfl_idx] & 0x8000) >> 15);
    const UInt16 cxt_page_offset = (mcxt->vfl_map_pages[bank][vfl_idx] & 0x7FFF);

    VFL_INF_PRINT(PPNVFL_PRINT_READVFLCXT_SUMMARY,
                  (TEXT("[PPNVFL:INF] readVFLCxtPage(0x%04X, 0x%04X) (line:%d)\n"), bank, vfl_idx, __LINE__));

    read_status = _readPageOfASet(mcxt, set, cxt_page_offset, buf, PPNVFL_CXT_IGNORE_BANKIDX, vfl_idx);

    if (read_status != ANDErrorCodeOk)
    {
        VFL_ERR_PRINT((TEXT(
                           "[PPNVFL:ERR] Error reading cxt copies status 0x%X spareType 0x%02X index 0x%02X"
                            " requested index 0x%02X bank 0x%02X requested bank 0x%02X age 0x%08X (line:%d)\n"),
                       read_status, spare->spareType, spare->idx, (UInt8)vfl_idx, spare->bank, (UInt8)bank, 
                       spare->cxt_age, __LINE__));
        return FALSE32;
    }
    return TRUE32;
}

static void
_convertVFLPageIdxToPhysicalPageBlockOffsets
    (PPNVFL_MainCxt * mcxt,
    UInt16 set,
    UInt16 vfl_page_idx,
    UInt16 *vfl_block_offset,
    UInt16 *vfl_page_offset)
{
    const UInt16 vfl_block_depth = (vfl_page_idx / mcxt->dev.pages_per_block_slc);

    *vfl_block_offset = ((set * mcxt->r_gen.vfl_blocks_redundancy * mcxt->r_gen.vfl_blocks_depth) +
                         (vfl_block_depth * mcxt->r_gen.vfl_blocks_redundancy));
    *vfl_page_offset = vfl_page_idx % mcxt->dev.pages_per_block_slc;
}

#ifndef AND_READONLY

/*
 * Name: _programVFLCxtPage
 * Description: programs a single VFL Cxt page (either mapping or VFLCxt struct)
 * Input: buffer, bank, index (what page to read).
 * Return value: TRUE32 is successful, FALSE32 otherwise.
 */
#define PPNVFL_PRINT_PROGRAMVFLCXT_SUMMARY (1)
#define PPNVFL_PRINT_PROGRAMVFLCXT_ADDRESSES (1)

static BOOL32
_programVFLCxtPage
    (PPNVFL_MainCxt * mcxt,
    Buffer *buf,
    UInt16 bank,
    UInt16 vfl_idx)
{
    PPNStatusType status;
    UInt16 redundant_copy, set = mcxt->r_gen.next_cxt_block_offset;
    UInt16 vfl_block_offset, vfl_page_offset;

    VFL_INF_PRINT(PPNVFL_PRINT_PROGRAMVFLCXT_SUMMARY,
                  (TEXT("[PPNVFL:INF] programVFLCxtPage(0x%04X, 0x%04X) page_offset:0x%X set:0x%X age:0x%08X (l:%d)\n"),
                   bank, vfl_idx, mcxt->r_gen.next_cxt_page_offset, set, mcxt->r_gen.cxt_age, __LINE__));

    _setVFLCxtSpare(mcxt, buf->pSpare, bank, vfl_idx);

    _convertVFLPageIdxToPhysicalPageBlockOffsets(mcxt, set, mcxt->r_gen.next_cxt_page_offset,
                                                 &vfl_block_offset, &vfl_page_offset);

    for (redundant_copy = 0; ((redundant_copy < mcxt->r_gen.vfl_blocks_redundancy) &&
                              (!mcxt->r_gen.need_rewrite)); redundant_copy++)
    {
        status = ppnMiscSingleOperation(&mcxt->dev, mcxt->single_command, PPN_COMMAND_PROGRAM,
                                        PPN_NO_OPTIONS,
                                        mcxt->gen.vfl_blocks[vfl_block_offset + redundant_copy].bank,
                                        mcxt->gen.vfl_blocks[vfl_block_offset + redundant_copy].block,
                                        vfl_page_offset, TRUE32, buf->pData, buf->pSpare);
        if (status & (PPN_PROGRAM_STATUS_FAIL | PPN_PROGRAM_STATUS_GEB))
        {
            mcxt->r_gen.need_rewrite = TRUE32;
            VFL_WRN_PRINT((TEXT(
                               "[PPNVFL:WRN] status 0x%X marking vflcxt block for scrubbing(bank:0x%X, block:0x%X) (line:%d)\n"),
                           status,
                           mcxt->gen.vfl_blocks[vfl_block_offset + redundant_copy].bank,
                           mcxt->gen.vfl_blocks[vfl_block_offset + redundant_copy].block,
                           __LINE__));
            _markBlockForReplacementNoProgram(mcxt,
                                              mcxt->gen.vfl_blocks[vfl_block_offset + redundant_copy].bank,
                                              mcxt->gen.vfl_blocks[vfl_block_offset + redundant_copy].block);
        }
    }
    if (mcxt->r_gen.need_rewrite)
    {
        VFL_WRN_PRINT((TEXT(
                           "[PPNVFL:WRN] failed _programVFLCxtPage mcxt->r_gen.need_rewrite is true (line:%d)\n"),
                       __LINE__));
        return FALSE32;
    }
    if (vfl_idx != PPNVFL_CXT_LAST_BANKIDX)
    {
        mcxt->vfl_map_pages[bank][vfl_idx] = (mcxt->r_gen.next_cxt_page_offset |
                                              ((mcxt->r_gen.next_cxt_block_offset & 0x0001) << 15));
    }

    mcxt->r_gen.next_cxt_page_offset++;
    mcxt->r_gen.cxt_age--;

    return TRUE32;
}

static ProgramVFLCxtPageWrapStatus
_programVFLCxtPageWrap
    (PPNVFL_MainCxt * mcxt,
    Buffer *buf,
    UInt16 bank,
    UInt16 vfl_idx)
{
    VFL_INF_PRINT(PPNVFL_PRINT_PROGRAMVFLCXT_SUMMARY,
                  (TEXT("[PPNVFL:INF] programVFLCxtPageWrap(0x%04X, 0x%04X) (line:%d)\n"), bank, vfl_idx, __LINE__));
    if (mcxt->r_gen.next_cxt_page_offset == mcxt->r_gen.vfl_pages_per_set ||
        mcxt->r_gen.need_rewrite)
    {
        VFL_WRN_PRINT((TEXT("[PPNVFL:WRN] Changing PPNVFL Cxt blocks pair ()\n")));
        if (_copyVFLCxtToOtherPair(mcxt) == FALSE32)
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

    return _programVFLCxtPage(mcxt, buf, bank, vfl_idx) ? VFLCxtProgramDone : VFLCxtProgramRetry;
}

#define PPNVFL_PRINT_COPYVFLCXT_SUMMARY (1)

static BOOL32
_checkForVFLBlocksInScrubList
    (PPNVFL_MainCxt * mcxt,
    UInt16 new_set_location,
    BOOL32 *replace_blocks_array,
    BOOL32 *replace_in_new_set)
{
    const UInt16 blocks_per_vfl_set = (mcxt->r_gen.num_of_vfl_blocks >> 1);
    const UInt16 new_set_mapping_offset = new_set_location * blocks_per_vfl_set;

    if (mcxt->gen.scrub_idx)
    {
        UInt16 scrub_idx, idx;
        for (scrub_idx = 0; scrub_idx < mcxt->gen.scrub_idx; scrub_idx++)
        {
            for (idx = 0; idx < blocks_per_vfl_set; idx++)
            {
                if ((mcxt->gen.scrub_list[scrub_idx].block ==
                     mcxt->gen.vfl_blocks[new_set_mapping_offset + idx].block) &&
                    (mcxt->gen.scrub_list[scrub_idx].bank == mcxt->gen.vfl_blocks[new_set_mapping_offset + idx].bank))
                {
                    if (*replace_in_new_set)
                    {
                        VFL_ERR_PRINT((TEXT(
                                           "[PPNVFL:ERR] more than one block to replace in the vfl cxt (0x%04X) (line:%d)\n"),
                                       __LINE__));
                        return FALSE32;
                    }
                    VFL_WRN_PRINT((TEXT(
                                       "[PPNVFL:WRN] vfl cxt block replacement - found a vfl cxt block in the scrub list (bank: 0x%X, block: 0x%X, idx: 0x%X) (line:%d)\n"),
                                   mcxt->gen.vfl_blocks[new_set_mapping_offset + idx].bank,
                                   mcxt->gen.vfl_blocks[new_set_mapping_offset + idx].block, idx, __LINE__));
                    replace_blocks_array[idx] = TRUE32;
                    *replace_in_new_set = TRUE32;
                }
            }
        }
    }
    return TRUE32;
}

// _copyVFLCxtToOtherPair - Use this function to copy the current vfl cxt from the pair it
// resides in to the other pair (either due to program failure or due to lack of space)
static BOOL32
_copyVFLCxtToOtherPair
    (PPNVFL_MainCxt * mcxt)
{
    UInt16 idx, bank;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);
    Buffer *buf_mcxt_copy = BUF_Get(BUF_MAIN_AND_SPARE);
    PPNStatusType status;
    BOOL32 replace_blocks[PPNVFL_MAX_INFO_BLOCK_COUNT >> 1] = { FALSE32 };
    BOOL32 replace_in_new_set = FALSE32;
    const UInt16 blocks_per_vfl_set = (mcxt->r_gen.num_of_vfl_blocks >> 1);
    const UInt16 new_set_location = (mcxt->r_gen.next_cxt_block_offset == 1 ? 0 : 1);
    const UInt16 new_set_mapping_offset = new_set_location * blocks_per_vfl_set;

    VFL_INF_PRINT(PPNVFL_PRINT_COPYVFLCXT_SUMMARY,
                  (TEXT("[PPNVFL:INF] _copyVFLCxtToOtherPair - original cxt set 0x%x, age 0x%X (line:%d)\n"),
                   mcxt->r_gen.next_cxt_block_offset, mcxt->r_gen.cxt_age, __LINE__));

    if (buf == NULL || buf_mcxt_copy == NULL)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failed allocating buf (line:%d)\n"), __LINE__));
        goto return_error;
    }

    // make a copy of the vfl map pages (case things go bad)
    _copyFromRamToOnNandCxt(mcxt, buf_mcxt_copy->pData);

    // check if there are replacement in the new pair
    if (!_checkForVFLBlocksInScrubList(mcxt, new_set_location, replace_blocks, &replace_in_new_set))
    {
        goto return_error;
    }

    mcxt->r_gen.need_rewrite = FALSE32;
    // erase the set of blocks we want to use
    if (!replace_in_new_set)
    {
        for (idx = 0; idx < blocks_per_vfl_set; idx++)
        {
            status = ppnMiscSingleOperation(&mcxt->dev, mcxt->single_command, PPN_COMMAND_ERASE,
                                            PPN_NO_OPTIONS,
                                            mcxt->gen.vfl_blocks[new_set_mapping_offset + idx].bank,
                                            mcxt->gen.vfl_blocks[new_set_mapping_offset + idx].block,
                                            0,
                                            TRUE32, NULL, NULL);

            if (status & PPN_ERASE_STATUS_RETIRE)
            {
                VFL_WRN_PRINT((TEXT(
                                   "[PPNVFL:WRN] failed erase vflcxt block (bank:0x%04X, block: 0x%04X) with retire status (line:%d)\n"),
                               mcxt->gen.vfl_blocks[new_set_mapping_offset + idx].bank,
                               mcxt->gen.vfl_blocks[new_set_mapping_offset + idx].block, __LINE__));
                replace_blocks[idx] = TRUE32;
                replace_in_new_set = TRUE32;
            }
            else if (status & PPN_ERASE_STATUS_FAIL)
            {
                VFL_ERR_PRINT((TEXT(
                                   "[PPNVFL:ERR] failed erase vflcxt block (bank:0x%04X, block: 0x%04X) with no retire status (line:%d)\n"),
                               mcxt->gen.vfl_blocks[new_set_mapping_offset + idx].bank,
                               mcxt->gen.vfl_blocks[new_set_mapping_offset + idx].block, __LINE__));
                goto return_error;
            }
        }
    }
    mcxt->r_gen.next_cxt_block_offset = new_set_location;
    mcxt->r_gen.next_cxt_page_offset = 0;

    if ((!replace_in_new_set) && (!_programCxtLastPage(mcxt, buf, FALSE32)))
    {
        VFL_WRN_PRINT((TEXT(
                           "[PPNVFL:WRN] failed _programCxtLastPage inside _copyVFLCxtToOtherPair() (line:%d)\n"),
                       __LINE__));
        if (!_checkForVFLBlocksInScrubList(mcxt, new_set_location, replace_blocks, &replace_in_new_set))
        {
            goto return_error;
        }
        if (!replace_in_new_set)
        {
            VFL_ERR_PRINT((TEXT(
                               "[PPNVFL:ERR] failed _programVFLCxtPage but no blocks in scrub list (line:%d)\n"),
                           __LINE__));
            goto return_error;
        }
    }

    for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
    {
        for (idx = 0; ((!replace_in_new_set) &&
                       (idx < mcxt->r_gen.pages_per_block_mapping)); idx++)
        {
            if (!_readVFLCxtPage(mcxt, buf, bank, idx))
            {
                VFL_ERR_PRINT((TEXT(
                                   "[PPNVFL:ERR] failed _readVFLCxtPage inside _copyVFLCxtToOtherPair () (line:%d)\n"),
                               __LINE__));
                goto return_error;
            }
            if (!_programVFLCxtPage(mcxt, buf, bank, idx))
            {
                VFL_WRN_PRINT((TEXT(
                                   "[PPNVFL:WRN] failed _programVFLCxtPage inside _copyVFLCxtToOtherPair () (line:%d)\n"),
                               __LINE__));
                if (!_checkForVFLBlocksInScrubList(mcxt, new_set_location, replace_blocks, &replace_in_new_set))
                {
                    goto return_error;
                }
                if (!replace_in_new_set)
                {
                    VFL_ERR_PRINT((TEXT(
                                       "[PPNVFL:ERR] failed _programVFLCxtPage but no blocks in scrub list (line:%d)\n"),
                                   __LINE__));
                    goto return_error;
                }
            }
        }
    }
    if ((!replace_in_new_set) && (!_programCxtLastPage(mcxt, buf, FALSE32)))
    {
        VFL_WRN_PRINT((TEXT(
                           "[PPNVFL:WRN] failed _programCxtLastPage inside _copyVFLCxtToOtherPair() (line:%d)\n"),
                       __LINE__));
        if (!_checkForVFLBlocksInScrubList(mcxt, new_set_location, replace_blocks, &replace_in_new_set))
        {
            goto return_error;
        }
        if (!replace_in_new_set)
        {
            VFL_ERR_PRINT((TEXT(
                               "[PPNVFL:ERR] failed _programVFLCxtPage but no blocks in scrub list (line:%d)\n"),
                           __LINE__));
            goto return_error;
        }
    }

    if (replace_in_new_set)
    {
        for (idx = 0; idx < blocks_per_vfl_set; idx++)
        {
            if (replace_blocks[idx])
            {
                // revert the map (case the error happen while programming the new set and mapping is off)
                _copyMapPagesFromOnNandCxtToRam(mcxt, buf_mcxt_copy->pData);

                BUF_Release(buf);
                buf = NULL;
                BUF_Release(buf_mcxt_copy);
                buf_mcxt_copy = NULL;
                // handle errors in working with the new pair
                if (!_replaceErasedCxtBlock(mcxt, (new_set_mapping_offset + idx)))
                {
                    VFL_ERR_PRINT((TEXT(
                                       "[PPNVFL:ERR] need to replace flag is set but nothing is marked for replacement (idx:0x%X) (line:%d)\n"),
                                   idx, __LINE__));
                    goto return_error;
                }
                break;
            }
        }
        if (idx == blocks_per_vfl_set)
        {
            VFL_ERR_PRINT((TEXT(
                               "[PPNVFL:ERR] need to replace flag is set but nothing is marked for replacement (idx:0x%X) (line:%d)\n"),
                           idx, __LINE__));
            goto return_error;
        }
    }

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
    (PPNVFL_MainCxt * mcxt,
    Buffer * buf,
    BOOL32 retry)
{
    ProgramVFLCxtPageWrapStatus program_status;

    do
    {
        _copyFromRamToOnNandCxt(mcxt, buf->pData);
        program_status = _programVFLCxtPageWrap(mcxt, buf, PPNVFL_CXT_LAST_BANKIDX, PPNVFL_CXT_LAST_BANKIDX);
    }
    while ((program_status == VFLCxtProgramRetry) && retry);

    return (program_status != VFLCxtProgramDone) ? FALSE32 : TRUE32;
}

/*
 * Name: _ppnvflChangeFTLCxtVbn
 * Description: implements ChangeFTLCxt for the vfl I/F
 * Input: new ftl cxt array
 * Return value: ANDErrorCodeOk if successful
 */
#define PPNVFL_PRINT_SET_NUM_OF_FTL_SUBLKS_SUMMARY (1)
static BOOL32
_setNumOfFTLSuBlks
    (PPNVFL_MainCxt * mcxt,
    UInt16 num_of_ftl_sublk)
{
    BOOL32 res;
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);

    if (num_of_ftl_sublk > mcxt->gen.num_of_vfl_sublk)
    {
        WMR_PANIC("invalid burnin-size settings: %d FTL, %d VFL sublks", num_of_ftl_sublk, mcxt->gen.num_of_vfl_sublk);
    }

    VFL_INF_PRINT(PPNVFL_PRINT_SET_NUM_OF_FTL_SUBLKS_SUMMARY,
                  (TEXT("[PPNVFL:INF]  _setNumOfFTLSuBlks (0x%04X)\n"), num_of_ftl_sublk));
    mcxt->gen.num_of_ftl_sublk = num_of_ftl_sublk;

    res = _programCxtLastPage(mcxt, buf, TRUE32);
    BUF_Release(buf);

    return res;
}

/* vfl cxt management functions - end */

#endif // ! AND_READONLY

/* vfl I/F implementation - start */
/*
 * Name: _ppnvflInit
 * Description: this function initialize the memory and structures for ppnvfl
 * Input: fpart function table
 * Return value: ANDErrorCodeOk if successfull
 */
static Int32
_ppnvflInit
    (PPNVFL_MainCxt * mcxt,
    FPartFunctions * pFPartFunctions)
{
    UInt32 i;
    UInt16 pages_per_vfl_cxt_copy; // including the cxt and mapping

    if (mcxt->initialized)
    {
        return TRUE32;
    }

    WMR_ASSERT(sizeof(PPNVFL_CxtSpare) == 16);

    mcxt->fpart = pFPartFunctions;
    mcxt->fil = mcxt->fpart->GetLowFuncTbl();

    ppnMiscFillDevStruct(&mcxt->dev, mcxt->fil);
    WMR_ASSERT(mcxt->dev.lbas_per_page == 1);
    mcxt->single_command = (PPNCommandStruct*)mcxt->fpart->GetSingleCommandStruct();

    // allocating the memory for the CAUs structures
    // this code may later on move to open
    if (mcxt->dev.num_channels == 0 || mcxt->dev.ces_per_channel == 0 || mcxt->dev.caus_per_ce == 0)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] _ppnvflInit num_channels 0x%X ces_per_channel 0x%X caus_per_ce 0x%X\n"),
                       mcxt->dev.num_channels, mcxt->dev.ces_per_channel, mcxt->dev.caus_per_ce));
    }

    mcxt->r_gen.pages_per_block_mapping = 1;
    i = mcxt->dev.blocks_per_cau * sizeof(P2BlockUsageType);
    while (i > mcxt->dev.main_bytes_per_page)
    {
        i -= mcxt->dev.main_bytes_per_page;
        mcxt->r_gen.pages_per_block_mapping++;
    }
    mcxt->r_gen.pbns_per_cxt_mapping_page = mcxt->dev.main_bytes_per_page / sizeof(P2BlockUsageType);

    // calculate vfl cxt on nand - depth, redundancy and pages per set
    mcxt->r_gen.vfl_blocks_redundancy = 4;
    // make sure that we have at least twice as many entries as needed for a single copy of vflcxt and
    // all the mapping
    pages_per_vfl_cxt_copy = 1 + (mcxt->r_gen.pages_per_block_mapping * mcxt->dev.num_of_banks);
    mcxt->r_gen.vfl_blocks_depth = 1;
    while ((UInt16)(mcxt->r_gen.vfl_blocks_depth * (UInt16)mcxt->dev.pages_per_block_slc) <
           (UInt32)(pages_per_vfl_cxt_copy * PPNVFL_NUM_OF_VFL_CXT_BLOCK_SETS))
    {
        mcxt->r_gen.vfl_blocks_depth++;
    }
    mcxt->r_gen.vfl_pages_per_set = (mcxt->r_gen.vfl_blocks_depth * mcxt->dev.pages_per_block_slc);
    mcxt->r_gen.num_of_vfl_blocks = (mcxt->r_gen.vfl_blocks_depth * mcxt->r_gen.vfl_blocks_redundancy *
                                     PPNVFL_NUM_OF_VFL_CXT_BLOCK_SETS);

    mcxt->gen.num_of_vfl_sublk = _calcNumOfVFLSuBlk(mcxt);
    mcxt->gen.num_of_ftl_sublk = mcxt->gen.num_of_vfl_sublk;
    mcxt->r_gen.cxt_age = 0xFFFFFFFF;
    mcxt->vfl_map_pages = (UInt16**)WMR_MALLOC(sizeof(UInt16*) * mcxt->dev.num_of_banks);
    mcxt->vfl_map_pages[0] = (UInt16*)WMR_MALLOC(sizeof(UInt16) * mcxt->dev.num_of_banks *
                                                 mcxt->r_gen.pages_per_block_mapping);
    for (i = 1; i < mcxt->dev.num_of_banks; i++)
    {
        mcxt->vfl_map_pages[i] = &mcxt->vfl_map_pages[0][mcxt->r_gen.pages_per_block_mapping * i];
    }
    mcxt->r_banks = (PPNVFL_BankRAMCxt*)WMR_MALLOC(sizeof(PPNVFL_BankRAMCxt) * mcxt->dev.num_of_banks);
    mcxt->v2p_main = (UInt16*)WMR_MALLOC(sizeof(UInt16) * mcxt->gen.num_of_vfl_sublk);
    mcxt->mixed_entries = 0;
    mcxt->v2p_mixed = (MixedEntryType*)WMR_MALLOC(_getV2PMixedArraySize(mcxt));

    WMR_MEMSET(mcxt->v2p_main, 0xFF, (sizeof(UInt16) * mcxt->gen.num_of_vfl_sublk));
    WMR_MEMSET(mcxt->v2p_mixed, 0xFF, _getV2PMixedArraySize(mcxt));

    // allocate memory for the commands
    WMR_ASSERT(mcxt->dev.num_of_banks <= PPNVFL_MAX_SUPPORTED_BANKS);
    WMR_BufZone_Init(&mcxt->bufzone);
    for (i = 0; i < mcxt->dev.num_channels; i++)
    {
        mcxt->commands[i] = (PPNCommandStruct *)WMR_Buf_Alloc_ForDMA(&mcxt->bufzone, sizeof(PPNCommandStruct));
    }
    WMR_BufZone_FinishedAllocs(&mcxt->bufzone);

    for (i = 0; i < mcxt->dev.num_channels; i++)
    {
        WMR_BufZone_Rebase(&mcxt->bufzone, (void **)&mcxt->commands[i]);
        WMR_ASSERT(mcxt->commands[i] != NULL);
    }
    WMR_BufZone_FinishedRebases(&mcxt->bufzone);

    if (mcxt->dev.ces_per_channel > 1)
    {
        mcxt->reorder = (PPNReorderStruct*)WMR_MALLOC(sizeof(PPNReorderStruct));
    }
    else
    {
        mcxt->reorder = NULL;
    }

#ifdef AND_COLLECT_STATISTICS
    WMR_MEMSET(&mcxt->stat, 0, sizeof(PPNVFLStatistics));
#endif

    mcxt->initialized = TRUE32;
    return ANDErrorCodeOk;
}

/*
 * Name: _ppnvflOpen
 * Description: reads the vfl structures from nand and build the ram version
 * Input:
 * Return value: ANDErrorCodeOk if successful.
 */
#define PPNVFL_PRINT_RAM_MAPPING_BUILDUP (0)
#define PPNVFL_PRINT_OPEN_V2PMAPPING (0)

static Int32
_ppnvflOpen
    (PPNVFL_MainCxt * mcxt,
    UInt32 dwKeepout,
    UInt32 minor_ver,
    UInt32 dwOptions)
{
    Buffer * buf = NULL;
    UInt16 bank, j;
    UInt8  *validate_mapping = NULL;
    SpecialBlockAddress *special_blocks = NULL;
    UInt16 *special_blocks_types = NULL;
    UInt16 total_special_blocks = 0;
    Int32 ret_val;
    
    mcxt->format_options = dwOptions;

    buf = BUF_Get(BUF_MAIN_AND_SPARE);
    // populate the General and CAU structures

    special_blocks = (SpecialBlockAddress *)WMR_MALLOC(AND_MAX_SPECIAL_BLOCKS *
                                                       sizeof(SpecialBlockAddress));
    special_blocks_types = (UInt16*)WMR_MALLOC(AND_MAX_SPECIAL_BLOCKS * sizeof(UInt16));

    // clear the buffer
    WMR_MEMSET(mcxt->v2p_main, 0xFF, (sizeof(UInt16) * mcxt->gen.num_of_vfl_sublk));
    WMR_MEMSET(mcxt->v2p_mixed, 0xFF, _getV2PMixedArraySize(mcxt));

    mcxt->r_gen.cxt_age = 0xFFFFFFFF;
    mcxt->r_gen.keepout = dwKeepout;
    mcxt->mixed_entries = 0;

    WMR_MEMSET(mcxt->vfl_map_pages[0], 0, (mcxt->dev.num_of_banks *
                                           mcxt->r_gen.pages_per_block_mapping * sizeof(UInt16)));
    WMR_MEMSET(mcxt->r_banks, 0, (sizeof(PPNVFL_BankRAMCxt) * mcxt->dev.num_of_banks));

    validate_mapping = (UInt8*)WMR_MALLOC(mcxt->dev.blocks_per_cau * sizeof(UInt8));

    mcxt->r_gen.search_for_cxt = TRUE32;
    if (_findOnNANDCxt(mcxt) == FALSE32)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:WRN] Error _findOnNANDCxt() (line:%d)\n"), __LINE__));
        BUF_Release(buf);
        return ANDErrorCodeHwErr;
    }
    mcxt->r_gen.search_for_cxt = FALSE32;

    WMR_MEMSET(validate_mapping, 0, (mcxt->dev.blocks_per_cau * sizeof(UInt8)));
    for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
    {
        // read the mapping and fill the v2p maps
        for (j = 0; j < mcxt->r_gen.pages_per_block_mapping; j++)
        {
            UInt16 block, startOffset, count;
            P2BlockUsageType *p2_block_usage_map = (P2BlockUsageType*)buf->pData;
            if (_readVFLCxtPage(mcxt, buf, bank, j) == FALSE32)
            {
                VFL_ERR_PRINT((TEXT("[PPNVFL:WRN] Error _readVFLCxtPage(bank=%d, %d) (line:%d)\n"), bank, j, __LINE__));
                BUF_Release(buf);
                return ANDErrorCodeHwErr;
            }
            startOffset = j * (mcxt->r_gen.pbns_per_cxt_mapping_page);
            count = WMR_MIN((mcxt->dev.blocks_per_cau - startOffset),
                            mcxt->r_gen.pbns_per_cxt_mapping_page);
            for (block = 0; block < count; block++)
            {
                UInt16 pbn = block + startOffset;
                
                if (!(p2_block_usage_map[block].p2sb.type & AND_SB_BIT_SPEICAL_FLAG))
                {
                    UInt16 vbn = p2_block_usage_map[block].p2v.vbn;
                    UInt8 offset = p2_block_usage_map[block].p2v.offset;
                    mcxt->r_banks[bank].num_p2v++;
                    if (vbn >= mcxt->gen.num_of_vfl_sublk)
                    {
                        WMR_PRINT(ERROR, "vbn %d of %d\n", vbn, mcxt->gen.num_of_vfl_sublk);
                        ret_val = ANDErrorCodeHwErr;
                        goto exit;
                    }
                    validate_mapping[vbn]++;

                    if (mcxt->v2p_main[vbn] == 0xFFFF)
                    {
                        if ((offset == bank) && (bank == 0))
                        {
                            VFL_INF_PRINT(PPNVFL_PRINT_RAM_MAPPING_BUILDUP,
                                          (TEXT("[PPNVFL:INF] main: bank 0x%X vbn 0x%X pbn 0x%X (line:%d)\n"),
                                           bank, vbn, pbn, __LINE__));
                            mcxt->v2p_main[vbn] = pbn;
                        }
                        else
                        {
                            mcxt->v2p_main[vbn] = PPN_V2P_UNMAPPED;
                            _addNewMixedBlockEntry(mcxt, vbn, offset, bank, pbn);
                        }
                    }
                    else if ((mcxt->v2p_main[vbn] != pbn) || (offset != bank))
                    {
                        // this is a mixed entry
                        if (_isMixedBlock(mcxt, vbn) == TRUE32)
                        {
                            VFL_INF_PRINT(PPNVFL_PRINT_RAM_MAPPING_BUILDUP,
                                          (TEXT("[PPNVFL:INF] set: vbn 0x%X offset 0x%X bank 0x%X pbn 0x%X (line:%d)\n"),
                                           vbn, offset, bank, pbn, __LINE__));
                            // this is an existing mixed entry use the allocated slot
                            _setMixedBlockEntry(mcxt, vbn, offset, bank, pbn);
                        }
                        else
                        {
                            VFL_INF_PRINT(PPNVFL_PRINT_RAM_MAPPING_BUILDUP,
                                          (TEXT(
                                               "[PPNVFL:INF] add[0x%04X]: vbn 0x%04X offset 0x%02X bank 0x%02X pbn 0x%04X (line:%d)\n"),
                                           mcxt->mixed_entries, vbn, offset, bank, pbn, __LINE__));
                            // this is a new entry
                            _addNewMixedBlockEntry(mcxt, vbn, offset, bank, pbn);
                        }
                    }
                }
                // count bad blocks (initial and grown and special blocks)
                else
                {
                    switch (p2_block_usage_map[block].p2sb.type)
                    {
                        case PPNVFL_TYPE_AVAILABLE_MARK:
                            mcxt->r_banks[bank].num_available++;
                            break;

                        case PPNVFL_TYPE_GROWN_BB:
                            mcxt->r_banks[bank].num_grown_bad++;
                            break;

                        case PPNVFL_TYPE_INITIAL_BB:
                            mcxt->r_banks[bank].num_initial_bad++;
                            break;

                        case PPNVFL_TYPE_VFL_INFO:
                            mcxt->r_banks[bank].num_cxt++;
                            break;

                        default:
                        {
                            if (andSBGetHandler(p2_block_usage_map[block].p2sb.type) != AND_SB_HANDLER_VFL_CXT)
                            {
                                if (AND_MAX_SPECIAL_BLOCKS <= total_special_blocks)
                                {
                                    VFL_ERR_PRINT((TEXT("total special blocks has exceeded the max allowed\n")));
                                    ret_val = ANDErrorCodeHwErr;
                                    goto exit;
                                }
                                mcxt->r_banks[bank].num_special_blocks++;
                                special_blocks[total_special_blocks].bank = bank;
                                special_blocks[total_special_blocks].block = pbn;
                                special_blocks_types[total_special_blocks] = p2_block_usage_map[block].p2sb.type;
                                total_special_blocks++;
                                break;
                            }
                            else
                            {
                                VFL_ERR_PRINT((TEXT("unknown block marking: 0x%08x\n"),
                                               p2_block_usage_map[block].p2sb.type));
                                ret_val = ANDErrorCodeHwErr;
                                goto exit;
                            }
                        }
                    }
                }
            }
        }
    }

    for (j = 0; j < mcxt->gen.num_of_vfl_sublk; j++)
    {
        if (validate_mapping[j] != mcxt->dev.num_of_banks)
        {
            VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] validate_mapping[%d] = %d (l:%d)\n"), j, validate_mapping[j],
                           __LINE__));
            ret_val = ANDErrorCodeHwErr;
            goto exit;
        }
    }

    mcxt->r_gen.cxt_age--;

    if (total_special_blocks != 0)
    {
        UInt16 sb_idx;

        mcxt->fpart->ChangeCacheState(FALSE32);
        // send the list to FPart
        for (sb_idx = 0; sb_idx < total_special_blocks; sb_idx++)
        {
            if (!mcxt->fpart->AllocateSpecialBlockType(&special_blocks[sb_idx], 1, special_blocks_types[sb_idx]))
            {
                VFL_ERR_PRINT((TEXT("fpart failed to allocate special block with type %p\n"),
                               special_blocks_types[sb_idx]));
                ret_val = ANDErrorCodeHwErr;
                goto exit;
            }
        }
        mcxt->fpart->ChangeCacheState(TRUE32);
    }

    ret_val = ANDErrorCodeOk;

    _printVFLBlockMapping(mcxt, __LINE__);
#ifndef AND_READONLY
    if (mcxt->r_gen.need_rewrite)
    {
        if (!_copyVFLCxtToOtherPair(mcxt))
        {
            VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failed _copyVFLCxtToOtherPair after need_rewrite = TRUE (line:%d)\n"),
                           __LINE__));
        }
        _printVFLBlockMapping(mcxt, __LINE__);
    }
#endif
 exit:
    if (NULL != buf)
    {
        BUF_Release(buf);
    }
    if (NULL != validate_mapping)
    {
        WMR_FREE(validate_mapping, (mcxt->dev.blocks_per_cau * sizeof(UInt8)));
    }
    if (NULL != special_blocks)
    {
        WMR_FREE(special_blocks, (AND_MAX_SPECIAL_BLOCKS * sizeof(SpecialBlockAddress)));
    }
    if (NULL != special_blocks_types)
    {
        WMR_FREE(special_blocks_types, (AND_MAX_SPECIAL_BLOCKS * sizeof(UInt16)));
    }

#if (defined(PPNVFL_PRINT_OPEN_V2PMAPPING) && PPNVFL_PRINT_OPEN_V2PMAPPING)
    _printV2PMapping(mcxt);
#endif

    return ret_val;
}

#ifndef AND_READONLY
/*
 * Name: _ppnvflFormat
 * Description: implements format function for the vfl I/F
 * Input:
 * Return value: ANDErrorCodeOk if successful.
 */
static Int32
_ppnvflFormat
    (PPNVFL_MainCxt * mcxt,
    UInt32 dwKeepout,
    UInt32 dwOptions)
{
    UInt16 bank, pbn, vbn, i, pre_mapped_special_blocks, vfl_info_idx;
    Buffer *buf = NULL;
    P2BlockUsageType **on_nand_block_map = NULL;
    PPNStatusType status;
    UInt16 perfect_match_bar;
    BOOL32 assign_vbns_out_of_banks = FALSE32;

    mcxt->format_options = dwOptions;
    
    // Init the general structure
    mcxt->r_gen.cxt_age = 0xFFFFFFFF;
    mcxt->r_gen.keepout = dwKeepout;
    mcxt->gen.ftl_type = 0xFFFFFFFF;

    // before we start - make sure the PPN configuration here make sense to this vfl
    if (mcxt->dev.ces_per_channel > PPN_MAX_CES_PER_BUS)
    {
        VFL_ERR_PRINT((TEXT(
                           "[PPNVFL:ERR] PPNVFL does not support configurations with more than %d CEs/Channel (found %d) (line:%d)!\n"),
                       PPN_MAX_CES_PER_BUS, mcxt->dev.ces_per_channel, __LINE__));
        goto return_error;
    }
    if (mcxt->dev.main_bytes_per_page < PPNVFL_MIN_PAGE_SIZE)
    {
        VFL_ERR_PRINT((TEXT(
                           "[PPNVFL:ERR] PPNVFL does not support configurations with less than %d bytes per page (found %d) (line:%d)!\n"),
                       PPNVFL_MIN_PAGE_SIZE, mcxt->dev.main_bytes_per_page, __LINE__));
        goto return_error;
    }
    if (mcxt->dev.num_of_banks > PPNVFL_MAX_SUPPORTED_BANKS)
    {
        VFL_ERR_PRINT((TEXT(
                           "[PPNVFL:ERR] PPNVFL does not support configurations with more than %d banks (found %d) (line:%d)!\n"),
                       PPNVFL_MAX_SUPPORTED_BANKS, mcxt->dev.num_of_banks, __LINE__));
        goto return_error;
    }

    for (i = 0; i < FTL_CXT_SECTION_SIZE; i++)
    {
        mcxt->gen.ftl_cxt_vbn[i] = i;
    }

    buf = BUF_Get(BUF_MAIN_AND_SPARE);
    if (buf == NULL)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR]  BUF_Get fail (line:%d)!\n"), __LINE__));
        return ANDErrorCodeHwErr;
    }

    // start - 1. allocate memory to hold phisycal to virtual mapping
    on_nand_block_map = (P2BlockUsageType**)WMR_MALLOC(sizeof(P2BlockUsageType*) * mcxt->dev.num_of_banks);
    if (on_nand_block_map == NULL)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR]  allocation fail (line:%d)!\n"), __LINE__));
        goto return_error;
    }
    on_nand_block_map[0] = (P2BlockUsageType*)WMR_MALLOC(mcxt->dev.blocks_per_cau * mcxt->dev.num_of_banks *
                                                         sizeof(P2BlockUsageType));
    if (on_nand_block_map[0] == NULL)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR]  allocation fail (line:%d)!\n"), __LINE__));
        goto return_error;
    }
    // assign the rest of the pointers
    for (bank = 1; bank < mcxt->dev.num_of_banks; bank++)
    {
        on_nand_block_map[bank] = &(on_nand_block_map[0][(bank * mcxt->dev.blocks_per_cau)]);
    }
    // end   - 1. allocate memory to hold phisycal to virtual mapping - end

    // start - 2. read the bbts
    for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
    {
        status = ppnMiscSingleOperation(&mcxt->dev, mcxt->single_command, PPN_COMMAND_CAUBBT,
                                        PPN_NO_OPTIONS, bank, 0, 0, TRUE32, buf->pData, NULL);
        if (status & PPN_OTHERS_STATUS_OP_FAILED)
        {
            VFL_ERR_PRINT((TEXT("[PPNVFL:ERR]  ReadCAUBBT fail (line:%d)!\n"), __LINE__));
            goto return_error;
        }

        for (pbn = 0; pbn < mcxt->dev.blocks_per_cau; pbn++)
        {
            if (ppnMiscIsBlockGood(buf->pData, pbn) == FALSE32)
            {
                on_nand_block_map[bank][pbn].p2sb.type = PPNVFL_TYPE_INITIAL_BB;
                on_nand_block_map[bank][pbn].p2sb.reserved = P2SB_RESERVED_VALUE;
            }
            else
            {
                on_nand_block_map[bank][pbn].p2sb.type = PPNVFL_TYPE_AVAILABLE_MARK;
                on_nand_block_map[bank][pbn].p2sb.reserved = P2SB_RESERVED_VALUE;
            }
        }
    }
    // end   - 2. read the bbts

    // start - 3. mark special blocks
    // start - 3.1 mark pre allocated special blocks
    if (mcxt->fpart->MapSpecialBlocks(NULL, NULL, &pre_mapped_special_blocks) == TRUE32)
    {
        UInt16 *special_blocks_types;
        SpecialBlockAddress *special_blocks;

        special_blocks_types = (UInt16*)WMR_MALLOC(pre_mapped_special_blocks * sizeof(UInt16));
        special_blocks = (SpecialBlockAddress*)WMR_MALLOC(pre_mapped_special_blocks *
                                                          sizeof(SpecialBlockAddress));

        if (mcxt->fpart->MapSpecialBlocks(special_blocks, special_blocks_types,
                                         &pre_mapped_special_blocks) == FALSE32)
        {
            VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] MapSpecialBlocks fail after success??? (line:%d)\n"),
                           __LINE__));
            goto return_error;
        }
        for (i = 0; i < pre_mapped_special_blocks; i++)
        {
            bank = special_blocks[i].bank;
            pbn = special_blocks[i].block;
            on_nand_block_map[bank][pbn].p2sb.type = special_blocks_types[i];
            on_nand_block_map[bank][pbn].p2sb.reserved = P2SB_RESERVED_VALUE;
        }
    }
    // end   - 3.1 mark pre allocated special blocks

    // start - 3.2 mark block zero
    // start - 3.3 mark nand boot blocks
    for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
    {
        if (ppnMiscGetCAUFromBank(&mcxt->dev, bank) == 0)
        {
            on_nand_block_map[bank][0].p2sb.type = AND_SB_TYPE_BLOCK_ZERO;
            // support legacy nand-boot area
            for (pbn = 0; pbn < mcxt->r_gen.keepout; pbn++)
            {
                if (on_nand_block_map[bank][pbn].p2sb.type == PPNVFL_TYPE_AVAILABLE_MARK)
                {
                    on_nand_block_map[bank][pbn].p2sb.type = AND_SB_TYPE_KEEPOUT_BOOT;
                }
            }
        }
    }

    // end   - 3.2 mark block zero
    // end   - 3.3 mark nand boot blocks

    // start - 3.4 mark vfl context blocks
    pbn = 0;
    for (vfl_info_idx = 0; ((vfl_info_idx < mcxt->r_gen.num_of_vfl_blocks) &&
                            (pbn < mcxt->dev.blocks_per_cau)); pbn++)
    {
        if (PPNVFL_CONTEXT_BLOCK_LIMIT(&mcxt->dev) == pbn)
        {
            // we don't want to panic, we just want to complain loudly for debug purposes
            WMR_PRINT(ERROR, "VFL context allocation not complete yet..."
                              "%lu of %lu sublks checked..."
                              "%lu of %lu vflblks allocated...\n",
                      pbn, mcxt->dev.blocks_per_cau, vfl_info_idx, mcxt->r_gen.num_of_vfl_blocks);
        }

        for (bank = 0; ((bank < mcxt->dev.num_of_banks) &&
                        (vfl_info_idx < mcxt->r_gen.num_of_vfl_blocks)); bank++)
        {
            if (on_nand_block_map[bank][pbn].p2sb.type == PPNVFL_TYPE_AVAILABLE_MARK)
            {
                if (ppnMiscTestSpecialBlock(&mcxt->dev, mcxt->commands[0], bank, pbn, PPNVFL_SPARE_TYPE_TST))
                {
                    on_nand_block_map[bank][pbn].p2sb.type = PPNVFL_TYPE_VFL_INFO;
                    mcxt->gen.vfl_blocks[vfl_info_idx].bank = bank;
                    mcxt->gen.vfl_blocks[vfl_info_idx].block = pbn;
                    vfl_info_idx++;
                }
                else
                {
                    on_nand_block_map[bank][pbn].p2sb.type = PPNVFL_TYPE_GROWN_BB;
                }
            }
        }
    }

    if (vfl_info_idx < mcxt->r_gen.num_of_vfl_blocks)
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failed allocating vfl info blocks (line:%d)\n"),
                       __LINE__));
        goto return_error;
    }

    // end   - 3.4 mark vfl context blocks
    // start - 3.5 generate a bar for the perfect striping (make sure we put special blocks on top)
    perfect_match_bar = mcxt->dev.blocks_per_cau - 1;
    for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
    {
        UInt16 sb_allocation_count;
        pbn = mcxt->dev.blocks_per_cau;
        for (sb_allocation_count = 0;
             (sb_allocation_count < SPECIAL_BLOCK_PRE_ALLOCATION_STRIPE) && (pbn >= PPN_SPECIAL_BLOCK_LIMIT(&mcxt->dev));
             sb_allocation_count++)
        {
            for (pbn--; pbn >= PPN_SPECIAL_BLOCK_LIMIT(&mcxt->dev); pbn--)
            {
                if (on_nand_block_map[bank][pbn].p2sb.type == PPNVFL_TYPE_AVAILABLE_MARK)
                {
                    break;
                }
            }
        }
        if (pbn < PPN_SPECIAL_BLOCK_LIMIT(&mcxt->dev))
        {
            VFL_ERR_PRINT((TEXT(
                               "[PPNVFL:WRN] _ppnvflFormat - unable to reserve special block stripe - 0x%X (line:%d)\n"),
                           bank, __LINE__));
            goto return_error;
        }
        if (pbn < perfect_match_bar)
        {
            perfect_match_bar = pbn;
        }
    }
    // end   - 3.5 generate a bar for the perfect striping (make sure we put special blocks on top)

    // end   - 3. mark special blocks

    // start - 4. allocate virtual blocks - start
    // start - 4.1. pick the best aligned blocks first
    WMR_MEMSET(mcxt->v2p_main, 0xFF, (sizeof(UInt16) * mcxt->gen.num_of_vfl_sublk));
    WMR_MEMSET(mcxt->v2p_mixed, 0xFF, _getV2PMixedArraySize(mcxt));
    mcxt->mixed_entries = 0;
    pbn = 0;
    for (vbn = 0; (vbn < mcxt->gen.num_of_vfl_sublk) && (pbn < perfect_match_bar); )
    {
        for (; pbn < perfect_match_bar; pbn++)
        {
            BOOL32 all_are_available = TRUE32;
            for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
            {
                if (on_nand_block_map[bank][pbn].p2sb.type != PPNVFL_TYPE_AVAILABLE_MARK)
                {
                    all_are_available = FALSE32;
                    break;
                }
            }
            if (all_are_available == TRUE32)
            {
                for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
                {
                    on_nand_block_map[bank][pbn].p2v.vbn = vbn;
                    on_nand_block_map[bank][pbn].p2v.offset = bank;
                    on_nand_block_map[bank][pbn].p2v.reserved = P2V_RESERVED_VALUE;
                }
                mcxt->v2p_main[vbn] = pbn;
                vbn++;
                break;
            }
        }
    }
    // end - 4.1. pick the best aligned blocks first

    // start - 4.2 pick any allowable combination for the rest of the virtual blocks

    // choose replacements from the same bank (until we run out)
    assign_vbns_out_of_banks = FALSE32;
    if (vbn < mcxt->gen.num_of_vfl_sublk)
    {
        UInt16 vbn_idx = vbn;
        for (vbn_idx = vbn; vbn_idx < mcxt->gen.num_of_vfl_sublk; vbn_idx++)
        {
            mcxt->v2p_main[vbn_idx] = PPN_V2P_UNMAPPED;
            if (!_addNewMixedBlockEntry(mcxt, vbn_idx, 0, 0, PPN_V2P_UNMAPPED))
            {
                VFL_ERR_PRINT((TEXT(
                                   "[PPNVFL:WRN] _ppnvflFormat - _addNewMixedBlockEntry fail (line:%d)\n"),
                               __LINE__));
                goto return_error;
            }
        }

        for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
        {
            for (vbn_idx = vbn, pbn = 0; (pbn < mcxt->dev.blocks_per_cau) && (vbn_idx < mcxt->gen.num_of_vfl_sublk);
                 pbn++)
            {
                if (on_nand_block_map[bank][pbn].p2sb.type == PPNVFL_TYPE_AVAILABLE_MARK)
                {
                    on_nand_block_map[bank][pbn].p2v.vbn = vbn_idx;
                    on_nand_block_map[bank][pbn].p2v.offset = bank;
                    on_nand_block_map[bank][pbn].p2v.reserved = P2V_RESERVED_VALUE;
                    if (!_setMixedBlockEntry(mcxt, vbn_idx, bank, bank, pbn))
                    {
                        VFL_ERR_PRINT((TEXT(
                                           "[PPNVFL:ERR] _ppnvflFormat - _setMixedBlockEntry fail (line:%d)\n"),
                                       __LINE__));
                        goto return_error;
                    }
                    vbn_idx++;
                }
            }
            if (vbn_idx < mcxt->gen.num_of_vfl_sublk)
            {
                VFL_WRN_PRINT((TEXT(
                                   "[PPNVFL:WRN] _ppnvflFormat - out of good blocks for vbns in bank - 0x%X (line:%d)\n"),
                               bank, __LINE__));
                assign_vbns_out_of_banks = TRUE32;
            }
        }
    }

    // if we need to replace out of bank do it...
    if (assign_vbns_out_of_banks)
    {
        UInt16 vbn_idx, mixed_array_offset;
        pbn = 0;
        bank = 0;
        for (vbn_idx = vbn; ((vbn_idx < mcxt->gen.num_of_vfl_sublk) &&
                             (pbn < mcxt->dev.blocks_per_cau)); vbn_idx++)
        {
            for (mixed_array_offset = 0; ((mixed_array_offset < mcxt->dev.num_of_banks) &&
                                          (pbn < mcxt->dev.blocks_per_cau)); mixed_array_offset++)
            {
                const UInt32 entry_offset = _getMixedArrayEntryOffset(mcxt, vbn_idx, mixed_array_offset);
                if (mcxt->v2p_mixed[entry_offset].pbn == PPN_V2P_UNMAPPED)
                {
                    BOOL32 replacement_found = FALSE32;
                    // choose the lowest physical available - from whichever bank that has free blocks
                    for (; ((pbn < mcxt->dev.blocks_per_cau) && (!replacement_found)); pbn++)
                    {
                        for (bank = (bank == mcxt->dev.num_of_banks ? 0 : bank);
                             ((bank < mcxt->dev.num_of_banks) && (!replacement_found)); bank++)
                        {
                            if (on_nand_block_map[bank][pbn].p2sb.type == PPNVFL_TYPE_AVAILABLE_MARK)
                            {
                                on_nand_block_map[bank][pbn].p2v.vbn = vbn_idx;
                                on_nand_block_map[bank][pbn].p2v.offset = mixed_array_offset;
                                on_nand_block_map[bank][pbn].p2v.reserved = P2V_RESERVED_VALUE;
                                if (!_setMixedBlockEntry(mcxt, vbn_idx, mixed_array_offset, bank, pbn))
                                {
                                    VFL_ERR_PRINT((TEXT(
                                                       "[PPNVFL:ERR] _ppnvflFormat - _setMixedBlockEntry fail (line:%d)\n"),
                                                   __LINE__));
                                    goto return_error;
                                }
                                replacement_found = TRUE32;
                            }
                        }
                    }
                    if (!replacement_found)
                    {
                        VFL_ERR_PRINT((TEXT(
                                           "[PPNVFL:ERR] _ppnvflFormat - could not find blocks for all the vbns (line:%d)\n"),
                                       __LINE__));
                        goto return_error;
                    }
                }
            }
        }
    }

    // end   - 4.2 pick any allowable combination for the rest of the virtual blocks
    // end   - 4. allocate virtual blocks - end

    // start - 5. program the context blocks
    // start - 5.1. erase all the vfl context blocks
    for (vfl_info_idx = 0; vfl_info_idx < mcxt->r_gen.num_of_vfl_blocks; vfl_info_idx++)
    {
        status = ppnMiscSingleOperation(&mcxt->dev, mcxt->single_command, PPN_COMMAND_ERASE,
                                        PPN_NO_OPTIONS,
                                        mcxt->gen.vfl_blocks[vfl_info_idx].bank,
                                        mcxt->gen.vfl_blocks[vfl_info_idx].block,
                                        0, TRUE32, NULL, NULL);
        if (status & PPN_ERASE_STATUS_FAIL)
        {
            // we should not get here - vfl blocks were tested
            VFL_ERR_PRINT((TEXT("[PPNVFL:ERR]  format fail erasing info block fail (line:%d)!\n"), __LINE__));
            goto return_error;
        }
    }
    // end   - 5.1. erase all the vfl context blocks

    // start - 5.2. program the cxt blocks
    if (!_programCxtLastPage(mcxt, buf, TRUE32))
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:WRN] _ppnvflFormat - failed _programCxtLastPage() (line:%d)\n"),
                       __LINE__));
        goto return_error;
    }

    for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
    {
        for (i = 0; i < mcxt->r_gen.pages_per_block_mapping; i++)
        {
            UInt16 start_offset = i * mcxt->r_gen.pbns_per_cxt_mapping_page;
            UInt16 size_to_copy = WMR_MIN(mcxt->dev.main_bytes_per_page,
                                          ((mcxt->dev.blocks_per_cau - start_offset) * sizeof(P2BlockUsageType)));
            WMR_MEMSET(buf->pData, 0xFF, mcxt->dev.main_bytes_per_page);
            WMR_MEMCPY(buf->pData, &on_nand_block_map[bank][start_offset], size_to_copy);
            if (!_programVFLCxtPage(mcxt, buf, bank, i))
            {
                VFL_ERR_PRINT((TEXT("[PPNVFL:WRN] _ppnvflFormat - failed _programCxtLastPage() (line:%d)\n"),
                               __LINE__));
                goto return_error;
            }
        }
    }

    if (!_programCxtLastPage(mcxt, buf, TRUE32))
    {
        VFL_ERR_PRINT((TEXT("[PPNVFL:WRN] _ppnvflFormat - failed _programCxtLastPage() (line:%d)\n"),
                       __LINE__));
        goto return_error;
    }
    // end   - 5.2. program the cxt blocks
    // end   - 5. program the context blocks

    // start - 6. clean up the mess
    // free temp allocations done in this function
    WMR_FREE(on_nand_block_map[0], mcxt->dev.blocks_per_cau * mcxt->dev.num_of_banks * sizeof(P2BlockUsageType));
    WMR_FREE(on_nand_block_map, (sizeof(P2BlockUsageType*) * mcxt->dev.num_of_banks));

    BUF_Release(buf);
    // end   - 6. clean up the mess

    return ANDErrorCodeOk;

 return_error:

    // start - 6. clean up the mess
    // free temp allocations done in this function
    if (on_nand_block_map != NULL && on_nand_block_map[0] != NULL)
    {
        WMR_FREE(on_nand_block_map[0], mcxt->dev.blocks_per_cau * mcxt->dev.num_of_banks * sizeof(P2BlockUsageType));
    }
    if (on_nand_block_map != NULL)
    {
        WMR_FREE(on_nand_block_map, (sizeof(P2BlockUsageType*) * mcxt->dev.num_of_banks));
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
    (PPNVFL_MainCxt * mcxt,
    UInt32 dwStructType,
    void *pvoidStructBuffer,
    UInt32 *pdwStructSize)
{
    BOOL32 boolRes = FALSE32;

    switch (dwStructType & AND_STRUCT_INDEX_MASK)
    {
        case AND_STRUCT_VFL_GET_TYPE:
        {
            UInt8 bVFLType = VFL_TYPE_PPNVFL;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &bVFLType,
                                      sizeof(bVFLType));
            break;
        }

#ifdef AND_COLLECT_STATISTICS
        case AND_STRUCT_VFL_STATISTICS_SIZE:
        {
            UInt32 dwStatSize = sizeof(PPNVFLStatistics);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwStatSize,
                                      sizeof(dwStatSize));
            break;
        }

        case AND_STRUCT_VFL_STATISTICS:
        {
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &mcxt->stat,
                                      sizeof(mcxt->stat));
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
            UInt16 dwNumFTLSuBlks = mcxt->gen.num_of_ftl_sublk;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwNumFTLSuBlks,
                                      sizeof(dwNumFTLSuBlks));
            break;
        }

        case AND_STRUCT_VFL_NUM_OF_SUBLKS:
        {
            UInt16 dwNumVFLSuBlks = mcxt->gen.num_of_vfl_sublk;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwNumVFLSuBlks,
                                      sizeof(dwNumVFLSuBlks));
            break;
        }

        case AND_STRUCT_VFL_BYTES_PER_PAGE:
        {
            UInt16 wPageSize = mcxt->dev.main_bytes_per_page;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wPageSize,
                                      sizeof(wPageSize));
            break;
        }

        case AND_STRUCT_VFL_PAGES_PER_SUBLK:
        {
            UInt16 wPagesPerSuBlk = mcxt->dev.pages_per_sublk;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wPagesPerSuBlk,
                                      sizeof(wPagesPerSuBlk));
            break;
        }

        case AND_STRUCT_VFL_NUM_CHANNELS:
        {
            UInt32 dwNumChannels = mcxt->dev.num_channels;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwNumChannels,
                                      sizeof(dwNumChannels));
            break;
        }

        case AND_STRUCT_VFL_CES_PER_CHANNEL:
        {
            UInt32 dwCesPerChannel = mcxt->dev.ces_per_channel;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwCesPerChannel,
                                      sizeof(dwCesPerChannel));
            break;
        }

        case AND_STRUCT_VFL_CAUS_PER_CE:
        {
            UInt32 dwCausPerCe = mcxt->dev.caus_per_ce;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwCausPerCe,
                                      sizeof(dwCausPerCe));
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
            UInt16 wBytesPerMetaData = (UInt16)mcxt->fil->GetDeviceInfo(AND_DEVINFO_FIL_META_BUFFER_BYTES);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wBytesPerMetaData,
                                      sizeof(wBytesPerMetaData));
            break;
        }

        case AND_STRUCT_VFL_VALID_BYTES_PER_META:
        {
            UInt16 wBytesPerMetaData = (UInt16)mcxt->fil->GetDeviceInfo(AND_DEVINFO_FIL_META_VALID_BYTES);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wBytesPerMetaData,
                                      sizeof(wBytesPerMetaData));
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
            const UInt32 cau_bbt_size = (mcxt->dev.blocks_per_cau / 8) + (mcxt->dev.blocks_per_cau % 8 ? 1 : 0);
            const UInt32 total_ces = mcxt->dev.ces_per_channel * mcxt->dev.num_channels;
            const UInt32 total_caus = mcxt->dev.caus_per_ce * total_ces;
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
            else
            {
                *pdwStructSize = required_buffer_size;
                boolRes = TRUE32;
                pData = (UInt8 *)pvoidStructBuffer;
                buf = BUF_Get(BUF_MAIN_AND_SPARE);
                WMR_ASSERT(buf != NULL);

                for (ce = 0; ce < total_ces; ce++)
                {
                    for (cau = 0; cau < mcxt->dev.caus_per_ce; cau++)
                    {
                        ppnMiscSingleOperation(&mcxt->dev, mcxt->single_command, PPN_COMMAND_CAUBBT,
                                               PPN_NO_OPTIONS, ((cau * total_ces) + ce),
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
            UInt16 correctableBits = (UInt16)mcxt->fil->GetDeviceInfo(AND_DEVINFO_CORRECTABLE_SIZE);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &correctableBits, sizeof(correctableBits));
            break;
        }

        case AND_STRUCT_VFL_CORRECTABLE_SIZE:
        {
            UInt16 correctableSize = (UInt16)mcxt->fil->GetDeviceInfo(AND_DEVINFO_CORRECTABLE_SIZE);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &correctableSize, sizeof(correctableSize));
            break;
        }

        case AND_STRUCT_VFL_PAGES_PER_BLOCK:
        {
            UInt16 pagesPerBlock = (UInt16)mcxt->fil->GetDeviceInfo(AND_DEVINFO_PAGES_PER_BLOCK);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &pagesPerBlock, sizeof(pagesPerBlock));
            break;
        }

        case AND_STRUCT_VFL_DEVICEINFO_SIZE:
        {
            UInt32 deviceInfoSize = sizeof(mcxt->dev);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &deviceInfoSize, sizeof(deviceInfoSize));
            break;
        }

        case AND_STRUCT_VFL_DEVICEINFO:
        {
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &mcxt->dev, sizeof(mcxt->dev));
            break;
        }

        case AND_STRUCT_VFL_GETADDRESS:
        {
            UInt16 bank, block, page_offset, cau;
            ANDAddressStruct *addrStruct = (ANDAddressStruct*)pvoidStructBuffer;
            *pdwStructSize = sizeof(ANDAddressStruct);
            _convertVpnToBankBlockPage(mcxt, addrStruct->dwVpn, &bank, &block, &page_offset);
            cau = ppnMiscGetCAUFromBank(&mcxt->dev, bank);
            addrStruct->dwPpn = ppnMiscConvertToPPNPageAddress(&mcxt->dev, cau, block, page_offset, FALSE32);
            addrStruct->dwCS = (UInt32)ppnMiscGetCEFromBank(&mcxt->dev, bank);
            addrStruct->dwCau = cau;
            boolRes = TRUE32;
            break;
        }

#ifdef AND_COLLECT_STATISTICS
        case AND_STRUCT_VFL_BURNIN_CODE:
        {
            UInt32 code = (UInt32)mcxt->stat.ddwBurnInCode;
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
                     (chip_enable_idx < (mcxt->dev.num_channels * mcxt->dev.ces_per_channel)))
            {
                PPNVFLMetaDataExport *exported_meta = (PPNVFLMetaDataExport*)pvoidStructBuffer;
                UInt16 bank;
                WMR_MEMSET(pvoidStructBuffer, 0xFF, sizeof(PPNVFLMetaDataExport));
                exported_meta->version = PPNVFL_EXPORT_META_VERSION;
                exported_meta->dev_info.num_of_channels = mcxt->dev.num_channels;
                exported_meta->dev_info.ces_per_channel = mcxt->dev.ces_per_channel;
                exported_meta->dev_info.caus_per_ce = mcxt->dev.caus_per_ce;
                for (bank = 0; bank < mcxt->dev.num_of_banks; bank++)
                {
                    exported_meta->per_bank_data[bank].geometry.chip_enable = ppnMiscGetCEFromBank(&mcxt->dev, bank);
                    exported_meta->per_bank_data[bank].geometry.chip_enable_idx = ppnMiscGetCEIdxFromBank(&mcxt->dev,
                                                                                                          bank);
                    exported_meta->per_bank_data[bank].geometry.channel = ppnMiscGetChannelFromBank(&mcxt->dev, bank);
                    exported_meta->per_bank_data[bank].geometry.cau = ppnMiscGetCAUFromBank(&mcxt->dev, bank);
                    exported_meta->per_bank_data[bank].num_special_blocks = mcxt->r_banks[bank].num_special_blocks;
                    exported_meta->per_bank_data[bank].num_grown_bad = mcxt->r_banks[bank].num_grown_bad;
                    exported_meta->per_bank_data[bank].num_initial_bad = mcxt->r_banks[bank].num_initial_bad;
                    exported_meta->per_bank_data[bank].num_available = mcxt->r_banks[bank].num_available;
                    exported_meta->per_bank_data[bank].num_cxt = mcxt->r_banks[bank].num_cxt;
                    exported_meta->per_bank_data[bank].num_p2v = mcxt->r_banks[bank].num_p2v;
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

                mcxt->fpart->MapSpecialBlocks(special_blocks, types, &count);
                max_count = WMR_MIN(count, (*pdwStructSize / sizeof(pastrBlocks[0])));
                for (i = 0; i < max_count; i++)
                {
                    UInt16 channel;
                    UInt16 ceIdx;
                    UInt16 cau;
                    UInt32 ppn_page;
                    channel = ppnMiscGetChannelFromBank(&mcxt->dev, special_blocks[i].bank);
                    ceIdx = ppnMiscGetCEIdxFromBank(&mcxt->dev, special_blocks[i].bank);
                    if (mcxt->format_options & WMR_INIT__USE_NEW_DUAL_CHANNEL_ADDRESS)
                    {
                        pastrBlocks[i].dwPhysicalCE = channel + (ceIdx * mcxt->dev.num_channels);
                    }
                    else
                    {
                        pastrBlocks[i].dwPhysicalCE = (channel * mcxt->dev.ces_per_channel) + ceIdx;
                    }
                    cau = ppnMiscGetCAUFromBank(&mcxt->dev, special_blocks[i].bank);
                    ppn_page = ppnMiscConvertToPPNPageAddress(&mcxt->dev, cau, special_blocks[i].block, 0, FALSE32);
                    pastrBlocks[i].dwPhysicalBlock = (ppn_page >> mcxt->dev.bits_per_page_addr);
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
            VFL_WRN_PRINT((TEXT(
                               "[PPNVFL:WRN]  _ppnvflGetStruct 0x%X is not identified as VFL data struct identifier!\n"),
                           dwStructType));
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
    (PPNVFL_MainCxt * mcxt,
    UInt32 dwParamType)
{
    UInt32 dwRetVal = 0xFFFFFFFF;

    switch (dwParamType)
    {
        case AND_DEVINFO_PAGES_PER_SUBLK:
        {
            dwRetVal = (UInt32)mcxt->dev.pages_per_sublk;
            break;
        }

        case AND_DEVINFO_BYTES_PER_PAGE:
        {
            dwRetVal = (UInt32)mcxt->dev.main_bytes_per_page;
            break;
        }

        case AND_DEVINFO_NUM_OF_BANKS:
        {
            dwRetVal = (UInt32)mcxt->dev.num_of_banks;
            break;
        }

        case AND_DEVINFO_NUM_OF_USER_SUBLK:
        {
            dwRetVal = (UInt32)mcxt->gen.num_of_ftl_sublk;
            break;
        }

        case AND_DEVINFO_FTL_TYPE:
        {
            dwRetVal = (UInt32)mcxt->gen.ftl_type;
            break;
        }

        case AND_DEVINFO_FIL_LBAS_PER_PAGE:
        {
            dwRetVal = (UInt32)mcxt->dev.lbas_per_page;
            break;
        }

        case AND_DEVINFO_FIL_META_VALID_BYTES:
        {
            dwRetVal = (UInt32)mcxt->dev.lba_meta_bytes_valid;
            break;
        }

        case AND_DEVINFO_FIL_META_BUFFER_BYTES:
        {
            dwRetVal = (UInt32)mcxt->dev.lba_meta_bytes_buffer;
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

#define PPNVFL_PRINT_READSCATTERED_SUMMARY (1)

/*
 * Name: _ppnvflReadScatteredPagesInVb
 * Description: implements scattered page read function for the vfl I/F
 * Input: addaress array of pages to read, number of pages to read,
 *        data buffer, meta buffer
 * Output: need to refresh flag, pabSectorStatus (for bonfire), actual status
 * Ignored: whitening flag
 * Return value: TRUE32 if successful, otherwise FALSE32
 */
static BOOL32
_ppnvflReadScatteredPagesInVb
    (PPNVFL_MainCxt * mcxt,
    UInt32 * padwVpn,
    UInt16 wNumPagesToRead,
    UInt8 * pbaData,
    UInt8 * pbaSpare,
    BOOL32 * pboolNeedRefresh,
    UInt8 * pabSectorStats,
    BOOL32 bDisableWhitening,
    Int32 *actualStatus)
{
    UInt16 page_idx;
    UInt16 page_count;
    UInt32 op_status = 0;
    const UInt8 num_sectors = (mcxt->dev.main_bytes_per_page) / 1024;   //Number of kilobyte sectors
    BOOL32 op_result = FALSE32;
    Int32 temp_actual_status;

    VFL_INF_PRINT(PPNVFL_PRINT_READSCATTERED_SUMMARY,
                  (TEXT("[PPNVFL:INF]  ReadScattered (0x%08X, 0x%04X)\n"), padwVpn, wNumPagesToRead));

#ifdef AND_COLLECT_STATISTICS
    mcxt->stat.ddwPagesReadCnt += wNumPagesToRead;
    mcxt->stat.ddwScatteredReadCallCnt++;
#endif

    for (page_idx = 0; page_idx < wNumPagesToRead; )
    {
        UInt16 pages_added = wNumPagesToRead - page_idx;
        UInt8 *data = &pbaData[page_idx * mcxt->dev.main_bytes_per_page];
        UInt8 *spare = &pbaSpare[page_idx * mcxt->dev.lba_meta_bytes_buffer];
        UInt32 *vpn_ptr = &padwVpn[page_idx];

        if (pabSectorStats != NULL)
        {
            _fillScatteredAddressesToCommandStructure(mcxt, mcxt->commands, vpn_ptr, &pages_added,
                                                      PPN_COMMAND_READ, data, spare, PPN_OPTIONS_REPORT_HEALTH);
        }
        else
        {
            _fillScatteredAddressesToCommandStructure(mcxt, mcxt->commands, vpn_ptr, &pages_added,
                                                      PPN_COMMAND_READ, data, spare, PPN_NO_OPTIONS);
        }
 
        mcxt->fil->PerformCommandList(mcxt->commands, mcxt->dev.num_channels);

        if (pabSectorStats != NULL)
        {
            for (page_count = page_idx; page_count < (page_idx + pages_added); page_count++)
            {
                //TO DO. refer <rdar://problem/7920272> Data & Metadata ordering for PPN Health monitoring
                WMR_MEMCPY((pabSectorStats + (page_count * num_sectors)),
                           (pbaData + (page_count * (mcxt->dev.main_bytes_per_page))), num_sectors);
            }
        }
        _analyzeMultiReadStatuses(mcxt, vpn_ptr, spare, &op_status,
                                  (pabSectorStats == NULL ? TRUE32 : FALSE32), pabSectorStats);

        page_idx += pages_added;
    }

    if (op_status & PPNVFL_READ_RES_UNIDENTIFIED)
    {
        // don't panic, per rdar://8234566
        WMR_PRINT(ERROR, "op_status 0x%02x\n", op_status);
        mcxt->fpart->GetLowFuncTbl()->Reset();
        temp_actual_status = ANDErrorCodeHwErr;
    } 
    else if (op_status & PPNVFL_READ_RES_UECC)
    {
        temp_actual_status = ANDErrorCodeUserDataErr;
    }
    else if(op_status & PPNVFL_READ_RES_CLEAN)
    {
        temp_actual_status = ANDErrorCodeCleanOk;
        op_result = TRUE32;
    }
    else if (op_status & PPNVFL_READ_RES_VALID_DATA)
    {
        op_result = TRUE32;
        temp_actual_status = ANDErrorCodeOk;
    }
    else
    {
        // don't panic, per rdar://8234566
        WMR_PRINT(ERROR, "op_status 0x%02x\n", op_status);
        mcxt->fpart->GetLowFuncTbl()->Reset();
        temp_actual_status = ANDErrorCodeHwErr;
    }

    if (actualStatus != NULL)
    {
        *actualStatus = temp_actual_status;
    }

    if (pboolNeedRefresh != NULL)
    {
        if (op_status & (PPNVFL_READ_RES_REFRESH | PPNVFL_READ_RES_RETIRE | PPNVFL_READ_RES_UECC))
        {
            *pboolNeedRefresh = TRUE32;
        }
        else
        {
            *pboolNeedRefresh = FALSE32;
        }
    }
    return op_result;
}

/*
 * Name: _ppnvflReadSinglePage
 * Description: implements single page read function for the vfl I/F
 * Input: virtual page address, data buffer, meta buffer, mark for scrub
 * Output: need to refresh flag
 * Ignored: pabSectorStatus and whitening flag and actual status
 * Return value: TRUE32 if successful, otherwise FALSE32
 */
static Int32
_ppnvflReadSinglePage
    (PPNVFL_MainCxt * mcxt,
    UInt32 nVpn,
    Buffer *pBuf,
    BOOL32 bCleanCheck,
    BOOL32 bMarkECCForScrub,
    BOOL32 * pboolNeedRefresh,
    UInt8 * pabSectorStats,
    BOOL32 bDisableWhitening)
{
    UInt32 op_status = 0;
    UInt16 num_pages = 1;

#ifdef AND_COLLECT_STATISTICS
    mcxt->stat.ddwPagesReadCnt++;
    mcxt->stat.ddwSingleReadCallCnt++;
#endif

    if (pabSectorStats != NULL)
    {
        _fillScatteredAddressesToCommandStructure(mcxt, mcxt->commands, &nVpn, &num_pages, PPN_COMMAND_READ,
                                                  pBuf->pData, pBuf->pSpare,
                                                  ((bMarkECCForScrub ? 0 : PPN_OPTIONS_IGNORE_BAD_BLOCK) |
                                                   PPN_OPTIONS_REPORT_HEALTH));
    }
    else
    {
        _fillScatteredAddressesToCommandStructure(mcxt, mcxt->commands, &nVpn, &num_pages, PPN_COMMAND_READ,
                                                  pBuf->pData, pBuf->pSpare,
                                                  (bMarkECCForScrub ? 0 : PPN_OPTIONS_IGNORE_BAD_BLOCK));
    }

    mcxt->fil->PerformCommandList(mcxt->commands, mcxt->dev.num_channels);
    
    if (pabSectorStats != NULL)
    {
        //TO DO. refer <rdar://problem/7920272> Data & Metadata ordering for PPN Health monitoring
        WMR_MEMCPY(pabSectorStats, pBuf->pData, ((mcxt->dev.main_bytes_per_page) / 1024));    //Copy corresponding to
                                                                                              // number of kilobyte
                                                                                              // sectors
    }
    _analyzeMultiReadStatuses(mcxt, &nVpn, pBuf->pSpare, &op_status, bMarkECCForScrub, pabSectorStats);

    if (pboolNeedRefresh != NULL)
    {
        if (op_status & (PPNVFL_READ_RES_REFRESH | PPNVFL_READ_RES_RETIRE | PPNVFL_READ_RES_UECC))
        {
            *pboolNeedRefresh = TRUE32;
        }
        else
        {
            *pboolNeedRefresh = FALSE32;
        }
    }

    if ((op_status & PPNVFL_READ_RES_UECC) ||
        ((op_status & PPNVFL_READ_RES_CLEAN) && (!bCleanCheck)))
    {
        return ANDErrorCodeUserDataErr;
    }
    else if (op_status & PPNVFL_READ_RES_CLEAN)
    {
        return ANDErrorCodeCleanOk;
    }
    else if (op_status & PPNVFL_READ_RES_VALID_DATA)
    {
        return ANDErrorCodeOk;
    }
    VFL_ERR_PRINT((TEXT("PPNVFL:ERR] _ppnvflReadSinglePage fail with op_status 0x%X (l:%d)\n"), op_status, __LINE__));
    return ANDErrorCodeHwErr;
}

/*
 * Name: _ppnvflGetFTLCxtVbn
 * Description: implements GetFTLCxt for the vfl I/F
 * Input:
 * Return value: ftl cxt addresses array
 */
static UInt16*
_ppnvflGetFTLCxtVbn
    (PPNVFL_MainCxt * mcxt)
{
    return mcxt->gen.ftl_cxt_vbn;
}

#ifndef AND_READONLY
/*
 * Name: _ppnvflWriteMultiplePagesInVb
 * Description: implements multi page program for the vfl I/F
 * Input: start virtual page address, number of page to program,
 *        data buffer, meta buffer
 * ignored: replace block on fail, whitening
 * Return value: TRUE32 if successful, otherwise FALSE32
 */

#define PPNVFL_PRINT_WRITEMULTI_SUMMARY (0)

static BOOL32
_ppnvflWriteMultiplePagesInVb
    (PPNVFL_MainCxt * mcxt,
    UInt32 dwVpn,
    UInt16 wNumPagesToWrite,
    UInt8 * pbaData,
    UInt8 * pbaSpare,
    BOOL32 boolReplaceBlockOnFail,
    BOOL32 bDisableWhitening)
{
    UInt16 channel, i;
    BOOL32 res = TRUE32, last_failure_updated = FALSE32;
    UInt16 page_idx;
    UInt16 phyCE;

    VFL_INF_PRINT(PPNVFL_PRINT_WRITEMULTI_SUMMARY, (TEXT("[PPNVFL:INF]  WriteMulti(0x%08X, 0x%04X) (line:%d)!\n"),
                                                    dwVpn, wNumPagesToWrite, __LINE__));
#ifdef AND_COLLECT_STATISTICS
    mcxt->stat.ddwPagesWrittenCnt += wNumPagesToWrite;
    mcxt->stat.ddwMultipleWriteCallCnt++;
#endif

    for (page_idx = 0; page_idx < wNumPagesToWrite && res; )
    {
        UInt16 pages_added = wNumPagesToWrite - page_idx;

        _fillSequentialAddressesToCommandStructure(mcxt, mcxt->commands, (dwVpn + page_idx),
                                                   &pages_added, PPN_COMMAND_PROGRAM,
                                                   &pbaData[page_idx * mcxt->dev.main_bytes_per_page],
                                                   &pbaSpare[page_idx * mcxt->dev.lba_meta_bytes_buffer],
                                                   PPN_NO_OPTIONS);
        mcxt->fil->PerformCommandList(mcxt->commands, mcxt->dev.num_channels);

        // handle program errors
        for (channel = 0; channel < mcxt->dev.num_channels; channel++)
        {
            PPNCommandStruct    *command = mcxt->commands[channel];

            for (i = 0; ((command->page_status_summary & PPN_PROGRAM_STATUS_FAIL) &&
                         (i < command->num_pages)); i++)
            {
                if (command->entry[i].status & PPN_PROGRAM_STATUS_FAIL)
                {
                    UInt16 bank, block, page_offset;
                    BOOL32 slc;

                    ppnMiscConvertPhysicalAddressToBankBlockPage(&mcxt->dev, channel,
                                                                 command->entry[i].ceIdx,
                                                                 command->entry[i].addr.row, &bank, &block,
                                                                 &page_offset,
                                                                 &slc);

                    if (last_failure_updated == FALSE32)
                    {
                        phyCE = ppnMiscGetCEFromBank(&mcxt->dev,bank);
                        _ReportFailure(VFLFailWrite, phyCE,
                                       command->entry[i].addr.row);
                        last_failure_updated = TRUE32;
                    }

                    if (boolReplaceBlockOnFail == TRUE32)
                    {
                        VFL_WRN_PRINT((TEXT(
                                           "[PPNVFL:WRN] marking (vpn: 0x%X, bank: 0x%X, block: 0x%X) for scrub (program failed) (l:%d)\n"),
                                       dwVpn, bank, block, __LINE__));
                        _markBlockForReplacement(mcxt, bank, block);
                    }
                    VFL_WRN_PRINT((TEXT(
                                       "[PPNVFL:WRN] failed programming (bank: 0x%X, block: 0x%X, page offset: 0x%X) (line:%d)\n"),
                                   bank, block, page_offset, __LINE__));
                    res = FALSE32;
                }
            }
        }
        page_idx += pages_added;
    }
    return res;
}

/*
 * Name: _ppnvflWriteMultiplePagesInVb
 * Description: implements multi page program for the vfl I/F
 * Input: virtual page address, buffer (main + spare)
 * ignored: replace block on fail, whitening
 * Return value: ANDErrorCodeOk if successful, otherwise ANDErrorWriteFailureErr
 */
static Int32
_ppnvflWriteSinglePage
    (PPNVFL_MainCxt * mcxt,
    UInt32 nVpn,
    Buffer *pBuf,
    BOOL32 boolReplaceBlockOnFail,
    BOOL32 bDisableWhitening)
{
    BOOL32 bWriteReturn;

    bWriteReturn = _ppnvflWriteMultiplePagesInVb(mcxt, nVpn, 1, pBuf->pData, pBuf->pSpare,
                                                 boolReplaceBlockOnFail, bDisableWhitening);
#ifdef AND_COLLECT_STATISTICS
    mcxt->stat.ddwSingleWriteCallCnt++;
    mcxt->stat.ddwMultipleWriteCallCnt--;
#endif
    if (bWriteReturn == TRUE32)
    {
        return ANDErrorCodeOk;
    }
    return ANDErrorWriteFailureErr;
}

/*
 * Name: _ppnvflErase
 * Description: implements erase single virtual block for the vfl I/F
 * Input: virtual block address, replace block on fail
 * Return value: ANDErrorCodeOk if successful, otherwise ANDErrorWriteFailureErr
 */
#define PPNVFL_PRINT_VFLERASE_SUMMARY (0)
static Int32
_ppnvflErase
    (PPNVFL_MainCxt * mcxt,
    UInt16 wVbn,
    BOOL32 bReplaceOnFail)
{
    UInt32 vpn;
    UInt16 i, channel, mixed_array_offset;
    UInt16 blocks_to_erase = mcxt->dev.num_of_banks;
    BOOL32 last_failure_updated = FALSE32;
    UInt16 phyCE;

    VFL_INF_PRINT(PPNVFL_PRINT_VFLERASE_SUMMARY, (TEXT("[PPNVFL:INF]  Erase (0x%04X)\n"), wVbn));

#ifdef AND_COLLECT_STATISTICS
    mcxt->stat.ddwBlocksErasedCnt++;
    mcxt->stat.ddwEraseCallCnt++;
#endif

    vpn = wVbn * mcxt->dev.pages_per_sublk;
    // check if there are any blocks to replace
    for (mixed_array_offset = 0; ((mcxt->gen.scrub_idx != 0) &&
                                  (mixed_array_offset < mcxt->dev.num_of_banks) &&
                                  bReplaceOnFail); mixed_array_offset++)
    {
        UInt16 temp_block, temp_bank, temp_page_offset;
        _convertVpnToBankBlockPage(mcxt, (vpn + mixed_array_offset), &temp_bank, &temp_block, &temp_page_offset);
        for (i = 0; i < mcxt->gen.scrub_idx; i++)
        {
            // check if this is the block we need to replace
            if (mcxt->gen.scrub_list[i].bank == temp_bank && mcxt->gen.scrub_list[i].block == temp_block)
            {
                if (bReplaceOnFail == TRUE32)
                {
                    if (_replaceBadBlock(mcxt, temp_bank, temp_block, wVbn, mixed_array_offset) == FALSE32)
                    {
                        VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failed replacing a bad block (bank:0x%04X) (line:%)\n"), temp_bank,
                                       __LINE__));
                        return ANDErrorWriteFailureErr;
                    }
                }
            }
        }
    }

    _fillSequentialAddressesToCommandStructure(mcxt, mcxt->commands, vpn, &blocks_to_erase, PPN_COMMAND_ERASE, NULL,
                                               NULL, PPN_NO_OPTIONS);
    mcxt->fil->PerformCommandList(mcxt->commands, mcxt->dev.num_channels);

    // handle erase errors
    for (channel = 0; channel < mcxt->dev.num_channels; channel++)
    {
        PPNCommandStruct    *command = mcxt->commands[channel];

        for (i = 0; ((command->page_status_summary & PPN_ERASE_STATUS_RETIRE) &&
                     (i < command->num_pages)); i++)
        {
            if (command->entry[i].status & PPN_ERASE_STATUS_RETIRE)
            {
                UInt16 bank, block, page_offset;
                BOOL32 slc;
                PPNStatusType status;
                UInt8 trials = 0;

                ppnMiscConvertPhysicalAddressToBankBlockPage(&mcxt->dev, channel,
                                                             command->entry[i].ceIdx,
                                                             command->entry[i].addr.row, &bank, &block,
                                                             &page_offset,
                                                             &slc);
                if (last_failure_updated == FALSE32)
                {
                    phyCE = ppnMiscGetCEFromBank(&mcxt->dev,bank);
                    _ReportFailure(VFLFailErase, phyCE,
                                   command->entry[i].addr.row);
                    last_failure_updated = TRUE32;
                }

                WMR_PRINT(QUAL, "Erase status failure on bank %d block %d\n", (UInt32)bank, (UInt32)block);
                // replace a bad block if the caller asked us to do so
                if (bReplaceOnFail == TRUE32)
                {
                    for (trials = 0; trials < 3; trials++)
                    {
                        for (mixed_array_offset = 0; mixed_array_offset < mcxt->dev.num_of_banks; mixed_array_offset++)
                        {
                            UInt16 temp_bank = 0, temp_block = 0, temp_page_offset = 0;
                            _convertVpnToBankBlockPage(mcxt, (vpn + mixed_array_offset), &temp_bank, &temp_block,
                                                       &temp_page_offset);
                            if (bank == temp_bank && block == temp_block)
                            {
                                break;
                            }
                        }

                        if (mixed_array_offset == mcxt->dev.num_of_banks)
                        {
                            VFL_ERR_PRINT((TEXT(
                                               "[PPNVFL:ERR] error matching the failing block to vbn (bank: 0x%X, block:0x%X, vbn: 0x%X) (l:%d)\n"),
                                           bank, block, wVbn, __LINE__));
                            return ANDErrorWriteFailureErr;
                        }

                        if (_replaceBadBlock(mcxt, bank, block, wVbn, mixed_array_offset) == FALSE32)
                        {
                            VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] error replacing a block\n")));
                            return ANDErrorWriteFailureErr;
                        }

                        // erase the replacement block
                        _convertVpnToBankBlockPage(mcxt, (vpn + mixed_array_offset), &bank, &block, &page_offset);
                        status =
                            ppnMiscSingleOperation(&mcxt->dev, mcxt->single_command, PPN_COMMAND_ERASE,
                                                   PPN_NO_OPTIONS, bank, block, page_offset, slc, NULL, NULL);
                        if (!(status & PPN_ERASE_STATUS_RETIRE || status & PPN_ERASE_STATUS_FAIL))
                        {
                            break;
                        }
                        else
                        {
                            WMR_PRINT(QUAL, "Erase status failure on bank %d block %d\n", (UInt32)bank, (UInt32)block);
                        }
                    }
                }
                else
                {
                    return ANDErrorWriteFailureErr;
                }
            }
        }
    }

    return ANDErrorCodeOk;
}

/*
 * Name: _ppnvflChangeFTLCxtVbn
 * Description: implements ChangeFTLCxt for the vfl I/F
 * Input: new ftl cxt array
 * Return value: ANDErrorCodeOk if successful
 */
#define PPNVFL_PRINT_CHANGEFTLCXT_SUMMARY (1)
static Int32
_ppnvflChangeFTLCxtVbn
    (PPNVFL_MainCxt * mcxt,
    UInt16 *aFTLCxtVbn)
{
    Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);

    VFL_INF_PRINT(PPNVFL_PRINT_CHANGEFTLCXT_SUMMARY, (TEXT("[PPNVFL:INF]  ChangeFTLCxtVbn (0x%04X, 0x%04X, 0x%04X)\n"),
                                                      aFTLCxtVbn[0], aFTLCxtVbn[1], aFTLCxtVbn[2]));
    WMR_MEMCPY(mcxt->gen.ftl_cxt_vbn, aFTLCxtVbn, (FTL_CXT_SECTION_SIZE * sizeof(UInt16)));

    _programCxtLastPage(mcxt, buf, TRUE32);
    BUF_Release(buf);

    return ANDErrorCodeOk;
}
#endif // ! AND_READONLY

/*
 * Name: _ppnvflSetStruct
 * Description: implements SetStruct for the vfl I/F
 * Input: type, buffer, buffer size
 * Return value: TRUE32 if successful, otherwise FALSE32
 */
static BOOL32
_ppnvflSetStruct
    (PPNVFL_MainCxt * mcxt,
    UInt32 dwStructType,
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
            mcxt->gen.ftl_type = (*((UInt32*)pvoidStructBuffer));

            result = _programCxtLastPage(mcxt, buf, TRUE32);
            BUF_Release(buf);
        }
        break;

        case AND_STRUCT_VFL_NUM_OF_FTL_SUBLKS:
        {
            result = _setNumOfFTLSuBlks(mcxt, *((UInt16*)pvoidStructBuffer));
        }
        break;
#endif // AND_READONLY

#ifdef AND_COLLECT_STATISTICS
        case AND_STRUCT_VFL_BURNIN_CODE:
        {
            if (NULL != pvoidStructBuffer)
            {
                mcxt->stat.ddwBurnInCode = *((UInt32*)pvoidStructBuffer);
                result = TRUE32;
            }
        }
        break;

        case AND_STRUCT_VFL_STATISTICS:
        {
            if ((NULL != pvoidStructBuffer))
            {
                WMR_MEMCPY(&mcxt->stat, pvoidStructBuffer, WMR_MIN(sizeof(PPNVFLStatistics), dwStructSize));
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
            VFL_ERR_PRINT((TEXT(
                               "[PPNVFL:ERR]  _ppnvflSetStruct 0x%X is not identified is VFL data struct identifier!\n"),
                           dwStructType));
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
    (PPNVFL_MainCxt * mcxt)
{
    if (mcxt->v2p_main != NULL)
    {
        WMR_FREE(mcxt->v2p_main, (sizeof(UInt16) * mcxt->gen.num_of_vfl_sublk));
        mcxt->v2p_main = NULL;
    }
    if (mcxt->v2p_mixed)
    {
        WMR_FREE(mcxt->v2p_mixed, _getV2PMixedArraySize(mcxt));
        mcxt->v2p_mixed = NULL;
    }
    if (mcxt->vfl_map_pages)
    {
        if (mcxt->vfl_map_pages[0])
        {
            WMR_FREE(mcxt->vfl_map_pages[0], (sizeof(UInt16) * mcxt->dev.num_of_banks * mcxt->r_gen.pages_per_block_mapping));
        }
        WMR_FREE(mcxt->vfl_map_pages, (sizeof(UInt16*) * mcxt->dev.num_of_banks));
        mcxt->vfl_map_pages = NULL;
    }
    if (mcxt->r_banks)
    {
        WMR_FREE(mcxt->r_banks, (sizeof(PPNVFL_BankRAMCxt) * mcxt->dev.num_of_banks));
        mcxt->r_banks = NULL;
    }

    if (mcxt->reorder)
    {
        WMR_FREE(mcxt->reorder, sizeof(PPNReorderStruct));
        mcxt->reorder = NULL;
    }

    if (mcxt->initialized)
    {
        WMR_BufZone_Free(&mcxt->bufzone);
    }
    WMR_MEMSET(mcxt, 0, sizeof(PPNVFL_MainCxt));
}

#ifndef AND_READONLY
#if (defined(AND_SUPPORT_BLOCK_BORROWING) && AND_SUPPORT_BLOCK_BORROWING)
static BOOL32
_ppnvflBorrowSpareBlock
    (PPNVFL_MainCxt * mcxt,
    UInt32 *pdwPhysicalCE,
    UInt32 *pdwPhysicalBlock,
    UInt16 wType)
{
    BOOL32 res;
    SpecialBlockAddress special_block;

    res = _ppnvflAllocateSpecialBlock(mcxt, &special_block, wType);
    if (res)
    {
        UInt16 channel;
        UInt16 ceIdx;
        UInt16 cau;
        UInt32 ppn_page;
        channel = ppnMiscGetChannelFromBank(&mcxt->dev, special_block.bank);
        ceIdx = ppnMiscGetCEIdxFromBank(&mcxt->dev, special_block.bank);
        if (mcxt->format_options & WMR_INIT__USE_NEW_DUAL_CHANNEL_ADDRESS)
        {
            *pdwPhysicalCE = channel + (ceIdx * mcxt->dev.num_channels);
        }
        else
        {
            *pdwPhysicalCE = (channel * mcxt->dev.ces_per_channel) + ceIdx;
        }
        cau = ppnMiscGetCAUFromBank(&mcxt->dev, special_block.bank);
        ppn_page = ppnMiscConvertToPPNPageAddress(&mcxt->dev, cau, special_block.block, 0, FALSE32);
        *pdwPhysicalBlock = (ppn_page >> mcxt->dev.bits_per_page_addr);
    }
    return res;
}
#endif // AND_SUPPORT_BLOCK_BORROWING

static BOOL32
_ppnvflMarkBlockAsGrownBad
    (PPNVFL_MainCxt * mcxt,
    SpecialBlockAddress *block)
{
    P2BlockUsageType block_usage;
    BOOL32 result;

    block_usage.p2sb.type = PPNVFL_TYPE_GROWN_BB;
    block_usage.p2sb.reserved = P2SB_RESERVED_VALUE;
    mcxt->r_banks[block->bank].num_special_blocks--;
    mcxt->r_banks[block->bank].num_grown_bad++;
    result = _setBlockUsageInVFLCxt(mcxt, block->bank, block->block, &block_usage, NULL);

    if (result)
    {
        Buffer *buf = BUF_Get(BUF_MAIN_AND_SPARE);

        result = _programCxtLastPage(mcxt, buf, TRUE32);
        BUF_Release(buf);
    }

    return result;
}
#endif // ! AND_READONLY

/* vfl I/F implementation - end */

/* global calls for the ppnvfl (using the declared global main vfl cxt struct - start */
static Int32
_g_ppnvflInit
    (FPartFunctions * pFPartFunctions)
{
    // Init a global main structure and call the real init
    if (g_ppnvflMain == NULL)
    {
        g_ppnvflMain = (PPNVFL_MainCxt*)WMR_MALLOC(sizeof(PPNVFL_MainCxt));
        if (g_ppnvflMain == NULL)
        {
            VFL_ERR_PRINT((TEXT("[PPNVFL:ERR] failure to allocate g_ppnvflMain (line:%d)"), __LINE__));
            return ANDErrorCodeHwErr;
        }
        WMR_MEMSET(g_ppnvflMain, 0, sizeof(PPNVFL_MainCxt));
    }
    return _ppnvflInit(g_ppnvflMain, pFPartFunctions);
}

static Int32
_g_ppnvflOpen
    (UInt32 dwKeepout,
    UInt32 minor_ver,
    UInt32 dwOptions)
{
    return _ppnvflOpen(g_ppnvflMain, dwKeepout, minor_ver, dwOptions);
}
#ifndef AND_READONLY
static Int32
_g_ppnvflFormat
    (UInt32 dwKeepout,
    UInt32 dwOptions)
{
    return _ppnvflFormat(g_ppnvflMain, dwKeepout, dwOptions);
}
#endif //!AND_READONLY

static BOOL32
_g_ppnvflGetStruct
    (UInt32 dwStructType,
    void * pvoidStructBuffer,
    UInt32 * pdwStructSize)
{
    return _ppnvflGetStruct(g_ppnvflMain, dwStructType, pvoidStructBuffer, pdwStructSize);
}

static UInt32
_g_ppnvflGetDeviceInfo
    (UInt32 dwParamType)
{
    return _ppnvflGetDeviceInfo(g_ppnvflMain, dwParamType);
}

static BOOL32
_g_ppnvflReadScatteredPagesInVb
    (UInt32 * padwVpn,
    UInt16 wNumPagesToRead,
    UInt8 * pbaData,
    UInt8 * pbaSpare,
    BOOL32 * pboolNeedRefresh,
    UInt8 * pabSectorStats,
    BOOL32 bDisableWhitening,
    Int32 *actualStatus)
{
    return _ppnvflReadScatteredPagesInVb(g_ppnvflMain, padwVpn, wNumPagesToRead, pbaData,
                                         pbaSpare, pboolNeedRefresh, pabSectorStats,
                                         bDisableWhitening, actualStatus);
}

static Int32
_g_ppnvflReadSinglePage
    (UInt32 nVpn,
    Buffer *pBuf,
    BOOL32 bCleanCheck,
    BOOL32 bMarkECCForScrub,
    BOOL32 * pboolNeedRefresh,
    UInt8 * pabSectorStats,
    BOOL32 bDisableWhitening)
{
    return _ppnvflReadSinglePage(g_ppnvflMain, nVpn, pBuf, bCleanCheck, bMarkECCForScrub,
                                 pboolNeedRefresh, pabSectorStats, bDisableWhitening);
}
static UInt16*
_g_ppnvflGetFTLCxtVbn
    (void)
{
    return _ppnvflGetFTLCxtVbn(g_ppnvflMain);
}

#ifndef AND_READONLY

static Int32
_g_ppnvflWriteSinglePage
    (UInt32 nVpn,
    Buffer *pBuf,
    BOOL32 boolReplaceBlockOnFail,
    BOOL32 bDisableWhitening)
{
    return _ppnvflWriteSinglePage(g_ppnvflMain, nVpn, pBuf, boolReplaceBlockOnFail, bDisableWhitening);
}

static Int32
_g_ppnvflErase
    (UInt16 wVbn,
    BOOL32 bReplaceOnFail)
{
    return _ppnvflErase(g_ppnvflMain, wVbn, bReplaceOnFail);
}

static Int32
_g_ppnvflChangeFTLCxtVbn
    (UInt16 *aFTLCxtVbn)
{
    return _ppnvflChangeFTLCxtVbn(g_ppnvflMain, aFTLCxtVbn);
}

static BOOL32
_g_ppnvflWriteMultiplePagesInVb
    (UInt32 dwVpn,
    UInt16 wNumPagesToWrite,
    UInt8 * pbaData,
    UInt8 * pbaSpare,
    BOOL32 boolReplaceBlockOnFail,
    BOOL32 bDisableWhitening)
{
    return _ppnvflWriteMultiplePagesInVb(g_ppnvflMain, dwVpn, wNumPagesToWrite, pbaData, pbaSpare,
                                         boolReplaceBlockOnFail, bDisableWhitening);
}

static BOOL32
_g_ppnvflAllocateSpecialBlock
    (SpecialBlockAddress *block,
    UInt16 type)
{
    return _ppnvflAllocateSpecialBlock(g_ppnvflMain, block, type);
}

#if (defined(AND_SUPPORT_BLOCK_BORROWING) && AND_SUPPORT_BLOCK_BORROWING)
static BOOL32
_g_ppnvflBorrowSpareBlock
    (UInt32 *pdwPhysicalCE,
    UInt32 *pdwPhysicalBlock,
    UInt16 wType)
{
    return _ppnvflBorrowSpareBlock(g_ppnvflMain, pdwPhysicalCE, pdwPhysicalBlock, wType);
}
#endif // AND_SUPPORT_BLOCK_BORROWING

static BOOL32
_g_ppnvflMarkBlockAsGrownBad
    (SpecialBlockAddress *block)
{
    return _ppnvflMarkBlockAsGrownBad(g_ppnvflMain, block);
}

#endif // ! AND_READONLY

static UInt32
_g_ppnvflGetMinorVersion
    (void)
{
    return kPPNVFLMinorVersion;
}

static UInt16
_g_ppnvflGetVbasPerVb
    (UInt16 wVbn)
{
    return g_ppnvflMain->dev.pages_per_sublk;
}

static BOOL32
_g_ppnvflSetStruct
    (UInt32 dwStructType,
    void * pvoidStructBuffer,
    UInt32 dwStructSize)
{
    return _ppnvflSetStruct(g_ppnvflMain, dwStructType, pvoidStructBuffer, dwStructSize);
}

static void
_g_ppnvflClose
    (void)
{
    if (g_ppnvflMain != NULL)
    {
        _ppnvflClose(g_ppnvflMain);
        WMR_FREE(g_ppnvflMain, sizeof(PPNVFL_MainCxt));
        g_ppnvflMain = NULL;
    }
}

void
PPNVFL_Register
    (VFLFunctions * pVFL_Functions)
{
    WMR_MEMSET(pVFL_Functions, 0, sizeof(VFLFunctions));
#ifndef AND_READONLY
    pVFL_Functions->Format = _g_ppnvflFormat;
    pVFL_Functions->WriteSinglePage = _g_ppnvflWriteSinglePage;
    pVFL_Functions->Erase = _g_ppnvflErase;
    pVFL_Functions->ChangeFTLCxtVbn = _g_ppnvflChangeFTLCxtVbn;
    pVFL_Functions->WriteMultiplePagesInVb = _g_ppnvflWriteMultiplePagesInVb;
#if (defined(AND_SUPPORT_BLOCK_BORROWING) && AND_SUPPORT_BLOCK_BORROWING)
    pVFL_Functions->BorrowSpareBlock = _g_ppnvflBorrowSpareBlock;
#endif // AND_SUPPORT_BLOCK_BORROWING
#endif // ! AND_READONLY
    pVFL_Functions->Init = _g_ppnvflInit;
    pVFL_Functions->Open = _g_ppnvflOpen;
    pVFL_Functions->Close = _g_ppnvflClose;
    pVFL_Functions->ReadScatteredPagesInVb = _g_ppnvflReadScatteredPagesInVb;
    pVFL_Functions->ReadSinglePage = _g_ppnvflReadSinglePage;
    pVFL_Functions->GetFTLCxtVbn = _g_ppnvflGetFTLCxtVbn;
    //	pVFL_Functions->GetAddress = VFL_GetAddress;
    pVFL_Functions->GetStruct = _g_ppnvflGetStruct;
    pVFL_Functions->SetStruct = _g_ppnvflSetStruct;
    pVFL_Functions->GetDeviceInfo = _g_ppnvflGetDeviceInfo;
#ifndef AND_READONLY
    pVFL_Functions->AllocateSpecialBlock = _g_ppnvflAllocateSpecialBlock;
    pVFL_Functions->MarkBlockAsGrownBad = _g_ppnvflMarkBlockAsGrownBad;
#endif
    pVFL_Functions->GetMinorVersion = _g_ppnvflGetMinorVersion;
    pVFL_Functions->GetVbasPerVb = _g_ppnvflGetVbasPerVb;
}

static VFLFunctions _ppnvfl_functions;

VFLFunctions *
PPN_VFL_GetFunctions
    (void)
{
    PPNVFL_Register(&_ppnvfl_functions);
    return &_ppnvfl_functions;
}

/* global calls for the ppnvfl (using the declared global main vfl cxt struct - end */
