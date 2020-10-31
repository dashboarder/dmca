/*****************************************************************************/
/*                                                                           */
/* COMPONENT   : Rainbow                                                     */
/* MODULE      : FTL                                                         */
/* NAME        : FTL interface                                               */
/* FILE        : FTLinterface.c                                              */
/* PURPOSE     : This file contains the exported routine for interfacing with*/
/*           	 the upper layer of FTL.                                     */
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
/*   22-JUL-2005 [Jaesung Jung] : first writing                              */
/*   03-NOV-2005 [Yangsup Lee ] : Add wear-leveling algorithm		     */
/*   13-NOV-2005 [Yangsup Lee ] : Remove checksum for MLC                    */
/*   22-NOV-2005 [Yangsup Lee ] : Modify wear-leveling algorithm             */
/*   16-FEB-2006 [Yangsup Lee ] : Remove write bitmap for Internal           */
/*                                Interleaving                               */
/*   08-MAR-2006 [Yangsup Lee ] : Don't merge FAT area                       */
/*   14-MAR-2006 [Yangsup Lee ] : Don't use bad mark area                    */
/*   29-MAR-2006 [Yangsup Lee ] : Modify wear-leveling algorithm(No grouping)*/
/*   31-MAR-2006 [Yangsup Lee ] : support ftl meta block wear leveling       */
/*                                                                           */
/*****************************************************************************/
#include "WMRConfig.h"
#include "ANDTypes.h"
#include "WMROAM.h"
#include "WMRBuf.h"
#include "VFLBuffer.h"
#include "VFL.h"
#include "FTLTypes.h"
#include "FTL.h"
#if AND_SUPPORT_LEGACY_FTL

#define GET_FTL_SPARE_TYPE(pSBuf)       ((UInt8 *)(pSBuf))[FTL_SPARE_TYPE_LOCATION]
#define SET_FTL_SPARE_TYPE(pSBuf, type)  ((UInt8 *)(pSBuf))[FTL_SPARE_TYPE_LOCATION] = (type)

#define FTL_ECC_MARK                (0x55)
#define GET_FTL_ECC_MARK(pSBuf)     (((UInt32 *)(pSBuf))[FTL_ECC_MARK_WORD] & FTL_ECC_MARK_MASK)
#define CHECK_FTL_ECC_MARK(pSBuf)   (GET_FTL_ECC_MARK((pSBuf)) == FTL_ECC_MARK_MASK ? TRUE32 : FALSE32) /* TRUE32 if there is no error FALSE32 if there is an error */
#define SET_FTL_ECC_MARK(pSBuf)     do { ((UInt8 *)(pSBuf))[FTL_ECC_MARK_BYTE] = FTL_ECC_MARK; ((UInt8 *)(pSBuf))[FTL_ECC_MARK_BYTE_LEGACY] = FTL_ECC_MARK; } while (0)

#define FTL_NUM_PAGES_PER_EC_TABLE          (((FTL_AREA_SIZE * sizeof(UInt16)) / BYTES_PER_PAGE) + (((FTL_AREA_SIZE * sizeof(UInt16)) % BYTES_PER_PAGE) ? 1 : 0))
#define FTL_NUM_PAGES_PER_RC_TABLE          (((FTL_AREA_SIZE * sizeof(UInt16)) / BYTES_PER_PAGE) + (((FTL_AREA_SIZE * sizeof(UInt16)) % BYTES_PER_PAGE) ? 1 : 0))
#define FTL_NUM_PAGES_PER_MAP_TABLE         (((USER_SUBLKS_TOTAL * sizeof(UInt16)) / BYTES_PER_PAGE) + (((USER_SUBLKS_TOTAL * sizeof(UInt16)) % BYTES_PER_PAGE) ? 1 : 0))
#define FTL_NUM_PAGES_PER_LOGCXT_MAP_TABLE  (((LOG_SECTION_SIZE * PAGES_PER_SUBLK * sizeof(UInt16)) / BYTES_PER_PAGE) + (((LOG_SECTION_SIZE * PAGES_PER_SUBLK * sizeof(UInt16)) % BYTES_PER_PAGE) ? 1 : 0))

#define FTL_NUM_PAGES_TO_WRITE_IN_STORECXT  (FTL_NUM_PAGES_PER_EC_TABLE + FTL_NUM_PAGES_PER_RC_TABLE + FTL_NUM_PAGES_PER_MAP_TABLE + FTL_NUM_PAGES_PER_LOGCXT_MAP_TABLE + 1 /* index */ + 1 /* statistics */)

#define FTL_NUM_PAGES_PER_CXT_WITHOUT_EC    (FTL_NUM_PAGES_PER_MAP_TABLE + 1)

#define CONVERT_L2V(l)                      (pstFTLCxt->pawMapTable[(l)])
#define MODIFY_LB_MAP(lb, vb)                (pstFTLCxt->pawMapTable[(lb)] = (vb))
#define GET_EC_PER_VB(vb)                   (pstFTLCxt->pawECCacheTable[(vb)])
#define GET_EC_PER_LB(lb)                   (GET_EC_PER_VB(CONVERT_L2V(lb)))

#define MARK_LOG_AS_EMPTY(pLog)             ((pLog)->wVbn = LOG_EMPTY_SLOT)
#define ASSIGN_VBN_TO_LOG(pLog, wVbnParam)  ((pLog)->wVbn = (wVbnParam))

#define IS_LBN_IN_RANGE(wLbnParam)          (((wLbnParam) == 0) || ((((wLbnParam) > 0)) && ((wLbnParam) < USER_SUBLKS_TOTAL)))
#define FTL_SET_LOG_LBN(pLogParam, wLbnParam)    \
    { \
        ((pLogParam)->wLbn = (wLbnParam)); \
         WMR_ASSERT(IS_LBN_IN_RANGE(wLbnParam)); \
         }

#define FTL_CHANGE_FTLCXT_TO_NEXT_BLOCK() \
    { pstFTLCxt->dwCurrMapCxtPage = GET_Vpn(GET_Vbn(pstFTLCxt->dwCurrMapCxtPage), (PAGES_PER_SUBLK - 1)); }
    
#define DIV2_ROUNDUP(a) ((((a) & 0x1) == 1) ? (((a)>>1)+1) : ((a)>>1))

/*****************************************************************************/
/* Debug Print #defines                                                      */
/*****************************************************************************/
#if (defined(AND_SIMULATOR) && AND_SIMULATOR)
#define     FTL_ERR_PRINT(x)            { WMR_RTL_PRINT(x); WMR_PANIC(); }
#else
#define     FTL_ERR_PRINT(x)            WMR_RTL_PRINT(x)
#endif
#define     FTL_WRN_PRINT(x)            WMR_RTL_PRINT(x)

#if defined (FTL_LOG_MSG_ON)
    #define     FTL_LOG_PRINT(x)            WMR_DBG_PRINT(x)
#else
    #define     FTL_LOG_PRINT(x)
#endif

#if defined (FTL_INF_MSG_ON)
    #define     FTL_INF_PRINT(x)            WMR_DBG_PRINT(x)
#else
    #define     FTL_INF_PRINT(x)
#endif

/*****************************************************************************/
/* Static variables definitions                                              */
/*****************************************************************************/
static FTLCxt2     *pstFTLCxt = NULL;
static FTLMeta     *pstFTLMeta = NULL;

WMR_BufZone_t FTL_BufZone;
static UInt8 * pstbaMultiplePagesWriteSpare = NULL;
#if !(defined(WMR_DISABLE_FTL_RECOVERY) && WMR_DISABLE_FTL_RECOVERY) || \
    !(defined(AND_READONLY) && AND_READONLY)
static UInt8 * pstSimpleMergeDataBuffer = NULL;
#endif
static UInt32 * pstadwScatteredReadVpn = NULL;
static VFLFunctions stVFLFunctions;
static FTLWMRDeviceInfo stFTLDeviceInfo;

#ifdef AND_COLLECT_STATISTICS
static FTLStatistics stFTLStatistics;
#endif /* AND_COLLECT_STATISTICS */

// make the function pointers in vfl struct look like the old vfl functions
#ifndef AND_READONLY
#define VFL_Format                  (stVFLFunctions.Format)
#define VFL_Write                   (stVFLFunctions.WriteSinglePage)
#define VFL_Erase                   (stVFLFunctions.Erase)
#define VFL_ChangeFTLCxtVbn         (stVFLFunctions.ChangeFTLCxtVbn)
#define VFL_WriteMultiplePagesInVb  (stVFLFunctions.WriteMultiplePagesInVb)
#endif // ! AND_READONLY
#define VFL_Init                    (stVFLFunctions.Init)
#define VFL_Open                    (stVFLFunctions.Open)
#define VFL_Close                   (stVFLFunctions.Close)
#define VFL_ReadMultiplePagesInVb   (stVFLFunctions.ReadMultiplePagesInVb)
#define VFL_ReadScatteredPagesInVb  (stVFLFunctions.ReadScatteredPagesInVb)
#define VFL_Read                    (stVFLFunctions.ReadSinglePage)
#define VFL_GetFTLCxtVbn            (stVFLFunctions.GetFTLCxtVbn)
//#define	VFL_GetAddress				(stVFLFunctions.GetAddress)
#define VFL_GetDeviceInfo           (stVFLFunctions.GetDeviceInfo)
#define VFL_GetStruct               (stVFLFunctions.GetStruct)
#define VFL_SetStruct               (stVFLFunctions.SetStruct)

/* maximum block erase count of each map */

/*****************************************************************************/
/* Definitions                                                               */
/*****************************************************************************/
#define     GET_LOGCxt(v)               (&(pstFTLCxt->aLOGCxtTable[v]))
#define     GET_FTLCxt()                (&(pstFTLMeta->stFTLCxt))
#define     GET_Vpn(Vbn, POffset)       ((UInt32)(Vbn) * (UInt32)PAGES_PER_SUBLK + (UInt32)(POffset))
#define     GET_PageOffset(page)        ((page) % PAGES_PER_SUBLK)
#define     GET_Vbn(page)               ((page) / PAGES_PER_SUBLK)

/*****************************************************************************/
/* Static function prototypes                                                */
/*****************************************************************************/
static void         _GetFTLCxtSpare(UInt32 *pAge, UInt8 *pStatus,
                                    UInt16 * pwLogicalNumber, UInt8 *pSBuf);
static void         _GetLOGCxtSpare(UInt8 *pSBuf, UInt32 * pdwLPn, UInt32 * pdwWriteAge);

static BOOL32       _LoadFTLCxt(void);
#if !(defined(WMR_DISABLE_FTL_RECOVERY) && WMR_DISABLE_FTL_RECOVERY)
static BOOL32       _MakeLogMap(void);
#endif
static LOGCxt       *_ScanLogSection(UInt32 nLbn);
static void         _InitLogCxt(LOGCxt *pLog);

static Int32        _FTLRead(UInt32 dwLpn, UInt32 dwNumOfPages, UInt8 *pBuf);

#ifdef AND_COLLECT_STATISTICS
static BOOL32       _FTLSetStatisticsFromCxt(UInt8 * pabData);
#endif // AND_COLLECT_STATISTICS

#ifndef AND_READONLY
static BOOL32       _AutoWearLevel(void);
static BOOL32       _AddLbnToRefreshList(UInt16 wLbn, UInt16 wVbn);
static BOOL32       _IsBlockInRefreshList(UInt16 wLbn, UInt16 wVbn, UInt32 * pdwIdx);
static BOOL32       _FTLMarkCxtNotValid(void);
#ifdef AND_COLLECT_STATISTICS
static BOOL32       _FTLGetStatisticsToCxt(UInt8 * pabData);
#endif // AND_COLLECT_STATISTICS
static BOOL32       _DoMoveMerge(LOGCxt *pLogVictim);
static BOOL32       _DoCopyMerge(LOGCxt *pLogVictim);
static BOOL32       _DoSimpleMerge(LOGCxt *pLogVictim);
static BOOL32       _Merge(LOGCxt *pLogVictim);
static void         _DisableCopyMerge(LOGCxt *pLog, UInt32 nLPOffset);
static BOOL32       _StoreFTLCxt(void);
static void         _SetFTLCxtSpare(UInt32 *pdwAge, UInt8 *pStatus,
                                    UInt16 * pwLogicalNumber, UInt8 *pabSBuf);
static void         _SetLOGCxtSpare(UInt32 dwLPn, UInt32 dwWriteAge, UInt8 *pSBuf);
static BOOL32       _GetFreeVb(UInt16 *pwVbn);
static BOOL32       _SetFreeVb(UInt16 wVbn);
static BOOL32       _HandleRefreshBlock(void);
#endif

// declaration for the functions FTL_Register publish

static Int32        FTL_Init(VFLFunctions *pVFLFunctions);
static Int32        FTL_Open(UInt32 *pTotalScts, UInt32 * pdwSectorSize, BOOL32 nandFullRestore, BOOL32 justFormatted, UInt32 dwMinorVer, UInt32 dwOptions);
static Int32        FTL_Read(UInt32 nLpn, UInt32 nNumOfScts, UInt8 *pBuf);
static void         FTL_Close(void);
static BOOL32       FTL_GetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 * pdwStructSize);

#ifndef AND_READONLY
static Int32        FTL_Write(UInt32 nLpn, UInt32 nNumOfScts, UInt8 *pBuf, BOOL32 isStatic);
static Int32        FTL_Format(UInt32 dwOptions);
static Int32        FTL_WearLevel(void);
static BOOL32       FTL_GarbageCollect(void);
static BOOL32       FTL_ShutdownNotify(BOOL32 boolMergeLogs);
#endif

/*****************************************************************************/
/* Code Implementation                                                       */
/*****************************************************************************/

/*

    NAME
        _EraseAndMarkEC
    DESCRIPTION
        This function updates the erase counters and call the VFL_Erase function
    PARAMETERS
        wVbn     [IN] 		block number
    RETURN VALUES
        VFL_Erase result
    NOTES

 */

#ifndef AND_READONLY
static Int32
_EraseAndMarkEC(UInt16 wVbn)
{
    pstFTLCxt->pawECCacheTable[wVbn]++;
    pstFTLCxt->pawRCCacheTable[wVbn] = 0;
    return VFL_Erase(wVbn, TRUE32);
}

/*

    NAME
        _SetFTLCxtSpare
    DESCRIPTION
        This function set context age & status mark to FTL context spare.
    PARAMETERS
        pAge     [IN] 		the pointer of context age
        pStatus  [IN] 		the pointer of status mark
 *pSBuf 	 [IN/OUT] 	the pointer of spare area buffer
    RETURN VALUES
        none
    NOTES

 */
static void
_SetFTLCxtSpare(UInt32 *pdwAge, UInt8 *pStatus, UInt16 * pwLogicalNumber, UInt8 *pabSBuf)
{
    FTLCxtSpare *pSpare = (FTLCxtSpare *)pabSBuf;

    WMR_MEMSET(pabSBuf, 0xFF, BYTES_PER_METADATA_RAW);
    if (pdwAge != NULL)
    {
        pSpare->dwCxtAge = *pdwAge;
    }
    if (pStatus != NULL)
    {
        pSpare->bStatusMark = *pStatus;
    }
    if (pwLogicalNumber != NULL)
    {
        pSpare->wLogicalNumber = *pwLogicalNumber;
    }
}
#endif // AND_READONLY

/*

    NAME
        _GetFTLCxtSpare
    DESCRIPTION
        This function returns context age & status mark
        from FTL context spare.
    PARAMETERS
        pAge     [OUT] 		the pointer of context age
        pStatus  [OUT] 		the pointer of status mark
 *pSBuf 	 [IN] 		the pointer of spare area buffer
    RETURN VALUES
        none
    NOTES

 */
static void
_GetFTLCxtSpare(UInt32 *pdwAge, UInt8 *pStatus, UInt16 * pwLogicalNumber, UInt8 *pSBuf)
{
    FTLCxtSpare *pSpare = (FTLCxtSpare *)pSBuf;

    if (pdwAge != NULL)
    {
        *pdwAge = pSpare->dwCxtAge;
    }
    if (pwLogicalNumber != NULL)
    {
        *pwLogicalNumber = pSpare->wLogicalNumber;
    }
    if (pStatus != NULL)
    {
        *pStatus = pSpare->bStatusMark;
    }
}

/*

    NAME
    _GetLOGCxtSpare
    DESCRIPTION
    This function returns logical page offset from log spare
    PARAMETERS
 *pSBuf 	 [IN] 		the pointer of spare area buffer
    RETURN VALUES
    NOTES

 */
static void
_GetLOGCxtSpare(UInt8 *pabSBuf, UInt32 * pdwLpn, UInt32 * pdwWriteAge)
{
    LOGSpare *pSpare;

    pSpare = (LOGSpare *)pabSBuf;
    if (pdwLpn != NULL)
    {
        *pdwLpn = pSpare->dwLPOffset;
    }
    if (pdwWriteAge != NULL)
    {
        *pdwWriteAge = pSpare->dwWriteAge;
    }
}

/*
    NAME
    _SetLOGCxtSpare
    DESCRIPTION
    This function set logical page offset on spare area.
    PARAMETERS
    pdwLpn   	[IN]				logical page number
    dwWriteAge	[IN]				counter (allow identifing the latest version of a logical page
                        case we have a reset / power failure) - see (FTLCxt type).
 *pSBuf		[IN/OUT]			the pointer of spare area buffer
    RETURN VALUES
    none
    NOTES

 */
#ifndef AND_READONLY
static void
_SetLOGCxtSpare(UInt32 pdwLpn, UInt32 dwWriteAge, UInt8 *pabSBuf)
{
    LOGSpare *pSpare;

    WMR_MEMSET(pabSBuf, 0xFF, BYTES_PER_METADATA_RAW);
    pSpare = (LOGSpare *)pabSBuf;
    pSpare->dwLPOffset = pdwLpn;
    pSpare->dwWriteAge = dwWriteAge;
    pSpare->bSpareType = FTL_SPARE_TYPE_LOG;
}
/*
    NAME
    _PrepareNextMapCxtPage
    DESCRIPTION
    This function updates the pstFTLCxt->dwCurrMapCxtPagevalue to enable the next write of Cxt information to the flash.
    PARAMETERS
    RETURN VALUES
    FLASE32	- for failure.
    TRUE32	- for success.
    NOTES
 */
static BOOL32
_PrepareNextMapCxtPage(void)
{
    FTL_LOG_PRINT((TEXT("[FTL: IN] ++_PrepareNextMapCxtPage()\n")));

    /* check if context block is full						 */
    if ((pstFTLCxt->dwCurrMapCxtPage + 1) % PAGES_PER_SUBLK == 0)
    {
        /* allocate idle context block				 */
        UInt16 wCurrCxtIdx, wReplacedCxtVbn;
        UInt16 wFreeVb;

        /* check what is the current Cxt index */
        for (wCurrCxtIdx = 0; wCurrCxtIdx < (UInt16)FTL_CXT_SECTION_SIZE; wCurrCxtIdx++)
        {
            if (pstFTLCxt->dwCurrMapCxtPage > GET_Vpn(pstFTLCxt->awMapCxtVbn[wCurrCxtIdx], 0) &&
                pstFTLCxt->dwCurrMapCxtPage < GET_Vpn((pstFTLCxt->awMapCxtVbn[wCurrCxtIdx] + 1), 0))
            {
                break;
            }
        }
        /* we want to throw out the oldest block the one opposite to the index (i.e. 0 -> 1, 1 -> 2, 2 -> 0) and replace it with the idle one */
        wCurrCxtIdx = ((wCurrCxtIdx + 1) % FTL_CXT_SECTION_SIZE);
        if (pstFTLCxt->dwMetaWearLevelCounter % (FTL_CXT_SECTION_SIZE * FTL_METAWEARLEVEL_RATIO) < FTL_CXT_SECTION_SIZE)
        {
            pstFTLCxt->dwMetaWearLevelCounter++;
            wReplacedCxtVbn = pstFTLCxt->awMapCxtVbn[wCurrCxtIdx];
            if (_GetFreeVb(&wFreeVb) == FALSE32)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR] _PrepareNextMapCxtPage failed _GetFreeVb(0x%X) (line:%d)\n"), wFreeVb, __LINE__));
                return FALSE32;
            }
            pstFTLCxt->awMapCxtVbn[wCurrCxtIdx] = wFreeVb;
            pstFTLCxt->dwCurrMapCxtPage = GET_Vpn(wFreeVb, 0);
            if (_SetFreeVb(wReplacedCxtVbn) == FALSE32)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR] _PrepareNextMapCxtPage failed _SetFreeVb(0x%X) (line:%d)\n"), wReplacedCxtVbn, __LINE__));
                return FALSE32;
            }
            if (VFL_ChangeFTLCxtVbn(pstFTLCxt->awMapCxtVbn) != VFL_SUCCESS)
            {
                return FALSE32;
            }
        }
        else
        {
            pstFTLCxt->dwMetaWearLevelCounter++;
            if (_EraseAndMarkEC(pstFTLCxt->awMapCxtVbn[wCurrCxtIdx]) != VFL_SUCCESS)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR] _PrepareNextMapCxtPage failed _EraseAndMarkEC(0x%X) (line:%d)\n"), pstFTLCxt->awMapCxtVbn[wCurrCxtIdx], __LINE__));
                return FALSE32;
            }
            pstFTLCxt->dwCurrMapCxtPage = GET_Vpn(pstFTLCxt->awMapCxtVbn[wCurrCxtIdx], 0);
        }
    }
    else
    {
        pstFTLCxt->dwCurrMapCxtPage++;
    }

    pstFTLCxt->dwAge--;

    FTL_LOG_PRINT((TEXT("[FTL:OUT] --_PrepareNextMapCxtPage()\n")));

    return TRUE32;
}

/*
    NAME
    _StoreFTLCxt
    DESCRIPTION
    This function writes the context & the map data on FTL info area.
    PARAMETERS
    RETURN VALUES
    TRUE32
        _StoreFTLCxt is completed
    FALSE32
        _StoreFTLCxt is failed
    NOTES

 */
