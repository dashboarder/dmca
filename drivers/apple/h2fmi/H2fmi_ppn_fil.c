// *****************************************************************************
//
// File: H2fmi_ppn_fil.c
//
// Copyright (C) 2010 Apple Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Inc.
//
// *****************************************************************************

#include "WMROAM.h"
#include <FIL.h>
#include <FILTypes.h>
#include "H2fmi_private.h"
#include "H2fmi.h"
#include "H2fmi_ppn.h"

#if SUPPORT_PPN

extern UInt32 *g_pScratchBuf;

extern h2fmi_t g_fmi0;
extern h2fmi_t g_fmi1;

#if (defined(AND_COLLECT_FIL_STATISTICS) && AND_COLLECT_FIL_STATISTICS)
#define AND_FIL_STAT(x)    (x)
extern FILStatistics stFILStatistics;
#else
#define AND_FIL_STAT(x)
#endif /* AND_COLLECT_FIL_STATISTICS */

void  h2fmi_ppn_recover_nand(void);
static Int32 h2fmiPpnReadMulti(h2fmi_t *fmi, PPNCommandStruct *ppnCommand);
static Int32 h2fmiPpnReadMultiWithSingles(h2fmi_t *fmi, PPNCommandStruct *ppnCommand);
static PPNCommandStruct   *topLevelSingle = NULL;
static PPNCommandStruct   *lowLevelSingle = NULL;
static struct dma_segment *dataSegments = NULL;
static struct dma_segment *metaSegments = NULL;

#if (defined(AND_SUPPORT_BLOCK_STORAGE) && AND_SUPPORT_BLOCK_STORAGE)
static Int32 h2fmiPpnWriteMulti(h2fmi_t *fmi, PPNCommandStruct *ppnCommand);
#endif

BOOL32 h2fmi_ppn_fil_init(void)
{
    if (NULL == lowLevelSingle)
    {
        lowLevelSingle = WMR_MALLOC(sizeof(*lowLevelSingle));
        if (NULL == lowLevelSingle)
        {
            return FALSE32;
        }
    }
    if (NULL == topLevelSingle)
    {
        topLevelSingle = WMR_MALLOC(sizeof(*topLevelSingle));
        if (NULL == topLevelSingle)
        {
            return FALSE32;
        }
    }
    if (NULL == dataSegments)
    {
        dataSegments = WMR_MALLOC(PPN_MAX_PAGES * sizeof(struct dma_segment));
        if (NULL == dataSegments)
        {
            return FALSE32;
        }
    }
    if (NULL == metaSegments)
    {
        metaSegments = WMR_MALLOC(PPN_MAX_PAGES * sizeof(struct dma_segment));
        if (NULL == metaSegments)
        {
            return FALSE32;
        }
    }
    return TRUE32;
}

void h2fmiPpnPerformCommandList(PPNCommandStruct **commands, UInt32 num_commands)
{
    UInt32  i;

    for (i = 0; i < num_commands; i++)
    {
        PPNCommandStruct  *command = commands[i];
        h2fmi_t           *fmi     = (command->context.bus_num == 1) ? &g_fmi1 : &g_fmi0;
        Int32              status;

        if (command->num_pages == 0)
        {
            continue;
        }

        switch(command->command)
        {
            case PPN_COMMAND_READ:
#if (defined(H2FMI_ALLOW_MULTIS) && H2FMI_ALLOW_MULTIS)                
                if (!(command->options & PPN_OPTIONS_REPORT_HEALTH))
                {
                    status = h2fmiPpnReadMulti(fmi, command);
                }
                else
#endif
                {
                    status = h2fmiPpnReadMultiWithSingles(fmi, command);   
                }
                break;

#if !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM
#if (defined(AND_SUPPORT_BLOCK_STORAGE) && AND_SUPPORT_BLOCK_STORAGE)
            case PPN_COMMAND_PROGRAM:                
                status = h2fmiPpnWriteMulti(fmi, command);
                break;
#endif

            case PPN_COMMAND_ERASE:
                status = h2fmi_ppn_erase_blocks(fmi, command);
                break;
#endif // !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM

            case PPN_COMMAND_CAUBBT:
                status = h2fmi_ppn_read_cau_bbt(fmi, command, command->mem_buffers[0].data);
                break;

            default:
                WMR_PANIC("Invalid PPN FIL command: %d", command->command);
                break;
        }

        switch(status)
        {
            case _kIOPFMI_STATUS_PPN_GENERAL_ERROR:
                WMR_PRINT(ERROR, "Command type 0x%08x failed due to PPN Controller General Error on bus %lu ce %lu\n",
                          command->command, fmi->bus_id, fmi->ppn->general_error_ce);

                h2fmi_ppn_recover_nand();
                break;

            case _kIOPFMI_STATUS_READY_BUSY_TIMEOUT:
                WMR_PRINT(ERROR, "Command type 0x%08x failed due to PPN controller status timeout on bus %lu\n",
                          command->command, fmi->bus_id);

                h2fmi_ppn_recover_nand();
                break;
        }
    }
}

