/*****************************************************************************/
/*                                                                           */
/* COMPONENT   : Rainbow				                                     */
/* MODULE      : Virtual Flash Layer                                         */
/* NAME    	   : VFL interface				                                 */
/* FILE        : VFLinterface.c                                              */
/* PURPOSE 	   : This file contains the exported routine for interfacing with*/
/*           	 the upper layer of VFL.                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*          COPYRIGHT 2003-2005 SAMSUNG ELECTRONICS CO., LTD.                */
/*                          ALL RIGHTS RESERVED                              */
/*                                                                           */
/*   Permission is hereby granted to licensees of Samsung Electronics        */
/*   Co., Ltd. products to use or abstract this computer program for the     */
/*   sole purpose of implementing a product based on Samsung                 */
/*   Electronics Co., Ltd. products. No other rights to reproduce, use,      */
/*   or disseminate this computer program, whether in part or in whole,      */
/*   are granted.                                                            */
/*                                                                           */
/*   Samsung Electronics Co., Ltd. makes no representation or warranties     */
/*   with respect to the performance of this computer program, and           */
/*   specifically disclaims any responsibility for any damages,              */
/*   special or consequential, connected with the use of this program.       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* REVISION HISTORY                                                          */
/*                                                                           */
/* 	 16-SEP-2003 [SJ Myoung]	: multi-block read speed up 	             */
/*						   flash_read(,,,UInt8 sector_offset) add argument	 */
/* 	 22-SEP-2003 [JH Kim]		: buf_get() bug-fix (if no free buffer)		 */
/* 	 29-SEP-2003 [HM Jung] 		: code size reduction 						 */
/*								  check init bad from PAGE_CLEAN			 */
/* 	 03-NOV-2003 [JH Kim]		: replace_bad_block() - add loop when erase	 */
/*								  if erase fail								 */
/* 	 12-JAN-2004 [JH Kim]		: fix by sequential order					 */
/* 	 26-JAN-2004 [JH Kim] 		: add buffer usage trace code				 */
/* 	 13-MAR-2005 [Jaesung Jung]	: duet port									 */
/* 	 21-MAR-2005 [Jaesung Jung] : move ASSERT DEFINE to copy_data.h			 */
/* 	 21-MAR-2005 [Jaesung Jung] : remove pattern_test code					 */
/*	 28-MAR-2005 [Jaesung Jung] : add logical to physical API				 */
/*	 07-APR-2005 [Jaesung Jung] : MLC support (2way)						 */
/*	 08-APR-2005 [Jaesung Jung] : add context checksum check				 */
/*	 11-APR-2005 [Jaesung Jung] : optimize PAGE_CLEAN check					 */
/*   18-JUL-2005 [Jaesung Jung] : reorganize code							 */
/*	 06-SEP-2005 [Jaesung Jung] : bug-fix from code inspection				 */
/*   22-NOV-2005 [Yangsup Lee ] : remove checksum code						 */
/*   24-JAN-2006 [Yangsup Lee ] : change sector offset to sector bitmap      */
/*   24-JAN-2006 [Yangsup Lee ] : support un-pair bad block management       */
/*   31-MAR-2006 [Yangsup Lee ] : support ftl meta block wear leveling       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Header file inclusions                                                    */
/*****************************************************************************/
#include "WMRConfig.h"
#include "ANDTypes.h"
#include "WMROAM.h"
#include "VFLBuffer.h"
#include "VFLTypes.h"
#include "FPart.h"
#include "VFL.h"
#include "FIL.h"
#if AND_SUPPORT_LEGACY_VFL
/*****************************************************************************/
/* Debug Print #defines                                                      */
/*****************************************************************************/

#if (defined(AND_SIMULATOR) && AND_SIMULATOR)
#define     VFL_ERR_PRINT(x)            { WMR_RTL_PRINT(x); WMR_PANIC(); }
#else
#define     VFL_ERR_PRINT(x)            WMR_RTL_PRINT(x)
#endif
#define     VFL_WRN_PRINT(x)            WMR_RTL_PRINT(x)

#if defined (VFL_LOG_MSG_ON)
#define     VFL_LOG_PRINT(x)            WMR_DBG_PRINT(x)
#else
#define     VFL_LOG_PRINT(x)
#endif

#if defined (VFL_INF_MSG_ON)
#define     VFL_INF_PRINT(x)            WMR_DBG_PRINT(x)
#else
#define     VFL_INF_PRINT(x)
#endif

/*****************************************************************************/
/* Constants					                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Static variables definitions                                              */
/*****************************************************************************/
#if (defined(AND_SIMULATOR) && AND_SIMULATOR)
VFLMeta     *pstVFLMeta = NULL;
#else
static VFLMeta     *pstVFLMeta = NULL;
#endif
static UInt32 * pstadwScatteredReadPpn = NULL;
static UInt16 * pstawScatteredReadBank = NULL;

static LowFuncTbl   *pFuncTbl = NULL;
static UInt8       *pstBBTArea = NULL;
static VFLWMRDeviceInfo stVFLDeviceInfo;
static FPartFunctions stFPartFunctions;

static UInt32 gdwMaxGlobalAge;
#ifdef AND_COLLECT_STATISTICS
static VFLStatistics stVFLStatistics;
#endif
/*****************************************************************************/
/* Definitions                                                               */
/*****************************************************************************/
#define     GET_VFLCxt(v)               (&(pstVFLMeta + (v))->stVFLCxt)
#define     GET_AsyncOp(v)              (pstAsyncOp + (v))
#define     GET_Ppn(Pbn, POffset)       ((Pbn) * PAGES_PER_BLOCK + (POffset)) /* transalte block + pages offset to page address*/
#define     GET_Pbn(Ppn)                ((UInt16)((UInt32)(Ppn) / PAGES_PER_BLOCK))
/*****************************************************************************/
/* VFL local function prototypes                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Static function prototypes                                                */
/*****************************************************************************/
static void         _GetVFLSpare(UInt32 *pAge, UInt8  *pStatus,
                                 UInt8  *pbType, UInt8  *pSBuf);
#ifndef AND_READONLY
static void         _SetVFLSpare(UInt32 *pAge, UInt8  *pSBuf);
static BOOL32       _StoreVFLCxt(UInt16 wBank);

static BOOL32       _GetReservedBlk(UInt16 wBank, UInt16 wPbn,
                                    UInt16 *pwIdx);
static BOOL32       _ReplaceBadBlk(UInt16 wBank, UInt16 wSrcPbn,
                                   UInt16 *pwDesPbn);
#endif
static BOOL32       _LoadVFLCxt(UInt16 wBank, UInt8  *pabBBT);
static void         _RemapBadBlk(UInt16 wBank, UInt16 wSrcPbn,
                                 UInt16 *pwDesPbn);
static void         _GetPhysicalAddr(UInt32 dwVpn, UInt16 *pwBank,
                                     UInt16 *pwPbn, UInt16 *pwPOffset);

#ifndef AND_READONLY
static BOOL32 _AddBlockToScrubList(UInt16 wBank, UInt16 wVbnToScrub);
static BOOL32 _RemoveBlockFromScrubList(UInt16 wBank, UInt16 wVbnToScrub);
static BOOL32 _IsBlockInScrubList(UInt16 wBank, UInt16 wVbnToScrub);
#endif

#ifndef AND_READONLY
static Int32    VFL_Format(UInt32 dwBootAreaSize, UInt32 dwOptions);
static Int32    VFL_Write(UInt32 nVpn, Buffer *pBuf, BOOL32 boolReplaceBlockOnFail, BOOL32 bDisableWhitening);
static Int32    VFL_Erase(UInt16 wVbn, BOOL32 bReplaceOnFail);
static Int32    VFL_ChangeFTLCxtVbn(UInt16 *aFTLCxtVbn);
static BOOL32   VFL_WriteMultiplePagesInVb(UInt32 dwVpn, UInt16 wNumPagesToWrite, UInt8 * pbaData, UInt8 * pbaSpare, BOOL32 boolReplaceBlockOnFail, BOOL32 bDisableWhitening);
#endif // ! AND_READONLY
static Int32    VFL_Init(FPartFunctions * pFPartFunctions);
static Int32    VFL_Open(UInt32 dwBootAreaSize, UInt32 minor_ver, UInt32 dwOptions);
static void     VFL_Close(void);
static BOOL32   VFL_ReadMultiplePagesInVb(UInt16 wVbn, UInt16 wStartPOffset, UInt16 wNumPagesToRead, UInt8 * pbaData, UInt8 * pbaSpare, BOOL32 * pboolNeedRefresh, UInt8 * pabSectorStat, BOOL32 bDisableWhitening);
static BOOL32   VFL_ReadScatteredPagesInVb(UInt32 * padwVpn, UInt16 wNumPagesToRead, UInt8 * pbaData, UInt8 * pbaSpare, BOOL32 * pboolNeedRefresh, UInt8 * pabSectorStat, BOOL32 bDisableWhitening, Int32 *actualStatus);
static Int32    VFL_Read(UInt32 nVpn, Buffer *pBuf, BOOL32 bCleanCheck, BOOL32 bMarkECCForScrub, BOOL32 * pboolNeedRefresh, UInt8 * pabSectorStat, BOOL32 bDisableWhitening);
static UInt16*  VFL_GetFTLCxtVbn(void);
//static BOOL32	VFL_GetAddress			(ANDAddressStruct * pANDAddressStruct);
static BOOL32   VFL_GetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 * pdwStructSize);
static BOOL32   VFL_SetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 dwStructSize);
static UInt32   VFL_GetDeviceInfo(UInt32 dwParamType);

#define BLOCK_IS_BAD                        0
#define BLOCK_IS_GOOD                       1

#define WMR_NUM_COPIES_OF_DEVICE_INFO_INDEX         (8)
#define WMR_NUM_OF_ERASE_TRIALS_FOR_INDEX_BLOCKS    (4)
#define WMR_MAX_ERRORS_WRITING_INDEX_BLOCKS         (4)

#define WMR_NUM_OF_ERASE_TRIALS_FOR_DATA_BLOCKS     (3)

#define VFL_NUM_OF_VFL_CXT_COPIES                   (8)
#define VFL_MAX_NUM_OF_VFL_CXT_ERRORS               (3)

#define VFL_INVALID_INFO_INDEX                      (0xFFFF)

/*

    NAME
        MARK_BLOCK_AS_BAD
    DESCRIPTION
        This define marks block as bad in the bbt.
    PARAMETERS
        pBBT 	 [IN]	bbt buffer
        idx		 [IN]	block index
    RETURN VALUES
        none
    NOTES

 */

#define MARK_BLOCK_AS_BAD(pBBT, idx) \
    { \
        UInt16 dByteLocation; \
        UInt8 bBit; \
        dByteLocation = ((UInt16)(idx)) >> 3; \
        bBit = (UInt8)(~(1 << (((UInt8)(idx)) & 0x07))); \
        ((UInt8 *)(pBBT))[dByteLocation] &= bBit; \
    }

/*

    NAME
        MARK_BLOCK_AS_GOOD
    DESCRIPTION
        This define marks block as good in the bbt.
    PARAMETERS
        pBBT 	 [IN]	bbt buffer
        idx		 [IN]	block index
    RETURN VALUES
        none
    NOTES

 */
#define MARK_BLOCK_AS_GOOD(pBBT, idx) \
    { \
        UInt16 dByteLocation; \
        UInt8 bBit; \
        dByteLocation = ((UInt16)(idx)) >> 3; \
        bBit = (1 << (((UInt8)(idx)) & 0x07)); \
        ((UInt8 *)(pBBT))[dByteLocation] |= bBit; \
    }

static void _NANDCheckSum(UInt32 * pbaData, UInt32 dwDataSize, UInt32 * pdwCheckSum1, UInt32 * pdwCheckSum2)
{
    UInt32 dwIdx;
    UInt32 dwXorsum;
    UInt32 dwChecksum;

    dwChecksum = 0;
    dwXorsum = 0;
    for (dwIdx = 0; dwIdx < (dwDataSize / 4); dwIdx++)
    {
        dwChecksum += pbaData[dwIdx];
        dwXorsum ^= pbaData[dwIdx];
    }
    /* avoid passing when vflcxt is zeroed */
    *pdwCheckSum1 = dwChecksum + 0xAABBCCDD;
    *pdwCheckSum2 = dwXorsum ^ 0xAABBCCDD;
}

