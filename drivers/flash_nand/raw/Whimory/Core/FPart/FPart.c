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
// avoid using the generic device info and move the info to the layers

#include "WMRConfig.h"
#include "ANDTypes.h"
#include "WMROAM.h"
#include "VFLBuffer.h"
#include "FIL.h"
#include "FPartTypes.h"
#include "FPart.h"

#define     GET_Ppn(Pbn, POffset)       ((Pbn) * BLOCK_STRIDE + (POffset)) /* translate block + pages offset to page address*/
#define     BLOCKS_PER_CS               (stUnderVFLDeviceInfo.wBlocksPerCS)
#define     CS_TOTAL                    (stUnderVFLDeviceInfo.wNumOfCEs)
#define     PAGES_PER_BLOCK             (stUnderVFLDeviceInfo.wPagesPerBlock)
#define     BYTES_PER_PAGE              (stUnderVFLDeviceInfo.wBytesPerPage)
#define     BYTES_PER_SPARE             (stUnderVFLDeviceInfo.wBytesPerSpare)
#define     INITIAL_BBT_TYPE            (stUnderVFLDeviceInfo.checkInitialBadType)
#define     BYTES_PER_BL_PAGE           (stUnderVFLDeviceInfo.wBytesPerBLPage)
#define     DIES_PER_CS                 (stUnderVFLDeviceInfo.wDiesPerCS)
#define     BLOCKS_PER_DIE              (stUnderVFLDeviceInfo.wBlocksPerDie)
#define     DIE_STRIDE                  (stUnderVFLDeviceInfo.wDieStride)
#define     BLOCK_STRIDE                (stUnderVFLDeviceInfo.wBlockStride)
#define     LAST_ADDRESSABLE_BLOCK      (stUnderVFLDeviceInfo.wLastBlock)
#define     GET_Pbn(wDieIdx, wBlkIdx)   ((wDieIdx) * DIE_STRIDE + (wBlkIdx))
#define     _Cbn2Pbn(Cbn)               (((Cbn) / BLOCKS_PER_DIE) * DIE_STRIDE + ((Cbn) % BLOCKS_PER_DIE)) //contiguous physical block to non-contiguous

// 4% bad blocks are calculated from the nearest power of two
#define     WORST_CASE_LAST_GOOD_BLOCK  (((1UL << WMR_LOG2(BLOCKS_PER_CS)) * 96) / 100)

/*****************************************************************************/
/* Constants					                                             */
/*****************************************************************************/
#define WMR_NUM_OF_ERASE_TRIALS_FOR_DATA_BLOCKS     (3)
#define MAX_ATTEMTS_BBT                             (3)  

/*****************************************************************************/
/* Static variables definitions                                              */
/*****************************************************************************/
specialBlockCache (*blockCacheArray)[MAX_NUM_SPECIAL_BLOCKS_PER_CS] = NULL;
/*****************************************************************************/
/* UnderVFL local function prototypes                                             */
/*****************************************************************************/

// Part 1 static declarations
static BOOL32       _ReadSpecialInfoBlock(UInt16 wCS, SpecialBlockHeaderStruct * pSpecialBlockHeaderStruct, UInt8 * pabData,
                                          UInt32 dwDataBufferSize, UInt8 * pbaSignature, UInt32 dwSignatureSize, BOOL32 boolCheckBlockZero,UInt16 *signatureLocation);

#ifndef AND_READONLY
static BOOL32       _CheckInitialBad(UInt16 wCS, UInt16 wPbn,
                                     Buffer  *pBuf, Buffer  *pSpareBuf, UInt32 *pInitBad);
static BOOL32       _CreateBBT(UInt16 wCS, UInt8 * pBBT);
static BOOL32       _WriteSpecialBlock(UInt16 wCS, UInt16 wPBlk, SpecialBlockHeaderStruct * pSpecialBlockHeaderStruct, UInt8 * pabData, UInt32 dwDataBufferSize);
static BOOL32        VFL_WriteDeviceUniqueInfo(UInt8 * pabData, UInt32 dwDataBufferSize);
static BOOL32        VFL_WriteDiagControlInfo(UInt8 * pabData, UInt32 dwDataBufferSize);
#endif // ! AND_READONLY

static LowFuncTbl   *pFuncTbl = NULL;
static UnderVFLDeviceInfo stUnderVFLDeviceInfo;
static FPartSignatureStyle stFPartSignatureStyle;
#ifndef AND_READONLY
static BOOL32 bSupportDevUniqueInfo = FALSE32;
static BOOL32 bSupportDiagCtrlInfo = FALSE32;
#endif // ! AND_READONLY

#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)

static BOOL32 specialBlockCachePopulated = FALSE32;


/*   special block cache */
static void specialBlockCache_Init(void)
{
    UInt8 i,j,k;

    if( blockCacheArray == NULL )
    {
        blockCacheArray = (specialBlockCache (*)[MAX_NUM_SPECIAL_BLOCKS_PER_CS])WMR_MALLOC(CS_TOTAL * MAX_NUM_SPECIAL_BLOCKS_PER_CS * sizeof(specialBlockCache));
        if(blockCacheArray == NULL)
        {
            WMR_PRINT(ERROR, TEXT("specialBlockCache_Init : unable to allocate special block cache of size 0x%x\n"),CS_TOTAL * MAX_NUM_SPECIAL_BLOCKS_PER_CS * sizeof(specialBlockCache));
            return;
        }
    }
    
    for(i=0;i<CS_TOTAL;i++)
        for(j=0;j<MAX_NUM_OF_SPECIALS;j++)
        {   
            for(k = 0;k < MAX_SPECIAL_BLOCK_COPIES;k++)
                blockCacheArray[i][j].sbLocation[k].locationValid = FALSE32;
            WMR_MEMSET(blockCacheArray[i][j].abSignature,0xff,SPECIAL_BLOCK_SIGNATURE_SIZE);
        }
    return;
}

static BOOL32 specialBlockCache_set(UInt8 csNo, UInt8 * blockSignature, UInt32 blockLocation)
{
    UInt8 i,j,defaultSignature[SPECIAL_BLOCK_SIGNATURE_SIZE];

    if((blockCacheArray == NULL) || (blockSignature == NULL) || (csNo >= CS_TOTAL))
        return FALSE32;

    WMR_MEMSET(defaultSignature,0xff,SPECIAL_BLOCK_SIGNATURE_SIZE);
    for(i=0;i<MAX_NUM_OF_SPECIALS;i++)
    {
        if((WMR_MEMCMP(blockSignature,blockCacheArray[csNo][i].abSignature,SPECIAL_BLOCK_SIGNATURE_SIZE) == 0) || (WMR_MEMCMP(blockCacheArray[csNo][i].abSignature,defaultSignature,SPECIAL_BLOCK_SIGNATURE_SIZE) == 0))
            break;
    }
    if(i < MAX_NUM_OF_SPECIALS)
    {
        WMR_MEMCPY(blockCacheArray[csNo][i].abSignature,blockSignature,SPECIAL_BLOCK_SIGNATURE_SIZE);
        for(j=0;j<MAX_SPECIAL_BLOCK_COPIES;j++)
            if(blockCacheArray[csNo][i].sbLocation[j].locationValid == FALSE32)
                break;
        if(j < MAX_SPECIAL_BLOCK_COPIES)
        {
            blockCacheArray[csNo][i].sbLocation[j].location = blockLocation;
            blockCacheArray[csNo][i].sbLocation[j].locationValid = TRUE32;
            return TRUE32;
        }
        
    }
    return FALSE32;
}

static BOOL32 specialBlockCache_hasEntry(UInt8 csNo, const UInt8 *signature)
{
    UInt32 idx;
    
    for(idx = 0; idx < MAX_NUM_OF_SPECIALS; idx++)
    {
        if(!WMR_MEMCMP(signature, blockCacheArray[csNo][idx].abSignature, SPECIAL_BLOCK_SIGNATURE_SIZE))
        {
            return TRUE32;
        }
    }
    return FALSE32;
}

static BOOL32 specialBlockCache_get(UInt8 csNo, specialBlockCache *sbCacheEntry)
{
    UInt8 i;

    if((blockCacheArray == NULL) || (sbCacheEntry == NULL) || (csNo >= CS_TOTAL))
        return FALSE32;

    for(i=0;i<MAX_NUM_OF_SPECIALS;i++)
    {
        if(WMR_MEMCMP(sbCacheEntry->abSignature,blockCacheArray[csNo][i].abSignature,SPECIAL_BLOCK_SIGNATURE_SIZE) == 0) 
            break;
    }
    if(i < MAX_NUM_OF_SPECIALS)
    {
        WMR_MEMCPY(sbCacheEntry,&(blockCacheArray[csNo][i]),sizeof(specialBlockCache));
        return TRUE32;
    }
    return FALSE32;
}

static BOOL32 specialBlockCache_invalidate(UInt8 csNo, const UInt8 * blockSignature, UInt8 arrayElement)
{
    UInt8 i;

    if((blockCacheArray == NULL) || (blockSignature == NULL) || (csNo >= CS_TOTAL))
        return FALSE32;

    for(i=0;i<MAX_NUM_OF_SPECIALS;i++)
    {
        if(WMR_MEMCMP(blockSignature,blockCacheArray[csNo][i].abSignature,SPECIAL_BLOCK_SIGNATURE_SIZE) == 0) 
            break;
    }
    if(i < MAX_NUM_OF_SPECIALS)
    {
        if(arrayElement < MAX_SPECIAL_BLOCK_COPIES)
        {
            if(blockCacheArray[csNo][i].sbLocation[arrayElement].locationValid == TRUE32)
            {
                blockCacheArray[csNo][i].sbLocation[arrayElement].location = 0xffffffff;
                blockCacheArray[csNo][i].sbLocation[arrayElement].locationValid = FALSE32;
            }
            return TRUE32;
        }
    }
    return FALSE32;
}

#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)

#ifndef AND_READONLY
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
#endif // ! AND_READONLY

BOOL32 VFL_ReadBBT(UInt16 wCS, UInt8* pBBT)
{
    BBTInfoHeaderStruct bbtInfoHeaderStruct;
    BOOL32 result;

    if (pBBT)
    {
        WMR_MEMSET(pBBT, 0, BBT_SIZE_PER_CS);
    }
    
    result = _ReadSpecialInfoBlock(wCS, (SpecialBlockHeaderStruct*) &bbtInfoHeaderStruct, pBBT, BBT_SIZE_PER_CS, (UInt8 *)BBT_BLOCK_SIGNATURE, BBT_BLOCK_SIGNATURE_SIZE, FALSE32, NULL);
    
    return result;
}