Int32 h2fmiPpnReadMulti(h2fmi_t          *fmi,
                               PPNCommandStruct *ppnCommand)
{
    const UInt32 numPages = ppnCommand->num_pages;
    UInt32       page;
    Int32        ret;
    UInt8       *metaBounceBuffer;
    UInt8       *metaSrc;

    WMR_ASSERT(metaSegments != NULL);
    WMR_ASSERT(dataSegments != NULL);
    
    WMR_MEMSET(dataSegments, 0, PPN_MAX_PAGES * sizeof(struct dma_segment));
    WMR_MEMSET(metaSegments, 0, PPN_MAX_PAGES * sizeof(struct dma_segment));

    metaBounceBuffer = (UInt8 *)g_pScratchBuf;

    WMR_MEMSET(metaBounceBuffer, 0xAB, ppnCommand->lbas * fmi->valid_bytes_per_meta);

    metaSegments[0].paddr  = (UInt32)metaBounceBuffer;
    metaSegments[0].length = ppnCommand->lbas * fmi->valid_bytes_per_meta;

    WMR_PREPARE_READ_BUFFER(metaBounceBuffer, ppnCommand->lbas * fmi->valid_bytes_per_meta);

    for (page = 0; page < numPages; page++)
    {
        const PPNCommandEntry   *entry       = &ppnCommand->entry[page];
        const PPNMemoryIndex    *memIndex    = &ppnCommand->mem_index[page];
        const PPNMemoryEntry    *memoryEntry = &ppnCommand->mem_buffers[memIndex->idx];
        UInt8                   *dataPtr;
        UInt32                   dataLength;

        dataPtr    = (UInt8 *)memoryEntry->data + (memIndex->offset * fmi->logical_page_size);
        dataLength = entry->lbas * fmi->logical_page_size;

        WMR_PREPARE_READ_BUFFER(dataPtr, dataLength);

        dataSegments[page].paddr = (UInt32)dataPtr;
        dataSegments[page].length = dataLength;
    }

#if FMISS_ENABLED
    ret = fmiss_ppn_read_multi(fmi, ppnCommand, dataSegments, metaSegments);
#else
    ret = h2fmi_ppn_read_multi(fmi, ppnCommand, dataSegments, metaSegments);
#endif

    WMR_COMPLETE_READ_BUFFER(metaBounceBuffer, ppnCommand->lbas * fmi->valid_bytes_per_meta);

    metaSrc = metaBounceBuffer;
    for (page = 0; page < ppnCommand->num_pages; page++)
    {
        const PPNCommandEntry  *entry       = &ppnCommand->entry[page];
        const PPNMemoryIndex   *memIndex    = &ppnCommand->mem_index[page];
        const PPNMemoryEntry   *memoryEntry = &ppnCommand->mem_buffers[memIndex->idx];

        WMR_COMPLETE_READ_BUFFER((void *)dataSegments[page].paddr, dataSegments[page].length);
        if (memoryEntry->meta)
        {
            UInt8 *metaDest;
            
            metaDest = (UInt8 *)memoryEntry->meta + (memIndex->offset * fmi->total_bytes_per_meta);

            WMR_MEMCPY(metaDest,
                       metaSrc,
                       entry->lbas * fmi->valid_bytes_per_meta);
            metaSrc += entry->lbas * fmi->valid_bytes_per_meta;
        }
    }

    return ret;
}

static Int32 h2fmiPpnReadMultiWithSingles(h2fmi_t *fmi,
                                          PPNCommandStruct *ppnCommand)
{
    UInt32    i;
    UInt8    *metaBounceBuffer;
    Int32     status = _kIOPFMI_STATUS_SUCCESS;

    metaBounceBuffer = (UInt8 *)g_pScratchBuf;

    ppnCommand->page_status_summary = 0;

    for (i = 0; (i < ppnCommand->num_pages) && (status == _kIOPFMI_STATUS_SUCCESS); i++)
    {
        const PPNCommandEntry *entry = &ppnCommand->entry[i];
        const PPNCommandCeInfo *ceInfo = &ppnCommand->ceInfo[entry->ceIdx];
        const PPNMemoryIndex *memIndex = &ppnCommand->mem_index[i];
        const PPNMemoryEntry *memEntry = &ppnCommand->mem_buffers[memIndex->idx];
        struct dma_segment    dataSegment;
        struct dma_segment    metaSegment;
        UInt8                *dataPtr;
        UInt8                *metaPtr = NULL;

        WMR_MEMSET(lowLevelSingle, 0, sizeof(*lowLevelSingle));

        lowLevelSingle->context               = ppnCommand->context;
        lowLevelSingle->command               = PPN_COMMAND_READ;
        lowLevelSingle->options               = ppnCommand->options;
        lowLevelSingle->entry[0].addr         = entry->addr;
        lowLevelSingle->entry[0].ceIdx        = 0;
        lowLevelSingle->entry[0].lbas         = entry->lbas;
        lowLevelSingle->ceInfo[0].pages       = 1;
        lowLevelSingle->ceInfo[0].ce          = ceInfo->ce;
        lowLevelSingle->mem_index[0]          = *memIndex;

        dataPtr     = (UInt8 *)memEntry->data + (memIndex->offset * fmi->logical_page_size);
        if (memEntry->meta)
        {
            metaPtr     = (UInt8 *)memEntry->meta + (memIndex->offset * fmi->total_bytes_per_meta);
        }

        WMR_PREPARE_READ_BUFFER(dataPtr, entry->lbas * fmi->logical_page_size);
        WMR_PREPARE_READ_BUFFER(metaBounceBuffer, entry->lbas * fmi->total_bytes_per_meta);

        dataSegment.paddr  = (UInt32)dataPtr;
        dataSegment.length = entry->lbas * fmi->logical_page_size;
        metaSegment.paddr  = (UInt32)metaBounceBuffer;
        metaSegment.length = entry->lbas * fmi->valid_bytes_per_meta;

        lowLevelSingle->num_pages = 1;
        lowLevelSingle->lbas = entry->lbas;

#if FMISS_ENABLED
        status = fmiss_ppn_read_multi(fmi, lowLevelSingle, &dataSegment, &metaSegment);
#else
        status = h2fmi_ppn_read_multi(fmi, lowLevelSingle, &dataSegment, &metaSegment);
#endif
        WMR_COMPLETE_READ_BUFFER(metaBounceBuffer, entry->lbas * fmi->total_bytes_per_meta);
        WMR_COMPLETE_READ_BUFFER(dataPtr, entry->lbas * fmi->logical_page_size);

        if ( NULL != metaPtr )
        {
            WMR_MEMCPY(metaPtr, metaBounceBuffer, entry->lbas * fmi->total_bytes_per_meta);
        }

        ppnCommand->entry[i].status = lowLevelSingle->entry[0].status;
        ppnCommand->page_status_summary |= lowLevelSingle->page_status_summary;
    }

    return status;
}