static BOOL32 _CheckVFLCxtDataIsUntouched(UInt16 wBank)
{
    UInt32 dwTempCheckSum;
    UInt32 dwTempXor;
    static UInt32 dwCount = 0;

    dwCount++;
    _NANDCheckSum((UInt32*)&pstVFLMeta[wBank], (sizeof(VFLMeta) - (2 * sizeof(UInt32))), &dwTempCheckSum, &dwTempXor);
    if (!(dwTempCheckSum == pstVFLMeta[wBank].dwCheckSum) && (dwTempXor == pstVFLMeta[wBank].dwXorSum))
    {
        return FALSE32;
    }
    return TRUE32;
}
static void _UpdateVFLCxtChecksum(UInt16 wBank)
{
    _NANDCheckSum((UInt32*)&pstVFLMeta[wBank], (sizeof(VFLMeta) - (2 * sizeof(UInt32))), &pstVFLMeta[wBank].dwCheckSum, &pstVFLMeta[wBank].dwXorSum);
}

typedef struct
{
    void (*UpdateChecksum)(UInt16 wBank);
    BOOL32 (*VerifyChecksum)(UInt16 wBank);
} ChecksumFuncStruct;

static ChecksumFuncStruct vflChecksumFunc;

#define RECALC_VFL_CXT_CHECK_SUM(bank)  vflChecksumFunc.UpdateChecksum(bank)

#define CHECK_VFL_CXT_CHECK_SUM(bank)   \
    { \
        if (vflChecksumFunc.VerifyChecksum(bank) == FALSE32) { \
            WMR_ASSERT(0); } \
    }

#define CHECK_VFL_CXT_CHECK_SUM_NO_ASSERT(bank) vflChecksumFunc.VerifyChecksum(bank)

#ifndef AND_READONLY
static BOOL32
_AddBlockToScrubList(UInt16 wBank, UInt16 wVbnToScrub)
{
    UInt16 wScrubIdx = WMR_MAX_RESERVED_SIZE;
    VFLCxt * pVFLCxt = GET_VFLCxt(wBank);

    if (_IsBlockInScrubList(wBank, wVbnToScrub) == TRUE32)
    {
        return TRUE32;
    }

    /* check if the list is not empty */
    if (pVFLCxt->wBadMapTableScrubIdx != WMR_MAX_RESERVED_SIZE)
    {
        /* check that the block in not allready in the list */
        while (--wScrubIdx && (pVFLCxt->wBadMapTableScrubIdx <= wScrubIdx))
        {
            if (pVFLCxt->aBadMapTable[wScrubIdx] == wVbnToScrub)
            {
                return TRUE32;
            }
        }
    }
    /* make sure we do not get close to the wBadMapTableMaxIdx */
    if (pVFLCxt->wBadMapTableScrubIdx == (pVFLCxt->wBadMapTableMaxIdx + 10))
    {
        return FALSE32;
    }
    CHECK_VFL_CXT_CHECK_SUM(wBank);
    pVFLCxt->wBadMapTableScrubIdx--;
    pVFLCxt->aBadMapTable[pVFLCxt->wBadMapTableScrubIdx] = wVbnToScrub;
    RECALC_VFL_CXT_CHECK_SUM(wBank);
    return _StoreVFLCxt(wBank);
}

static BOOL32
_RemoveBlockFromScrubList(UInt16 wBank, UInt16 wVbnToScrub)
{
    UInt16 wScrubIdx = WMR_MAX_RESERVED_SIZE;
    VFLCxt * pVFLCxt = GET_VFLCxt(wBank);

    /* check if the list is not empty */
    if (pVFLCxt->wBadMapTableScrubIdx != WMR_MAX_RESERVED_SIZE)
    {
        /* check that the block in not allready in the list */
        while (--wScrubIdx && (pVFLCxt->wBadMapTableScrubIdx <= wScrubIdx))
        {
            if (pVFLCxt->aBadMapTable[wScrubIdx] == wVbnToScrub)
            {
                CHECK_VFL_CXT_CHECK_SUM(wBank);
                pVFLCxt->aBadMapTable[wScrubIdx] = 0;
                /* we found the block - if it is in the middle of the list replace it with the bottom one */
                if ((wScrubIdx != pVFLCxt->wBadMapTableScrubIdx) && (pVFLCxt->wBadMapTableScrubIdx != (WMR_MAX_RESERVED_SIZE - 1)))
                {
                    pVFLCxt->aBadMapTable[wScrubIdx] = pVFLCxt->aBadMapTable[pVFLCxt->wBadMapTableScrubIdx];
                }
                pVFLCxt->wBadMapTableScrubIdx++;
                RECALC_VFL_CXT_CHECK_SUM(wBank);
                return _StoreVFLCxt(wBank);
            }
        }
    }
    return FALSE32;
}

static BOOL32
_IsBlockInScrubList(UInt16 wBank, UInt16 wVbnToScrub)
{
    UInt16 wScrubIdx = WMR_MAX_RESERVED_SIZE;
    VFLCxt * pVFLCxt = GET_VFLCxt(wBank);

    /* check if the list is not empty */
    if (pVFLCxt->wBadMapTableScrubIdx != WMR_MAX_RESERVED_SIZE)
    {
        /* check that the block in not allready in the list */
        while (--wScrubIdx && (pVFLCxt->wBadMapTableScrubIdx <= wScrubIdx))
        {
            if (pVFLCxt->aBadMapTable[wScrubIdx] == wVbnToScrub)
            {
                return TRUE32;
            }
        }
    }
    return FALSE32;
}
#endif // AND_READONLY

static BOOL32 _isBlockGood(UInt8 * pbaBBT, UInt16 wIdx)
{
    UInt16 dByteLocation;
    UInt8 bBit;

    dByteLocation = ((UInt16)wIdx) >> 3;
    bBit = (1 << (((UInt8)wIdx) & 0x07));
    if (pbaBBT[dByteLocation] & bBit)
    {
        return TRUE32;
    }
    return FALSE32;
}

BOOL32 _isBadMarkTableMarkGood(UInt8 * pbaBadMark, UInt16 wPbn)
{
    UInt32 dwByteOffset, dwBitOffset;
    UInt8 bValue;

    dwByteOffset = wPbn / BAD_MARK_COMPRESS_SIZE / 8;
    dwBitOffset = 7 - (wPbn / BAD_MARK_COMPRESS_SIZE) % 8;
    bValue = pbaBadMark[dwByteOffset];

    if (!((bValue >> dwBitOffset) & 0x1))
    {
        return FALSE32;
    }
    return TRUE32;
}

void _SetBadMarkTable(UInt8 * pbaBadMark, UInt16 wPbn, BOOL32 boolGood)
{
    UInt32 dwByteOffset, dwBitOffset;

    dwByteOffset = wPbn / BAD_MARK_COMPRESS_SIZE / 8;
    dwBitOffset = 7 - (wPbn / BAD_MARK_COMPRESS_SIZE) % 8;

    if (boolGood)
    {
        pbaBadMark[dwByteOffset] |= (1 << dwBitOffset);
    }
    else
    {
        pbaBadMark[dwByteOffset] &= ~(1 << dwBitOffset);
    }
}

/*****************************************************************************/
/* Code Implementation                                                       */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _GetVFLSpare                                                         */
/* DESCRIPTION                                                               */
/*      This function returns VFL context info from spare area.			     */
/* PARAMETERS                                                                */
/*      *pdwAge  [OUT] 	the pointer of context age                           */
/*      *pStatus [OUT] 	the pointer of status mark                           */
/*      *pSBu    [IN] 	the pointer of spare buffer                          */
/* RETURN VALUES                                                             */
/*		none																 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static void
_GetVFLSpare(UInt32 *pdwAge, UInt8 *pStatus, UInt8 *pbType, UInt8 *pSBuf)
{
    VFLSpare *pSpare = (VFLSpare *)pSBuf;

    if (pdwAge != NULL)
    {
        *pdwAge = pSpare->dwCxtAge;
    }
    if (pbType != NULL)
    {
        *pbType = pSpare->bSpareType;
    }
    if (pStatus != NULL)
    {
        *pStatus = pSpare->cStatusMark;
    }
}

/*

    NAME
        _SetVFLSpare
    DESCRIPTION
        This function stores VFL context info on spare area.
    PARAMETERS
 *pdwAge  [OUT] 	the pointer of context age
 *pSBuf   [IN] 	the pointer of spare buffer
    RETURN VALUES
        none
    NOTES

 */
#ifndef AND_READONLY
static void
_SetVFLSpare(UInt32 *pdwAge, UInt8 *pSBuf)
{
    VFLSpare *pSpare = (VFLSpare *)pSBuf;

    if (pdwAge != NULL)
    {
        pSpare->dwCxtAge = *pdwAge;
    }
    /* the status mark of MLC is always valid */
    pSpare->cStatusMark = PAGE_VALID;
    pSpare->bSpareType = VFL_SPARE_TYPE_CXT;
}

/* writes the VFLCxt 8 times to and verifies it was written properly at least 5 times */
static BOOL32 _WriteVFLCxtToFlash(UInt16 wBank)
{
    UInt16 wPageIdx, wNumOfErrors;
    UInt32 dwCxtAge;
    Buffer  *pBuf = NULL;
    VFLCxt  *pVFLCxt;
    BOOL32 boolWriteVFLCxtSuccess = FALSE32;
    UInt16 wCurrCxtPOffset;

    VFL_LOG_PRINT((TEXT("[VFL:INF] ++_WriteVFLCxtToFlash(wBank:%d)\n"), wBank));

    /* check current state			*/
    if (wBank >= BANKS_TOTAL)
    {
        return FALSE32;
    }

    pVFLCxt = GET_VFLCxt(wBank);
    pBuf = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBuf == NULL)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] error BUF_Get(line:%d)\n"), __LINE__));
        return FALSE32;
    }
    CHECK_VFL_CXT_CHECK_SUM(wBank);
    dwCxtAge = --(pVFLCxt->dwCxtAge);
    pVFLCxt->dwGlobalCxtAge = ++gdwMaxGlobalAge;
    wCurrCxtPOffset = pVFLCxt->wNextCxtPOffset;
    pVFLCxt->wNextCxtPOffset += VFL_NUM_OF_VFL_CXT_COPIES;
    RECALC_VFL_CXT_CHECK_SUM(wBank);
    WMR_MEMCPY(pBuf->pData, &pstVFLMeta[wBank], sizeof(VFLMeta));

    /* writing the information */
    for (wPageIdx = 0; wPageIdx < VFL_NUM_OF_VFL_CXT_COPIES; wPageIdx++)
    {
        WMR_MEMSET(pBuf->pSpare, 0xFF, BYTES_PER_METADATA_RAW);
        _SetVFLSpare(&dwCxtAge, pBuf->pSpare);

        /* we are not checking the result of the operation here - we will check the content below using read */
        if (pFuncTbl->Write(wBank, GET_Ppn(pVFLCxt->awInfoBlk[pVFLCxt->wCxtLocation],
                                           (wCurrCxtPOffset + wPageIdx)), pBuf->pData, pBuf->pSpare, FALSE32) != FIL_SUCCESS)
        {
            VFL_WRN_PRINT((TEXT("[VFL:WRN] _WriteVFLCxtToFlash failed write (0x%X, 0x%X) (line:%d)\n"),
                           wBank, GET_Ppn(pVFLCxt->awInfoBlk[pVFLCxt->wCxtLocation], (wCurrCxtPOffset + wPageIdx)), __LINE__));
            BUF_Release(pBuf);
            return FALSE32;
        }
    }

    wNumOfErrors = 0;

    for (wPageIdx = 0; wPageIdx < VFL_NUM_OF_VFL_CXT_COPIES; wPageIdx++)
    {
        Int32 nFILRet;
        nFILRet = pFuncTbl->ReadWithECC(wBank, GET_Ppn(pVFLCxt->awInfoBlk[pVFLCxt->wCxtLocation],
                                                       (wCurrCxtPOffset + wPageIdx)), pBuf->pData, pBuf->pSpare, NULL, NULL, FALSE32);
        if (nFILRet == FIL_SUCCESS)
        {
            UInt8 bStatusMark, bSpareType;
            _GetVFLSpare(&dwCxtAge, &bStatusMark, &bSpareType, pBuf->pSpare);
            if ((WMR_MEMCMP(pBuf->pData, pVFLCxt, sizeof(VFLCxt)) != 0) || (dwCxtAge != pVFLCxt->dwCxtAge) || (bStatusMark != PAGE_VALID) || (bSpareType != VFL_SPARE_TYPE_CXT))
            {
                wNumOfErrors++;
            }
        }
        else
        {
            wNumOfErrors++;
        }
    }

    if (wNumOfErrors <= VFL_MAX_NUM_OF_VFL_CXT_ERRORS)
    {
        boolWriteVFLCxtSuccess = TRUE32;
    }
    else
    {
        VFL_ERR_PRINT((TEXT("[VFL:WRN] Fail writing VFLCxt(line:%d) bank %d block %d page %d\n"), __LINE__, wBank, pVFLCxt->wCxtLocation, wCurrCxtPOffset));
    }

    BUF_Release(pBuf);
    VFL_LOG_PRINT((TEXT("[VFL: IN] --_WriteVFLCxtToFlash(wBank:%d)\n"), wBank));
    return boolWriteVFLCxtSuccess;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _StoreVFLCxt                                                         */