BOOL32 VFL_ReadBBTWithoutSpecial(UInt16 wCS, UInt8* pBBT)
{
    BOOL32 result;
    UInt32 idx;
    BBTInfoHeaderStruct tmpHeader;

    result = _ReadSpecialInfoBlock(wCS, (SpecialBlockHeaderStruct*)&tmpHeader, pBBT, BBT_SIZE_PER_CS, (UInt8 *)BBT_BLOCK_SIGNATURE, BBT_BLOCK_SIGNATURE_SIZE, FALSE32, NULL);

    // Block 0 is guaranteed to be good
    _MarkBlockAsGoodInBBT(pBBT, 0);

    // Find the FPart special blocks that have been marked as bad
    for (idx = 0;
         (idx < MAX_NUM_SPECIAL_BLOCKS_PER_CS) && ((tmpHeader.adwSpecialBlocks[idx] & VFL_BLOCK_INDEX_MASK) != VFL_INVALID_INFO_INDEX);
         ++idx)
    {
        _MarkBlockAsGoodInBBT(pBBT, tmpHeader.adwSpecialBlocks[idx]);
    }

    return result;
}

static BOOL32 VFL_VerifyProductionFormat(void)
{
    BOOL32 res = TRUE32;
    UInt16 wCS;
    BBTInfoHeaderStruct bbtInfoHeaderStruct;
	
    // Since we're in recovery mode, check if we're whitening or not
    
    if (FIL_GetFuncTbl()->SetWhiteningState)
    {
        FIL_GetFuncTbl()->SetWhiteningState(TRUE32);
    }

    for (wCS = 0; wCS < CS_TOTAL && res == TRUE32; wCS++)
    {
        res = VFL_ReadBBT(wCS, NULL);
        if (!res && (wCS == 0) && FIL_GetFuncTbl()->SetWhiteningState)
        {
            // If we couldn't find a BBT with whitening, 
            // try it without and leave it turned off if bbt located
            FIL_GetFuncTbl()->SetWhiteningState(FALSE32);
            res = VFL_ReadBBT(0, NULL);
            if (!res && (wCS == 0))
            {
                /* virgin device . go with whitening if avaiable */
                FIL_GetFuncTbl()->SetWhiteningState(TRUE32);
            }
            if( res && (wCS == 0))
                WMR_PRINT(ALWAYS, TEXT("[FIL] Disable data whitening\n"));
        }
    }

    if ( res && _ReadSpecialInfoBlock(0, (SpecialBlockHeaderStruct *)&bbtInfoHeaderStruct, NULL, 0, (UInt8*)BBT_BLOCK_SIGNATURE, BBT_BLOCK_SIGNATURE_SIZE, TRUE32, NULL) )
    {
        WMR_PRINT(ALWAYS, TEXT("[VFL:WRN] BBT found in CS 0 Block 0... Forcing a failure of %s().\n"), __FUNCTION__);
        res = FALSE32;
    }

    return res;
}

#define CLEAN_PAGE_THRESHOLD 3
#define HW_ERROR_THRESHOLD 2

static BOOL32 readOneSpecialBlock(UInt16 wCS, UInt16 wBlkIdx, SpecialBlockHeaderStruct * pSpecialBlockHeaderStruct, UInt8 * pabData,
                                  UInt32 dwDataBufferSize, UInt8 * pbaSignature, UInt32 dwSignatureSize, UInt16 *signatureLocation, BOOL32 updateSBCache)
{
    UInt16 wPageIdx, wCleanPages = 0, wHWErrors = 0;
    Buffer *pBuf;
    
    if (pFuncTbl == NULL)
    {
        pFuncTbl = FIL_GetFuncTbl();
    }
    pBuf = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBuf == NULL)
    {
        return FALSE32;
    }
    
    if(pbaSignature == NULL)
        return FALSE32;
    
    for (wPageIdx = 0; (wPageIdx < PAGES_PER_BLOCK) && (wCleanPages < CLEAN_PAGE_THRESHOLD) && (wHWErrors < HW_ERROR_THRESHOLD); wPageIdx++)
    {
        Int32 nRet;
        nRet = pFuncTbl->ReadMaxECC(wCS, GET_Ppn(_Cbn2Pbn(wBlkIdx), wPageIdx), pBuf->pData);
        if (nRet == FIL_SUCCESS)
        {
            if (WMR_MEMCMP(pBuf->pData, pbaSignature, dwSignatureSize) == 0)
            {
                /* we found the relevant page */
                UInt32 dwDataSizeOnFlash;
                UInt32 dwBytesRead = 0, dwBytesToRead;
                
                if(signatureLocation != NULL)
                    *signatureLocation = wBlkIdx;
                // header information back to the caller
                if (pSpecialBlockHeaderStruct != NULL)
                {
                    WMR_MEMCPY(pSpecialBlockHeaderStruct, pBuf->pData, sizeof(SpecialBlockHeaderStruct));
                }
                
                // retrieve the data size
                dwDataSizeOnFlash = ((SpecialBlockHeaderStruct *)pBuf->pData)->dwDataSize;
                if (pabData == NULL)
                {
                    BUF_Release(pBuf);
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
                    if(updateSBCache == TRUE32)
                        specialBlockCache_set(wCS, pbaSignature, wBlkIdx);
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
                    return TRUE32;
                }
                dwBytesToRead = WMR_MIN(dwDataSizeOnFlash, dwDataBufferSize);
                // retrieve the data if it is all in one page
                if (dwBytesToRead <= (BYTES_PER_PAGE - sizeof(SpecialBlockHeaderStruct)))
                {
                    WMR_MEMCPY(pabData, &pBuf->pData[sizeof(SpecialBlockHeaderStruct)], WMR_MIN(dwDataSizeOnFlash, dwDataBufferSize));
                    BUF_Release(pBuf);
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
                    if(updateSBCache == TRUE32)
                        specialBlockCache_set(wCS, pbaSignature, wBlkIdx);
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
                    return TRUE32;
                }
                WMR_MEMCPY(pabData, &pBuf->pData[sizeof(SpecialBlockHeaderStruct)], (BYTES_PER_PAGE - sizeof(SpecialBlockHeaderStruct)));
                dwBytesRead += (BYTES_PER_PAGE - sizeof(SpecialBlockHeaderStruct));
                wPageIdx++;
                while (dwBytesRead < dwBytesToRead && wPageIdx < PAGES_PER_BLOCK)
                {
                    nRet = pFuncTbl->ReadMaxECC(wCS, GET_Ppn(_Cbn2Pbn(wBlkIdx), wPageIdx), pBuf->pData);
                    if (nRet == FIL_SUCCESS)
                    {
                        UInt32 dwBytesToCopyThisTime = WMR_MIN(BYTES_PER_PAGE, (dwBytesToRead - dwBytesRead));
                        WMR_MEMCPY(&pabData[dwBytesRead], pBuf->pData, dwBytesToCopyThisTime);
                        dwBytesRead += dwBytesToCopyThisTime;
                        wPageIdx++;
                    }
                    else
                    {
                        break;
                    }
                }
                if (dwBytesToRead == dwBytesRead)
                {
                    BUF_Release(pBuf);
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
                    if(updateSBCache == TRUE32)
                        specialBlockCache_set(wCS, pbaSignature, wBlkIdx);
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
                    return TRUE32;
                }
            }
        }
        if (nRet == FIL_SUCCESS_CLEAN)
        {
            wCleanPages++;
        }
        if (nRet == FIL_CRITICAL_ERROR)
        {
            wHWErrors++;
        }
    }
    BUF_Release(pBuf);
    return FALSE32;    
}

static BOOL32 _ReadSpecialInfoBlock(UInt16 wCS, SpecialBlockHeaderStruct * pSpecialBlockHeaderStruct, UInt8 * pabData,
                                    UInt32 dwDataBufferSize, UInt8 * pbaSignature, UInt32 dwSignatureSize, BOOL32 boolCheckBlockZero,UInt16 *signatureLocation)
{
    UInt16 wBlkIdx, wBlkEnd;
    specialBlockCache sbCacheEntry;

    WMR_MEMSET(&sbCacheEntry,0,sizeof(specialBlockCache));
    if(pbaSignature != NULL)
        WMR_MEMCPY(sbCacheEntry.abSignature,pbaSignature,SPECIAL_BLOCK_SIGNATURE_SIZE);
    else
        return FALSE32;
    
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    if((boolCheckBlockZero == FALSE32) && (specialBlockCache_get(wCS, &sbCacheEntry)))
    {  // !!!! Vadim : makes ure thet maxSpecial is according to the signature
        UInt8 i;
        UInt8 maxSpecialNo = 2;
        if(WMR_MEMCMP(sbCacheEntry.abSignature,NAND_DRIVER_SIGN_BLOCK_SIGNATURE,SPECIAL_BLOCK_SIGNATURE_SIZE) == 0)
            maxSpecialNo = 1;
        for(i = 0;i < maxSpecialNo;i++)
        {
            if(sbCacheEntry.sbLocation[i].locationValid == TRUE32)
                wBlkIdx = sbCacheEntry.sbLocation[i].location;
            else
                continue;
            if( readOneSpecialBlock(wCS, wBlkIdx, pSpecialBlockHeaderStruct, pabData,
                                    dwDataBufferSize, pbaSignature, dwSignatureSize, signatureLocation,FALSE32) )
                return TRUE32;
            else
                specialBlockCache_invalidate(wCS, pbaSignature, i);
            
            
        }

    }
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    {
        if (boolCheckBlockZero)
        {
            wBlkIdx = 1;
            wBlkEnd = 1;
        }
        else
        {
            /* search for a valid index structure - starting from the last block */
            wBlkIdx = BLOCKS_PER_CS;
            wBlkEnd = WORST_CASE_LAST_GOOD_BLOCK;
        }
        
        while (wBlkIdx-- >= wBlkEnd)
        {
           if( readOneSpecialBlock(wCS, wBlkIdx, pSpecialBlockHeaderStruct, pabData,
                            dwDataBufferSize, pbaSignature, dwSignatureSize, signatureLocation,TRUE32) )
           {
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
               if(WMR_MEMCMP(sbCacheEntry.abSignature,BBT_BLOCK_SIGNATURE,SPECIAL_BLOCK_SIGNATURE_SIZE) == 0)
               {
                   if(wCS == 0)
                   {
                       specialBlockCache_set(wCS, (UInt8 *)BBT_BLOCK_SIGNATURE, pSpecialBlockHeaderStruct->adwReserved[FPART_SPECIAL_BLK_BBT_BACKUP_IDX]);
                       specialBlockCache_set(wCS, (UInt8 *)UNIQUE_INFO_BLOCK_SIGNATURE, pSpecialBlockHeaderStruct->adwReserved[FPART_SPECIAL_BLK_DEV_UNIQUE_PRIMARY_IDX]);
                       specialBlockCache_set(wCS, (UInt8 *)UNIQUE_INFO_BLOCK_SIGNATURE, pSpecialBlockHeaderStruct->adwReserved[FPART_SPECIAL_BLK_DEV_UNIQUE_BACKUP_IDX]);
                       specialBlockCache_set(wCS, (UInt8 *)NAND_DRIVER_SIGN_BLOCK_SIGNATURE, pSpecialBlockHeaderStruct->adwReserved[FPART_SPECIAL_BLK_DEV_SIGNATURE_IDX]);
                       specialBlockCache_set(wCS, (UInt8 *)DIAG_INFO_BLOCK_SIGNATURE, pSpecialBlockHeaderStruct->adwReserved[FPART_SPECIAL_BLK_DIAG_CTRL_PRIMARY_IDX]);
                       specialBlockCache_set(wCS, (UInt8 *)DIAG_INFO_BLOCK_SIGNATURE, pSpecialBlockHeaderStruct->adwReserved[FPART_SPECIAL_BLK_DIAG_CTRL_BACKUP_IDX]);
                       specialBlockCachePopulated = TRUE32;
                   }
                   else
                   {
                       specialBlockCache_set(wCS, (UInt8 *)BBT_BLOCK_SIGNATURE, pSpecialBlockHeaderStruct->adwReserved[FPART_SPECIAL_BLK_BBT_BACKUP_IDX]);
                   }
               }
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
               return TRUE32;
           }
        }
    }
    return FALSE32;
}