#if !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM
#if (defined(AND_SUPPORT_BLOCK_STORAGE) && AND_SUPPORT_BLOCK_STORAGE)
static Int32 h2fmiPpnWriteMulti(h2fmi_t          *fmi,
                                PPNCommandStruct *ppnCommand)
{
    UInt32    i;
    UInt8    *metaBounceBuffer;
    Int32     status;

    status           = _kIOPFMI_STATUS_SUCCESS;
    metaBounceBuffer = (UInt8 *)g_pScratchBuf;

    ppnCommand->page_status_summary = 0;

    for (i = 0; (i < ppnCommand->num_pages) && (status == _kIOPFMI_STATUS_SUCCESS); i++)
    {
        const PPNCommandEntry *entry = &ppnCommand->entry[i];
        const PPNCommandCeInfo *ceInfo = &ppnCommand->ceInfo[entry->ceIdx];
        const PPNMemoryIndex *memIndex = &ppnCommand->mem_index[i];
        const PPNMemoryEntry *memEntry = &ppnCommand->mem_buffers[memIndex->idx];
        struct dma_segment    dataSegment;
        struct dma_segment    metaSegment;
        UInt8                *dataPtr;
        UInt8                *metaPtr = NULL;

        WMR_MEMSET(lowLevelSingle, 0, sizeof(*lowLevelSingle));

        lowLevelSingle->context               = ppnCommand->context;
        lowLevelSingle->command               = PPN_COMMAND_PROGRAM;
        lowLevelSingle->options               = ppnCommand->options;
        lowLevelSingle->entry[0].addr         = entry->addr;
        lowLevelSingle->entry[0].ceIdx        = 0;
        lowLevelSingle->entry[0].lbas         = entry->lbas;
        lowLevelSingle->ceInfo[0].pages       = 1;
        lowLevelSingle->ceInfo[0].ce          = ceInfo->ce;
        lowLevelSingle->mem_index[0]          = *memIndex;

        dataPtr     = (UInt8 *)memEntry->data + (memIndex->offset * fmi->logical_page_size);
        if (memEntry->meta)
        {
            metaPtr = (UInt8 *)memEntry->meta + (memIndex->offset * fmi->total_bytes_per_meta);
            WMR_MEMCPY(metaBounceBuffer, metaPtr, entry->lbas * fmi->total_bytes_per_meta);
        }
        else
        {
            WMR_MEMSET(metaBounceBuffer, 0xA5, entry->lbas * fmi->total_bytes_per_meta);
        }

        WMR_PREPARE_WRITE_BUFFER(dataPtr, entry->lbas * fmi->logical_page_size);
        WMR_PREPARE_WRITE_BUFFER(metaBounceBuffer, entry->lbas * fmi->total_bytes_per_meta);

        dataSegment.paddr  = (UInt32)dataPtr;
        dataSegment.length = entry->lbas * fmi->logical_page_size;
        metaSegment.paddr  = (UInt32)metaBounceBuffer;
        metaSegment.length = entry->lbas * fmi->valid_bytes_per_meta;

        lowLevelSingle->num_pages = 1;
        lowLevelSingle->lbas = entry->lbas;

#if FMISS_ENABLED
        status = fmiss_ppn_write_multi(fmi, lowLevelSingle, &dataSegment, &metaSegment);
#else
        status = h2fmi_ppn_write_multi(fmi, lowLevelSingle, &dataSegment, &metaSegment);
#endif
        ppnCommand->entry[i].status = lowLevelSingle->entry[0].status;
        ppnCommand->page_status_summary |= lowLevelSingle->page_status_summary;
    }

    return status;
}
#endif
#endif // !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM

