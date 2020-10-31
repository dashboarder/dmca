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
#include "VSVFLTypes.h"
#include "FPart.h"
#include "VFL.h"
#include "FIL.h"

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
#define     VFL_LOG_PRINT(x)            //WMR_RTL_PRINT(x)
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
#ifndef AND_READONLY
static UInt16 *pawPbnOriginal = NULL;
static UInt16 *pawPbn = NULL;
static UInt16 *pawPbnUnmapped = NULL;
#endif
static UInt16 *pawPbnRemapped = NULL;
static UInt32 *padwPpnRemapped = NULL;
static VFLMeta *pstVFLMeta = NULL;
static UInt32 *pstadwScatteredReadWritePpn = NULL;
static UInt16 *pstawScatteredReadWriteCS = NULL;

static LowFuncTbl   *pFuncTbl = NULL;
static UInt8       *pstabCSBBTArea[16];
static VSVFLWMRDeviceInfo stVSVFLWMRDeviceInfo;
static FPartFunctions stFPartFunctions;

// vendor specific conversion functions (bank, Tpn to CS, Ppn)
static void (*_pfnConvertP2C)(UInt32 dwBank, UInt32 dwPpn, UInt32* pdwCE, UInt32* pdwCpn) = NULL;
static void (*_pfnConvertC2P)(UInt32 dwCE, UInt32 dwCpn, UInt32* pdwBank, UInt32* pdwPpn) = NULL;
static BOOL32 _SetBankToCSConvertFunctions(VendorSpecificType vendorSpecificType);
static UInt16 _GetBanksPerCS(VendorSpecificType vendorSpecificType);
static BOOL32 _SetV2PFunctions(VendorSpecificType vendorSpecificType);
static void (*_ConvertP2T)(UInt16 wPbn, UInt16* pwBank, UInt16* pwTbn);
static void (*_ConvertT2P)(UInt16 wBank, UInt16 wTbn, UInt16* pwPbn);

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
#if (!AND_DISABLE_READ_MULTIPLE)
static BOOL32   VFL_ReadMultiplePagesInVb(UInt16 wVbn, UInt16 wStartPOffset, UInt16 wNumPagesToRead, UInt8 * pbaData, UInt8 * pbaSpare, BOOL32 * pboolNeedRefresh, UInt8 * pdwaSectorStats, BOOL32 bDisableWhitening);
#endif /* if (!AND_DISABLE_READ_MULTIPLE) */
static BOOL32   VFL_ReadScatteredPagesInVb(UInt32 * padwVpn, UInt16 wNumPagesToRead, UInt8 * pbaData, UInt8 * pbaSpare, BOOL32 * pboolNeedRefresh, UInt8 * pdwaSectorStats, BOOL32 bDisableWhitening, Int32* actualStatus);
static Int32    VFL_Read(UInt32 nVpn, Buffer *pBuf, BOOL32 bCleanCheck, BOOL32 bMarkECCForScrub, BOOL32 * pboolNeedRefresh, UInt8 * pdwaSectorStats, BOOL32 bDisableWhitening);
static UInt16*  VFL_GetFTLCxtVbn(void);
//static BOOL32	VFL_GetAddress			(ANDAddressStruct * pANDAddressStruct);
static BOOL32   VFL_GetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 * pdwStructSize);
static UInt32   VFL_GetDeviceInfo(UInt32 dwParamType);

static UInt32 gdwMaxGlobalAge;
static VFLFailureDetails gstLastFailure;
#ifdef AND_COLLECT_STATISTICS
static VFLStatistics stVFLStatistics;
#endif
#ifndef AND_READONLY
#define RETIRE_ON_REFRESH_TRESHOLD 2
static UInt8 * vflBlockRefreshTrigger = NULL;
#endif
/*****************************************************************************/
/* Definitions                                                               */
/*****************************************************************************/
#define     _GetNumOfSuBlks()           (pstVFLMeta[0].stVFLCxt.wNumOfVFLSuBlk)
#define     _GetNumOfFTLSuBlks()        (pstVFLMeta[0].stVFLCxt.wNumOfFTLSuBlk)
#define     _GetFTLType()               (pstVFLMeta[0].stVFLCxt.dwFTLType)
#define     GET_VFLCxt(v)               (&(pstVFLMeta + (v))->stVFLCxt)
#define     _Cbn2Ppn(Cbn, POffset)       (_Cbn2Pbn(Cbn) * BLOCK_STRIDE + (POffset)) /* translate block + pages offset to page address*/
#define     _Vpn2Vbn(dwVpn)             ((UInt16)((UInt32)(dwVpn) / PAGES_PER_SUBLK))
#define     _Vbn2Tbn(wVbn)              (wVbn)
#define     _ReserveIdx2Bank(wIdx)      ((wIdx) / VFL_RESERVED_SECTION_SIZE_BANK)
#define     _ReserveIdx2Tbn(wIdx)       (((wIdx) % VFL_RESERVED_SECTION_SIZE_BANK) + _GetNumOfSuBlks())
#define     _Bank2CS(wBank)             ((wBank) % CS_TOTAL)
#define     _Vpn2Bank(dwVpn)            ((UInt16)((dwVpn) % BANKS_TOTAL))
#define     _CSBank2Bank(wCS, wBank)    (((wBank) * CS_TOTAL) + (wCS))
#define     _Bank2CSBank(wBank)         ((wBank) / CS_TOTAL)
#define     _Cbn2Pbn(Cbn)               (((Cbn) / BLOCKS_PER_DIE ) * DIE_STRIDE + ((Cbn) % BLOCKS_PER_DIE)) //contiguous physical block to non-contiguous

/*****************************************************************************/
/* Global variables	redefinitions (WMRDeviceInfo)							 */
/*****************************************************************************/
#define     PAGES_PER_BLOCK         (stVSVFLWMRDeviceInfo.wPagesPerBlock)
#define     PAGES_PER_SUBLK         (stVSVFLWMRDeviceInfo.wPagesPerSuBlk)

#define     USER_BLOCKS_PER_BANK    (USER_BLOCKS_TOTAL / BANKS_PER_CS)
#define     USER_BLOCKS_TOTAL        (stVSVFLWMRDeviceInfo.wUserBlocksPerCS)
#define     USER_SUBLKS_TOTAL       (stVSVFLWMRDeviceInfo.wNumOfFTLSuBlks)
#define     USER_PAGES_TOTAL        (stVSVFLWMRDeviceInfo.dwUserPagesTotal)

#define     BYTES_PER_SECTOR        (WMR_SECTOR_SIZE)
#define     BYTES_PER_PAGE          (stVSVFLWMRDeviceInfo.wBytesPerPage)
#define     PAGES_PER_CS            (BLOCKS_PER_CS * BLOCK_STRIDE)

/*****************************************************************************/
/* Global variables	redefinitions (WMRConfig)					             */
/*****************************************************************************/
#define     BANKS_TOTAL             (stVSVFLWMRDeviceInfo.wNumOfBank)
#define     CS_TOTAL                (stVSVFLWMRDeviceInfo.wNumOfCEs)
#define     BLOCKS_PER_CS           (stVSVFLWMRDeviceInfo.wBlocksPerCS)
#define     BANKS_PER_CS            (stVSVFLWMRDeviceInfo.wNumOfBanksPerCS)
#define     BLOCKS_PER_BANK         (stVSVFLWMRDeviceInfo.wBlocksPerBank)
#define     BLOCKS_PER_DIE          (stVSVFLWMRDeviceInfo.wBlocksPerDie)
#define     DIE_STRIDE              (stVSVFLWMRDeviceInfo.wDieStride)
#define     BLOCK_STRIDE            (stVSVFLWMRDeviceInfo.wBlockStride)

/*****************************************************************************/
/* VFL local function prototypes                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Static function prototypes                                                */
/*****************************************************************************/
static void         _GetVFLSpare(UInt32 *pAge, UInt8  *pStatus,
                                 UInt8  *pbType, UInt8  *pSBuf);
static BOOL32       _LoadVFLCxt(UInt16 wCS, UInt8  *pabBBT);
static UInt16       _RemapBlock(UInt16 wCS, UInt16 wPbn, BOOL32 *pboolAligned, BOOL32 *pboolCorrectBank);
static VFLCxt *     _GetNewestVFLCxt(void);
static BOOL32       _GetFactoryBBTBuffer(void* pvoidStructBuffer, UInt32* pdwStructSize);
static void         _ReportECCFailureMultiple(UInt16 wVbn);
static void         _ReportECCFailureSingle(UInt16 wFailingCE, UInt16 dwPPN);
#ifndef AND_READONLY
static UInt16       _CalcNumOfVFLSuBlk(void);
static BOOL32       _SetFTLType(UInt32 dwFTLType);
static BOOL32       _SetNumOfFTLSuBlks(UInt16 wNumOfFTLSuBlks);
static BOOL32       _StoreVFLCxt(UInt16 wCS);
static void         _SetVFLSpare(UInt32 *pAge, UInt8  *pSBuf);
static BOOL32       _ReplaceBadBlock(UInt16 wCSIdx, UInt16 wIdx);
static BOOL32       _SetNewFTLParams(void);
static BOOL32       _SetFTLType(UInt32 dwFTLType);
static BOOL32       _SetNewFTLType(UInt32 dwFTLType);
static BOOL32       _SetNewNumOfFTLSuBlks(UInt16 wNumOfFTLSuBlks);
static BOOL32       _SetNumOfFTLSuBlks(UInt16 wNumOfFTLSuBlks);
static BOOL32       _IsBlockInScrubList(UInt16 wCS, UInt16 wPbn);
static BOOL32       _AddBlockToScrubList(UInt16 wCS, UInt16 wPbn);
static BOOL32       _RemoveBlockFromScrubList(UInt16 wCS, UInt16 wPbn);
static BOOL32       _WriteVFLCxtToFlash(UInt16 wCS);
static void         _MarkWriteFailBlockForDeleteWithVpn(UInt16 wFailingCE, UInt32 dwVpn);
static void         _MarkWriteFailBlockForDeleteWithPbn(UInt16 wFailingCE, UInt16 wPbn);
static void         _MarkWriteScatteredFailBlockForDelete(UInt16 wFailingCE, UInt32 dwVpn,
                                                          UInt32 wFailingPage, UInt16 ceList[], UInt32 physPageList[], UInt16 wPbnUnmapped[], UInt16 wPagesToWriteNow);
static void         _MarkWriteMultipleFailBlockForDelete(UInt16 wFailingCE, UInt32 dwVpn,
                                                         UInt32 wFailingPage, UInt16 awPbnUnmapped[], UInt16 awPbnRemapped[], UInt32 adwPpnRemapped[], UInt16 wPagesToWriteNow);
static void         _ReportWriteFailureMultiple(UInt16 wFailingCE, UInt32 dwVbn, Int32 nFILRet);
static void         _ReportWriteFailureSingle(UInt16 wFailingCE, UInt32 dwPPN, Int32 nFILRet);
static void         _ReportWriteScatteredFailBlock(UInt16 wFailingCE, UInt32 wVbn,
                                                          UInt32 wFailingPage, UInt16 ceList[], UInt32 physPageList[], UInt16 wPagesToWriteNow, Int32 nFILRet);
static void         _ReportWriteMultipleFailBlock(UInt16 wFailingCE, UInt32 wVbn,
                                                         UInt32 wFailingPage, UInt32 adwPpnRemapped[], UInt16 wPagesToWriteNow, Int32 nFILRet);
static void         _ReportEraseFailureMultiple(UInt16 wFailingCE, UInt16 dwVbn, Int32 nFILRet);
static void         _ReportEraseFailureSingle(UInt16 wFailingCE, UInt16 dwBlock, Int32 nFILRet);
#endif

#define WMR_NUM_OF_ERASE_TRIALS_FOR_DATA_BLOCKS     (3)

#define VFL_NUM_OF_VFL_CXT_COPIES                   (8)
#define VFL_MAX_NUM_OF_VFL_CXT_ERRORS               (3)

void _Vpn2Cbn(UInt32 dwVpn, UInt16 *pwCS, UInt16 *pdwCbn)
{
    UInt16 wVbn, wBank, wTbn, wPbn;
    
    wVbn = _Vpn2Vbn(dwVpn);
    wBank = _Vpn2Bank(dwVpn);
    wTbn = _Vbn2Tbn(wVbn);
    _ConvertT2P(wBank, wTbn, &wPbn);
    *pwCS = _Bank2CS(wBank);
    *pdwCbn = _RemapBlock(*pwCS, wPbn, NULL, NULL);
}

void _Vpn2Ppn(UInt32 dwVpn, UInt16 *pwCS, UInt32 *pdwPpn)
{
    UInt16 wVbn, wBank, wTbn, wPbn, wPOffset;

    wVbn = _Vpn2Vbn(dwVpn);
    wBank = _Vpn2Bank(dwVpn);
    wTbn = _Vbn2Tbn(wVbn);
    _ConvertT2P(wBank, wTbn, &wPbn);
    *pwCS = _Bank2CS(wBank);
    wPbn = _RemapBlock(*pwCS, wPbn, NULL, NULL);
    wPOffset = (UInt16)((dwVpn % PAGES_PER_SUBLK) / BANKS_TOTAL);
    *pdwPpn = _Cbn2Ppn(wPbn, wPOffset);
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

static BOOL32 _CheckVFLCxtDataIsUntouched(UInt16 wCS)
{
    UInt32 dwTempCheckSum;
    UInt32 dwTempXor;
    static UInt32 dwCount = 0;

    WMR_ASSERT(wCS < CS_TOTAL);

    dwCount++;
    _NANDCheckSum((UInt32*)&pstVFLMeta[wCS], (sizeof(VFLMeta) - (2 * sizeof(UInt32))), &dwTempCheckSum, &dwTempXor);
    if (!((dwTempCheckSum == pstVFLMeta[wCS].dwCheckSum) && (dwTempXor == pstVFLMeta[wCS].dwXorSum)) )
    {
        return FALSE32;
    }
    return TRUE32;
}
static void _UpdateVFLCxtChecksum(UInt16 wCS)
{
    _NANDCheckSum((UInt32*)&pstVFLMeta[wCS], (sizeof(VFLMeta) - (2 * sizeof(UInt32))), &pstVFLMeta[wCS].dwCheckSum, &pstVFLMeta[wCS].dwXorSum);
}

typedef struct
{
    void (*UpdateChecksum)(UInt16 wCS);
    BOOL32 (*VerifyChecksum)(UInt16 wCS);
} ChecksumFuncStruct;

static ChecksumFuncStruct vflChecksumFunc;

#define RECALC_VFL_CXT_CHECK_SUM(cs)    vflChecksumFunc.UpdateChecksum(cs)

#define CHECK_VFL_CXT_CHECK_SUM(cs) \
    { \
        if (vflChecksumFunc.VerifyChecksum(cs) == FALSE32) { \
            WMR_ASSERT(0); } \
    }

#define CHECK_VFL_CXT_CHECK_SUM_NO_ASSERT(cs)   vflChecksumFunc.VerifyChecksum(cs)

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


static void _ReportECCFailureMultiple(UInt16 wVbn)
{
    UInt16 wBankIdx;

    WMR_MEMSET(&gstLastFailure, VFL_VALUE_UNKNOWN, sizeof(VFLFailureDetails));
    gstLastFailure.mode = VFLFailUECC;

    // Restrict the number reported, since this is really broad
    // Hopefully someone will read-single through this and find the real error
    for (wBankIdx = 0; 
         (wBankIdx < BANKS_TOTAL) && (wBankIdx < VFL_MAX_FAILURE_REGIONS); 
         ++wBankIdx )
    {
        gstLastFailure.wCE[wBankIdx] = _Bank2CS(wBankIdx);
        _ConvertT2P(wBankIdx, _Vbn2Tbn(wVbn), &gstLastFailure.wPhysicalBlock[wBankIdx]);
    }
}

static void  _ReportECCFailureSingle(UInt16 wFailingCE, UInt16 dwPPN)
{
    WMR_MEMSET(&gstLastFailure, VFL_VALUE_UNKNOWN, sizeof(VFLFailureDetails));
    gstLastFailure.mode = VFLFailUECC;
    gstLastFailure.wCE[0] = wFailingCE;
    gstLastFailure.wPhysicalBlock[0] = dwPPN / PAGES_PER_BLOCK;
    gstLastFailure.dwPhysicalPage = dwPPN;
}


#ifndef AND_READONLY
static void _MarkWriteFailBlockForDeleteWithVpn(UInt16 wFailingCE, UInt32 dwVpn)
{
    UInt16 wPbn;
    UInt16 wBankIdx;

    UInt16 wVbn = _Vpn2Vbn(dwVpn);

    if (wFailingCE < CS_TOTAL)
    {
        for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
        {
            //scrub all blocks in failed CE
            if (_Bank2CS(wBankIdx) == wFailingCE)
            {
                _ConvertT2P(wBankIdx, _Vbn2Tbn(wVbn), &wPbn);
                _AddBlockToScrubList(wFailingCE, wPbn);
                VFL_WRN_PRINT((TEXT("[VFL:WRN] Marked write fail block for delete, CE %d, block 0x%X (line:%d)\n"), wFailingCE, wPbn, __LINE__));
            }
        }
    }
    else
    {
        VFL_WRN_PRINT((TEXT("[VFL:WRN] invalid CE 0x%X, no marking done (line:%d).\n"), wFailingCE, __LINE__));
    }
}

static void _MarkWriteFailBlockForDeleteWithPbn(UInt16 wFailingCE, UInt16 wPbn)
{
    _AddBlockToScrubList(wFailingCE, wPbn);
    VFL_WRN_PRINT((TEXT("[VFL:WRN] Marked write fail block for delete, CE %d, block 0x%X (line:%d)\n"), wFailingCE, wPbn, __LINE__));
}


static void _MarkWriteScatteredFailBlockForDelete(UInt16 wFailingCE, UInt32 dwVpn, UInt32 wFailingPage, 
                                                  UInt16 ceList[], UInt32 physPageList[], UInt16 wPbnUnmapped[], UInt16 wPagesToWriteNow)
{
    BOOL32 foundFailingPageInfo = FALSE32;
    UInt16 wIndex;
    for ( wIndex = 0; wIndex < wPagesToWriteNow; wIndex++ )
    {
        if ( (ceList[wIndex] == wFailingCE) && (physPageList[wIndex] == wFailingPage) )
        {
            _MarkWriteFailBlockForDeleteWithPbn(wFailingCE, wPbnUnmapped[wIndex]);
            foundFailingPageInfo = TRUE32;
            break;
        }
    }
    if ( !foundFailingPageInfo )
    {
        VFL_WRN_PRINT((TEXT("[VFL:WRN] Failed to find failing page (pg: 0x%x, CE: %d, CS[] @ %p, pg[] @ %p, n=%d)\n"),
              wFailingPage,wFailingCE,ceList,physPageList,wPagesToWriteNow));
        _MarkWriteFailBlockForDeleteWithVpn(wFailingCE, dwVpn);
    }
}

static void _MarkWriteMultipleFailBlockForDelete(UInt16 wFailingCE, UInt32 dwVpn, UInt32 wFailingPage, 
                                                 UInt16 awPbnUnmapped[], UInt16 awPbnRemapped[], UInt32 adwPpnRemapped[], UInt16 wPagesToWriteNow)
{
    BOOL32 foundFailingPageInfo = FALSE32;
    int iSlice = -1;
    UInt32 wIndex;
    VFL_WRN_PRINT((TEXT("[VFL:WRN] Scanning for page 0x%x\n"),wFailingPage));
    for ( wIndex = 0; wIndex < wPagesToWriteNow; wIndex++ )
    {
        if ( 0 == (wIndex % BANKS_TOTAL) )
        {
            iSlice++;
        }
        if ( (_Bank2CS(wIndex % BANKS_TOTAL) == wFailingCE) && ((adwPpnRemapped[wIndex % BANKS_TOTAL] + iSlice) == wFailingPage) )
        {
            VFL_WRN_PRINT((TEXT("[VFL:WRN] Found page on slice %d (of %d), pg: 0x%x, blk: 0x%x\n"),
                           iSlice,wPagesToWriteNow,(adwPpnRemapped[wIndex % BANKS_TOTAL] + iSlice),awPbnRemapped[wIndex % BANKS_TOTAL]));
            _MarkWriteFailBlockForDeleteWithPbn(wFailingCE, awPbnUnmapped[wIndex % BANKS_TOTAL]);
            foundFailingPageInfo = TRUE32;
            break;
        }
    }
    if ( !foundFailingPageInfo )
    {
        VFL_WRN_PRINT((TEXT("[VFL:WRN] Failed to find failing page!  Falling back to CE/Vpn\n")));
        _MarkWriteFailBlockForDeleteWithVpn(wFailingCE, dwVpn);
    }
}

static void _ReportWriteFailureMultiple(UInt16 wFailingCE, UInt32 dwVbn, Int32 nFILRet)
{
    UInt16 wBankIdx;
    UInt16 wFailIdx;

    WMR_MEMSET(&gstLastFailure, VFL_VALUE_UNKNOWN, sizeof(VFLFailureDetails));
    if(nFILRet == FIL_CRITICAL_ERROR)
    {
        gstLastFailure.mode = VFLFailWriteTimeOut;
        return;
    }
    gstLastFailure.mode = VFLFailWrite;

    wFailIdx = 0;
    for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; ++wBankIdx)
    {
        if (_Bank2CS(wBankIdx) == wFailingCE)
        {
            gstLastFailure.wCE[wFailIdx] = wFailingCE;
            _ConvertT2P(wBankIdx, _Vbn2Tbn(dwVbn), &gstLastFailure.wPhysicalBlock[wFailIdx]);
            wFailIdx += 1;
        }
    }
}