#ifndef AND_READONLY

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      VFL_FactoryReformat                                                  */
/* DESCRIPTION                                                               */
/*      This function reformats a unit safely                                */
/* PARAMETERS                                                                */
/*		BOOL32 Force - wipe all blocks without care							 */
/* RETURN VALUES                                                             */
/* 		ANDErrorCodeOk                                                          */
/*            Format is completed.                                           */
/*      ANDErrorCodeHwErr                                                   */
/*            Format is failed.     		                                 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static Int32
VFL_FactoryReformat(BOOL32 boolForce, BOOL32 boolEraseBlockZero, UInt32 dwKeepoutArea)
{
    UInt16 wCSIdx, wDieIdx, wBlkIdx, wBBTIdx;
    UInt8 *pabBBTBuffer = NULL;

    WMR_PRINT(LOG, TEXT("[VFL: IN] ++VFL_FactoryReformat()\n"));

    /* allocate buffer for the BBT */
    pabBBTBuffer = (UInt8*)WMR_MALLOC(BBT_SIZE_PER_CS);
    if (pabBBTBuffer == NULL)
    {
        WMR_PRINT(ERROR, TEXT("[VFL:ERR] failed to allocate BBT buffer!\n"));
        return ANDErrorCodeHwErr;
    }

    // check if there is any hope at all of getting a valid BBT
    if ((boolForce == TRUE32) || (VFL_VerifyProductionFormat() == TRUE32))
    {
        for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
        {
            if ((boolForce == TRUE32) || (VFL_ReadBBT(wCSIdx, pabBBTBuffer) == TRUE32))
            {
                WMR_PRINT(ALWAYS, TEXT("[VFL:INF] BBT found for CS %d\n"), wCSIdx);
                wBBTIdx = dwKeepoutArea;
                for (wDieIdx = 0; wDieIdx < DIES_PER_CS; wDieIdx++)
                {
                    if( wDieIdx == 0 )
                        wBlkIdx = dwKeepoutArea;
                    else
                        wBlkIdx = 0;
                    for (; wBlkIdx < BLOCKS_PER_DIE; wBlkIdx++)
                    {
                        // only erase good blocks
                        if ((boolForce == TRUE32) || _isBlockGood(pabBBTBuffer, wBBTIdx))
                        {
                            /* check that the block is erasable */
                            UInt16 wEraseCnt;
                            for (wEraseCnt = 0; wEraseCnt < WMR_NUM_OF_ERASE_TRIALS_FOR_DATA_BLOCKS; wEraseCnt++)
                            {
                                Int32 nFILRet;
                                nFILRet = pFuncTbl->Erase(wCSIdx, GET_Pbn(wDieIdx, wBlkIdx));
                                if (nFILRet == FIL_SUCCESS)
                                {
                                    break;
                                }
                            }
                            if (wEraseCnt == WMR_NUM_OF_ERASE_TRIALS_FOR_DATA_BLOCKS)
                            {
                                WMR_PRINT(ERROR, TEXT("[VFL:ERR] Failed erasing CS %d block 0x%X\n"), wCSIdx, wBBTIdx);
                                _MarkBlockAsBadInBBT(pabBBTBuffer, wBBTIdx);
                            }
                        }
                        wBBTIdx++;
                    }
                }
                if (boolEraseBlockZero == TRUE32)
                {
                    Int32 nFILRet;
                    nFILRet = pFuncTbl->Erase(wCSIdx, 0);
                    if (nFILRet != FIL_SUCCESS)
                    {
                        WMR_PRINT(ERROR, TEXT("[VFL:ERR] Faild erasing block 0\n"));
                    }
                }
            }
            else
            {
                WMR_PRINT(ERROR, TEXT("[VFL:ERR]  No BBT found for CS %d\n"), wCSIdx);
                WMR_FREE(pabBBTBuffer, BBT_SIZE_PER_CS);
                return ANDErrorCodeHwErr;
            }
        }
    }
    else
    {
        WMR_PRINT(ERROR, TEXT("[VFL:ERR]  No factory bad block tables found!\n"));
        WMR_FREE(pabBBTBuffer, BBT_SIZE_PER_CS);
        return ANDErrorCodeHwErr;
    }

    WMR_FREE(pabBBTBuffer, BBT_SIZE_PER_CS);
    WMR_PRINT(LOG, TEXT("[VFL:OUT] --VFL_FactoryReformat()\n"));
    return ANDErrorCodeOk;
}

static BOOL32 _testSpecialBlockCandidate(UInt16 wBank, UInt16 wPBlk)
{
    Buffer *pBuf = NULL;
    UInt16 i;
    Int32 nFILRet;
    BOOL32 res = TRUE32;

    pBuf = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBuf == NULL)
    {
        WMR_PRINT(ERROR, TEXT("[VFL:ERR] VFL_WriteInitialBBT failed allocating buffer for spare\n"));
        return FALSE32;
    }

    WMR_MEMSET(pBuf->pData , 0, BYTES_PER_PAGE);
    WMR_MEMSET(pBuf->pSpare , 0, BYTES_PER_METADATA_RAW);
    nFILRet = pFuncTbl->Erase(wBank, _Cbn2Pbn(wPBlk));
    if(nFILRet != FIL_SUCCESS )
    {
        WMR_PRINT(ALWAYS, TEXT("[VFL:WRN] testSpecialBlockCandidate failed erasing bank %d, 0x%04X \n"), wBank, wPBlk);
        res = FALSE32;
    }
    for(i=0;(i < PAGES_PER_BLOCK) && (nFILRet == FIL_SUCCESS);i++)
    {
        nFILRet = pFuncTbl->Write(wBank, GET_Ppn(_Cbn2Pbn(wPBlk), i), pBuf->pData, pBuf->pSpare, TRUE32);
        if (nFILRet != FIL_SUCCESS)
        {
            WMR_PRINT(ALWAYS, TEXT("[VFL:WRN] testSpecialBlockCandidate failed writing to bank %d, 0x%04X \n"), wBank, wPBlk);
            res = FALSE32;
            break;
        }
        nFILRet = pFuncTbl->ReadWithECC(wBank, GET_Ppn(_Cbn2Pbn(wPBlk), i), pBuf->pData, pBuf->pSpare, NULL, NULL, TRUE32);
        if (nFILRet != FIL_SUCCESS)
        {
            res = FALSE32;
            WMR_PRINT(ALWAYS, TEXT("[VFL:WRN] testSpecialBlockCandidate failed reading bank %d, 0x%04X \n"), wBank, wPBlk);
            break;
        }
    }

#if (defined(WMR_ENABLE_ERASE_PROG_BAD_BLOCK))
    if (!res)
    {
        WMR_ENABLE_ERASE_PROG_BAD_BLOCK(wBank, _Cbn2Pbn(wPBlk));
    }
#endif // WMR_ENABLE_ERASE_PROG_BAD_BLOCK

    nFILRet = pFuncTbl->Erase(wBank, _Cbn2Pbn(wPBlk));
    
#if (defined(WMR_DISABLE_ERASE_PROG_BAD_BLOCK))
    if (!res)
    {
        WMR_DISABLE_ERASE_PROG_BAD_BLOCK(wBank, _Cbn2Pbn(wPBlk));
    }
#endif // WMR_DISABLE_ERASE_PROG_BAD_BLOCK

    if (nFILRet != FIL_SUCCESS)
    {
        WMR_PRINT(ALWAYS, TEXT("[VFL:WRN] testSpecialBlockCandidate failed erasing bank %d, 0x%04X \n"), wBank, wPBlk);
        res = FALSE32;
    }
    BUF_Release(pBuf);
    return res;	
}