Int32 h2fmiPpnReadSinglePage(UInt16 wVirtualCE,
                             UInt32 dwPpn,
                             UInt8* pabDataBuf,
                             UInt8* pabMetaBuf,
                             UInt8* pbMaxCorrectedBits,
                             UInt8* pdwaSectorStats,
                             BOOL32 bDisableWhitening)
{
    PPNCommandStruct *ppnCommand;
    h2fmi_t   *fmi = h2fmiTranslateVirtualCEToBus(wVirtualCE);
    UInt16     ce  = h2fmiTranslateVirtualCEToCe(wVirtualCE);
    Int32      ret;
    
    WMR_ASSERT(NULL != fmi);

    ppnCommand = topLevelSingle;
    WMR_MEMSET(ppnCommand, 0, sizeof(PPNCommandStruct));

    if (pbMaxCorrectedBits)
    {
        *pbMaxCorrectedBits = 0;
    }

    ppnCommand->context.device_info          = NULL;
    ppnCommand->context.handle               = NULL;
    ppnCommand->context.bus_num              = fmi->bus_id;
    ppnCommand->command                      = PPN_COMMAND_READ;
    ppnCommand->options                      = 0;
    ppnCommand->entry[0].ceIdx               = 0;
    ppnCommand->entry[0].addr.length         = fmi->bytes_per_page + fmi->total_bytes_per_meta;
    ppnCommand->entry[0].addr.column         = 0;
    ppnCommand->entry[0].addr.row            = dwPpn;
    ppnCommand->ceInfo[0].pages              = 1;
    ppnCommand->ceInfo[0].ce                 = ce;
    ppnCommand->mem_index[0].offset          = 0;
    ppnCommand->mem_index[0].idx             = 0;
    ppnCommand->entry[0].lbas                = fmi->bytes_per_page / fmi->logical_page_size;
    ppnCommand->mem_buffers[0].data          = pabDataBuf;
    ppnCommand->mem_buffers[0].meta          = pabMetaBuf;
    ppnCommand->mem_buffers[0].num_of_lbas   = 1;
    ppnCommand->num_pages                    = 1;
    ppnCommand->lbas                         = fmi->bytes_per_page / fmi->logical_page_size;

    if(pdwaSectorStats != NULL)
    {
        ppnCommand->options = PPN_OPTIONS_REPORT_HEALTH;
    }
    
    h2fmiPpnPerformCommandList(&ppnCommand, 1);

    if(pdwaSectorStats != NULL)
    {
        WMR_MEMCPY(pdwaSectorStats, pabDataBuf, fmi->sectors_per_page);
    }
    
    switch (ppnCommand->page_status_summary)
    {
        case 0x40:
            ret = FIL_SUCCESS;
            break;

        case 0x42:
            ret = FIL_SUCCESS;
            WMR_PRINT(ERROR, "Single Page PPN Read saw refresh - read status: 0x%02X, CE: %d, Page: 0x%08X\n",
                      ppnCommand->page_status_summary, wVirtualCE, dwPpn);
            break;

        case 0x44:
        case 0x43:
        case 0x45:
            ret = FIL_U_ECC_ERROR;
            break;

        case 0x49:
            ret = FIL_SUCCESS_CLEAN;
            break;

        default:
            WMR_PRINT(ALWAYS, "Single Page PPN Read failed - overall NAND status 0x%02x\n",
                      ppnCommand->page_status_summary);
            ret = FIL_CRITICAL_ERROR;
            break;
    }

    return ret;
}

Int32 h2fmiPpnReadBootpage(UInt16 wVirtualCE,
                           UInt32 dwPpn,
                           UInt8 *pabData)
{
    Int32      nRet = FIL_SUCCESS;
    UInt32     status;
    
    h2fmi_t*   fmi = h2fmiTranslateVirtualCEToBus(wVirtualCE);
    h2fmi_ce_t ce  = h2fmiTranslateVirtualCEToCe(wVirtualCE);

    WMR_ASSERT(NULL != fmi);
    WMR_ASSERT(WMR_CHECK_BUFFER(pabData, H2FMI_BOOT_BYTES_PER_PAGE));
    WMR_PREPARE_READ_BUFFER(pabData, H2FMI_BOOT_BYTES_PER_PAGE);

    status = h2fmi_ppn_read_bootpage(fmi, ce, dwPpn, pabData, NULL);

    switch (status)
    {
        case _kIOPFMI_STATUS_BLANK:
            WMR_PRINT(READ, "[FIL:LOG] Clean Page\n");
            AND_FIL_STAT(stFILStatistics.ddwReadCleanCnt++);
            nRet = FIL_SUCCESS_CLEAN;
            break;
        case _kIOPFMI_STATUS_SUCCESS:
            WMR_PRINT(READ, "[FIL:LOG] Good page\n");
            nRet = FIL_SUCCESS;
            break;
        case _kIOPFMI_STATUS_AT_LEAST_ONE_UECC:
            WMR_PRINT(ERROR, "UECC ce: %d p:0x%08x\n", (UInt32) wVirtualCE, dwPpn);
            AND_FIL_STAT(stFILStatistics.ddwReadECCErrorCnt++);
            nRet = FIL_U_ECC_ERROR;
            break;
        case _kIOPFMI_STATUS_PPN_GENERAL_ERROR:
            WMR_PRINT(ERROR, "GEB ce: %d p:0x%08x\n", (UInt32) wVirtualCE, dwPpn);
            nRet = FIL_CRITICAL_ERROR;
            h2fmi_ppn_recover_nand();
            break;
        default:
            WMR_PRINT(ERROR, "Error status: 0x%08x ce: %d p:0x%08x\n", status, (UInt32) wVirtualCE, dwPpn);
            AND_FIL_STAT(stFILStatistics.ddwReadHWErrorCnt++);
            nRet = FIL_CRITICAL_ERROR;
            h2fmi_ppn_recover_nand();
            break;
    }

    AND_FIL_STAT(stFILStatistics.ddwPagesReadCnt++);

    WMR_COMPLETE_READ_BUFFER(pabData, H2FMI_BOOT_BYTES_PER_PAGE);

    return nRet;
}