static BOOL32
_StoreFTLCxt(void)
{
    BOOL32 bRet = TRUE32;
    const UInt8 baStoreCxt[] =
    {
        FTL_SPARE_TYPE_CXT_EC_TABLE,
        FTL_SPARE_TYPE_CXT_RC_TABLE,
        FTL_SPARE_TYPE_CXT_MAP_TABLE,
        FTL_SPARE_TYPE_CXT_LOGCXT_MAP,
        FTL_SPARE_TYPE_CXT_STAT,
        FTL_SPARE_TYPE_CXT_INDEX,
    };
    UInt32 dwCxtIdx = 0;
    UInt16 dwPageIdx = 0;
    UInt32 dwSize = 0;
    UInt8 *pbaSrc = NULL;
    UInt32 *padwPtrs = NULL;
    UInt32 dwBufIdx = 0;
    UInt32 dwWriteIdx = 0;
    UInt8 *pbaData = pstSimpleMergeDataBuffer;
    UInt8 *pbaSpare = pstbaMultiplePagesWriteSpare;

    /* Initialize FTL Meta */
    pstFTLMeta->dwVersion = FTL_VERSION;
    pstFTLMeta->dwVersionNot = FTL_VERSION_NOT;

    /* If entire FTL Context won't fit in the current block, get the next one */
    if (GET_Vbn(pstFTLCxt->dwCurrMapCxtPage + 1) !=
        GET_Vbn(pstFTLCxt->dwCurrMapCxtPage + FTL_NUM_PAGES_TO_WRITE_IN_STORECXT + 1))
    {
        FTL_CHANGE_FTLCXT_TO_NEXT_BLOCK();
    }

    dwWriteIdx = GET_PageOffset(pstFTLCxt->dwCurrMapCxtPage + 1);

    /* Loop while we have context to write or a failure occurs */
    while ( (bRet) && (NUMELEMENTS(baStoreCxt) > dwCxtIdx) )
    {
        /* If we don't have content to write, get more */
        if ( (0 == dwSize) || (NULL == pbaSrc) )
        {
            switch ( baStoreCxt[dwCxtIdx] )
            {
            case FTL_SPARE_TYPE_CXT_EC_TABLE:
                dwSize = FTL_AREA_SIZE * sizeof(UInt16);
                pbaSrc = (UInt8 *)pstFTLCxt->pawECCacheTable;
                padwPtrs = pstFTLCxt->adwECTablePtrs;
                break;

            case FTL_SPARE_TYPE_CXT_RC_TABLE:
                dwSize = FTL_AREA_SIZE * sizeof(UInt16);
                pbaSrc = (UInt8 *)pstFTLCxt->pawRCCacheTable;
                padwPtrs = pstFTLCxt->adwRCTablePtrs;
                break;

            case FTL_SPARE_TYPE_CXT_MAP_TABLE:
                dwSize = USER_SUBLKS_TOTAL * sizeof(UInt16);
                pbaSrc = (UInt8 *)pstFTLCxt->pawMapTable;
                padwPtrs = pstFTLCxt->adwMapTablePtrs;
                break;

            case FTL_SPARE_TYPE_CXT_LOGCXT_MAP:
                dwSize = (LOG_SECTION_SIZE + 1) * PAGES_PER_SUBLK * sizeof(UInt16);
                pbaSrc = (UInt8 *)pstFTLCxt->pawLOGCxtMapTable;
                padwPtrs = pstFTLCxt->adwLOGCxtMapPtrs;
                break;

            case FTL_SPARE_TYPE_CXT_STAT:
                pstFTLCxt->adwStatPtrs[0] = FTL_TABLE_PTR_INVALID_VALUE;
                pstFTLCxt->adwStatPtrs[1] = FTL_TABLE_PTR_INVALID_VALUE;
                if (0 == dwSize)
                {
                    dwSize = BYTES_PER_PAGE;
                }
#ifdef AND_COLLECT_STATISTICS
                _FTLGetStatisticsToCxt(&pbaData[dwBufIdx * BYTES_PER_PAGE]);
#else //AND_COLLECT_STATISTICS
                WMR_MEMSET(&pbaData[dwBufIdx * BYTES_PER_PAGE], 0x00, BYTES_PER_PAGE);
#endif //AND_COLLECT_STATISTICS
                padwPtrs = pstFTLCxt->adwStatPtrs;
                break;

            case FTL_SPARE_TYPE_CXT_INDEX:
                pstFTLCxt->boolFlashCxtIsValid = TRUE32;
                dwSize = sizeof(FTLMeta);
                pbaSrc = (UInt8 *)pstFTLMeta;
                break;

            default:
                WMR_PANIC("%s(): Unknown FTL Spare Context 0x%02X!\n", __FUNCTION__, baStoreCxt[dwCxtIdx]);
                bRet = FALSE32;
                break;
            }
        }

        /* Break if an error occurred */
        if ( !bRet )
        {
            break;
        }

        /* Prepare page for current content */
        if (!_PrepareNextMapCxtPage())
        {
            bRet = FALSE32;
            break;
        }

        /*
         * If a source pointer was provided, copy one byte at a time.
         * Otherwise, assume the data was copied over for us.
         */
        if (NULL != pbaSrc)
        {
            WMR_MEMCPY
            (
                &pbaData[dwBufIdx * BYTES_PER_PAGE],
                &pbaSrc[dwPageIdx * BYTES_PER_PAGE],
                WMR_MIN(dwSize, BYTES_PER_PAGE)
            );
        }

        /* Prepare spare data */
        _SetFTLCxtSpare(&pstFTLCxt->dwAge, NULL, &dwPageIdx, &pbaSpare[dwBufIdx * BYTES_PER_METADATA_RAW]);
        SET_FTL_SPARE_TYPE(&pbaSpare[dwBufIdx * BYTES_PER_METADATA_RAW], baStoreCxt[dwCxtIdx]);

        /* Update pointers with content's VPN if necessary */
        if (NULL != padwPtrs)
        {
            padwPtrs[dwPageIdx] = pstFTLCxt->dwCurrMapCxtPage;
        }

        dwBufIdx++;

        /* If content size is greater than a page, update size and index for another loop. */
        if ( dwSize > BYTES_PER_PAGE )
        {
            dwSize -= BYTES_PER_PAGE;
            dwPageIdx++;
        }
        /* Otherwise, prepare for next set of data */
        else
        {
            dwCxtIdx++;
            dwSize = 0;
            pbaSrc = NULL;
            padwPtrs = NULL;
            dwPageIdx = 0;
        }

        /* Make sure no one caused a buffer overflow */
        WMR_ASSERT ( ((UInt32)PAGES_PER_SIMPLE_MERGE_BUFFER >= dwBufIdx ) ||
                    (NUMELEMENTS(baStoreCxt) >= dwCxtIdx));

        /*
         * If the buffer is full or we've exhausted our FTL Context, perform a write */
        if (((UInt32)PAGES_PER_SIMPLE_MERGE_BUFFER == dwBufIdx ) ||
            (NUMELEMENTS(baStoreCxt) == dwCxtIdx))
        {
            BOOL32 bWriteStatus = TRUE32;

            bWriteStatus = VFL_WriteMultiplePagesInVb(
                GET_Vpn(GET_Vbn(pstFTLCxt->dwCurrMapCxtPage), dwWriteIdx),
                dwBufIdx,
                pbaData,
                pbaSpare,
                TRUE32,
                FALSE32);

            if ( !bWriteStatus )
            {
                FTL_WRN_PRINT((TEXT("[FTL:WRN] %s(): VFL_WriteMultiplePagesInVb() failed!\n"), __FUNCTION__));
                bRet = FALSE32;
                break;
            }
            else
            {
                /* Update our write index and mark the buffer empty */
                dwWriteIdx += dwBufIdx;
                dwBufIdx = 0;
            }
        }
    }

    return bRet;
}
#endif

/*

    NAME
    _LoadFTLCxt
    DESCRIPTION
    This function loads the minimum aged ftl context.
    PARAMETERS
    none
    RETURN VALUES
    TRUE32
        _LoadFTLCxt is completed
    FALSE32
        _LoadFTLCxt is failed
    NOTES

 */
static BOOL32
_LoadFTLCxt(void)
{
    UInt16 wCxtPage, wLoadIdx;
    UInt16 wCxtVbn, wVbnIdx;
    UInt32 dwMinAge = 0xFFFFFFFF;
    Buffer *pBuf = NULL;
    Int32 nVFLRet;
    BOOL32 boolFoundValidIndex;
    UInt16 * pawMapTable = pstFTLCxt->pawMapTable;
#ifndef AND_READONLY
    UInt16 * pawECCacheTable = pstFTLCxt->pawECCacheTable;
    UInt16 * pawRCCacheTable = pstFTLCxt->pawRCCacheTable;
#endif /* #ifndef AND_READONLY */
    UInt16 * pawLOGCxtMapTable = pstFTLCxt->pawLOGCxtMapTable;
    UInt16 * pawCxtMap;

    FTL_LOG_PRINT((TEXT("[FTL: IN] ++_LoadFTLCxt()\n")));

    wCxtVbn = 0xFFFF;

    pawCxtMap = VFL_GetFTLCxtVbn();
    if (pawCxtMap == NULL)
    {
        return FALSE32; /* that does not suppose to happen */
    }
    else
    {
        WMR_MEMCPY(pstFTLCxt->awMapCxtVbn, pawCxtMap, (FTL_CXT_SECTION_SIZE * sizeof(UInt16)));
    }

    pBuf = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBuf == NULL)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] BUF_Get error (line:%d)!\n"), __LINE__));
        return FALSE32;
    }

    /* search the minimum aged context virtual block & page offset */
    for (wVbnIdx = 0; wVbnIdx < FTL_CXT_SECTION_SIZE; wVbnIdx++)
    {
        UInt32 dwAge;

        nVFLRet = VFL_Read(GET_Vpn(pstFTLCxt->awMapCxtVbn[wVbnIdx], 0), pBuf, TRUE32, FALSE32, NULL, NULL, FALSE32);

        /* ignore uncorrectable ECC error		*/
        if (nVFLRet == VFL_CRITICAL_ERROR)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR]	there is error on VFL_Read(line:%d)!\n"), __LINE__));
            BUF_Release(pBuf);
            return FALSE32;
        }

        _GetFTLCxtSpare(&dwAge, NULL, NULL, pBuf->pSpare);
        if (FTL_SPARE_IS_CXT_MARK(GET_FTL_SPARE_TYPE(pBuf->pSpare)) && nVFLRet == VFL_SUCCESS)
        {
            if (wCxtVbn == 0xFFFF || dwAge < dwMinAge)
            {
                dwMinAge = dwAge;
                wCxtVbn = pstFTLCxt->awMapCxtVbn[wVbnIdx];
            }
        }
    }

    BUF_Release(pBuf);

    /* if can not find context			*/
    if (wCxtVbn == 0xFFFF)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] Can not find context!\n")));
        return FALSE32;
    }

    pBuf = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBuf == NULL)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] BUF_Get error (line:%d)!\n"), __LINE__));
        return FALSE32;
    }
    /* find if there is a relavant index cxt page */
    boolFoundValidIndex = FALSE32;

#if FTL_FIND_IDX_PG_BY_BIN_SRCH
    {
        Int16 startPage = 0, endPage = PAGES_PER_SUBLK-1, midPage;
        Int32 nVFLRet;

        while (endPage >= startPage)
        {
            midPage = DIV2_ROUNDUP(endPage + startPage);
            wCxtPage = midPage;
            nVFLRet = VFL_Read(GET_Vpn(wCxtVbn, wCxtPage), pBuf, TRUE32, FALSE32, NULL, NULL, FALSE32);


            if (nVFLRet == VFL_SUCCESS_CLEAN)
            {
                endPage = midPage - 1;
            }
            else if (nVFLRet == VFL_SUCCESS)
            {
                if (startPage == endPage)
                {
                    /* check that this is an index */
                    if (GET_FTL_SPARE_TYPE(pBuf->pSpare) == FTL_SPARE_TYPE_CXT_INDEX)
                    {
                        boolFoundValidIndex = TRUE32;
                        WMR_MEMCPY(pstFTLMeta, pBuf->pData, sizeof(FTLMeta));
                    }
                    break;
                }
                else
                {
                    startPage = midPage;
                }
            }
            else
            {
                /* the page is written but not probably assume we have no valid index (i.e. power failure)*/
                break;
            }
        }
    }
#else
    for (wCxtPage = (UInt16)PAGES_PER_SUBLK; wCxtPage != 0;)
    {
        wCxtPage--;
        nVFLRet = VFL_Read(GET_Vpn(wCxtVbn, wCxtPage), pBuf, TRUE32, FALSE32, NULL, NULL, FALSE32);
        if (nVFLRet == VFL_SUCCESS)
        {
            /* check that this is an index */
            if (GET_FTL_SPARE_TYPE(pBuf->pSpare) == FTL_SPARE_TYPE_CXT_INDEX)
            {
                boolFoundValidIndex = TRUE32;
                WMR_MEMCPY(pstFTLMeta, pBuf->pData, sizeof(FTLMeta));
            }
            break;
        }
        else if (nVFLRet != VFL_SUCCESS_CLEAN)
        {
            /* the page is written but not probably assume we have no valid index (i.e. power failure)*/
            break;
        }
    }
#endif

    /* change the pointers to what they were */
    pstFTLCxt->pawMapTable = pawMapTable;
#ifndef AND_READONLY
    pstFTLCxt->pawECCacheTable = pawECCacheTable;
    pstFTLCxt->pawRCCacheTable = pawRCCacheTable;
#endif /* #ifndef AND_READONLY */
    pstFTLCxt->pawLOGCxtMapTable = pawLOGCxtMapTable;
    for (wLoadIdx = 0; wLoadIdx < (LOG_SECTION_SIZE + 1); wLoadIdx++)
    {
        pstFTLCxt->aLOGCxtTable[wLoadIdx].paPOffsetL2P = &pstFTLCxt->pawLOGCxtMapTable[PAGES_PER_SUBLK * wLoadIdx];
    }

    /* load the data using the pointers in the index */
    /* load maps */
    for (wLoadIdx = 0; (boolFoundValidIndex == TRUE32) &&
         (wLoadIdx < FTL_NUM_PAGES_PER_MAP_TABLE);
         wLoadIdx++)
    {
        nVFLRet = VFL_Read(pstFTLCxt->adwMapTablePtrs[wLoadIdx], pBuf, TRUE32, TRUE32, NULL, NULL, FALSE32);
        if (nVFLRet != VFL_SUCCESS)
        {
            boolFoundValidIndex = FALSE32;
            break;
        }
        WMR_MEMCPY(&(((UInt8 *)pstFTLCxt->pawMapTable)[wLoadIdx * BYTES_PER_PAGE]), pBuf->pData, WMR_MIN(((USER_SUBLKS_TOTAL * sizeof(UInt16)) - (wLoadIdx * BYTES_PER_PAGE)), BYTES_PER_PAGE));
    }

    /* load logcxt maps here */
    for (wLoadIdx = 0; (boolFoundValidIndex == TRUE32) &&
         (wLoadIdx < FTL_NUM_PAGES_PER_LOGCXT_MAP_TABLE);
         wLoadIdx++)
    {
        nVFLRet = VFL_Read(pstFTLCxt->adwLOGCxtMapPtrs[wLoadIdx], pBuf, TRUE32, TRUE32, NULL, NULL, FALSE32);
        if (nVFLRet != VFL_SUCCESS)
        {
            boolFoundValidIndex = FALSE32;
            break;
        }
        WMR_MEMCPY(&(((UInt8 *)pstFTLCxt->pawLOGCxtMapTable)[wLoadIdx * BYTES_PER_PAGE]), pBuf->pData, WMR_MIN(((LOG_SECTION_SIZE * PAGES_PER_SUBLK * sizeof(UInt16)) - (wLoadIdx * BYTES_PER_PAGE)), BYTES_PER_PAGE));
    }

#ifndef AND_READONLY
    /* load EC Table */
    for (wLoadIdx = 0; (boolFoundValidIndex == TRUE32) &&
         (wLoadIdx < FTL_NUM_PAGES_PER_EC_TABLE);
         wLoadIdx++)
    {
        nVFLRet = VFL_Read(pstFTLCxt->adwECTablePtrs[wLoadIdx], pBuf, TRUE32, TRUE32, NULL, NULL, FALSE32);
        if (nVFLRet != VFL_SUCCESS)
        {
            boolFoundValidIndex = FALSE32;
            break;
        }
        WMR_MEMCPY(&(((UInt8 *)pstFTLCxt->pawECCacheTable)[wLoadIdx * BYTES_PER_PAGE]), pBuf->pData, WMR_MIN(((FTL_AREA_SIZE * sizeof(UInt16)) - (wLoadIdx * BYTES_PER_PAGE)), BYTES_PER_PAGE));
    }
#endif /* #ifndef AND_READONLY */

#ifndef AND_READONLY
    /* load RC Table */
    for (wLoadIdx = 0; (boolFoundValidIndex == TRUE32) &&
            (wLoadIdx < FTL_NUM_PAGES_PER_RC_TABLE);
            wLoadIdx++)
    {
        nVFLRet = VFL_Read(pstFTLCxt->adwRCTablePtrs[wLoadIdx], pBuf, TRUE32, TRUE32, NULL, NULL, FALSE32);
        if (nVFLRet != VFL_SUCCESS)
        {
            boolFoundValidIndex = FALSE32;
            break;
        }
        WMR_MEMCPY(&(((UInt8 *)pstFTLCxt->pawRCCacheTable)[wLoadIdx * BYTES_PER_PAGE]), pBuf->pData, WMR_MIN(((FTL_AREA_SIZE * sizeof(UInt16)) - (wLoadIdx * BYTES_PER_PAGE)), BYTES_PER_PAGE));
    }
#endif /* #ifndef AND_READONLY */

#ifdef AND_COLLECT_STATISTICS
    if (boolFoundValidIndex == TRUE32)
    {
        // read the stats
        if ((pstFTLCxt->adwStatPtrs[1] == FTL_TABLE_PTR_INVALID_VALUE) &&
            (pstFTLCxt->adwStatPtrs[0] / PAGES_PER_SUBLK < VFL_GetDeviceInfo(AND_DEVINFO_NUM_OF_USER_SUBLK)))
        {
            nVFLRet = VFL_Read(pstFTLCxt->adwStatPtrs[0], pBuf, TRUE32, TRUE32, NULL, NULL, FALSE32);
            if (nVFLRet != VFL_SUCCESS)
            {
                boolFoundValidIndex = FALSE32;
            }
            else
            {
                _FTLSetStatisticsFromCxt(pBuf->pData);
            }
        }
    }
#endif // AND_COLLECT_STATISTICS

    BUF_Release(pBuf);

    FTL_LOG_PRINT((TEXT("[FTL:OUT] --_LoadFTLCxt()\n")));
    return boolFoundValidIndex;
}

#if !(defined(WMR_DISABLE_FTL_RECOVERY) && WMR_DISABLE_FTL_RECOVERY)
/*

    NAME
    _RecoverLOGCxtInfo
    DESCRIPTION
    This function recovers the log mapping based on the info found in the block.
    This function should only be used as part as _FTLRestore.
    PARAMETERS
    none
    RETURN VALUES
    TRUE32
        success
    FALSE32
        failure (should only occour if allocating a buffer failed).
    NOTES

 */

BOOL32 _RecoverLOGCxtInfo(LOGCxt * pLog)
{
    UInt16 wVpnIdx;
    Buffer *pBuf = NULL;

    pBuf = BUF_Get(BUF_MAIN_AND_SPARE);

    if (pBuf == NULL)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] BUF_Get error (line:%d)!\n"), __LINE__));
        return FALSE32;
    }

    for (wVpnIdx = 0; wVpnIdx < PAGES_PER_SUBLK; wVpnIdx++)
    {
        UInt32 dwVpn;
        Int32 nVFLRet;

        dwVpn = GET_Vpn(pLog->wVbn, wVpnIdx);

        nVFLRet = VFL_Read(dwVpn, pBuf, TRUE32, FALSE32, NULL, NULL, FALSE32);

        if (nVFLRet == VFL_SUCCESS)
        {
            UInt32 dwLpn;
            _GetLOGCxtSpare(pBuf->pSpare, &dwLpn, NULL);
            if (pLog->paPOffsetL2P[(dwLpn % PAGES_PER_SUBLK)] == 0xFFFF)
            {
                pLog->wNumOfValidLP++;
            }
            pLog->paPOffsetL2P[(dwLpn % PAGES_PER_SUBLK)] = wVpnIdx;
        }
    }
    BUF_Release(pBuf);
    return TRUE32;
}
#endif // ! WMR_DISABLE_FTL_RECOVERY

/*

    NAME
    _MakeLogMap
    DESCRIPTION
    This function makes log virtual block context from scanning log blocks.
    PARAMETERS
    none
    RETURN VALUES
    TRUE32
        _MakeLogMap is completed
    FALSE32
        _MakeLogMap is failed
    NOTES

 */
#if !(defined(WMR_DISABLE_FTL_RECOVERY) && WMR_DISABLE_FTL_RECOVERY)
static BOOL32
_MakeLogMap(void)
{
    LOGCxt *pLog;
    UInt16 wIdx;

    FTL_LOG_PRINT((TEXT("[FTL: IN] ++_MakeLogMap()\n")));

    for (wIdx = 0; wIdx < LOG_SECTION_SIZE; wIdx++)
    {
        pLog = GET_LOGCxt(wIdx);

        _InitLogCxt(pLog);

        if (pLog->wVbn == LOG_EMPTY_SLOT)
        {
            continue;
        }

        if (_RecoverLOGCxtInfo(pLog) == FALSE32)
        {
            return FALSE32;
        }

        /* avoid suprises - since we are not sure what got us to restore we can not assume the rest of the log is usable */
        pLog->boolCopyMerge = FALSE32;
        pLog->wFreePOffset = PAGES_PER_SUBLK;
    }

    FTL_LOG_PRINT((TEXT("[FTL:OUT] --_MakeLogMap()\n")));
    return TRUE32;
}
#endif //#if (defined(WMR_ENABLE_FTL_RECOVERY) && WMR_ENABLE_FTL_RECOVERY)

#ifndef AND_READONLY
/*

    NAME
    _DisableCopyMerge
    DESCRIPTION
    This function chooses whether the log can be copymerged or not.
    PARAMETERS
    pLog    	[IN] 	the pointer of log block context
    nLPOffset  	[IN] 	logical page offset
    RETURN VALUES
    none
    NOTES

 */
static void
_DisableCopyMerge(LOGCxt *pLog, UInt32 nLPOffset)
{
    UInt32 nPPOffset;

    nPPOffset = pLog->paPOffsetL2P[nLPOffset];

    if (pLog->wFreePOffset != pLog->wNumOfValidLP ||
        nPPOffset != nLPOffset)
    {
        /* miss align case */
        pLog->boolCopyMerge = FALSE32;
    }
}
#endif // ! AND_READONLY

/*

    NAME
    _ScanLogSection
    DESCRIPTION
    This function returns log block context from the logical block number.
    (super block address)
    PARAMETERS
    nLbn    	[IN] 	logical block number (super block address)
    RETURN VALUES
    LOGCxt*
        the pointer of log block context
    NULL
        there is no log to match the lbn.
    NOTES

 */
static LOGCxt *
_ScanLogSection(UInt32 nLbn)
{
    UInt32 nIdx;

    for (nIdx = 0; nIdx < LOG_SECTION_SIZE; nIdx++)
    {
        if (pstFTLCxt->aLOGCxtTable[nIdx].wVbn != LOG_EMPTY_SLOT && pstFTLCxt->aLOGCxtTable[nIdx].wLbn == nLbn)
        {
            return &(pstFTLCxt->aLOGCxtTable[nIdx]);
        }
    }

    return NULL;
}
#ifndef AND_READONLY
/*

    NAME
    _GetFreeVb
    DESCRIPTION
    This function returns virtual block number of free block.
    PARAMETERS
    pVbn    	[OUT] 	the pointer of virtual block number
    pEC         [OUT]   the erase count of free block
    RETURN VALUES
    TRUE32
        _GetFreeVb is completed
    FALSE32
        _GetFreeVb is failed
    NOTES

 */
static BOOL32
_GetFreeVb(UInt16 *pwVbn)
{
    UInt16 wIdx;

    UInt16 wFreeVbListTail = pstFTLCxt->wFreeVbListTail;
    UInt16 wRetVbn;
    UInt32 wMinEC = 0xFFFF;
    UInt16 wFreeIdx = FREE_SECTION_SIZE;
    UInt16 wTemp;
    static UInt16 wLastAllocatedFreeVb = 0xFFFF;

    FTL_LOG_PRINT((TEXT("[FTL: IN] ++_GetFreeVb()\n")));

    /* choose minimum erased free block */
    for (wIdx = 0; wIdx < pstFTLCxt->wNumOfFreeVb; wIdx++)
    {
        UInt16 wIdxTemp = wIdx + wFreeVbListTail;

        if (wIdxTemp >= FREE_SECTION_SIZE)
        {
            wIdxTemp -= FREE_SECTION_SIZE;
        }

        /* avoid giving the last allocated vb two times in a row */
        if (wLastAllocatedFreeVb != pstFTLCxt->awFreeVbList[wIdxTemp])
        {
            if (wMinEC > pstFTLCxt->pawECCacheTable[pstFTLCxt->awFreeVbList[wIdxTemp]])
            {
                wMinEC = pstFTLCxt->pawECCacheTable[pstFTLCxt->awFreeVbList[wIdxTemp]];
                wFreeIdx = wIdxTemp;
            }
        }
    }
    // WMR_ASSERT(wFreeIdx < FREE_SECTION_SIZE);
    if (wFreeIdx >= FREE_SECTION_SIZE)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR]  _GetFreeVb failed wFreeIdx = 0x%X (line:%d)!\n"), wFreeIdx, __LINE__));
        return FALSE32;
    }
    /* if the chosen block (nFreeIdx) is not the one in the wFreeVbListTail location of the aFreeVbList
        swap the pointers and erase counters (EC) between the chosen and the wFreeVbListTail */
    if (wFreeIdx != wFreeVbListTail)
    {
        wTemp = pstFTLCxt->awFreeVbList[wFreeVbListTail];
        pstFTLCxt->awFreeVbList[wFreeVbListTail] = pstFTLCxt->awFreeVbList[wFreeIdx];
        pstFTLCxt->awFreeVbList[wFreeIdx] = wTemp;
    }

    wRetVbn = pstFTLCxt->awFreeVbList[wFreeVbListTail];
    wLastAllocatedFreeVb = wRetVbn;

    // WMR_ASSERT(wRetVbn < FTL_AREA_SIZE);
    if (wRetVbn >= FTL_AREA_SIZE)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR]  _GetFreeVb failed wRetVbn = 0x%X (line:%d)!\n"), wRetVbn, __LINE__));
        return FALSE32;
    }

    /* move free vb list's pointer */
    if ((wFreeVbListTail + 1) == FREE_SECTION_SIZE)
    {
        pstFTLCxt->wFreeVbListTail = 0;
    }
    else
    {
        pstFTLCxt->wFreeVbListTail = wFreeVbListTail + 1;
    }

    pstFTLCxt->wNumOfFreeVb--;

    //WMR_ASSERT(wFreeVbListTail < FREE_SECTION_SIZE);
    //WMR_ASSERT(pstFTLCxt->wNumOfFreeVb < FREE_SECTION_SIZE);
    if ((wFreeVbListTail >= FREE_SECTION_SIZE) || (pstFTLCxt->wNumOfFreeVb >= FREE_SECTION_SIZE))
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR]  _GetFreeVb failed wFreeVbListTail = 0x%X, pstFTLCxt->wNumOfFreeVb = 0x%X (line:%d)!\n"), wFreeVbListTail, pstFTLCxt->wNumOfFreeVb, __LINE__));
        return FALSE32;
    }

    *pwVbn = wRetVbn;

    FTL_LOG_PRINT((TEXT("[FTL:OUT] --_GetFreeVb()\n")));

    return TRUE32;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _SetFreeVb		                                                     */