static BOOL32
VFL_WriteInitialBBT(void)
{
    BOOL32 res = TRUE32, bbtDone = FALSE32;
    UInt16 wCSIdx;
    BBTInfoHeaderStruct bbtInfoHeaderStruct;
    UInt8 *pabBBTBuffer = NULL,*pabBBTBufferRAMOnly = NULL;
    UInt8 tryNo, BBTRAMCS = 0xff;
    Buffer *pBuf = NULL;

    // Special-block ordering currently requires that we have syscfg
    // <rdar://problem/7889334> FPart Special Block Assignment Bug
    WMR_ASSERT(bSupportDevUniqueInfo);
    
    /* allocate buffer for the BBT */
    pabBBTBuffer = (UInt8*)WMR_MALLOC(BBT_SIZE_PER_CS);
    pabBBTBufferRAMOnly = (UInt8*)WMR_MALLOC(BBT_SIZE_PER_CS);
    for(tryNo = 0, wCSIdx = 0 ; ( tryNo < MAX_ATTEMTS_BBT ) && ( bbtDone == FALSE32 ) ; tryNo++)
    {
        res = TRUE32;
        while(wCSIdx < CS_TOTAL)
        {
            UInt16 wBBIdx, wBBEnd;
            UInt16 wMarkIdx;
            BOOL32 boolBlk0HasBBT = FALSE32;
            UInt32 dwCurrentCENumSpecialBlocks;

            WMR_MEMCPY(bbtInfoHeaderStruct.abSignature, BBT_BLOCK_SIGNATURE, BBT_BLOCK_SIGNATURE_SIZE);
            WMR_MEMSET(bbtInfoHeaderStruct.adwSpecialBlocks, 0xFF, sizeof(bbtInfoHeaderStruct.adwSpecialBlocks));
            bbtInfoHeaderStruct.dwBBTSize = BBT_SIZE_PER_CS;
            bbtInfoHeaderStruct.dwVersion = VFL_BBT_STRUCT_VERSION;
            WMR_PRINT(ALWAYS, TEXT("[VFL:inf] VFL_WriteInitialBBT about to scan CS %d \n"), wCSIdx);
            /* check if block 0 has the BBT in it */
            WMR_MEMSET(pabBBTBuffer, 0, BBT_SIZE_PER_CS);
            boolBlk0HasBBT = _ReadSpecialInfoBlock(wCSIdx, (SpecialBlockHeaderStruct *)&bbtInfoHeaderStruct, pabBBTBuffer, BBT_SIZE_PER_CS, (UInt8*)BBT_BLOCK_SIGNATURE, BBT_BLOCK_SIGNATURE_SIZE, TRUE32, NULL);

            /* get bbt */
            if (boolBlk0HasBBT == FALSE32)
            {
                WMR_PRINT(ALWAYS, TEXT("[VFL:inf] VFL_WriteInitialBBT no BBT in block 0 scanning CS %d \n"), wCSIdx);
                WMR_MEMSET(pabBBTBuffer, 0, BBT_SIZE_PER_CS);
                res = _CreateBBT(wCSIdx, pabBBTBuffer);
                if (res == FALSE32)
                {
                    WMR_PRINT(FACTORY, "[VFL:inf] VFL_WriteInitialBBT _CreateBBT failed CS 0x%x\n", wCSIdx);
                    break;
                }
                /* Mark block 0 as bad - we use this block to store the bootloader / signature */
                _MarkBlockAsBadInBBT(pabBBTBuffer, 0);
                /* write the BBT to the BBT blocks as well as block 0 */
                res = _WriteSpecialBlock(wCSIdx, 0, (SpecialBlockHeaderStruct *)&bbtInfoHeaderStruct, pabBBTBuffer, BBT_SIZE_PER_CS);
                if (res == FALSE32)
                {
                    WMR_PRINT(FACTORY, "[VFL:inf] VFL_WriteInitialBBT updating block 0 at CS 0x%x failed \n", wCSIdx);
                    pFuncTbl->Erase(wCSIdx, 0);
                    break;
                }
            }	
            else
            {              
                WMR_PRINT(ALWAYS, TEXT("[VFL:WRN] VFL_WriteIntialBBT found BBT in Block0, but not SpecialBlocks\n"));	
            }
            if (BBTRAMCS != wCSIdx)
            {
                WMR_MEMCPY(pabBBTBufferRAMOnly, pabBBTBuffer, BBT_SIZE_PER_CS);
                BBTRAMCS = wCSIdx;
            }
            if (wCSIdx == 0)
            {
                dwCurrentCENumSpecialBlocks = DEFAULT_NUMBER_OF_SPECIAL_BLOCKS_CS0;
                if (bSupportDevUniqueInfo)
                {
                    dwCurrentCENumSpecialBlocks += FPART_COPIES_OF_SPECIAL_INFO;
                }
                if (bSupportDiagCtrlInfo)
                {
                    dwCurrentCENumSpecialBlocks += FPART_COPIES_OF_SPECIAL_INFO;
                }
            }
            else
            {
                dwCurrentCENumSpecialBlocks = NUMBER_OF_SPECIAL_BLOCKS_NOTCS0;
            }
            
            wBBIdx = (UInt16)(BLOCKS_PER_CS);
            wBBEnd = WORST_CASE_LAST_GOOD_BLOCK;
            for (wMarkIdx = 0; wMarkIdx < dwCurrentCENumSpecialBlocks; )
            {
                while (wBBIdx-- >= wBBEnd)
                {
                    if (_isBlockGood(pabBBTBufferRAMOnly, wBBIdx))
                    {
                        break;
                    }
                }
                if (wBBIdx < wBBEnd - 1)
                {
                    WMR_PRINT(FACTORY, "[VFL:ERR] VFL_WriteInitialBBT ran out of Special Block candidates: CS %d, passed %d, needed %d, tested %d\n",
                            wCSIdx, wMarkIdx, dwCurrentCENumSpecialBlocks, BLOCKS_PER_CS - WORST_CASE_LAST_GOOD_BLOCK + 1);
                    res = FALSE32;
                    break;
                }
                if( _testSpecialBlockCandidate( wCSIdx,  wBBIdx) == FALSE32 )
                {
                    WMR_PRINT(ERROR, TEXT("[VFL:inf] VFL_WriteInitialBBT testSpecialBlockCandidate failed at 0x%x 0x%x \n"),wCSIdx,  wBBIdx);
                    _MarkBlockAsBadInBBT(pabBBTBufferRAMOnly, wBBIdx);
                    continue;
                }
                _MarkBlockAsBadInBBT(pabBBTBuffer, wBBIdx);
                bbtInfoHeaderStruct.adwSpecialBlocks[wMarkIdx] = wBBIdx;
                wMarkIdx++;
            }
            if(res == FALSE32)
            {
                break;
            }
            res = _WriteSpecialBlock(wCSIdx, (UInt16)bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_BBT_PRIMARY_IDX], (SpecialBlockHeaderStruct *)&bbtInfoHeaderStruct, pabBBTBuffer, BBT_SIZE_PER_CS);
            if (res == FALSE32)
            {
                WMR_PRINT(FACTORY, "[VFL:inf] VFL_WriteInitialBBT failed udating  CS  0x%x block 0x%x \n", wCSIdx, bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_BBT_PRIMARY_IDX]);
                _MarkBlockAsBadInBBT(pabBBTBufferRAMOnly, bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_BBT_PRIMARY_IDX]);
                pFuncTbl->Erase(wCSIdx, _Cbn2Pbn((UInt16)bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_BBT_PRIMARY_IDX]));
                break;
            }
            res = _WriteSpecialBlock(wCSIdx, (UInt16)bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_BBT_BACKUP_IDX], (SpecialBlockHeaderStruct *)&bbtInfoHeaderStruct, pabBBTBuffer, BBT_SIZE_PER_CS);
            if (res == FALSE32)
            {
                WMR_PRINT(FACTORY, "[VFL:inf] VFL_WriteInitialBBT failed udating  CS  0x%x block 0x%x \n", wCSIdx, bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_BBT_BACKUP_IDX]);
                _MarkBlockAsBadInBBT(pabBBTBufferRAMOnly, bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_BBT_BACKUP_IDX]);
                pFuncTbl->Erase(wCSIdx, _Cbn2Pbn((UInt16)bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_BBT_PRIMARY_IDX]));
                pFuncTbl->Erase(wCSIdx, _Cbn2Pbn((UInt16)bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_BBT_BACKUP_IDX]));
                break;
            }
            if(res == TRUE32)
            {
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
                specialBlockCache_set(wCSIdx, bbtInfoHeaderStruct.abSignature, bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_BBT_PRIMARY_IDX]);
                specialBlockCache_set(wCSIdx, bbtInfoHeaderStruct.abSignature, bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_BBT_BACKUP_IDX]);
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
                wCSIdx++;
            }
        }
        if(res == TRUE32)
            bbtDone = TRUE32;
    }

#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    specialBlockCachePopulated = TRUE32;
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)

    // Write a default unique info buffer so it's quick to find
    if (res)
    {
        pBuf = BUF_Get(BUF_MAIN_ONLY);
        WMR_ASSERT(NULL != pBuf);
        WMR_MEMSET(pBuf->pData, 0xFF, BYTES_PER_PAGE);
    }
    if (res && bSupportDevUniqueInfo)
    {
        res = VFL_WriteDeviceUniqueInfo(pBuf->pData, BYTES_PER_PAGE);
    }
    if (res && bSupportDiagCtrlInfo)
    {
        res = VFL_WriteDiagControlInfo(pBuf->pData, BYTES_PER_PAGE);
    }
    if (NULL != pBuf)
    {
        BUF_Release(pBuf);
    }

    for (wCSIdx = 0; (wCSIdx < CS_TOTAL) && (res == TRUE32); wCSIdx++)
    {
        pFuncTbl->Erase(wCSIdx, 0);
    }

    WMR_FREE(pabBBTBuffer, BBT_SIZE_PER_CS);
    WMR_FREE(pabBBTBufferRAMOnly, BBT_SIZE_PER_CS);
    return res;
}