static void _ReportWriteFailureSingle(UInt16 wFailingCE, UInt32 dwPPN, Int32 nFILRet)
{
    WMR_MEMSET(&gstLastFailure, VFL_VALUE_UNKNOWN, sizeof(VFLFailureDetails));
    
    if(nFILRet == FIL_CRITICAL_ERROR)
    {
        gstLastFailure.mode = VFLFailWriteTimeOut;
        return;
    }

    gstLastFailure.mode = VFLFailWrite;

    gstLastFailure.wCE[0] = wFailingCE;
    gstLastFailure.wPhysicalBlock[0] = dwPPN / PAGES_PER_BLOCK;
    gstLastFailure.dwPhysicalPage = dwPPN;
}

static void _ReportWriteScatteredFailBlock(UInt16 wFailingCE, UInt32 wVbn, UInt32 wFailingPage, 
                                                  UInt16 ceList[], UInt32 physPageList[], UInt16 wPagesToWriteNow, Int32 nFILRet)
{
    BOOL32 foundFailingPageInfo = FALSE32;
    UInt16 wIndex;
    for ( wIndex = 0; wIndex < wPagesToWriteNow; wIndex++ )
    {
        if ( (ceList[wIndex] == wFailingCE) && (physPageList[wIndex] == wFailingPage) )
        {
            _ReportWriteFailureSingle(wFailingCE,wFailingPage, nFILRet);
            foundFailingPageInfo = TRUE32;
            break;
        }
    }
    if ( !foundFailingPageInfo )
    {
        VFL_ERR_PRINT((TEXT("Failed to find failing page (pg: 0x%x, CE: %d, CS[] @ %p, pg[] @ %p, n=%d)\n"),
              wFailingPage,wFailingCE,ceList,physPageList,wPagesToWriteNow));
        _ReportWriteFailureMultiple(wFailingCE, wVbn, nFILRet);
    }
}

static void _ReportWriteMultipleFailBlock(UInt16 wFailingCE, UInt32 wVbn, UInt32 wFailingPage, 
                                                 UInt32 adwPpnRemapped[], UInt16 wPagesToWriteNow, Int32 nFILRet)
{
    BOOL32 foundFailingPageInfo = FALSE32;
    int iSlice = -1;
    UInt32 wIndex;
    for ( wIndex = 0; wIndex < wPagesToWriteNow; wIndex++ )
    {
        if ( 0 == (wIndex % BANKS_TOTAL) )
        {
            iSlice++;
        }
        if ( (_Bank2CS(wIndex % BANKS_TOTAL) == wFailingCE) && ((adwPpnRemapped[wIndex % BANKS_TOTAL] + iSlice) == wFailingPage) )
        {
            _ReportWriteFailureSingle(wFailingCE, wFailingPage, nFILRet);
            foundFailingPageInfo = TRUE32;
            break;
        }
    }
    if ( !foundFailingPageInfo )
    {
        _ReportWriteFailureMultiple(wFailingCE, wVbn, nFILRet);
    }
}

static void _ReportEraseFailureMultiple(UInt16 wFailingCE, UInt16 dwVbn, Int32 nFILRet)
{
    UInt16 wBankIdx;
    UInt16 wFailIdx;

    WMR_MEMSET(&gstLastFailure, VFL_VALUE_UNKNOWN, sizeof(VFLFailureDetails));
    if(nFILRet == FIL_CRITICAL_ERROR)
    {
        gstLastFailure.mode = VFLFailEraseTimeOut;
        return;
    }
    gstLastFailure.mode = VFLFailErase;

    wFailIdx = 0;
    for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; ++wBankIdx)
    {
        if (_Bank2CS(wBankIdx) == wFailingCE)
        {
            gstLastFailure.wCE[wFailIdx] = wFailingCE;
            _ConvertT2P(wBankIdx, _Vbn2Tbn(dwVbn), &gstLastFailure.wPhysicalBlock[wFailIdx]);
            wFailIdx += 1;
        }
    }
}

static void _ReportEraseFailureSingle(UInt16 wFailingCE, UInt16 dwBlock, Int32 nFILRet)
{
    WMR_MEMSET(&gstLastFailure, VFL_VALUE_UNKNOWN, sizeof(VFLFailureDetails));
    if(nFILRet == FIL_CRITICAL_ERROR)
    {
        gstLastFailure.mode = VFLFailEraseTimeOut;
        return;
    }
    gstLastFailure.mode = VFLFailErase;

    gstLastFailure.wCE[0] = wFailingCE;
    gstLastFailure.wPhysicalBlock[0] = dwBlock;
}

static BOOL32 _AddBlockToScrubList(UInt16 wCS, UInt16 wPbn)
{
    VFLCxt * pVFLCxt = GET_VFLCxt(wCS);

    if (_IsBlockInScrubList(wCS, wPbn) == TRUE32)
    {
        return TRUE32;
    }

    /* make sure we do not get close to the wBadMapTableMaxIdx */
    if (pVFLCxt->wScrubIdx >= VFL_SCRUB_LIST_SIZE)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _AddBlockToScrubList pVFLCxt->wScrubIdx = %d wCS = %d (line:%d)\n"), pVFLCxt->wScrubIdx, wCS, __LINE__));
        return FALSE32;
    }
    CHECK_VFL_CXT_CHECK_SUM(wCS);
    pVFLCxt->awScrubList[pVFLCxt->wScrubIdx] = wPbn;
    pVFLCxt->wScrubIdx++;
    RECALC_VFL_CXT_CHECK_SUM(wCS);
    return _StoreVFLCxt(wCS);
}

static BOOL32 _IsBlockInScrubList(UInt16 wCS, UInt16 wPbn)
{
    VFLCxt * pVFLCxt = GET_VFLCxt(wCS);
    UInt16 wScrubIdx = 0;

    for (wScrubIdx = 0; wScrubIdx < pVFLCxt->wScrubIdx; wScrubIdx++)
    {
        if (pVFLCxt->awScrubList[wScrubIdx] == wPbn)
        {
            return TRUE32;
        }
    }
    return FALSE32;
}

static BOOL32 _RemoveBlockFromScrubList(UInt16 wCS, UInt16 wPbn)
{
    VFLCxt * pVFLCxt = GET_VFLCxt(wCS);
    UInt16 wScrubIdx = 0;

    if (_IsBlockInScrubList(wCS, wPbn) == FALSE32)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _RemoveBlockFromScrubList(%d, 0x%X) failed calling _IsBlockInScrubList (line:%d)\n"), wCS, wPbn, __LINE__));
        return FALSE32;
    }
    for (wScrubIdx = 0; wScrubIdx < pVFLCxt->wScrubIdx; wScrubIdx++)
    {
        if (pVFLCxt->awScrubList[wScrubIdx] == wPbn)
        {
            CHECK_VFL_CXT_CHECK_SUM(wCS);
            pVFLCxt->awScrubList[wScrubIdx] = 0;
            /* we found the block - if it is in the middle of the list replace it with the bottom one */
            if ((wScrubIdx != pVFLCxt->wScrubIdx) && (pVFLCxt->wScrubIdx != 1))
            {
                pVFLCxt->awScrubList[wScrubIdx] = pVFLCxt->awScrubList[pVFLCxt->wScrubIdx - 1];
            }
            pVFLCxt->wScrubIdx--;
            RECALC_VFL_CXT_CHECK_SUM(wCS);
            return _StoreVFLCxt(wCS);
        }
    }
    return FALSE32;
}

#if AND_SUPPORT_BLOCK_BORROWING

static void _MarkReservedBlock(const UInt16 wCS, const UInt16 wPbn, const UInt32 dwReplacementIdx, const UInt16 wMark)
{
    VFLCxt *pVFLCxt = &pstVFLMeta[wCS].stVFLCxt;
    UInt8 *pabBBT = pstabCSBBTArea[wCS];
    
    // Make sure we aren't being asked to lend a bad block
    WMR_ASSERT(_isBlockGood(pabBBT, wPbn));

    _MarkBlockAsBadInBBT(pabBBT, wPbn);
    
    // Find the replacement index of the block we've been asked to lend
    WMR_ASSERT(dwReplacementIdx < (UInt32)VFL_RESERVED_SECTION_SIZE_CS);
    WMR_ASSERT(VFL_BAD_MAP_TABLE_AVAILABLE_MARK == pVFLCxt->awBadMapTable[dwReplacementIdx]);

    // Mark that block as used (but don't change replacement index
    pVFLCxt->awBadMapTable[dwReplacementIdx] = wMark;
}

static void _findBestBlockToBorrow(UInt16 *pdwCE, UInt16 *pdwCbn, UInt32 *pdwReplacementIdx)
{
    // Find a CE/Block pair that is eligible to borrow
    UInt16 wBank;
    UInt16 wCS;
    UInt16 wSearchIdx;
    UInt16 wStartIdx;
    UInt16 wEndIdx;
    
    UInt32 dwMostAvailableBlocks = 0;
    UInt32 dwBestBank = ~0;
    UInt32 dwBestCS = ~0;
    
    VFLCxt *pVFLCxt = NULL;
    
    // Select the bank with the most replacement blocks available
    for (wCS = 0; wCS < CS_TOTAL; ++wCS)
    {
        UInt16 *pawReplacements = pstVFLMeta[wCS].stVFLCxt.awReplacementIdx;
        pVFLCxt = &pstVFLMeta[wCS].stVFLCxt;
        for (wBank = 0; wBank < BANKS_PER_CS; ++wBank)
        {
            UInt32 dwAvailableBlocks = 0;
            for(wSearchIdx = pawReplacements[wBank]; 
                wSearchIdx < ((wBank + 1) * VFL_RESERVED_SECTION_SIZE_BANK); 
                ++wSearchIdx)
            {
                if (VFL_BAD_MAP_TABLE_AVAILABLE_MARK == pVFLCxt->awBadMapTable[wSearchIdx])
                {
                    dwAvailableBlocks++;
                }
            }
            if (dwAvailableBlocks > dwMostAvailableBlocks)
            {
                dwMostAvailableBlocks = dwAvailableBlocks;
                dwBestCS = wCS;
                dwBestBank = wBank;
            }
        }
    }
    // Make sure we found a candidate
    if (!((dwBestCS < CS_TOTAL) && (dwBestBank < BANKS_PER_CS)))
    {
#if (defined(AND_SIMULATOR) && AND_SIMULATOR)
        WMR_SIM_EXIT("Out of blocks for borrowing\n");
#endif // AND_SIMULATOR
        WMR_PANIC("Out of blocks for borrowing\n");
    }
    
    // Find the last replacement block on that Bank/CS
    pVFLCxt = &pstVFLMeta[dwBestCS].stVFLCxt;
    // Calculate the last reserved area index for the selected bank
    wStartIdx = ((dwBestBank + 1) * VFL_RESERVED_SECTION_SIZE_BANK) - 1;
    wEndIdx = pVFLCxt->awReplacementIdx[dwBestBank];
    for (wSearchIdx = wStartIdx; wSearchIdx >= wEndIdx; --wSearchIdx)
    {
        if (VFL_BAD_MAP_TABLE_AVAILABLE_MARK == pVFLCxt->awBadMapTable[wSearchIdx])
        {
            UInt16 wPbn;
            *pdwCE = dwBestCS;
            _ConvertT2P(_CSBank2Bank(dwBestCS, _ReserveIdx2Bank(wSearchIdx)), _ReserveIdx2Tbn(wSearchIdx), &wPbn);
            *pdwCbn = wPbn;
            *pdwReplacementIdx = wSearchIdx;
            return;
        }
    }
    WMR_PANIC("No available blocks found CE %d Bank %d\n", (UInt32) dwBestCS, (UInt32) dwBestBank);
}

static BOOL32 _stressBlock(UInt16 wCS, UInt16 wCbn)
{
    // Initial block screening intended to weed out beginning-of-bathtub failures
#define TEST_ITERATIONS (3)
    const UInt8 bTestPattern[TEST_ITERATIONS] = {0x00, 0xA5, 0x5A};
    UInt32 dwIteration;
    Int32 andStatus;
    UInt32 dwPageOffset;
    BOOL32 result = FALSE32;
    Buffer *pBuf = BUF_Get(BUF_MAIN_AND_SPARE);
    Buffer *pBufVerify = BUF_Get(BUF_MAIN_AND_SPARE);
    
    WMR_ASSERT(NULL != pBuf);
    WMR_ASSERT(NULL != pBufVerify);

    for (dwIteration = 0; dwIteration < TEST_ITERATIONS; ++dwIteration)
    {
        andStatus = pFuncTbl->Erase(wCS, _Cbn2Pbn(wCbn));
        if (ANDErrorCodeOk != andStatus)
        {
            WMR_PRINT(ERROR, "Iteration %d write CE %d Block %d failed 0x%08x\n", 
                      dwIteration, (UInt32) wCS, _Cbn2Pbn(wCbn), andStatus);
            result = FALSE32;
            goto exit;
        }

        WMR_MEMSET(pBuf->pSpare, 0, stVSVFLWMRDeviceInfo.dwValidMetaPerLogicalPage);

        for(dwPageOffset = 0; dwPageOffset < PAGES_PER_BLOCK; ++dwPageOffset)
        {
            const UInt32 dwPageAddress = _Cbn2Ppn(wCbn, dwPageOffset);
            WMR_MEMSET(pBuf->pData, bTestPattern[dwIteration] + dwPageOffset, BYTES_PER_PAGE);
            WMR_MEMCPY(pBuf->pSpare, &dwPageAddress, sizeof(dwPageAddress));
            andStatus = pFuncTbl->Write(wCS, dwPageAddress, pBuf->pData, pBuf->pSpare, TRUE32);
            if (ANDErrorCodeOk != andStatus)
            {
                WMR_PRINT(VFLWARN, "Iteration %d write CE %d Page %d failed 0x%08x\n",
                          dwIteration, (UInt32) wCS, dwPageAddress, andStatus);
                result = FALSE32;
                goto exit;
            }
        }

        for(dwPageOffset = 0; dwPageOffset < PAGES_PER_BLOCK; ++dwPageOffset)
        {
            const UInt32 dwPageAddress = _Cbn2Ppn(wCbn, dwPageOffset);
            UInt8 bCorrectedBits = 0;
            
            // Set up expected data
            WMR_MEMSET(pBuf->pData, bTestPattern[dwIteration] + dwPageOffset, BYTES_PER_PAGE);
            WMR_MEMCPY(pBuf->pSpare, &dwPageAddress, sizeof(dwPageAddress));
            
            // Watermark
            WMR_MEMSET(pBufVerify->pSpare, 0xB6, stVSVFLWMRDeviceInfo.dwValidMetaPerLogicalPage);
            WMR_MEMSET(pBufVerify->pData, 0xC7, BYTES_PER_PAGE);
            andStatus = pFuncTbl->ReadWithECC(wCS, dwPageAddress, pBufVerify->pData, pBufVerify->pSpare, &bCorrectedBits, NULL, TRUE32);
            if (ANDErrorCodeOk != andStatus)
            {
                WMR_PRINT(ERROR, "Iteration %d read CE %d Page %d failed 0x%08x\n", 
                          dwIteration, (UInt32) wCS, dwPageAddress, andStatus);
                result = FALSE32;
                goto exit;
            }
            
            if (WMR_MEMCMP(pBufVerify->pData, pBuf->pData, BYTES_PER_PAGE))
            {
                WMR_PRINT(ERROR, "Iteration %d main data miscompare CE %d Page %d\n", 
                          dwIteration, (UInt32) wCS, dwPageAddress);
                result = FALSE32;
                goto exit;
            }
            
            if (WMR_MEMCMP(pBufVerify->pSpare, pBuf->pSpare, stVSVFLWMRDeviceInfo.dwValidMetaPerLogicalPage))
            {
                WMR_PRINT(ERROR, "Iteration %d metadata miscompare CE %d Page %d\n", 
                          dwIteration, (UInt32) wCS, dwPageAddress);
                result = FALSE32;
                goto exit;
            }
            
            if (bCorrectedBits >= stVSVFLWMRDeviceInfo.bRefreshThreshold)
            {
                WMR_PRINT(ERROR, "Iteration %d exceeded refresh CE %d Page %d\n", 
                          dwIteration, (UInt32) wCS, dwPageAddress);
                result = FALSE32;
                goto exit;
            }
        }
    }
    
    result = TRUE32;

    andStatus = pFuncTbl->Erase(wCS, _Cbn2Pbn(wCbn));
    if (ANDErrorCodeOk != andStatus)
    {
        WMR_PRINT(ERROR, "Cleanup erase failed 0x%08x\n", andStatus);
        result = FALSE32;
    }

exit:
    
    BUF_Release(pBuf);
    BUF_Release(pBufVerify);

    WMR_PRINT(ALWAYS, "CE %d Block %d %s\n", 
              (UInt32) wCS, (UInt32) _Cbn2Pbn(wCbn), (result ? "PASSED" : "FAILED"));
    
    return result;
}

static BOOL32 _BorrowSpareBlock(UInt16 *pdwCE, UInt16 *pdwBlock)
{
    UInt16 wBlock = ~0;
    UInt16 wCS = ~0;
    UInt32 dwReplacementIdx = ~0;
    BOOL32 found = FALSE32;
    
    if ((NULL == pdwCE) || (NULL == pdwBlock))
    {
        return FALSE32;
    }

    // Find blocks until one passes initial screening
    while (!found)
    {
        _findBestBlockToBorrow(&wCS, &wBlock, &dwReplacementIdx);
        if (_stressBlock(wCS, wBlock))
        {
            found = TRUE32;
        }
        else
        {
            CHECK_VFL_CXT_CHECK_SUM(wCS);
            _MarkReservedBlock(wCS, wBlock, dwReplacementIdx, VFL_BAD_MAP_TABLE_STRESS_FAIL_MARK);
            RECALC_VFL_CXT_CHECK_SUM(wCS);
            if (!_StoreVFLCxt(wCS))
            {
                WMR_PANIC("_StoreVFLCxt(%d) failed\n", (UInt32) wCS);
            }
        }
    }
    
    // Take the block from the reserved area
    CHECK_VFL_CXT_CHECK_SUM(wCS);
    _MarkReservedBlock(wCS, wBlock, dwReplacementIdx, VFL_BAD_MAP_TABLE_BORROW_MARK);
    RECALC_VFL_CXT_CHECK_SUM(wCS);

    if (!_StoreVFLCxt(wCS))
    {
        WMR_PANIC("_StoreVFLCxt(%d) failed\n", (UInt32) wCS);
    }
    
    // Report physical address to the caller (to be passed directly to FIL)
    *pdwCE = wCS;
    *pdwBlock = _Cbn2Pbn(wBlock);
    
    return TRUE32;
}

static BOOL32 _VSVFL_BorrowBlock(UInt32 *pdwCE, UInt32 *pdwPhysicalBlock, UInt16 wType)
{
    UInt16 ce;
    UInt16 block;
    BOOL32 ret;
	
    if (AND_SB_TYPE_NAND_BOOT != wType)
    {
        // only support nand-boot blocks for now
        ret = FALSE32;
    }
    else if ((NULL == pdwCE) || (NULL == pdwPhysicalBlock))
    {
        ret = FALSE32;
    }
    else
    {
        ret = _BorrowSpareBlock(&ce, &block);
        *pdwCE = ce;
        *pdwPhysicalBlock = block;
    }
    
    return ret;
}

#endif // AND_SUPPORT_BLOCK_BORROWING
#endif // ! AND_READONLY

#if AND_SUPPORT_BLOCK_BORROWING
static BOOL32 _VSVFL_FindBorrowedBlocks(BorrowBlockAddress *pastrBlocks, UInt32 *pdwnumBlocks)
{
    UInt16 wCE, wReserveIdx, wBlockIdx;

    if (NULL == pdwnumBlocks)
    {
        return FALSE32;
    }

    wBlockIdx = 0;
    for (wCE = 0; 
         wCE < CS_TOTAL; 
         ++wCE)
    {
        UInt16 *pawMap = pstVFLMeta[wCE].stVFLCxt.awBadMapTable;
        
        for (wReserveIdx = 0; 
             wReserveIdx < VFL_RESERVED_SECTION_SIZE_CS; 
             ++wReserveIdx)
        {
            if ( VFL_BAD_MAP_TABLE_BORROW_MARK == pawMap[wReserveIdx])
            {
                if ((NULL != pastrBlocks))
                {
                    UInt16 wCbn;
                    pastrBlocks[wBlockIdx].dwPhysicalCE = wCE;
                    _ConvertT2P(_CSBank2Bank(wCE, _ReserveIdx2Bank(wReserveIdx)), _ReserveIdx2Tbn(wReserveIdx), &wCbn);
                    pastrBlocks[wBlockIdx].dwPhysicalBlock = _Cbn2Pbn(wCbn);
                    pastrBlocks[wBlockIdx].wType = AND_SB_TYPE_NAND_BOOT;
                }
                ++wBlockIdx;
                if (wBlockIdx >= *pdwnumBlocks)
                {
                    *pdwnumBlocks = wBlockIdx;
                    return TRUE32;
                }
            }
        }
        
    }
    *pdwnumBlocks = wBlockIdx;
    
    return TRUE32;
}
#endif // AND_SUPPORT_BLOCK_BORROWING

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