/* DESCRIPTION                                                               */
/*      This function adds virtual block to the free vb list.				 */
/* PARAMETERS                                                                */
/*      wVbn    	[IN] 	virtual block number					         */
/* RETURN VALUES                                                             */
/*		none																 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static BOOL32
_SetFreeVb(UInt16 wVbn)
{
    UInt32 nFreeVbListHead;

    WMR_ASSERT(wVbn < FTL_AREA_SIZE);

    nFreeVbListHead = pstFTLCxt->wFreeVbListTail + pstFTLCxt->wNumOfFreeVb++;

    if (_EraseAndMarkEC(wVbn) != VFL_SUCCESS)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] _SetFreeVb failed _EraseAndMarkEC(0x%X) (line:%d)\n"), wVbn, __LINE__));
        return FALSE32;
    }

    if (nFreeVbListHead >= FREE_SECTION_SIZE)
    {
        nFreeVbListHead -= FREE_SECTION_SIZE;
    }

    WMR_ASSERT(nFreeVbListHead < FREE_SECTION_SIZE);
    WMR_ASSERT(pstFTLCxt->wNumOfFreeVb <= FREE_SECTION_SIZE);

    pstFTLCxt->awFreeVbList[nFreeVbListHead] = wVbn;

    WMR_ASSERT(pstFTLCxt->wNumOfFreeVb <= FREE_SECTION_SIZE);

    return TRUE32;
}
#endif /* AND_READONLY */

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _InitLogCxt		                                                     */
/* DESCRIPTION                                                               */
/*      This function initializes log context.								 */
/* PARAMETERS                                                                */
/*      pLog    	[IN] 	the pointer of log context				         */
/* RETURN VALUES                                                             */
/*		none																 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static void
_InitLogCxt(LOGCxt *pLog)
{
    WMR_MEMSET(pLog->paPOffsetL2P, 0xFF, (PAGES_PER_SUBLK * sizeof(UInt16)));

    pLog->boolCopyMerge = TRUE32;
    pLog->wFreePOffset = 0;
    pLog->wNumOfValidLP = 0;
}

#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _MoveD2F															 */
/* DESCRIPTION                                                               */
/*      This function copies from data block to free block					 */
/* RETURN VALUES                                                             */
/*		TRUE32																 */
/*				_MoveD2F is completed										 */
/*		FALSE32																 */
/*				_MoveD2F is failed											 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static BOOL32
_MoveD2F(UInt16 wDataLbn, UInt16 wFreeVbn)
{
    Int32 nRet;
    UInt16 wIdx, wLPOffsetIdx;
    BOOL32 boolCopyRes = FALSE32;
    // use a different area of the spare buffer than the one used by _FTLRead that reads PAGES_PER_SIMPLE_MERGE_BUFFER pages
    UInt8   *pabMultiSpareBuffer = &pstbaMultiplePagesWriteSpare[PAGES_PER_SIMPLE_MERGE_BUFFER * BYTES_PER_METADATA_RAW];

    FTL_LOG_PRINT((TEXT("[FTL:OUT] ++_MoveD2F()\n")));

#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwMoveD2FCnt++;
#endif
    /* copy from data block to free block */

    pstFTLCxt->dwWriteAge++;
    for (wIdx = 0; wIdx < PAGES_PER_SUBLK; wIdx += PAGES_PER_SIMPLE_MERGE_BUFFER)
    {
        if (_FTLRead(((((UInt32)wDataLbn) * PAGES_PER_SUBLK) + (UInt32)wIdx), PAGES_PER_SIMPLE_MERGE_BUFFER, pstSimpleMergeDataBuffer) == FTL_SUCCESS)
        {
            for (wLPOffsetIdx = 0; wLPOffsetIdx < PAGES_PER_SIMPLE_MERGE_BUFFER; wLPOffsetIdx++)
            {
                UInt8 * pbaTempSpare = &(pabMultiSpareBuffer[wLPOffsetIdx * BYTES_PER_METADATA_RAW]);
                _SetLOGCxtSpare(GET_Vpn(wDataLbn, (wIdx + wLPOffsetIdx)), pstFTLCxt->dwWriteAge, pbaTempSpare);
            }
        }
        else
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR] failed calling _FTLRead(0x%X, 0x%X, 0x%X) (line:%d)\n"),
                           ((((UInt32)wDataLbn) * PAGES_PER_SUBLK) + (UInt32)wIdx), PAGES_PER_SIMPLE_MERGE_BUFFER, pstSimpleMergeDataBuffer, __LINE__));
            for (wLPOffsetIdx = 0; wLPOffsetIdx < PAGES_PER_SIMPLE_MERGE_BUFFER; wLPOffsetIdx++)
            {
                UInt8 * pbaTempSpare = &(pabMultiSpareBuffer[wLPOffsetIdx * BYTES_PER_METADATA_RAW]);
                _SetLOGCxtSpare(GET_Vpn(wDataLbn, (wIdx + wLPOffsetIdx)), pstFTLCxt->dwWriteAge, pbaTempSpare);
                if (_FTLRead(((((UInt32)wDataLbn) * PAGES_PER_SUBLK) + (UInt32)(wIdx + wLPOffsetIdx)), 1, &pstSimpleMergeDataBuffer[BYTES_PER_PAGE * wLPOffsetIdx]) != FTL_SUCCESS)
                {
                    FTL_ERR_PRINT((TEXT("[FTL:ERR] failed calling _FTLRead(0x%X, 0x%X, 0x%X) (line:%d)\n"),
                                   ((((UInt32)wDataLbn) * PAGES_PER_SUBLK) + (UInt32)(wIdx + wLPOffsetIdx)), 1, &pstSimpleMergeDataBuffer[BYTES_PER_PAGE * wLPOffsetIdx], __LINE__));
                    SET_FTL_ECC_MARK(pbaTempSpare);
                }
            }
        }

        if ((wLPOffsetIdx + wIdx) == PAGES_PER_SUBLK)
        {
            SET_FTL_SPARE_TYPE(&(pabMultiSpareBuffer[(wLPOffsetIdx - 1) * BYTES_PER_METADATA_RAW]), FTL_SPARE_TYPE_DATA);
        }
        boolCopyRes = VFL_WriteMultiplePagesInVb(GET_Vpn(wFreeVbn, wIdx), PAGES_PER_SIMPLE_MERGE_BUFFER, pstSimpleMergeDataBuffer, pabMultiSpareBuffer, TRUE32, FALSE32);
        if (boolCopyRes == FALSE32)
        {
            break;
        }
    }
    if (boolCopyRes == FALSE32)
    {
        /* Erase the failed free block to avoid failure in restore */
        nRet = _EraseAndMarkEC(wFreeVbn);
        if (nRet != VFL_SUCCESS)
        {
            FTL_WRN_PRINT((TEXT("[FTL:WRN] _MoveD2F error running VFL_Erase(0x%X) (line:%d)!\n"), wFreeVbn, __LINE__));
        }
        FTL_WRN_PRINT((TEXT("[FTL:WRN] _MoveD2F failed moving the data block to a new block!\n")));
        return FALSE32;
    }

    FTL_LOG_PRINT((TEXT("[FTL:OUT] --_MoveD2F()\n")));

    return TRUE32;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _AutoWearLevel														 */
/* DESCRIPTION                                                               */
/*      This function operates wear level.									 */
/* RETURN VALUES                                                             */
/*		TRUE32																 */
/*				_AutoWearLevel is completed									 */
/*		FALSE32																 */
/*				_AutoWearLevel is failed									 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static BOOL32
_AutoWearLevel(void)
{
    UInt16 wIdx;

    UInt16 wMinEC = 0xFFFF;       /* minimum erase count */
    UInt16 wMaxEC = 0x0;      /* maximum erase count */

    UInt16 wMinVbn = 0;
    UInt16 wMinLbn = 0;

    UInt16 wFreeIdx = FREE_SECTION_SIZE; /* need to be given a starting value */
    UInt16 wFreeVbn;
    static UInt16 wLastFreeVbn = 0xFFFF;
    Int32 nVFLRet;

    FTL_LOG_PRINT((TEXT("[FTL:OUT] ++_AutoWearLevel()\n")));

    if (_FTLMarkCxtNotValid() == FALSE32)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] _AutoWearLevel failed calling _FTLMarkCxtNotValid\n")));
        return FALSE32;
    }
    /* choose maximum erased free block */
    for (wIdx = 0; wIdx < pstFTLCxt->wNumOfFreeVb; wIdx++)
    {
        UInt16 wIdxTemp = wIdx + pstFTLCxt->wFreeVbListTail;

        if (wIdxTemp >= FREE_SECTION_SIZE)
        {
            wIdxTemp -= FREE_SECTION_SIZE; /* making the aFreeVbEC list cyclic */
        }

        /* verify that we are not getting the same free vb time after time */
        if (wLastFreeVbn != pstFTLCxt->awFreeVbList[wIdxTemp])
        {
            if ((wMaxEC < pstFTLCxt->pawECCacheTable[pstFTLCxt->awFreeVbList[wIdxTemp]]) || (wFreeIdx == FREE_SECTION_SIZE))
            {
                wMaxEC = pstFTLCxt->pawECCacheTable[pstFTLCxt->awFreeVbList[wIdxTemp]];
                wFreeIdx = wIdxTemp;
            }
        }
    }

    WMR_ASSERT(wFreeIdx < FREE_SECTION_SIZE);

    wFreeVbn = pstFTLCxt->awFreeVbList[wFreeIdx];
    wLastFreeVbn = wFreeVbn;
    /* calculate data blocks erase count */
    for (wIdx = 0; wIdx < USER_SUBLKS_TOTAL; wIdx++)
    {
        UInt16 wECTemp;

        /* using vb from the map table we make sure to swap only data blocks */
        wECTemp = pstFTLCxt->pawECCacheTable[CONVERT_L2V(wIdx)];

        if (wMinEC >= wECTemp && _ScanLogSection(wIdx) == NULL)
        {
            wMinEC = wECTemp;
            wMinVbn = CONVERT_L2V(wIdx);
            wMinLbn = wIdx;
        }

        if (wMaxEC <= wECTemp)
        {
            wMaxEC = wECTemp;
        }
    }

    if (wMaxEC == 0 || WMR_WEAR_LEVEL_MAX_DIFF > (wMaxEC - wMinEC) || wMinVbn == wFreeVbn)
    {
        return TRUE32;
    }

    /* erase free block */
    nVFLRet = _EraseAndMarkEC(wFreeVbn);
    if (nVFLRet != VFL_SUCCESS)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on VFL_Erase(0x%X) (line:%d)!\n"), wFreeVbn, __LINE__));
        return FALSE32;
    }

    if (_MoveD2F(wMinLbn, wFreeVbn) == FALSE32)
    {
        FTL_WRN_PRINT((TEXT("[FTL:WRN] _MoveD2F Failed (line:%d)!\n"), __LINE__));
        return FALSE32;
    }
    else
    {
        if (_EraseAndMarkEC(CONVERT_L2V(wMinLbn)) != VFL_SUCCESS)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR] _AutoWearLevel failed _EraseAndMarkEC(0x%X) (line:%d)\n"), wMinLbn, __LINE__));
        }
        pstFTLCxt->awFreeVbList[wFreeIdx] = CONVERT_L2V(wMinLbn);
        MODIFY_LB_MAP(wMinLbn, wFreeVbn);
    }

    FTL_LOG_PRINT((TEXT("[FTL:OUT] --_AutoWearLevel()\n")));

    return TRUE32;
}

/*

    NAME
    FTL_WearLevel
    DESCRIPTION
    This function operates wearlevels a single VB.
    RETURN VALUES
    FTL_SUCCESS
        success
    FTL_CRITICAL_ERROR
        failure
    FTL_OUT_OF_RANGE
        counter is under WMR_WEAR_LEVEL_FREQUENCY (we are done wearleveling).
 */
static Int32
FTL_WearLevel(void)
{
    if (_IsBlockInRefreshList(0xFFFF, 0xFFFF, NULL) == TRUE32)
    {
        UInt8 bTrialIdx;
#ifdef AND_COLLECT_STATISTICS
        stFTLStatistics.ddwWearLevelCnt++;
#endif
        /* only wear level after a write operation to avoid unvalidating the FTLCxt before wearlelveling */
        for (bTrialIdx = 0; bTrialIdx < 4; bTrialIdx++)
        {
            if (_HandleRefreshBlock() == TRUE32)
            {
                return FTL_SUCCESS;
            }
        }
        if (bTrialIdx == 4)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR] FTL_WearLevel failed _HandleRefreshBlock() (line:%d)\n"), __LINE__));
            return FTL_CRITICAL_ERROR;
        }
    }
    else if (pstFTLCxt->wNumOfFreeVb < (FREE_LIST_SIZE + 3))
    {
#ifdef AND_COLLECT_STATISTICS
        stFTLStatistics.ddwWearLevelCnt++;
#endif
        if (_Merge(NULL) == FALSE32)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR] FTL_WearLevel failed _Merge(NULL) (line:%d)\n"), __LINE__));
            return FTL_CRITICAL_ERROR;
        }
        return FTL_SUCCESS;
    }
    else if (pstFTLCxt->boolFlashCxtIsValid == FALSE32 && pstFTLCxt->wWearLevelCounter >= WMR_WEAR_LEVEL_FREQUENCY)
    {
        /* only wear level after a write operation to avoid unvalidating the FTLCxt before wearlelveling */
        UInt8 bTrialIdx;
#ifdef AND_COLLECT_STATISTICS
        stFTLStatistics.ddwWearLevelCnt++;
#endif
        for (bTrialIdx = 0; bTrialIdx < 4; bTrialIdx++)
        {
            if (_AutoWearLevel() == TRUE32)
            {
                pstFTLCxt->wWearLevelCounter -= WMR_WEAR_LEVEL_FREQUENCY;
                return FTL_SUCCESS;
            }
        }
        if (bTrialIdx == 4)
        {
            return FTL_CRITICAL_ERROR;
        }
    }
    return FTL_OUT_OF_RANGE_ERROR;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _PrepareLog		                                                     */
/* DESCRIPTION                                                               */
/*      This function prepares log block context. 							 */
/* PARAMETERS                                                                */
/*      nLbn    	[IN] 	logical block number					         */
/*      pLOGCxt    	[OUT] 	the pointer of log block context		         */
/* RETURN VALUES                                                             */
/*		TRUE32																 */
/*				_PrepareLog is completed									 */
/*		FALSE32																 */
/*				_PrepareLog is failed										 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static LOGCxt *
_PrepareLog(UInt16 wLbn)
{
    LOGCxt *pLog;

    FTL_LOG_PRINT((TEXT("[FTL: IN] ++_PrepareLog(nLbn:%d)\n"), wLbn));

    pLog = _ScanLogSection(wLbn);

    if (pLog == NULL)
    {
        /* if log is not allocated */
        BOOL32 boolCleanLogFound = FALSE32;

        for (pLog = pstFTLCxt->aLOGCxtTable;
             pLog != pstFTLCxt->aLOGCxtTable + LOG_SECTION_SIZE;
             pLog++)
        {
            if (pLog->wVbn != LOG_EMPTY_SLOT &&
                pLog->wFreePOffset == 0)
            {
                boolCleanLogFound = TRUE32;
                break;
            }
        }

        if (boolCleanLogFound == TRUE32)
        {
            /* find free log context */
            FTL_SET_LOG_LBN(pLog, wLbn);
        }
        else
        {
            if (pstFTLCxt->wNumOfFreeVb == FREE_LIST_SIZE)
            {
                /* make a new free block */
                if (_Merge(NULL) == FALSE32)
                {
                    FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on _Merge!\n")));
                    return NULL;
                }
            }

            pLog = pstFTLCxt->aLOGCxtTable;

            while (pLog->wVbn != LOG_EMPTY_SLOT)
            {
                pLog++;
            }

            FTL_SET_LOG_LBN(pLog, wLbn);
            {
                UInt16 wVbnForLog;
                if (_GetFreeVb(&wVbnForLog) == FALSE32)
                {
                    FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on _GetFreeVb (line:%d)!\n"), __LINE__));
                    return NULL;
                }
                ASSIGN_VBN_TO_LOG(pLog, wVbnForLog);
            }
        }

        /* initialize the newly generated log context */
        _InitLogCxt(pLog);
    }

    pLog->dwLogAge = pstFTLCxt->dwWriteAge - 1;

    if (pLog->dwLogAge == 0) /* NirW - check to see if this if is needed */
    {
        UInt32 nIdx;
        LOGCxt *pTempLog;

        for (nIdx = 0; nIdx < LOG_SECTION_SIZE; nIdx++)
        {
            pTempLog = GET_LOGCxt(nIdx);
            pTempLog->dwLogAge = 0;
        }
    }

    FTL_LOG_PRINT((TEXT("[FTL:OUT] --_PrepareLog(nLbn:%d)\n"), wLbn));

    return pLog;
}
#endif

#if !defined(AND_READONLY) || \
    !(defined(WMR_DISABLE_FTL_RECOVERY) && WMR_DISABLE_FTL_RECOVERY)

#define FTL_EXTRA_LOG_CXT   (&(pstFTLCxt->aLOGCxtTable[LOG_SECTION_SIZE]))
void _CopyLOGCxt(LOGCxt * pLOGDst, LOGCxt * pLOGSrc)
{
    pLOGDst->boolCopyMerge = pLOGSrc->boolCopyMerge;
    pLOGDst->dwLogAge = pLOGSrc->dwLogAge;
    pLOGDst->wFreePOffset = pLOGSrc->wFreePOffset;
    FTL_SET_LOG_LBN(pLOGDst, pLOGSrc->wLbn);
    pLOGDst->wNumOfValidLP = pLOGSrc->wNumOfValidLP;
    pLOGDst->wVbn = pLOGSrc->wVbn;
    WMR_MEMCPY(pLOGDst->paPOffsetL2P, pLOGSrc->paPOffsetL2P, (PAGES_PER_SUBLK * sizeof(UInt16)));
}
#endif /* !defined(AND_READONLY) || \
          !(defined(WMR_DISABLE_FTL_RECOVERY) && WMR_DISABLE_FTL_RECOVERY) */

#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _DoMoveMerge		                                                 */
/* DESCRIPTION                                                               */
/*      This function operates move merge. (log --> free)					 */
/* PARAMETERS                                                                */
/*      pLogVictim 	[IN] 	the pointer of log block context (victim)        */
/* RETURN VALUES                                                             */
/*		TRUE32																 */
/*				_DoMoveMerge is completed									 */
/*		FALSE32																 */
/*				_DoMoveMerge is failed										 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static BOOL32
_DoMoveMerge(LOGCxt *pLogVictim)
{
    UInt32 nIdx;
    UInt16 wF2LVbn;
    UInt16 wL2FVbn;
    UInt32 nSrcPOffset;
    UInt8 bTrialIdx;
    BOOL32 boolRes = FALSE32;
    Buffer *pBuff;

    FTL_LOG_PRINT((TEXT("[FTL: IN] ++_DoMoveMerge(nLbn:%d)\n"), pLogVictim->wLbn));

#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwDoMoveMergeCnt++;
#endif
    /* check for the rare case that the log failed completely and contains no valid blocks */
    if (pLogVictim->wNumOfValidLP == 0)
    {
        if (_SetFreeVb(pLogVictim->wVbn) == FALSE32)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR] failed _SetFreeVb(0x%X) (line:%d)!\n"), pLogVictim->wVbn, __LINE__));
            return FALSE32;
        }
        MARK_LOG_AS_EMPTY(pLogVictim);
        return TRUE32;
    }
    pBuff = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBuff == NULL)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] failed BUF_Get (line:%d)!\n"), __LINE__));
        return FALSE32;
    }
    wL2FVbn = pLogVictim->wVbn;
    _CopyLOGCxt(FTL_EXTRA_LOG_CXT, pLogVictim);

    for (bTrialIdx = 0; bTrialIdx < 4; bTrialIdx++)
    {
        UInt16 wPagesToCopy;
        UInt16 wWriteOffset;
        /* get free block from free block list */
        if (_GetFreeVb(&wF2LVbn) == FALSE32)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on _GetFreeVb (line:%d)!\n"), __LINE__));
            BUF_Release(pBuff);
            return FALSE32;
        }
        pLogVictim->wFreePOffset = 0;
        pLogVictim->boolCopyMerge = TRUE32;

        /* recalculate number of valid page for disable copy merge operation */
        pLogVictim->wNumOfValidLP = 0;
        wPagesToCopy = 0;
        wWriteOffset = 0;
        pstFTLCxt->dwWriteAge++;
        for (nIdx = 0; nIdx < PAGES_PER_SUBLK; nIdx++)
        {
            nSrcPOffset = pLogVictim->paPOffsetL2P[nIdx];

            if (nSrcPOffset != 0xFFFF)
            {
                pstadwScatteredReadVpn[wPagesToCopy] = GET_Vpn(wL2FVbn, nSrcPOffset);
                pstadwScatteredReadVpn[PAGES_PER_SIMPLE_MERGE_BUFFER + wPagesToCopy] = GET_Vpn(pLogVictim->wLbn, nIdx); // this is the Lpn
                pLogVictim->paPOffsetL2P[nIdx] = pLogVictim->wFreePOffset;

                pLogVictim->wFreePOffset++;

                pLogVictim->wNumOfValidLP++;

                if (pLogVictim->boolCopyMerge == TRUE32)
                {
                    _DisableCopyMerge(pLogVictim, nIdx);
                }
                wPagesToCopy++;
            }
            if (wPagesToCopy == PAGES_PER_SIMPLE_MERGE_BUFFER || ((wPagesToCopy != 0) && (nIdx == (UInt32)(PAGES_PER_SUBLK - 1))))
            {
                UInt16 wLPOffsetIdx;
                if (VFL_ReadScatteredPagesInVb(pstadwScatteredReadVpn, wPagesToCopy, pstSimpleMergeDataBuffer, pstbaMultiplePagesWriteSpare, NULL, NULL, FALSE32, NULL) == TRUE32)
                {
                    for (wLPOffsetIdx = 0; wLPOffsetIdx < wPagesToCopy; wLPOffsetIdx++)
                    {
                        UInt8 * pbaTempSpare = &(pstbaMultiplePagesWriteSpare[wLPOffsetIdx * BYTES_PER_METADATA_RAW]);
                        _SetLOGCxtSpare(pstadwScatteredReadVpn[PAGES_PER_SIMPLE_MERGE_BUFFER + wLPOffsetIdx], pstFTLCxt->dwWriteAge, pbaTempSpare);
                    }
                }
                else
                {
                    for (wLPOffsetIdx = 0; wLPOffsetIdx < wPagesToCopy; wLPOffsetIdx++)
                    {
                        Int32 nRet;
                        UInt8 * pbaTempSpare = &(pstbaMultiplePagesWriteSpare[wLPOffsetIdx * BYTES_PER_METADATA_RAW]);
                        _SetLOGCxtSpare(pstadwScatteredReadVpn[PAGES_PER_SIMPLE_MERGE_BUFFER + wLPOffsetIdx], pstFTLCxt->dwWriteAge, pbaTempSpare);
                        pBuff->pData = &pstSimpleMergeDataBuffer[BYTES_PER_PAGE * wLPOffsetIdx];
                        nRet = VFL_Read(pstadwScatteredReadVpn[wLPOffsetIdx], pBuff, TRUE32, TRUE32, NULL, NULL, FALSE32);
                        if (nRet != VFL_SUCCESS && nRet != VFL_SUCCESS_CLEAN)
                        {
                            SET_FTL_ECC_MARK(pbaTempSpare);
                        }
                    }
                }
                // write
                if (VFL_WriteMultiplePagesInVb(GET_Vpn(wF2LVbn, wWriteOffset), wPagesToCopy, pstSimpleMergeDataBuffer, pstbaMultiplePagesWriteSpare, TRUE32, FALSE32) == FALSE32)
                {
                    break;
                }

                wWriteOffset += wPagesToCopy;
                wPagesToCopy = 0;
            }
        }
        if (nIdx == PAGES_PER_SUBLK)
        {
            /* release the log block to free block */
            if (_SetFreeVb(wL2FVbn) == FALSE32)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR] failed _SetFreeVb(0x%X) (line:%d)!\n"), pLogVictim->wVbn, __LINE__));
                boolRes = FALSE32;
                break;
            }

            ASSIGN_VBN_TO_LOG(pLogVictim, wF2LVbn);
            boolRes = TRUE32;
            break;
        }
        else
        {
            /* restore the values in the log */
            _CopyLOGCxt(pLogVictim, FTL_EXTRA_LOG_CXT);
            if (_SetFreeVb(wF2LVbn) == FALSE32)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR] failed _SetFreeVb(0x%X) (line:%d)!\n"), wF2LVbn, __LINE__));
                boolRes = FALSE32;
                break;
            }
        }
    }
    BUF_Release(pBuff);
    FTL_LOG_PRINT((TEXT("[FTL:OUT] --_DoMoveMerge(nLbn:%d)\n"), pLogVictim->wLbn));
    if (boolRes == FALSE32)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] _DoMoveMerge failed!\n")));
    }
    return boolRes;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _DoCopyMerge		                                                 */