static BOOL32
_HasSandiskDefectMark(UInt8 *pabData)
{
    UInt32 dwByteIdx;
    
    // Block is defective if the first 6 bytes of this page are 0x0
    
    for (dwByteIdx = 0; dwByteIdx < 6; ++dwByteIdx)
    {
        if (0x00 != pabData[dwByteIdx])
        {
            return FALSE32;
        }
    }
    
    return TRUE32;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _CheckInitialBad	                                                 */
/* DESCRIPTION                                                               */
/*      This function checks spare to find initial bad block.				 */
/* PARAMETERS                                                                */
/*      wBank    [IN] 	physical bank number		                         */
/*      wPbn     [IN] 	physical block number		                         */
/*      pabSBuf  [IN] 	the pointer of spare buffer	                         */
/*      pInitBad [OUT] 	initial bad or not                                   */
/* RETURN VALUES                                                             */
/*		TRUE32																 */
/*				_CheckInitialBad is completed								 */
/*		FALSE32																 */
/*				_CheckInitialBad is failed									 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static BOOL32
_CheckInitialBad(UInt16 wCS, UInt16 wPbn, Buffer *pBuf, Buffer * pBufSpare, UInt32 *pInitBad)
{
    Int32 nFILRet;
    UInt32 dwPpn;

    WMR_PRINT(LOG, TEXT("[VFL: IN] ++_CheckInitialBad(wCS:%d, wPbn:%d)\n"), wCS, wPbn);

    WMR_ASSERT(wCS < CS_TOTAL && wPbn < LAST_ADDRESSABLE_BLOCK);

    *pInitBad = 0xFFFFFFFF;

    switch (INITIAL_BBT_TYPE)
    {
    case INIT_BBT_SANDISK_MLC:
    {
        UInt32 dwPageOffset;
        
        for (dwPageOffset = 0; dwPageOffset < 2; ++dwPageOffset)
        {
            dwPpn = GET_Ppn(wPbn, dwPageOffset);
            
            nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);
            
            if (nFILRet != FIL_SUCCESS)
            {
                WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
                return FALSE32;
            }
            
            if (_HasSandiskDefectMark(pBuf->pData))
            {
                *pInitBad = 0x0;
                break; // break for-loop
            }
        }
    }
        break;
    
    case INIT_BBT_TOSHIBA_MLC:
    {
        dwPpn = GET_Ppn(wPbn, 0);

        nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);

        if (nFILRet != FIL_SUCCESS)
        {
            WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
            return FALSE32;
        }

        if ((pBuf->pData[0] != 0xFF) || (pBufSpare->pData[0] != 0xFF))
        {
            *pInitBad = 0x0;
            break;
        }

        dwPpn = GET_Ppn(wPbn, (PAGES_PER_BLOCK - 1));

        nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);

        if (nFILRet != FIL_SUCCESS)
        {
            WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
            return FALSE32;
        }

        if ((pBuf->pData[0] != 0xFF) || (pBufSpare->pData[0] != 0xFF))
        {
            *pInitBad = 0x0;
        }
    }
        break;

    case INIT_BBT_SAMSUNG_MLC:
    {
        dwPpn = GET_Ppn(wPbn, (PAGES_PER_BLOCK - 1));

        nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);

        if (nFILRet != FIL_SUCCESS)
        {
            WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
            return FALSE32;
        }

        if (pBufSpare->pData[0] != 0xFF)
        {
            *pInitBad = 0x0;
        }
    }
        break;

    case INIT_BBT_SAMSUNG_MLC_8K:
    {
        dwPpn = GET_Ppn(wPbn, 0);
        
        nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);
        
        if (nFILRet != FIL_SUCCESS)
        {
            WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
            return FALSE32;
        }
        
        if (pBufSpare->pData[0] != 0xFF)
        {
            *pInitBad = 0x0;
            break;
        }
        
        dwPpn = GET_Ppn(wPbn, (PAGES_PER_BLOCK - 1));
        
        nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);
        
        if (nFILRet != FIL_SUCCESS)
        {
            WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
            return FALSE32;
        }
        
        if (pBufSpare->pData[0] != 0xFF)
        {
            *pInitBad = 0x0;
        }
    }
        break;

    case INIT_BBT_HYNIX_MLC:
    {
        dwPpn = GET_Ppn(wPbn, (PAGES_PER_BLOCK - 3));

        nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);

        if (nFILRet != FIL_SUCCESS)
        {
            WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
            return FALSE32;
        }

        if (pBufSpare->pData[0] != 0xFF)
        {
            *pInitBad = 0x0;
            break;
        }

        dwPpn = GET_Ppn(wPbn, (PAGES_PER_BLOCK - 1));

        nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);

        if (nFILRet != FIL_SUCCESS)
        {
            WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
            return FALSE32;
        }

        if (pBufSpare->pData[0] != 0xFF)
        {
            *pInitBad = 0x0;
        }
    }
        break;

    case INIT_BBT_SAMSUNG_SLC:
    {
        dwPpn = GET_Ppn(wPbn, 0);

        nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);

        if (nFILRet != FIL_SUCCESS)
        {
            WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
            return FALSE32;
        }

        if (pBufSpare->pData[0] != 0xFF)
        {
            *pInitBad = 0x0;
            break;
        }

        dwPpn = GET_Ppn(wPbn, 1);

        nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);

        if (nFILRet != FIL_SUCCESS)
        {
            WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
            return FALSE32;
        }

        if (pBufSpare->pData[0] != 0xFF)
        {
            *pInitBad = 0x0;
        }
    }
        break;

    case INIT_BBT_INTEL_ONFI:
    {
        // From page 82 of June 2007 version of "Intel MD516 NAND Flash Memory" datasheet:
        //
        // "NAND Flash devices are shipped from the factory erased. The factory identifies invalid
        //  blocks before shipping by programming 00h into the first spare location (column
        //  address 4,096) of the first page of each bad block.
        //
        // "System software should check columns 4096 to 4313 on the first page of each block
        //  prior to performing any program or erase operations on the NAND Flash device. A bad
        //  block table can then be created, allowing system software to map around these areas.
        //  Factory testing is performed under worst-case conditions. Because blocks marked
        //  'bad' may be marginal, it may not be possible to recover this information if the block is
        //  erased."

        UInt16 wIdx;

        dwPpn = GET_Ppn(wPbn, 0);

        nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);

        if (nFILRet != FIL_SUCCESS)
        {
            WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
            return FALSE32;
        }

        for (wIdx = 0; wIdx < BYTES_PER_SPARE; wIdx++)
        {
            if (pBufSpare->pData[wIdx] == 0x00)
            {
                *pInitBad = 0;
                break;
            }
        }
    }
        break;

    case INIT_BBT_MICRON_ONFI:
    {
        // From Micron's L63 datasheet, July 2008 version, pg 87, Error management section:
        // NAND Flash devices are shipped from the factory erased. The factory identifies invalid 
        // blocks before shipping by attempting to program the bad-block mark into every location 
        // in the first page of each invalid block. It may not be possible to program every location in 
        // an invalid block with the bad-block mark. However, the first spare area location in each 
        // bad block is guaranteed to contain the bad-block mark. This method is compliant with 
        // ONFI Factory Defect Mapping requirements.

        dwPpn = GET_Ppn(wPbn, 0);

        nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);
        if (nFILRet != FIL_SUCCESS)
        {
            WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
            return FALSE32;
        }

        if (pBufSpare->pData[0] == 0x00)
        {
            *pInitBad = 0;
        }
    }
        break;

    case INIT_BBT_ONFI:
    {
        // From section 3.2.1 on page 23 in revision 1.0 of the ONFI specification:
        //
        // "If a block is defective and 8-bit data access is used, the manufacturer shall mark the block as
        //  defective by setting at least one byte in the defect area...of the first or last
        //  page of the defective block to a value of 00h.  If a block is defective and 16-bit data access is
        //  used, the manufacturer shall mark the block as defective by setting at least one word in the defect
        //  area of the first or last page of the defective block to a value of 0000h."

        UInt16 wIdx;

        dwPpn = GET_Ppn(wPbn, 0);

        nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);

        if (nFILRet != FIL_SUCCESS)
        {
            WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
            return FALSE32;
        }

        for (wIdx = 0; wIdx < BYTES_PER_SPARE; wIdx++)
        {
            if (pBufSpare->pData[wIdx] == 0x00)
            {
                *pInitBad = 0;
                break;
            }
        }

        if (0 != *pInitBad)
        {
            dwPpn = GET_Ppn(wPbn, (PAGES_PER_BLOCK - 1));

            nFILRet = pFuncTbl->ReadNoECC(wCS, dwPpn, pBuf->pData, pBufSpare->pData);

            if (nFILRet != FIL_SUCCESS)
            {
                WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Read(wCS:%d) fail!\n"), wCS);
                return FALSE32;
            }

            for (wIdx = 0; wIdx < BYTES_PER_SPARE; wIdx++)
            {
                if (pBufSpare->pData[wIdx] == 0x00)
                {
                    *pInitBad = 0;
                    break;
                }
            }
        }
    }
        break;

    default:
        WMR_PRINT(ERROR, TEXT("[VFL:ERR]  Unknown initial bad block algorithm type (0x%x).\n"),
                       INITIAL_BBT_TYPE);
        return FALSE32;
    }

    WMR_PRINT(LOG, TEXT("[VFL:OUT] --_CheckInitialBad(wCS:%d, wPbn:%d)\n"), wCS, wPbn);

    return TRUE32;
}

static BOOL32
_CreateBBT(UInt16 wCS, UInt8 * pabBBT)
{
    UInt16 wBBTIdx, wDieIdx, wBlkIdx;
    UInt32 nBad;
    UInt32 dwBBCnt = 0;
    Buffer *pBufSpare = NULL, *pBufMain = NULL;

    WMR_ASSERT(wCS < CS_TOTAL);
    pBufMain = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBufMain == NULL)
    {
        WMR_PRINT(ERROR, TEXT("[VFL:ERR] _CreateBBT failed allocating buffer for spare\n"));
        return FALSE32;
    }

    pBufSpare = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBufSpare == NULL)
    {
        BUF_Release(pBufMain);
        WMR_PRINT(ERROR, TEXT("[VFL:ERR] _CreateBBT failed allocating buffer for spare\n"));
        return FALSE32;
    }
    /* mark all blocks as good */
    WMR_MEMSET(pabBBT, 0xFF, BBT_SIZE_PER_CS);
    wBBTIdx = 0;
    for (wDieIdx = 0; wDieIdx < DIES_PER_CS; wDieIdx++)
    {
        for (wBlkIdx = 0; wBlkIdx < BLOCKS_PER_DIE; wBlkIdx++)
        {
            if (_CheckInitialBad(wCS, GET_Pbn(wDieIdx, wBlkIdx), pBufMain, pBufSpare, &nBad) == FALSE32)
            {
                WMR_PRINT(ERROR, TEXT("[VFL:ERR] _CreateBBT failed calling _CheckInitialBad\n"));
                BUF_Release(pBufSpare);
                return FALSE32;
            }
            if (nBad == 0) /* block is bad */
            {
                _MarkBlockAsBadInBBT(pabBBT, wBBTIdx);
                dwBBCnt++;
            }
            else
            {
                _MarkBlockAsGoodInBBT(pabBBT, wBBTIdx);
            }
            wBBTIdx++;
        }
    }
    WMR_PRINT(ALWAYS, TEXT("[VFL:INF] _CreateBBT found %d bad blocks in wCS 0x%x\n"), dwBBCnt, wCS);

    BUF_Release(pBufMain);
    BUF_Release(pBufSpare);
    return TRUE32;
}