#ifndef AND_READONLY
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

// helper function list sorting
static void _SortList(UInt16 *pawIndex, UInt16 * pawValues, UInt16 wSize)
{
    UInt16 wIdx1, wIdx2;

    if (wSize == 1)
    {
        return;
    }
    for (wIdx1 = 0; wIdx1 < (wSize - 1); wIdx1++)
    {
        for (wIdx2 = wIdx1 + 1; wIdx2 < wSize; wIdx2++)
        {
            if (pawValues[pawIndex[wIdx2]] < pawValues[pawIndex[wIdx1]])
            {
                UInt16 wTemp = pawIndex[wIdx2];
                pawIndex[wIdx2] = pawIndex[wIdx1];
                pawIndex[wIdx1] = wTemp;
            }
        }
    }
}

/**
 * Replaces bad block, updating CXT but does NOT update the 
 * checksum -- the caller must do that. 
 * 
 * @return BOOL32 
 */
static BOOL32 _ReplaceBadBlock(UInt16 wCS, UInt16 wPbn)
{
    VFLCxt *pVFLCxt = &pstVFLMeta[wCS].stVFLCxt;
    UInt16 wTbn, wBank, wIdx;
    UInt16 awBanksSorted[AND_MAX_BANKS_PER_CS];
    UInt8 *pabBBT = pstabCSBBTArea[wCS];

    _MarkBlockAsBadInBBT(pabBBT, wPbn);
    _ConvertP2T(wPbn, &wBank, &wTbn);

    // first thing make sure the block was not replaced by another block
    // if it did mark that block as bad and cancel the previous replacement
    for (wIdx = 0; wIdx < VFL_RESERVED_SECTION_SIZE_CS; wIdx++)
    {
        if (pVFLCxt->awBadMapTable[wIdx] == wPbn)
        {
            UInt16 wTempPbn;
            pVFLCxt->awBadMapTable[wIdx] = VFL_BAD_MAP_TABLE_BAD_MARK;
            // mark the replacement block as bad
            _ConvertT2P(_CSBank2Bank(wCS, _ReserveIdx2Bank(wIdx)), _ReserveIdx2Tbn(wIdx), &wTempPbn);
            _MarkBlockAsBadInBBT(pabBBT, wTempPbn);
        }
    }

    // check if we can replace the block from its own bank
    if (pVFLCxt->awReplacementIdx[wBank] < VFL_RESERVED_SECTION_SIZE_BANK)
    {
        while (pVFLCxt->awReplacementIdx[wBank] < VFL_RESERVED_SECTION_SIZE_BANK)
        {
            UInt16 wNextReplacementBlkIdx = pVFLCxt->awReplacementIdx[wBank] + (wBank * VFL_RESERVED_SECTION_SIZE_BANK);
            if (pVFLCxt->awBadMapTable[wNextReplacementBlkIdx] == VFL_BAD_MAP_TABLE_AVAILABLE_MARK)
            {
                pVFLCxt->awBadMapTable[wNextReplacementBlkIdx] = wPbn;
                pVFLCxt->awReplacementIdx[wBank]++;
                return TRUE32;
            }
            pVFLCxt->awReplacementIdx[wBank]++;
        }
    }

    // check which other block as the most available blocks
    for (wIdx = 0; wIdx < BANKS_PER_CS; wIdx++)
    {
        awBanksSorted[wIdx] = wIdx;
    }

    _SortList(awBanksSorted, pVFLCxt->awReplacementIdx, BANKS_PER_CS);
    for (wIdx = 0; wIdx < BANKS_PER_CS; wIdx++)
    {
        if (pVFLCxt->awReplacementIdx[awBanksSorted[wIdx]] < VFL_RESERVED_SECTION_SIZE_BANK)
        {
            while (pVFLCxt->awReplacementIdx[awBanksSorted[wIdx]] < VFL_RESERVED_SECTION_SIZE_BANK)
            {
                UInt16 wNextReplacementBlkIdx = pVFLCxt->awReplacementIdx[awBanksSorted[wIdx]] + (awBanksSorted[wIdx] * VFL_RESERVED_SECTION_SIZE_BANK);
                if (pVFLCxt->awBadMapTable[wNextReplacementBlkIdx] == VFL_BAD_MAP_TABLE_AVAILABLE_MARK)
                {
                    pVFLCxt->awBadMapTable[wNextReplacementBlkIdx] = wPbn;
                    pVFLCxt->awReplacementIdx[awBanksSorted[wIdx]]++;
                    return TRUE32;
                }
                pVFLCxt->awReplacementIdx[awBanksSorted[wIdx]]++;
            }
        }
    }
    // no reason to continue once NAND runs out of replacement blocks
#if (defined(AND_SIMULATOR) && AND_SIMULATOR)
    WMR_SIM_EXIT("Out of replacement blocks");
#else // defined(AND_SIMULATOR) && AND_SIMULATOR
    WMR_PANIC("Failed _ReplaceBadBlock CE %d PBN %d", (UInt32) wCS, (UInt32) wPbn);
#endif // defined(AND_SIMULATOR) && AND_SIMULATOR
    return FALSE32;
}
#endif // ! AND_READONLY

static UInt16 _RemapBlock(UInt16 wCS, UInt16 wPbn, BOOL32 *pboolAligned, BOOL32 *pboolCorrectBank)
{
    UInt16 wIdx;
    UInt8 * pabBBT = pstabCSBBTArea[wCS];
    VFLCxt * pVFLCxt = GET_VFLCxt(wCS);

    // check if the original block is bad
    if (_isBlockGood(pabBBT, wPbn) == TRUE32)
    {
        return wPbn;
    }

    if (pboolAligned != NULL)
    {
        *pboolAligned = FALSE32;
    }
    for (wIdx = 0; wIdx < VFL_RESERVED_SECTION_SIZE_CS; wIdx++)
    {
        if (pVFLCxt->awBadMapTable[wIdx] == wPbn)
        {
            UInt16 wRemappedPbn, wTbn, wBank;
            wBank = _CSBank2Bank(wCS, _ReserveIdx2Bank(wIdx));
            wTbn = _ReserveIdx2Tbn(wIdx);
            _ConvertT2P(wBank, wTbn, &wRemappedPbn);
            if (pboolCorrectBank != NULL)
            {
                UInt16 wOrgBank, wOrgTbn;
                _ConvertP2T(wPbn, &wOrgBank, &wOrgTbn);
                if (_Bank2CSBank(wBank) != wOrgBank)
                {
                    *pboolCorrectBank = FALSE32;
                }
            }
            return wRemappedPbn;
        }
    }
    WMR_PANIC("Failed to map CE %d Block 0x%04x\n", (UInt32) wCS, (UInt32) wPbn);
    return VFL_BAD_MAP_TABLE_BAD_MARK;
}

#ifndef AND_READONLY
/* writes the VFLCxt 8 times to and verifies it was written properly at least 5 times */
static BOOL32 _WriteVFLCxtToFlash(UInt16 wCS)
{
    UInt16 wPageIdx, wNumOfErrors;
    UInt32 dwCxtAge;
    Buffer  *pBuf = NULL;
    VFLCxt  *pVFLCxt;
    BOOL32 boolWriteVFLCxtSuccess = FALSE32;
    UInt16 wCurrCxtPOffset;
    Int32  nFILRet;

    VFL_LOG_PRINT((TEXT("[VFL:INF] ++_WriteVFLCxtToFlash(wCS:%d)\n"), wCS));

    /* check current state			*/
    if (wCS >= CS_TOTAL)
    {
        return FALSE32;
    }

    pVFLCxt = GET_VFLCxt(wCS);
    pBuf = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBuf == NULL)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] error BUF_Get(line:%d)\n"), __LINE__));
        return FALSE32;
    }
    CHECK_VFL_CXT_CHECK_SUM(wCS);
    dwCxtAge = --(pVFLCxt->dwCxtAge);
    pVFLCxt->dwGlobalCxtAge = ++gdwMaxGlobalAge;
    wCurrCxtPOffset = pVFLCxt->wNextCxtPOffset;
    pVFLCxt->wNextCxtPOffset += VFL_NUM_OF_VFL_CXT_COPIES;
    RECALC_VFL_CXT_CHECK_SUM(wCS);
    WMR_MEMCPY(pBuf->pData, &pstVFLMeta[wCS], sizeof(VFLMeta));

    /* writing the information */
    for (wPageIdx = 0; wPageIdx < VFL_NUM_OF_VFL_CXT_COPIES; wPageIdx++)
    {
        WMR_MEMSET(pBuf->pSpare, 0xFF, BYTES_PER_METADATA_RAW);
        _SetVFLSpare(&dwCxtAge, pBuf->pSpare);

        /* we are not checking the result of the operation here - we will check the content below using read */
        nFILRet = pFuncTbl->Write(wCS, _Cbn2Ppn(pVFLCxt->awInfoBlk[pVFLCxt->wCxtLocation],
                                         (wCurrCxtPOffset + wPageIdx)), pBuf->pData, pBuf->pSpare, FALSE32);
        if ( nFILRet != FIL_SUCCESS)
        {
            VFL_WRN_PRINT((TEXT("[VFL:WRN] _WriteVFLCxtToFlash failed write (0x%X, 0x%X) (line:%d)\n"),
                           wCS, _Cbn2Ppn(pVFLCxt->awInfoBlk[pVFLCxt->wCxtLocation], (wCurrCxtPOffset + wPageIdx)), __LINE__));
            BUF_Release(pBuf);
            _ReportWriteFailureSingle(wCS, _Cbn2Ppn(pVFLCxt->awInfoBlk[pVFLCxt->wCxtLocation],
                                         (wCurrCxtPOffset + wPageIdx)), nFILRet);
            return FALSE32;
        }
    }

    wNumOfErrors = 0;

    for (wPageIdx = 0; wPageIdx < VFL_NUM_OF_VFL_CXT_COPIES; wPageIdx++)
    {
        Int32 nFILRet;
        nFILRet = pFuncTbl->ReadWithECC(wCS, _Cbn2Ppn(pVFLCxt->awInfoBlk[pVFLCxt->wCxtLocation],
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
            _ReportECCFailureSingle(wCS, _Cbn2Ppn(pVFLCxt->awInfoBlk[pVFLCxt->wCxtLocation],
                                                     (wCurrCxtPOffset + wPageIdx)));
            wNumOfErrors++;
        }
    }

    if (wNumOfErrors <= VFL_MAX_NUM_OF_VFL_CXT_ERRORS)
    {
        boolWriteVFLCxtSuccess = TRUE32;
    }
    else
    {
        VFL_ERR_PRINT((TEXT("[VFL:WRN] Fail writing VFLCxt(line:%d) CS %d block %d page %d\n"), __LINE__, wCS, pVFLCxt->wCxtLocation, wCurrCxtPOffset));
    }

    BUF_Release(pBuf);
    VFL_LOG_PRINT((TEXT("[VFL: IN] --_WriteVFLCxtToFlash(wCS:%d)\n"), wCS));
    return boolWriteVFLCxtSuccess;
}

/*
    NAME
        _StoreVFLCxt
    DESCRIPTION
        This function stores VFL context on VFL area.
    PARAMETERS
        wCS   CS number
    RETURN VALUES
        TRUE32
            _StoreVFLCxt is completed
        FALSE32
            _StoreVFLCxt is failed
    NOTES
 */
static BOOL32
_StoreVFLCxt(UInt16 wCS)
{
    VFLCxt  *pVFLCxt;
    BOOL32 boolWriteVFLCxtSuccess = FALSE32;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++_StoreVFLCxt(wCS:%d)\n"), wCS));

    /* check current state			*/
    WMR_ASSERT(wCS < CS_TOTAL);

    pVFLCxt = GET_VFLCxt(wCS);

    /* check if we still have space in the current block */
    if ((pVFLCxt->wNextCxtPOffset + VFL_NUM_OF_VFL_CXT_COPIES) <= PAGES_PER_BLOCK)
    {
        // we have space in the current block
        boolWriteVFLCxtSuccess = _WriteVFLCxtToFlash(wCS);
    }

    if (boolWriteVFLCxtSuccess == FALSE32)
    {
        // we do NOT have space in the current block or write operation in the current block failed
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

#if (defined(WMR_ENABLE_ERASE_PROG_BAD_BLOCK))
                WMR_ENABLE_ERASE_PROG_BAD_BLOCK(wCS, _Cbn2Pbn(pVFLCxt->awInfoBlk[wVFLCxtBlkIdx]));
#endif // WMR_ENABLE_ERASE_PROG_BAD_BLOCK

                nFILRet = pFuncTbl->Erase(wCS, _Cbn2Pbn(pVFLCxt->awInfoBlk[wVFLCxtBlkIdx]));

#if (defined(WMR_DISABLE_ERASE_PROG_BAD_BLOCK))
                WMR_DISABLE_ERASE_PROG_BAD_BLOCK(wCS, _Cbn2Pbn(pVFLCxt->awInfoBlk[wVFLCxtBlkIdx]));
#endif // WMR_DISABLE_ERASE_PROG_BAD_BLOCK

                if (nFILRet == FIL_SUCCESS)
                {
                    break;
                }
                else
                {
                    _ReportEraseFailureSingle(wCS, pVFLCxt->awInfoBlk[wVFLCxtBlkIdx], nFILRet);
                }
            }

            if (wEraseCnt == WMR_NUM_OF_ERASE_TRIALS_FOR_INDEX_BLOCKS)
            {
                // pVFLCxt->awInfoBlk[wVFLCxtBlkIdx] = VFL_INVALID_INFO_INDEX;
                VFL_ERR_PRINT((TEXT("[VFL:ERR] Fail erasing VFLBlock CS %d block 0x%X (line:%d)\n"), wCS, pVFLCxt->awInfoBlk[wVFLCxtBlkIdx], __LINE__));
                wVFLCxtBlkIdx = (wVFLCxtBlkIdx + 1) % VFL_INFO_SECTION_SIZE;
                continue;
            }

            CHECK_VFL_CXT_CHECK_SUM(wCS);
            pVFLCxt->wCxtLocation = wVFLCxtBlkIdx;
            pVFLCxt->wNextCxtPOffset = 0;
            RECALC_VFL_CXT_CHECK_SUM(wCS);

            boolWriteVFLCxtSuccess = _WriteVFLCxtToFlash(wCS);
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

    VFL_LOG_PRINT((TEXT("[VFL:OUT] --_StoreVFLCxt(wCS:%d)\n"), wCS));
    if (boolWriteVFLCxtSuccess == FALSE32)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _StoreVFLCxt(line:%d) fail CS %d!\n"), __LINE__, wCS));
    }

    return boolWriteVFLCxtSuccess;
}
#endif // ! AND_READONLY

static BOOL32 _ReadVFLCxtFromFlash(UInt16 wCS, UInt16 wBlk, UInt16 wStartPOffset, Buffer * pBuf)
{
    Int32 nFILRet;
    UInt16 wPageIdx;

    for (wPageIdx = 0; wPageIdx < VFL_NUM_OF_VFL_CXT_COPIES; wPageIdx++)
    {
        nFILRet = pFuncTbl->ReadWithECC(wCS, _Cbn2Ppn(wBlk, (wStartPOffset + wPageIdx)), pBuf->pData, pBuf->pSpare, NULL, NULL, FALSE32);
        if (nFILRet == FIL_SUCCESS)
        {
            // NirW - we can check here the number of valid VFLCxt copies and refresh if needed
            UInt8 bStatus, bSpareType;
            /* verify the status byte */
            _GetVFLSpare(NULL, &bStatus, &bSpareType, pBuf->pSpare);
            if (bStatus == PAGE_VALID && bSpareType == VFL_SPARE_TYPE_CXT)
            {
                VFLMeta *tmpPtr= (VFLMeta *)(pBuf->pData);
                if(tmpPtr->dwVersion > VFL_META_VERSION)
                {
                    VFL_ERR_PRINT((TEXT("[VFL:ERR] _ReadVFLCxtFromFlash VFLCXT version mismatch CS 0x%x :  0x%x 0x%x \n"), wCS,tmpPtr->dwVersion,VFL_META_VERSION));
                    return FALSE32;
                }
                return TRUE32;
            }
        }
        else
        {
            _ReportECCFailureSingle(wCS, _Cbn2Ppn(wBlk, (wStartPOffset + wPageIdx)));
        }
    }
    return FALSE32;
}

/*
    NAME
        _LoadVFLCxt
    DESCRIPTION
        This function loads VFL context from VFL area.
    PARAMETERS
        wCS   CS number
    RETURN VALUES
        TRUE32
    _LoadVFLCxt is completed
        FALSE32
    _LoadVFLCxt is failed
        NOTES
 */
static BOOL32
_LoadVFLCxt(UInt16 wCS, UInt8 * pabBBT)
{
    VFLCxt  *pVFLCxt;
    UInt16 wBlkIdx, wPOffsetIdx;
    Buffer  *pBuf = NULL;
    UInt32 dwMinAge;
    UInt16 wNewCxt, wNewPOffset;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++_LoadVFLCxt(wCS:%d)\n"), wCS));

    /* check current state			*/
    if (wCS >= CS_TOTAL)
    {
        return FALSE32;
    }

    pVFLCxt = GET_VFLCxt(wCS);
    pBuf = BUF_Get(BUF_MAIN_AND_SPARE);

    if (pBuf == NULL)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _LoadVFLCxt(wCS:%d) failed BUF_Get(line:%d)\n"), wCS, __LINE__));
        return FALSE32;
    }

    /* first thing - find the first block with VFLCxt information and extract the awInfoBlk from it */
    for (wBlkIdx = VFL_FIRST_BLK_TO_SEARCH_CXT; wBlkIdx < VFL_LAST_BLK_TO_SEARCH_CXT; wBlkIdx++)
    {
        if (_ReadVFLCxtFromFlash(wCS, wBlkIdx, 0, pBuf) == TRUE32)
        {
            VFLCxt  *pVFLCxtTemp;
            pVFLCxtTemp = (VFLCxt *)pBuf->pData;
            WMR_MEMCPY(pVFLCxt->awInfoBlk, pVFLCxtTemp->awInfoBlk, (sizeof(UInt16) * VFL_INFO_SECTION_SIZE));
            break;
        }
    }
    if (wBlkIdx == VFL_LAST_BLK_TO_SEARCH_CXT)
    {
        BUF_Release(pBuf);
        /* we should never get here - getting here means we fail to find a valid VFLCxt in the VFL_AREA */
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _LoadVFLCxt(line:%d) fail CS %d!\n"), __LINE__, wCS));
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
            if (_ReadVFLCxtFromFlash(wCS, pVFLCxt->awInfoBlk[wBlkIdx], 0, pBuf) == FALSE32)
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
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _LoadVFLCxt(line:%d) CW %d!\n"), __LINE__, wCS));
        BUF_Release(pBuf);
        return FALSE32;
    }

    /* look for the youngest valid VFLCxt version in the youngest block (i.e. search the youngest block pages) */
    wNewPOffset = 0;
    for (wPOffsetIdx = VFL_NUM_OF_VFL_CXT_COPIES; wPOffsetIdx < PAGES_PER_BLOCK; wPOffsetIdx += VFL_NUM_OF_VFL_CXT_COPIES)
    {
        if (_ReadVFLCxtFromFlash(wCS, pVFLCxt->awInfoBlk[wNewCxt], wPOffsetIdx, pBuf) == FALSE32)
        {
            break;
        }

        wNewPOffset = wPOffsetIdx;
    }
    if (_ReadVFLCxtFromFlash(wCS, pVFLCxt->awInfoBlk[wNewCxt], wNewPOffset, pBuf) == FALSE32)
    {
        /* this shouldn't happen since are using valid pointers */
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _LoadVFLCxt(line:%d) CS %d!\n"), __LINE__, wCS));
        BUF_Release(pBuf);
        return FALSE32;
    }

    WMR_MEMCPY(&pstVFLMeta[wCS], pBuf->pData, sizeof(VFLMeta));

    if (pVFLCxt->dwGlobalCxtAge > gdwMaxGlobalAge)
    {
        gdwMaxGlobalAge = pVFLCxt->dwGlobalCxtAge;
    }

    BUF_Release(pBuf);
    if (CHECK_VFL_CXT_CHECK_SUM_NO_ASSERT(wCS) == FALSE32)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] CHECK_VFL_CXT_CHECK_SUM_NO_ASSERT(%d) failed (line:%d)!\n"), wCS, __LINE__));
        return FALSE32;
    }
    VFL_LOG_PRINT((TEXT("[VFL:OUT] --_LoadVFLCxt(wCS:%d)\n"), wCS));

    return TRUE32;
}

static BOOL32 _SetUpVendorSpecificRelatedParams(VendorSpecificType vendorSpecificType)
{
    // set the relevant FIL parameters
    stVSVFLWMRDeviceInfo.wNumOfBanksPerCS = _GetBanksPerCS(vendorSpecificType);

    stVSVFLWMRDeviceInfo.wPagesPerSuBlk = stVSVFLWMRDeviceInfo.wPagesPerBlock * stVSVFLWMRDeviceInfo.wNumOfBanksPerCS * stVSVFLWMRDeviceInfo.wNumOfCEs;
    stVSVFLWMRDeviceInfo.wNumOfBank = stVSVFLWMRDeviceInfo.wNumOfBanksPerCS * stVSVFLWMRDeviceInfo.wNumOfCEs;
    stVSVFLWMRDeviceInfo.wBlocksPerBank = (UInt16)(stVSVFLWMRDeviceInfo.wBlocksPerCS / stVSVFLWMRDeviceInfo.wNumOfBanksPerCS);

    _SetV2PFunctions(vendorSpecificType);
    _SetBankToCSConvertFunctions(vendorSpecificType);

    pFuncTbl->SetDeviceInfo(AND_DEVINFO_BANKS_PER_CS, stVSVFLWMRDeviceInfo.wNumOfBanksPerCS);
    pFuncTbl->SetDeviceInfo(AND_DEVINFO_VENDOR_SPECIFIC_TYPE, vendorSpecificType);
    return TRUE32;
}