/* DESCRIPTION                                                               */
/*      This function operates copy merge. (data --> log)					 */
/* PARAMETERS                                                                */
/*      pLogVictim 	[IN] 	the pointer of log block context (victim)        */
/* RETURN VALUES                                                             */
/*		TRUE32																 */
/*				_DoCopyMerge is completed									 */
/*		FALSE32																 */
/*				_DoCopyMerge is failed										 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static BOOL32
_DoCopyMerge(LOGCxt *pLogVictim)
{
    UInt16 wD2FVbn, wL2DVbn;
    BOOL32 boolRes = FALSE32;
    UInt16 wPagesToCopy;
    Buffer * pBuff;

    FTL_LOG_PRINT((TEXT("[FTL: IN] ++_DoCopyMerge(nLbn:%d)\n"), pLogVictim->wLbn));

    //WMR_ASSERT(pLogVictim->boolCopyMerge == TRUE32);
    //WMR_ASSERT(pLogVictim->wFreePOffset == pLogVictim->wNumOfValidLP);
    if ((pLogVictim->boolCopyMerge == FALSE32) || (pLogVictim->wFreePOffset != pLogVictim->wNumOfValidLP))
    {
        return FALSE32;
    }

    pBuff = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBuff == NULL)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] failed BUF_Get (line:%d)!\n"), __LINE__));
        return FALSE32;
    }

    wD2FVbn = CONVERT_L2V(pLogVictim->wLbn);

    wL2DVbn = pLogVictim->wVbn;
#ifdef AND_COLLECT_STATISTICS
    if (pLogVictim->wFreePOffset < PAGES_PER_SUBLK)
    {
        stFTLStatistics.ddwDoCopyMergeCnt2++;
    }
    else
    {
        stFTLStatistics.ddwDoCopyMergeCnt1++;
    }
#endif

    /* copy rest pages from data to log */
    for (; pLogVictim->wFreePOffset < PAGES_PER_SUBLK; pLogVictim->wFreePOffset += wPagesToCopy)
    {
        BOOL32 boolReadRes;
        UInt16 wLPOffsetIdx;
        wPagesToCopy = WMR_MIN((PAGES_PER_SUBLK - pLogVictim->wFreePOffset), PAGES_PER_SIMPLE_MERGE_BUFFER);
        // No need to count reads here, as this is the last read from the block before it is recycled
        if (NULL != VFL_ReadMultiplePagesInVb)
        {
            boolReadRes = VFL_ReadMultiplePagesInVb(wD2FVbn, pLogVictim->wFreePOffset, wPagesToCopy, pstSimpleMergeDataBuffer, pstbaMultiplePagesWriteSpare, NULL, NULL, FALSE32);
        }
        else
        {
            for (wLPOffsetIdx = 0; wLPOffsetIdx < wPagesToCopy; wLPOffsetIdx++)
            {
                pstadwScatteredReadVpn[wLPOffsetIdx] = GET_Vpn(wD2FVbn, pLogVictim->wFreePOffset + wLPOffsetIdx);
            }
            boolReadRes = VFL_ReadScatteredPagesInVb(pstadwScatteredReadVpn, wPagesToCopy, pstSimpleMergeDataBuffer, pstbaMultiplePagesWriteSpare, NULL, NULL, FALSE32, NULL);
        }
        if (TRUE32 == boolReadRes)
        {
            for (wLPOffsetIdx = 0; wLPOffsetIdx < wPagesToCopy; wLPOffsetIdx++)
            {
                UInt8 * pbaTempSpare = &(pstbaMultiplePagesWriteSpare[wLPOffsetIdx * BYTES_PER_METADATA_RAW]);
                _SetLOGCxtSpare(GET_Vpn(pLogVictim->wLbn, (pLogVictim->wFreePOffset + wLPOffsetIdx)), pstFTLCxt->dwWriteAge, pbaTempSpare);
            }
        }
        else
        {
            for (wLPOffsetIdx = 0; wLPOffsetIdx < wPagesToCopy; wLPOffsetIdx++)
            {
                Int32 nRet;
                UInt8 * pbaTempSpare = &(pstbaMultiplePagesWriteSpare[wLPOffsetIdx * BYTES_PER_METADATA_RAW]);
                _SetLOGCxtSpare(GET_Vpn(pLogVictim->wLbn, (pLogVictim->wFreePOffset + wLPOffsetIdx)), pstFTLCxt->dwWriteAge, pbaTempSpare);
                pBuff->pData = &pstSimpleMergeDataBuffer[BYTES_PER_PAGE * wLPOffsetIdx];
                nRet = VFL_Read(GET_Vpn(wD2FVbn, (pLogVictim->wFreePOffset + wLPOffsetIdx)), pBuff, TRUE32, TRUE32, NULL, NULL, FALSE32);
                if (nRet != VFL_SUCCESS && nRet != VFL_SUCCESS_CLEAN)
                {
                    SET_FTL_ECC_MARK(pbaTempSpare);
                }
            }
        }
        /* check if this write contains the last page, if it does, mark as data */
        if ((pLogVictim->wFreePOffset + wLPOffsetIdx) == PAGES_PER_SUBLK)
        {
            SET_FTL_SPARE_TYPE(&(pstbaMultiplePagesWriteSpare[(wLPOffsetIdx - 1) * BYTES_PER_METADATA_RAW]), FTL_SPARE_TYPE_DATA);
        }
        if (VFL_WriteMultiplePagesInVb(GET_Vpn(wL2DVbn, pLogVictim->wFreePOffset), wPagesToCopy, pstSimpleMergeDataBuffer, pstbaMultiplePagesWriteSpare, TRUE32, FALSE32) == FALSE32)
        {
            break;
        }
    }

    if (pLogVictim->wFreePOffset < PAGES_PER_SUBLK)
    {
        pLogVictim->boolCopyMerge = FALSE32;
        boolRes = _DoSimpleMerge(pLogVictim);
    }
    else
    {
        /* release data block to free block */
        if (_SetFreeVb(wD2FVbn) == FALSE32)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR] failed _SetFreeVb(0x%X) (line:%d)!\n"), wD2FVbn, __LINE__));
        }

        /* modify map table */
        MODIFY_LB_MAP(pLogVictim->wLbn, wL2DVbn);

        MARK_LOG_AS_EMPTY(pLogVictim);
        boolRes = TRUE32;
    }
    BUF_Release(pBuff);
    FTL_LOG_PRINT((TEXT("[FTL:OUT] --_DoCopyMerge(nLbn:%d)\n"), pLogVictim->wLbn));

    return boolRes;
}

/*
    NAME
    _DoSimpleMerge
    DESCRIPTION
    This function operates simple merge. (log & data --> free)
    PARAMETERS
    pLogVictim 	[IN] 	the pointer of log block context (victim)
    RETURN VALUES
    TRUE32
        _DoSimpleMerge is completed
    FALSE32
        _DoSimpleMerge is failed
    NOTES
 */

static BOOL32
_DoSimpleMerge(LOGCxt *pLogVictim)
{
    UInt16 wIdx, wLPOffsetIdx;
    UInt16 wF2DVbn;
    UInt8 bTrialIdx;
    BOOL32 boolRes = FALSE32, boolCopyRes = FALSE32;
    // use a different area of the spare buffer than the one used by _FTLRead that reads PAGES_PER_SIMPLE_MERGE_BUFFER pages
    UInt8   *pabMultiSpareBuffer = &pstbaMultiplePagesWriteSpare[PAGES_PER_SIMPLE_MERGE_BUFFER * BYTES_PER_METADATA_RAW];

    FTL_LOG_PRINT((TEXT("[FTL: IN] ++_DoSimpleMerge(nLbn:%d)\n"), pLogVictim->wLbn));

    // WMR_ASSERT(pLogVictim != NULL);
    if (pLogVictim == NULL)
    {
        return FALSE32;
    }

#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwDoSimpleMergeCnt++;
#endif

    for (bTrialIdx = 0; bTrialIdx < 4; bTrialIdx++)
    {
        if (_GetFreeVb(&wF2DVbn) == FALSE32)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on _GetFreeVb (line:%d)!\n"), __LINE__));
            return FALSE32;
        }
        pstFTLCxt->dwWriteAge++;
        for (wIdx = 0; wIdx < PAGES_PER_SUBLK; wIdx += PAGES_PER_SIMPLE_MERGE_BUFFER)
        {
            if (_FTLRead(((((UInt32)pLogVictim->wLbn) * PAGES_PER_SUBLK) + (UInt32)wIdx), PAGES_PER_SIMPLE_MERGE_BUFFER, pstSimpleMergeDataBuffer) == FTL_SUCCESS)
            {
                for (wLPOffsetIdx = 0; wLPOffsetIdx < PAGES_PER_SIMPLE_MERGE_BUFFER; wLPOffsetIdx++)
                {
                    UInt8 * pbaTempSpare = &(pabMultiSpareBuffer[wLPOffsetIdx * BYTES_PER_METADATA_RAW]);
                    _SetLOGCxtSpare(GET_Vpn(pLogVictim->wLbn, (wIdx + wLPOffsetIdx)), pstFTLCxt->dwWriteAge, pbaTempSpare);
                }
            }
            else
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR] failed calling _FTLRead(0x%X, 0x%X, 0x%X)(line:%d)\n"),
                               ((((UInt32)pLogVictim->wLbn) * PAGES_PER_SUBLK) + (UInt32)wIdx), PAGES_PER_SIMPLE_MERGE_BUFFER, pstSimpleMergeDataBuffer, __LINE__));
                for (wLPOffsetIdx = 0; wLPOffsetIdx < PAGES_PER_SIMPLE_MERGE_BUFFER; wLPOffsetIdx++)
                {
                    UInt8 * pbaTempSpare = &(pabMultiSpareBuffer[wLPOffsetIdx * BYTES_PER_METADATA_RAW]);
                    _SetLOGCxtSpare(GET_Vpn(pLogVictim->wLbn, (wIdx + wLPOffsetIdx)), pstFTLCxt->dwWriteAge, pbaTempSpare);
                    if (_FTLRead(((((UInt32)pLogVictim->wLbn) * PAGES_PER_SUBLK) + (UInt32)(wIdx + wLPOffsetIdx)), 1, &pstSimpleMergeDataBuffer[BYTES_PER_PAGE * wLPOffsetIdx]) != FTL_SUCCESS)
                    {
                        SET_FTL_ECC_MARK(pbaTempSpare);
                        FTL_ERR_PRINT((TEXT("[FTL:ERR] failed calling _FTLRead(0x%X, 0x%X, 0x%X) (line:%d)\n"),
                                       ((((UInt32)pLogVictim->wLbn) * PAGES_PER_SUBLK) + (UInt32)(wIdx + wLPOffsetIdx)), 1, &pstSimpleMergeDataBuffer[BYTES_PER_PAGE * wLPOffsetIdx], __LINE__));
                    }
                }
            }
            if ((wLPOffsetIdx + wIdx) == PAGES_PER_SUBLK)
            {
                SET_FTL_SPARE_TYPE(&(pabMultiSpareBuffer[(wLPOffsetIdx - 1) * BYTES_PER_METADATA_RAW]), FTL_SPARE_TYPE_DATA);
            }
            boolCopyRes = VFL_WriteMultiplePagesInVb(GET_Vpn(wF2DVbn, wIdx), PAGES_PER_SIMPLE_MERGE_BUFFER, pstSimpleMergeDataBuffer, pabMultiSpareBuffer, TRUE32, FALSE32);
            if (boolCopyRes == FALSE32)
            {
                FTL_WRN_PRINT((TEXT("[FTL:WRN] _DoSimpleMerge error writing to free page\n")));
                break;
            }
        }
        if (boolCopyRes == TRUE32)
        {
            boolRes = TRUE32;
            break;
        }
        else
        {
            if (_SetFreeVb(wF2DVbn) == FALSE32)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR] _DoSimpleMerge failed _SetFreeVb(0x%X) (line:%d)\n"), wF2DVbn, __LINE__));
            }
        }
    }
    if (boolRes == TRUE32)
    {
        /* release log and data blocks */
        if (_SetFreeVb(pLogVictim->wVbn) == FALSE32)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR] _DoSimpleMerge failed _SetFreeVb(0x%X) (line:%d)\n"), pLogVictim->wVbn, __LINE__));
        }
        MARK_LOG_AS_EMPTY(pLogVictim);
        if (_SetFreeVb(CONVERT_L2V(pLogVictim->wLbn)) == FALSE32)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR] _DoSimpleMerge failed _SetFreeVb(0x%X) (line:%d)\n"), CONVERT_L2V(pLogVictim->wLbn), __LINE__));
        }
        /* modify map table */
        MODIFY_LB_MAP(pLogVictim->wLbn, wF2DVbn);
    }

    FTL_LOG_PRINT((TEXT("[FTL:OUT] --_DoSimpleMerge(nLbn:%d)\n"), pLogVictim->wLbn));
    if (boolRes == FALSE32)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] _DoSimpleMerge failed!\n")));
    }
    return boolRes;
}
/*
    NAME
    _Merge
    DESCRIPTION
    This function select merge operation.
    PARAMETERS
    pLogVictim 	[IN] 	the pointer of log block context (victim)
    RETURN VALUES
    TRUE32
        _Merge is completed
    FALSE32
        _Merge is failed
    NOTES
 */
static BOOL32
_Merge(LOGCxt *pLogVictim)
{
    UInt32 eMergeMethod = 0;

    enum
    {
        MERGE_SIMPLE, MERGE_COPY_SWAP, MERGE_MOVE
    };

    FTL_LOG_PRINT((TEXT("[FTL: IN] ++_Merge()\n")));

    if (_FTLMarkCxtNotValid() == FALSE32)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] _Merge failed calling _FTLMarkCxtNotValid\n")));
        return FALSE32;
    }
    /* search victim log block if there is no victim */
    /* copy > simple (victim == NULL)	 			 */
    /* move > copy > simple (vicim != NULL)			 */
    if (pLogVictim == NULL)
    {
        UInt32 nMinMergeCost = 0xFFFFFFFF;
        UInt32 nIdx;

        for (nIdx = 0; nIdx < LOG_SECTION_SIZE; nIdx++)
        {
            UInt32 nMergeCost, eMergeMethodTemp;
            LOGCxt *pLog = GET_LOGCxt(nIdx);

            if (pLog->wVbn == LOG_EMPTY_SLOT)
            {
                continue;
            }

            //WMR_ASSERT(pLog->wFreePOffset != 0);
            //WMR_ASSERT(pLog->wNumOfValidLP != 0);
            if ((pLog->wFreePOffset == 0) || (pLog->wNumOfValidLP == 0))
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR]  _Merge error(line:%d)!\n"), __LINE__));
                return FALSE32;
            }

            if (pLog->boolCopyMerge == TRUE32)
            {
                eMergeMethodTemp = MERGE_COPY_SWAP;
                nMergeCost = pLog->dwLogAge;
            }
            else
            {
                eMergeMethodTemp = MERGE_SIMPLE;
                nMergeCost = pLog->dwLogAge;
            }

            if (nMergeCost < nMinMergeCost)
            {
                nMinMergeCost = nMergeCost;
                eMergeMethod = eMergeMethodTemp;
                pLogVictim = pLog;
            }
            else if (pLogVictim != NULL &&
                     nMergeCost == nMinMergeCost &&
                     pLog->wNumOfValidLP > pLogVictim->wNumOfValidLP)
            {
                nMinMergeCost = nMergeCost;
                eMergeMethod = eMergeMethodTemp;
                pLogVictim = pLog;
            }
        }
    }
    else
    {
        if (pLogVictim->wNumOfValidLP <= PAGES_PER_SUBLK / 2)
        {
            // note that the _DoMoveMerge function does not turn log to data even if the number of valid blocks is PAGES_PER_SUBLK
            eMergeMethod = MERGE_MOVE;
        }
        else if (pLogVictim->boolCopyMerge == TRUE32)
        {
            eMergeMethod = MERGE_COPY_SWAP;
        }
        else
        {
            eMergeMethod = MERGE_SIMPLE;
        }
    }

    if (pLogVictim == NULL)
    {
        return FALSE32;
    }
    if (eMergeMethod == MERGE_MOVE)
    {
        if (_DoMoveMerge(pLogVictim) == FALSE32)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on _DoMoveMerge!\n")));
            return FALSE32;
        }
    }
    else
    {
        if (eMergeMethod == MERGE_COPY_SWAP)
        {
            if (_DoCopyMerge(pLogVictim) == FALSE32)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on _DoCopyMerge!\n")));
                return FALSE32;
            }
        }
        else
        {
            if (_DoSimpleMerge(pLogVictim) == FALSE32)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on _DoSimpleMerge!\n")));
                return FALSE32;
            }
        }
    }

    pstFTLCxt->wWearLevelCounter++;

    FTL_LOG_PRINT((TEXT("[FTL:OUT] --_Merge()\n")));

    return TRUE32;
}
#endif // AND_READONLY

#if !(defined(WMR_DISABLE_FTL_RECOVERY) && WMR_DISABLE_FTL_RECOVERY)
BOOL32 _ScanForFreeBlk(UInt16 wVBlk)
{
    UInt16 wIdx;

    for (wIdx = 0; wIdx < pstFTLCxt->wNumOfFreeVb; wIdx++)
    {
        UInt16 wIdxTemp = wIdx + pstFTLCxt->wFreeVbListTail;

        if (wIdxTemp >= FREE_SECTION_SIZE)
        {
            wIdxTemp -= FREE_SECTION_SIZE;
        }
        if (pstFTLCxt->awFreeVbList[wIdxTemp] == wVBlk)
        {
            return TRUE32;
        }
    }
    return FALSE32;
}
// return values:
// FLASE32 - log block
// TRUE32 - potential data block
BOOL32  _CheckLogBlock(UInt16 wVbn)
{
    Int32 nVFLRet;
    UInt16 wPageIdx;
    Buffer * pBuff;

    pBuff = BUF_Get(BUF_MAIN_AND_SPARE);

    if (pBuff == NULL)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] failed BUF_Get (line:%d)!\n"), __LINE__));
        return FALSE32;
    }

    for (wPageIdx = 0; wPageIdx < PAGES_PER_SUBLK; wPageIdx++)
    {
        UInt32 dwWrittenLpn;
        nVFLRet = VFL_Read(GET_Vpn(wVbn, wPageIdx), pBuff, TRUE32, FALSE32, NULL, NULL, FALSE32);
        if (nVFLRet == VFL_SUCCESS)
        {
            _GetLOGCxtSpare(pBuff->pSpare, &dwWrittenLpn, NULL);
            if ((dwWrittenLpn % PAGES_PER_SUBLK) != wPageIdx)
            {
                BUF_Release(pBuff);
                return FALSE32;
            }
        }
    }
    BUF_Release(pBuff);
    return TRUE32;
}
#endif //#if (defined(WMR_ENABLE_FTL_RECOVERY) && WMR_ENABLE_FTL_RECOVERY)

/* this function sould be called if there are no valid context structures */

#define FTL_RESTORE_BLOCK_FULL              0
#define FTL_RESTORE_BLOCK_0_IS_WRITTEN      1
#define FTL_RESTORE_BLOCK_0_IS_EMPTY        2
#define FTL_RESTORE_BLOCK_0_IS_INVALID      3

#if !(defined(WMR_DISABLE_FTL_RECOVERY) && WMR_DISABLE_FTL_RECOVERY)
BOOL32
_FTLRestore(void)
{
    Buffer * pBuff;
    BOOL32 boolRes = TRUE32;
    FTLRestoreStruct * pFTLRestoreStruct = NULL; /* per virtual block (vb) mark logical block it contains */
    UInt16 wVBlkIdx, wLBlkIdx;
    UInt16 wDataBlkCnt, wLogBlkCnt, wFreeBlkCnt;

#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwFTLRestoreCnt++;
#endif /* AND_COLLECT_STATISTICS */
    FTL_WRN_PRINT((TEXT("[FTL:WRN] Recovering NAND Data Structures - this will take some time!\n")));
    /* allocate a buffer to read pages from the nand */
    pBuff = BUF_Get(BUF_MAIN_AND_SPARE);
    if (pBuff == NULL)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] BUF_Get error (line:%d)!\n"), __LINE__));
        return FALSE32;
    }

    /* instead of allocating, we now share with pstSimpleMergeDataBuffer (32KB) */
    pFTLRestoreStruct = (FTLRestoreStruct *) pstSimpleMergeDataBuffer;

    /* this function expects pFTLRestoreStruct to be initialized with 0xFF */
    WMR_MEMSET(pFTLRestoreStruct, 0xFF, FTL_AREA_SIZE * sizeof(FTLRestoreStruct));

    /* init mapping in FTLCxt */
    WMR_MEMSET(pstFTLCxt->pawMapTable, 0xFF, (sizeof(UInt16) * USER_SUBLKS_TOTAL)); /* clear the map table */

    // init the refresh block information
    WMR_MEMSET(pstFTLCxt->aFTLReadRefreshList, 0xFF,
               (sizeof(FTLReadRefreshStruct) * FTL_READ_REFRESH_LIST_SIZE));
    pstFTLCxt->dwRefreshListIdx = 0;

    /* init the log structures */
    for (wLBlkIdx = 0; wLBlkIdx < (LOG_SECTION_SIZE + 1); wLBlkIdx++)
    {
        _InitLogCxt(&pstFTLCxt->aLOGCxtTable[wLBlkIdx]);
        pstFTLCxt->aLOGCxtTable[wLBlkIdx].wLbn = pstFTLCxt->aLOGCxtTable[wLBlkIdx].wVbn = LOG_EMPTY_SLOT;
    }

#ifndef AND_READONLY
    /* init the ec cache */
    WMR_MEMSET(pstFTLCxt->pawECCacheTable, 0, (FTL_AREA_SIZE * sizeof(UInt16))); /* clear the ec cache table */

    /* init the rc cache */
    WMR_MEMSET(pstFTLCxt->pawRCCacheTable, 0, (FTL_AREA_SIZE * sizeof(UInt16))); /* clear the ec cache table */

    /* init the ec and rc pointers */
    for (wLBlkIdx = 0; wLBlkIdx < MAX_NUM_OF_EC_TABLES; wLBlkIdx++)
    {
        pstFTLCxt->adwECTablePtrs[wLBlkIdx] = FTL_TABLE_PTR_INVALID_VALUE; /* 0 might be a valid value */
        pstFTLCxt->adwRCTablePtrs[wLBlkIdx] = FTL_TABLE_PTR_INVALID_VALUE; /* 0 might be a valid value */
    }