#if !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM || !H2FMI_READONLY
Int32 h2fmiPpnWriteSinglePage(UInt16 wVirtualCE,
                              UInt32 dwPpn,
                              UInt8 *pabDataBuf,
                              UInt8 *pabMetaBuf,
                              BOOL32 disableWhitening)
{
    PPNCommandStruct *ppnCommand;
    Int32             result;
    h2fmi_t          *fmi          = h2fmiTranslateVirtualCEToBus(wVirtualCE);
    UInt16            ce           = h2fmiTranslateVirtualCEToCe(wVirtualCE);

    WMR_ASSERT(NULL != fmi);
    ppnCommand = topLevelSingle;
    WMR_ASSERT(ppnCommand != NULL);

    WMR_MEMSET(ppnCommand, 0, sizeof(PPNCommandStruct));

    ppnCommand->context.device_info             = NULL;
    ppnCommand->context.handle                  = NULL;
    ppnCommand->context.bus_num                 = fmi->bus_id;
    ppnCommand->command                         = PPN_COMMAND_PROGRAM;
    ppnCommand->options                         = 0;
    ppnCommand->entry[0].ceIdx                  = 0;
    ppnCommand->entry[0].addr.row               = dwPpn;
    ppnCommand->ceInfo[0].pages                 = 1;
    ppnCommand->ceInfo[0].ce                    = ce;
    ppnCommand->mem_index[0].offset             = 0;
    ppnCommand->mem_index[0].idx                = 0;
    ppnCommand->entry[0].lbas                   = (fmi->sectors_per_page * H2FMI_BYTES_PER_SECTOR) /  fmi->logical_page_size;
    ppnCommand->mem_buffers[0].data             = pabDataBuf;
    ppnCommand->mem_buffers[0].meta             = pabMetaBuf;
    ppnCommand->mem_buffers[0].num_of_lbas      = 1;
    ppnCommand->num_pages                       = 1;

    h2fmiPpnPerformCommandList(&ppnCommand, 1);

    if (ppnCommand->page_status_summary & PPN_CONTROLLER_STATUS__PENDING_ERRORS)
    {
        result = FIL_WRITE_FAIL_ERROR;
    }
    else if (ppnCommand->page_status_summary & PPN_CONTROLLER_STATUS__GENERAL_ERROR)
    {
        result = FIL_CRITICAL_ERROR;
    }
    else
    {
        result = FIL_SUCCESS;
    }

    return result;
}
#endif // !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM || !H2FMI_READONLY

Int32 h2fmiPpnWriteBootpage(UInt16 wVirtualCE,
                            UInt32 dwPpn,
                            UInt8 *pabData)
{
    Int32      nRet;
    UInt32     status;
    h2fmi_t*   fmi = h2fmiTranslateVirtualCEToBus(wVirtualCE);
    h2fmi_ce_t ce  = h2fmiTranslateVirtualCEToCe(wVirtualCE);

    WMR_ASSERT(NULL != fmi);
    WMR_ASSERT(WMR_CHECK_BUFFER(pabData, H2FMI_BOOT_BYTES_PER_PAGE));
    WMR_PREPARE_READ_BUFFER(pabData, H2FMI_BOOT_BYTES_PER_PAGE);

    status = h2fmi_ppn_write_bootpage(fmi, ce, dwPpn, pabData);

    switch (status)
    {
        case _kIOPFMI_STATUS_PGM_ERROR:
            WMR_PRINT(ERROR, "[FIL:LOG] Error programming boot page\n");
            AND_FIL_STAT(stFILStatistics.ddwWriteNANDErrCnt++);
            nRet = FIL_WRITE_FAIL_ERROR;
            break;
        case _kIOPFMI_STATUS_SUCCESS:
            WMR_PRINT(READ, "[FIL:LOG] Wrote Good Boot page\n");
            nRet = FIL_SUCCESS;
            break;
        case _kIOPFMI_STATUS_PPN_GENERAL_ERROR:
            WMR_PRINT(ERROR, "[FIL:ERR] General Error writing bootpage\n");
            nRet = FIL_CRITICAL_ERROR;
            h2fmi_ppn_recover_nand();
            break;
        default:
            WMR_PRINT(ERROR, "[FIL:ERR] Hardware error: 0x%08x\n", status);
            AND_FIL_STAT(stFILStatistics.ddwWriteHWErrCnt++);
            nRet = FIL_CRITICAL_ERROR;
            h2fmi_ppn_recover_nand();
            break;
    }

    AND_FIL_STAT(stFILStatistics.ddwPagesWrittenCnt++);

    WMR_COMPLETE_READ_BUFFER(pabData, H2FMI_BOOT_BYTES_PER_PAGE);

    return nRet;
}