static BOOL32 _WriteSpecialBlock(UInt16 wBank, UInt16 wPBlk, SpecialBlockHeaderStruct * pSpecialBlockHeaderStruct, UInt8 * pabData, UInt32 dwDataBufferSize)
{
    BOOL32 res = TRUE32;
    UInt16 wTrialCnt = 0, wWriteIdx;
    Int32 nFILRet;
    Buffer *pBuffSrc, *pBuffVerify;

    // check the arguments
    if (pSpecialBlockHeaderStruct == NULL || pabData == NULL || dwDataBufferSize == 0)
    {
        return FALSE32;
    }
    pBuffSrc = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBuffSrc == NULL)
    {
        return FALSE32;
    }
    pBuffVerify = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBuffVerify == NULL)
    {
        BUF_Release(pBuffSrc);
        return FALSE32;
    }

    for (wTrialCnt = 0; wTrialCnt < WMR_NUM_OF_ERASE_TRIALS_FOR_INDEX_BLOCKS; wTrialCnt++)
    {
        res = TRUE32;
#if (defined(WMR_ENABLE_ERASE_PROG_BAD_BLOCK))
        WMR_ENABLE_ERASE_PROG_BAD_BLOCK(wBank, _Cbn2Pbn(wPBlk));
#endif

        nFILRet = pFuncTbl->Erase(wBank, _Cbn2Pbn(wPBlk));

#if (defined(WMR_DISABLE_ERASE_PROG_BAD_BLOCK))
        WMR_DISABLE_ERASE_PROG_BAD_BLOCK(wBank, _Cbn2Pbn(wPBlk));
#endif

        if (nFILRet != FIL_SUCCESS)
        {
#if (defined(AND_SIMULATOR) && AND_SIMULATOR)
            WMR_SIM_EXIT_NO_VALIDATION("Failed to erase a special block - code does not support reallocation\n");
#endif // AND_SIMULATOR
            res = FALSE32;
            continue;
        }

        /* write the data to whole pages in the block */
        for (wWriteIdx = 0; wWriteIdx < PAGES_PER_BLOCK && res == TRUE32;)
        {
            UInt32 dwBytesWritten = 0;
            // internal loop for x pages per single copy
            while (dwBytesWritten < dwDataBufferSize && wWriteIdx < PAGES_PER_BLOCK)
            {
                WMR_MEMSET(pBuffSrc->pData, 0x00, BYTES_PER_PAGE);
                WMR_MEMSET(pBuffSrc->pSpare, 0x00, BYTES_PER_METADATA_RAW);
                // put the data together
                if (dwBytesWritten == 0)
                {
                    dwBytesWritten = WMR_MIN((BYTES_PER_PAGE - sizeof(SpecialBlockHeaderStruct)), dwDataBufferSize);
                    WMR_MEMCPY(pBuffSrc->pData, pSpecialBlockHeaderStruct, sizeof(SpecialBlockHeaderStruct));
                    WMR_MEMCPY(&pBuffSrc->pData[sizeof(SpecialBlockHeaderStruct)], pabData, dwBytesWritten);
                }
                else
                {
                    UInt32 dwBytesWrittenThisTime = WMR_MIN(BYTES_PER_PAGE, (dwDataBufferSize - dwBytesWritten));
                    WMR_MEMCPY(pBuffSrc->pData, &pabData[dwBytesWritten], dwBytesWrittenThisTime);
                    dwBytesWritten += dwBytesWrittenThisTime;
                }
                // now write and imidiately read and verify
                nFILRet = pFuncTbl->WriteMaxECC(wBank, GET_Ppn(_Cbn2Pbn(wPBlk), wWriteIdx), pBuffSrc->pData);
                if (nFILRet != FIL_SUCCESS)
                {
                    res = FALSE32;
                    WMR_PRINT(ALWAYS, TEXT("[VFL:WRN] _WriteSpecialBlock failed writing to bank %d, 0x%04X, 0x%04X, line %d \n"), wBank, wPBlk, wWriteIdx, __LINE__);
                    break;
                }

                nFILRet = pFuncTbl->ReadMaxECC(wBank, GET_Ppn(_Cbn2Pbn(wPBlk), wWriteIdx), pBuffVerify->pData);
                if (nFILRet != FIL_SUCCESS)
                {
                    res = FALSE32;
                    WMR_PRINT(ALWAYS, TEXT("[VFL:WRN] _WriteSpecialBlock failed reading from bank %d, 0x%04X, 0x%04X, line %d \n"), wBank, wPBlk, wWriteIdx, __LINE__);
                    break;
                }
                if (WMR_MEMCMP(pBuffVerify->pData, pBuffSrc->pData, BYTES_PER_PAGE) != 0)
                {
                    res = FALSE32;
                    WMR_PRINT(ALWAYS, TEXT("[VFL:WRN] _WriteSpecialBlock failed verifying bank %d, 0x%04X, 0x%04X, line %d \n"), wBank, wPBlk, wWriteIdx, __LINE__);
                    break;
                }
                wWriteIdx++;
            }
        }
        if (res == TRUE32)
        {
            break;
        }
    }
    BUF_Release(pBuffSrc);
    BUF_Release(pBuffVerify);
    if (wTrialCnt == WMR_NUM_OF_ERASE_TRIALS_FOR_INDEX_BLOCKS)
    {
        return FALSE32;
    }
    return res;
}

#endif


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      VFL_Neuralize     	                                                 */
/* DESCRIPTION                                                               */
/*      This function erases all blocks but the factory-marked bad blocks,   */
/*      thus making the NAND look like they came fresh from the factory.     */
/* PARAMETERS                                                                */
/*      (none)                                                               */
/* RETURN VALUES                                                             */
/*		TRUE32																 */
/*				Passed                                                       */
/*		FALSE32																 */
/*				Failed                                                       */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
#ifndef AND_READONLY
BOOL32
VFL_Neuralize()
{
#if !(defined(AND_SUPPORT_NEURALIZE) && AND_SUPPORT_NEURALIZE)
    return FALSE32;
#else
    UInt16 wCSIdx, wDieIdx, wBlkIdx, wBBTIdx;
    UInt8 *pabBBTBuffer = NULL;

    WMR_PRINT(LOG, TEXT("[VFL: IN] ++VFL_Neuralize()\n"));

    /* allocate buffer for the BBT */
    pabBBTBuffer = (UInt8*)WMR_MALLOC(BBT_SIZE_PER_CS);
    if (pabBBTBuffer == NULL)
    {
        WMR_PRINT(ERROR, TEXT("[VFL:ERR] failed to allocate BBT buffer!\n"));
        return FALSE32;
    }

    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        if (VFL_ReadBBTWithoutSpecial(wCSIdx, pabBBTBuffer) == TRUE32)
        {
            WMR_PRINT(ALWAYS, TEXT("[VFL:INF] BBT found for CS %d\n"), wCSIdx);
            wBBTIdx = 0;
            for (wDieIdx = 0; wDieIdx < DIES_PER_CS; wDieIdx++)
            {
                for (wBlkIdx = 0; wBlkIdx < BLOCKS_PER_DIE; wBlkIdx++)
                {
                    // only erase good blocks
                    if (_isBlockGood(pabBBTBuffer, wBBTIdx))
                    {
                        /* check that the block is erasable */
                        UInt16 wEraseCnt;
                        for (wEraseCnt = 0; wEraseCnt < WMR_NUM_OF_ERASE_TRIALS_FOR_DATA_BLOCKS; wEraseCnt++)
                        {
                            Int32 nFILRet;
                            nFILRet = pFuncTbl->Erase(wCSIdx, GET_Pbn(wDieIdx, wBlkIdx));
                            if (nFILRet == FIL_SUCCESS)
                            {
                                break;
                            }
                            WMR_PRINT(ERROR, TEXT("[VFL:ERR] Failed erasing CS %d block 0x%X\n"), wCSIdx, (wDieIdx * BLOCK_STRIDE + wBlkIdx));
                        }
                    }
                    wBBTIdx++;
                }
            }
        }
        else
        {
            WMR_PRINT(ERROR, TEXT("[VFL:ERR]  No BBT found for CS %d\n"), wCSIdx);
        }
    }

    WMR_FREE(pabBBTBuffer, BBT_SIZE_PER_CS);
    WMR_PRINT(LOG, TEXT("[VFL:OUT] --VFL_Neuralize()\n"));
    return TRUE32;
#endif
}
#endif //AND_READONLY

static BOOL32 VFL_ReadDeviceUniqueInfo(UInt8 * pabData, UInt32 dwDataBufferSize)
{
    BOOL32 boolRet = FALSE32;
#if !(defined (DISABLE_NAND_SYSCFG) && DISABLE_NAND_SYSCFG)
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    if (specialBlockCache_hasEntry(0, UNIQUE_INFO_BLOCK_SIGNATURE) || !specialBlockCachePopulated)
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    {
        boolRet = _ReadSpecialInfoBlock(0, NULL, pabData, dwDataBufferSize, (UInt8*)UNIQUE_INFO_BLOCK_SIGNATURE, UNIQUE_INFO_BLOCK_SIGNATURE_SIZE, FALSE32, NULL);    
    }
#endif

    return boolRet;
}