#endif /* #ifndef AND_READONLY */

    // init the stat pointers
    pstFTLCxt->adwStatPtrs[0] = FTL_TABLE_PTR_INVALID_VALUE;
    pstFTLCxt->adwStatPtrs[1] = FTL_TABLE_PTR_INVALID_VALUE;

    pstFTLCxt->dwWriteAge = 0;
    pstFTLCxt->dwAge = 0xFFFFFFFF;

    /* mark the cxt blocks */
    WMR_MEMCPY(pstFTLCxt->awMapCxtVbn, VFL_GetFTLCxtVbn(), (sizeof(UInt16) * FTL_CXT_SECTION_SIZE));
    for (wLBlkIdx = 0; wLBlkIdx < FTL_CXT_SECTION_SIZE; wLBlkIdx++)
    {
        Int32 nVFLRet;
        nVFLRet = VFL_Read(GET_Vpn(pstFTLCxt->awMapCxtVbn[wLBlkIdx], 0), pBuff, TRUE32, FALSE32, NULL, NULL, FALSE32);
        if (nVFLRet == VFL_SUCCESS && FTL_SPARE_IS_CXT_MARK(GET_FTL_SPARE_TYPE(pBuff->pSpare)))
        {
            _GetFTLCxtSpare(&(pFTLRestoreStruct[pstFTLCxt->awMapCxtVbn[wLBlkIdx]].dwWriteAge), NULL, NULL, pBuff->pSpare);
            pstFTLCxt->dwAge = WMR_MIN(pFTLRestoreStruct[pstFTLCxt->awMapCxtVbn[wLBlkIdx]].dwWriteAge, pstFTLCxt->dwAge);
        }
        pFTLRestoreStruct[pstFTLCxt->awMapCxtVbn[wLBlkIdx]].bSpareType = FTL_SPARE_TYPE_CXT;
        if (pstFTLCxt->dwAge == pFTLRestoreStruct[pstFTLCxt->awMapCxtVbn[wLBlkIdx]].dwWriteAge)
        {
            pstFTLCxt->dwCurrMapCxtPage = GET_Vpn(pstFTLCxt->awMapCxtVbn[wLBlkIdx], (PAGES_PER_SUBLK - 1));
        }
    }

    /* search blocks that belong to the FTL layer look for data and log block types */
    for (wVBlkIdx = 0; wVBlkIdx < FTL_AREA_SIZE; wVBlkIdx++)
    {
        Int32 nVFLRet;

        if (pFTLRestoreStruct[wVBlkIdx].bSpareType == FTL_SPARE_TYPE_CXT)
        {
            continue;
        }
        /* read the last page of the VB (look for full logs and data blocks) */
        WMR_MEMSET(pBuff->pSpare, 0xFF, BYTES_PER_METADATA_RAW);
        nVFLRet = VFL_Read(GET_Vpn(wVBlkIdx, (PAGES_PER_SUBLK - 1)), pBuff, TRUE32, FALSE32, NULL, NULL, FALSE32);
        if (nVFLRet == VFL_SUCCESS)
        {
            /* sort the VBs by the type byte */
            switch (GET_FTL_SPARE_TYPE(pBuff->pSpare))
            {
                /* full logs and data blocks */
            case FTL_SPARE_TYPE_LOG:
            case FTL_SPARE_TYPE_DATA:
                {
                    UInt32 dwLbn, dwLpn;
                    _GetLOGCxtSpare(pBuff->pSpare, &dwLpn, &(pFTLRestoreStruct[wVBlkIdx].dwWriteAge));
                    dwLbn = (dwLpn / PAGES_PER_SUBLK);
                    if (IS_LBN_IN_RANGE((UInt16)dwLbn))
                    {
                        pFTLRestoreStruct[wVBlkIdx].bSpareType = GET_FTL_SPARE_TYPE(pBuff->pSpare);
                        pFTLRestoreStruct[wVBlkIdx].bIsBlkFull = FTL_RESTORE_BLOCK_FULL;
                        pFTLRestoreStruct[wVBlkIdx].wLbn = (UInt16)dwLbn;
                        /* if this is a data block map it */
                        if (pFTLRestoreStruct[wVBlkIdx].bSpareType == FTL_SPARE_TYPE_DATA)
                        {
                            UInt16 wLbn;
                            wLbn = pFTLRestoreStruct[wVBlkIdx].wLbn;

                            if (pstFTLCxt->pawMapTable[wLbn] == 0xFFFF)
                            {
                                /* this is the first data block copy for this logical block - map it */
                                MODIFY_LB_MAP(wLbn, wVBlkIdx);
                            }
                            else
                            {
                                UInt16 wOldVbn;
                                wOldVbn = pstFTLCxt->pawMapTable[wLbn];
                                /* this is not the only data block copy for this logical block - map it only if it is newer than the previous version */
                                if (pFTLRestoreStruct[wVBlkIdx].dwWriteAge > pFTLRestoreStruct[wOldVbn].dwWriteAge)
                                {
                                    MODIFY_LB_MAP(wLbn, wVBlkIdx);
                                }
                                else
                                {
                                    wOldVbn = wVBlkIdx;
                                }
                                pFTLRestoreStruct[wOldVbn].bSpareType = FTL_SPARE_TYPE_FREE; /* mark the older block as free */
                            }
                        }
                        if (pstFTLCxt->dwWriteAge < pFTLRestoreStruct[wVBlkIdx].dwWriteAge)
                        {
                            pstFTLCxt->dwWriteAge = pFTLRestoreStruct[wVBlkIdx].dwWriteAge;
                        }
                    }
                    else
                    {
                        FTL_ERR_PRINT((TEXT("[FTL:ERR] _FTLRestore found block (#%d) with wLbn %d!\n"), wVBlkIdx, dwLbn));
                        pFTLRestoreStruct[wVBlkIdx].bIsBlkFull = FTL_RESTORE_BLOCK_0_IS_WRITTEN;
                        pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_FREE;
                    }
                    break;
                }

                /* FTLCxt types */
            case FTL_SPARE_TYPE_CXT_INDEX:
            case FTL_SPARE_TYPE_CXT_MAP_TABLE:
            case FTL_SPARE_TYPE_CXT_EC_TABLE:
            case FTL_SPARE_TYPE_CXT_RC_TABLE:
            case FTL_SPARE_TYPE_CXT_LOGCXT_MAP:
            case FTL_SPARE_TYPE_CXT_INVALID:
            case FTL_SPARE_TYPE_CXT_STAT:
                {
                    /* mark the type of the block as free - real FTL_SPARE_TYPE_CXT will be marked later */
                    pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_FREE;

                    pFTLRestoreStruct[wVBlkIdx].bIsBlkFull = FTL_RESTORE_BLOCK_FULL;
                    _GetFTLCxtSpare(&(pFTLRestoreStruct[wVBlkIdx].dwWriteAge), NULL, NULL, pBuff->pSpare);
                    break;
                }

            default:
                FTL_ERR_PRINT((TEXT("[FTL:ERR] _FTLRestore found block (#%d) with unidentified spare(page is 0x%X) spare is 0x%X !\n"),
                               wVBlkIdx, (PAGES_PER_SUBLK - 1), GET_FTL_SPARE_TYPE(pBuff->pSpare)));
                pFTLRestoreStruct[wVBlkIdx].bIsBlkFull = FTL_RESTORE_BLOCK_0_IS_WRITTEN;
                pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_FREE;
            }
        }
        else
        {
            /* if the last page is not written  */
            WMR_MEMSET(pBuff->pSpare, 0xFF, BYTES_PER_METADATA_RAW);
            nVFLRet = VFL_Read(GET_Vpn(wVBlkIdx, 0), pBuff, TRUE32, FALSE32, NULL, NULL, FALSE32);
            if (nVFLRet == VFL_SUCCESS)
            {
                /* this is not a full page - it should be either CXT or LOG */
                switch (GET_FTL_SPARE_TYPE(pBuff->pSpare))
                {
                case FTL_SPARE_TYPE_LOG:
                    {
                        UInt32 dwLbn, dwLpn;
                        _GetLOGCxtSpare(pBuff->pSpare, &dwLpn, &(pFTLRestoreStruct[wVBlkIdx].dwWriteAge));
                        dwLbn = (dwLpn / PAGES_PER_SUBLK);
                        if (IS_LBN_IN_RANGE((UInt16)dwLbn))
                        {
                            pFTLRestoreStruct[wVBlkIdx].bSpareType = GET_FTL_SPARE_TYPE(pBuff->pSpare);
                            pFTLRestoreStruct[wVBlkIdx].bIsBlkFull = FTL_RESTORE_BLOCK_0_IS_WRITTEN;
                            pFTLRestoreStruct[wVBlkIdx].wLbn = (UInt16)dwLbn;
                            if (pstFTLCxt->dwWriteAge < pFTLRestoreStruct[wVBlkIdx].dwWriteAge)
                            {
                                pstFTLCxt->dwWriteAge = pFTLRestoreStruct[wVBlkIdx].dwWriteAge;
                            }
                        }
                        else
                        {
                            pFTLRestoreStruct[wVBlkIdx].bIsBlkFull = FTL_RESTORE_BLOCK_0_IS_WRITTEN;
                            pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_FREE;
                        }
                        break;
                    }

                case FTL_SPARE_TYPE_CXT_INDEX:
                case FTL_SPARE_TYPE_CXT_MAP_TABLE:
                case FTL_SPARE_TYPE_CXT_EC_TABLE:
                case FTL_SPARE_TYPE_CXT_RC_TABLE:
                case FTL_SPARE_TYPE_CXT_LOGCXT_MAP:
                case FTL_SPARE_TYPE_CXT_INVALID:
                case FTL_SPARE_TYPE_CXT_STAT:
                    {
                        /* mark the type of the block as free - real FTL_SPARE_TYPE_CXT will be marked later */
                        pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_FREE;

                        pFTLRestoreStruct[wVBlkIdx].bIsBlkFull = FTL_RESTORE_BLOCK_0_IS_WRITTEN;
                        _GetFTLCxtSpare(&(pFTLRestoreStruct[wVBlkIdx].dwWriteAge), NULL, NULL, pBuff->pSpare);
                        break;
                    }

                default:
                    {
                        FTL_ERR_PRINT((TEXT("[FTL:ERR] _FTLRestore found block (#%d) with unidentified spare(page is 0) 0x%X!\n"),
                                       wVBlkIdx, GET_FTL_SPARE_TYPE(pBuff->pSpare)));
                        pFTLRestoreStruct[wVBlkIdx].bIsBlkFull = FTL_RESTORE_BLOCK_0_IS_WRITTEN;
                        pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_FREE;
                        break;
                    }
                }
            }
            else if (nVFLRet == VFL_SUCCESS_CLEAN)
            {
                pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_FREE;
                pFTLRestoreStruct[wVBlkIdx].bIsBlkFull = FTL_RESTORE_BLOCK_0_IS_EMPTY; // the virtual block is blank
            }
        }
        // this is a simple solution - though we might loose a lot of info this way -
        if (nVFLRet != VFL_SUCCESS_CLEAN && nVFLRet != VFL_SUCCESS)
        {
            pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_FREE;
            pFTLRestoreStruct[wVBlkIdx].bIsBlkFull = FTL_RESTORE_BLOCK_0_IS_WRITTEN;
        }
    }

    /*
        at this point - any data block that has been written and taken off the log list should be in the map.
        for the rest we can assign free blocks.
     */

    /* mark any log that is older than the matching data block as free and put relevant logs in the FTLMeta structure */
    for (wVBlkIdx = 0; wVBlkIdx < FTL_AREA_SIZE; wVBlkIdx++)
    {
        if (pFTLRestoreStruct[wVBlkIdx].bSpareType == FTL_SPARE_TYPE_LOG)
        {
            /* check if the matching data block dwWriteAge is bigger than this log */
            UInt32 dwWriteAge = 0;
            wLBlkIdx = pFTLRestoreStruct[wVBlkIdx].wLbn;
            if (pstFTLCxt->pawMapTable[wLBlkIdx] != 0xFFFF)
            {
                dwWriteAge = pFTLRestoreStruct[pstFTLCxt->pawMapTable[wLBlkIdx]].dwWriteAge;
            }
            else
            {
                // take a look at the log blocks - if it is organized as data block mark it as one...
                if (_CheckLogBlock(wVBlkIdx) == TRUE32)
                {
                    // this can be marked as a data block
                    MODIFY_LB_MAP(wLBlkIdx, wVBlkIdx);
                    pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_DATA;
                    continue;
                }
            }
            if (pFTLRestoreStruct[wVBlkIdx].dwWriteAge < dwWriteAge)
            {
                pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_FREE;
            }
            else
            {
                UInt32 dwLogIdx;
                /* this is possibly real log - check for log with matching lbn */
                for (dwLogIdx = 0; dwLogIdx < (LOG_SECTION_SIZE + 1); dwLogIdx++)
                {
                    BOOL32 boolUpdateLog = FALSE32;
                    /* if we got to a log with lbn set to 0xFFFF than there is no log with this lbn */
                    if (pstFTLCxt->aLOGCxtTable[dwLogIdx].wLbn == 0xFFFF)
                    {
                        boolUpdateLog = TRUE32;
                    }
                    else if (pstFTLCxt->aLOGCxtTable[dwLogIdx].wLbn == pFTLRestoreStruct[wVBlkIdx].wLbn)
                    {
                        UInt16 wOldVbn = pstFTLCxt->aLOGCxtTable[dwLogIdx].wVbn;

                        if (pFTLRestoreStruct[wVBlkIdx].dwWriteAge == pstFTLCxt->dwWriteAge || pstFTLCxt->aLOGCxtTable[dwLogIdx].dwLogAge == pstFTLCxt->dwWriteAge)
                        {
                            LOGCxt * pLOGCxtOld;
                            LOGCxt * pLOGCxtNew;
                            UInt16 wPageIdx;
                            /* this is possibly a power failure issue */
                            FTL_EXTRA_LOG_CXT->dwLogAge = pFTLRestoreStruct[wVBlkIdx].dwWriteAge;
                            FTL_EXTRA_LOG_CXT->wLbn = pFTLRestoreStruct[wVBlkIdx].wLbn;
                            FTL_EXTRA_LOG_CXT->wVbn = (UInt16)wVBlkIdx;
                            _RecoverLOGCxtInfo(FTL_EXTRA_LOG_CXT);
                            _RecoverLOGCxtInfo(&(pstFTLCxt->aLOGCxtTable[dwLogIdx]));
                            /* now we should have the # of valid pages set for both log versions */
                            if (pFTLRestoreStruct[wVBlkIdx].dwWriteAge == pstFTLCxt->dwWriteAge)
                            {
                                pLOGCxtNew = FTL_EXTRA_LOG_CXT;
                                pLOGCxtOld = &(pstFTLCxt->aLOGCxtTable[dwLogIdx]);
                            }
                            else
                            {
                                pLOGCxtOld = FTL_EXTRA_LOG_CXT;
                                pLOGCxtNew = &(pstFTLCxt->aLOGCxtTable[dwLogIdx]);
                            }
                            /* verify that the newer version contains at least all the pages that were in the old version */
                            for (wPageIdx = 0; wPageIdx < PAGES_PER_SUBLK; wPageIdx++)
                            {
                                if (pLOGCxtOld->paPOffsetL2P[wPageIdx] != 0xFFFF && pLOGCxtNew->paPOffsetL2P[wPageIdx] == 0xFFFF)
                                {
                                    break;
                                }
                            }
                            /* check if the newer version is valid */
                            if (wPageIdx == PAGES_PER_SUBLK)
                            {
                                /* the newer log is valid get rid of the old one */
                                if (pLOGCxtOld->dwLogAge == pFTLRestoreStruct[wVBlkIdx].dwWriteAge)
                                {
                                    pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_FREE;
                                }
                                else
                                {
                                    boolUpdateLog = TRUE32;
                                    pFTLRestoreStruct[wOldVbn].bSpareType = FTL_SPARE_TYPE_FREE;
                                }
                            }
                            else
                            {
                                /* the newer log is invalid get rid of it */
                                if (pLOGCxtNew->dwLogAge == pFTLRestoreStruct[wVBlkIdx].dwWriteAge)
                                {
                                    pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_FREE;
                                }
                                else
                                {
                                    boolUpdateLog = TRUE32;
                                    pFTLRestoreStruct[wOldVbn].bSpareType = FTL_SPARE_TYPE_FREE;
                                }
                            }
                            /* clean FTL_EXTRA_LOG_CXT */
                            FTL_EXTRA_LOG_CXT->wVbn = 0xFFFF;
                            FTL_EXTRA_LOG_CXT->wLbn = 0xFFFF;
                            _InitLogCxt(FTL_EXTRA_LOG_CXT);
                            _InitLogCxt(&(pstFTLCxt->aLOGCxtTable[dwLogIdx]));
                        }
                        else if (pFTLRestoreStruct[wOldVbn].dwWriteAge < pFTLRestoreStruct[wVBlkIdx].dwWriteAge)
                        {
                            boolUpdateLog = TRUE32;
                            pFTLRestoreStruct[wOldVbn].bSpareType = FTL_SPARE_TYPE_FREE;
                        }
                        else
                        {
                            pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_FREE;
                        }
                        if (boolUpdateLog == FALSE32)
                        {
                            break;
                        }
                    }

                    if (boolUpdateLog == TRUE32)
                    {
                        pstFTLCxt->aLOGCxtTable[dwLogIdx].dwLogAge = pFTLRestoreStruct[wVBlkIdx].dwWriteAge;
                        pstFTLCxt->aLOGCxtTable[dwLogIdx].wLbn = pFTLRestoreStruct[wVBlkIdx].wLbn;
                        pstFTLCxt->aLOGCxtTable[dwLogIdx].wVbn = (UInt16)wVBlkIdx;
                        break;
                    }
                }
            }
        }
    }

    if (pstFTLCxt->aLOGCxtTable[LOG_SECTION_SIZE].wVbn != 0xFFFF)
    {
        UInt16 wLogIdx;
        // case we have all the logs full + one in the middle of a move...
        //
        // look for a log with age matching the age in FTLCxt - mark it as free and copy the spare log to to it

        for (wLogIdx = 0; wLogIdx < (LOG_SECTION_SIZE + 1); wLogIdx++)
        {
            if (pstFTLCxt->aLOGCxtTable[wLogIdx].dwLogAge == pstFTLCxt->dwWriteAge)
            {
                break;
            }
        }

        // if we did not find the right LOG fail _FTLRestore
        if (wLogIdx == (LOG_SECTION_SIZE + 1))
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR]  _FTLRestore - # of LOG blocks > LOG_SECTION_SIZE failing _FTLRestore (line:%d)\n"), __LINE__));
            return FALSE32;
        }

        pFTLRestoreStruct[pstFTLCxt->aLOGCxtTable[wLogIdx].wVbn].bSpareType = FTL_SPARE_TYPE_FREE;
        _CopyLOGCxt(&pstFTLCxt->aLOGCxtTable[wLogIdx], FTL_EXTRA_LOG_CXT);
    }
    /* now we have the the data blocks in the map and log indexes - next recover map locations that point to empty blocks */
    wVBlkIdx = 0; /* check if we need to start elsewhere to leave space for the FTLCxt info */
    for (wLBlkIdx = 0; (wLBlkIdx < USER_SUBLKS_TOTAL) && (wVBlkIdx < FTL_AREA_SIZE); wLBlkIdx++)
    {
        /* check if the map location is free */
        if (pstFTLCxt->pawMapTable[wLBlkIdx] == 0xFFFF)
        {
            do
            {
                if (pFTLRestoreStruct[wVBlkIdx].bSpareType == FTL_SPARE_TYPE_FREE)
                {
                    MODIFY_LB_MAP(wLBlkIdx, wVBlkIdx);
                    pFTLRestoreStruct[wVBlkIdx].wLbn = wLBlkIdx;
                    pFTLRestoreStruct[wVBlkIdx].bSpareType = FTL_SPARE_TYPE_DATA;
                    /* erase blocks that suppose to be empty - case we use blocks that were partially erased or had bad info */
#ifndef AND_READONLY
                    if (FTL_SUCCESS != _EraseAndMarkEC(wVBlkIdx))
                    {
                        FTL_ERR_PRINT((TEXT("[FTL:ERR]  _FTLRestore - failed EraseAndMarkEC in spare pool (line:%d)\n"), __LINE__));
                        return FALSE32;
                    }
#endif
                    wVBlkIdx++;
                    break;
                }
                wVBlkIdx++;
            }
            while (wVBlkIdx < FTL_AREA_SIZE);
        }
    }

    /* last - recover the free vb */
    {
        UInt16 wLogIdx, wFreeIdx;
        UInt16 wFreeVbCounter;
        for (wLogIdx = 0; wLogIdx < LOG_SECTION_SIZE; wLogIdx++)
        {
            if (pstFTLCxt->aLOGCxtTable[wLogIdx].wLbn == 0xFFFF)
            {
                break;
            }
        }

        wFreeVbCounter = 0;
        wVBlkIdx = 0;
        wFreeIdx = wLogIdx;
        /* NirW - write the below code again to make it clearer */
        for (; (wFreeIdx < FREE_SECTION_SIZE) && (wVBlkIdx < FTL_AREA_SIZE); wFreeIdx++)
        {
            do
            {
                if (pFTLRestoreStruct[wVBlkIdx].bSpareType == FTL_SPARE_TYPE_FREE)
                {
                    pstFTLCxt->awFreeVbList[wFreeIdx] = wVBlkIdx;
                    wFreeVbCounter++;
                    wVBlkIdx++;
                    break;
                }
                wVBlkIdx++;
            }
            while (wVBlkIdx < FTL_AREA_SIZE);
        }
        if ((wFreeVbCounter + wLogIdx) != FREE_SECTION_SIZE)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR]  _FTLRestore - not all free vb were located - disk is heading toward corruptions!!!\n")));
            return FALSE32;
        }
        /* mark the rest */
        for (wLogIdx = 0; wLogIdx < (FREE_SECTION_SIZE - wFreeVbCounter); wLogIdx++)
        {
            pstFTLCxt->awFreeVbList[wLogIdx] = 0xFFFF;
        }
        pstFTLCxt->wNumOfFreeVb = wFreeVbCounter;
        pstFTLCxt->wFreeVbListTail = FREE_SECTION_SIZE - wFreeVbCounter;
    }

    /* count!!! - go over the map table and make sure we have everything covered */
    for (wLBlkIdx = 0; wLBlkIdx < USER_SUBLKS_TOTAL; wLBlkIdx++)
    {
        if (pstFTLCxt->pawMapTable[wLBlkIdx] >= FTL_AREA_SIZE)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR]  _FTLRestore - pstFTLCxt->pawMapTable[0x%X] = 0x%X - restore is about to fail (line:%d)!!!\n"), wLBlkIdx, pstFTLCxt->pawMapTable[wLBlkIdx], __LINE__));
            boolRes = FALSE32;
        }
        else if (pFTLRestoreStruct[pstFTLCxt->pawMapTable[wLBlkIdx]].wLbn != wLBlkIdx)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR]  _FTLRestore - pFTLRestoreStruct[pstFTLCxt->pawMapTable[0x%X]].wLbn = 0x%X\n"),
                           wLBlkIdx, pFTLRestoreStruct[pstFTLCxt->pawMapTable[wLBlkIdx]].wLbn));
            FTL_ERR_PRINT((TEXT("[FTL:ERR]  pstFTLCxt->pawMapTable[0x%X] = 0x%X - restore is about to fail (line:%d)!!!\n"),
                           wLBlkIdx, pstFTLCxt->pawMapTable[wLBlkIdx], __LINE__));
            boolRes = FALSE32;
        }
    }

    /* count!!! - go over the VBs and make sure we have everything covered */
    wDataBlkCnt = 0, wLogBlkCnt = 0, wFreeBlkCnt = 0;
    for (wVBlkIdx = 0; (boolRes == TRUE32) && (wVBlkIdx < FTL_AREA_SIZE); wVBlkIdx++)
    {
        switch (pFTLRestoreStruct[wVBlkIdx].bSpareType)
        {
        case FTL_SPARE_TYPE_FREE:
            wFreeBlkCnt++;
            if (_ScanForFreeBlk(wVBlkIdx) == FALSE32)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR]  _FTLRestore - _ScanForFreeBlk(0x%X) failed - restore is about to fail (line:%d)!!!\n"), wVBlkIdx, __LINE__));
                boolRes = FALSE32;
            }
            break;

        case FTL_SPARE_TYPE_LOG:
            {
                LOGCxt * pLOGCxt;
                wLogBlkCnt++;
                pLOGCxt = _ScanLogSection(pFTLRestoreStruct[wVBlkIdx].wLbn);
                if (pLOGCxt == NULL)
                {
                    FTL_ERR_PRINT((TEXT("[FTL:ERR]  _FTLRestore - _ScanLogSection(0x%X) failed - restore is about to fail (line:%d)!!!\n"), wVBlkIdx, __LINE__));
                    boolRes = FALSE32;
                }
                else if (pLOGCxt->wVbn != wVBlkIdx)
                {
                    FTL_ERR_PRINT((TEXT("[FTL:ERR]  _FTLRestore - _ScanLogSection(0x%X) failed - restore is about to fail (line:%d)!!!\n"), wVBlkIdx, __LINE__));
                    boolRes = FALSE32;
                }
                break;
            }

        case FTL_SPARE_TYPE_DATA:
            {
                wDataBlkCnt++;
                if (!IS_LBN_IN_RANGE(pFTLRestoreStruct[wVBlkIdx].wLbn))
                {
                    FTL_ERR_PRINT((TEXT("[FTL:ERR]  _FTLRestore - IS_LBN_IN_RANGE failed - pFTLRestoreStruct[0x%X].wLbn = 0x%X restore is about to fail (line:%d)!!!\n"), wVBlkIdx, pFTLRestoreStruct[wVBlkIdx].wLbn, __LINE__));
                    boolRes = FALSE32;
                }
                else if (pstFTLCxt->pawMapTable[pFTLRestoreStruct[wVBlkIdx].wLbn] != wVBlkIdx)
                {
                    FTL_ERR_PRINT((TEXT("[FTL:ERR]  _FTLRestore - IS_LBN_IN_RANGE failed - pstFTLCxt->pawMapTable[pFTLRestoreStruct[0x%X].wLbn] = 0x%X  .Lbn is 0x%X restore is about to fail (line:%d)!!!\n"),
                                   wVBlkIdx, pstFTLCxt->pawMapTable[pFTLRestoreStruct[wVBlkIdx].wLbn], pFTLRestoreStruct[wVBlkIdx].wLbn, __LINE__));
                    boolRes = FALSE32;
                }
                break;
            }

        case FTL_SPARE_TYPE_CXT:
            break;

        default:
            FTL_ERR_PRINT((TEXT("[FTL:ERR]  _FTLRestore - pFTLRestoreStruct[0x%X].bSpareType = 0x%X (unknown) - restore is about to fail (line:%d)!!!\n"), wVBlkIdx, pFTLRestoreStruct[wVBlkIdx].bSpareType, __LINE__));
            boolRes = FALSE32;
            break;
        }
    }
    if (wDataBlkCnt != USER_SUBLKS_TOTAL || wFreeBlkCnt != pstFTLCxt->wNumOfFreeVb ||
        wLogBlkCnt != (FREE_SECTION_SIZE - pstFTLCxt->wNumOfFreeVb))
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR]  _FTLRestore - wDataBlkCnt = 0x%X, wFreeBlkCnt = 0x%X\n"), wDataBlkCnt, wFreeBlkCnt));
        FTL_ERR_PRINT((TEXT("[FTL:ERR]  _FTLRestore - pstFTLCxt->wNumOfFreeVb = 0x%X, wLogBlkCnt = 0x%X - restore is about to fail (line:%d)!!!\n"), pstFTLCxt->wNumOfFreeVb, wLogBlkCnt, __LINE__));
        boolRes = FALSE32;
    }

    /* look for the latest FTLCxt structure to retrieve EC information */
    {
        UInt16 awSortedCxtVBlk[FTL_CXT_SECTION_SIZE]; /* to be sorted by age */
        UInt16 wTempVBlkIdx1, wTempVBlkIdx2 = 0, wECFoundCounter, wRCFoundCounter, wStatFoundCounter;

        for (wTempVBlkIdx1 = 0; wTempVBlkIdx1 < FTL_CXT_SECTION_SIZE; wTempVBlkIdx1++)
        {
            if (pstFTLCxt->dwCurrMapCxtPage == GET_Vpn(pstFTLCxt->awMapCxtVbn[wTempVBlkIdx1], (PAGES_PER_SUBLK - 1)))
            {
                wTempVBlkIdx2 = wTempVBlkIdx1;
                break;
            }
        }
        wTempVBlkIdx2 += FTL_CXT_SECTION_SIZE;
        for (wTempVBlkIdx1 = 0; wTempVBlkIdx1 < FTL_CXT_SECTION_SIZE; wTempVBlkIdx1++)
        {
            awSortedCxtVBlk[wTempVBlkIdx1] = pstFTLCxt->awMapCxtVbn[((wTempVBlkIdx2 - wTempVBlkIdx1) % FTL_CXT_SECTION_SIZE)];
        }

        /* 0 is the latest cxt info */
        wECFoundCounter = 0;
        wRCFoundCounter = 0;
        wStatFoundCounter = 0;
        for (wTempVBlkIdx1 = 0; (wTempVBlkIdx1 < FTL_CXT_SECTION_SIZE) && ((wECFoundCounter < FTL_NUM_PAGES_PER_EC_TABLE) || (wRCFoundCounter < FTL_NUM_PAGES_PER_RC_TABLE) || (wStatFoundCounter < 1)); wTempVBlkIdx1++)
        {
            for (wTempVBlkIdx2 = PAGES_PER_SUBLK; (wTempVBlkIdx2 != 0) && ((wECFoundCounter < FTL_NUM_PAGES_PER_EC_TABLE) || (wRCFoundCounter < FTL_NUM_PAGES_PER_RC_TABLE) || (wStatFoundCounter < 1));)
            {
                Int32 nVFLRet;

                wTempVBlkIdx2--;

                nVFLRet = VFL_Read(GET_Vpn(awSortedCxtVBlk[wTempVBlkIdx1], wTempVBlkIdx2), pBuff, TRUE32, FALSE32, NULL, NULL, FALSE32);
                if (nVFLRet == VFL_SUCCESS)
                {
                    if (FTL_SPARE_IS_CXT_MARK(GET_FTL_SPARE_TYPE(pBuff->pSpare)))
                    {
                        UInt16 wSpareIndex;
                        UInt32 dwECSpareAge;
                        _GetFTLCxtSpare(&dwECSpareAge, NULL, &wSpareIndex, pBuff->pSpare);
                        if (pstFTLCxt->dwAge > dwECSpareAge)
                        {
                            pstFTLCxt->dwAge = dwECSpareAge;
                            /* we assume this is the last page written - to be on the safe side we will write the next page in a different block */
                        }
                        if (GET_FTL_SPARE_TYPE(pBuff->pSpare) == FTL_SPARE_TYPE_CXT_EC_TABLE && pstFTLCxt->adwECTablePtrs[wSpareIndex] == FTL_TABLE_PTR_INVALID_VALUE)
                        {
#ifndef AND_READONLY
                            UInt16 wBytesToCopy;
                            wECFoundCounter++;
                            pstFTLCxt->adwECTablePtrs[wSpareIndex] = GET_Vpn(awSortedCxtVBlk[wTempVBlkIdx1], wTempVBlkIdx2);
                            wBytesToCopy = WMR_MIN(((sizeof(UInt16) * FTL_AREA_SIZE) - (wSpareIndex * BYTES_PER_PAGE)), BYTES_PER_PAGE);
                            WMR_MEMCPY(&(pstFTLCxt->pawECCacheTable[(wSpareIndex * BYTES_PER_PAGE) / sizeof(UInt16)]), pBuff->pData, wBytesToCopy);
#endif /* #ifndef AND_READONLY */
                        }
                        else if (GET_FTL_SPARE_TYPE(pBuff->pSpare) == FTL_SPARE_TYPE_CXT_RC_TABLE && pstFTLCxt->adwRCTablePtrs[wSpareIndex] == FTL_TABLE_PTR_INVALID_VALUE)
                        {
#ifndef AND_READONLY
                            UInt16 wBytesToCopy;
                            wRCFoundCounter++;
                            pstFTLCxt->adwRCTablePtrs[wSpareIndex] = GET_Vpn(awSortedCxtVBlk[wTempVBlkIdx1], wTempVBlkIdx2);
                            wBytesToCopy = WMR_MIN(((sizeof(UInt16) * FTL_AREA_SIZE) - (wSpareIndex * BYTES_PER_PAGE)), BYTES_PER_PAGE);
                            WMR_MEMCPY(&(pstFTLCxt->pawRCCacheTable[(wSpareIndex * BYTES_PER_PAGE) / sizeof(UInt16)]), pBuff->pData, wBytesToCopy);
#endif /* #ifndef AND_READONLY */
                        }
                        else if (GET_FTL_SPARE_TYPE(pBuff->pSpare) == FTL_SPARE_TYPE_CXT_STAT && pstFTLCxt->adwStatPtrs[0] == FTL_TABLE_PTR_INVALID_VALUE && wSpareIndex == 0)
                        {
#ifdef AND_COLLECT_STATISTICS
                            _FTLSetStatisticsFromCxt(pBuff->pData);
#endif // AND_COLLECT_STATISTICS
                            wStatFoundCounter++;
                            pstFTLCxt->adwStatPtrs[0] = FTL_TABLE_PTR_INVALID_VALUE;
                        }
                    }
                }
            }
        }
#ifndef AND_READONLY
        /* check that we found all the EC pointers */
        if (wECFoundCounter != FTL_NUM_PAGES_PER_EC_TABLE)
        {
            FTL_WRN_PRINT((TEXT("[FTL:WRN]  _FTLRestore - lost part of the EC Table\n")));
        }
        if (wRCFoundCounter != FTL_NUM_PAGES_PER_RC_TABLE)
        {
            FTL_WRN_PRINT((TEXT("[FTL:WRN]  _FTLRestore - lost part of the RC Table\n")));
        }
        if (wStatFoundCounter != 1)
        {
            FTL_WRN_PRINT((TEXT("[FTL:WRN]  _FTLRestore - lost part of the Stat information\n")));
        }
#endif // AND_READONLY
    }