static BOOL32 _InitDeviceInfo(void)
{
    pFuncTbl = stFPartFunctions.GetLowFuncTbl();
    stVSVFLWMRDeviceInfo.wBlocksPerCS = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CS);
    stVSVFLWMRDeviceInfo.wBytesPerPage = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    stVSVFLWMRDeviceInfo.wNumOfCEs = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_NUM_OF_CS);
    stVSVFLWMRDeviceInfo.wPagesPerBlock = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_PAGES_PER_BLOCK);
    stVSVFLWMRDeviceInfo.bRefreshThreshold = (UInt8)pFuncTbl->GetDeviceInfo(AND_DEVINFO_REFRESH_THRESHOLD);

    stVSVFLWMRDeviceInfo.wNumOfBanksPerCS = 1;
    stVSVFLWMRDeviceInfo.wBlocksPerDie = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_DIE);
    stVSVFLWMRDeviceInfo.wDieStride = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_DIE_STRIDE);
    stVSVFLWMRDeviceInfo.wBlockStride = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_BLOCK_STRIDE);
    stVSVFLWMRDeviceInfo.wUserBlocksPerCS = (UInt16)pFuncTbl->GetDeviceInfo(AND_DEVINFO_USER_BLOCKS_PER_CS);

    stVSVFLWMRDeviceInfo.dwValidMetaPerLogicalPage = pFuncTbl->GetDeviceInfo(AND_DEVINFO_FIL_META_VALID_BYTES);
    stVSVFLWMRDeviceInfo.dwTotalMetaPerLogicalPage = pFuncTbl->GetDeviceInfo(AND_DEVINFO_FIL_META_BUFFER_BYTES);
    stVSVFLWMRDeviceInfo.dwLogicalPageSize = pFuncTbl->GetDeviceInfo(AND_DEVINFO_FIL_LBAS_PER_PAGE);

    VFL_LOG_PRINT((TEXT("[VFL:LOG] _ConvertP2T = 0X%X\n"), _ConvertP2T));
    VFL_LOG_PRINT((TEXT("[VFL:LOG] _ConvertT2P = 0X%X\n"), _ConvertT2P));
    VFL_LOG_PRINT((TEXT("[VFL:LOG] wPagesPerBlock = 0X%X\n"), stVSVFLWMRDeviceInfo.wPagesPerBlock));
    VFL_LOG_PRINT((TEXT("[VFL:LOG] bRefreshThreshold = 0X%X\n"), stVSVFLWMRDeviceInfo.bRefreshThreshold));
    VFL_LOG_PRINT((TEXT("[VFL:LOG] wNumOfCEs = 0X%X\n"), stVSVFLWMRDeviceInfo.wNumOfCEs));
    VFL_LOG_PRINT((TEXT("[VFL:LOG] wBlocksPerCS = 0X%X\n"), stVSVFLWMRDeviceInfo.wBlocksPerCS));
    VFL_LOG_PRINT((TEXT("[VFL:LOG] wBytesPerPage = 0X%X\n"), stVSVFLWMRDeviceInfo.wBytesPerPage));
    VFL_LOG_PRINT((TEXT("[VFL:LOG] dwValidMetaPerLogicalPage = 0X%X\n"), stVSVFLWMRDeviceInfo.dwValidMetaPerLogicalPage));
    VFL_LOG_PRINT((TEXT("[VFL:LOG] dwTotalMetaPerLogicalPage = 0X%X\n"), stVSVFLWMRDeviceInfo.dwTotalMetaPerLogicalPage));
    VFL_LOG_PRINT((TEXT("[VFL:LOG] dwLogicalPageSize = 0X%X\n"), stVSVFLWMRDeviceInfo.dwLogicalPageSize));

    return TRUE32;
}


static BOOL32 _initWriteMultipleTables(void)
{
    WMR_ASSERT(BANKS_TOTAL > 0);

#ifndef AND_READONLY    
    // init pawPbnOriginal
    if (pawPbnOriginal == NULL)
    {
        pawPbnOriginal = (UInt16 *)WMR_MALLOC(sizeof(*pawPbnOriginal) * BANKS_TOTAL);
        if ( pawPbnOriginal == NULL )
        {
            return FALSE32;
        }
    }

    // init pawPbn
    if (pawPbn == NULL)
    {
        pawPbn = (UInt16 *)WMR_MALLOC(sizeof(*pawPbn) * BANKS_TOTAL);
        if ( pawPbn == NULL )
        {
            return FALSE32;
        }
    }

    // init pawPbnUnmapped
    if (pawPbnUnmapped == NULL)
    {
        pawPbnUnmapped = (UInt16 *)WMR_MALLOC(sizeof(*pawPbnUnmapped) * BANKS_TOTAL);
        if ( pawPbnUnmapped == NULL )
        {
            return FALSE32;
        }
    }
#endif

    // init pawPbnRemapped
    if (pawPbnRemapped == NULL)
    {
        pawPbnRemapped = (UInt16 *)WMR_MALLOC(sizeof(*pawPbnRemapped) * BANKS_TOTAL);
        if ( pawPbnRemapped == NULL )
        {
            return FALSE32;
        }
    }

    // init padwPpnRemapped
    if (padwPpnRemapped == NULL)
    {
        padwPpnRemapped = (UInt32 *)WMR_MALLOC(sizeof(*padwPpnRemapped) * BANKS_TOTAL);
        if ( padwPpnRemapped == NULL )
        {
            return FALSE32;
        }
    }

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
    UInt16 wCSIdx;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++VFL_Init()\n")));

    WMR_ASSERT(FTL_FORMAT_STRUCT_SIZE == sizeof(FTLFormatType));
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
        pstVFLMeta = (VFLMeta *)WMR_MALLOC(sizeof(VFLMeta) * CS_TOTAL);
        if ( pstVFLMeta == NULL )
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  there is no memory to initialize pstVFLCxt!\n")));
            return VFL_CRITICAL_ERROR;
        }
    }

    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        pstabCSBBTArea[wCSIdx] = (UInt8*)WMR_MALLOC(BBT_SIZE_PER_CS);
    }

    /* get low level function table	*/
    pFuncTbl = stFPartFunctions.GetLowFuncTbl();

    if (pFuncTbl == NULL)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  FIL_GetFuncTbl() fail!\n")));
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
Int32
VFL_Open(UInt32 dwBootAreaSize, UInt32 minor_ver, UInt32 dwOptions)
{
    UInt16 wCSIdx, wIdx;
    VFLCxt *pVFLCxt;

#ifndef AND_READONLY
    FTLFormatType * pFTLFormatType;
#endif
    VendorSpecificType vendorSpecificType;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++VFL_Open()\n")));

    stVSVFLWMRDeviceInfo.dwVFLAreaStart = dwBootAreaSize;
    
    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        BOOL32 bRet;
        UInt8 * pabBBT = pstabCSBBTArea[wCSIdx];

        bRet = VFL_ReadBBT(wCSIdx, pabBBT);
        if (bRet == FALSE32)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_ReadBBT(wCS:%d)(line %d) fail!\n"), wCSIdx, __LINE__));
            return VFL_CRITICAL_ERROR;
        }
        bRet = _LoadVFLCxt(wCSIdx, pabBBT);

        if (bRet == FALSE32)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  _LoadVFLCxt(wCSIdx:%d)(line %d) fail!\n"), wCSIdx, __LINE__));
            return VFL_CRITICAL_ERROR;
        }
    }

    pVFLCxt = _GetNewestVFLCxt();
    if (pVFLCxt->wNumOfFTLSuBlk > pVFLCxt->wNumOfVFLSuBlk)
    {
        VFL_ERR_PRINT((TEXT("FTLSuBlks(=%d) > VFLSuBlks(=%d)\n"), pVFLCxt->wNumOfFTLSuBlk, pVFLCxt->wNumOfVFLSuBlk));
        return VFL_CRITICAL_ERROR;
    }

    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        /* make sure all are up to date so later storing of VFL Cxt would be correct */
        WMR_MEMCPY(pstVFLMeta[wCSIdx].stVFLCxt.awFTLCxtVbn, pVFLCxt->awFTLCxtVbn, FTL_CXT_SECTION_SIZE * sizeof(UInt16));
        pstVFLMeta[wCSIdx].stVFLCxt.wNumOfVFLSuBlk = pVFLCxt->wNumOfVFLSuBlk;
        pstVFLMeta[wCSIdx].stVFLCxt.wNumOfFTLSuBlk = pVFLCxt->wNumOfFTLSuBlk;
        pstVFLMeta[wCSIdx].stVFLCxt.dwFTLType = pVFLCxt->dwFTLType;
        WMR_MEMCPY(pstVFLMeta[wCSIdx].stVFLCxt.abFTLFormat, pVFLCxt->abFTLFormat, FTL_FORMAT_STRUCT_SIZE);
        RECALC_VFL_CXT_CHECK_SUM(wCSIdx);
    }

    // since we use s vs value from the VFLCxt we need to move the allocation from Init here
    WMR_MEMCPY(&vendorSpecificType, pstVFLMeta[0].stVFLCxt.abVSFormtType, 4);
#if (defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)
    if (vendorSpecificType != FIL_VS_SIMPLE)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Open context vs-type is not simple (line %d)\n"), __LINE__));
        return VFL_CRITICAL_ERROR;
    }
#endif    
    if (vendorSpecificType == FIL_VS_UNKNOWN)
    {
        vendorSpecificType = (VendorSpecificType)pFuncTbl->GetDeviceInfo(AND_DEVINFO_VENDOR_SPECIFIC_TYPE);
    }

    _SetUpVendorSpecificRelatedParams(vendorSpecificType);

    /* Allocate buffers for scattered read/write operation */
    if (pstadwScatteredReadWritePpn == NULL && pstawScatteredReadWriteCS == NULL)
    {
#if (defined(AND_READONLY) && AND_READONLY) && defined(WMR_MAX_READONLY_PAGES)
        pstadwScatteredReadWritePpn = (UInt32 *)WMR_MALLOC(WMR_MAX_READONLY_PAGES * sizeof(UInt32));
        pstawScatteredReadWriteCS = (UInt16 *)WMR_MALLOC(WMR_MAX_READONLY_PAGES * sizeof(UInt16));
#else
        pstadwScatteredReadWritePpn = (UInt32 *)WMR_MALLOC(PAGES_PER_SUBLK * sizeof(UInt32));
        pstawScatteredReadWriteCS = (UInt16 *)WMR_MALLOC(PAGES_PER_SUBLK * sizeof(UInt16));
#endif

        if (pstadwScatteredReadWritePpn == NULL || pstawScatteredReadWriteCS == NULL)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  there is no memory to initialize pstadwScatteredReadPpn or pstawScatteredReadCS!\n")));
            return VFL_CRITICAL_ERROR;
        }
    }

    if (!_initWriteMultipleTables())
    {
        return VFL_CRITICAL_ERROR;
    }

    // Per CS, build the bitmap BBT
    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        UInt8 *pabBBT = pstabCSBBTArea[wCSIdx];
        VFLCxt *pVFLCxt = &pstVFLMeta[wCSIdx].stVFLCxt;
        UInt16 wBankIdx;
        WMR_MEMSET(pabBBT, 0xFF, BBT_SIZE_PER_CS);
        for (wBankIdx = 0; wBankIdx < BANKS_PER_CS; wBankIdx++)
        {
            for (wIdx = 0; wIdx < VFL_RESERVED_SECTION_SIZE_BANK; wIdx++)
            {
                UInt16 wCurrIdxToCheck = wIdx + (VFL_RESERVED_SECTION_SIZE_BANK * wBankIdx);
                if (pVFLCxt->awBadMapTable[wCurrIdxToCheck] > VFL_BAD_MAP_TABLE_AVAILABLE_MARK)
                {
                    UInt16 wPbn, wTbn = wIdx + _GetNumOfSuBlks();
                    _ConvertT2P(_CSBank2Bank(wCSIdx, wBankIdx), wTbn, &wPbn);
                    _MarkBlockAsBadInBBT(pabBBT, wPbn);
                }
                else if (pVFLCxt->awBadMapTable[wCurrIdxToCheck] < BLOCKS_PER_CS)
                {
                    _MarkBlockAsBadInBBT(pabBBT, pVFLCxt->awBadMapTable[wCurrIdxToCheck]);
                }
                else if (pVFLCxt->awBadMapTable[wCurrIdxToCheck] != VFL_BAD_MAP_TABLE_AVAILABLE_MARK)
                {
                    WMR_PANIC("[VFL:ERR] CS %d Bad Map Table entry %d contains invalid value 0x%08x\n",
                                (UInt32) wCSIdx, (UInt32) wCurrIdxToCheck, (UInt32) pVFLCxt->awBadMapTable[wCurrIdxToCheck]);
                }
            }
        }
    }
#ifndef AND_READONLY
    pFTLFormatType = (FTLFormatType *)pstVFLMeta[0].stVFLCxt.abFTLFormat;
    if (pFTLFormatType->bFormatFTL)
    {
        _SetNewFTLParams();
    }
    vflBlockRefreshTrigger=(UInt8 *)WMR_MALLOC(pVFLCxt->wNumOfVFLSuBlk);
    if(vflBlockRefreshTrigger == NULL)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] cannot alocate refreshtrigger array. continue to operate in legacy mode \n")));
    }
    else
        WMR_MEMSET(vflBlockRefreshTrigger,0,pVFLCxt->wNumOfVFLSuBlk);	
#endif

    VFL_LOG_PRINT((TEXT("[VFL:OUT] --VFL_Open()\n")));

    return VFL_SUCCESS;
}