#ifndef AND_READONLY
static BOOL32 VFL_WriteDeviceUniqueInfo(UInt8 * pabData, UInt32 dwDataBufferSize)
{
#if (defined (DISABLE_NAND_SYSCFG) && DISABLE_NAND_SYSCFG)
    return FALSE32;
#else
    BBTInfoHeaderStruct bbtInfoHeaderStruct;
    SpecialBlockHeaderStruct specialBlockHeaderStruct;
    BOOL32 boolRes;
    specialBlockCache sbCacheEntry;
    
    WMR_MEMSET(&sbCacheEntry,0,sizeof(specialBlockCache));
    WMR_MEMCPY(sbCacheEntry.abSignature, UNIQUE_INFO_BLOCK_SIGNATURE, UNIQUE_INFO_BLOCK_SIGNATURE_SIZE);
    WMR_MEMSET(&bbtInfoHeaderStruct, 0, sizeof(bbtInfoHeaderStruct));
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    if(specialBlockCache_get(0, &sbCacheEntry))
    {   
        if( sbCacheEntry.sbLocation[0].locationValid)
            bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DEV_UNIQUE_PRIMARY_IDX] = sbCacheEntry.sbLocation[0].location;
        if( sbCacheEntry.sbLocation[1].locationValid)
            bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DEV_UNIQUE_BACKUP_IDX] = sbCacheEntry.sbLocation[1].location;
    }
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    if((sbCacheEntry.sbLocation[0].locationValid == FALSE32) || (sbCacheEntry.sbLocation[1].locationValid == FALSE32 ))
    {
        if (_ReadSpecialInfoBlock(0, (SpecialBlockHeaderStruct *)&bbtInfoHeaderStruct, NULL, 0, (UInt8*)BBT_BLOCK_SIGNATURE, BBT_BLOCK_SIGNATURE_SIZE, FALSE32, NULL) == FALSE32)
        {
            return FALSE32;
        }
    }
    if ((bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DEV_UNIQUE_PRIMARY_IDX] >= BLOCKS_PER_CS) ||
        (bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DEV_UNIQUE_BACKUP_IDX] >= BLOCKS_PER_CS))
    {
        // Diag Control Info has not been allocated
        return FALSE32;
    }
    WMR_MEMSET(&specialBlockHeaderStruct, 0, sizeof(SpecialBlockHeaderStruct));
    WMR_MEMCPY(specialBlockHeaderStruct.abSignature, UNIQUE_INFO_BLOCK_SIGNATURE, UNIQUE_INFO_BLOCK_SIGNATURE_SIZE);
    specialBlockHeaderStruct.dwDataSize = dwDataBufferSize;
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    specialBlockCache_invalidate(0, UNIQUE_INFO_BLOCK_SIGNATURE, 0);
    specialBlockCache_invalidate(0, UNIQUE_INFO_BLOCK_SIGNATURE, 1);
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    boolRes = _WriteSpecialBlock(0, (UInt16)bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DEV_UNIQUE_PRIMARY_IDX], &specialBlockHeaderStruct, pabData, dwDataBufferSize);
    if (boolRes == TRUE32)
    {
        boolRes = _WriteSpecialBlock(0, (UInt16)bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DEV_UNIQUE_BACKUP_IDX], &specialBlockHeaderStruct, pabData, dwDataBufferSize);
    }
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    if (boolRes == TRUE32)
    {
        specialBlockCache_set(0, specialBlockHeaderStruct.abSignature, bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DEV_UNIQUE_PRIMARY_IDX]);
        specialBlockCache_set(0, specialBlockHeaderStruct.abSignature, bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DEV_UNIQUE_BACKUP_IDX]);
    }
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    return boolRes;
#endif // DISABLE_NAND_SYSCFG
}
#endif /* ! AND_READONLY */

static BOOL32 VFL_ReadDiagControlInfo(UInt8 * pabData, UInt32 dwDataBufferSize)
{
    BOOL32 boolRet = FALSE32;
#if !(defined (DISABLE_NAND_SYSCFG) && DISABLE_NAND_SYSCFG)
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    if (specialBlockCache_hasEntry(0, DIAG_INFO_BLOCK_SIGNATURE) || !specialBlockCachePopulated)
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    {
        boolRet = _ReadSpecialInfoBlock(0, NULL, pabData, dwDataBufferSize, (UInt8*)DIAG_INFO_BLOCK_SIGNATURE, DIAG_INFO_BLOCK_SIGNATURE_SIZE, FALSE32, NULL);    
    }
#endif

    return boolRet;
}

#ifndef AND_READONLY
static BOOL32 VFL_WriteDiagControlInfo(UInt8 * pabData, UInt32 dwDataBufferSize)
{
#if (defined (DISABLE_NAND_SYSCFG) && DISABLE_NAND_SYSCFG)
    return FALSE32;
#else
    BBTInfoHeaderStruct bbtInfoHeaderStruct;
    SpecialBlockHeaderStruct specialBlockHeaderStruct;
    BOOL32 boolRes, boolRes2;
    specialBlockCache sbCacheEntry;
    
    WMR_MEMSET(&sbCacheEntry,0,sizeof(specialBlockCache));
    WMR_MEMCPY(sbCacheEntry.abSignature, DIAG_INFO_BLOCK_SIGNATURE, DIAG_INFO_BLOCK_SIGNATURE_SIZE);
    WMR_MEMSET(&bbtInfoHeaderStruct, 0, sizeof(bbtInfoHeaderStruct));
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    if(specialBlockCache_get(0, &sbCacheEntry))
    {   
        if( sbCacheEntry.sbLocation[0].locationValid)
            bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DIAG_CTRL_PRIMARY_IDX] = sbCacheEntry.sbLocation[0].location;
        if( sbCacheEntry.sbLocation[1].locationValid)
            bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DIAG_CTRL_BACKUP_IDX] = sbCacheEntry.sbLocation[1].location;
    }
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    if((sbCacheEntry.sbLocation[0].locationValid == FALSE32) || (sbCacheEntry.sbLocation[1].locationValid == FALSE32 ))
    {
        if (_ReadSpecialInfoBlock(0, (SpecialBlockHeaderStruct *)&bbtInfoHeaderStruct, NULL, 0, (UInt8*)BBT_BLOCK_SIGNATURE, BBT_BLOCK_SIGNATURE_SIZE, FALSE32, NULL) == FALSE32)
        {
            return FALSE32;
        }
    }
    if ((bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DIAG_CTRL_PRIMARY_IDX] >= BLOCKS_PER_CS) ||
        (bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DIAG_CTRL_BACKUP_IDX] >= BLOCKS_PER_CS))
    {
        // Diag Control Info has not been allocated
        return FALSE32;
    }
    WMR_MEMSET(&specialBlockHeaderStruct, 0, sizeof(SpecialBlockHeaderStruct));
    WMR_MEMCPY(specialBlockHeaderStruct.abSignature, DIAG_INFO_BLOCK_SIGNATURE, DIAG_INFO_BLOCK_SIGNATURE_SIZE);
    specialBlockHeaderStruct.dwDataSize = dwDataBufferSize;
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    specialBlockCache_invalidate(0, DIAG_INFO_BLOCK_SIGNATURE, 0);
    specialBlockCache_invalidate(0, DIAG_INFO_BLOCK_SIGNATURE, 1);
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    boolRes = _WriteSpecialBlock(0, (UInt16)bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DIAG_CTRL_PRIMARY_IDX], &specialBlockHeaderStruct, pabData, dwDataBufferSize);
    if (boolRes == TRUE32)
    {
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
        specialBlockCache_set(0, specialBlockHeaderStruct.abSignature, bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DIAG_CTRL_PRIMARY_IDX]);
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    }
    else 
    {
        pFuncTbl->Erase(0, _Cbn2Pbn((UInt16)bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DIAG_CTRL_PRIMARY_IDX]));
    }
    boolRes2 = _WriteSpecialBlock(0, (UInt16)bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DIAG_CTRL_BACKUP_IDX], &specialBlockHeaderStruct, pabData, dwDataBufferSize);
    if (boolRes2 == TRUE32)
    {
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
        specialBlockCache_set(0, specialBlockHeaderStruct.abSignature, bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DIAG_CTRL_BACKUP_IDX]);
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    }
    else 
    {
        pFuncTbl->Erase(0, _Cbn2Pbn((UInt16)bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DIAG_CTRL_BACKUP_IDX]));
    }	
    if (boolRes == FALSE32 && boolRes2 == FALSE32)
    {
        WMR_PRINT(ERROR, TEXT("[VFL:ERR] VFL_WriteDiagControlInfo failed writing to special blocks (line:%d)\n"), __LINE__);
        return FALSE32;
    }
    else
        return TRUE32;
	
#endif // DISABLE_NAND_SYSCFG
}

#endif /* ! AND_READONLY */

#ifndef AND_READONLY
static BOOL32 VFL_WriteSignature(void * pvoidSignature, UInt32 dwSignatureSize)
{
    BBTInfoHeaderStruct bbtInfoHeaderStruct;
    SpecialBlockHeaderStruct specialBlockHeaderStruct;
    BOOL32 boolRes;
    specialBlockCache sbCacheEntry;
    
    WMR_MEMSET(&sbCacheEntry,0,sizeof(specialBlockCache));
    WMR_MEMCPY(sbCacheEntry.abSignature, NAND_DRIVER_SIGN_BLOCK_SIGNATURE, NAND_DRIVER_SIGN_BLOCK_SIGNATURE_SIZE);
    WMR_MEMSET(&bbtInfoHeaderStruct, 0, sizeof(bbtInfoHeaderStruct));
    WMR_MEMSET(&specialBlockHeaderStruct, 0, sizeof(specialBlockHeaderStruct));
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    if(specialBlockCache_get(0, &sbCacheEntry))
    {   
        if( sbCacheEntry.sbLocation[0].locationValid)
            bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DEV_SIGNATURE_IDX] = sbCacheEntry.sbLocation[0].location;
    }
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    if(sbCacheEntry.sbLocation[0].locationValid == FALSE32)
    {
        if (_ReadSpecialInfoBlock(0, (SpecialBlockHeaderStruct *)&bbtInfoHeaderStruct, NULL, 0, (UInt8*)BBT_BLOCK_SIGNATURE, BBT_BLOCK_SIGNATURE_SIZE, FALSE32, NULL) == FALSE32)
        {
            return FALSE32;
        }
    }
    WMR_MEMSET(&specialBlockHeaderStruct, 0, sizeof(SpecialBlockHeaderStruct));
    WMR_MEMCPY(specialBlockHeaderStruct.abSignature, NAND_DRIVER_SIGN_BLOCK_SIGNATURE, NAND_DRIVER_SIGN_BLOCK_SIGNATURE_SIZE);
    specialBlockHeaderStruct.dwDataSize = dwSignatureSize;
    if (pvoidSignature == NULL)
    {
        UInt16 signatureBlock=0xffff;
        Buffer *signV ;
        
        signV = BUF_Get(BUF_MAIN_AND_SPARE);
        if (!signV)
        {
            WMR_PRINT(ERROR, TEXT("BUF_Get failed in VFL_ReadBlockZeroSignatureNoHeader!\n"));
            return FALSE32;
        }
        
        while (_ReadSpecialInfoBlock(0, NULL, signV->pData, dwSignatureSize, (UInt8*)NAND_DRIVER_SIGN_BLOCK_SIGNATURE, NAND_DRIVER_SIGN_BLOCK_SIGNATURE_SIZE, FALSE32,&signatureBlock) == TRUE32)
        {
            if(signatureBlock != 0xffff)
            {
                pFuncTbl->Erase(0, _Cbn2Pbn(signatureBlock));
            }
            signatureBlock = 0xffff;
        }
        BUF_Release(signV);
        return TRUE32;
    }
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    specialBlockCache_invalidate(0, NAND_DRIVER_SIGN_BLOCK_SIGNATURE, 0);
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    boolRes = _WriteSpecialBlock(0, (UInt16)bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DEV_SIGNATURE_IDX], &specialBlockHeaderStruct, (UInt8*)pvoidSignature, dwSignatureSize);
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    if(boolRes == TRUE32 )
        specialBlockCache_set(0, specialBlockHeaderStruct.abSignature, bbtInfoHeaderStruct.adwSpecialBlocks[FPART_SPECIAL_BLK_DEV_SIGNATURE_IDX]);
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    return boolRes;
}
#endif /* ! AND_READONLY */