#ifndef AND_READONLY
    {
        UInt16 wIdx;
        for (wIdx = 0; wIdx < pstFTLCxt->wNumOfFreeVb; wIdx++)
        {
            UInt16 wIdxTemp = wIdx + pstFTLCxt->wFreeVbListTail;

            if (wIdxTemp >= FREE_SECTION_SIZE)
            {
                wIdxTemp -= FREE_SECTION_SIZE;
            }

            if (_EraseAndMarkEC(pstFTLCxt->awFreeVbList[wIdxTemp]) != VFL_SUCCESS)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR] _FTLRestore failed _EraseAndMarkEC(0x%X) (line:%d)\n"), pstFTLCxt->awFreeVbList[wIdxTemp], __LINE__));
            }
        }
    }
#endif
    pstFTLCxt->wWearLevelCounter = 0;
    pstFTLCxt->dwReadsSinceLastRefreshMark = 0;

    if (boolRes == TRUE32)
    {
        boolRes = _MakeLogMap();
    }

    // zero-init as if this buffer came from WMR_MALLOC
    WMR_MEMSET(pFTLRestoreStruct, 0, FTL_AREA_SIZE * sizeof(FTLRestoreStruct));
    BUF_Release(pBuff);
    if (boolRes == TRUE32)
    {
        pstFTLCxt->boolFlashCxtIsValid = FALSE32;
#ifndef AND_READONLY
        if (FTL_ShutdownNotify(FALSE32) == FALSE32)
        {
            FTL_WRN_PRINT((TEXT("[FTL:WRN] Failure running FTL_ShutdownNotify after _FTLRestore!\n")));
        }
#endif
    }
    FTL_WRN_PRINT((TEXT("[FTL:WRN] _FTLRestore OK!\n")));
    return boolRes;
}
#endif // #if (defined(WMR_ENABLE_FTL_RECOVERY) && WMR_ENABLE_FTL_RECOVERY)

/*

    NAME
    FTL_Open
    DESCRIPTION
    This function loads ftl context & initialize log context.
    PARAMETERS
    pTotalPages 	[OUT] 	the count of pages which user can use.
    pdwPageSize 	[OUT]	page size in bytes.
    RETURN VALUES
    FTL_SUCCESS
        FTL_Open is completed
    FTL_CRITICAL_ERROR
        FTL_Open is failed
    NOTES
    Exports native page size as file system's sector size (LBA).
    H file with the function name uses sectors rather than pages in
    paramter names.

 */
static Int32
FTL_Open(UInt32 *pTotalPages, UInt32 * pdwPageSize, BOOL32 nandFullRestore,  BOOL32 justFormatted, UInt32 dwMinorVer, UInt32 dwOptions)
{
    FTL_LOG_PRINT((TEXT("[FTL: IN] ++FTL_Open()\n")));

    if (_LoadFTLCxt() == FALSE32)
    {
        FTL_WRN_PRINT((TEXT("[FTL:WRN] Failure running _LoadFTLCxt!\n")));
#ifdef AND_USE_NOTIFY
        {
            LowFuncTbl  *pLowFuncTbl;

            pLowFuncTbl = FIL_GetFuncTbl();
            pLowFuncTbl->Notify(AND_NOTIFY_RECOVER);
        }
#endif /* AND_USE_NOTIFY */
#if !(defined(WMR_DISABLE_FTL_RECOVERY) && WMR_DISABLE_FTL_RECOVERY)
        if (_FTLRestore() == FALSE32)
        {
            /* Always release all buffers before returning to the extrnal caller */
            BUF_ReleaseAllBuffers();
            return FTL_CRITICAL_ERROR;
        }
#else
        FTL_ERR_PRINT((TEXT("[FTL:ERR] Application does not contain FTL_Restore code. Run debug bootloader.\n")));
		return FTL_CRITICAL_ERROR;
#endif // #if (defined(WMR_ENABLE_FTL_RECOVERY) && WMR_ENABLE_FTL_RECOVERY)
    }

    *pTotalPages = USER_PAGES_TOTAL;
    *pdwPageSize = BYTES_PER_PAGE;

    FTL_LOG_PRINT((TEXT("[FTL:OUT] --FTL_Open()\n")));
    /* Always release all buffers before returning to the extrnal caller */
    BUF_ReleaseAllBuffers();
    return FTL_SUCCESS;
}

/*
    NAME
    FTL_Read
    DESCRIPTION
    This function reads pages within user predefined area.
    PARAMETERS
    nLpn 		[IN] 		logical page number.
    nNumOfPages [IN] 		the number of pages to read.
    pBuf 		[IN/OUT] 	buffer for read.
    RETURN VALUES
    FTL_SUCCESS
        FTL_Read is completed
    FTL_CRITICAL_ERROR
        FTL_Read is failed
    FTL_USERDATA_ERROR
        there is an uncorrectable read error on user data
    FTL_OUT_OF_RANGE
        input parameters are invalid
    NOTES
 */
static Int32
FTL_Read(UInt32 dwLpn, UInt32 dwNumOfPages, UInt8 *pBuf)
{
    Int32 nRetValue;

#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwPagesReadCnt += dwNumOfPages;
    stFTLStatistics.ddwFTLReadCnt++;
#endif
    nRetValue = _FTLRead(dwLpn, dwNumOfPages, pBuf);
    if (nRetValue != FTL_SUCCESS)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] failed calling _FTLRead(0x%08X, 0x%08X, 0x%08X)(line:%d)\n"),
                       dwLpn, dwNumOfPages, (UInt32)pBuf, __LINE__));
    }

    // NirW - for now fix right here, we should come with better strategy later
#ifndef AND_READONLY
    if ((pstFTLCxt->dwRefreshListIdx > (FTL_READ_REFRESH_LIST_SIZE >> 1)) ||
        (_IsBlockInRefreshList(0xFFFF, 0xFFFF, NULL) &&
         pstFTLCxt->dwReadsSinceLastRefreshMark > FTL_READS_SINCE_LAST_REFRESH_MARK_TRASHOLD))
    {
        _HandleRefreshBlock();
    }
#endif // ! AND_READONLY
       /* release all buffers as a security measure */
    BUF_ReleaseAllBuffers();

    return nRetValue;
}

static UInt32
_Lbn2Vpn(LOGCxt * pLog, UInt32 dwLbn, UInt16 wLPOffset)
{
    UInt32 dwVpn;

    dwVpn = 0xFFFFFFFF;
    if (pLog != NULL)
    {
        if (pLog->paPOffsetL2P[wLPOffset] != 0xFFFF)
        {
            UInt32 nVPOffset;

            nVPOffset = pLog->paPOffsetL2P[wLPOffset];
            dwVpn = GET_Vpn(pLog->wVbn, nVPOffset);
        }
    }
    if (dwVpn == 0xFFFFFFFF)
    {
        dwVpn = wLPOffset + ((UInt32)CONVERT_L2V(dwLbn) * PAGES_PER_SUBLK);
    }
    return dwVpn;
}

/*
    NAME
    _FTLRead
    DESCRIPTION
    This function reads pages within user predefined area.
    PARAMETERS
    nLpn 		[IN] 		logical page number.
    nNumOfPages [IN] 		the number of pages to read.
    pBuf 		[IN/OUT] 	buffer for read.
    RETURN VALUES
    FTL_SUCCESS
        _FTLRead is completed
    FTL_CRITICAL_ERROR
        _FTLRead is failed
    FTL_USERDATA_ERROR
        there is an uncorrectable read error on user data
    FTL_OUT_OF_RANGE
        input parameters are invalid
    NOTES
 */

static Int32
_FTLRead(UInt32 dwLpn, UInt32 dwNumOfPages, UInt8 *pBuf)
{
    Buffer *pInBuf = NULL;
    LOGCxt *pLog;
    UInt32 dwLbn, dwReadPages = 0, dwVpn;
    UInt16 wLPOffset;
    BOOL32 bLast = FALSE32, bReadErr = FALSE32, boolReadMultiplePagesFailure = FALSE32;
    Int32 nVFLRet;

    FTL_LOG_PRINT((TEXT("[FTL: IN] ++_FTLRead(dwLpn:%d, dwNumOfPages:%d)\n"), dwLpn, dwNumOfPages));

    pstFTLCxt->dwReadsSinceLastRefreshMark++;
    /* input parameter check */
    // check the data buffer is not a NULL pointer
    if (pBuf == NULL)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR]  pBuf == NULL!\n")));
        return FTL_CRITICAL_ERROR;
    }
    // check that the paramters are not zero pages to read or out of range
    if (dwNumOfPages == 0 || (dwLpn + dwNumOfPages) > USER_PAGES_TOTAL)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR]  input parameters are invalid!\n")));
        return FTL_OUT_OF_RANGE_ERROR;
    }

    /* calculate logical block number & page offset */
    dwLbn = dwLpn / PAGES_PER_SUBLK;
    wLPOffset = (UInt16)(dwLpn - dwLbn * PAGES_PER_SUBLK);

    /* initialize interal buffer */
    pInBuf = BUF_Get(BUF_MAIN_AND_SPARE);

    if (pInBuf == NULL)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] BUF_Get error (line:%d)!\n"), __LINE__));
        return FTL_CRITICAL_ERROR;
    }

    /* find log block if there is */
    pLog = _ScanLogSection(dwLbn);

    for (;;)
    {
        if (boolReadMultiplePagesFailure == FALSE32)
        {
            BOOL32 boolReadRes;
            UInt16 wPagesReadThisTime = WMR_MIN((UInt16)(dwNumOfPages - dwReadPages), (UInt16)(PAGES_PER_SUBLK - wLPOffset));

            // check if either the log or the data block went beyond exceptable read counter
#ifndef AND_READONLY
            if (pstFTLCxt->pawRCCacheTable[CONVERT_L2V(dwLbn)] >= FTL_RC_TRESHOLD)
            {
                _AddLbnToRefreshList((UInt16)dwLbn, CONVERT_L2V(dwLbn));
            }
            if (pLog != NULL)
            {
                if (pstFTLCxt->pawRCCacheTable[pLog->wVbn] >= FTL_RC_TRESHOLD)
                {
                    _AddLbnToRefreshList((UInt16)dwLbn, pLog->wVbn);
                }
            }
#endif // AND_READONLY
            if (pLog == NULL)
            {
                BOOL32 boolAddVbToRefreshList;
                // count reads per bank
#ifndef AND_READONLY
                pstFTLCxt->pawRCCacheTable[CONVERT_L2V(dwLbn)] += ((wPagesReadThisTime / BANKS_TOTAL) + ((wPagesReadThisTime % BANKS_TOTAL) ? 1 : 0));
#endif /* #ifndef AND_READONLY */
                if (NULL != VFL_ReadMultiplePagesInVb)
                {
                    boolReadRes = VFL_ReadMultiplePagesInVb(CONVERT_L2V(dwLbn), wLPOffset, wPagesReadThisTime,
                                                            &pBuf[dwReadPages * BYTES_PER_PAGE], pstbaMultiplePagesWriteSpare, &boolAddVbToRefreshList, NULL, FALSE32);
                }
                else
                {
                    UInt16 wPageIdx;
                    for (wPageIdx = 0; wPageIdx < wPagesReadThisTime; wPageIdx++)
                    {
                        pstadwScatteredReadVpn[wPageIdx] = GET_Vpn(CONVERT_L2V(dwLbn), wLPOffset + wPageIdx);
                    }
                    boolReadRes = VFL_ReadScatteredPagesInVb(pstadwScatteredReadVpn, wPagesReadThisTime,
                                                            &pBuf[dwReadPages * BYTES_PER_PAGE], pstbaMultiplePagesWriteSpare, &boolAddVbToRefreshList, NULL, FALSE32, NULL);
                }
                if (boolAddVbToRefreshList == TRUE32)
                {
                    FTL_WRN_PRINT((TEXT("[FTL:WRN] _AddLbnToRefreshList (0x%X, 0x%X) (line:%d)!\n"), dwLbn, CONVERT_L2V(dwLbn), __LINE__));
#ifndef AND_READONLY
                    _AddLbnToRefreshList((UInt16)dwLbn, CONVERT_L2V(dwLbn));
#endif // ! AND_READONLY
                }
            }
            else
            {
                BOOL32 boolAddVbToRefreshList;
                UInt16 wPageIdx;
                for (wPageIdx = 0; wPageIdx < wPagesReadThisTime; wPageIdx++)
                {
                    pstadwScatteredReadVpn[wPageIdx] = _Lbn2Vpn(pLog, dwLbn, (wLPOffset + wPageIdx));
#ifndef AND_READONLY
                    if (GET_Vbn(pstadwScatteredReadVpn[wPageIdx]) == pLog->wVbn)
                    {
                        pstFTLCxt->pawRCCacheTable[pLog->wVbn]++;
                    }
                    else
                    {
                        pstFTLCxt->pawRCCacheTable[pstFTLCxt->pawMapTable[dwLbn]]++;
                    }
#endif /* #ifndef AND_READONLY */
                }

                boolReadRes = VFL_ReadScatteredPagesInVb(pstadwScatteredReadVpn,
                                                         wPagesReadThisTime,
                                                         &pBuf[dwReadPages * BYTES_PER_PAGE], pstbaMultiplePagesWriteSpare, &boolAddVbToRefreshList, NULL, FALSE32, NULL);

                if (boolAddVbToRefreshList == TRUE32)
                {
                    // this should not be main stream case - no big deal if we mark both blocks
                    // to be refreshed
                    FTL_WRN_PRINT((TEXT("[FTL:WRN] _AddLbnToRefreshList (0x%X, 0x%X, 0x%X) (line:%d)!\n"), dwLbn, CONVERT_L2V(dwLbn), pLog->wVbn, __LINE__));
#ifndef AND_READONLY
                    _AddLbnToRefreshList((UInt16)dwLbn, CONVERT_L2V(dwLbn));
                    _AddLbnToRefreshList((UInt16)dwLbn, pLog->wVbn);
#endif // ! AND_READONLY
                }
            }

            if (boolReadRes == TRUE32)
            {
                UInt16 wPageIdx;

                for (wPageIdx = 0; wPageIdx < wPagesReadThisTime; wPageIdx++)
                {
                    if (CHECK_FTL_ECC_MARK(&pstbaMultiplePagesWriteSpare[(wPageIdx * BYTES_PER_METADATA_RAW)]) == FALSE32)
                    {
                        ANDAddressStruct andAddressStruct;
                        UInt32 dwSize = sizeof(andAddressStruct);
                        FTL_ERR_PRINT((TEXT("[FTL:ERR] _FTLRead ECC Mark lbn 0x%X, offset 0x%X, pIdx 0x%X mark 0x%08X (line:%d)!\n"), dwLbn, wLPOffset, wPageIdx,
                                       GET_FTL_ECC_MARK(&pstbaMultiplePagesWriteSpare[(wPageIdx * BYTES_PER_METADATA_RAW)]), __LINE__));
                        andAddressStruct.dwLpn = GET_Vpn(dwLbn, (wLPOffset + wPageIdx));
                        FTL_GetStruct(AND_STRUCT_FTL_GETADDRESS, &andAddressStruct, &dwSize);
                        FTL_ERR_PRINT((TEXT("[FTL:ERR] address l:0x%08X v:0x%08X cs:0x%02X p:0x%08X (line:%d)!\n"),
                                       andAddressStruct.dwLpn, andAddressStruct.dwVpn, andAddressStruct.dwCS, andAddressStruct.dwPpn, __LINE__));
                        bReadErr = TRUE32; /* identify pages that were copied from pages with bad ecc */
                    }
                }
                dwReadPages += wPagesReadThisTime;
                dwLpn += wPagesReadThisTime;
                wLPOffset += wPagesReadThisTime;
                if (dwReadPages == dwNumOfPages)
                {
                    bLast = TRUE32;
                }
                else
                {
                    bLast = FALSE32;
                }
            }
            else
            {
                if (pLog != NULL)
                {
                    FTL_WRN_PRINT((TEXT("[FTL:WRN] VFL_ReadScatteredPagesInVb failed (0x%X, 0x%X, 0x%X); will retry with multiple VFL_Read calls (line:%d)!\n"),
                                   dwLbn, CONVERT_L2V(dwLbn), pLog->wVbn, __LINE__));
                }
                else
                {
                    FTL_WRN_PRINT((TEXT("[FTL:WRN] VFL_ReadMultiplePagesInVb failed (0x%X, 0x%X); will retry with multiple VFL_Read calls (line:%d)!\n"),
                                   dwLbn, CONVERT_L2V(dwLbn), __LINE__));
                }
                boolReadMultiplePagesFailure = TRUE32;
            }
        }
        if (!((dwReadPages == dwNumOfPages) || (wLPOffset == PAGES_PER_SUBLK)))
        {
            UInt8 * pabOrgData;
            BOOL32 boolAddVbToRefreshList;

            /* if log block is allocated */
            dwVpn = _Lbn2Vpn(pLog, dwLbn, wLPOffset);

            /* check last read case */
            if (dwReadPages + 1 == dwNumOfPages)
            {
                bLast = TRUE32;
            }
            else
            {
                bLast = FALSE32;
            }

            pabOrgData = pInBuf->pData;
            pInBuf->pData = (pBuf + dwReadPages * BYTES_PER_PAGE);

            nVFLRet = VFL_Read(dwVpn, pInBuf, TRUE32, TRUE32, &boolAddVbToRefreshList, NULL, FALSE32);
            pInBuf->pData = pabOrgData;
#ifndef AND_READONLY
            if (boolAddVbToRefreshList == TRUE32)
            {
                UInt16 wVbn = (UInt16)GET_Vbn(dwVpn);
                FTL_WRN_PRINT((TEXT("[FTL:WRN] _AddLbnToRefreshList (0x%X, 0x%X) (line:%d)!\n"), (UInt16)dwLbn, wVbn, __LINE__));
                _AddLbnToRefreshList((UInt16)dwLbn, wVbn);
            }
#endif // ! AND_READONLY

            if (nVFLRet == VFL_CRITICAL_ERROR)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on VFL_Read(line:%d)!\n"), __LINE__));
                BUF_Release(pInBuf);
                return FTL_CRITICAL_ERROR;
            }

            if (nVFLRet == VFL_U_ECC_ERROR || (CHECK_FTL_ECC_MARK(pInBuf->pSpare) == FALSE32))
            {
                ANDAddressStruct andAddressStruct;
                UInt32 dwSize = sizeof(andAddressStruct);
                FTL_ERR_PRINT((TEXT("[FTL:ERR] Real ECC ERROR VFL_Read(line:%d) ECC mark is 0x%02X!\n"), __LINE__, GET_FTL_ECC_MARK(pInBuf->pSpare)));
                andAddressStruct.dwLpn = GET_Vpn(dwLbn, wLPOffset);
                FTL_GetStruct(AND_STRUCT_FTL_GETADDRESS, &andAddressStruct, &dwSize);
                FTL_ERR_PRINT((TEXT("[FTL:ERR] address l:0x%08X v:0x%08X cs:0x%02X p:0x%08X (line:%d)!\n"),
                               andAddressStruct.dwLpn, andAddressStruct.dwVpn, andAddressStruct.dwCS, andAddressStruct.dwPpn, __LINE__));

                if (pLog != NULL)
                {
                    FTL_ERR_PRINT((TEXT("[FTL:ERR]  lbn 0x%X pLog->wVbn 0x%X dataVbn 0x%X offset 0x%X vpn 0x%X!\n"),
                                   dwLbn, pLog->wVbn, CONVERT_L2V(dwLbn), wLPOffset, _Lbn2Vpn(pLog, dwLbn, wLPOffset)));
                }
                else
                {
                    FTL_ERR_PRINT((TEXT("[FTL:ERR]  lbn 0x%X dataVbn 0x%X offset 0x%X vpn 0x%X!\n"),
                                   dwLbn, CONVERT_L2V(dwLbn), wLPOffset, _Lbn2Vpn(pLog, dwLbn, wLPOffset)));
                }
                bReadErr = TRUE32; /* identify pages that were copied from pages with bad ecc */
            }

            if (nVFLRet == VFL_SUCCESS)
            {
                UInt32 dwWrittenLpn;
                _GetLOGCxtSpare(pInBuf->pSpare, &dwWrittenLpn, NULL);
                if (dwWrittenLpn != dwLpn)
                {
                    FTL_ERR_PRINT((TEXT("[FTL:ERR]  VFL_Read (dwWrittenLpn(0x%X) != dwLpn(0x%X))!\n"), dwWrittenLpn, dwLpn));
                }
            }

            dwLpn++;
            dwReadPages++;
            wLPOffset++;
            if (dwReadPages == dwNumOfPages)
            {
                bLast = TRUE32;
            }
        }

        if (bLast)
        {
            break;
        }

        if (wLPOffset == PAGES_PER_SUBLK)
        {
            dwLbn++;
            if ((UInt16)dwLbn >= USER_SUBLKS_TOTAL)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on VFL_Read(line:%d)!\n"), __LINE__));
                BUF_Release(pInBuf);
                return FTL_CRITICAL_ERROR;
            }
            wLPOffset = 0;
            pLog = _ScanLogSection(dwLbn);
            boolReadMultiplePagesFailure = FALSE32;
        }
    }

    BUF_Release(pInBuf);

    FTL_LOG_PRINT((TEXT("[FTL:OUT] --FTL_Read(dwLpn:%d, dwNumOfPages:%d)\n"), dwLpn, dwNumOfPages));

    if (bReadErr)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] _FTLRead returns FTL_USERDATA_ERROR (line:%d)!\n"), __LINE__));
        return FTL_USERDATA_ERROR;
    }
    else
    {
        return FTL_SUCCESS;
    }
}
#ifndef AND_READONLY
static BOOL32 _FTLMarkCxtNotValid(void)
{
    if (pstFTLCxt->boolFlashCxtIsValid == TRUE32)
    {
        Int32 nRetVal;
        UInt8 bTryIdx = 4;
        while (--bTryIdx)
        {
            /* invalidate the cache */
            Buffer * pBuf;

            pBuf = BUF_Get(BUF_MAIN_AND_SPARE);
            if (pBuf == NULL)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR] BUF_Get error (line:%d)!\n"), __LINE__));
                return FALSE32;
            }
            if (_PrepareNextMapCxtPage() == FALSE32)
            {
                BUF_Release(pBuf);
                return FALSE32;
            }
            /* set the data area */
            WMR_MEMSET(pBuf->pData, 0xFF, BYTES_PER_PAGE);
            /* set the spare */
            _SetFTLCxtSpare(&pstFTLCxt->dwAge, NULL, NULL, pBuf->pSpare);
            SET_FTL_SPARE_TYPE(pBuf->pSpare, FTL_SPARE_TYPE_CXT_INVALID);
            nRetVal = VFL_Write(pstFTLCxt->dwCurrMapCxtPage, pBuf, TRUE32, FALSE32);
            BUF_Release(pBuf);
            if (nRetVal == VFL_SUCCESS)
            {
                pstFTLCxt->boolFlashCxtIsValid = FALSE32;
                break;
            }
            // write fail therefore we have to change context blocks
            FTL_CHANGE_FTLCXT_TO_NEXT_BLOCK();
        }
        if (bTryIdx == 0)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR] Error invalidating the Index (line:%d)!\n"), __LINE__));
            return FALSE32;
        }
    }
    return TRUE32;
}