/* DESCRIPTION                                                               */
/*      This function stores VFL context on VFL area.					     */
/* PARAMETERS                                                                */
/*      wBank   bank number                                                  */
/* RETURN VALUES                                                             */
/*		TRUE32																 */
/*				_StoreVFLCxt is completed									 */
/*		FALSE32																 */
/*				_StoreVFLCxt is failed										 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/

/*
    Changes in _StoreVFLCxt:
    - store several copies of the same context information (8 copies of the same data).
    - after each write, sync and verify the content - fail the current block if 4 out of the 8 do not match or do not pass ECC.
    -
 */
static BOOL32
_StoreVFLCxt(UInt16 wBank)
{
    VFLCxt  *pVFLCxt;
    BOOL32 boolWriteVFLCxtSuccess = FALSE32;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++_StoreVFLCxt(wBank:%d)\n"), wBank));

    /* check current state			*/
    if (wBank >= BANKS_TOTAL)
    {
        return FALSE32;
    }

    pVFLCxt = GET_VFLCxt(wBank);

    /* check if we still have space in the current block */
    if ((pVFLCxt->wNextCxtPOffset + VFL_NUM_OF_VFL_CXT_COPIES) <= PAGES_PER_BLOCK)
    {
        // -> we have space in the current block

        /* write copies in the current block */
        boolWriteVFLCxtSuccess = _WriteVFLCxtToFlash(wBank);
    }

    if (boolWriteVFLCxtSuccess == FALSE32)
    {
        // -> we do NOT have space in the current block or write operation in the current block failed
        UInt16 wVFLCxtBlkIdx, wOriginalCxtLocation;
        /* search for a valid erase unit we can write to */
        wOriginalCxtLocation = pVFLCxt->wCxtLocation;
        wVFLCxtBlkIdx = (pVFLCxt->wCxtLocation + 1) % VFL_INFO_SECTION_SIZE;
        while (wVFLCxtBlkIdx != wOriginalCxtLocation)
        {
            UInt16 wEraseCnt;

            /* check if this index point to a valid block */
            if (pVFLCxt->awInfoBlk[wVFLCxtBlkIdx] == VFL_INVALID_INFO_INDEX)
            {
                wVFLCxtBlkIdx = (wVFLCxtBlkIdx + 1) % VFL_INFO_SECTION_SIZE;
                continue;
            }

            /* block is valid try to erase it - up to x times */
            for (wEraseCnt = 0; wEraseCnt < WMR_NUM_OF_ERASE_TRIALS_FOR_INDEX_BLOCKS; wEraseCnt++)
            {
                Int32 nFILRet;
                nFILRet = pFuncTbl->Erase(wBank, pVFLCxt->awInfoBlk[wVFLCxtBlkIdx]);
                if (nFILRet == FIL_SUCCESS)
                {
                    break;
                }
            }

            if (wEraseCnt == WMR_NUM_OF_ERASE_TRIALS_FOR_INDEX_BLOCKS)
            {
                // pVFLCxt->awInfoBlk[wVFLCxtBlkIdx] = VFL_INVALID_INFO_INDEX;
                VFL_ERR_PRINT((TEXT("[VFL:ERR] Fail erasing VFLBlock bank %d block 0x%X (line:%d)\n"), wBank, pVFLCxt->awInfoBlk[wVFLCxtBlkIdx], __LINE__));
                wVFLCxtBlkIdx = (wVFLCxtBlkIdx + 1) % VFL_INFO_SECTION_SIZE;
                continue;
            }

            CHECK_VFL_CXT_CHECK_SUM(wBank);
            pVFLCxt->wCxtLocation = wVFLCxtBlkIdx;
            pVFLCxt->wNextCxtPOffset = 0;
            RECALC_VFL_CXT_CHECK_SUM(wBank);

            boolWriteVFLCxtSuccess = _WriteVFLCxtToFlash(wBank);
            if (boolWriteVFLCxtSuccess == FALSE32)
            {
                wVFLCxtBlkIdx = (wVFLCxtBlkIdx + 1) % VFL_INFO_SECTION_SIZE;
                continue;
            }
            else
            {
                break;
            }
        }
    }

    VFL_LOG_PRINT((TEXT("[VFL:OUT] --_StoreVFLCxt(wBank:%d)\n"), wBank));
    if (boolWriteVFLCxtSuccess == FALSE32)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _StoreVFLCxt(line:%d) fail bank %d!\n"), __LINE__, wBank));
    }

    return boolWriteVFLCxtSuccess;
}
#endif

static BOOL32 _ReadVFLCxtFromFlash(UInt16 wBank, UInt16 wBlk, UInt16 wStartPOffset, Buffer * pBuf)
{
    Int32 nFILRet;
    UInt16 wPageIdx;

    for (wPageIdx = 0; wPageIdx < VFL_NUM_OF_VFL_CXT_COPIES; wPageIdx++)
    {
        nFILRet = pFuncTbl->ReadWithECC(wBank, GET_Ppn(wBlk, (wStartPOffset + wPageIdx)), pBuf->pData, pBuf->pSpare, NULL, NULL, FALSE32);
        if (nFILRet == FIL_SUCCESS)
        {
            UInt8 bStatus, bSpareType;
            /* verify the status byte */
            _GetVFLSpare(NULL, &bStatus, &bSpareType, pBuf->pSpare);
            if (bStatus == PAGE_VALID && bSpareType == VFL_SPARE_TYPE_CXT)
            {
                return TRUE32;
            }
        }
    }
    return FALSE32;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _LoadVFLCxt                                                          */
/* DESCRIPTION                                                               */
/*      This function loads VFL context from VFL area.					     */
/* PARAMETERS                                                                */
/*      wBank   bank number                                                  */
/* RETURN VALUES                                                             */
/*		TRUE32																 */
/*				_LoadVFLCxt is completed									 */
/*		FALSE32																 */
/*				_LoadVFLCxt is failed										 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static BOOL32
_LoadVFLCxt(UInt16 wBank, UInt8 * pabBBT)
{
    VFLCxt  *pVFLCxt;
    UInt16 wBlkIdx, wPOffsetIdx;
    Buffer  *pBuf = NULL;
    UInt32 dwMinAge;
    UInt16 wNewCxt, wNewPOffset;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++_LoadVFLCxt(wBank:%d)\n"), wBank));

    /* check current state			*/
    if (wBank >= BANKS_TOTAL)
    {
        return FALSE32;
    }

    pVFLCxt = GET_VFLCxt(wBank);
    pBuf = BUF_Get(BUF_MAIN_AND_SPARE);

    if (pBuf == NULL)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _LoadVFLCxt(wBank:%d) failed BUF_Get(line:%d)\n"), wBank, __LINE__));
        return FALSE32;
    }

    /* first thing - find the first block with VFLCxt information and extract the awInfoBlk from it */
    for (wBlkIdx = VFL_AREA_START; wBlkIdx < VFL_AREA_SIZE; wBlkIdx++)
    {
        if (_isBlockGood(pabBBT, wBlkIdx) == TRUE32)
        {
            if (_ReadVFLCxtFromFlash(wBank, wBlkIdx, 0, pBuf) == TRUE32)
            {
                VFLCxt  *pVFLCxtTemp;
                pVFLCxtTemp = (VFLCxt *)pBuf->pData;
                WMR_MEMCPY(pVFLCxt->awInfoBlk, pVFLCxtTemp->awInfoBlk, (sizeof(UInt16) * VFL_INFO_SECTION_SIZE));
                break;
            }
        }
    }
    if (wBlkIdx == VFL_AREA_SIZE)
    {
        BUF_Release(pBuf);
        /* we should never get here - getting here means we fail to find a valid VFLCxt in the VFL_AREA */
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _LoadVFLCxt(line:%d) fail bank %d!\n"), __LINE__, wBank));
        return FALSE32;
    }

    /* look for the youngest VFLCxt block */
    dwMinAge = 0xFFFFFFFF;
    wNewCxt = VFL_INFO_SECTION_SIZE;
    for (wBlkIdx = 0; wBlkIdx < VFL_INFO_SECTION_SIZE; wBlkIdx++)
    {
        UInt32 dwAge;

        if (pVFLCxt->awInfoBlk[wBlkIdx] != VFL_INVALID_INFO_INDEX)
        {
            if (_ReadVFLCxtFromFlash(wBank, pVFLCxt->awInfoBlk[wBlkIdx], 0, pBuf) == FALSE32)
            {
                continue;
            }
        }
        else
        {
            continue;
        }
        _GetVFLSpare(&dwAge, NULL, NULL, pBuf->pSpare);
        if (dwMinAge >= dwAge && dwAge != 0)
        {
            wNewCxt = wBlkIdx;
            dwMinAge = dwAge;
        }
        else
        {
            continue;
        }
    }
    if (wNewCxt == VFL_INFO_SECTION_SIZE)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _LoadVFLCxt(line:%d) bank %d!\n"), __LINE__, wBank));
        BUF_Release(pBuf);
        return FALSE32;
    }

    /* look for the youngest valid VFLCxt version in the youngest block (i.e. search the youngest block pages) */
    wNewPOffset = 0;
    for (wPOffsetIdx = VFL_NUM_OF_VFL_CXT_COPIES; wPOffsetIdx < PAGES_PER_BLOCK; wPOffsetIdx += VFL_NUM_OF_VFL_CXT_COPIES)
    {
        if (_ReadVFLCxtFromFlash(wBank, pVFLCxt->awInfoBlk[wNewCxt], wPOffsetIdx, pBuf) == FALSE32)
        {
            break;
        }

        wNewPOffset = wPOffsetIdx;
    }
    if (_ReadVFLCxtFromFlash(wBank, pVFLCxt->awInfoBlk[wNewCxt], wNewPOffset, pBuf) == FALSE32)
    {
        /* this shouldn't happen since are using valid pointers */
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _LoadVFLCxt(line:%d) bank %d!\n"), __LINE__, wBank));
        BUF_Release(pBuf);
        return FALSE32;
    }

    WMR_MEMCPY(&pstVFLMeta[wBank], pBuf->pData, sizeof(VFLMeta));

    if (pVFLCxt->dwGlobalCxtAge > gdwMaxGlobalAge)
    {
        gdwMaxGlobalAge = pVFLCxt->dwGlobalCxtAge;
    }

    BUF_Release(pBuf);
    if (CHECK_VFL_CXT_CHECK_SUM_NO_ASSERT(wBank) == FALSE32)
    {
        return FALSE32;
    }
    VFL_LOG_PRINT((TEXT("[VFL:OUT] --_LoadVFLCxt(wBank:%d)\n"), wBank));

    return TRUE32;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _GetReservedBlk                                                      */