static BOOL32 VFL_ReadSignature(void * pvoidSignature, UInt32 dwSignatureSize)
{
    BOOL32 ret;
    if (FIL_GetFuncTbl()->SetWhiteningState)
    {
        FIL_GetFuncTbl()->SetWhiteningState(TRUE32);
    }
    ret = _ReadSpecialInfoBlock(0, NULL, (UInt8*)pvoidSignature, dwSignatureSize, (UInt8*)NAND_DRIVER_SIGN_BLOCK_SIGNATURE, NAND_DRIVER_SIGN_BLOCK_SIGNATURE_SIZE, FALSE32, NULL);
    if (!ret && FIL_GetFuncTbl()->SetWhiteningState)
    {
        // disable whitening and try again
        FIL_GetFuncTbl()->SetWhiteningState(FALSE32);
        ret = _ReadSpecialInfoBlock(0, NULL, (UInt8*)pvoidSignature, dwSignatureSize, (UInt8*)NAND_DRIVER_SIGN_BLOCK_SIGNATURE, NAND_DRIVER_SIGN_BLOCK_SIGNATURE_SIZE, FALSE32, NULL);
        if( !ret )
        {
            FIL_GetFuncTbl()->SetWhiteningState(TRUE32);
        }
        else
            WMR_PRINT(ALWAYS, TEXT("[FIL] Disable data whitening\n"));
    }
    return ret;
}

static BOOL32 UnderVFL_Init(LowFuncTbl *pFILFunctions, UInt32 dwOptions)
{
#ifndef AND_READONLY
    bSupportDevUniqueInfo = (dwOptions & FPART_INIT_OPTION_DEV_UNIQUE) ? TRUE32 : FALSE32;
    bSupportDiagCtrlInfo = (dwOptions & FPART_INIT_OPTION_DIAG_CTRL) ? TRUE32 : FALSE32;
#endif // ! AND_READONLY

    stFPartSignatureStyle = SIGNATURE_STYLE_NOTSET;
    pFuncTbl = pFILFunctions;
    stUnderVFLDeviceInfo.checkInitialBadType = (CheckInitialBadType)pFuncTbl->GetDeviceInfo(AND_DEVINFO_BBT_TYPE);
    stUnderVFLDeviceInfo.wBlocksPerCS = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CS);
    stUnderVFLDeviceInfo.wDiesPerCS = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_DIES_PER_CS);
    stUnderVFLDeviceInfo.wBlocksPerDie = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_DIE);
    stUnderVFLDeviceInfo.wDieStride = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_DIE_STRIDE);  
    stUnderVFLDeviceInfo.wBlockStride = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_BLOCK_STRIDE);
    stUnderVFLDeviceInfo.wLastBlock = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_LAST_BLOCK);
    stUnderVFLDeviceInfo.wUserBlocksPerCS = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_USER_BLOCKS_PER_CS);
    stUnderVFLDeviceInfo.wBytesPerPage = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    stUnderVFLDeviceInfo.wNumOfCEs = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_NUM_OF_CS);
    stUnderVFLDeviceInfo.wPagesPerBlock = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_PAGES_PER_BLOCK);
    stUnderVFLDeviceInfo.wBytesPerSpare = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_BYTES_PER_SPARE);
    stUnderVFLDeviceInfo.wBytesPerBLPage = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_BYTES_PER_BL_PAGE);
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    specialBlockCache_Init();
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    return TRUE32;
}

static void UnderVFL_Close(void)
{
#if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
    if( blockCacheArray != NULL )
    {
        WMR_FREE(blockCacheArray, CS_TOTAL * MAX_NUM_SPECIAL_BLOCKS_PER_CS * sizeof(specialBlockCache));
        blockCacheArray = NULL;
    }
#endif // #if !(defined(AND_DISABLE_SPECIAL_BLOCK_CACHE) && AND_DISABLE_SPECIAL_BLOCK_CACHE)
}

// Add:
// 1. read signature function that search for signature in block zero with no header (M68 style).
// 2. undervfl/pflash register.
// 3. add path through functions to fil.
// 4. need to change the factory reformat to format single fpartition (specified).

static BOOL32
VFL_ReadBlockZeroSignatureNoHeader(void * pvoidData, UInt32 dwSignatureSize)
{
    Buffer *pBuf;
    Int32 nRet;
    UInt16 wPageIdx;
    BOOL32 boolFoundValidData;
    UInt8  *pabData = (UInt8*)pvoidData;

    /*
     * Search block zero, bank zero for the driver signature.
     *
     * Sets boolSignatureFound to TRUE32 and sets nSig if a signature is found.
     * Sets boolBlk0IsClean appropriately.
     */
    boolFoundValidData = FALSE32;

    pBuf = BUF_Get(BUF_MAIN_AND_SPARE);
    if (!pBuf)
    {
        WMR_PRINT(ERROR, TEXT("BUF_Get failed in VFL_ReadBlockZeroSignatureNoHeader!\n"));
        return FALSE32;
    }
    for (wPageIdx = 0; wPageIdx < PAGES_PER_BLOCK; wPageIdx++)
    {
        nRet = pFuncTbl->ReadMaxECC(0, wPageIdx, pBuf->pData);
        if (nRet == FIL_U_ECC_ERROR)
        {
            nRet = pFuncTbl->ReadWithECC(0, wPageIdx, pBuf->pData, pBuf->pSpare, NULL, NULL, FALSE32);
        }
        /*
         * If we successfully read a page that contains a compatible signature,
         * save the signature block and consider ourselves done.
         */
        if (nRet == FIL_SUCCESS)
        {
            WMR_MEMCPY(pabData, pBuf->pData, dwSignatureSize);
            boolFoundValidData = TRUE32;
            break;
        }
    }
    BUF_Release(pBuf);
    return boolFoundValidData;
}

#ifndef AND_READONLY
static BOOL32
VFL_WriteBlockZeroSignatureNoHeader(void * pvoidData, UInt32 dwSignatureSize)
{
    Int32 nRetTemp;
    Buffer * pBuf;
    UInt16 wPageIdx;
    LowFuncTbl *pLowFuncTbl;
    UInt8  *pabData = (UInt8*)pvoidData;

    pLowFuncTbl = FIL_GetFuncTbl();

    pBuf = BUF_Get(BUF_MAIN_AND_SPARE);

    if (pBuf == NULL)
    {
        return FALSE32;
    }

    WMR_MEMSET(pBuf->pData, 0, BYTES_PER_PAGE);
    WMR_MEMSET(pBuf->pSpare, 0, BYTES_PER_METADATA_RAW);
    WMR_MEMCPY(pBuf->pData, pabData, dwSignatureSize);

    /* erase the first block */
    nRetTemp = pLowFuncTbl->Erase(0, 0);

    if (nRetTemp != FIL_SUCCESS)
    {
        return FALSE32;
    }
    if (pvoidData != NULL && dwSignatureSize != 0)
    {
        for (wPageIdx = 0; wPageIdx < PAGES_PER_BLOCK; wPageIdx++)
        {
            nRetTemp = pLowFuncTbl->WriteMaxECC(0, wPageIdx, pBuf->pData);

            if (nRetTemp != FIL_SUCCESS)
            {
                return FALSE32;
            }
        }
    }
    BUF_Release(pBuf);
    return TRUE32;
}
#endif // ! AND_READONLY
void FPART_SetSignatureStyle(void * pvoidFPartFunctions, FPartSignatureStyle fpartSignatureStyle)
{
    FPartFunctions * pFPartFunctions = (FPartFunctions *)pvoidFPartFunctions;

    stFPartSignatureStyle = fpartSignatureStyle;
    switch (stFPartSignatureStyle)
    {
    case SIGNATURE_STYLE_BLOCKZERO_NOHEADER:
        pFPartFunctions->ReadSignature = VFL_ReadBlockZeroSignatureNoHeader;
#ifndef AND_READONLY
        pFPartFunctions->WriteSignature = VFL_WriteBlockZeroSignatureNoHeader;
#endif
        break;

    case SIGNATURE_STYLE_RESERVEDBLOCK_HEADER:
        pFPartFunctions->ReadSignature = VFL_ReadSignature;
#ifndef AND_READONLY
        pFPartFunctions->WriteSignature = VFL_WriteSignature;
#endif
        break;

    default:
        pFPartFunctions->ReadSignature = NULL;
#ifndef AND_READONLY
        pFPartFunctions->WriteSignature = NULL;
#endif
    }
}

void FPart_Register(FPartFunctions * pFPartFunctions)
{
#ifndef AND_READONLY
    pFPartFunctions->WriteInitialBBT = VFL_WriteInitialBBT;
    pFPartFunctions->FactoryReformat = VFL_FactoryReformat;
    pFPartFunctions->Neuralize = VFL_Neuralize;
    pFPartFunctions->WriteDeviceUniqueInfo = VFL_WriteDeviceUniqueInfo;
    pFPartFunctions->WriteDiagnosticInfo = VFL_WriteDiagControlInfo;
    pFPartFunctions->WriteSignature = NULL;
#endif // ! AND_READONLY
    pFPartFunctions->ReadDeviceUniqueInfo = VFL_ReadDeviceUniqueInfo;
    pFPartFunctions->ReadDiagnosticInfo = VFL_ReadDiagControlInfo;
    pFPartFunctions->VerifyProductionFormat = VFL_VerifyProductionFormat;
    pFPartFunctions->ReadSignature = NULL;
    pFPartFunctions->Init = UnderVFL_Init;
    pFPartFunctions->Close = UnderVFL_Close;
    pFPartFunctions->GetLowFuncTbl = FIL_GetFuncTbl;
    pFPartFunctions->SetSignatureStyle = FPART_SetSignatureStyle;
}