/*

    NAME
    FTL_Write
    DESCRIPTION
    This function writes pages within user predefined area.
    PARAMETERS
    nLpn 		[IN] 		logical page number.
    nNumOfPages [IN] 		the number of pages to read.
    pBuf 		[IN] 		buffer for write.
    RETURN VALUES
    FTL_SUCCESS
        FTL_Write is completed
    FTL_CRITICAL_ERROR
        FTL_Write is failed
    FTL_OUT_OF_RANGE
        input parameters are invalid
    NOTES

 */
static Int32
FTL_Write(UInt32 dwLpn, UInt32 dwNumOfPages, UInt8 *pabDataBuf, BOOL32 isStatic)
{
    LOGCxt *pLog = NULL;
    UInt16 wLbn;
    UInt16 wLPOffset;
    UInt32 dwWritePages = 0;
    UInt32 dwRemainPages;

    FTL_LOG_PRINT((TEXT("[FTL: IN] ++FTL_Write(dwLpn:%d, dwNumOfPages:%d)\n"), dwLpn, dwNumOfPages));
    // isStatic is not supported
    WMR_ASSERT(!isStatic);

#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwPagesWrittenCnt += dwNumOfPages;
    stFTLStatistics.ddwFTLWriteCnt++;
#endif
    if (pabDataBuf == NULL)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR]  pabDataBuf == NULL!\n")));
        return FTL_CRITICAL_ERROR;
    }

    /* Sanity check for arguments */
    if (dwNumOfPages == 0 || (dwLpn + dwNumOfPages) > USER_PAGES_TOTAL)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR]  input parameters are invalid!\n")));
        return FTL_OUT_OF_RANGE_ERROR;
    }

    // Step A:	Check if pstFTLCxt is flash valid (i.e. first write operation since flush)
    //			If this is the first write, mark the pstFTLCxt in the flash as invalid.
    if (_FTLMarkCxtNotValid() == FALSE32)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] FTL_Write failed calling _FTLMarkCxtNotValid\n")));
        return FTL_CRITICAL_ERROR;
    }
    // Step B: Prepare variables before getting into the write loop
    wLbn = (UInt16)(dwLpn / PAGES_PER_SUBLK);
    wLPOffset = (UInt16)(dwLpn - wLbn * PAGES_PER_SUBLK);

    dwRemainPages = dwNumOfPages;

    if ((pLog = _PrepareLog(wLbn)) == NULL)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on _PrepareLog!\n")));
        BUF_ReleaseAllBuffers();
        return FTL_CRITICAL_ERROR;
    }

    // Step C: run the write loop until all pages are written to the flash
    for (;;)
    {
        BOOL32 boolFullLBWrite = FALSE32;

        // if the log block is full merge it - if it is released create another log block...
        // the code should be changed so:
        // 1. we first check for the case of full vb write operation.
        // 2. we check for the case of not enough space in the log here -
        //		case there is not enough space we should merge the relevant info to a new log block
        //		then copy the new info.
        if (pLog->wFreePOffset == PAGES_PER_SUBLK)
        {
            /* if log block is full */
            if (_Merge(pLog) == FALSE32)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on _Merge!\n")));
                BUF_ReleaseAllBuffers();
                return FTL_CRITICAL_ERROR;
            }

            /* check if the log has a free block attached to it */
            if (pLog->wVbn == LOG_EMPTY_SLOT)
            {
                UInt16 wVbnForLog;
                /* set a new free block to log after merge operation*/
                if (_GetFreeVb(&wVbnForLog) == FALSE32)
                {
                    FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on _GetFreeVb (line:%d)!\n"), __LINE__));
                    BUF_ReleaseAllBuffers();
                    return FTL_CRITICAL_ERROR;
                }
                ASSIGN_VBN_TO_LOG(pLog, wVbnForLog);

                _InitLogCxt(pLog);
            }
        }

        /* check if we can do multiple pages write operation - do we have more that BANKS_TOTAL to write in the current logical block */
        /*
            we want to use this function in one of two cases:
            1. we have a write operation for a whole vb.
            2. we have a write operation for a log that have enough free space.
         */
        if ((dwRemainPages >= PAGES_PER_SUBLK) && (wLPOffset == 0))
        {
            boolFullLBWrite = TRUE32;
        }

        {
            UInt16 wLPOffsetIdx;
            UInt16 wDesVbn;
            UInt16 wVPOffset;
            UInt16 wNumOfPagesToWrite = (UInt16)WMR_MIN(((UInt32)(PAGES_PER_SUBLK - wLPOffset)), dwRemainPages);
            if (boolFullLBWrite == FALSE32)
            {
                wNumOfPagesToWrite = WMR_MIN((PAGES_PER_SUBLK - pLog->wFreePOffset), wNumOfPagesToWrite);
            }

            /* fill up the spare data */
            wDesVbn = pLog->wVbn;
            wVPOffset = pLog->wFreePOffset;
            if (boolFullLBWrite == TRUE32 && pLog->wFreePOffset != 0)
            {
                if (_GetFreeVb(&wDesVbn) == FALSE32)
                {
                    FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on _GetFreeVb (line:%d)!\n"), __LINE__));
                    BUF_ReleaseAllBuffers();
                    return FTL_CRITICAL_ERROR;
                }
                wVPOffset = 0;
            }
            if (wVPOffset == 0)
            {
                pstFTLCxt->dwWriteAge++;
            }
            for (wLPOffsetIdx = 0; wLPOffsetIdx < wNumOfPagesToWrite; wLPOffsetIdx++)
            {
                UInt8 * pbaTempSpare = &(pstbaMultiplePagesWriteSpare[wLPOffsetIdx * BYTES_PER_METADATA_RAW]);
                _SetLOGCxtSpare(GET_Vpn(pLog->wLbn, (wLPOffset + wLPOffsetIdx)), pstFTLCxt->dwWriteAge, pbaTempSpare);
            }

            /* change the spare type from log to data if relevant */
            if ((wLPOffset + wLPOffsetIdx) == PAGES_PER_SUBLK)
            {
                UInt8 * pbaTempSpare = &(pstbaMultiplePagesWriteSpare[(--wLPOffsetIdx) * BYTES_PER_METADATA_RAW]);
                if (boolFullLBWrite == TRUE32)
                {
                    SET_FTL_SPARE_TYPE(pbaTempSpare, FTL_SPARE_TYPE_DATA);
                }
                else if (pLog->boolCopyMerge == TRUE32 && pLog->wFreePOffset == wLPOffset && pLog->wFreePOffset == pLog->wNumOfValidLP)
                {
                    SET_FTL_SPARE_TYPE(pbaTempSpare, FTL_SPARE_TYPE_DATA);
                }
            }

            if (VFL_WriteMultiplePagesInVb(GET_Vpn(wDesVbn, wVPOffset), wNumOfPagesToWrite, (pabDataBuf + (dwWritePages * BYTES_PER_PAGE)), pstbaMultiplePagesWriteSpare, TRUE32, FALSE32) == FALSE32)
            {
                if (wDesVbn == pLog->wVbn)
                {
                    pLog->wFreePOffset = PAGES_PER_SUBLK;
                    pLog->boolCopyMerge = FALSE32;
                }
                else
                {
                    if (_SetFreeVb(wDesVbn) == FALSE32)
                    {
                        FTL_ERR_PRINT((TEXT("[FTL:ERR] FTL_Write failed _SetFreeVb(0x%X) (line:%d)\n"), wDesVbn, __LINE__));
                    }
                }
                continue;
            }
            else
            {
                if (wDesVbn != pLog->wVbn)
                {
                    /* this is a full block write to free block - release the log */
                    if (_SetFreeVb(pLog->wVbn) == FALSE32)
                    {
                        FTL_ERR_PRINT((TEXT("[FTL:ERR] FTL_Write failed _SetFreeVb(0x%X) (line:%d)\n"), pLog->wVbn, __LINE__));
                    }
                    if (_SetFreeVb(CONVERT_L2V(pLog->wLbn)) == FALSE32)
                    {
                        FTL_ERR_PRINT((TEXT("[FTL:ERR] FTL_Write failed _SetFreeVb(0x%X) (line:%d)\n"), CONVERT_L2V(pLog->wLbn), __LINE__));
                    }
                    MODIFY_LB_MAP(pLog->wLbn, wDesVbn);
                    MARK_LOG_AS_EMPTY(pLog);
                    wLPOffset = PAGES_PER_SUBLK;
                }
                else
                {
                    for (wLPOffsetIdx = 0; wLPOffsetIdx < wNumOfPagesToWrite; wLPOffsetIdx++, wLPOffset++)
                    {
                        if (pLog->paPOffsetL2P[wLPOffset] == 0xFFFF)
                        {
                            pLog->wNumOfValidLP++;
                        }
                        else
                        {
                            pLog->boolCopyMerge = FALSE32;
                        }
                        pLog->paPOffsetL2P[wLPOffset] = pLog->wFreePOffset++;
                    }
                    if (pLog->wFreePOffset != pLog->wNumOfValidLP || pLog->wNumOfValidLP != wLPOffset)
                    {
                        pLog->boolCopyMerge = FALSE32;
                    }
                }
                dwWritePages += wNumOfPagesToWrite;
                dwRemainPages -= wNumOfPagesToWrite;
                wLPOffset--;
            }
        }
        /* NirW - check if boolCopyMerge == TRUE32 - if it does take the log out for the log list and mark the original unit as free */
        if (pLog->wFreePOffset == PAGES_PER_SUBLK && pLog->boolCopyMerge == TRUE32)
        {
            _Merge(pLog);
        }

        if (dwRemainPages == 0)
        {
            break;
        }

        if (++wLPOffset == PAGES_PER_SUBLK)
        {
            if (++wLbn == USER_SUBLKS_TOTAL)
            {
                break;
            }

            if ((pLog = _PrepareLog(wLbn)) == NULL) /* crossing to the next block */
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR]  there is error on _PrepareLog!\n")));
                BUF_ReleaseAllBuffers();
                return FTL_CRITICAL_ERROR;
            }

            wLPOffset = 0;
        }
    }

    if (pstFTLCxt->dwRefreshListIdx > (FTL_READ_REFRESH_LIST_SIZE >> 1))
    {
        _HandleRefreshBlock();
    }
    else if (pstFTLCxt->wWearLevelCounter >= WMR_WEAR_LEVEL_FREQUENCY_DELAY)
    {
        UInt8 bTrialIdx;
        pstFTLCxt->wWearLevelCounter -= WMR_WEAR_LEVEL_FREQUENCY;
        for (bTrialIdx = 0; bTrialIdx < 4; bTrialIdx++)
        {
            if (_AutoWearLevel() == TRUE32)
            {
                break;
            }
        }
    }

    FTL_LOG_PRINT((TEXT("[FTL:OUT] --FTL_Write(dwLpn:%d, dwNumOfPages:%d)\n"), dwLpn, dwNumOfPages));

    /* Always release all buffers before returning to the extrnal caller */
    BUF_ReleaseAllBuffers();
    return FTL_SUCCESS;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      FTL_Format			                                                 */
/* DESCRIPTION                                                               */
/*      This function formats ftl.											 */
/* PARAMETERS                                                                */
/*		none																 */
/* RETURN VALUES                                                             */
/*		FTL_SUCCESS															 */
/*				FTL_Format is completed										 */
/*		FTL_CRITICAL_ERROR													 */
/*				FTL_Format is failed										 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static Int32
FTL_Format(UInt32 dwOptions)
{
    UInt16 wIdx;
    Int32 nFTLRet = FTL_SUCCESS;
    UInt32 dwFTLType;

    FTL_LOG_PRINT((TEXT("[FTL: IN] ++FTL_Format()\n")));
	
    for (wIdx = 0; wIdx < FTL_AREA_SIZE; wIdx++)
    {
        VFL_Erase(wIdx, TRUE32);
    }

    /* init EC table */
    WMR_MEMSET(pstFTLCxt->pawECCacheTable, 0, (FTL_AREA_SIZE * sizeof(UInt16)));
    /* init RC table */
    WMR_MEMSET(pstFTLCxt->pawRCCacheTable, 0, (FTL_AREA_SIZE * sizeof(UInt16)));

    /* initialize ftl context (free virtual block list) */
    for (wIdx = 0; wIdx < FREE_SECTION_SIZE; wIdx++)
    {
        pstFTLCxt->awFreeVbList[wIdx] = wIdx + FREE_SECTION_START;
        if (_EraseAndMarkEC(wIdx + FREE_SECTION_START) != VFL_SUCCESS)
        {
            FTL_ERR_PRINT((TEXT("[FTL:ERR] FTL_Format failed _EraseAndMarkEC(0x%X) (line:%d)\n"), (wIdx + FREE_SECTION_START), __LINE__));
        }
    }

    pstFTLCxt->wNumOfFreeVb = FREE_SECTION_SIZE;
    pstFTLCxt->wFreeVbListTail = 0;

    /* init logs table */
    for (wIdx = 0; wIdx < LOG_SECTION_SIZE; wIdx++)
    {
        pstFTLCxt->aLOGCxtTable[wIdx].wVbn = LOG_EMPTY_SLOT;
    }

    /* init map table */
    for (wIdx = 0; wIdx < USER_SUBLKS_TOTAL; wIdx++)
    {
        pstFTLCxt->pawMapTable[wIdx] = (UInt16)(DATA_SECTION_START + wIdx);
    }

    /* init read reclaim info - NirW - to be added */

    /* init the Cxt pointers */
    for (wIdx = 0; wIdx < FTL_CXT_SECTION_SIZE; wIdx++)
    {
        pstFTLCxt->awMapCxtVbn[wIdx] = wIdx;
    }
    pstFTLCxt->dwCurrMapCxtPage = GET_Vpn(pstFTLCxt->awMapCxtVbn[0], (PAGES_PER_SUBLK - 1));

    pstFTLCxt->dwAge = 0xFFFFFFFF;
    pstFTLCxt->dwWriteAge = 0; /* NirW */

    pstFTLCxt->wWearLevelCounter = 0;
    pstFTLCxt->dwMetaWearLevelCounter = 0;

    // init the refresh block information
    WMR_MEMSET(pstFTLCxt->aFTLReadRefreshList, 0xFF,
               (sizeof(FTLReadRefreshStruct) * FTL_READ_REFRESH_LIST_SIZE));
    pstFTLCxt->dwRefreshListIdx = 0;
    pstFTLCxt->dwReadsSinceLastRefreshMark = 0;

    if (VFL_ChangeFTLCxtVbn(pstFTLCxt->awMapCxtVbn) != VFL_SUCCESS)
    {
        return FTL_CRITICAL_ERROR;
    }
    {
        UInt8 bTryStoreFTLCxt = 3;
        while (_StoreFTLCxt() == FALSE32 && --bTryStoreFTLCxt)
        {
            /* make sure that the next time we try it is in a different block */
            FTL_CHANGE_FTLCXT_TO_NEXT_BLOCK();
        }
        if (bTryStoreFTLCxt == 0)
        {
            nFTLRet = FTL_CRITICAL_ERROR;
        }
    }

    dwFTLType = FTL_TYPE_FTL;
    VFL_SetStruct(AND_STRUCT_VFL_FTLTYPE, &dwFTLType, sizeof(UInt32));

    FTL_LOG_PRINT((TEXT("[FTL: IN] --FTL_Format()\n")));

    return nFTLRet;
}
#endif // AND_READONLY
static BOOL32 _InitDeviceInfo(void)
{
    stFTLDeviceInfo.wPagesPerVb = (UInt16)VFL_GetDeviceInfo(AND_DEVINFO_PAGES_PER_SUBLK);
    stFTLDeviceInfo.wUserVbTotal =
        (UInt16)VFL_GetDeviceInfo(AND_DEVINFO_NUM_OF_USER_SUBLK) -
    (FTL_CXT_SECTION_SIZE + FREE_SECTION_SIZE);
    stFTLDeviceInfo.wBytesPerPage = (UInt16)VFL_GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    stFTLDeviceInfo.wNumOfBanks = (UInt16)VFL_GetDeviceInfo(AND_DEVINFO_NUM_OF_BANKS);
    stFTLDeviceInfo.dwUserPagesTotal =
        (UInt32)stFTLDeviceInfo.wUserVbTotal * (UInt32)stFTLDeviceInfo.wPagesPerVb;
    return TRUE32;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      FTL_Init			                                                 */
/* DESCRIPTION                                                               */
/*      This function initializes ftl context.								 */
/* PARAMETERS                                                                */
/*		none																 */
/* RETURN VALUES                                                             */
/*		FTL_SUCCESS															 */
/*				FTL_Init is completed										 */
/*		FTL_CRITICAL_ERROR													 */
/*				FTL_Init is failed											 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static Int32
FTL_Init(VFLFunctions *pVFLFunctions)
{
    UInt16 wLogIdx;

    WMR_MEMCPY(&stVFLFunctions, pVFLFunctions, sizeof(VFLFunctions));
    FTL_LOG_PRINT((TEXT("[FTL: IN] ++FTL_Init()\n")));

    _InitDeviceInfo();
    // needed to verify that the index information will not cross two super blocks
    WMR_ASSERT(FTL_NUM_PAGES_TO_WRITE_IN_STORECXT < PAGES_PER_SUBLK);
    WMR_ASSERT((PAGES_PER_SIMPLE_MERGE_BUFFER << 1) < PAGES_PER_SUBLK); // using pstadwScatteredReadVpn for both read and write in _DoMoveMerge
#ifdef AND_COLLECT_STATISTICS
    WMR_ASSERT(sizeof(FTLStatistics) < AND_STATISTICS_SIZE_PER_LAYER);
    WMR_MEMSET(&stFTLStatistics, 0, sizeof(FTLStatistics));
#endif // AND_COLLECT_STATISTICS
       /* init pstFTLCxt */
    if (pstFTLCxt == NULL)
    {
        pstFTLMeta = (FTLMeta *)WMR_MALLOC(sizeof(FTLMeta));
        if (pstFTLMeta == NULL)
        {
            return FTL_CRITICAL_ERROR;
        }
        pstFTLCxt = &pstFTLMeta->stFTLCxt;
        WMR_MEMSET(pstFTLMeta->abReserved, 0, ((BYTES_PER_SECTOR - 2) * sizeof(UInt32)) - sizeof(FTLCxt2));
        pstFTLCxt->boolFlashCxtIsValid = FALSE32; /* make sure the cache is marked as invalid until loading */

        pstFTLCxt->pawMapTable = (UInt16 *)WMR_MALLOC(sizeof(UInt16) * USER_SUBLKS_TOTAL); /* covers lb area (user available logical blocks) */
        /* added 1 LOGCxt for _DoMoveMerge */
        pstFTLCxt->pawLOGCxtMapTable = (UInt16 *)WMR_MALLOC((LOG_SECTION_SIZE + 1) * PAGES_PER_SUBLK * sizeof(UInt16)); /* covers log tables area (+ offsets array) */
#ifndef AND_READONLY
        pstFTLCxt->pawECCacheTable = (UInt16 *)WMR_MALLOC(FTL_AREA_SIZE * sizeof(UInt16)); /* covers vb area available to FTL - data and tables (log and FTLCtx) */
        pstFTLCxt->pawRCCacheTable = (UInt16 *)WMR_MALLOC(FTL_AREA_SIZE * sizeof(UInt16)); /* covers vb area available to FTL - data and tables (log and FTLCtx) */
#endif
#if (defined(AND_READONLY) && AND_READONLY) && defined(WMR_MAX_READONLY_PAGES)
        pstbaMultiplePagesWriteSpare = (UInt8 *)WMR_MALLOC(WMR_MAX_READONLY_PAGES * BYTES_PER_METADATA_RAW);
        pstadwScatteredReadVpn = (UInt32 *)WMR_MALLOC(WMR_MAX_READONLY_PAGES * sizeof(UInt32));
#else
        pstbaMultiplePagesWriteSpare = (UInt8 *)WMR_MALLOC(PAGES_PER_SUBLK * BYTES_PER_METADATA_RAW);
        pstadwScatteredReadVpn = (UInt32 *)WMR_MALLOC(PAGES_PER_SUBLK * sizeof(UInt32));
#endif

        // Buffer allocation
#if !(defined(WMR_DISABLE_FTL_RECOVERY) && WMR_DISABLE_FTL_RECOVERY) || \
    !(defined(AND_READONLY) && AND_READONLY)
        WMR_BufZone_Init(&FTL_BufZone);
        pstSimpleMergeDataBuffer = (UInt8*)WMR_Buf_Alloc_ForDMA(&FTL_BufZone, PAGES_PER_SIMPLE_MERGE_BUFFER * BYTES_PER_PAGE);
        WMR_BufZone_FinishedAllocs(&FTL_BufZone);
        WMR_BufZone_Rebase(&FTL_BufZone, (void**)&pstSimpleMergeDataBuffer);
        WMR_BufZone_FinishedRebases(&FTL_BufZone);
#endif

        if (pstFTLCxt->pawMapTable == NULL || pstFTLCxt->pawLOGCxtMapTable == NULL ||
            pstbaMultiplePagesWriteSpare == NULL 
#ifndef AND_READONLY
            || pstFTLCxt->pawRCCacheTable == NULL || pstFTLCxt->pawECCacheTable == NULL 
#endif
#if !(defined(WMR_DISABLE_FTL_RECOVERY) && WMR_DISABLE_FTL_RECOVERY) || \
    !(defined(AND_READONLY) && AND_READONLY)
            || pstSimpleMergeDataBuffer == NULL
#endif
            )
        {
            if (pstFTLCxt->pawMapTable != NULL)
            {
                WMR_FREE(pstFTLCxt->pawMapTable, (sizeof(UInt16) * USER_SUBLKS_TOTAL));
            }
            if (pstFTLCxt->pawLOGCxtMapTable != NULL)
            {
                WMR_FREE(pstFTLCxt->pawLOGCxtMapTable, ((LOG_SECTION_SIZE + 1) * PAGES_PER_SUBLK * sizeof(UInt16)));
            }
            if (pstFTLCxt->pawECCacheTable != NULL)
            {
                WMR_FREE(pstFTLCxt->pawECCacheTable, (FTL_AREA_SIZE * sizeof(UInt16)));
            }
            if (pstFTLCxt->pawRCCacheTable != NULL)
            {
                WMR_FREE(pstFTLCxt->pawECCacheTable, (FTL_AREA_SIZE * sizeof(UInt16)));
            }
            if (pstFTLCxt != NULL)
            {
                WMR_FREE(pstFTLCxt, (sizeof(FTLCxt2)));
            }
            if (pstbaMultiplePagesWriteSpare != NULL)
            {
                WMR_FREE(pstbaMultiplePagesWriteSpare, (PAGES_PER_SUBLK * BYTES_PER_METADATA_RAW));
            }
#ifndef AND_READONLY
            WMR_BufZone_Free(&FTL_BufZone);
#endif
            return FTL_CRITICAL_ERROR;
        }
        /* init the pointers to the log arrays */
        for (wLogIdx = 0; wLogIdx < (LOG_SECTION_SIZE + 1); wLogIdx++)
        {
            pstFTLCxt->aLOGCxtTable[wLogIdx].paPOffsetL2P = &pstFTLCxt->pawLOGCxtMapTable[PAGES_PER_SUBLK * wLogIdx];
            _InitLogCxt(&pstFTLCxt->aLOGCxtTable[wLogIdx]);
        }
    }
    FTL_LOG_PRINT((TEXT("[FTL:OUT] --FTL_Init()\n")));

    return FTL_SUCCESS;
}
#ifndef AND_READONLY
/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      FTL_GarbageCollect	                                                 */
/* DESCRIPTION                                                               */
/*      This function call _Merge(NULL) function.							 */
/* PARAMETERS                                                                */
/*		none																 */
/* RETURN VALUES                                                             */
/*		TRUE32																 */
/*				FTL_GarbageCollect is success.								 */
/*		FALSE32																 */
/*				FTL_GarbageCollect is failed.								 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
static BOOL32
FTL_GarbageCollect(void)
{
    UInt32 nIdx;
    LOGCxt *pLog;
    BOOL32 bLogExist = FALSE32;

    for (nIdx = 0; nIdx < LOG_SECTION_SIZE; nIdx++)
    {
        pLog = GET_LOGCxt(nIdx);

        if (pLog->wVbn != LOG_EMPTY_SLOT)
        {
            bLogExist = TRUE32;
            break;
        }
    }

    if (bLogExist == TRUE32)
    {
        return _Merge(NULL);
    }

    return TRUE32;
}

static BOOL32 FTL_ShutdownNotify(BOOL32 boolMergeLogs)
{
    BOOL32 boolStat = TRUE32, boolRetTemp;
    UInt16 wIdx;

    if (pstFTLCxt->boolFlashCxtIsValid == TRUE32)
    {
        return TRUE32;
    }

#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwFlushCnt++;
#endif
    if (boolMergeLogs == TRUE32)
    {
        for (wIdx = 0; wIdx < LOG_SECTION_SIZE; wIdx++)
        {
            LOGCxt * pLog = GET_LOGCxt(wIdx);

            if (pLog->wVbn != LOG_EMPTY_SLOT)
            {
                pstFTLCxt->wWearLevelCounter++;
                if (pLog->boolCopyMerge == TRUE32)
                {
                    boolRetTemp = _DoCopyMerge(pLog);
                }
                else
                {
                    boolRetTemp = _DoSimpleMerge(pLog);
                }
                if (boolRetTemp == FALSE32)
                {
                    boolStat = boolRetTemp;
                }
            }
        }
    }
    if (boolStat == TRUE32)
    {
        UInt8 bTryStoreFTLCxt = 5;
        while (_StoreFTLCxt() == FALSE32 && --bTryStoreFTLCxt)
        {
            /* make sure that the next time we try it is in a different block */
            FTL_CHANGE_FTLCXT_TO_NEXT_BLOCK();
        }
        if (bTryStoreFTLCxt == 0)
        {
            boolStat = FALSE32;
        }
    }
    /* Always release all buffers before returning to the extrnal caller */
    BUF_ReleaseAllBuffers();
    return boolStat;
}
#endif // AND_READONLY