/* DESCRIPTION                                                               */
/*      This function returns the reserved block index.              		 */
/* PARAMETERS                                                                */
/*      wBank    [IN] 	bank number					                         */
/*      nPbn     [IN] 	the physical bad block address.                      */
/*      *pIdx[OUT] 	the pointer of the replaced good block index.            */
/* RETURN VALUES                                                             */
/*		TRUE32																 */
/*				_GetReservedBlk is completed								 */
/*		FALSE32																 */
/*				_GetReservedBlk is failed									 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
#ifndef AND_READONLY
static BOOL32
_GetReservedBlk(UInt16 wBank, UInt16 wPbn, UInt16 *pwIdx)
{
    VFLCxt  *pVFLCxt;
    UInt16 wIdx;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++_GetReservedBlk(wBank:%d, wPbn:%d)\n"), wBank, wPbn));

    if ((wBank > BANKS_TOTAL) || (wPbn >= BLOCKS_PER_BANK) || (pwIdx == NULL))
    {
        return FALSE32;
    }

    pVFLCxt = GET_VFLCxt(wBank);

    for (wIdx = 0; wIdx < pVFLCxt->wReservedSecSize; wIdx++)
    {
        /* find good block index of reserved blocks */
        {
            if (pVFLCxt->aBadMapTable[wIdx] == 0)
            {
                *pwIdx = wIdx;
                break;
            }
        }
    }

    /* there is no reserved block */
    if (wIdx == pVFLCxt->wReservedSecSize)
    {
        if (pwIdx != NULL)
        {
            *pwIdx = 0xFFFF;
        }
    }

    VFL_LOG_PRINT((TEXT("[VFL:OUT] --_GetReservedBlk(wBank:%d, wPbn:%d)\n"), wBank, wPbn));
    return TRUE32;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _ReplaceBadBlk                                                       */
/* DESCRIPTION                                                               */
/*      This function replaces the bad block with the reserved block. 		 */
/* PARAMETERS                                                                */
/*      wBank    [IN] 	bank number					                         */
/*      nSrcPbn  [IN] 	the physical source bad block address                */
/*      *pDesLPbn[OUT] 	the pointer of the replaced good block address       */
/*      *pDesRPbn[OUT] 	the pointer of the replaced good block address       */
/* RETURN VALUES                                                             */
/*		TRUE32																 */
/*				_ReplaceBadBlk is completed									 */
/*		FALSE32																 */
/*				_ReplaceBadBlk is failed									 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static BOOL32
_ReplaceBadBlk(UInt16 wBank, UInt16 wSrcPbn, UInt16 *pwDesPbn)
{
    VFLCxt  *pVFLCxt;
    Int32 nFILRet = VFL_CRITICAL_ERROR;
    UInt16 wIdx = 0, wReplacementIdx = 0;
    UInt8 bTrialIdx1, bTrialIdx2;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++_ReplaceBadBlk(wBank:%d, wSrcPbn:%d)\n"), wBank, wSrcPbn));

    /* check current state			*/
    if ((wBank >= BANKS_TOTAL) || (wSrcPbn >= BLOCKS_PER_BANK) ||
        (pwDesPbn == NULL))
    {
        return FALSE32;
    }

    pVFLCxt = GET_VFLCxt(wBank);

    /* get reserved block index of good block */
    for (bTrialIdx1 = 0; bTrialIdx1 < 3; bTrialIdx1++)
    {
        if (_GetReservedBlk(wBank, wSrcPbn, pwDesPbn) == FALSE32)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  _GetReservedBlk(wBank:%d, wSrcPbn:%d) fail!\n"), wBank, wSrcPbn));
            return FALSE32;
        }

        WMR_ASSERT(pwDesPbn != NULL);

        if (*pwDesPbn == 0xFFFF)
        {
#if (defined(AND_SIMULATOR) && AND_SIMULATOR)
            WMR_SIM_EXIT("Out of replacement blocks");
#else // defined(AND_SIMULATOR) && AND_SIMULATOR
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  there is no reserved block!!!\n")));
#endif // defined(AND_SIMULATOR) && AND_SIMULATOR
            *pwDesPbn = wSrcPbn;

            return FALSE32;
        }

        wReplacementIdx = *pwDesPbn;

        for (bTrialIdx2 = 0; bTrialIdx2 < 3; bTrialIdx2++)
        {   /* erase the reserved block */
            nFILRet = pFuncTbl->Erase(wBank, (wReplacementIdx + pVFLCxt->wReservedSecStart));
            if (nFILRet == VFL_SUCCESS)
            {
                break;
            }
        }
        if (nFILRet != FIL_SUCCESS)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  erase fail occured while replacing a bad block!!!\n")));
        }
        else
        {
            break;
        }
    }
    if (nFILRet != FIL_SUCCESS)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  erase fail occurs when replaces bad block(line:%d)!!!\n"), __LINE__));
    }
    /* update bad block management table */
    CHECK_VFL_CXT_CHECK_SUM(wBank);
    for (wIdx = 0; wIdx < wReplacementIdx; wIdx++)
    {
        if (pVFLCxt->aBadMapTable[wIdx] == wSrcPbn)
        {
            pVFLCxt->aBadMapTable[wIdx] = 0xFFFF;
        }
    }
    pVFLCxt->aBadMapTable[wReplacementIdx] = (UInt16)wSrcPbn;

    *pwDesPbn = wReplacementIdx + pVFLCxt->wReservedSecStart;

    if (pVFLCxt->wBadMapTableMaxIdx <= wReplacementIdx)
    {
        pVFLCxt->wBadMapTableMaxIdx = (UInt16)(wReplacementIdx + 1);
    }

    /* update bad mark table */
    if (_isBadMarkTableMarkGood(pVFLCxt->aBadMark, wSrcPbn))
    {
        _SetBadMarkTable(pVFLCxt->aBadMark, wSrcPbn, FALSE32);
    }
    RECALC_VFL_CXT_CHECK_SUM(wBank);

    VFL_LOG_PRINT((TEXT("[VFL:OUT] --_ReplaceBadBlk(wBank:%d, wSrcPbn:%d)\n"), wBank, wSrcPbn));
    return TRUE32;
}
#endif
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _RemapBadBlk                                                         */
/* DESCRIPTION                                                               */
/*      This function remaps the block address if it is bad block. 			 */
/* PARAMETERS                                                                */
/*      wBank    [IN] 	bank number					                         */
/*      nSrcPbn  [IN] 	the physical source block address 		             */
/*      pwDesPbn [OUT] 	the pointer of the remapped good block address       */
/* RETURN VALUES                                                             */
/*		TRUE32																 */
/*				_RemapBadBlk is completed									 */
/*		FALSE32																 */
/*				_RemapBadBlk is failed									     */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static void
_RemapBadBlk(UInt16 wBank, UInt16 wSrcPbn, UInt16 *pwDesPbn)
{
    VFLCxt *pVFLCxt;
    UInt16 wIdx;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++_RemapBadBlk(wBank:%d, wSrcPbn:%d)\n"), wBank, wSrcPbn));

    pVFLCxt = GET_VFLCxt(wBank);

    /* check wSrcPbn block is bad block or not */
    {
        BOOL32 bRemap = FALSE32;

        if (!_isBadMarkTableMarkGood(pVFLCxt->aBadMark, wSrcPbn))
        {
            for (wIdx = 0; wIdx < pVFLCxt->wBadMapTableMaxIdx; wIdx++)
            {
                if (pVFLCxt->aBadMapTable[wIdx] == (UInt16)wSrcPbn)
                {
                    *pwDesPbn = pVFLCxt->wReservedSecStart + wIdx;
                    bRemap = TRUE32;

                    WMR_ASSERT(*pwDesPbn < BLOCKS_PER_BANK);
                    break;
                }
            }
        }

        if (bRemap == FALSE32)
        {
            *pwDesPbn = wSrcPbn;
        }
    }

    VFL_LOG_PRINT((TEXT("[VFL:OUT] --_RemapBadBlk(wBank:%d, wSrcPbn:%d)\n"), wBank, wSrcPbn));
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _GetPhysicalAddr	                                                 */
/* DESCRIPTION                                                               */
/*      This function returns the physical bank, block & page address of	 */
/*		the virtual page address											 */
/* PARAMETERS                                                                */
/*      nVpn     [IN] 	virtual page number			                         */
/*      pBank    [OUT] 	physical bank number		                         */
/*      pPbn     [OUT] 	physical block number		                         */
/*      pPOffset [OUT] 	physical page offset		                         */
/* RETURN VALUES                                                             */
/*		none																 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static void
_GetPhysicalAddr(UInt32 dwVpn, UInt16 *pwBank, UInt16 *pwPbn, UInt16 *pwPOffset)
{
    VFL_LOG_PRINT((TEXT("[VFL: IN] ++_GetPhysicalAddr(nVpn:%d)\n"), dwVpn));

    WMR_ASSERT(dwVpn < PAGES_TOTAL);
    WMR_ASSERT(dwVpn >= PAGES_PER_SUBLK);

    *pwBank = (UInt16)(dwVpn % BANKS_TOTAL);
    *pwPbn = (UInt16)(dwVpn / PAGES_PER_SUBLK);
    *pwPOffset = (UInt16)((dwVpn / BANKS_TOTAL) % PAGES_PER_BLOCK);

    VFL_LOG_PRINT((TEXT("[VFL:OUT] --_GetPhysicalAddr(nVpn:%d)\n"), dwVpn));
    return;
}

static UInt16
_CalcNumOfVFLSuBlk(void)
{
    // Any blocks above the binary size will be used as spares
    UInt32 wRoundedBlocksPerBank = 1UL << WMR_LOG2(BLOCKS_PER_BANK);
    
    return VFL_USER_BLKS_PER_1K * (wRoundedBlocksPerBank / 1024);
}
static BOOL32 _InitDeviceInfo(void)
{
    pFuncTbl = stFPartFunctions.GetLowFuncTbl();
    stVFLDeviceInfo.wBlocksPerBank = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CS); // this vfl does not use VS -> banks = cs
    stVFLDeviceInfo.wBytesPerPage = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    stVFLDeviceInfo.wNumOfBank = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_NUM_OF_CS);
    stVFLDeviceInfo.wPagesPerBlock = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_PAGES_PER_BLOCK);
    stVFLDeviceInfo.bRefreshThreshold = (UInt8)pFuncTbl->GetDeviceInfo(AND_DEVINFO_REFRESH_THRESHOLD);
    stVFLDeviceInfo.wUserSuBlkTotal = _CalcNumOfVFLSuBlk();
    stVFLDeviceInfo.dwPagesTotal = (UInt32)stVFLDeviceInfo.wBlocksPerBank *
                                   (UInt32)stVFLDeviceInfo.wPagesPerBlock * (UInt32)stVFLDeviceInfo.wNumOfBank;
    stVFLDeviceInfo.wPagesPerSuBlk = stVFLDeviceInfo.wPagesPerBlock * stVFLDeviceInfo.wNumOfBank;
    stVFLDeviceInfo.dwUserPagesTotal = (UInt32)stVFLDeviceInfo.wUserSuBlkTotal * (UInt32)stVFLDeviceInfo.wPagesPerSuBlk;

    stVFLDeviceInfo.dwValidMetaPerLogicalPage = pFuncTbl->GetDeviceInfo(AND_DEVINFO_FIL_META_VALID_BYTES);
    stVFLDeviceInfo.dwTotalMetaPerLogicalPage = pFuncTbl->GetDeviceInfo(AND_DEVINFO_FIL_META_BUFFER_BYTES);
    stVFLDeviceInfo.dwLogicalPageSize = pFuncTbl->GetDeviceInfo(AND_DEVINFO_FIL_LBAS_PER_PAGE);


    if (stVFLDeviceInfo.wNumOfBank != 1)
    {
	    RESERVED_SECTION_SIZE = stVFLDeviceInfo.wBlocksPerBank - (stVFLDeviceInfo.dwVFLAreaStart + FTL_AREA_SIZE + VFL_INFO_SECTION_SIZE);
    }
    else
    {
        UInt32 dwRemainSupBlk;

        dwRemainSupBlk = stVFLDeviceInfo.wBlocksPerBank - FTL_AREA_SIZE - stVFLDeviceInfo.wNumOfBank * (stVFLDeviceInfo.dwVFLAreaStart + VFL_INFO_SECTION_SIZE);

        RESERVED_SECTION_SIZE = (UInt16)(dwRemainSupBlk / stVFLDeviceInfo.wNumOfBank);
    }

    VFL_AREA_SIZE = VFL_INFO_SECTION_SIZE + RESERVED_SECTION_SIZE;

    FTL_CXT_SECTION_START = FTL_AREA_START;
    pFuncTbl->SetDeviceInfo(AND_DEVINFO_BANKS_PER_CS, 1);
    pFuncTbl->SetDeviceInfo(AND_DEVINFO_VENDOR_SPECIFIC_TYPE, FIL_VS_SIMPLE);

    return TRUE32;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      VFL_Init                                                             */
/* DESCRIPTION                                                               */
/*      This function initializes VFL layer.							     */
/* PARAMETERS                                                                */
/*      none			                                                     */
/* RETURN VALUES                                                             */
/* 		VFL_SUCCESS                                                          */
/*            VFL_Init is completed.                                         */
/*      VFL_CRITICAL_ERROR                                                   */
/*            VFL_Init is failed    		                                 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static Int32
VFL_Init(FPartFunctions * pFPartFunctions)
{
    VFL_LOG_PRINT((TEXT("[VFL: IN] ++VFL_Init()\n")));

    WMR_MEMCPY(&stFPartFunctions, pFPartFunctions, sizeof(FPartFunctions));
    _InitDeviceInfo();

    /* the size of FTLMeta must be 2048 bytes */
    WMR_ASSERT(sizeof(VFLMeta) == 2048);
#ifdef AND_COLLECT_STATISTICS
    WMR_ASSERT(sizeof(VFLStatistics) < AND_STATISTICS_SIZE_PER_LAYER);
    WMR_MEMSET(&stVFLStatistics, 0, sizeof(VFLStatistics));
#endif // AND_COLLECT_STATISTICS
       /* init VFL meta				*/
    if (pstVFLMeta == NULL)
    {
        pstVFLMeta = (VFLMeta *)WMR_MALLOC(sizeof(VFLMeta) * BANKS_TOTAL);
        if (pstVFLMeta == NULL)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  there is no memory to initialize pstVFLCxt!\n")));
            return VFL_CRITICAL_ERROR;
        }
    }

    /* init BBT area - allocate a byte per 8 blocks to store the BBT */
    if (pstBBTArea == NULL)
    {
        pstBBTArea = (UInt8 *)WMR_MALLOC(BBT_SIZE_PER_BANK);

        if (pstBBTArea == NULL)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  there is no memory to initialize pstBBTArea!\n")));
            return VFL_CRITICAL_ERROR;
        }
    }

    /* Allocate buffers for scattered read operation */
    if (pstadwScatteredReadPpn == NULL && pstawScatteredReadBank == NULL)
    {
        pstadwScatteredReadPpn = (UInt32 *)WMR_MALLOC(PAGES_PER_SUBLK * sizeof(UInt32));
        pstawScatteredReadBank = (UInt16 *)WMR_MALLOC(PAGES_PER_SUBLK * sizeof(UInt16));

        if (pstadwScatteredReadPpn == NULL || pstawScatteredReadBank == NULL)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  there is no memory to initialize pstBBTArea!\n")));
            return VFL_CRITICAL_ERROR;
        }
    }

    /* get low level function table	*/
    pFuncTbl = stFPartFunctions.GetLowFuncTbl();

    if (pFuncTbl == NULL)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  stFPartFunctions.GetLowFuncTbl() fail!\n")));
        return VFL_CRITICAL_ERROR;
    }
    gdwMaxGlobalAge = 0;
    VFL_LOG_PRINT((TEXT("[VFL:OUT] --VFL_Init()\n")));

    /* set the poitners to the checksum functions */
    vflChecksumFunc.UpdateChecksum = _UpdateVFLCxtChecksum;
    vflChecksumFunc.VerifyChecksum = _CheckVFLCxtDataIsUntouched;

    return VFL_SUCCESS;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      VFL_Open                                                             */