BOOL32 _BlankCheckSpare(UInt32 * padwBuff, UInt16 wSize)
{
    UInt16 i, j, zeroes;
    UInt32 dwVal;

    zeroes = 0;
    for (i = 0; i < wSize; i += sizeof(UInt32), padwBuff++)
    {
        dwVal = *padwBuff;
        if (dwVal != 0xFFFFFFFF)
        {
            for (j = 0; j < 32; j++)
            {
                if (!(dwVal & 1) && (++zeroes > 2))
                {
                    return FALSE32;
                }
                dwVal >>= 1;
            }
        }
    }
    return TRUE32;
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
VFL_Read(UInt32 dwVpn, Buffer *pBuf, BOOL32 boolCleanCheck, BOOL32 bMarkECCForScrub, BOOL32 * pboolNeedRefresh, UInt8 * pdwaSectorStats, BOOL32 bDisableWhitening)
{
    UInt16 wCS;
    UInt32 dwPpn;
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

    /* calculate physical address		*/
    _Vpn2Ppn(dwVpn, &wCS, &dwPpn);

    nFILRet = pFuncTbl->ReadWithECC(wCS, dwPpn, pBuf->pData,
                                    pBuf->pSpare, &bECCCorrectedBits, pdwaSectorStats, bDisableWhitening);

    // in case we are no ready to process clean pages mark the page as ecc error if it is clean
    if (boolCleanCheck == FALSE32 && nFILRet == FIL_SUCCESS_CLEAN)
    {
        nFILRet = FIL_U_ECC_ERROR;
        _ReportECCFailureSingle(wCS, dwPpn);
    }

    // check if the page need to be refreshed
    if (pboolNeedRefresh != NULL)
    {
        if ((bECCCorrectedBits >= FIL_ECC_REFRESH_TRESHOLD && nFILRet == FIL_SUCCESS) ||
            (nFILRet == FIL_U_ECC_ERROR))
        {
#ifndef AND_READONLY
            if(bMarkECCForScrub == TRUE32 && bECCCorrectedBits >= FIL_ECC_REFRESH_TRESHOLD && nFILRet == FIL_SUCCESS && vflBlockRefreshTrigger != NULL)
                vflBlockRefreshTrigger[dwVpn/PAGES_PER_SUBLK]++;
#endif			
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
        nFILRet = pFuncTbl->ReadWithECC(wCS, dwPpn, pBuf->pData,
                                        pBuf->pSpare, NULL, NULL, bDisableWhitening);
        // in case we are no ready to process clean pages mark the page as ecc error if it is clean
        if (boolCleanCheck == FALSE32 && nFILRet == FIL_SUCCESS_CLEAN)
        {
            nFILRet = FIL_U_ECC_ERROR;

            _ReportECCFailureSingle(wCS, dwPpn);
        }
    }
#ifndef AND_READONLY    
    if (((nFILRet == FIL_U_ECC_ERROR) && (bMarkECCForScrub == TRUE32)) || (bMarkECCForScrub == TRUE32 && vflBlockRefreshTrigger != NULL && vflBlockRefreshTrigger[dwVpn/PAGES_PER_SUBLK] >= RETIRE_ON_REFRESH_TRESHOLD))
    {
        UInt16 tCS;
        UInt16 tcbN;
        _Vpn2Cbn(dwVpn, &tCS, &tcbN);
        _AddBlockToScrubList(tCS, tcbN);
        if(vflBlockRefreshTrigger != NULL)
            vflBlockRefreshTrigger[dwVpn/PAGES_PER_SUBLK] = 0;
    }
#endif    

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
    VFL_LOG_PRINT((TEXT("[VFL: IN] ++VFL_Write(dwVpn:%d)\n"), dwVpn));

#ifdef AND_COLLECT_STATISTICS
    stVFLStatistics.ddwSingleWriteCallCnt++;
#endif

    /* write page data					*/
    if (VFL_WriteMultiplePagesInVb(dwVpn, ((UInt16)1), pBuf->pData, pBuf->pSpare, boolReplaceBlockOnFail, bDisableWhitening) != TRUE32)
    {
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
    UInt16 wPbn;
    Int32 nRetVal = VFL_CRITICAL_ERROR;
    UInt16 wBankIdx;
    BOOL32 boolAligned = TRUE32, boolCorrectBank = TRUE32;
    UInt16 wFailingCE = CE_STATUS_UNINITIALIZED;
    UInt32 dwFailingBlock = BLOCK_STATUS_UNINITIALIZED;
    UInt32 numReplacements;
    BOOL32 foundBlock;
    BOOL32 bReplaceBadBlockResult;
    Int32  nFILRet = FIL_CRITICAL_ERROR;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++VSVFL_Erase(wVbn:%d)\n"), wVbn));

#ifdef AND_COLLECT_STATISTICS
    stVFLStatistics.ddwBlocksErasedCnt++;
    stVFLStatistics.ddwEraseCallCnt++;
#endif

    for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
    {
        UInt16 wCS;

        _ConvertT2P(wBankIdx, _Vbn2Tbn(wVbn), &wPbn);
        wCS = _Bank2CS(wBankIdx);

        pawPbnRemapped[wBankIdx] = wPbn;

        if (_IsBlockInScrubList(wCS, pawPbnRemapped[wBankIdx]) == TRUE32)
        {
            _ReplaceBadBlock(wCS, wPbn);
            RECALC_VFL_CXT_CHECK_SUM(wCS);
            _RemoveBlockFromScrubList(wCS, pawPbnRemapped[wBankIdx]);
            pawPbnRemapped[wBankIdx] = _RemapBlock(wCS, wPbn, &boolAligned, &boolCorrectBank);
        }
        pawPbnRemapped[wBankIdx] = _RemapBlock(wCS, wPbn, NULL, NULL);
        pawPbnRemapped[wBankIdx] = _Cbn2Pbn(pawPbnRemapped[wBankIdx]);
    }
    if (pFuncTbl->EraseMultiple != NULL)
    {
        numReplacements = 0;
        do
        {
            dwFailingBlock = BLOCK_STATUS_UNINITIALIZED;
            wFailingCE = CE_STATUS_UNINITIALIZED;
            nFILRet = pFuncTbl->EraseMultiple(pawPbnRemapped, boolAligned, boolCorrectBank, &wFailingCE, &dwFailingBlock);
            if ( nFILRet == FIL_SUCCESS)            {
                return VFL_SUCCESS;
            }
            
            _ReportEraseFailureMultiple(wFailingCE, wVbn, nFILRet);

            if (!bReplaceOnFail)
            {
                return VFL_CRITICAL_ERROR;
            }

            VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_Erase failed erasing in parallel Vbn 0x%X Pbn 0x%X on CE %d! (line:%d)\n"), wVbn, dwFailingBlock, wFailingCE, __LINE__));

            if (CE_STATUS_UNINITIALIZED == wFailingCE)
            {
                VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_Erase failing CE not provided; Vbn 0x%X on all banks will be replaced!\n"), wVbn));
            }
            else if (BLOCK_STATUS_UNINITIALIZED == dwFailingBlock)
            {
                VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_Erase failing block not provided; Vbn 0x%X on all banks of CE %d will be replaced!\n"), wVbn, wFailingCE));
            }

            // now we have to mark failing block bad and to replace
            foundBlock = FALSE32;
            for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
            {
                if ((CE_STATUS_UNINITIALIZED == wFailingCE) ||
                    ((_Bank2CS(wBankIdx) == wFailingCE) &&
                     ((BLOCK_STATUS_UNINITIALIZED == dwFailingBlock) ||
                      (pawPbnRemapped[wBankIdx] == dwFailingBlock))))
                {
                    _ConvertT2P(wBankIdx, _Vbn2Tbn(wVbn), &wPbn);
        
                    CHECK_VFL_CXT_CHECK_SUM(_Bank2CS(wBankIdx));
                    pstVFLMeta[_Bank2CS(wBankIdx)].stVFLCxt.wNumOfEraseFail++;
                    bReplaceBadBlockResult = _ReplaceBadBlock(_Bank2CS(wBankIdx), wPbn);
                    RECALC_VFL_CXT_CHECK_SUM(_Bank2CS(wBankIdx));
                    if ( bReplaceBadBlockResult != TRUE32)
                    {
                        VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Erase failed _ReplaceBadBlk\n")));
                        return VFL_CRITICAL_ERROR;
                    }
                    if (_StoreVFLCxt(_Bank2CS(wBankIdx)) != TRUE32)
                    {
                        VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Erase failed _StoreVFLCxt\n")));
                        return VFL_CRITICAL_ERROR;
                    }
                    if (nRetVal != VFL_CRITICAL_ERROR)
                    {
#ifdef AND_COLLECT_STATISTICS
                        stVFLStatistics.ddwEraseTimeoutFailCnt++;
#endif
                    }

                    foundBlock = TRUE32;
                    numReplacements++;
                    pawPbnRemapped[wBankIdx] = _Cbn2Pbn(_RemapBlock(_Bank2CS(wBankIdx), wPbn, NULL, NULL));
                }
            }
            if ( !foundBlock )
            {
                VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Erase failed, but no blocks were replaced!\n")));
                return VFL_CRITICAL_ERROR;
            }
        }
        while (numReplacements <= (UInt32)(2 * BANKS_TOTAL));

        if (numReplacements > (UInt32)(2 * BANKS_TOTAL))
        {
            VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_Erase failed after attemting block replacements %d times\n"), numReplacements));
            return VFL_CRITICAL_ERROR;
        }
    }

    // if EraseMultiple not found then sequentially erase blocks
    for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
    {
        UInt8 bTrialIdx;
        UInt16 wCS, wPbnRemapped;

        _ConvertT2P(wBankIdx, _Vbn2Tbn(wVbn), &wPbn);
        wCS = _Bank2CS(wBankIdx);

        /* erase blocks						*/
        for (bTrialIdx = 0; bTrialIdx < WMR_NUM_OF_ERASE_TRIALS_FOR_DATA_BLOCKS; bTrialIdx++)
        {
            /* calculate replaced physical block address */
            wPbnRemapped = _RemapBlock(wCS, wPbn, NULL, NULL);
            nRetVal = pFuncTbl->Erase(wCS, _Cbn2Pbn(wPbnRemapped));

            if (nRetVal == FIL_SUCCESS)
            {
                break;
            }
            else
            {
                _ReportEraseFailureSingle(wCS, wPbn, nRetVal);

                VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_Erase failed erasing pbn 0x%X bank %d status = 0x%X (line:%d)!\n"), wPbnRemapped, wBankIdx, nRetVal, __LINE__));
                if (!bReplaceOnFail)
                {
                    return VFL_CRITICAL_ERROR;
                }
                CHECK_VFL_CXT_CHECK_SUM(wCS);
                pstVFLMeta[wCS].stVFLCxt.wNumOfEraseFail++;
                bReplaceBadBlockResult = _ReplaceBadBlock(wCS, wPbn);
                RECALC_VFL_CXT_CHECK_SUM(wCS);
                if ( bReplaceBadBlockResult != TRUE32)
                {
                    VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Erase failed _ReplaceBadBlk\n")));
                    return VFL_CRITICAL_ERROR;
                }
                if (_StoreVFLCxt(_Bank2CS(wBankIdx)) != TRUE32)
                {
                    VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Erase failed _StoreVFLCxt\n")));
                    return VFL_CRITICAL_ERROR;
                }
                if (nRetVal != FIL_WRITE_FAIL_ERROR)
                {
#ifdef AND_COLLECT_STATISTICS
                    stVFLStatistics.ddwEraseTimeoutFailCnt++;
#endif
                }
            }
        }
        if (bTrialIdx == WMR_NUM_OF_ERASE_TRIALS_FOR_DATA_BLOCKS)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Erase failed erasing / replacing vbn 0x%X bank %d!\n"), wVbn, wBankIdx));
            break;
        }
    }
    VFL_LOG_PRINT((TEXT("[VFL:OUT] --VSVFL_Erase(wVbn:%d)\n"), wVbn));

    WMR_ASSERT(nRetVal == FIL_SUCCESS);
    return nRetVal;
}

static UInt16
_CalcNumOfVFLSuBlk(void)
{
    // Any blocks above the binary size will be used as spares
    UInt32 wRoundedBlocksPerBank = 1UL << WMR_LOG2(BLOCKS_PER_BANK);

    return VSVFL_USER_BLKS_PER_1K * (wRoundedBlocksPerBank / 1024);
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
    BOOL32 bReplaceBadBlockResult;
    UInt16 wCSIdx;
    VFLCxt  *pVFLCxt;
    UInt16 wNumOfVFLSuBlk;
    VendorSpecificType vendorSpecificType = (VendorSpecificType)pFuncTbl->GetDeviceInfo(AND_DEVINFO_VENDOR_SPECIFIC_TYPE);

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++VFL_Format()\n")));

    WMR_MEMSET(pstVFLMeta, 0, sizeof(VFLMeta) * CS_TOTAL);

    stVSVFLWMRDeviceInfo.dwVFLAreaStart = dwBootAreaSize;
    
#if (defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)
    if (vendorSpecificType != FIL_VS_SIMPLE)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_Format context vs-type is not simple (line %d)\n"), __LINE__));
        return VFL_CRITICAL_ERROR;
    }
#endif

    _SetUpVendorSpecificRelatedParams(vendorSpecificType);
    // moved out of the init since we need VFL to open or format to know
    // what the size of the super block is
    if (pstadwScatteredReadWritePpn == NULL && pstawScatteredReadWriteCS == NULL)
    {
        pstadwScatteredReadWritePpn = (UInt32 *)WMR_MALLOC(PAGES_PER_SUBLK * sizeof(UInt32));
        pstawScatteredReadWriteCS = (UInt16 *)WMR_MALLOC(PAGES_PER_SUBLK * sizeof(UInt16));

        if (pstadwScatteredReadWritePpn == NULL || pstawScatteredReadWriteCS == NULL)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  there is no memory to initialize pstadwScatteredReadPpn or pstawScatteredReadCS!\n")));
            return VFL_CRITICAL_ERROR;
        }
    }

    if (!_initWriteMultipleTables())
    {
        return VFL_CRITICAL_ERROR;
    }
    
    wNumOfVFLSuBlk = _CalcNumOfVFLSuBlk();
    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        UInt8   *pabBBT = pstabCSBBTArea[wCSIdx];
        Buffer  *pBuf = NULL;
        UInt16 wIdx, wPbn, wBankIdx;

        if (VFL_ReadBBT(wCSIdx, pabBBT) == FALSE32)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_ReadBBT(%d) fail!\n"), wCSIdx));
            return VFL_CRITICAL_ERROR;
        }
        pstVFLMeta[wCSIdx].dwVersion = VFL_META_VERSION;
        pVFLCxt = GET_VFLCxt(wCSIdx);

        // set the number super blocks
        pVFLCxt->wNumOfVFLSuBlk = wNumOfVFLSuBlk;
        pVFLCxt->wNumOfFTLSuBlk = wNumOfVFLSuBlk;
        WMR_MEMCPY(pVFLCxt->abVSFormtType, &vendorSpecificType, 4);

        pVFLCxt->dwGlobalCxtAge = wCSIdx;
        for (wIdx = 0; wIdx < FTL_CXT_SECTION_SIZE; wIdx++)
        {
            pVFLCxt->awFTLCxtVbn[wIdx] = (UInt16)(wIdx);
        }

        pBuf = BUF_Get(BUF_MAIN_AND_SPARE);

        if (pBuf == NULL)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  BUF_Get fail (line:%d)!\n"), __LINE__));
            return VFL_CRITICAL_ERROR;
        }

		pVFLCxt->wNumOfInitBadBlk = 0;
		
		//count initial bad block number and save in VFLCxt
		for (wIdx = 0; wIdx < BLOCKS_PER_CS; wIdx++)
		{
			if (_isBlockGood(pabBBT, wIdx) != TRUE32)
			{
				pVFLCxt->wNumOfInitBadBlk++;
			}
		}
		
        /* initial bad check of VFL_INFO_SECTION */
        for (wIdx = 0, wPbn = VFL_FIRST_BLK_TO_SEARCH_CXT; wIdx < VFL_INFO_SECTION_SIZE && wPbn < VFL_LAST_BLK_TO_SEARCH_CXT; wPbn++)
        {
            if (_isBlockGood(pabBBT, wPbn))
            {
                /* check that the block is erasable */
                UInt16 wEraseCnt;
                for (wEraseCnt = 0; wEraseCnt < WMR_NUM_OF_ERASE_TRIALS_FOR_INDEX_BLOCKS; wEraseCnt++)
                {
                    Int32 nFILRet;
                    nFILRet = pFuncTbl->Erase(wCSIdx, _Cbn2Pbn(wPbn));
                    if (nFILRet == FIL_SUCCESS)
                    {
                        break;
                    }
                    _ReportEraseFailureSingle(wCSIdx, wPbn, nFILRet);
                }
                if (wEraseCnt == WMR_NUM_OF_ERASE_TRIALS_FOR_INDEX_BLOCKS)
                {
                    VFL_WRN_PRINT((TEXT("[VFL:ERR] VFL info section: exceeded erase limit at wCSIdx %x wPbn %x (line:%d)!\n"), wCSIdx, wPbn, __LINE__));
                }
                else
                {
                    pVFLCxt->awInfoBlk[wIdx++] = wPbn;
                }
            }
            else
            {
                //VFL_WRN_PRINT((TEXT("[VFL:ERR] VFL info section: bad block at wCSIdx %x wPbn %x (line:%d)!\n"), wCSIdx, wPbn, __LINE__));
            }
        }
        if (wPbn == VFL_LAST_BLK_TO_SEARCH_CXT)
        {
            VFL_ERR_PRINT((TEXT("[VFL:ERR]  wPbn == VFL_LAST_BLK_TO_SEARCH_CXT (line:%d)!\n"), __LINE__));
            return VFL_CRITICAL_ERROR;
        }

        /* initial bad check of reserved area */
        for (wBankIdx = 0; wBankIdx < BANKS_PER_CS; wBankIdx++)
        {
            for (wIdx = 0; wIdx < VFL_RESERVED_SECTION_SIZE_BANK; wIdx++)
            {
                UInt16 wPbn;
                UInt16 wTbn = wIdx + _GetNumOfSuBlks();

                _ConvertT2P(_CSBank2Bank(wCSIdx, wBankIdx), wTbn, &wPbn);
                if (_isBlockGood(pabBBT, wPbn))
                {
                    pVFLCxt->awBadMapTable[(VFL_RESERVED_SECTION_SIZE_BANK * wBankIdx) + wIdx] = VFL_BAD_MAP_TABLE_AVAILABLE_MARK;
                }
                else
                {
                    pVFLCxt->awBadMapTable[(VFL_RESERVED_SECTION_SIZE_BANK * wBankIdx) + wIdx] = VFL_BAD_MAP_TABLE_BAD_MARK;
                }
            }
        }
        // replace blocks we assigned to VFL_INFO_SECTION_SIZE
        for (wIdx = 0; wIdx <= pVFLCxt->awInfoBlk[VFL_INFO_SECTION_SIZE - 1]; wIdx++)
        {
            bReplaceBadBlockResult = _ReplaceBadBlock(wCSIdx, wIdx);
            RECALC_VFL_CXT_CHECK_SUM(wCSIdx);

            if ( bReplaceBadBlockResult == FALSE32)
            {
                VFL_ERR_PRINT((TEXT("[VFL:ERR]  _ReplaceBadBlock failed (line:%d)!\n"), __LINE__));
                return VFL_CRITICAL_ERROR;
            }
        }
        for (wBankIdx = 0; wBankIdx < BANKS_PER_CS; wBankIdx++)
        {
            for (wIdx = 0; wIdx < _GetNumOfSuBlks(); wIdx++)
            {
                UInt16 wPbn;
                _ConvertT2P(_CSBank2Bank(wCSIdx, wBankIdx), wIdx, &wPbn);
                if ((wPbn > pVFLCxt->awInfoBlk[VFL_INFO_SECTION_SIZE - 1]) && _isBlockGood(pabBBT, wPbn) == FALSE32)
                {
                    bReplaceBadBlockResult = _ReplaceBadBlock(wCSIdx, wPbn);
                    RECALC_VFL_CXT_CHECK_SUM(wCSIdx);
                    if ( bReplaceBadBlockResult == FALSE32)
                    {
                        VFL_ERR_PRINT((TEXT("[VFL:ERR]  _ReplaceBadBlock failed (line:%d)!\n"), __LINE__));
                        return VFL_CRITICAL_ERROR;
                    }
                }
            }
        }

        BUF_Release(pBuf);
        RECALC_VFL_CXT_CHECK_SUM(wCSIdx);
        if (_StoreVFLCxt(wCSIdx) == FALSE32)
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
void
VFL_Close(void)
{
    UInt16 wCSIdx;

    VFL_LOG_PRINT((TEXT("[VFL: IN] ++VFL_Close()\n")));

    /* release buffer manager			*/
    BUF_Close();

#ifndef AND_READONLY
    if (NULL != vflBlockRefreshTrigger)
    {
        WMR_FREE(vflBlockRefreshTrigger, _GetNewestVFLCxt()->wNumOfVFLSuBlk);
        vflBlockRefreshTrigger = NULL;
    }
#endif

    /* release VFL meta				*/
    if (pstVFLMeta != NULL)
    {
        WMR_FREE(pstVFLMeta, sizeof(VFLMeta) * CS_TOTAL);
        pstVFLMeta = NULL;
    }

#ifndef AND_READONLY
    /* release pawPbnOriginal				*/
    if (pawPbnOriginal != NULL)
    {
        WMR_FREE(pawPbnOriginal, sizeof(*pawPbnOriginal) * BANKS_TOTAL);
        pawPbnOriginal = NULL;
    }

    /* release pawPbn				*/
    if (pawPbn != NULL)
    {
        WMR_FREE(pawPbn, sizeof(*pawPbn) * BANKS_TOTAL);
        pawPbn = NULL;
    }

    /* release pawPbnUnmapped				*/
    if (pawPbnUnmapped != NULL)
    {
        WMR_FREE(pawPbnUnmapped, sizeof(*pawPbnUnmapped) * BANKS_TOTAL);
        pawPbnUnmapped = NULL;
    }
#endif

    /* release pawPbnRemapped				*/
    if (pawPbnRemapped != NULL)
    {
        WMR_FREE(pawPbnRemapped, sizeof(*pawPbnRemapped) * BANKS_TOTAL);
        pawPbnRemapped = NULL;
    }

    /* release padwPpnRemapped				*/
    if (padwPpnRemapped != NULL)
    {
        WMR_FREE(padwPpnRemapped, sizeof(*padwPpnRemapped) * BANKS_TOTAL);
        padwPpnRemapped = NULL;
    }

    if (pstadwScatteredReadWritePpn != NULL)
    {
        WMR_FREE(pstadwScatteredReadWritePpn, (PAGES_PER_SUBLK * sizeof(UInt32)));
        pstadwScatteredReadWritePpn = NULL;
    }

    if (pstawScatteredReadWriteCS != NULL)
    {
        WMR_FREE(pstawScatteredReadWriteCS, (PAGES_PER_SUBLK * sizeof(UInt16)));
        pstawScatteredReadWriteCS = NULL;
    }
    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        WMR_FREE(pstabCSBBTArea[wCSIdx], BBT_SIZE_PER_CS);
        pstabCSBBTArea[wCSIdx] = NULL;
    }

#ifdef AND_COLLECT_STATISTICS
    WMR_MEMSET(&stVFLStatistics, 0, sizeof(VFLStatistics));
#endif
    pFuncTbl = NULL;

    VFL_LOG_PRINT((TEXT("[VFL:OUT] --VFL_Close()\n")));

    return;
}

static VFLCxt * _GetNewestVFLCxt(void)
{
    UInt32 wCSIdx;
    VFLCxt  *pNewestVFLCxt = NULL;
    VFLCxt  *pVFLCxt = NULL;
    UInt32 aMaxGlobalAge = 0;

    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        pVFLCxt = GET_VFLCxt(wCSIdx);

        if (pVFLCxt->dwGlobalCxtAge >= aMaxGlobalAge)
        {
            aMaxGlobalAge = pVFLCxt->dwGlobalCxtAge;
            pNewestVFLCxt = pVFLCxt;
        }
    }
    return pNewestVFLCxt;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      VFL_GetFTLCxtVbn                                                     */
/* DESCRIPTION                                                               */
/*      This function is returning the recent FTL context block position     */
/* PARAMETERS                                                                */
/*		aFTLCxtVbn  [OUT]   Recent FTL context block list					 */
/* RETURN VALUES                                                             */
/* 		none                                                                 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static UInt16*
VFL_GetFTLCxtVbn(void)
{
    return _GetNewestVFLCxt()->awFTLCxtVbn;
}

#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      VFL_ChangeFTLCxtVbn	                                                 */
/* DESCRIPTION                                                               */
/*      This function change the virtual block number of FTL context block   */
/* PARAMETERS                                                                */
/*      awFTLCxtVbn  [IN]    FTL context block list                           */
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
    UInt16 wCSIdx;

    wWritePosition = (UInt16)(gdwMaxGlobalAge % CS_TOTAL);

    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        CHECK_VFL_CXT_CHECK_SUM(wCSIdx);
        /* make sure all are up to date so later storing of VFL Cxt would be correct */
        WMR_MEMCPY(pstVFLMeta[wCSIdx].stVFLCxt.awFTLCxtVbn, pwaFTLCxtVbn, FTL_CXT_SECTION_SIZE * sizeof(UInt16));
        RECALC_VFL_CXT_CHECK_SUM(wCSIdx);
    }

    if (!_StoreVFLCxt(wWritePosition))
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL context write fail !\n")));
        return VFL_CRITICAL_ERROR;
    }

    return VFL_SUCCESS;
}

static BOOL32
_SetFTLType(UInt32 dwFTLType)
{
    UInt16 wWritePosition;
    UInt16 wCSIdx;
    VFLCxt  *pVFLCxt = NULL;

    wWritePosition = (UInt16)(gdwMaxGlobalAge % CS_TOTAL);

    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        pVFLCxt = &pstVFLMeta[wCSIdx].stVFLCxt;
        CHECK_VFL_CXT_CHECK_SUM(wCSIdx);
        /* make sure all are up to date so later storing of VFL Cxt would be correct */
        pVFLCxt->dwFTLType = dwFTLType;
        // setting the FTL should come from the FTL format function
        // clear any new VFL settings we had...
        WMR_MEMSET(pVFLCxt->abFTLFormat, 0, FTL_FORMAT_STRUCT_SIZE);
        RECALC_VFL_CXT_CHECK_SUM(wCSIdx);
    }

    if (!_StoreVFLCxt(wWritePosition))
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _SetFTLType fail !\n")));
        return FALSE32;
    }

    return TRUE32;
}

static BOOL32
_SetNewFTLType(UInt32 dwFTLType)
{
    UInt16 wWritePosition;
    UInt16 wCSIdx;
    VFLCxt  *pVFLCxt = NULL;
    FTLFormatType * pFTLFormatType;

    wWritePosition = (UInt16)(gdwMaxGlobalAge % CS_TOTAL);

    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        pVFLCxt = &pstVFLMeta[wCSIdx].stVFLCxt;
        pFTLFormatType = (FTLFormatType *)pVFLCxt->abFTLFormat;

        CHECK_VFL_CXT_CHECK_SUM(wCSIdx);
        WMR_MEMCPY(pFTLFormatType->abNewNumOfFTLSuBlk, &pVFLCxt->wNumOfFTLSuBlk, sizeof(UInt16));
        WMR_MEMCPY(pFTLFormatType->abNewFTLType, &dwFTLType, sizeof(UInt32));
        pFTLFormatType->bFormatFTL = 1; // mark for format
        RECALC_VFL_CXT_CHECK_SUM(wCSIdx);
    }

    if (!_StoreVFLCxt(wWritePosition))
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  _SetNewFTLType fail !\n")));
        return FALSE32;
    }

    return TRUE32;
}