#ifdef AND_COLLECT_STATISTICS
static BOOL32 FTL_ECBins(void * pvoidStructBuffer, UInt32 * pdwStructSize)
{
    BOOL32 boolRes = FALSE32;
    UInt16 blockIdx;
    FTLBinsStruct hdr;
    const UInt32 kdwStructSize = sizeof(hdr) + sizeof(*hdr.usage) * FTL_NUM_EC_BINS;

    if (!pstFTLCxt || !pstFTLCxt->pawECCacheTable)
    {
        return FALSE32;
    }

    if (pvoidStructBuffer && pdwStructSize && (*pdwStructSize >= kdwStructSize))
    {
        const UInt32 max_bin_val = FTL_MAX_EC_BIN_VAL;
        const UInt32 bin_size = FTL_EC_BIN_SIZE;
        UInt32 size = *pdwStructSize; // only used for WMR_FILL_STRUCT

        hdr.maxValue = max_bin_val;
        hdr.binCount = FTL_NUM_EC_BINS;
        WMR_MEMSET(pvoidStructBuffer, 0, kdwStructSize);
        WMR_FILL_STRUCT(pvoidStructBuffer, &size, &hdr, sizeof(hdr));

        for (blockIdx = 0; blockIdx < FTL_AREA_SIZE; ++blockIdx)
        {
            UInt32 index;
            UInt16 usage;
            void *cursor;

            if (pstFTLCxt->pawECCacheTable[blockIdx] >= max_bin_val)
            {
                index = FTL_NUM_EC_BINS - 1;
            }
            else
            {
                index = pstFTLCxt->pawECCacheTable[blockIdx] / bin_size;
            }

            cursor = ((char *)pvoidStructBuffer) + WMR_OFFSETOF(FTLBinsStruct, usage[index]);
            WMR_MEMCPY(&usage, cursor, sizeof(usage));
            usage++;
            WMR_MEMCPY(cursor, &usage, sizeof(usage));
        }
        boolRes = TRUE32;
    }

    if (pdwStructSize)
    {
        *pdwStructSize = kdwStructSize;
        boolRes = TRUE32;
    }

    return boolRes;
}

static BOOL32 FTL_RCBins(void * pvoidStructBuffer, UInt32 * pdwStructSize)
{
    BOOL32 boolRes = FALSE32;
    UInt16 blockIdx;
    FTLBinsStruct hdr;
    const UInt32 kdwStructSize = sizeof(hdr) + sizeof(*hdr.usage) * FTL_NUM_RC_BINS;

    if (!pstFTLCxt || !pstFTLCxt->pawRCCacheTable)
    {
        return FALSE32;
    }

    if (pvoidStructBuffer && pdwStructSize && (*pdwStructSize >= kdwStructSize))
    {
        const UInt32 max_bin_val = FTL_RC_TRESHOLD;
        const UInt32 bin_size = FTL_RC_BIN_SIZE(max_bin_val);
        UInt32 size = *pdwStructSize; // only used for WMR_FILL_STRUCT

        hdr.maxValue = max_bin_val;
        hdr.binCount = FTL_NUM_RC_BINS;
        WMR_MEMSET(pvoidStructBuffer, 0, kdwStructSize);
        WMR_FILL_STRUCT(pvoidStructBuffer, &size, &hdr, sizeof(hdr));

        for (blockIdx = 0; blockIdx < FTL_AREA_SIZE; ++blockIdx)
        {
            UInt32 index;
            UInt16 usage;
            void *cursor;

            if (pstFTLCxt->pawRCCacheTable[blockIdx] >= max_bin_val)
            {
                index = FTL_NUM_RC_BINS - 1;
            }
            else
            {
                index = pstFTLCxt->pawRCCacheTable[blockIdx] / bin_size;
            }

            cursor = ((char *)pvoidStructBuffer) + WMR_OFFSETOF(FTLBinsStruct, usage[index]);
            WMR_MEMCPY(&usage, cursor, sizeof(usage));
            usage++;
            WMR_MEMCPY(cursor, &usage, sizeof(usage));
        }
        boolRes = TRUE32;
    }

    if (pdwStructSize)
    {
        *pdwStructSize = kdwStructSize;
        boolRes = TRUE32;
    }

    return boolRes;
}
#endif //AND_COLLECT_STATISTICS
static BOOL32 FTL_GetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 * pdwStructSize)
{
    BOOL32 boolRes = FALSE32;

    switch (dwStructType)
    {
    case AND_STRUCT_FTL_GET_TYPE:
        {
            UInt8 bFTLType = FTL_TYPE_FTL;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &bFTLType, sizeof(bFTLType));
            break;
        }

    case AND_STRUCT_FTL_GETADDRESS:
        {
            ANDAddressStruct stAddr;
            UInt16 wLbn, wPOffset;
            UInt32 dwAddrSize = sizeof(stAddr);
            LOGCxt * pLog;
            
            if (pvoidStructBuffer && pdwStructSize && (*pdwStructSize >= sizeof(stAddr)))
            {
                WMR_MEMCPY(&stAddr, pvoidStructBuffer, sizeof(stAddr));

                wLbn = (UInt16)GET_Vbn(stAddr.dwLpn);
                pLog = _ScanLogSection(wLbn);
                wPOffset = (UInt16)GET_PageOffset(stAddr.dwLpn);
                stAddr.dwVpn = _Lbn2Vpn(pLog, wLbn, wPOffset);
                boolRes = VFL_GetStruct(AND_STRUCT_VFL_GETADDRESS, &stAddr, &dwAddrSize);
                
                if (boolRes)
                {
                    WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &stAddr, sizeof(stAddr));
                }
            }
            else if (pdwStructSize)
            {
                *pdwStructSize = sizeof(ANDAddressStruct);
            }
            break;
        }

#ifdef AND_COLLECT_STATISTICS
    case AND_STRUCT_FTL_GET_FTL_STATS_SIZE:
        {
            UInt32 dwStatsSize = sizeof(FTLStatistics);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwStatsSize, sizeof(dwStatsSize));
            break;
        }

    case AND_STRUCT_FTL_STATISTICS:
        {
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &stFTLStatistics, sizeof(stFTLStatistics));
            break;
        }
#endif // AND_COLLECT_STATISTICS

    case AND_STRUCT_FTL_GET_FTL_DEVICEINFO_SIZE:
        {
            UInt32 dwDeviceInfoSize = sizeof(FTLWMRDeviceInfo);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwDeviceInfoSize, sizeof(dwDeviceInfoSize));
            break;
        }

    case AND_STRUCT_FTL_GET_FTL_DEVICEINFO:
        {
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &stFTLDeviceInfo, sizeof(stFTLDeviceInfo));
            break;
        }

    case AND_STRUCT_FTL_FTLCXT:
        {
            if (pstFTLCxt)
            {
                boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, pstFTLCxt, sizeof(*pstFTLCxt));
            }
            break;
        }

    case AND_STRUCT_FTL_MAPTABLE:
        {
            if (pstFTLCxt && pstFTLCxt->pawMapTable)
            {
                const UInt32 dwMapTableSize = sizeof(UInt16) * USER_SUBLKS_TOTAL;
                boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, pstFTLCxt->pawMapTable, dwMapTableSize);
            }
            break;
        }

    case AND_STRUCT_FTL_LOGCXTTABLE:
        {
            if (pstFTLCxt && pstFTLCxt->pawLOGCxtMapTable)
            {
                const UInt32 dwLogMapSize = (LOG_SECTION_SIZE + 1) * PAGES_PER_SUBLK * sizeof(UInt16);
                boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, pstFTLCxt->pawLOGCxtMapTable, dwLogMapSize);
            }
            break;
        }

    case AND_STRUCT_FTL_ECTABLE:
        {
            if (pstFTLCxt && pstFTLCxt->pawECCacheTable)
            {
                const UInt32 dwECTableSize = FTL_AREA_SIZE * sizeof(UInt16);
                boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, pstFTLCxt->pawECCacheTable, dwECTableSize);
            }
            break;
        }

    case AND_STRUCT_FTL_RCTABLE:
        {
            if (pstFTLCxt && pstFTLCxt->pawRCCacheTable)
            {
                const UInt32 dwRCTableSize = FTL_AREA_SIZE * sizeof(UInt16);
                boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, pstFTLCxt->pawRCCacheTable, dwRCTableSize);
            }
            break;
        }
#ifdef AND_COLLECT_STATISTICS
    case AND_STRUCT_FTL_GET_FTL_EC_BINS:
        {
            boolRes = FTL_ECBins(pvoidStructBuffer, pdwStructSize);
            break;
        }
            
    case AND_STRUCT_FTL_GET_FTL_RC_BINS:
        {
            boolRes = FTL_RCBins(pvoidStructBuffer, pdwStructSize);
            break;
        }
#endif            
    default:
        FTL_WRN_PRINT((TEXT("[FTL:WRN]  FTL_GetStruct 0x%X is not identified as FTL data struct identifier!\n"), dwStructType));
        boolRes = FALSE32;
    }
    return boolRes;
}

// Added close to enable better testing on the simulator (no need to stop the application to reload AND).
// this code does not touch the NAND itself (just release memory, and clean pointers and statics).
static void FTL_Close(void)
{
    if (pstFTLCxt != NULL)
    {
        WMR_FREE(pstFTLCxt->pawMapTable, (sizeof(UInt16) * USER_SUBLKS_TOTAL));
        WMR_FREE(pstFTLCxt->pawLOGCxtMapTable, ((LOG_SECTION_SIZE + 1) * PAGES_PER_SUBLK * sizeof(UInt16)));
#ifndef AND_READONLY
        WMR_FREE(pstFTLCxt->pawECCacheTable, (FTL_AREA_SIZE * sizeof(UInt16)));
        WMR_FREE(pstFTLCxt->pawRCCacheTable, (FTL_AREA_SIZE * sizeof(UInt16)));
#endif /* #ifndef AND_READONLY */
        WMR_FREE(pstFTLCxt, sizeof(FTLMeta));
        WMR_FREE(pstbaMultiplePagesWriteSpare, (PAGES_PER_SUBLK * BYTES_PER_METADATA_RAW));
        WMR_FREE(pstadwScatteredReadVpn, (PAGES_PER_SUBLK * sizeof(UInt32)));

#ifndef AND_READONLY
        WMR_BufZone_Free(&FTL_BufZone);
#endif

        pstFTLCxt = NULL;
        pstbaMultiplePagesWriteSpare = NULL;
#if !(defined(WMR_DISABLE_FTL_RECOVERY) && WMR_DISABLE_FTL_RECOVERY) || \
    !(defined(AND_READONLY) && AND_READONLY)
        pstSimpleMergeDataBuffer = NULL;
#endif
        pstadwScatteredReadVpn = NULL;
#ifdef AND_COLLECT_STATISTICS
        WMR_MEMSET(&stFTLStatistics, 0, sizeof(FTLStatistics));
#endif
    }
}

#ifndef AND_READONLY
static BOOL32 _IsBlockInRefreshList(UInt16 wLbn, UInt16 wVbn, UInt32 * pdwIdx)
{
    BOOL32 boolRes = FALSE32;
    UInt32 dwIdx;

    if (pstFTLCxt->dwRefreshListIdx == 0)
    {
        return FALSE32;
    }
    else if (wLbn == 0xFFFF && wVbn == 0xFFFF)
    {
        return TRUE32;
    }

    for (dwIdx = 0; dwIdx < pstFTLCxt->dwRefreshListIdx && boolRes == FALSE32; dwIdx++)
    {
        // if the wVbn == 0xFFFF, find any matching wLbn
        if (wVbn == 0xFFFF)
        {
            if (pstFTLCxt->aFTLReadRefreshList[dwIdx].wLbn == wLbn)
            {
                boolRes = TRUE32;
            }
        }
        else if (wLbn == 0xFFFF)
        {
            if (pstFTLCxt->aFTLReadRefreshList[dwIdx].wVbn == wVbn)
            {
                boolRes = TRUE32;
            }
        }
        else if (pstFTLCxt->aFTLReadRefreshList[dwIdx].wLbn == wLbn &&
                 pstFTLCxt->aFTLReadRefreshList[dwIdx].wVbn == wVbn)
        {
            boolRes = TRUE32;
        }
    }
    if (pdwIdx != NULL)
    {
        *pdwIdx = dwIdx;
    }
    return boolRes;
}
static void _RemoveBlockFromRefreshList(UInt16 wLbn, UInt16 wVbn)
{
    UInt32 dwIdx = 0;

    if ((0xFFFF == wLbn) && (0xFFFF == wVbn))
    {
        return;
    }
    if (_IsBlockInRefreshList(wLbn, wVbn, &dwIdx) == FALSE32)
    {
        return;
    }

    do
    {
        if (dwIdx == pstFTLCxt->dwRefreshListIdx - 1)
        {
            pstFTLCxt->dwRefreshListIdx--;
            pstFTLCxt->aFTLReadRefreshList[dwIdx].wLbn = 0xFFFF;
            pstFTLCxt->aFTLReadRefreshList[dwIdx].wVbn = 0xFFFF;
        }
        else
        {
            pstFTLCxt->dwRefreshListIdx--;
            pstFTLCxt->aFTLReadRefreshList[dwIdx].wLbn =
                pstFTLCxt->aFTLReadRefreshList[pstFTLCxt->dwRefreshListIdx].wLbn;
            pstFTLCxt->aFTLReadRefreshList[dwIdx].wVbn =
                pstFTLCxt->aFTLReadRefreshList[pstFTLCxt->dwRefreshListIdx].wVbn;
            pstFTLCxt->aFTLReadRefreshList[pstFTLCxt->dwRefreshListIdx].wLbn = 0xFFFF;
            pstFTLCxt->aFTLReadRefreshList[pstFTLCxt->dwRefreshListIdx].wVbn = 0xFFFF;
        }
    }
    while (_IsBlockInRefreshList(wLbn, wVbn, &dwIdx) == TRUE32);
}

static BOOL32 _HandleRefreshBlock(void)
{
    UInt16 wFreeVbn, wTempWord, wLbn, wVbn;
    LOGCxt * pLOGCxt;
    BOOL32 boolFoundMatch = FALSE32;

    // sanity check
    if (pstFTLCxt->dwRefreshListIdx == 0)
    {
        return TRUE32;
    }

    if (_FTLMarkCxtNotValid() == FALSE32)
    {
        FTL_ERR_PRINT((TEXT("[FTL:ERR] __HandleRefreshBlock failed calling _FTLMarkCxtNotValid\n")));
        return FALSE32;
    }

    wLbn = pstFTLCxt->aFTLReadRefreshList[0].wLbn;
    wVbn = pstFTLCxt->aFTLReadRefreshList[0].wVbn;

    // check if the information in the list is still relevant
    pLOGCxt = _ScanLogSection(wLbn);
    wTempWord = pstFTLCxt->pawMapTable[wLbn];
    if (wTempWord == wVbn)
    {
        boolFoundMatch = TRUE32;
    }
    else if (pLOGCxt != NULL)
    {
        if (pLOGCxt->wVbn == wVbn)
        {
            boolFoundMatch = TRUE32;
        }
    }

    if (boolFoundMatch == FALSE32)
    {
        FTL_WRN_PRINT((TEXT("[FTL:WRN] info in the refresh list seems to be out of date - removing from list (0x%X, 0x%X) (line:%d)\n"),
                       wLbn, wVbn, __LINE__));
        _RemoveBlockFromRefreshList(wLbn, wVbn);
        return TRUE32;
    }

    if (pstFTLCxt->boolFlashCxtIsValid == TRUE32)
    {
        pstFTLCxt->boolFlashCxtIsValid = FALSE32;
    }
    if (pLOGCxt != NULL)
    {
        BOOL32 boolRes;
        // this block has an active log - do simple merge here
        boolRes = _DoSimpleMerge(pLOGCxt);
        if (boolRes == TRUE32)
        {
            _RemoveBlockFromRefreshList(wLbn, 0xFFFF);
            return TRUE32;
        }
        FTL_WRN_PRINT((TEXT("[FTL:WRN] _HandleRefreshBlock failed _DoSimpleMerge (line:%d)\n"), __LINE__));
        return FALSE32;
    }

    // this block does not have log - use move
    if (_GetFreeVb(&wFreeVbn) == FALSE32)
    {
        FTL_WRN_PRINT((TEXT("[FTL:WRN] _HandleRefreshBlock failed _GetFreeVb (line:%d)\n"), __LINE__));
        return FALSE32;
    }

    if (wFreeVbn != 0xFFFF)
    {
        BOOL32 boolRes;
        boolRes = _MoveD2F(wLbn, wFreeVbn);

        if (boolRes == FALSE32)
        {
            FTL_WRN_PRINT((TEXT("[FTL:WRN] _HandleRefreshBlock failed _MoveD2F (line:%d)\n"), __LINE__));
            if (_SetFreeVb(wFreeVbn) == FALSE32)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR] _HandleRefreshBlock failed _SetFreeVb(0x%X) (line:%d)\n"), wFreeVbn, __LINE__));
            }
            return FALSE32;
        }
        else
        {
            _RemoveBlockFromRefreshList(wLbn, 0xFFFF);
            if (_SetFreeVb(wVbn) == FALSE32)
            {
                FTL_ERR_PRINT((TEXT("[FTL:ERR] _HandleRefreshBlock failed _SetFreeVb(0x%X) (line:%d)\n"), wVbn, __LINE__));
            }
            MODIFY_LB_MAP(wLbn, wFreeVbn);
            return TRUE32;
        }
    }
    return FALSE32;
}

static BOOL32 _AddLbnToRefreshList(UInt16 wLbn, UInt16 wVbn)
{
    UInt32 dwIdx;

    if (_IsBlockInRefreshList(wLbn, wVbn, &dwIdx) == TRUE32)
    {
        return TRUE32;
    }

    pstFTLCxt->dwReadsSinceLastRefreshMark = 0;
    if (pstFTLCxt->dwRefreshListIdx == FTL_READ_REFRESH_LIST_SIZE)
    {
        FTL_WRN_PRINT((TEXT("[FTL:WRN]  _AddLbnToRefreshList(0x%X, 0x%X) failed adding new block to list - list is full!!! (line:%d)!\n"),
                       wLbn, wVbn, __LINE__));
        return FALSE32;
    }

    if (pstFTLCxt->dwRefreshListIdx == FTL_READ_REFRESH_LIST_SIZE)
    {
        FTL_WRN_PRINT((TEXT("[FTL:WRN]  _AddLbnToRefreshList(0x%X, 0x%X) failed calling (line:%d)!\n"),
                       wLbn, wVbn, __LINE__));
        return FALSE32;
    }
    pstFTLCxt->aFTLReadRefreshList[pstFTLCxt->dwRefreshListIdx].wLbn = wLbn;
    pstFTLCxt->aFTLReadRefreshList[pstFTLCxt->dwRefreshListIdx].wVbn = wVbn;
    pstFTLCxt->dwRefreshListIdx++;
#ifdef AND_COLLECT_STATISTICS
    stFTLStatistics.ddwReadDisturbHandleCall++;
#endif /* AND_COLLECT_STATISTICS */
    return TRUE32;
}
#endif // AND_READONLY

#ifdef AND_COLLECT_STATISTICS
static void _UpdateStatisticsCounters(UInt8 *pabSrc, void *pvoidStat, UInt32 dwSize)
{
    UInt32 dwIdx;
    UInt64 * paddwStat = (UInt64 *)pvoidStat;

    for (dwIdx = 0; dwIdx < (dwSize / sizeof(UInt64)); dwIdx++)
    {
        UInt64 ddwTemp;
        WMR_MEMCPY(&ddwTemp, &pabSrc[dwIdx * sizeof(UInt64)], sizeof(UInt64));
        paddwStat[dwIdx] += ddwTemp;
    }
}

#ifndef AND_READONLY
static BOOL32 _FTLGetStatisticsToCxt(UInt8 * pabData)
{
    UInt32 dwStatBuffSize;
    UInt32 dwStatVersion = AND_EXPORT_STRUCTURE_VERSION;
    UInt8  *pabCursor = pabData;
    UInt8  *pabVersion = &pabData[BYTES_PER_PAGE - sizeof(UInt32)];
    

    WMR_MEMSET(pabCursor, 0, BYTES_PER_PAGE);

    WMR_MEMCPY(pabCursor, &stFTLStatistics, sizeof(stFTLStatistics));
    pabCursor += AND_STATISTICS_SIZE_PER_LAYER;
    
    dwStatBuffSize = AND_STATISTICS_SIZE_PER_LAYER;
    VFL_GetStruct(AND_STRUCT_VFL_STATISTICS, pabCursor, &dwStatBuffSize);
    pabCursor += AND_STATISTICS_SIZE_PER_LAYER;

    dwStatBuffSize = AND_STATISTICS_SIZE_PER_LAYER;
    VFL_GetStruct(AND_STRUCT_VFL_FILSTATISTICS, pabCursor, &dwStatBuffSize);

    // mark the statistics version
    WMR_MEMCPY(pabVersion, &dwStatVersion, sizeof(UInt32));
    return TRUE32;
}
#endif // ! AND_READONLY
static BOOL32 _FTLSetStatisticsFromCxt(UInt8 * pabData)
{
    UInt32 dwStatBuffSize;
    Buffer *pBuff = BUF_Get(BUF_MAIN_AND_SPARE);
    UInt8  *pabCursor;
    if (!pBuff)
    {
        FTL_ERR_PRINT((TEXT("BUF_Get failed in _FTLSetStatisticsFromCxt!\n")));
        return FALSE32;
    }

    pabCursor = pabData;
    
    _UpdateStatisticsCounters(pabCursor, &stFTLStatistics, sizeof(FTLStatistics));
    pabCursor += AND_STATISTICS_SIZE_PER_LAYER;

    dwStatBuffSize = AND_STATISTICS_SIZE_PER_LAYER;
    VFL_GetStruct(AND_STRUCT_VFL_STATISTICS, pBuff->pData, &dwStatBuffSize);
    _UpdateStatisticsCounters(pabCursor, pBuff->pData, dwStatBuffSize);
    pabCursor += AND_STATISTICS_SIZE_PER_LAYER;
    VFL_SetStruct(AND_STRUCT_VFL_STATISTICS, pBuff->pData, dwStatBuffSize);

    dwStatBuffSize = AND_STATISTICS_SIZE_PER_LAYER;
    VFL_GetStruct(AND_STRUCT_VFL_FILSTATISTICS, pBuff->pData, &dwStatBuffSize);
    _UpdateStatisticsCounters(pabCursor, pBuff->pData, dwStatBuffSize);
    VFL_SetStruct(AND_STRUCT_VFL_FILSTATISTICS, pBuff->pData, dwStatBuffSize);

    BUF_Release(pBuff);
    return TRUE32;
}
#endif // AND_COLLECT_STATISTICS
void FTL_Register(FTLFunctions * pFTLFunctions)
{
    pFTLFunctions->Init = FTL_Init;
    pFTLFunctions->Open = FTL_Open;
    pFTLFunctions->Read = FTL_Read;
    pFTLFunctions->Close = FTL_Close;
    pFTLFunctions->GetStruct = FTL_GetStruct;

#ifndef AND_READONLY
    pFTLFunctions->Write = FTL_Write;
    pFTLFunctions->Unmap = NULL;
    pFTLFunctions->Format = FTL_Format;
    pFTLFunctions->WearLevel = FTL_WearLevel;
    pFTLFunctions->GarbageCollect = FTL_GarbageCollect;
    pFTLFunctions->ShutdownNotify = FTL_ShutdownNotify;
    pFTLFunctions->WriteStats = NULL;
#endif
}

#endif //AND_SUPPORT_LEGACY_FTL