/* DESCRIPTION                                                               */
/*      This function opens VFL layer.									     */
/* PARAMETERS                                                                */
/*      none			                                                     */
/* RETURN VALUES                                                             */
/* 		VFL_SUCCESS                                                          */
/*            VFL_Open is completed.                                         */
/*      VFL_CRITICAL_ERROR                                                   */
/*            VFL_Open is failed    		                                 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static Int32
VFL_Open(UInt32 dwBootAreaSize, UInt32 minor_ver, UInt32 dwOptions)
{
    UInt16 wBankIdx;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++VFL_Open()\n")));

    for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
    {
        BOOL32 bRet;

        bRet = VFL_ReadBBT(wBankIdx, pstBBTArea);
        if (bRet == FALSE32)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_ReadBBT(wBank:%d)(line %d) fail!\n"), wBankIdx, __LINE__));
            return VFL_CRITICAL_ERROR;
        }
        bRet = _LoadVFLCxt(wBankIdx, pstBBTArea);

        if (bRet == FALSE32)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  _LoadVFLCxt(wBank:%d)(line %d) fail!\n"), wBankIdx, __LINE__));
            return VFL_CRITICAL_ERROR;
        }
    }

    /* make sure that FTLCxt is updated everywhere */
    {
        UInt16 awFTLCxtVbnTemp[FTL_CXT_SECTION_SIZE];
        WMR_MEMCPY(awFTLCxtVbnTemp, VFL_GetFTLCxtVbn(), (FTL_CXT_SECTION_SIZE * sizeof(UInt16)));
        for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
        {
            /* make sure all are up to date so later storing of VFL Cxt would be correct */
            WMR_MEMCPY(pstVFLMeta[wBankIdx].stVFLCxt.aFTLCxtVbn, awFTLCxtVbnTemp, FTL_CXT_SECTION_SIZE * sizeof(UInt16));
            RECALC_VFL_CXT_CHECK_SUM(wBankIdx);
        }
    }

    VFL_LOG_PRINT((TEXT("[VFL:OUT] --VFL_Open()\n")));

    return VFL_SUCCESS;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      VFL_Read                                                             */
/* DESCRIPTION                                                               */
/*      This function reads virtual page.								     */
/* PARAMETERS                                                                */
/*      nVpn		[IN]	virtual page number		                         */
/*      pBuf		[IN]	Buffer pointer			                         */
/*      bCleanCheck	[IN]	clean check or not		                         */
/*      bMarkECCForScrub	[IN]	in case of ECC error - add to scrub		 */
/*                               list or not		                         */
/* RETURN VALUES                                                             */
/* 		VFL_SUCCESS                                                          */
/*            VFL_Read is completed.                                         */
/*      VFL_CRITICAL_ERROR                                                   */
/*            VFL_Read is failed.    		                                 */
/*		VFL_U_ECC_ERROR														 */
/*			  ECC uncorrectable error occurs from FIL read function.		 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static Int32
VFL_Read(UInt32 dwVpn, Buffer *pBuf, BOOL32 boolCleanCheck, BOOL32 bMarkECCForScrub, BOOL32 * pboolNeedRefresh, UInt8 * pabSectorStat, BOOL32 bDisableWhitening)
{
    UInt16 wBank;
    UInt16 wPbn;
    UInt16 wPOffset;
    UInt16 wPbnRemapped;
    UInt32 dwPpnRemapped;
    UInt32 dwVpnCorrected;
    Int32 nFILRet;
    UInt8 bECCCorrectedBits;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++VFL_Read(dwVpn:%d,boolCleanCheck:%d)\n"), dwVpn, boolCleanCheck));

    if (pboolNeedRefresh != NULL)
    {
        *pboolNeedRefresh = FALSE32;
    }

#ifdef AND_COLLECT_STATISTICS
    stVFLStatistics.ddwPagesReadCnt++;
    stVFLStatistics.ddwSingleReadCallCnt++;
#endif
    dwVpnCorrected = dwVpn + (FTL_CXT_SECTION_START * PAGES_PER_SUBLK);
    /* check virtual page address	*/
    if (dwVpnCorrected >= PAGES_TOTAL || dwVpnCorrected < PAGES_PER_SUBLK)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Read(dwVpn:%d) dwVpn Overflow!\n"), dwVpnCorrected));
        return VFL_CRITICAL_ERROR;
    }

    /* calculate physical address		*/
    _GetPhysicalAddr(dwVpnCorrected, &wBank, &wPbn, &wPOffset);

    /* get remapped block address from VFL context	*/
    _RemapBadBlk(wBank, wPbn, &wPbnRemapped);

    /* get remapped page address		*/
    dwPpnRemapped = GET_Ppn(wPbnRemapped, wPOffset);

    nFILRet = pFuncTbl->ReadWithECC(wBank, dwPpnRemapped, pBuf->pData,
                                    pBuf->pSpare, &bECCCorrectedBits, NULL, bDisableWhitening);

    // in case we are no ready to process clean pages mark the page as ecc error if it is clean
    if (boolCleanCheck == FALSE32 && nFILRet == FIL_SUCCESS_CLEAN)
    {
        nFILRet = FIL_U_ECC_ERROR;
    }

    // check if the page need to be refreshed
    if (pboolNeedRefresh != NULL)
    {
        if ((bECCCorrectedBits >= FIL_ECC_REFRESH_TRESHOLD && nFILRet == FIL_SUCCESS) ||
            (nFILRet == FIL_U_ECC_ERROR))
        {
            // Mark the block for refresh
            if (pboolNeedRefresh != NULL)
            {
                *pboolNeedRefresh = TRUE32;
            }
        }
    }

    if (nFILRet == FIL_CRITICAL_ERROR || nFILRet == FIL_U_ECC_ERROR)
    {
        // reset the bank in the hopes of making it behave
        pFuncTbl->Reset();
        nFILRet = pFuncTbl->ReadWithECC(wBank, dwPpnRemapped, pBuf->pData,
                                        pBuf->pSpare, NULL, NULL, bDisableWhitening);
        // in case we are no ready to process clean pages mark the page as ecc error if it is clean
        if (boolCleanCheck == FALSE32 && nFILRet == FIL_SUCCESS_CLEAN)
        {
            nFILRet = FIL_U_ECC_ERROR;
        }
        if (nFILRet == FIL_CRITICAL_ERROR || nFILRet == FIL_U_ECC_ERROR)
        {
#ifndef AND_READONLY
            UInt32 dwIdx, dwFFMetaCnt = 0;
            // reset the bank in the hopes of making it behave for the next user
            pFuncTbl->Reset();
            //
            // IMPORTANT!!! caution when porting this code - we might need to allocate another buffer here
            //
            // use read with no ecc - this will get us the closest data to the original
            pFuncTbl->ReadNoECC(wBank, dwPpnRemapped, pBuf->pData, pBuf->pSpare);
            VFL_WRN_PRINT((TEXT("[VFL:WRN]  Read(0x%X) returns 0x%X fail!\n"), dwVpn, nFILRet));
            for (dwIdx = 0; dwIdx < BYTES_PER_METADATA_RAW; dwIdx++)
            {
                if (pBuf->pSpare[dwIdx] == 0xFF)
                {
                    dwFFMetaCnt++;
                }
            }
            VFL_WRN_PRINT((TEXT("[VFL:WRN]  Meta: %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X Meta FF Count %d (line:%d)\n"),
                           pBuf->pSpare[0], pBuf->pSpare[1], pBuf->pSpare[2], pBuf->pSpare[3],
                           pBuf->pSpare[4], pBuf->pSpare[5], pBuf->pSpare[6], pBuf->pSpare[7],
                           pBuf->pSpare[8], pBuf->pSpare[9], pBuf->pSpare[10], pBuf->pSpare[11],
                           dwFFMetaCnt, __LINE__));

            /* if the error is ECC mark the block for scrubing */
            if ((nFILRet == FIL_U_ECC_ERROR) && (bMarkECCForScrub == TRUE32))
            {
                _AddBlockToScrubList(wBank, wPbn);
            }
#endif
            return nFILRet;
        }
    }

    if (nFILRet == FIL_SUCCESS_CLEAN)
    {
        WMR_MEMSET(pBuf->pSpare, 0xFF, BYTES_PER_METADATA_RAW);
    }

    VFL_LOG_PRINT((TEXT("[VFL:OUT] --VFL_Read(dwVpn:%d,boolCleanCheck:%d)\n"), dwVpn, boolCleanCheck));
    return nFILRet;
}
#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      VFL_Write                                                            */
/* DESCRIPTION                                                               */
/*      This function writes virtual page.								     */
/* PARAMETERS                                                                */
/*      nVpn		[IN]	virtual page number		                         */
/*      pBuf		[IN]	Buffer pointer			                         */
/* RETURN VALUES                                                             */
/* 		VFL_SUCCESS                                                          */
/*            VFL_Write is completed.                                        */
/*      VFL_CRITICAL_ERROR                                                   */
/*            VFL_Write is failed.    		                                 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static Int32
VFL_Write(UInt32 dwVpn, Buffer *pBuf, BOOL32 boolReplaceBlockOnFail, BOOL32 bDisableWhitening)
{
    UInt16 wBank;
    UInt16 wPbn;
    UInt16 wPOffset;
    UInt16 wPbnRemapped;
    UInt32 dwPpnRemapped;
    UInt32 dwVpnCorrected;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++VFL_Write(dwVpn:%d)\n"), dwVpn));

#ifdef AND_COLLECT_STATISTICS
    stVFLStatistics.ddwPagesWrittenCnt++;
    stVFLStatistics.ddwSingleWriteCallCnt++;
#endif

    dwVpnCorrected = dwVpn + (FTL_CXT_SECTION_START * PAGES_PER_SUBLK); // FTL can access only vb FTL_CXT_SECTION_START and up.

    /* check virtual page address	*/
    if (dwVpnCorrected >= PAGES_TOTAL || dwVpnCorrected < PAGES_PER_SUBLK)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Write(dwVpn:%d) dwVpn Overflow!\n"), dwVpn));
        return VFL_CRITICAL_ERROR;
    }

    /* calculate physical address		*/
    _GetPhysicalAddr(dwVpnCorrected, &wBank, &wPbn, &wPOffset);

    /* get remapped block address from VFL context	*/
    _RemapBadBlk(wBank, wPbn, &wPbnRemapped);

    /* get remapped page address		*/
    dwPpnRemapped = GET_Ppn(wPbnRemapped, wPOffset);

    /* write page data					*/
    if (pFuncTbl->Write(wBank, dwPpnRemapped, pBuf->pData, pBuf->pSpare, bDisableWhitening) != FIL_SUCCESS)
    {
        CHECK_VFL_CXT_CHECK_SUM(wBank);
        pstVFLMeta[wBank].stVFLCxt.wNumOfWriteFail++;
        RECALC_VFL_CXT_CHECK_SUM(wBank);
        _AddBlockToScrubList(wBank, wPbn);
        VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_Write(dwVpn:%d) failed!\n"), dwVpn));
        return VFL_CRITICAL_ERROR;
    }

    VFL_LOG_PRINT((TEXT("[VFL:OUT] --VFL_Write(dwVpn:%d)\n"), dwVpn));

    return VFL_SUCCESS;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      VFL_Erase                                                            */