static BOOL32
_SetNewNumOfFTLSuBlks(UInt16 wNumOfFTLSuBlks)
{
    UInt16 wWritePosition;
    UInt16 wCSIdx;
    VFLCxt  *pVFLCxt = NULL;
    FTLFormatType * pFTLFormatType;

    wWritePosition = (UInt16)(gdwMaxGlobalAge % CS_TOTAL);

    if (wNumOfFTLSuBlks > _GetNumOfSuBlks()) {
        WMR_PANIC("invalid burnin-size settings: %d FTL, %d VFL sublks", wNumOfFTLSuBlks, _GetNumOfSuBlks());
    }

    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        pVFLCxt = &pstVFLMeta[wCSIdx].stVFLCxt;
        pFTLFormatType = (FTLFormatType *)pVFLCxt->abFTLFormat;

        CHECK_VFL_CXT_CHECK_SUM(wCSIdx);
        // make sure we keep the current FTL type
        WMR_MEMCPY(pFTLFormatType->abNewFTLType, &pVFLCxt->dwFTLType, sizeof(UInt32));
        WMR_MEMCPY(pFTLFormatType->abNewNumOfFTLSuBlk, &wNumOfFTLSuBlks, sizeof(UInt16));
        pFTLFormatType->bFormatFTL = 1; // mark for format
        RECALC_VFL_CXT_CHECK_SUM(wCSIdx);
    }

    if (!_StoreVFLCxt(wWritePosition))
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  _SetNewNumOfFTLSuBlks fail !\n")));
        return FALSE32;
    }

    return TRUE32;
}

static BOOL32
_SetNumOfFTLSuBlks(UInt16 wNumOfFTLSuBlks)
{
    UInt16 wWritePosition;
    UInt16 wCSIdx;

    wWritePosition = (UInt16)(gdwMaxGlobalAge % CS_TOTAL);

    if (wNumOfFTLSuBlks > _GetNumOfSuBlks()) {
        WMR_PANIC("invalid burnin-size settings: %d FTL, %d VFL sublks", wNumOfFTLSuBlks, _GetNumOfSuBlks());
    }

    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        CHECK_VFL_CXT_CHECK_SUM(wCSIdx);
        /* make sure all are up to date so later storing of VFL Cxt would be correct */
        if (wNumOfFTLSuBlks == 0)
        {
            pstVFLMeta[wCSIdx].stVFLCxt.wNumOfFTLSuBlk = pstVFLMeta[wCSIdx].stVFLCxt.wNumOfVFLSuBlk;
        }
        else
        {
            pstVFLMeta[wCSIdx].stVFLCxt.wNumOfFTLSuBlk = wNumOfFTLSuBlks;
        }
        RECALC_VFL_CXT_CHECK_SUM(wCSIdx);
    }

    if (!_StoreVFLCxt(wWritePosition))
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _SetNewNumOfFTLSuBlks fail !\n")));
        return FALSE32;
    }

    return TRUE32;
}

static BOOL32
_SetNewFTLParams(void)
{
    UInt16 wWritePosition;
    UInt16 wCSIdx;
    VFLCxt  *pVFLCxt = NULL;
    FTLFormatType * pFTLFormatType;

    wWritePosition = (UInt16)(gdwMaxGlobalAge % CS_TOTAL);

    for (wCSIdx = 0; wCSIdx < CS_TOTAL; wCSIdx++)
    {
        pVFLCxt = &pstVFLMeta[wCSIdx].stVFLCxt;
        pFTLFormatType = (FTLFormatType *)pVFLCxt->abFTLFormat;

        CHECK_VFL_CXT_CHECK_SUM(wCSIdx);
        // make sure we keep the current FTL type
        WMR_MEMCPY(&pVFLCxt->dwFTLType, pFTLFormatType->abNewFTLType, sizeof(UInt32));
        WMR_MEMCPY(&pVFLCxt->wNumOfFTLSuBlk, pFTLFormatType->abNewNumOfFTLSuBlk, sizeof(UInt16));
        if (pVFLCxt->wNumOfFTLSuBlk == 0)
        {
            pVFLCxt->wNumOfFTLSuBlk = pVFLCxt->wNumOfVFLSuBlk;
        }
        RECALC_VFL_CXT_CHECK_SUM(wCSIdx);
    }

    if (!_StoreVFLCxt(wWritePosition))
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL context write fail !\n")));
        return FALSE32;
    }

    return TRUE32;
}

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


BOOL32 VFL_WriteMultiplePagesInVb(UInt32 dwVpn, UInt16 wNumPagesToWrite, UInt8 * pbaData, UInt8 * pbaSpare, BOOL32 boolReplaceBlockOnFail, BOOL32 bDisableWhitening)
{
    UInt16 wPOffsetIdx, wCurrPOffset;
    UInt16 wBankIdx;
    UInt16 wPagesToWriteNow;
    BOOL32 boolAligned = TRUE32, boolCorrectBank = TRUE32;
    UInt16 wVbn, wStartPOffset;

    UInt16 wFailingCE = CE_STATUS_UNINITIALIZED;
    UInt32 wFailingPage = PAGE_STATUS_UNINITIALIZED;
    Int32  nFILRet = FIL_CRITICAL_ERROR;

    wStartPOffset = (UInt16)(dwVpn % PAGES_PER_SUBLK);
    wVbn = _Vpn2Vbn(dwVpn);

    // WMR_ASSERT((wStartPOffset + wNumPagesToWrite) < (PAGES_PER_SUBLK + 1));
    if ((wStartPOffset + wNumPagesToWrite) > PAGES_PER_SUBLK)
    {
        return FALSE32;
    }

#ifdef AND_COLLECT_STATISTICS
    stVFLStatistics.ddwPagesWrittenCnt += wNumPagesToWrite;
    stVFLStatistics.ddwMultipleWriteCallCnt++;
#endif

    for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
    {
        UInt16 wPbn;
        _ConvertT2P(wBankIdx, _Vbn2Tbn(wVbn), &wPbn);
        pawPbnUnmapped[wBankIdx] = wPbn;
        pawPbnRemapped[wBankIdx] = _RemapBlock(_Bank2CS(wBankIdx), wPbn, &boolAligned, &boolCorrectBank);
    }
    /* write the first few sectors until we are BANKS_TOTAL aligned - start */
    wCurrPOffset = wStartPOffset;
    wPagesToWriteNow = WMR_MIN(wNumPagesToWrite, (BANKS_TOTAL - (wCurrPOffset % BANKS_TOTAL)));
    if (wPagesToWriteNow != 0 && wPagesToWriteNow != BANKS_TOTAL)
    {
        for (wPOffsetIdx = 0; wPOffsetIdx < wPagesToWriteNow; wPOffsetIdx++)
        {
            UInt16 wBank = _Vpn2Bank((wPOffsetIdx + wCurrPOffset));
            // prepare the data for write scattered
            pstawScatteredReadWriteCS[wPOffsetIdx] = _Bank2CS(wBank);
            pawPbn[wPOffsetIdx] = pawPbnRemapped[wBank];
            pawPbnOriginal[wPOffsetIdx] = pawPbnUnmapped[wBank];
            pstadwScatteredReadWritePpn[wPOffsetIdx] = _Cbn2Ppn(pawPbn[wPOffsetIdx], ((wPOffsetIdx + wCurrPOffset) / BANKS_TOTAL));
        }
        if (pFuncTbl->WriteScatteredPages != NULL)
        {
            nFILRet = pFuncTbl->WriteScatteredPages(pstawScatteredReadWriteCS, pstadwScatteredReadWritePpn, pbaData, pbaSpare, wPagesToWriteNow, &wFailingCE, bDisableWhitening, &wFailingPage);
            if (nFILRet != FIL_SUCCESS)
            {
                VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_WriteMultiplePagesInVb(wVbn:0x%0X) (line:%d) fail!\n"), wVbn, __LINE__));

                if (wFailingCE < CS_TOTAL)
                {
                    CHECK_VFL_CXT_CHECK_SUM(wFailingCE);
                    pstVFLMeta[wFailingCE].stVFLCxt.wNumOfWriteFail++;
                    RECALC_VFL_CXT_CHECK_SUM(wFailingCE);                   
                }
                _ReportWriteScatteredFailBlock(wFailingCE,wVbn,wFailingPage,pstawScatteredReadWriteCS,pstadwScatteredReadWritePpn,wPagesToWriteNow, nFILRet);

                if (boolReplaceBlockOnFail == TRUE32)
                {
                    _MarkWriteScatteredFailBlockForDelete(wFailingCE, dwVpn, wFailingPage, pstawScatteredReadWriteCS, pstadwScatteredReadWritePpn, pawPbnOriginal, wPagesToWriteNow);
                }

                return FALSE32;
            }
            pbaData += (wPagesToWriteNow * BYTES_PER_PAGE);
            pbaSpare += (wPagesToWriteNow * BYTES_PER_METADATA_RAW);
        }
        else
        {
            for (wPOffsetIdx = 0; wPOffsetIdx < wPagesToWriteNow; wPOffsetIdx++)
            {
                nFILRet = pFuncTbl->Write(pstawScatteredReadWriteCS[wPOffsetIdx], pstadwScatteredReadWritePpn[wPOffsetIdx], pbaData, pbaSpare, bDisableWhitening);
                if (nFILRet != FIL_SUCCESS)
                {
                    VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_WriteMultiplePagesInVb(wVbn:0x%0X) (line:%d) fail!\n"), wVbn, __LINE__));

                    wFailingCE = pstawScatteredReadWriteCS[wPOffsetIdx]; // We know the failingCE since we only tried a single.
                    CHECK_VFL_CXT_CHECK_SUM(wFailingCE);
                    pstVFLMeta[wFailingCE].stVFLCxt.wNumOfWriteFail++;
                    RECALC_VFL_CXT_CHECK_SUM(wFailingCE);
                    _ReportWriteFailureSingle(wFailingCE, pstadwScatteredReadWritePpn[wPOffsetIdx], nFILRet);

                    if (boolReplaceBlockOnFail == TRUE32)
                    {
                        _MarkWriteFailBlockForDeleteWithPbn(wFailingCE, pawPbnOriginal[wPOffsetIdx]);
                    }

                    return FALSE32;
                }
                pbaData += BYTES_PER_PAGE;
                pbaSpare += BYTES_PER_METADATA_RAW;
            }
        }
        // recalc the offsets
        wCurrPOffset += wPagesToWriteNow;
    }
    /* write the first few sectors until we are BANKS_TOTAL aligned - end */

    /* write full super pages using write multiple - start */
    if (((wStartPOffset + wNumPagesToWrite) - wCurrPOffset) > BANKS_TOTAL)
    {
        wPagesToWriteNow = ((((wStartPOffset + wNumPagesToWrite) - wCurrPOffset) / BANKS_TOTAL) * BANKS_TOTAL);
        for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
        {
            padwPpnRemapped[wBankIdx] = _Cbn2Ppn(pawPbnRemapped[wBankIdx], (wCurrPOffset / BANKS_TOTAL));
            // for now - we check here - later this check should be done when generating pawPbnRemapped
        }

        nFILRet = pFuncTbl->WriteMultiplePages(padwPpnRemapped, pbaData, pbaSpare, wPagesToWriteNow, boolAligned, boolCorrectBank, &wFailingCE, bDisableWhitening, &wFailingPage);
        if ( nFILRet != FIL_SUCCESS)
        {
            VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_WriteMultiplePagesInVb(wVbn:0x%0X) (line:%d) fail!\n"), wVbn, __LINE__));

            if (wFailingCE < CS_TOTAL)
            {
                CHECK_VFL_CXT_CHECK_SUM(wFailingCE);
                pstVFLMeta[wFailingCE].stVFLCxt.wNumOfWriteFail++;
                RECALC_VFL_CXT_CHECK_SUM(wFailingCE);
            }
            _ReportWriteMultipleFailBlock(wFailingCE,wVbn,wFailingPage,padwPpnRemapped,wPagesToWriteNow, nFILRet);

            if (boolReplaceBlockOnFail == TRUE32)
            {
                _MarkWriteMultipleFailBlockForDelete(wFailingCE, dwVpn, wFailingPage, pawPbnUnmapped, pawPbnRemapped, padwPpnRemapped, wPagesToWriteNow);
            }

            return FALSE32;
        }
        pbaData += wPagesToWriteNow * BYTES_PER_PAGE;
        pbaSpare += wPagesToWriteNow * BYTES_PER_METADATA_RAW;
        wCurrPOffset += wPagesToWriteNow;
    }
    /* write full super pages using write multiple - end */

    /* write the last not banks aligned pages - start */
    wPagesToWriteNow = (wStartPOffset + wNumPagesToWrite) - wCurrPOffset;
    if (wPagesToWriteNow != 0)
    {
        for (wPOffsetIdx = 0; wPOffsetIdx < wPagesToWriteNow; wPOffsetIdx++)
        {
            UInt16 wBank = _Vpn2Bank((wPOffsetIdx + wCurrPOffset));
            // prepare the data for write scattered
            pstawScatteredReadWriteCS[wPOffsetIdx] = _Bank2CS(wBank);
            pawPbn[wPOffsetIdx] = pawPbnRemapped[wBank];
            pawPbnOriginal[wPOffsetIdx] = pawPbnUnmapped[wBank];
            pstadwScatteredReadWritePpn[wPOffsetIdx] = _Cbn2Ppn(pawPbn[wPOffsetIdx], ((wPOffsetIdx + wCurrPOffset) / BANKS_TOTAL));
        }

        if (pFuncTbl->WriteScatteredPages != NULL)
        {
            nFILRet = pFuncTbl->WriteScatteredPages(pstawScatteredReadWriteCS, pstadwScatteredReadWritePpn, pbaData, pbaSpare, wPagesToWriteNow, &wFailingCE, bDisableWhitening, &wFailingPage);
            if (nFILRet != FIL_SUCCESS)
            {
                VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_WriteMultiplePagesInVb(wVbn:0x%0X) (line:%d) fail!\n"), wVbn, __LINE__));

                if (wFailingCE < CS_TOTAL)
                {
                    CHECK_VFL_CXT_CHECK_SUM(wFailingCE);
                    pstVFLMeta[wFailingCE].stVFLCxt.wNumOfWriteFail++;
                    RECALC_VFL_CXT_CHECK_SUM(wFailingCE);
                }
                _ReportWriteScatteredFailBlock(wFailingCE,wVbn,wFailingPage,pstawScatteredReadWriteCS,pstadwScatteredReadWritePpn,wPagesToWriteNow, nFILRet);

                if (boolReplaceBlockOnFail == TRUE32)
                {
                    _MarkWriteScatteredFailBlockForDelete(wFailingCE, dwVpn, wFailingPage, pstawScatteredReadWriteCS, pstadwScatteredReadWritePpn, pawPbnOriginal, wPagesToWriteNow);
                }

                return FALSE32;
            }
        }
        else
        {
            for (wPOffsetIdx = 0; wPOffsetIdx < wPagesToWriteNow; wPOffsetIdx++)
            {
                nFILRet = pFuncTbl->Write(pstawScatteredReadWriteCS[wPOffsetIdx], pstadwScatteredReadWritePpn[wPOffsetIdx], pbaData, pbaSpare, bDisableWhitening);
                if (nFILRet != FIL_SUCCESS)
                {
                    VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_WriteMultiplePagesInVb(wVbn:0x%0X) (line:%d) fail!\n"), wVbn, __LINE__));

                    wFailingCE = pstawScatteredReadWriteCS[wPOffsetIdx]; // We know the failingCE since we only tried a single.
                    CHECK_VFL_CXT_CHECK_SUM(wFailingCE);
                    pstVFLMeta[wFailingCE].stVFLCxt.wNumOfWriteFail++;
                    RECALC_VFL_CXT_CHECK_SUM(wFailingCE);
                    _ReportWriteFailureSingle(wFailingCE, pstadwScatteredReadWritePpn[wPOffsetIdx], nFILRet);

                    if (boolReplaceBlockOnFail == TRUE32)
                    {
                        _MarkWriteFailBlockForDeleteWithPbn(wFailingCE, pawPbnOriginal[wPOffsetIdx]);
                    }

                    return FALSE32;
                }
                pbaData += BYTES_PER_PAGE;
                pbaSpare += BYTES_PER_METADATA_RAW;
            }
        }
    }
    // no need to recalc offsets here...
    /* write the last not banks aligned pages - end */
    return TRUE32;
}
#endif

#if (!AND_DISABLE_READ_MULTIPLE)

BOOL32 VFL_ReadMultiplePagesInVb(UInt16 wVbn, UInt16 wStartPOffset, UInt16 wNumPagesToRead, UInt8 * pbaData, UInt8 * pbaSpare, BOOL32 * pboolNeedRefresh, UInt8 * pdwaSectorStats, BOOL32 bDisableWhitening)
{
    UInt16 wBankIdx, wPagesToReadNow;
    UInt16 wPOffsetIdx, wCurrPOffset;
    UInt8 bECCCorrectedBits;

#ifdef AND_COLLECT_STATISTICS
    stVFLStatistics.ddwPagesReadCnt += wNumPagesToRead;
    stVFLStatistics.ddwSequetialReadCallCnt++;
#endif

    for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
    {
        UInt16 wPbn;
        _ConvertT2P(wBankIdx, _Vbn2Tbn(wVbn), &wPbn);
        pawPbnRemapped[wBankIdx] = _RemapBlock(_Bank2CS(wBankIdx), wPbn, NULL, NULL);
    }

    if (pboolNeedRefresh != NULL)
    {
        *pboolNeedRefresh = FALSE32;
    }
    /* read the first few sectors until we are BANKS_TOTAL aligned - start */
    wCurrPOffset = wStartPOffset;

    wPagesToReadNow = WMR_MIN(wNumPagesToRead, (BANKS_TOTAL - (wCurrPOffset % BANKS_TOTAL)));
    if (wPagesToReadNow != 0 && wPagesToReadNow != BANKS_TOTAL)
    {
        Int32 nRet;
        for (wPOffsetIdx = 0; wPOffsetIdx < wPagesToReadNow; wPOffsetIdx++)
        {
            UInt16 wBank = _Vpn2Bank((wPOffsetIdx + wCurrPOffset));
            // prepare the data for write scattered
            pstawScatteredReadWriteCS[wPOffsetIdx] = _Bank2CS(wBank);
            pstadwScatteredReadWritePpn[wPOffsetIdx] = _Cbn2Ppn(pawPbnRemapped[wBank], ((wPOffsetIdx + wCurrPOffset) / BANKS_TOTAL));
        }

        nRet = pFuncTbl->ReadScatteredPages(pstawScatteredReadWriteCS, pstadwScatteredReadWritePpn, pbaData, pbaSpare, wPagesToReadNow, &bECCCorrectedBits, pdwaSectorStats, bDisableWhitening);
        if ((nRet != FIL_SUCCESS) && (nRet != FIL_SUCCESS_CLEAN))
        {
            VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_ReadMultiplePagesInVb(wVbn:0x%0X) (line:%d) fail!\n"), wVbn, __LINE__));
            
            _ReportECCFailureMultiple(wVbn);
            return FALSE32;
        }
        if (bECCCorrectedBits >= FIL_ECC_REFRESH_TRESHOLD && pboolNeedRefresh != NULL)
        {
            *pboolNeedRefresh = TRUE32;
        }
        // recalc the offsets
        wCurrPOffset += wPagesToReadNow;
        pbaData += (wPagesToReadNow * BYTES_PER_PAGE);
        pbaSpare += (wPagesToReadNow * BYTES_PER_METADATA_RAW);
    }
    /* read the first few sectors until we are BANKS_TOTAL aligned - end */

    /* read full super pages using write multiple - start */
    if (((wStartPOffset + wNumPagesToRead) - wCurrPOffset) > BANKS_TOTAL)
    {
        Int32 nFILRet;
        wPagesToReadNow = ((((wStartPOffset + wNumPagesToRead) - wCurrPOffset) / BANKS_TOTAL) * BANKS_TOTAL);
        for (wBankIdx = 0; wBankIdx < BANKS_TOTAL; wBankIdx++)
        {
            padwPpnRemapped[wBankIdx] = _Cbn2Ppn(pawPbnRemapped[wBankIdx], (wCurrPOffset / BANKS_TOTAL));
        }

        nFILRet = pFuncTbl->ReadMultiplePages(padwPpnRemapped, pbaData, pbaSpare, wPagesToReadNow, &bECCCorrectedBits, pdwaSectorStats, bDisableWhitening);
        if (nFILRet != FIL_SUCCESS && nFILRet != FIL_SUCCESS_CLEAN)
        {
            _ReportECCFailureMultiple(wVbn);
            return FALSE32;
        }
        if (bECCCorrectedBits >= FIL_ECC_REFRESH_TRESHOLD && pboolNeedRefresh != NULL)
        {
            *pboolNeedRefresh = TRUE32;
        }
        pbaData += wPagesToReadNow * BYTES_PER_PAGE;
        pbaSpare += wPagesToReadNow * BYTES_PER_METADATA_RAW;
        wCurrPOffset += wPagesToReadNow;
    }
    /* read full super pages using write multiple - end */

    /* read the last not banks aligned pages - start */
    wPagesToReadNow = (wStartPOffset + wNumPagesToRead) - wCurrPOffset;
    if (wPagesToReadNow != 0)
    {
        Int32 nFILRet;
        for (wPOffsetIdx = 0; wPOffsetIdx < wPagesToReadNow; wPOffsetIdx++)
        {
            UInt16 wBank = _Vpn2Bank((wPOffsetIdx + wCurrPOffset));
            // prepare the data for write scattered
            pstawScatteredReadWriteCS[wPOffsetIdx] = _Bank2CS(wBank);
            pstadwScatteredReadWritePpn[wPOffsetIdx] = _Cbn2Ppn(pawPbnRemapped[wBank], ((wPOffsetIdx + wCurrPOffset) / BANKS_TOTAL));
        }
        nFILRet = pFuncTbl->ReadScatteredPages(pstawScatteredReadWriteCS, pstadwScatteredReadWritePpn, pbaData, pbaSpare, wPagesToReadNow, &bECCCorrectedBits, pdwaSectorStats, bDisableWhitening);
        if (nFILRet != FIL_SUCCESS && nFILRet != FIL_SUCCESS_CLEAN)
        {
            VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_ReadMultiplePagesInVb(wVbn:0x%0X) (line:%d) fail!\n"), wVbn, __LINE__));

            _ReportECCFailureMultiple(wVbn);

            return FALSE32;
        }
        if (bECCCorrectedBits >= FIL_ECC_REFRESH_TRESHOLD && pboolNeedRefresh != NULL)
        {
            *pboolNeedRefresh = TRUE32;
        }
    }
    /* read the last not banks aligned pages - end */
    return TRUE32;
}