#if !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM
Int32 h2fmiPpnEraseSingleBlock(UInt16 wVirtualCE,
                               UInt32 wPbn)
{
    PPNCommandStruct *ppnCommand;
    h2fmi_t*   fmi    = h2fmiTranslateVirtualCEToBus(wVirtualCE);
    UInt16     ce     = h2fmiTranslateVirtualCEToCe(wVirtualCE);
    UInt32     wPpn   = (wPbn << fmi->ppn->device_params.page_address_bits);
    Int32      ret;

    WMR_ASSERT(NULL != fmi);
    ppnCommand = topLevelSingle;
    WMR_ASSERT(ppnCommand != NULL);

    WMR_MEMSET(ppnCommand, 0, sizeof(PPNCommandStruct));

    ppnCommand->context.device_info             = NULL;
    ppnCommand->context.handle                  = NULL;
    ppnCommand->context.bus_num                 = fmi->bus_id;
    ppnCommand->command                         = PPN_COMMAND_ERASE;
    ppnCommand->options                         = 0;
    ppnCommand->entry[0].ceIdx                  = 0;
    ppnCommand->entry[0].addr.row               = wPpn;
    ppnCommand->ceInfo[0].pages                 = 1;
    ppnCommand->ceInfo[0].ce                    = ce;
    ppnCommand->num_pages                       = 1;

    h2fmiPpnPerformCommandList(&ppnCommand, 1);

    switch(ppnCommand->page_status_summary)
    {
        case 0x40:
            ret = FIL_SUCCESS;
            break;
        case 0x45:
            ret = FIL_WRITE_FAIL_ERROR;
            break;
        default:
            ret = FIL_CRITICAL_ERROR;
            break;
    }

    return ret;
}
#endif // !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM

Int32 h2fmiPpnGetFirmwareVersion(UInt16 virtualCe, UInt8 *buffer)
{
    h2fmi_t   *fmi;
    h2fmi_ce_t ce;
    BOOL32     ret = FALSE32;

    fmi = h2fmiTranslateVirtualCEToBus(virtualCe);
    ce  = h2fmiTranslateVirtualCEToCe(virtualCe);

    if (NULL != fmi)
    {
        ret = h2fmi_ppn_get_fw_version(fmi, ce, buffer);
    }

    return (ret ? FIL_SUCCESS : FIL_CRITICAL_ERROR);
}

BOOL32 h2fmiPpnValidateFirmwareVersions(UInt8 *expected, UInt32 ce_count)
{
    UInt8  buffer[NAND_DEV_PARAM_LEN_PPN];
    UInt32 virtualCe;
    Int32  ret;
    BOOL32 mismatch = FALSE32;

    for(virtualCe = 0; virtualCe < ce_count; virtualCe++)
    {
        ret = h2fmiPpnGetFirmwareVersion(virtualCe, buffer);
        if (ret == FIL_SUCCESS)
        {
            if (WMR_MEMCMP(buffer, expected, NAND_DEV_PARAM_LEN_PPN) != 0)
            {
                WMR_PRINT(ERROR, "Bank %d reports incorrect PPN firmware version: expected \"%16s\", got \"%16s\"\n",
                          virtualCe, expected, buffer);
                mismatch = TRUE32;
            }
        }
        else
        {
                WMR_PRINT(ERROR, "Failed to retrieve PPN firmware version from bank %d\n", virtualCe);
                mismatch = TRUE32;
        }
    }

    return mismatch ? FALSE32 : TRUE32;
}

Int32 h2fmiPpnGetControllerHwId(UInt16 virtualCe, UInt8 *buffer)
{
    h2fmi_t   *fmi;
    h2fmi_ce_t ce;
    BOOL32     ret = FALSE32;

    fmi = h2fmiTranslateVirtualCEToBus(virtualCe);
    ce  = h2fmiTranslateVirtualCEToCe(virtualCe);

    if (fmi)
    {
        ret = h2fmi_ppn_get_controller_hw_id(fmi, ce, buffer);
    }

    return (ret ? FIL_SUCCESS : FIL_CRITICAL_ERROR);
}