/* DESCRIPTION                                                               */
/*      This function erases virtual block(super block).				     */
/* PARAMETERS                                                                */
/*      wVbn	[IN]	virtual block number 								 */
/* RETURN VALUES                                                             */
/* 		VFL_SUCCESS                                                          */
/*            VFL_Erase is completed.  	                                     */
/*      VFL_CRITICAL_ERROR                                                   */
/*            VFL_Erase is failed.    		                                 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static Int32
VFL_Erase(UInt16 wVbn, BOOL32 bReplaceOnFail)
{
    UInt16 wPbnRemapped;
    UInt16 wVbnCorrected;
    Int32 nRetVal = VFL_CRITICAL_ERROR;
    UInt16 wBankIdx;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++VFL_Erase(wVbn:%d)\n"), wVbn));

#ifdef AND_COLLECT_STATISTICS
    stVFLStatistics.ddwBlocksErasedCnt++;
    stVFLStatistics.ddwEraseCallCnt++;
#endif

    wVbnCorrected = wVbn + FTL_CXT_SECTION_START;
    if (wVbnCorrected >= BLOCKS_PER_BANK || wVbnCorrected == 0 || (wVbnCorrected >= VFL_AREA_START && wVbnCorrected < VFL_AREA_START + VFL_AREA_SIZE))
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Erase(wVbn:%d) input overflow!\n"), wVbn));
        return VFL_CRITICAL_ERROR;
    }
    // NirW - code need to change to work with new erase in parallel.

    for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
    {
        UInt8 bTrialIdx;
        if (_IsBlockInScrubList(wBankIdx, wVbnCorrected))
        {
            UInt16 wReplacementPbn;
            _ReplaceBadBlk(wBankIdx, wVbnCorrected, &wReplacementPbn);
            _RemoveBlockFromScrubList(wBankIdx, wVbnCorrected);
        }

        /* erase blocks						*/
        for (bTrialIdx = 0; bTrialIdx < WMR_NUM_OF_ERASE_TRIALS_FOR_DATA_BLOCKS; bTrialIdx++)
        {
            /* calculate replaced physical block address */
            _RemapBadBlk(wBankIdx, wVbnCorrected, &wPbnRemapped);

            nRetVal = pFuncTbl->Erase(wBankIdx, wPbnRemapped);
            if (nRetVal != FIL_SUCCESS)
            {
                // try one more time...
                nRetVal = pFuncTbl->Erase(wBankIdx, wPbnRemapped);
            }

            if (nRetVal == FIL_SUCCESS)
            {
                break;
            }
            else if (!bReplaceOnFail)
            {
                return nRetVal;
            }
            else if (nRetVal == FIL_WRITE_FAIL_ERROR)
            {
                UInt16 wReplacementPbn;
                VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_Erase failed erasing pbn 0x%X bank %d!\n"), wPbnRemapped, wBankIdx));
                CHECK_VFL_CXT_CHECK_SUM(wBankIdx);
                pstVFLMeta[wBankIdx].stVFLCxt.wNumOfEraseFail++;
                RECALC_VFL_CXT_CHECK_SUM(wBankIdx);
                if (_ReplaceBadBlk(wBankIdx, wVbnCorrected, &wReplacementPbn) != TRUE32)
                {
                    VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Erase failed _ReplaceBadBlk\n")));
                    return VFL_CRITICAL_ERROR;
                }
                if (_StoreVFLCxt(wBankIdx) != TRUE32)
                {
                    VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Erase failed _StoreVFLCxt\n")));
                    return VFL_CRITICAL_ERROR;
                }
            }
        }
        if (bTrialIdx == WMR_NUM_OF_ERASE_TRIALS_FOR_DATA_BLOCKS)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Erase failed erasing / replacing vbn 0x%X bank %d!\n"), wVbnCorrected, wBankIdx));
            break;
        }
    }
    VFL_LOG_PRINT((TEXT("[VFL:OUT] --VFL_Erase(wVbn:%d)\n"), wVbn));

    return nRetVal;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      VFL_Format                                                           */
/* DESCRIPTION                                                               */
/*      This function formats VFL.										     */
/* PARAMETERS                                                                */
/*		none																 */
/* RETURN VALUES                                                             */
/* 		VFL_SUCCESS                                                          */
/*            VFL_Format is completed.                                       */
/*      VFL_CRITICAL_ERROR                                                   */
/*            VFL_Format is failed.    		                                 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static Int32
VFL_Format(UInt32 dwBootAreaSize, UInt32 dwOptions)
{
    UInt16 wBankIdx;
    VFLCxt  *pVFLCxt;
    UInt8 * pbaBBT;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++VFL_Format()\n")));

    WMR_MEMSET(pstVFLMeta, 0, sizeof(VFLMeta) * BANKS_TOTAL);
    pstVFLMeta->dwVersion = VFL_META_VERSION;
    pbaBBT = pstBBTArea;

    for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
    {
        Buffer *pBuf = NULL;
        UInt16 wIdx, wPbn;

        if (VFL_ReadBBT(wBankIdx, pbaBBT) == FALSE32)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_ReadBBT(%d) fail!\n"), wBankIdx));
            return VFL_CRITICAL_ERROR;
        }
        pVFLCxt = GET_VFLCxt(wBankIdx);

        pVFLCxt->dwGlobalCxtAge = wBankIdx;
        for (wIdx = 0; wIdx < FTL_INFO_SECTION_SIZE; wIdx++)
        {
            pVFLCxt->aFTLCxtVbn[wIdx] = (UInt16)(wIdx);
        }

        pVFLCxt->wNumOfInitBadBlk = 0;
        pVFLCxt->wNumOfEraseFail = 0;
        pVFLCxt->wNumOfWriteFail = 0;
        pVFLCxt->wCxtLocation = 0;
        pVFLCxt->wNextCxtPOffset = 0;
        pVFLCxt->dwCxtAge = 0;
        pVFLCxt->wBadMapTableMaxIdx = 0;
        pVFLCxt->wBadMapTableScrubIdx = WMR_MAX_RESERVED_SIZE;

        WMR_MEMSET(pVFLCxt->aBadMark, 0xFF, VFL_BAD_MARK_INFO_TABLE_SIZE);

        pBuf = BUF_Get(BUF_MAIN_AND_SPARE);

        if (pBuf == NULL)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  BUF_Get fail (line:%d)!\n"), __LINE__));
            return VFL_CRITICAL_ERROR;
        }

        /* initial bad check of VFL_INFO_SECTION */
        for (wIdx = 0, wPbn = VFL_AREA_START; wIdx < VFL_INFO_SECTION_SIZE && wPbn < VFL_AREA_SIZE; wPbn++)
        {
            if (_isBlockGood(pbaBBT, wPbn))
            {
                /* check that the block is erasable */
                UInt16 wEraseCnt;
                for (wEraseCnt = 0; wEraseCnt < WMR_NUM_OF_ERASE_TRIALS_FOR_INDEX_BLOCKS; wEraseCnt++)
                {
                    Int32 nFILRet;
                    nFILRet = pFuncTbl->Erase(wBankIdx, wPbn);
                    if (nFILRet == FIL_SUCCESS)
                    {
                        break;
                    }
                }
                if (wEraseCnt == WMR_NUM_OF_ERASE_TRIALS_FOR_INDEX_BLOCKS)
                {
                    pVFLCxt->wNumOfInitBadBlk++;
                }
                else
                {
                    pVFLCxt->awInfoBlk[wIdx++] = wPbn;
                }
            }
            else
            {
                pVFLCxt->wNumOfInitBadBlk++;
            }
        }

        pVFLCxt->wReservedSecStart = wPbn;
        pVFLCxt->wReservedSecSize = RESERVED_SECTION_SIZE - pVFLCxt->wNumOfInitBadBlk;
        if (pVFLCxt->wReservedSecSize >= WMR_MAX_RESERVED_SIZE)
        {
            /* we should never get to this line - WMR_MAX_RESERVED_SIZE should cover all the flahses we work with */
            pVFLCxt->wReservedSecSize = WMR_MAX_RESERVED_SIZE;
        }

        /* initial bad check of reserved area */
        for (wIdx = 0; wIdx < pVFLCxt->wReservedSecSize; wIdx++)
        {
            if (_isBlockGood(pbaBBT, (wIdx + pVFLCxt->wReservedSecStart)))
            {
                pVFLCxt->aBadMapTable[wIdx] = 0;
            }
            else
            {
                pVFLCxt->aBadMapTable[wIdx] = 0xFFFF;
                pVFLCxt->wNumOfInitBadBlk++;
            }
        }

        for (wIdx = (VFL_AREA_START + VFL_AREA_SIZE); wIdx < BLOCKS_PER_BANK; wIdx++)
        {
            if (!_isBlockGood(pbaBBT, wIdx))
            {
                UInt16 wTemp;

                pVFLCxt->wNumOfInitBadBlk++;
                RECALC_VFL_CXT_CHECK_SUM(wBankIdx);
                if (_ReplaceBadBlk(wBankIdx, wIdx, &wTemp) == FALSE32)
                {
                    VFL_ERR_PRINT((TEXT("[VFL:ERR]  _ReplaceBadBlk fail!\n")));
                    return VFL_CRITICAL_ERROR;
                }
            }
        }

        BUF_Release(pBuf);
        RECALC_VFL_CXT_CHECK_SUM(wBankIdx);
        if (_StoreVFLCxt(wBankIdx) == FALSE32)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  _StoreVFLCxt fail!\n")));
            return VFL_CRITICAL_ERROR;
        }
    }
    VFL_LOG_PRINT((TEXT("[VFL:OUT] --VFL_Format()\n")));
    return VFL_SUCCESS;
}
#endif // ! AND_READONLY

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      VFL_Close                                                            */
/* DESCRIPTION                                                               */
/*      This function releases VFL layer.								     */
/* PARAMETERS                                                                */
/*      none			                                                     */
/* RETURN VALUES                                                             */
/*      none			                                                     */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static void
VFL_Close(void)
{
    VFL_LOG_PRINT((TEXT("[VFL: IN] ++VFL_Close()\n")));

    /* release buffer manager			*/
    BUF_Close();

    /* release VFL meta				*/
    if (pstVFLMeta != NULL)
    {
        WMR_FREE(pstVFLMeta, sizeof(VFLMeta) * BANKS_TOTAL);
        pstVFLMeta = NULL;
    }

    if (pstBBTArea != NULL)
    {
        WMR_FREE(pstBBTArea, BBT_SIZE_PER_BANK);
        pstBBTArea = NULL;
    }

    if (pstadwScatteredReadPpn != NULL)
    {
        WMR_FREE(pstadwScatteredReadPpn, (PAGES_PER_SUBLK * sizeof(UInt32)));
        pstadwScatteredReadPpn = NULL;
    }

    if (pstawScatteredReadBank != NULL)
    {
        WMR_FREE(pstawScatteredReadBank, (PAGES_PER_SUBLK * sizeof(UInt16)));
        pstawScatteredReadBank = NULL;
    }

#ifdef AND_COLLECT_STATISTICS
    WMR_MEMSET(&stVFLStatistics, 0, sizeof(VFLStatistics));
#endif
    pFuncTbl = NULL;

    VFL_LOG_PRINT((TEXT("[VFL:OUT] --VFL_Close()\n")));

    return;
}