#endif /* if (!AND_DISABLE_READ_MULTIPLE) */

BOOL32 VFL_ReadScatteredPagesInVb(UInt32 * padwVpn, UInt16 wNumPagesToRead, UInt8 * pbaData, UInt8 * pbaSpare, BOOL32 * pboolNeedRefresh, UInt8 * pdwaSectorStats, BOOL32 bDisableWhitening, Int32 *actualStatus)
{
    UInt16 wPageIdx;
    Int32 nRet;
    UInt8 bCorrectableECC = 0;

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
        // translate Vpn address to CS and dwPpn
        _Vpn2Ppn(padwVpn[wPageIdx], &pstawScatteredReadWriteCS[wPageIdx], &pstadwScatteredReadWritePpn[wPageIdx]);
    }

    nRet = pFuncTbl->ReadScatteredPages(pstawScatteredReadWriteCS, pstadwScatteredReadWritePpn, pbaData, pbaSpare, wNumPagesToRead, &bCorrectableECC, pdwaSectorStats, bDisableWhitening);
    if (bCorrectableECC >= FIL_ECC_REFRESH_TRESHOLD && pboolNeedRefresh != NULL)
    {
        VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_ReadScatteredPagesInVb mark page for refresh (line:%d)!\n"), __LINE__));
        *pboolNeedRefresh = TRUE32;
    }
    if(actualStatus != NULL)
        *actualStatus = nRet;
    if ((nRet != FIL_SUCCESS) && (nRet != FIL_SUCCESS_CLEAN))
    {
        _ReportECCFailureMultiple(_Vpn2Vbn(padwVpn[0])); // Make the best guess
        return FALSE32;
    }
    else
    {
        return TRUE32;
    }
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
        if (pstVFLMeta && (wCS < CS_TOTAL))
        {
            VFLMeta *pSrcMeta = &pstVFLMeta[wCS];
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, pSrcMeta, sizeof(*pSrcMeta));
        }
        break;
    }

    case AND_STRUCT_VFL_GET_TYPE:
    {
        UInt8 bVFLType = VFL_TYPE_VSVFL;
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
        UInt32 dwDevInfoSize = sizeof(stVSVFLWMRDeviceInfo);
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwDevInfoSize, sizeof(dwDevInfoSize));
        break;
    }

#ifdef AND_COLLECT_STATISTICS
    case  AND_STRUCT_VFL_DEVICEINFO:
    {
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &stVSVFLWMRDeviceInfo, sizeof(stVSVFLWMRDeviceInfo));
        break;
    }
#endif
            
    case AND_STRUCT_VFL_FILSTATISTICS:
    {
        boolRes = FIL_GetStruct(AND_STRUCT_FIL_STATISTICS, pvoidStructBuffer, pdwStructSize);
        break;
    }

    case AND_STRUCT_VFL_NUM_OF_FTL_SUBLKS:
    {
        UInt16 dwNumFTLSuBlks = _GetNumOfFTLSuBlks();
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwNumFTLSuBlks, sizeof(dwNumFTLSuBlks));
        break;
    }

    case AND_STRUCT_VFL_NUM_OF_SUBLKS:
    {
        UInt16 dwNumVFLSuBlks = _GetNumOfSuBlks();
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwNumVFLSuBlks, sizeof(dwNumVFLSuBlks));
        break;
    }

    case AND_STRUCT_VFL_GETADDRESS:
    {
        ANDAddressStruct stAddress;
        
        if (pvoidStructBuffer)
        {
            UInt16 wCS = 0;
            
            WMR_MEMCPY(&stAddress, pvoidStructBuffer, sizeof(stAddress));
            _Vpn2Ppn(stAddress.dwVpn, &wCS, &stAddress.dwPpn);
            stAddress.dwCS = wCS;
            
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &stAddress, sizeof(stAddress));
        }
        else if (pdwStructSize)
        {
            *pdwStructSize = sizeof(stAddress);
            boolRes = TRUE32;
        }
                 
        break;
    }

#ifdef AND_COLLECT_STATISTICS
    case AND_STRUCT_VFL_BURNIN_CODE:
    {
        UInt32 dwBurnInCodeTrunc = (UInt32) stVFLStatistics.ddwBurnInCode;
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwBurnInCodeTrunc, sizeof(dwBurnInCodeTrunc));
        break;
    }
#endif //AND_COLLECT_STATISTICS

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
		UInt16 wBytesPerMetaData = (UInt16) pFuncTbl->GetDeviceInfo(AND_DEVINFO_FIL_META_BUFFER_BYTES);
		boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wBytesPerMetaData, sizeof(wBytesPerMetaData));
		break;
	}
			
	case AND_STRUCT_VFL_VALID_BYTES_PER_META:
	{
		UInt16 wBytesPerMetaData = (UInt16) pFuncTbl->GetDeviceInfo(AND_DEVINFO_FIL_META_VALID_BYTES);
		boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wBytesPerMetaData, sizeof(wBytesPerMetaData));
		break;
	}

    case AND_STRUCT_VFL_NAND_TYPE:
    {
        UInt32 nandType = NAND_TYPE_RAW;   
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &nandType,
                                  sizeof(nandType));
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

    case AND_STRUCT_VFL_PUBLIC_FBBT:
        // Fall Through
    case AND_STRUCT_VFL_FACTORY_BBT:
    {
        boolRes = _GetFactoryBBTBuffer(pvoidStructBuffer, pdwStructSize);
        break;
    }

    case AND_STRUCT_VFL_PAGES_PER_BLOCK:
    {
        UInt16 wPagesPerBlock = (UInt16) pFuncTbl->GetDeviceInfo(AND_DEVINFO_PAGES_PER_BLOCK);
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &wPagesPerBlock, sizeof(wPagesPerBlock));
        break;
    }

    case AND_STRUCT_VFL_LAST_ERROR:
    {
        boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &gstLastFailure, sizeof(gstLastFailure));
        break;
    }
            
#if AND_SUPPORT_BLOCK_BORROWING
    case AND_STRUCT_VFL_FIND_BORROWED_BLOCKS:
    {
        UInt32 maxBlocks;
        if (NULL != pdwStructSize)
        {
            BorrowBlockAddress *pastrBlocks = (BorrowBlockAddress*)pvoidStructBuffer;
            maxBlocks = *pdwStructSize / sizeof(pastrBlocks[0]);
            boolRes = _VSVFL_FindBorrowedBlocks(pastrBlocks, &maxBlocks);
            *pdwStructSize = maxBlocks * sizeof(pastrBlocks[0]);
        }
        break;
    }
#endif

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
#ifndef AND_READONLY
    case AND_STRUCT_VFL_FTLTYPE:
    {
        _SetFTLType(*((UInt32*)pvoidStructBuffer));
        break;
    }

    case AND_STRUCT_VFL_NUM_OF_FTL_SUBLKS:
    {
        _SetNumOfFTLSuBlks(*((UInt16*)pvoidStructBuffer));
        break;
    }

    case AND_STRUCT_VFL_NEW_FTLTYPE:
    {
        _SetNewFTLType(*((UInt32*)pvoidStructBuffer));
        break;
    }

    case AND_STRUCT_VFL_NEW_NUM_OF_FTL_SUBLKS:
    {
        _SetNewNumOfFTLSuBlks(*((UInt16*)pvoidStructBuffer));
        break;
    }
#endif

    case AND_STRUCT_VFL_BURNIN_CODE:
    {
#ifdef AND_COLLECT_STATISTICS
        stVFLStatistics.ddwBurnInCode = *((UInt32*)pvoidStructBuffer);
#else
        boolRes = FALSE32;
#endif
        break;
    }

    case AND_STRUCT_VFL_STATISTICS:
    {
#ifdef AND_COLLECT_STATISTICS
        WMR_MEMCPY(&stVFLStatistics, pvoidStructBuffer, WMR_MIN(sizeof(VFLStatistics), dwStructSize));
#else
        boolRes = FALSE32;
#endif
        break;
    }

#ifndef AND_READONLY
    case AND_STRUCT_VFL_FILSTATISTICS:
    {
#ifdef AND_COLLECT_STATISTICS
        FIL_SetStruct(AND_STRUCT_FIL_STATISTICS, pvoidStructBuffer, dwStructSize);
#else
        boolRes = FALSE32;
#endif
        break;
    }
#endif // ! AND_READONLY

    default:
        VFL_WRN_PRINT((TEXT("[VFL:WRN]  VFL_SetStruct 0x%X is not identified is VFL data struct identifier!\n"), dwStructType));
        boolRes = FALSE32;
    }
    return boolRes;
}

static UInt16 VFL_GetVbasPerVb(UInt16 wVbn)
{
    return PAGES_PER_SUBLK;
}

void VSVFL_Register(VFLFunctions * pVFL_Functions)
{
#ifndef AND_READONLY
    pVFL_Functions->Format = VFL_Format;
    pVFL_Functions->WriteSinglePage = VFL_Write;
    pVFL_Functions->Erase = VFL_Erase;
    pVFL_Functions->ChangeFTLCxtVbn = VFL_ChangeFTLCxtVbn;
    pVFL_Functions->WriteMultiplePagesInVb = VFL_WriteMultiplePagesInVb;
#if AND_SUPPORT_BLOCK_BORROWING
    pVFL_Functions->BorrowSpareBlock = _VSVFL_BorrowBlock;
#endif //AND_SUPPORT_BLOCK_BORROWING
#endif // ! AND_READONLY
    pVFL_Functions->Init = VFL_Init;
    pVFL_Functions->Open = VFL_Open;
    pVFL_Functions->Close = VFL_Close;
#if (!AND_DISABLE_READ_MULTIPLE)
    pVFL_Functions->ReadMultiplePagesInVb = VFL_ReadMultiplePagesInVb;
#else /* if (!AND_DISABLE_READ_MULTIPLE) */
    pVFL_Functions->ReadMultiplePagesInVb = NULL;
#endif /* if (!AND_DISABLE_READ_MULTIPLE) */
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
        dwRetVal = (UInt32)_GetNumOfFTLSuBlks();
        break;
    }

    case AND_DEVINFO_FTL_TYPE:
    {
        dwRetVal = (UInt32)_GetFTLType();
        break;
    }

    case AND_DEVINFO_FTL_NEED_FORMAT:
    {
        FTLFormatType * pFTLFormatType = (FTLFormatType *)pstVFLMeta[0].stVFLCxt.abFTLFormat;
        dwRetVal = (UInt32)(pFTLFormatType->bFormatFTL);
        break;
    }
    case AND_DEVINFO_FIL_LBAS_PER_PAGE:
    {
        dwRetVal = stVSVFLWMRDeviceInfo.dwLogicalPageSize;
        break;
    }
    case AND_DEVINFO_FIL_META_BUFFER_BYTES:
    {
        dwRetVal = stVSVFLWMRDeviceInfo.dwTotalMetaPerLogicalPage;
        break;
    }
    case AND_DEVINFO_FIL_META_VALID_BYTES:
    {
        dwRetVal = stVSVFLWMRDeviceInfo.dwValidMetaPerLogicalPage;
        break;
    }

    default:
        VFL_ERR_PRINT((TEXT("[VFL:ERR]  VFL_GetDeviceInfo dwParamType not supported (0x%X) (line:%d)!\n"), dwParamType, __LINE__));
        break;
    }
    return dwRetVal;
}

// =============================================================================
// Private Addressing and Timing Functions
// =============================================================================

// direct block and page mapping address "conversions" (i.e. noop)
void _pfnConvert_Direct(UInt32 dwBankOrCE, UInt32 dwPpnOrCpn, UInt32* pdwCEOrBank, UInt32* pdwCpnOrPpn)
{
    *pdwCEOrBank = dwBankOrCE;
    *pdwCpnOrPpn = dwPpnOrCpn;
}
#if !(defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)
// generic address conversion functions
void _Helper_ConvertP2C_OneBitReorder(UInt32 dwBank, UInt32 dwPpn, UInt32* pdwCE, UInt32* pdwCpn, UInt32 dwReorderMask)
{
    const UInt32 dwBelowReorderMask = dwReorderMask - 1;    // Assumption:  dwReorderMask is a power of 2!
    const UInt32 dwAboveReorderMask = ~dwBelowReorderMask;

    // insert reorder bit back in correct position of "chip" page number by extracting from MSB of "physical" bank number
    *pdwCpn = ((dwPpn & dwBelowReorderMask) |
               (((dwBank / CS_TOTAL) & 0x1) ? dwReorderMask : 0) |
               ((dwPpn & dwAboveReorderMask) << 1));

    // strip reorder bit from MSB of "physical" bank number to produce "chip" CE
    *pdwCE = dwBank % CS_TOTAL;
}

void _Helper_ConvertC2P_OneBitReorder(UInt32 dwCE, UInt32 dwCpn, UInt32* pdwBank, UInt32* pdwPpn, UInt32 dwReorderMask)
{
    const UInt32 dwBelowReorderMask = dwReorderMask - 1;    // Assumption:  dwReorderMask is a power of 2!
    const UInt32 dwAboveReorderMask = ~dwBelowReorderMask << 1;

    // remove reorder bit from "chip" page number to produce "physical" page number
    *pdwPpn = ((dwCpn & dwBelowReorderMask) |
               ((dwCpn & dwAboveReorderMask) >> 1));

    // prepend psuedo-bank bit to "chip" CE as MSB of "physical" bank number
    *pdwBank = dwCE + ((dwCpn & dwReorderMask) ? CS_TOTAL : 0);
}
void _Helper_ConvertP2C_TwoBitReorder(UInt32 dwBank, UInt32 dwPpn, UInt32* pdwCE, UInt32* pdwCpn, UInt32 dw1stReorderMask, UInt32 dw2ndReorderMask)
{
    const BOOL32 bool1stIsUpper = (dw1stReorderMask > dw2ndReorderMask);

    const UInt32 dwUpperReorderMask = bool1stIsUpper ? dw1stReorderMask : dw2ndReorderMask;
    const UInt32 dwLowerReorderMask = bool1stIsUpper ? dw2ndReorderMask : dw1stReorderMask;

    const UInt32 dwBelowReorderMask = dwLowerReorderMask - 1;
    const UInt32 dwAboveReorderMask = ~(dwUpperReorderMask - 1);
    const UInt32 dwBetweenReorderMask = ~(dwBelowReorderMask | dwAboveReorderMask);

    // insert reorder bits back in correct positions of "chip" page number
    // by extracting from two most significant bits of "physical" bank number
    *pdwCpn = ((dwPpn & dwBelowReorderMask) |
               ((dwPpn & dwBetweenReorderMask) << 1) |
               ((dwPpn & dwAboveReorderMask) << 2) |
               (((dwBank / CS_TOTAL) & 0x1) ? dw1stReorderMask : 0) |
               (((dwBank / CS_TOTAL) & 0x2) ? dw2ndReorderMask : 0));

    // strip reorder bits from two most significant bits of "physical" bank number
    // to produce "chip" CE
    *pdwCE = dwBank % CS_TOTAL;
}
void _Helper_ConvertC2P_TwoBitReorder(UInt32 dwCE, UInt32 dwCpn, UInt32* pdwBank, UInt32* pdwPpn, UInt32 dw1stReorderMask, UInt32 dw2ndReorderMask)
{
    const BOOL32 bool1stIsUpper = (dw1stReorderMask > dw2ndReorderMask);

    const UInt32 dwUpperReorderMask = bool1stIsUpper ? dw1stReorderMask : dw2ndReorderMask;
    const UInt32 dwLowerReorderMask = bool1stIsUpper ? dw2ndReorderMask : dw1stReorderMask;

    const UInt32 dwBelowReorderMask = dwLowerReorderMask - 1;
    const UInt32 dwAboveReorderMask = ~(dwUpperReorderMask - 1) << 1;
    const UInt32 dwBetweenReorderMask = ~(dwBelowReorderMask | dwLowerReorderMask | dwUpperReorderMask | dwAboveReorderMask);

    // remove reorder bits from "chip" page number to produce "physical" page number
    *pdwPpn = ((dwCpn & dwBelowReorderMask) |
               ((dwCpn & dwBetweenReorderMask) >> 1) |
               ((dwCpn & dwAboveReorderMask) >> 2));

    // prepend psuedo-bank bits to "chip" CE as most significant bits of "physical" bank number
    *pdwBank = (((dwCpn & dw2ndReorderMask) ? (2 * CS_TOTAL) : 0) +
                ((dwCpn & dw1stReorderMask) ? CS_TOTAL : 0) +
                dwCE);
}

// address conversions for alternating two planes, addressed on least significant block address
void _pfnConvertP2C_TwoPlaneLSB(UInt32 dwBank, UInt32 dwPpn, UInt32* pdwCE, UInt32* pdwCpn)
{
    _Helper_ConvertP2C_OneBitReorder(dwBank, dwPpn, pdwCE, pdwCpn, PAGES_PER_BLOCK);
}
void _pfnConvertC2P_TwoPlaneLSB(UInt32 dwCE, UInt32 dwCpn, UInt32* pdwBank, UInt32* pdwPpn)
{
    _Helper_ConvertC2P_OneBitReorder(dwCE, dwCpn, pdwBank, pdwPpn, PAGES_PER_BLOCK);
}

// address conversions for alternating two dies, addressed on most significant block address
void _pfnConvertP2C_TwoDieMSB(UInt32 dwBank, UInt32 dwPpn, UInt32* pdwCE, UInt32* pdwCpn)
{
    _Helper_ConvertP2C_OneBitReorder(dwBank, dwPpn, pdwCE, pdwCpn, PAGES_PER_CS >> 1);
}
void _pfnConvertC2P_TwoDieMSB(UInt32 dwCE, UInt32 dwCpn, UInt32* pdwBank, UInt32* pdwPpn)
{
    _Helper_ConvertC2P_OneBitReorder(dwCE, dwCpn, pdwBank, pdwPpn, PAGES_PER_CS >> 1);
}

// address conversions for alternating two planes, addressed on least significant block address,
// then two dies, addressed on most significant block address (i.e. D0:P0, D0:P1, D1:P0, D1:P1)
void _pfnConvertP2C_TwoPlaneLSBTwoDieMSB(UInt32 dwBank, UInt32 dwPpn, UInt32* pdwCE, UInt32* pdwCpn)
{
    _Helper_ConvertP2C_TwoBitReorder(dwBank, dwPpn, pdwCE, pdwCpn, PAGES_PER_BLOCK, PAGES_PER_CS >> 1);
}
void _pfnConvertC2P_TwoPlaneLSBTwoDieMSB(UInt32 dwCE, UInt32 dwCpn, UInt32* pdwBank, UInt32* pdwPpn)
{
    _Helper_ConvertC2P_TwoBitReorder(dwCE, dwCpn, pdwBank, pdwPpn, PAGES_PER_BLOCK, PAGES_PER_CS >> 1);
}

// address conversions for alternating between Toshiba-style districts
void _pfnConvertP2C_ToshibaTwoDistrict(UInt32 dwBank, UInt32 dwPpn, UInt32* pdwCE, UInt32* pdwCpn)
{
    const UInt32 dwDistrictMask = PAGES_PER_BLOCK;  // Assumes this is a power of 2.

    _Helper_ConvertP2C_OneBitReorder(dwBank, dwPpn, pdwCE, pdwCpn, dwDistrictMask);
}
void _pfnConvertC2P_ToshibaTwoDistrict(UInt32 dwCE, UInt32 dwCpn, UInt32* pdwBank, UInt32* pdwPpn)
{
    const UInt32 dwDistrictMask = PAGES_PER_BLOCK;  // Assumes this is a power of 2.

    _Helper_ConvertC2P_OneBitReorder(dwCE, dwCpn, pdwBank, pdwPpn, dwDistrictMask);
}

// Converts to CE / Cpn without power of 2 assumptions on # of 
// blocks per chip select. 
//  
// Assumed vendor ordering is as follows (for a 2-CE part): 
//  
//  bank    CE  Die Plane|District
//  
//   0       0   0      0
//   1       1   0      0
// ------------------------ nCE 
//   2       0   0      1
//   3       1   0      1
// ------------------------ 2 * nCE ( 2 = nPlanes )
//  
//   Which is to say that we alternate with the CS first, and then the Plane/District.
//  
//   This algorithm assumes that planes / districts are based
//   upon even/odd block #'s (even = plane0) and that there are
//   128 pages per block (meaning we need to preserve the seven
//   least significant bits of the Ppn).
void _pfnConvertP2C_SingleDie_7bitPlaneNoPower2(UInt32 dwBank, UInt32 dwPpn, UInt32* pdwCE, UInt32* pdwCpn)
{
    const UInt32 wPlane_x_Banks = 2 * CS_TOTAL; // Assumes 2 planes...
    const UInt32 fDistrictZero = ( (dwBank % wPlane_x_Banks)<CS_TOTAL ? 1 : 0 );

    UInt32 wBlockNum_x_2 = (dwPpn / PAGES_PER_BLOCK) <<  1;

    *pdwCE = ( dwBank % CS_TOTAL );
    if ( !fDistrictZero )
    {
        // District 1
        wBlockNum_x_2++;
    }
    *pdwCpn = PAGES_PER_BLOCK * wBlockNum_x_2 | (dwPpn & 0x7f); // Assumes 128 pages per block
}