Int32 h2fmiPpnGetManufacturerId(UInt16 virtualCe, UInt8 *buffer)
{
    h2fmi_t   *fmi;
    h2fmi_ce_t ce;
    BOOL32     ret = FALSE32;

    fmi = h2fmiTranslateVirtualCEToBus(virtualCe);
    ce  = h2fmiTranslateVirtualCEToCe(virtualCe);

    if (fmi)
    {
        ret = h2fmi_ppn_get_manufacturer_id(fmi, ce, buffer);
    }
    return (ret ? FIL_SUCCESS : FIL_CRITICAL_ERROR);
}

BOOL32 h2fmiPpnValidateManufacturerIds(UInt8 *expected, UInt32 ce_count)
{
    UInt8  buffer[NAND_DEV_PARAM_LEN_PPN];
    UInt32 virtualCe;
    Int32  ret;
    BOOL32 mismatch = FALSE32;

    for(virtualCe = 0; virtualCe < ce_count; virtualCe++)
    {
        ret = h2fmiPpnGetManufacturerId(virtualCe, buffer);
        if (ret == FIL_SUCCESS)
        {
            if (WMR_MEMCMP(buffer, expected, PPN_DEVICE_ID_LEN) != 0)
            {
                WMR_PRINT(ERROR, "Bank %d reports incorrect PPN manufacturer id: expected %02X %02X %02X %02X %02X %02X, got %02X %02X %02X %02X %02X %02X\n",
                          virtualCe,
                          expected[0], expected[1], expected[2], expected[3], expected[4], expected[5],
                          buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
                mismatch = TRUE32;
            }
        }
        else
        {
                WMR_PRINT(ERROR, "Failed to retrieve PPN manufacturer id from bank %d\n", virtualCe);
                mismatch = TRUE32;
        }
    }

    return mismatch ? FALSE32 : TRUE32;
}

Int32 h2fmiPpnGetControllerInfo(UInt16 virtualCe, UInt8 *mfg_id, UInt8 *fw_version)
{
    Int32 ret = FIL_SUCCESS;

    if (FIL_SUCCESS == h2fmiPpnGetManufacturerId(0, mfg_id))
    {
        WMR_PRINT(INIT, "PPN Manufacturer ID: %02X %02X %02X %02X %02X %02X\n",
                  mfg_id[0], mfg_id[1], mfg_id[2], mfg_id[3], mfg_id[4], mfg_id[5]);
    }
    else
    {
        WMR_PRINT(ERROR, "Retrieving PPN Manufacturer ID failed!\n");
        ret = FIL_CRITICAL_ERROR;
    }

    if (FIL_SUCCESS == h2fmiPpnGetFirmwareVersion(0, fw_version))
    {
        WMR_PRINT(INIT, "PPN Controller Version: %16.16s\n", fw_version);
    }
    else
    {
        WMR_PRINT(ERROR, "Retrieving PPN Controller Version failed!\n");
        ret = FIL_CRITICAL_ERROR;
    }

    return ret;
}

#define MAX_ATTEMPTS (3)
Int32 h2fmiPpnUpdateFw(UInt16 wVirtualCE, UInt8 *fw, UInt32 fw_length, UInt8 *fwa, UInt32 fwa_length)
{
    h2fmi_t           *fmi;
    h2fmi_ce_t         ce;
    struct dma_segment sgl;
    int                i;
    Int32              status;
    Int32              ret_val = FIL_CRITICAL_ERROR;

    fmi = h2fmiTranslateVirtualCEToBus(wVirtualCE);
    ce  = h2fmiTranslateVirtualCEToCe(wVirtualCE);

    if (NULL == fmi)
    {
        return FIL_UNSUPPORTED_ERROR;
    }

    for( i = 0; (i < MAX_ATTEMPTS) && (ret_val != FIL_SUCCESS); i++)
    {
        if (NULL != fw)
        {
            WMR_PRINT(ALWAYS, "Updating fw on fmi %d, ce %d\n", fmi->bus_id, ce);
            sgl.paddr  = (UInt32)fw;
            sgl.length = fw_length;
        
            status = h2fmi_ppn_fw_update(fmi, ce, &sgl, fw_length, ppnFwTypeFw);
            if (status == FIL_SUCCESS)
            {
                ret_val = FIL_SUCCESS;
            }
            else if (status == FIL_UNSUPPORTED_ERROR)
            {
                return status;
            }
            else
            {
                ret_val = status;
                WMR_PRINT(ERROR, "Firmware Update Failed: attempt %d/%d\n", i+1, MAX_ATTEMPTS);
                continue;
            }
        }
        if (NULL != fwa)
        {
            WMR_PRINT(ALWAYS, "Updating fw args on fmi %d, ce %d\n", fmi->bus_id, ce);
            sgl.paddr  = (UInt32)fwa;
            sgl.length = fwa_length;
        
            status = h2fmi_ppn_fw_update(fmi, ce, &sgl, fwa_length, ppnFwTypeFwa);
            if (status == FIL_SUCCESS)
            {
                ret_val = FIL_SUCCESS;
            }
            else if (status == FIL_UNSUPPORTED_ERROR)
            {
                return status;
            }
            else
            {
                ret_val = status;
                WMR_PRINT(ERROR, "Firmware Arguments Update Failed: attempt %d/%d\n", i+1, MAX_ATTEMPTS);
                continue;
            }
        }
    }

    return ret_val;
}

void h2fmi_ppn_recover_nand(void)
{
    h2fmi_nand_reset_all(&g_fmi0);
    if(g_fmi1.num_of_ce)
    {
        h2fmi_nand_reset_all(&g_fmi1);
    }

#if SUPPORT_TOGGLE_NAND
    if (g_fmi0.is_toggle_system)
    {
        g_fmi0.is_toggle = FALSE32;
        h2fmi_reset(&g_fmi0);

        if (g_fmi1.num_of_ce)
        {
            WMR_ASSERT(g_fmi1.is_toggle_system);
            g_fmi1.is_toggle = FALSE32;
            h2fmi_reset(&g_fmi1);
        }
    }
#endif

    h2fmi_ppn_post_rst_pre_pwrstate_operations(&g_fmi0);
    if (g_fmi1.num_of_ce)
    {
        h2fmi_ppn_post_rst_pre_pwrstate_operations(&g_fmi1);
    }

    h2fmi_ppn_set_channel_power_state(&g_fmi0, 
#if SUPPORT_TOGGLE_NAND
                   (g_fmi0.is_toggle_system) ? PPN_PS_TRANS_LOW_POWER_TO_DDR : 
#endif
                   PPN_PS_TRANS_LOW_POWER_TO_ASYNC);

    if (g_fmi1.num_of_ce)
    {
        h2fmi_ppn_set_channel_power_state(&g_fmi1, 
#if SUPPORT_TOGGLE_NAND
                       (g_fmi1.is_toggle_system) ? PPN_PS_TRANS_LOW_POWER_TO_DDR : 
#endif
                       PPN_PS_TRANS_LOW_POWER_TO_ASYNC);
    }

#if SUPPORT_TOGGLE_NAND
    if (g_fmi0.is_toggle_system)
    {
        g_fmi0.is_toggle = TRUE32;
        h2fmi_reset(&g_fmi0);
        if(g_fmi1.num_of_ce)
        {
            WMR_ASSERT(g_fmi1.is_toggle_system);
            g_fmi1.is_toggle = TRUE32;
            h2fmi_reset(&g_fmi1);
        }
    }
#endif
    g_fmi0.ppn->general_error = FALSE32;
    g_fmi1.ppn->general_error = FALSE32;

#if H2FMI_PPN_VERIFY_SET_FEATURES
    if(g_fmi0.num_of_ce)
    {
        h2fmi_ppn_verify_feature_shadow(&g_fmi0);
    }
    if(g_fmi1.num_of_ce)
    {
        h2fmi_ppn_verify_feature_shadow(&g_fmi1);
    }
#endif // H2FMI_PPN_VERIFY_SET_FEATURES
}

Int32 h2fmiDmaDebugData(UInt8 virCE, UInt32 page, UInt32 pageCount, struct dma_segment *sgl, BOOL32 waitFortGDD)
{
    h2fmi_t                    *fmi;
    h2fmi_ce_t                  ce;

    WMR_ASSERT(sgl != NULL);

    fmi = h2fmiTranslateVirtualCEToBus(virCE);
    ce  = h2fmiTranslateVirtualCEToCe(virCE);

    if (!fmi)
    {
        return FIL_UNSUPPORTED_ERROR;
    }

    h2fmi_ppn_dma_debug_data_payload(fmi, ce, page, pageCount, sgl, waitFortGDD);

    return FIL_SUCCESS;
}

Int32 h2fmiVthSweepSetup(UInt8 virCE, UInt32 blk, h2fmi_ppn_failure_info_t *failInfo, UInt32 *totalDataToRead)
{
    h2fmi_t        *fmi;
    h2fmi_ce_t      ce;
    UInt32          rowAddr;
    LowFuncTbl     *fil = FIL_GetFuncTbl();
    const UInt32    block_shift = fil->GetDeviceInfo(AND_DEVINFO_BITS_PER_PAGE_ADDRESS);

    if (!failInfo || !totalDataToRead)
    {
        return FIL_CRITICAL_ERROR;
    }

    fmi = h2fmiTranslateVirtualCEToBus(virCE);
    ce  = h2fmiTranslateVirtualCEToCe(virCE);

    if (!fmi)
    {
        return FIL_UNSUPPORTED_ERROR;
    }

    rowAddr = blk << block_shift;
    h2fmi_ppn_set_debug_data(fmi, ce, PPN_SET_DEBUG_DATA__EMB_SWEEP_ADDRESS, (UInt8*)&rowAddr, sizeof(rowAddr));

    if (!fmi->ppn->general_error)
    {
        WMR_PRINT(ERROR, "No GEB during Vth Setup!\n");
        return FIL_CRITICAL_ERROR;
    }

    if (h2fmi_ppn_get_general_error_info(fmi, ce, failInfo, totalDataToRead, NULL))
    {
        return FIL_SUCCESS;
    }
    else
    {
        return FIL_CRITICAL_ERROR;
    }
}

#endif //SUPPORT_PPN