static UInt16*
VFL_GetFTLCxtVbn(void)
{
    UInt32 nBankIdx;
    UInt16  *pFTLCxtVbn = NULL;
    VFLCxt  *pVFLCxt = NULL;
    UInt32 aMaxGlobalAge = 0;

    for (nBankIdx = 0; nBankIdx < BANKS_TOTAL; nBankIdx++)
    {
        pVFLCxt = GET_VFLCxt(nBankIdx);

        if (pVFLCxt->dwGlobalCxtAge >= aMaxGlobalAge)
        {
            aMaxGlobalAge = pVFLCxt->dwGlobalCxtAge;
            pFTLCxtVbn = pVFLCxt->aFTLCxtVbn;
        }
    }
    return pFTLCxtVbn;
}

#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      VFL_ChangeFTLCxtVbn	                                                 */
/* DESCRIPTION                                                               */
/*      This function change the virtual block number of FTL context block   */
/* PARAMETERS                                                                */
/*      aFTLCxtVbn  [IN]    FTL context block list                           */
/*                          that replace old FTL context positon             */
/* RETURN VALUES                                                             */
/* 		VFL_SUCCESS                                                          */
/*            VFL_Format is completed.                                       */
/*      VFL_CRITICAL_ERROR                                                   */
/*            VFL_Format is failed.    		                                 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static Int32
VFL_ChangeFTLCxtVbn(UInt16 *pwaFTLCxtVbn)
{
    UInt16 wWritePosition;
    UInt16 wBankIdx;

    wWritePosition = (UInt16)(gdwMaxGlobalAge % BANKS_TOTAL);

    for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
    {
        CHECK_VFL_CXT_CHECK_SUM(wBankIdx);
        /* make sure all are up to date so later storing of VFL Cxt would be correct */
        WMR_MEMCPY(pstVFLMeta[wBankIdx].stVFLCxt.aFTLCxtVbn, pwaFTLCxtVbn, FTL_CXT_SECTION_SIZE * sizeof(UInt16));
        RECALC_VFL_CXT_CHECK_SUM(wBankIdx);
    }

    if (!_StoreVFLCxt(wWritePosition))
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL context write fail !\n")));
        return VFL_CRITICAL_ERROR;
    }

    return VFL_SUCCESS;
}

/* this function suppose to make the writing of several pages in the same block more efficient */
/*
    NAME
        VFL_WriteMultiplePagesInVb
    DESCRIPTION
        This function suppose to make the writing of several pages in the same block
        more efficient.
        Data and spare info (only meta data) for pages the write is given by the FTL
        as an array.

    PARAMETERS
        wVbn - the Vbn we are writing to.
        wStartPOffset - start offset from the beginning of the Vbn.
        wNumPagesToWrite - number of pages to write.
        pbaData - data buffer.
        pbaSpare - meta data buffer.
    RETURN VALUES
        TRUE32
            Operation completed successfully.
        FALSE32
            Operation failed.
    NOTES
        This function should not be used for writes of less than TOTAL_BANKS Vbs!!!
 */

static BOOL32 VFL_WriteMultiplePagesInVb(UInt32 dwVpn, UInt16 wNumPagesToWrite, UInt8 * pbaData, UInt8 * pbaSpare, BOOL32 boolReplaceBlockOnFail, BOOL32 bDisableWhitening)
{
    UInt16 awPbnRemapped[WMR_MAX_DEVICE];
    UInt32 adwPpnRemapped[WMR_MAX_DEVICE];
    UInt16 wVbnCorrected;
    UInt16 wPOffsetIdx;
    UInt16 wBankIdx;
    UInt16 wVbn, wStartPOffset;

    UInt16 wFailingCE = CE_STATUS_UNINITIALIZED;
    UInt32 wFailingPage = PAGE_STATUS_UNINITIALIZED;

    wStartPOffset = (UInt16)(dwVpn % PAGES_PER_SUBLK);
    wVbn = (UInt16)(dwVpn / PAGES_PER_SUBLK);
    if ((wStartPOffset + wNumPagesToWrite) > PAGES_PER_SUBLK)
    {
        return FALSE32;
    }

#ifdef AND_COLLECT_STATISTICS
    stVFLStatistics.ddwPagesWrittenCnt += wNumPagesToWrite;
    stVFLStatistics.ddwMultipleWriteCallCnt++;
#endif

    wVbnCorrected = (wVbn + FTL_CXT_SECTION_START); // FTL can access only vb FTL_CXT_SECTION_START and up.

    for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
    {
        _RemapBadBlk(wBankIdx, wVbnCorrected, &(awPbnRemapped[wBankIdx]));
    }

    /* write the first few sectors until we are BANKS_TOTAL aligned */
    for (wPOffsetIdx = wStartPOffset; wPOffsetIdx < (wStartPOffset + wNumPagesToWrite) && (wPOffsetIdx % BANKS_TOTAL); wPOffsetIdx++, pbaData += BYTES_PER_PAGE, pbaSpare += BYTES_PER_METADATA_RAW)
    {
        wBankIdx = (wPOffsetIdx % BANKS_TOTAL);

        if (pFuncTbl->Write(wBankIdx, GET_Ppn(awPbnRemapped[wBankIdx], (wPOffsetIdx / BANKS_TOTAL)), pbaData, pbaSpare, bDisableWhitening) != FIL_SUCCESS)
        {
            /* Add the block to the Write Failures list */
            CHECK_VFL_CXT_CHECK_SUM(wBankIdx);
            pstVFLMeta[wBankIdx].stVFLCxt.wNumOfWriteFail++;
            RECALC_VFL_CXT_CHECK_SUM(wBankIdx);
            _AddBlockToScrubList(wBankIdx, wVbnCorrected);
            VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_WriteMultiplePagesInVb(wVbn:0x%0X) fail!\n"), wVbn));
            return FALSE32;
        }
    }

    if (((wStartPOffset + wNumPagesToWrite) - wPOffsetIdx) > BANKS_TOTAL)
    {
        for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
        {
            adwPpnRemapped[wBankIdx] = GET_Ppn(awPbnRemapped[wBankIdx], (wPOffsetIdx / BANKS_TOTAL));
        }

        if (pFuncTbl->WriteMultiplePages(adwPpnRemapped, pbaData, pbaSpare, (UInt16)((((wStartPOffset + wNumPagesToWrite) - wPOffsetIdx) / BANKS_TOTAL) * BANKS_TOTAL), FALSE32, FALSE32, &wFailingCE, bDisableWhitening, &wFailingPage) != FIL_SUCCESS)
        {
            return FALSE32;
        }
        pbaData += ((((wStartPOffset + wNumPagesToWrite) - wPOffsetIdx) / BANKS_TOTAL) * BANKS_TOTAL) * BYTES_PER_PAGE;
        pbaSpare += ((((wStartPOffset + wNumPagesToWrite) - wPOffsetIdx) / BANKS_TOTAL) * BANKS_TOTAL) * BYTES_PER_METADATA_RAW;
        wPOffsetIdx += ((((wStartPOffset + wNumPagesToWrite) - wPOffsetIdx) / BANKS_TOTAL) * BANKS_TOTAL);
    }

    for (; wPOffsetIdx < (wStartPOffset + wNumPagesToWrite); wPOffsetIdx++, pbaData += BYTES_PER_PAGE, pbaSpare += BYTES_PER_METADATA_RAW)
    {
        wBankIdx = (wPOffsetIdx % BANKS_TOTAL);

        if (pFuncTbl->Write(wBankIdx, GET_Ppn(awPbnRemapped[wBankIdx], (wPOffsetIdx / BANKS_TOTAL)), pbaData, pbaSpare, bDisableWhitening) != FIL_SUCCESS)
        {
            /* Add the block to the Write Failures list */
            CHECK_VFL_CXT_CHECK_SUM(wBankIdx);
            pstVFLMeta[wBankIdx].stVFLCxt.wNumOfWriteFail++;
            RECALC_VFL_CXT_CHECK_SUM(wBankIdx);
            _AddBlockToScrubList(wBankIdx, wVbnCorrected);
            VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_WriteMultiplePagesInVb(wVbn:0x%0X) fail!\n"), wVbn));
            return FALSE32;
        }
    }

    return TRUE32;
}
#endif

VFLMeta * FTL_GetVFLMetaPtr(void)
{
    return pstVFLMeta;
}

static BOOL32
VFL_ReadMultiplePagesInVb(UInt16 wVbn, UInt16 wStartPOffset, UInt16 wNumPagesToRead, UInt8 * pbaData, UInt8 * pbaSpare, BOOL32 * pboolNeedRefresh, UInt8 * pabSectorStat, BOOL32 bDisableWhitening)
{
    UInt16 wBankIdx;
    UInt16 wVbnCorrected;
    UInt16 awPbnRemapped[WMR_MAX_DEVICE];
    UInt32 adwPpnRemapped[WMR_MAX_DEVICE];
    Int32 nRet;
    Buffer buffer;
    UInt16 wPagesRead = 0;

#ifdef AND_COLLECT_STATISTICS
    stVFLStatistics.ddwPagesReadCnt += wNumPagesToRead;
    stVFLStatistics.ddwSequetialReadCallCnt++;
#endif

    if (pboolNeedRefresh != NULL)
    {
        *pboolNeedRefresh = FALSE32;
    }
    wVbnCorrected = (wVbn + FTL_CXT_SECTION_START); // FTL can access only vb FTL_CXT_SECTION_START and up.

    // possibly change the code here to use scatter if the call is not aligned
    // also consider changing the code so we have a single call for the start leftover and end leftover...
    while (wStartPOffset % BANKS_TOTAL && wPagesRead < wNumPagesToRead)
    {
        buffer.pData = &pbaData[wPagesRead * BYTES_PER_PAGE];
        buffer.pSpare = &pbaSpare[wPagesRead * BYTES_PER_METADATA_RAW];
        nRet = VFL_Read((((UInt32)wVbn * PAGES_PER_SUBLK) + wStartPOffset++), &buffer, TRUE32, FALSE32, pboolNeedRefresh, NULL, bDisableWhitening);
#ifdef AND_COLLECT_STATISTICS
        // correct the numbers changed by VFL_Read...
        stVFLStatistics.ddwPagesReadCnt--;
        stVFLStatistics.ddwSingleReadCallCnt--;
#endif
        if (nRet != FIL_SUCCESS && nRet != FIL_SUCCESS_CLEAN)
        {
            VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_ReadMultiplePagesInVb 0x%X (line:%d)!\n"), nRet, __LINE__));
            return FALSE32;
        }
        wPagesRead++;
    }
    if (wPagesRead == wNumPagesToRead)
    {
        return TRUE32;
    }

    for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
    {
        _RemapBadBlk(wBankIdx, wVbnCorrected, &(awPbnRemapped[wBankIdx]));
        adwPpnRemapped[wBankIdx] = GET_Ppn(awPbnRemapped[wBankIdx], (wStartPOffset / BANKS_TOTAL));
    }

    if ((wNumPagesToRead - wPagesRead) > BANKS_TOTAL)
    {
        UInt8 bECCCorrectedBits;
        nRet = pFuncTbl->ReadMultiplePages(adwPpnRemapped, &pbaData[wPagesRead * BYTES_PER_PAGE],
                                           &pbaSpare[wPagesRead * BYTES_PER_METADATA_RAW],
                                           (((wNumPagesToRead - wPagesRead) / BANKS_TOTAL) * BANKS_TOTAL),
                                           &bECCCorrectedBits, NULL, bDisableWhitening);
        if (nRet != FIL_SUCCESS && nRet != FIL_SUCCESS_CLEAN)
        {
            VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_ReadMultiplePagesInVb 0x%X (line:%d)!\n"), nRet, __LINE__));
            return FALSE32;
        }
        if (bECCCorrectedBits >= FIL_ECC_REFRESH_TRESHOLD && pboolNeedRefresh != NULL)
        {
            *pboolNeedRefresh = TRUE32;
        }
        wStartPOffset += ((wNumPagesToRead - wPagesRead) / BANKS_TOTAL) * BANKS_TOTAL;
        wPagesRead += ((wNumPagesToRead - wPagesRead) / BANKS_TOTAL) * BANKS_TOTAL;
    }

    while (wPagesRead < wNumPagesToRead)
    {
        buffer.pData = &pbaData[wPagesRead * BYTES_PER_PAGE];
        buffer.pSpare = &pbaSpare[wPagesRead * BYTES_PER_METADATA_RAW];
        nRet = VFL_Read((((UInt32)wVbn * PAGES_PER_SUBLK) + wStartPOffset++), &buffer,
                        TRUE32, FALSE32,
                        (pboolNeedRefresh != NULL && *pboolNeedRefresh != TRUE32 ? pboolNeedRefresh : NULL),
                        NULL,
                        bDisableWhitening);
#ifdef AND_COLLECT_STATISTICS
        // correct the numbers changed by VFL_Read...
        stVFLStatistics.ddwPagesReadCnt--;
        stVFLStatistics.ddwSingleReadCallCnt--;
#endif
        if (nRet != FIL_SUCCESS && nRet != FIL_SUCCESS_CLEAN)
        {
            VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_ReadMultiplePagesInVb 0x%X (line:%d)!\n"), nRet, __LINE__));
            return FALSE32;
        }
        wPagesRead++;
    }
    return TRUE32;
}