// Converts to bank / Ppn without power of 2 assumption for 
// blocks per chip select. 
//  
// @see _pfnConvertP2C_TwoDie2_7bitPlaneNoPower2
void _pfnConvertC2P_SingleDie_7bitPlaneNoPower2(UInt32 dwCE, UInt32 dwCpn, UInt32* pdwBank, UInt32* pdwPpn)
{
    UInt32 wBlkNum;
    UInt32 fDistrictZero;

    const UInt32 wPlane_x_Banks = 2 * CS_TOTAL; // Assumes 2 planes...

    *pdwBank = dwCE;

    wBlkNum = dwCpn / PAGES_PER_BLOCK;
    fDistrictZero = ( 0 == (wBlkNum & 1) ? 1 : 0 );

    if ( !fDistrictZero )
    {
        // district 1
        wBlkNum--;
        *pdwBank += (wPlane_x_Banks / 2);
    }
    *pdwPpn = PAGES_PER_BLOCK * (wBlkNum >>  1) | (dwCpn & 0x7f); // Assumes 128 pages per block
}

// Converts to CE / Cpn without power of 2 assumptions on # of 
// blocks per chip select. 
//  
// Assumed vendor ordering is as follows (for a 2-CE part): 
//  
//  bank    CE  Die Plane|District
//  
//   0       0   0      0
//   1       1   0      0
// ------------------------ nCE 
//   2       0   0      1
//   3       1   0      1
// ------------------------ 2 * nCE ( 2 = nPlanes )
//   4       0   1      0
//   5       1   1      0
//   6       0   1      1
//   7       1   1      1
// ------------------------ 2 * nCE * nDie ( 2 = nPlanes )
//  
//   Which is to say that we alternate with the CS first,
//   Plane/District second and finally the Die.
//  
//   This algorithm assumes that planes / districts are based
//   upon even/odd block #'s (even = plane0) and that there are
//   128 pages per block (meaning we need to preserve the seven
//   least significant bits of the Ppn).
void _pfnConvertP2C_TwoDie2_7bitPlaneNoPower2(UInt32 dwBank, UInt32 dwPpn, UInt32* pdwCE, UInt32* pdwCpn)
{
    const UInt32 wPlane_x_Banks = 2 * CS_TOTAL; // Assumes 2 planes...
    const UInt32 fDistrictZero = ( (dwBank % wPlane_x_Banks)<CS_TOTAL ? 1 : 0 );
    const UInt32 fDieZero = ( dwBank<wPlane_x_Banks ? 1 : 0 );

    UInt32 wBlockNum_x_2 = (dwPpn / PAGES_PER_BLOCK) <<  1;

    *pdwCE = ( dwBank % CS_TOTAL );
    if ( !fDistrictZero )
    {
        // District 1
        wBlockNum_x_2++;
    }
    *pdwCpn = PAGES_PER_BLOCK * wBlockNum_x_2 | (dwPpn & 0x7f); // Assumes 128 pages per block

    if ( !fDieZero )
    {
        *pdwCpn += PAGES_PER_CS / 2;
    }
}

// Converts to bank / Ppn without power of 2 assumption for 
// blocks per chip select. 
//  
// @see _pfnConvertP2C_TwoDie2_7bitPlaneNoPower2
void _pfnConvertC2P_TwoDie2_7bitPlaneNoPower2(UInt32 dwCE, UInt32 dwCpn, UInt32* pdwBank, UInt32* pdwPpn)
{
    UInt32 wBlkNum;
    UInt32 fDistrictZero;

    const UInt32 wPlane_x_Banks = 2 * CS_TOTAL; // Assumes 2 planes...
    const UInt32 wPagesPerDie = (UInt32)(PAGES_PER_CS / 2);

    *pdwBank = dwCE;
    if ( dwCpn >=  wPagesPerDie )
    {
        dwCpn -= wPagesPerDie;
        *pdwBank += wPlane_x_Banks;
    }

    wBlkNum = dwCpn / PAGES_PER_BLOCK;
    fDistrictZero = ( 0 == (wBlkNum & 1) ? 1 : 0 );

    if ( !fDistrictZero )
    {
        // district 1
        wBlkNum--;
        *pdwBank += (wPlane_x_Banks / 2);
    }
    *pdwPpn = PAGES_PER_BLOCK * (wBlkNum >>  1) | (dwCpn & 0x7f); // Assumes 128 pages per block
}

// address conversions for alternating between Toshiba-style districts with extended areas
//
void _pfnConvertP2C_ToshibaTwoDistrictExt(UInt32 dwBank, UInt32 dwPpn, UInt32* pdwCE, UInt32* pdwCpn)
{
    const UInt32 dwExtBlocks = 128;
    const UInt32 dwExtPages = dwExtBlocks * PAGES_PER_BLOCK;
    const UInt32 dwExtDistrictMask = dwExtPages >> 2;

    const UInt32 dwMainBlocks = BLOCKS_PER_CS - dwExtBlocks;
    const UInt32 dwMainPages = dwMainBlocks * PAGES_PER_BLOCK;
    const UInt32 dwMainDistrictMask = dwMainPages >> 2;

    // note that the Main/Ext page boundary gets shifted down by one bit
    // to reflect that Ppn already has reorder bit removed from Cpn
    const UInt32 dwDistrictMask = ((dwMainPages >> 1) > dwPpn) ? dwMainDistrictMask : dwExtDistrictMask;

    _Helper_ConvertP2C_OneBitReorder(dwBank, dwPpn, pdwCE, pdwCpn, dwDistrictMask);
}
void _pfnConvertC2P_ToshibaTwoDistrictExt(UInt32 dwCE, UInt32 dwCpn, UInt32* pdwBank, UInt32* pdwPpn)
{
    const UInt32 dwExtBlocks = 128;
    const UInt32 dwExtPages = dwExtBlocks * PAGES_PER_BLOCK;
    const UInt32 dwExtDistrictMask = dwExtPages >> 2;

    const UInt32 dwMainBlocks = BLOCKS_PER_CS - dwExtBlocks;
    const UInt32 dwMainPages = dwMainBlocks * PAGES_PER_BLOCK;
    const UInt32 dwMainDistrictMask = dwMainPages >> 2;

    const UInt32 dwDistrictMask = (dwMainPages > dwCpn) ? dwMainDistrictMask : dwExtDistrictMask;

    _Helper_ConvertC2P_OneBitReorder(dwCE, dwCpn, pdwBank, pdwPpn, dwDistrictMask);
}
#endif // #if !(defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)
void _ConvertT2P_Default(UInt16 wBank, UInt16 wTbn, UInt16 *pwPbn)
{
    UInt32 dwCpn, dwCS;
    // note that the naming conventions are messed up here
    if (_pfnConvertP2C == NULL)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _pfnConvertP2C is not set (line:%d)!\n"), __LINE__));
        return;
    }
    _pfnConvertP2C((UInt32)wBank, (UInt32)(wTbn * PAGES_PER_BLOCK), &dwCS, &dwCpn);
    *pwPbn = (UInt16)(dwCpn / PAGES_PER_BLOCK);
}

// Note - this function return the bank offset with the CS (ast if the system as a single CS)
void _ConvertP2T_Default(UInt16 wPbn, UInt16 *pwBank, UInt16 *pwTbn)
{
    // note that the naming conventions are messed up here
    UInt32 dwPpn, dwBank;
    
    // note that the naming conventions are messed up here
    if (_pfnConvertC2P == NULL)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _pfnConvertC2P is not set (line:%d)!\n"), __LINE__));
        return;
    }
    _pfnConvertC2P((UInt32)0, (UInt32)(wPbn * PAGES_PER_BLOCK), &dwBank, &dwPpn);
    *pwTbn = (UInt16)(dwPpn / PAGES_PER_BLOCK);
    *pwBank = _Bank2CSBank((UInt16)dwBank);
}
#if !(defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)
// Note - this function return the bank offset with the CS (ast if the system as a single CS)
void _ConvertP2T_ToshibaTwoDie(UInt16 wPbn, UInt16 *pwBank, UInt16 *pwTbn)
{
    // note that the naming conventions are messed up here
    UInt32 dwPpn, dwBank;
    
    // note that the naming conventions are messed up here
    if (_pfnConvertC2P == NULL)
    {
        VFL_ERR_PRINT((TEXT("[VFL:ERR] _pfnConvertC2P is not set (line:%d)!\n"), __LINE__));
        return;
    }
    _pfnConvertC2P((UInt32)0, (UInt32)(wPbn * BLOCK_STRIDE), &dwBank, &dwPpn);
    *pwTbn = (UInt16)(dwPpn / BLOCK_STRIDE);
    *pwBank = _Bank2CSBank((UInt16)dwBank);
}
#endif // #if !(defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)
static BOOL32 _SetV2PFunctions(VendorSpecificType vendorSpecificType)
{
    switch (VS_GET_FMT(vendorSpecificType))
    {
        default:
        {
            _ConvertP2T = _ConvertP2T_Default;
            _ConvertT2P = _ConvertT2P_Default;
        }   
    }
    return TRUE32;
}

#if !(defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)
static BOOL32 _SetBankToCSConvertFunctionsOld(VendorSpecificType vendorSpecificType)
{
    switch (vendorSpecificType)
    {
    case FIL_VS_SIMPLE:
        _pfnConvertP2C = _pfnConvert_Direct;
        _pfnConvertC2P = _pfnConvert_Direct;
        break;

    case FIL_VS_HYNIX_2P:
        _pfnConvertP2C = _pfnConvertP2C_TwoPlaneLSB;
        _pfnConvertC2P = _pfnConvertC2P_TwoPlaneLSB;
        break;

    case FIL_VS_ONFI_2D:
        _pfnConvertP2C = _pfnConvertP2C_TwoDieMSB;
        _pfnConvertC2P = _pfnConvertC2P_TwoDieMSB;
        break;

    case FIL_VS_ONFI_2P:
        _pfnConvertP2C = _pfnConvertP2C_TwoPlaneLSB;
        _pfnConvertC2P = _pfnConvertC2P_TwoPlaneLSB;
        break;

    case FIL_VS_ONFI_2P_2D:
        _pfnConvertP2C = _pfnConvertP2C_TwoPlaneLSBTwoDieMSB;
        _pfnConvertC2P = _pfnConvertC2P_TwoPlaneLSBTwoDieMSB;
        break;

    case FIL_VS_ONFI_2D_CACHE:
        _pfnConvertP2C = _pfnConvertP2C_TwoDieMSB;
        _pfnConvertC2P = _pfnConvertC2P_TwoDieMSB;
        break;

    case FIL_VS_ONFI_2P_CACHE:
        _pfnConvertP2C = _pfnConvertP2C_TwoPlaneLSB;
        _pfnConvertC2P = _pfnConvertC2P_TwoPlaneLSB;
        break;

    case FIL_VS_ONFI_2P_2D_CACHE:
        _pfnConvertP2C = _pfnConvertP2C_TwoPlaneLSBTwoDieMSB;
        _pfnConvertC2P = _pfnConvertC2P_TwoPlaneLSBTwoDieMSB;
        break;

    case FIL_VS_SAMSUNG_2D:
        _pfnConvertP2C = _pfnConvertP2C_TwoDieMSB;
        _pfnConvertC2P = _pfnConvertC2P_TwoDieMSB;
        break;

    case FIL_VS_SAMSUNG_2P_2D:
        _pfnConvertP2C = _pfnConvertP2C_TwoPlaneLSBTwoDieMSB;
        _pfnConvertC2P = _pfnConvertC2P_TwoPlaneLSBTwoDieMSB;
        break;

    case FIL_VS_SAMSUNG_2P_2D_EXT:
        _pfnConvertP2C = _pfnConvertP2C_TwoDie2_7bitPlaneNoPower2;
        _pfnConvertC2P = _pfnConvertC2P_TwoDie2_7bitPlaneNoPower2;
        break;

    case FIL_VS_TOSHIBA_2P:
        _pfnConvertP2C = _pfnConvertP2C_ToshibaTwoDistrict;
        _pfnConvertC2P = _pfnConvertC2P_ToshibaTwoDistrict;
        break;

    case FIL_VS_TOSHIBA_2P_CACHE:
        _pfnConvertP2C = _pfnConvertP2C_ToshibaTwoDistrict;
        _pfnConvertC2P = _pfnConvertC2P_ToshibaTwoDistrict;
        break;

    case FIL_VS_TOSHIBA_2P_EXT:
        _pfnConvertP2C = _pfnConvertP2C_SingleDie_7bitPlaneNoPower2;
        _pfnConvertC2P = _pfnConvertC2P_SingleDie_7bitPlaneNoPower2;
        break;

    case FIL_VS_TOSHIBA_2P_EXT_CACHE:
        _pfnConvertP2C = _pfnConvertP2C_ToshibaTwoDistrictExt;
        _pfnConvertC2P = _pfnConvertC2P_ToshibaTwoDistrictExt;
        break;

    default:
        _pfnConvertP2C = _pfnConvert_Direct;
        _pfnConvertC2P = _pfnConvert_Direct;
        return FALSE32;
    }
    return TRUE32;
}
#endif // #if !(defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)

static BOOL32 _SetBankToCSConvertFunctions(VendorSpecificType vendorSpecificType)
{
#if !(defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)    
    void (*_pfnConvertP2CTemp)(UInt32 dwBank, UInt32 dwPpn, UInt32* pdwCE, UInt32* pdwCpn) = NULL;
    void (*_pfnConvertC2PTemp)(UInt32 dwCE, UInt32 dwCpn, UInt32* pdwBank, UInt32* pdwPpn) = NULL;
    _SetBankToCSConvertFunctionsOld(vendorSpecificType);

    _pfnConvertP2CTemp = _pfnConvertP2C;
    _pfnConvertC2PTemp = _pfnConvertC2P;

    switch (VS_GET_FMT(vendorSpecificType))
    {
    case VS_FMT_SIMPLE:
        _pfnConvertP2C = _pfnConvert_Direct;
        _pfnConvertC2P = _pfnConvert_Direct;
        break;

    case VS_FMT_2BANKS_LSB:
        _pfnConvertP2C = _pfnConvertP2C_TwoPlaneLSB;
        _pfnConvertC2P = _pfnConvertC2P_TwoPlaneLSB;
        break;

    case VS_FMT_2BANKS_MSB:
        _pfnConvertP2C = _pfnConvertP2C_TwoDieMSB;
        _pfnConvertC2P = _pfnConvertC2P_TwoDieMSB;
        break;

    case VS_FMT_4BANKS_LSB_MSB_NOPWR2:
        _pfnConvertP2C = _pfnConvertP2C_TwoDie2_7bitPlaneNoPower2;
        _pfnConvertC2P = _pfnConvertC2P_TwoDie2_7bitPlaneNoPower2;
        break;

    case VS_FMT_4BANKS_LSB_MSB:
        _pfnConvertP2C = _pfnConvertP2C_TwoPlaneLSBTwoDieMSB;
        _pfnConvertC2P = _pfnConvertC2P_TwoPlaneLSBTwoDieMSB;
        break;

    case VS_FMT_TOSHIBA_TWO_DISTRICT:
        _pfnConvertP2C = _pfnConvertP2C_ToshibaTwoDistrict;
        _pfnConvertC2P = _pfnConvertC2P_ToshibaTwoDistrict;
        break;

    case VS_FMT_TOSHIBA_TWO_DISTRICT_EXT:
        _pfnConvertP2C = _pfnConvertP2C_SingleDie_7bitPlaneNoPower2;
        _pfnConvertC2P = _pfnConvertC2P_SingleDie_7bitPlaneNoPower2;
        break;

    case VS_FMT_UNKNOWN:
    default:
        _pfnConvertP2C = _pfnConvert_Direct;
        _pfnConvertC2P = _pfnConvert_Direct;
        return FALSE32;
    }
    if (!(_pfnConvertP2CTemp == _pfnConvertP2C &&
          _pfnConvertC2PTemp == _pfnConvertC2P))
    {
        VFL_ERR_PRINT((TEXT("Comparing old and new _SetBankToCSConvertFunctions failure (line:%d)!\n"), __LINE__));
    }
#else
    _pfnConvertP2C = _pfnConvert_Direct;
    _pfnConvertC2P = _pfnConvert_Direct;
#endif // #if !(defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)
    return TRUE32;
}

#if !(defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)
static UInt16 _GetBanksPerCSOld(VendorSpecificType vendorSpecificType)
{
    UInt16 wBanksPerCS = 0xFFFF;

    switch (vendorSpecificType)
    {
    case FIL_VS_SIMPLE:
        wBanksPerCS = 1;
        break;

    case FIL_VS_HYNIX_2P:
        wBanksPerCS = 2;
        break;

    case FIL_VS_ONFI_2D:
        wBanksPerCS = 2;
        break;

    case FIL_VS_ONFI_2P:
        wBanksPerCS = 2;
        break;

    case FIL_VS_ONFI_2P_2D:
        wBanksPerCS = 4;
        break;

    case FIL_VS_ONFI_2D_CACHE:
        wBanksPerCS = 2;
        break;

    case FIL_VS_ONFI_2P_CACHE:
        wBanksPerCS = 2;
        break;

    case FIL_VS_ONFI_2P_2D_CACHE:
        wBanksPerCS = 4;
        break;

    case FIL_VS_SAMSUNG_2P_2D:
        wBanksPerCS = 4;
        break;

    case FIL_VS_SAMSUNG_2D:
        wBanksPerCS = 2;
        break;

    case FIL_VS_TOSHIBA_2P:
        wBanksPerCS = 2;
        break;

    case FIL_VS_TOSHIBA_2D:
        wBanksPerCS = 1;
        break;

    case FIL_VS_TOSHIBA_2P_CACHE:
        wBanksPerCS = 2;
        break;

    case FIL_VS_TOSHIBA_2P_EXT:
        wBanksPerCS = 2;
        break;

    case FIL_VS_TOSHIBA_2P_EXT_CACHE:
        wBanksPerCS = 2;
        break;

    case FIL_VS_UNKNOWN:
    default:
        break;
    }
    return wBanksPerCS;
}
#endif // #if !(defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)

static UInt16 _GetBanksPerCS(VendorSpecificType vendorSpecificType)
{
#if !(defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)    
    UInt16 wBanksPerCS = 0xFFFF, wBanksPerCSOld;

    wBanksPerCSOld = _GetBanksPerCSOld(vendorSpecificType);
    switch (VS_GET_FMT(vendorSpecificType))
    {
    case VS_FMT_TOSHIBA_TWO_DIE:		
    case VS_FMT_SIMPLE:
        wBanksPerCS = 1;
        break;

    case VS_FMT_TOSHIBA_TWO_DISTRICT:
    case VS_FMT_TOSHIBA_TWO_DISTRICT_EXT:
    case VS_FMT_2BANKS_MSB:
    case VS_FMT_2BANKS_LSB:
        wBanksPerCS = 2;
        break;

    case VS_FMT_4BANKS_LSB_MSB_NOPWR2:
    case VS_FMT_4BANKS_LSB_MSB:
        wBanksPerCS = 4;
        break;

    case VS_FMT_UNKNOWN:
    default:
        break;
    }
    if (wBanksPerCSOld != wBanksPerCS)
    {
        VFL_ERR_PRINT((TEXT("Comparing old and new _GetBanksPerCS failure (line:%d)!\n"), __LINE__));
    }
    return wBanksPerCS;
#else
    return 1;
#endif // #if !(defined(AND_FORCE_VS_SIMPLE) && AND_FORCE_VS_SIMPLE)
}

BOOL32 _GetFactoryBBTBuffer(void* pvoidStructBuffer, UInt32* pdwStructSize)
{
    UInt32 idx;
    UInt16 wCS;
    UInt8 *pabCursor = (UInt8*)pvoidStructBuffer;
    const UInt32 dwOutputBufferSize = CS_TOTAL * BBT_SIZE_PER_CS;
    
    if (!pvoidStructBuffer && pdwStructSize)
    {
        *pdwStructSize = dwOutputBufferSize;
        return TRUE32;
    }
    else if (!pvoidStructBuffer || !pdwStructSize || (*pdwStructSize < dwOutputBufferSize))
    {
        return FALSE32;
    }

    for (wCS = 0; wCS < CS_TOTAL; ++wCS)
    {
        if (!VFL_ReadBBTWithoutSpecial(wCS, pabCursor))
        {
            return FALSE32;
        }

        // Find VFL Meta blocks and set them to GOOD
        for (idx = 0;
             (idx < VFL_INFO_SECTION_SIZE) && (GET_VFLCxt(wCS)->awInfoBlk[idx] != VFL_INVALID_INFO_INDEX);
             ++idx)
        {
            _MarkBlockAsGoodInBBT(pabCursor, GET_VFLCxt(wCS)->awInfoBlk[idx]);
        }

        pabCursor += BBT_SIZE_PER_CS;
    }

    *pdwStructSize = dwOutputBufferSize;
    return TRUE32;
}