static BOOL32
VFL_ReadScatteredPagesInVb(UInt32 * padwVpn, UInt16 wNumPagesToRead, UInt8 * pbaData, UInt8 * pbaSpare, BOOL32 * pboolNeedRefresh, UInt8 * pabSectorStat, BOOL32 bDisableWhitening, Int32 * actualStatus)
{
    UInt16 wPageIdx;
    Int32 nRet;
    UInt8 bCorrectableECC;

#ifdef AND_COLLECT_STATISTICS
    stVFLStatistics.ddwPagesReadCnt += wNumPagesToRead;
    stVFLStatistics.ddwScatteredReadCallCnt++;
#endif

    if (pboolNeedRefresh != NULL)
    {
        *pboolNeedRefresh = FALSE32;
    }

    for (wPageIdx = 0; wPageIdx < wNumPagesToRead; wPageIdx++)
    {
        UInt32 dwVpnCorrected;
        UInt16 wPbn, wPbnRemapped, wPOffset;

        dwVpnCorrected = padwVpn[wPageIdx] + (FTL_CXT_SECTION_START * PAGES_PER_SUBLK);

        /* calculate physical address		*/
        _GetPhysicalAddr(dwVpnCorrected, &pstawScatteredReadBank[wPageIdx], &wPbn, &wPOffset);

        /* get remapped block address from VFL context	*/
        _RemapBadBlk(pstawScatteredReadBank[wPageIdx], wPbn, &wPbnRemapped);

        /* get remapped page address		*/
        pstadwScatteredReadPpn[wPageIdx] = GET_Ppn(wPbnRemapped, wPOffset);
    }

    nRet = pFuncTbl->ReadScatteredPages(pstawScatteredReadBank, pstadwScatteredReadPpn, pbaData, pbaSpare, wNumPagesToRead, &bCorrectableECC, NULL, bDisableWhitening);
    if (bCorrectableECC >= FIL_ECC_REFRESH_TRESHOLD && pboolNeedRefresh != NULL)
    {
        VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_ReadScatteredPagesInVb mark page for refresh (line:%d)!\n"), __LINE__));
        *pboolNeedRefresh = TRUE32;
    }

    return (nRet == FIL_SUCCESS || nRet == FIL_SUCCESS_CLEAN) ? TRUE32 : FALSE32;
}

static BOOL32
VFL_GetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 * pdwStructSize)
{
    BOOL32 boolRes = FALSE32;

    switch (dwStructType & AND_STRUCT_INDEX_MASK)
    {
    case AND_STRUCT_VFL_VFLMETA:
    {
        const UInt32 wCS = dwStructType & AND_STRUCT_CS_MASK;
        if (pstVFLMeta && (wCS < BANKS_TOTAL))
        {
            VFLMeta *pSrcMeta = &pstVFLMeta[wCS];
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, pSrcMeta, sizeof(*pSrcMeta));
        }
        break;
    }

    case AND_STRUCT_VFL_GET_TYPE:
    {
        UInt8 bVFLType = VFL_TYPE_VFL;
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &bVFLType, sizeof(bVFLType));
        break;
    }

#ifdef AND_COLLECT_STATISTICS
    case AND_STRUCT_VFL_STATISTICS_SIZE:
    {
        UInt32 dwStatSize = sizeof(VFLStatistics);
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwStatSize, sizeof(dwStatSize));
        break;
    }

    case AND_STRUCT_VFL_STATISTICS:
    {
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &stVFLStatistics, sizeof(stVFLStatistics));
        break;
    }
#endif // AND_COLLECT_STATISTICS

    case AND_STRUCT_VFL_DEVICEINFO_SIZE:
    {
        UInt32 dwDevInfoSize = sizeof(VFLWMRDeviceInfo);
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwDevInfoSize, sizeof(dwDevInfoSize));
        break;
    }

#ifdef AND_COLLECT_STATISTICS
    case  AND_STRUCT_VFL_DEVICEINFO:
    {
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &stVFLDeviceInfo, sizeof(stVFLDeviceInfo));
        break;
    }

    case AND_STRUCT_VFL_FILSTATISTICS:
    {
        boolRes = FIL_GetStruct(AND_STRUCT_FIL_STATISTICS, pvoidStructBuffer, pdwStructSize);
        break;
    }
#endif

    case AND_STRUCT_VFL_NUM_OF_FTL_SUBLKS:
        // fall through
    case AND_STRUCT_VFL_NUM_OF_SUBLKS:
    {
        UInt16 wNumSuBlks = FTL_AREA_SIZE;
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wNumSuBlks, sizeof(wNumSuBlks));
        break;
    }

    case AND_STRUCT_VFL_BYTES_PER_PAGE:
    {
        UInt16 wPageSize = BYTES_PER_PAGE;
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wPageSize, sizeof(wPageSize));
        break;
    }

    case AND_STRUCT_VFL_PAGES_PER_SUBLK:
    {
        UInt16 wPagesPerSuBlk = PAGES_PER_SUBLK;
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wPagesPerSuBlk, sizeof(wPagesPerSuBlk));
        break;
    }

    case AND_STRUCT_VFL_BYTES_PER_META:
    {
        UInt16 wBytesPerMetaData = BYTES_PER_METADATA_RAW;
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wBytesPerMetaData, sizeof(wBytesPerMetaData));
        break;
    }

    case AND_STRUCT_VFL_CORRECTABLE_BITS:
    {
        UInt16 wCorrectableBits = (UInt16) pFuncTbl->GetDeviceInfo(AND_DEVINFO_CORRECTABLE_BITS);
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wCorrectableBits, sizeof(wCorrectableBits));
        break;
    }

    case AND_STRUCT_VFL_CORRECTABLE_SIZE:
    {
        UInt16 wCorrectableSize = (UInt16) pFuncTbl->GetDeviceInfo(AND_DEVINFO_CORRECTABLE_SIZE);
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wCorrectableSize, sizeof(wCorrectableSize));
        break;
    }

    default:
        VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_GetStruct 0x%X is not identified is VFL data struct identifier!\n"), dwStructType));
        boolRes = FALSE32;
    }
    return boolRes;
}

static BOOL32
VFL_SetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 dwStructSize)
{
    BOOL32 boolRes = TRUE32;

    switch (dwStructType & AND_STRUCT_INDEX_MASK)
    {
    case AND_STRUCT_VFL_STATISTICS:
    {
#ifdef AND_COLLECT_STATISTICS
        WMR_MEMCPY(&stVFLStatistics, pvoidStructBuffer, WMR_MIN(sizeof(VFLStatistics), dwStructSize));
#else
        boolRes = FALSE32;
#endif
        break;
    }

    case AND_STRUCT_VFL_FILSTATISTICS:
    {
#ifdef AND_COLLECT_STATISTICS
        FIL_SetStruct(AND_STRUCT_FIL_STATISTICS, pvoidStructBuffer, dwStructSize);
#else
        boolRes = FALSE32;
#endif
        break;
    }

    case AND_STRUCT_VFL_FTLTYPE:
    case AND_STRUCT_VFL_NEW_FTLTYPE:
    case AND_STRUCT_VFL_NEW_NUM_OF_FTL_SUBLKS:
    {
        VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_SetStruct 0x%X is not supported!\n"), dwStructType));
        break;
    }

    default:
        VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_SetStruct 0x%X is not a VFL data struct identifier!\n"), dwStructType));
        boolRes = FALSE32;
    }
    return boolRes;
}

static UInt16 VFL_GetVbasPerVb(UInt16 wVbn)
{
    return PAGES_PER_SUBLK;
}

void VFL_Register(VFLFunctions * pVFL_Functions)
{
#ifndef AND_READONLY
    pVFL_Functions->Format = VFL_Format;
    pVFL_Functions->WriteSinglePage = VFL_Write;
    pVFL_Functions->Erase = VFL_Erase;
    pVFL_Functions->ChangeFTLCxtVbn = VFL_ChangeFTLCxtVbn;
    pVFL_Functions->WriteMultiplePagesInVb = VFL_WriteMultiplePagesInVb;
#endif // ! AND_READONLY
    pVFL_Functions->Init = VFL_Init;
    pVFL_Functions->Open = VFL_Open;
    pVFL_Functions->Close = VFL_Close;
    pVFL_Functions->ReadMultiplePagesInVb = VFL_ReadMultiplePagesInVb;
    pVFL_Functions->ReadScatteredPagesInVb = VFL_ReadScatteredPagesInVb;
    pVFL_Functions->ReadSinglePage = VFL_Read;
    pVFL_Functions->GetFTLCxtVbn = VFL_GetFTLCxtVbn;
//	pVFL_Functions->GetAddress = VFL_GetAddress;
    pVFL_Functions->GetStruct = VFL_GetStruct;
    pVFL_Functions->SetStruct = VFL_SetStruct;
    pVFL_Functions->GetDeviceInfo = VFL_GetDeviceInfo;
    pVFL_Functions->GetVbasPerVb = VFL_GetVbasPerVb;
}

static UInt32 VFL_GetDeviceInfo(UInt32 dwParamType)
{
    UInt32 dwRetVal = 0xFFFFFFFF;

    switch (dwParamType)
    {
    case AND_DEVINFO_PAGES_PER_SUBLK:
    {
        dwRetVal = (UInt32)PAGES_PER_SUBLK;
        break;
    }

    case AND_DEVINFO_BYTES_PER_PAGE:
    {
        dwRetVal = (UInt32)BYTES_PER_PAGE;
        break;
    }

    case AND_DEVINFO_NUM_OF_BANKS:
    {
        dwRetVal = (UInt32)BANKS_TOTAL;
        break;
    }

    case AND_DEVINFO_NUM_OF_USER_SUBLK:
    {
        dwRetVal = (UInt32)FTL_AREA_SIZE;
        break;
    }

    case AND_DEVINFO_FTL_TYPE:
    {
        dwRetVal = (UInt32)FTL_TYPE_FTL;
        break;
    }

    case AND_DEVINFO_FTL_NEED_FORMAT:
    {
        dwRetVal = (UInt32)0;
        break;
    }
    case AND_DEVINFO_FIL_LBAS_PER_PAGE:
    {
        dwRetVal = stVFLDeviceInfo.dwLogicalPageSize;
        break;
    }
    case AND_DEVINFO_FIL_META_BUFFER_BYTES:
    {
        dwRetVal = stVFLDeviceInfo.dwTotalMetaPerLogicalPage;
        break;
    }
    case AND_DEVINFO_FIL_META_VALID_BYTES:
    {
        dwRetVal = stVFLDeviceInfo.dwValidMetaPerLogicalPage;
        break;
    }

    default:
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_GetDeviceInfo dwParamType not supported (0x%X) (line:%d)!\n"), dwParamType, __LINE__));
        break;
    }
    return dwRetVal;
}

#endif //#if AND_SUPPORT_LEGACY_VFL
