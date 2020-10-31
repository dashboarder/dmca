/*****************************************************************************/
/*                                                                           */
/* COMPONENT   : Rainbow                                                     */
/* MODULE      : Example                                                     */
/* NAME        : Example                                                     */
/* FILE        : WMRExam.c                                                   */
/* PURPOSE     : the example for whimory initialization                      */
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
/*   12-SEP-2005 [Jaesung Jung] : first writing                              */
/*   15-MAR-2006 [Yangsup Lee ] : bad block detection bug fix                */
/*                                                                           */
/*****************************************************************************/
#if (!defined(AND_FPART_ONLY) || !AND_FPART_ONLY)
#include "ANDTypes.h"
#include "WMRConfig.h"
#include "WMROAM.h"
#include "VFLBuffer.h"
#include "FPart.h"
#include "VFL.h"
#include "FTL.h"
#include "FIL.h"
#include "WMRExam.h"
#include "FPartTypes.h"
#include "ANDStats.h"
#include "WMRTest.h"
#else
#include "ANDTypes.h"
#include "WMRConfig.h"
#include "WMROAM.h"
#include "VFLBuffer.h"
#include "FPart.h"
#include "FIL.h"
#include "WMRExam.h"
#include "FPartTypes.h"
#endif

/*****************************************************************************/
/* Debug Print #defines                                                      */
/*****************************************************************************/
#define     EXAM_INF_MSG_ON

#define     EXAM_ERR_PRINT(x)            WMR_RTL_PRINT(x)
#define     EXAM_RTL_PRINT(x)            WMR_RTL_PRINT(x)

#if defined (EXAM_LOG_MSG_ON)
#define     EXAM_LOG_PRINT(x)            WMR_DBG_PRINT(x)
#else
#define     EXAM_LOG_PRINT(x)
#endif

#if defined (EXAM_INF_MSG_ON)
#define     EXAM_INF_PRINT(x)            WMR_DBG_PRINT(x)
#else
#define     EXAM_INF_PRINT(x)
#endif

#if (!defined(AND_FPART_ONLY) || !AND_FPART_ONLY)
static ANDDriverSignature g_flashANDSignature; // Keep what was read from flash
static ANDDriverSignature g_scratchANDSignature; // For writing new signatures
static VFLFunctions stVflFunctions;
static FTLFunctions stFtlFunctions;
#endif
static FPartFunctions stFPartFunctions;
static BOOL32 boolFPartInitialized = FALSE32;

/*****************************************************************************/
/* Code Implementation                                                       */
/*****************************************************************************/

static Int32 _initFPart(UInt32 dwFormatType, UInt32 dwBootAreaSize)
{
    UInt32 dwFPartOptions = 0;
    Int32 nRet;

    if (FALSE32 == boolFPartInitialized) 
    {
        WMR_MEMSET(&stFPartFunctions, 0, sizeof(FPartFunctions));
		FPart_Register(&stFPartFunctions);    
		dwFPartOptions = 0;
		dwFPartOptions |= (dwFormatType & WMR_INIT_USE_DEV_UNIQUE_INFO) ? FPART_INIT_OPTION_DEV_UNIQUE : 0;
		dwFPartOptions |= (dwFormatType & WMR_INIT_USE_DIAG_CTRL_INFO) ? FPART_INIT_OPTION_DIAG_CTRL : 0;
		nRet = stFPartFunctions.Init(FIL_GetFuncTbl(), dwFPartOptions);
		if (nRet != TRUE32)
		{
			return WMR_CRITICAL_ERROR;
		}
		boolFPartInitialized = TRUE32;
    }
	return WMR_SUCCESS;
}

Int32
WMR_PreInit(UInt32 dwFormatType, UInt32 dwBootAreaSize)
{
    Int32 nRet;

    OAM_Init();

    nRet = FIL_Init();
    if (nRet != FIL_SUCCESS)
    {
        return WMR_CRITICAL_ERROR;
    }
    if (0 == FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_NUM_OF_CS)) 
    {
        EXAM_RTL_PRINT((TEXT("[FTL:MSG] No NAND attached\n")));
        return WMR_CRITICAL_ERROR;
    }

    nRet = BUF_Init((UInt16)FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE), 
                    BYTES_PER_METADATA_RAW, 
                    WMR_NUM_BUFFERS);
    if (nRet != BUF_SUCCESS)
    {
        return WMR_CRITICAL_ERROR;
    }
    
    nRet = _initFPart(dwFormatType, dwBootAreaSize);
    if (nRet != WMR_SUCCESS)
    {
        return WMR_CRITICAL_ERROR;
    }

    return WMR_SUCCESS;
}

FPartFunctions * WMR_GetFPart(void)
{
    return &stFPartFunctions;
}

#if (!defined(AND_FPART_ONLY) || !AND_FPART_ONLY)

static BOOL32 _isSignatureCompatible(ANDDriverSignature *pstSignature)
{
    const UInt8 bPlatform = WMR_SIGNATURE_PLATFORM(pstSignature->dwMainSignature);
    const UInt8 bLayout   = WMR_SIGNATURE_LAYOUT(pstSignature->dwMainSignature);
    const UInt8 bVFLType  = WMR_SIGNATURE_VFL(pstSignature->dwMainSignature);
    const UInt32 dw2ndId  = WMR_SIGNATURE_FORMATTING_VERSION(pstSignature->dw2ndId);
    if ( WMR_PLATFORM_DARWIN != bPlatform)
    {
        return FALSE32;
    }
    
    if ( (WMR_LAYOUT_SIMPLE != bLayout) && (WMR_LAYOUT_VS != bLayout) )
    {
        return FALSE32;
    }
    
    if ( (WMR_M68_VFL != bVFLType) && (WMR_VSVFL != bVFLType) )
    {
        return FALSE32;
    }
    
    // Only VS layout supports 2ndID field
    if ((WMR_LAYOUT_VS == bLayout) && (dw2ndId > WMR_FORMATTING_VERSION))
    {
        return FALSE32;
    }

    return TRUE32;
}

static BOOL32 _selectVFL(BOOL32 boolSignatureFound,
                         BOOL32 boolAllowVSVFL,
                         BOOL32 boolSignInBlockZero,
                         ANDDriverSignature * pOldANDDriverSignature,
                         ANDDriverSignature * pNewANDDriverSignature)
{
    // decide what VFL to use
    if (((boolSignatureFound == TRUE32) &&
         WMR_SIGNATURE_VFL(pOldANDDriverSignature->dwMainSignature) == WMR_VSVFL)
#ifndef AND_READONLY
        || ((boolSignatureFound == FALSE32) && (boolAllowVSVFL == TRUE32))
#endif
        )

    {
        VSVFL_Register(&stVflFunctions);
        pNewANDDriverSignature->dwMainSignature = WMR_VSVFL_VS_SIGNATURE;
        EXAM_RTL_PRINT((TEXT("[FTL:MSG] VSVFL Register  [OK]\n")));
        if (boolSignInBlockZero == FALSE32)
        {
            stFPartFunctions.SetSignatureStyle(&stFPartFunctions, SIGNATURE_STYLE_RESERVEDBLOCK_HEADER);
        }
        else
        {
            EXAM_ERR_PRINT((TEXT("[WMR:ERR] Signature Style RESERVEDBLOCK HEADER has to be used with VSVFL formatting.\n")));
            return FALSE32;
        }
        if ((WMR_SIGNATURE_METADATA(pOldANDDriverSignature->dw2ndId) == WMR_METADATA_WHITENING))
        {
            if (FIL_GetFuncTbl()->SetWhiteningMetadataState)
            {
                EXAM_RTL_PRINT((TEXT("[WMR:MSG] Metadata whitening is set in NAND signature\n")));
                FIL_GetFuncTbl()->SetWhiteningMetadataState(TRUE32);
            }
            else
            {
                EXAM_RTL_PRINT((TEXT("[WMR:ERR] Metadata whitening not supported.\n")));
                return FALSE32;
            }
        }
        else if (FIL_GetFuncTbl()->SetWhiteningMetadataState)
        {
            FIL_GetFuncTbl()->SetWhiteningMetadataState(FALSE32);
        }
    }
    else if (((boolSignatureFound == TRUE32) &&
         WMR_SIGNATURE_VFL(pOldANDDriverSignature->dwMainSignature) == WMR_M68_VFL)
#ifndef AND_READONLY
        || ((boolSignatureFound == FALSE32) && (boolAllowVSVFL == FALSE32))
#endif
        )
    {
#if AND_SUPPORT_LEGACY_VFL
        VFL_Register(&stVflFunctions);
        pNewANDDriverSignature->dwMainSignature = WMR_VFL_SIGNATURE;
        EXAM_RTL_PRINT((TEXT("[FTL:MSG] VFL Register    [OK]\n")));
        if (boolSignInBlockZero == TRUE32)
        {
            stFPartFunctions.SetSignatureStyle(&stFPartFunctions, SIGNATURE_STYLE_BLOCKZERO_NOHEADER);
        }
        else
        {
            EXAM_ERR_PRINT((TEXT("[WMR:ERR] Signature Style BLOCKZERO NOHEADER has to be used with VFL formatting.\n")));
            return FALSE32;
        }
#else
        EXAM_ERR_PRINT((TEXT("[FTL:ERR] Legacy VFL is not supported - init fail\n")));
        return FALSE32;
#endif
    }
    else
    {
        EXAM_ERR_PRINT((TEXT("[WMR:ERR] No Valid VFL type found main signature 0x%08X\n"), 
            pOldANDDriverSignature->dwMainSignature));
        return FALSE32;
    }
    return TRUE32;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      WMR_Init                                                             */
/* DESCRIPTION                                                               */
/*      This function inits whimory & format & open.                         */
/* PARAMETERS                                                                */
/*      pTotalScts  [OUT]   the count of sectors which user can use.         */
/* RETURN VALUES                                                             */
/*      WMR_SUCCESS                                                          */
/*              FTL_Open is completed                                        */
/*      WMR_CRITICAL_ERROR                                                   */
/*              FTL_Open is failed                                           */
/*      WMR_UNRECOVERABLE_ERROR                                              */
/*              Open failed but we can possibly read syscfg                  */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
Int32
WMR_Init(UInt32 *pTotalScts, UInt32 * pdwSectorSize, UInt32 * tmpIndexCache, UInt32 dwFormatType, UInt32 dwBootAreaSize)
{
    Int32 nRet;
    BOOL32 boolSignatureFound = FALSE32;
    volatile BOOL32 boolProductionFormatVerified = TRUE32;
    static UInt32 alreadyCalled = 0;
    BOOL32 boolFTLRegistered = FALSE32;
    BOOL32 boolSignInBlockZero = (dwFormatType & WMR_INIT_SIGNATURE_IN_BLOCKZERO ? TRUE32 : FALSE32);
    BOOL32 nandFullRestore = (dwFormatType & WMR_INIT_NAND_RESTORE ? TRUE32 : FALSE32);	
    UInt32 ftlOpenOptions = 0;
#if (defined(AND_LOW_POWER_MOUNT) && AND_LOW_POWER_MOUNT)
    UInt32 dwInitPowerMode = (dwFormatType & WMR_INIT_LOW_POWER_MODE) ? NAND_POWER_MODE_SINGLE_CE : NAND_POWER_MODE_FULL_SPEED;
#endif
    BOOL32 boolAllowVSVFL = (dwFormatType & WMR_INIT_ALLOW_VSVFL ? TRUE32 : FALSE32);
    BOOL32 boolMetadataWhitening = (dwFormatType & WMR_INIT_METADATA_WHITENING ? TRUE32 : FALSE32);
    BOOL32 boolMetadataWhiteningCheck = (dwFormatType & WMR_INIT_METADATA_WHITENING_CHECK ? TRUE32 : FALSE32);

#ifndef AND_READONLY
    BOOL32 boolStatus;
    BOOL32 boolProductionFormatCompleted = FALSE32;
    BOOL32 bFormatAllowed = (dwFormatType & WMR_INIT_ALLOW_FORMAT ? TRUE32 : FALSE32);
    BOOL32 boolAllowYAFTL = (dwFormatType & WMR_INIT_ALLOW_YAFTL ? TRUE32 : FALSE32);
    BOOL32 boolRunProductionReformat = (dwFormatType & WMR_INIT_RUN_PRODUCTION_FORMAT ? TRUE32 : FALSE32);
    BOOL32 boolRunFTLFormat = (dwFormatType & WMR_INIT_RUN_FTL_FORMAT ? TRUE32 : FALSE32);
    WMR_ASSERT(!(boolRunFTLFormat && boolRunProductionReformat));
#endif

    // make sure we init the functions
    WMR_MEMSET(&stVflFunctions, 0, sizeof(VFLFunctions));
    WMR_MEMSET(&stFtlFunctions, 0, sizeof(FTLFunctions));

    WMR_MEMSET(&g_flashANDSignature, 0, sizeof(ANDDriverSignature));
    WMR_MEMSET(&g_scratchANDSignature, 0, sizeof(ANDDriverSignature));

    OAM_Init();

#ifdef AND_READONLY
    EXAM_RTL_PRINT((TEXT("[FTL:MSG] Apple NAND Driver (AND) RO\n")));
#else
    EXAM_RTL_PRINT((TEXT("[FTL:MSG] Apple NAND Driver (AND) RW\n")));
#endif
    if (alreadyCalled)
    {
        EXAM_ERR_PRINT((TEXT("[WMR:ERR] WMR_Init already called!\n")));
    }

    alreadyCalled++;

    nRet = FIL_Init();

    if (nRet != FIL_SUCCESS)
    {
        return WMR_CRITICAL_ERROR;
    }
#if (defined(AND_LOW_POWER_MOUNT) && AND_LOW_POWER_MOUNT)
    FIL_SetStruct(AND_STRUCT_FIL_POWER_MODE, &dwInitPowerMode, sizeof(UInt32));
#endif

    EXAM_RTL_PRINT((TEXT("[FTL:MSG] FIL_Init            [OK]\n")));
    if (0 == FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_NUM_OF_CS)) {
        EXAM_RTL_PRINT((TEXT("[FTL:MSG] No NAND attached\n")));
        return WMR_CRITICAL_ERROR;
    }
#if defined(WMR_UNIT_TEST_ENABLED) && WMR_UNIT_TEST_ENABLED
    if (!FIL_Test())
    {
        return WMR_CRITICAL_ERROR;
    }
#endif /* FIL_TEST_ENABLED */

    nRet = BUF_Init((UInt16)FIL_GetFuncTbl()->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE), BYTES_PER_METADATA_RAW, WMR_NUM_BUFFERS);

    if (nRet != BUF_SUCCESS)
    {
        return WMR_CRITICAL_ERROR;
    }

    EXAM_RTL_PRINT((TEXT("[FTL:MSG] BUF_Init            [OK]\n")));

    nRet = _initFPart(dwFormatType, dwBootAreaSize);
    if (nRet != WMR_SUCCESS)
    {
        return WMR_CRITICAL_ERROR;
    }
    EXAM_RTL_PRINT((TEXT("[FTL:MSG] FPart Init          [OK]\n")));

    /* signature searching */
    // try old style signature first (quicker)

    if (boolSignInBlockZero == TRUE32)
    {
        stFPartFunctions.SetSignatureStyle(&stFPartFunctions, SIGNATURE_STYLE_BLOCKZERO_NOHEADER);
    }
    else
    {
        stFPartFunctions.SetSignatureStyle(&stFPartFunctions, SIGNATURE_STYLE_RESERVEDBLOCK_HEADER);
    }

    if (stFPartFunctions.ReadSignature(&g_flashANDSignature, sizeof(ANDDriverSignature)))
    {
        if (boolSignInBlockZero == TRUE32)
        {
            EXAM_RTL_PRINT((TEXT("read old style signature 0x%08X (line:%d)\n"), g_flashANDSignature.dwMainSignature, __LINE__));
        }
        else
        {
            EXAM_RTL_PRINT((TEXT("read new style signature 0x%08X (line:%d)\n"), g_flashANDSignature.dwMainSignature, __LINE__));
        }

        if (_isSignatureCompatible(&g_flashANDSignature))
        {
            boolSignatureFound = TRUE32;
#ifndef AND_READONLY
            if (boolRunProductionReformat == TRUE32)
            {
                stFPartFunctions.WriteSignature(NULL, 0);
            }
#endif
            if (boolMetadataWhiteningCheck)
            {
                BOOL32 boolSignatureHasMetadataWhitening = (WMR_SIGNATURE_METADATA(g_flashANDSignature.dw2ndId) == WMR_METADATA_WHITENING);
                if ( (boolSignatureHasMetadataWhitening == TRUE32 && boolMetadataWhitening == FALSE32) 
                    ||  (boolSignatureHasMetadataWhitening == FALSE32 && boolMetadataWhitening == TRUE32))
                {
#ifndef AND_READONLY
                    stFPartFunctions.WriteSignature(NULL, 0);
#endif
                    EXAM_RTL_PRINT((TEXT("[FTL:ERR] metadata whitening mismatch, please reformat\n")));
                    return WMR_UNRECOVERABLE_ERROR;
                }
            }
        }
        else
        {
            EXAM_RTL_PRINT((TEXT("[FTL:WRN] Incompatible Signature!\n")));
        }
    }

    /*
     * If we have been instructed to run a production reformat, force it by faking a signature
     * mismatch
     */
#ifndef AND_READONLY
    if (boolRunProductionReformat)
    {
        boolSignatureFound = FALSE32;
    }
#endif // ! AND_READONLY

    /*
     * If any of the basic criteria are violated, we are going to have to reformat.
     * If this isn't permitted, print a diagnoistic and bail out.
     */
    if (!boolSignatureFound)
    {
        /*
         * Verify that we can read the BBT from all banks.
         */
#ifndef AND_READONLY
        if (!bFormatAllowed)
#endif
        {
            EXAM_RTL_PRINT((TEXT("[WMR:ERR] NAND format invalid (mismatch, corrupt, read error or blank NAND device)\n")));
            EXAM_RTL_PRINT((TEXT("[WMR:ERR] boolSignatureFound %s  boolProductionFormatVerified %s nSig 0x%x\n"),
                            boolSignatureFound ? "true" : "false",
                            boolProductionFormatVerified ? "true" : "false",
                            g_flashANDSignature.dwMainSignature));
            goto fail_readonly;
        }
#ifndef AND_READONLY
        boolProductionFormatVerified = stFPartFunctions.VerifyProductionFormat();
#ifdef AND_USE_NOTIFY
        /* one or more of the failed criteria will require a reformat operation */
        FIL_GetFuncTbl()->Notify(AND_NOTIFY_REFORMAT);
#endif
#endif //!AND_READONLY
    }

    // decide what VFL to use
    if (FALSE32 == _selectVFL(boolSignatureFound, boolAllowVSVFL, 
        boolSignInBlockZero, &g_flashANDSignature, &g_scratchANDSignature))
    {
        return WMR_UNRECOVERABLE_ERROR;
    }
    nRet = stVflFunctions.Init(&stFPartFunctions);

    if (nRet != VFL_SUCCESS)
    {
        return WMR_UNRECOVERABLE_ERROR;
    }
    EXAM_RTL_PRINT((TEXT("[FTL:MSG] VFL Init            [OK]\n")));

#ifndef AND_READONLY
    /*
     * If we couldn't find the BBT, or the signature is missing (or we are pretending it's missing),
     * we need to think about (re)formatting.
     */
    if (boolProductionFormatVerified != TRUE32 || boolSignatureFound != TRUE32)
    {
        Int32 nRetTemp;
        if (boolMetadataWhitening)
        {
            if (FIL_GetFuncTbl()->SetWhiteningMetadataState)
            {
                EXAM_RTL_PRINT((TEXT("[WMR:INF] Formatting with metadata whitening\n")));
                FIL_GetFuncTbl()->SetWhiteningMetadataState(TRUE32);
            }
            else
            {
                EXAM_RTL_PRINT((TEXT("[WMR:ERR] Metadata whitening unavailable!\n")));
                return WMR_UNRECOVERABLE_ERROR;
            }
        }
        if (boolProductionFormatVerified == FALSE32)
        {
            // this is a either virgin device or a device that went through
            // power failure during initial format. We have to scan for
            // the initial bad blocks and save them as a factory bad-block table.
            EXAM_RTL_PRINT((TEXT("[WMR:INF] Calling VFL_ProductionFormat()\n")));
            if (stFPartFunctions.WriteInitialBBT() != TRUE32)
            {
                EXAM_RTL_PRINT((TEXT("[WMR:ERR] Failed VFL_ProductionFormat\n")));
                return WMR_UNRECOVERABLE_ERROR;
            }
            EXAM_RTL_PRINT((TEXT("[WMR:MSG] Production Format OK!\n")));
            boolProductionFormatCompleted = TRUE32;
        }

        if (boolProductionFormatCompleted == FALSE32)
        {
            EXAM_RTL_PRINT((TEXT("[FTL:MSG] Calling VFL_FactoryReformat()\n")));
            // erase signature before clearing the NAND
            stFPartFunctions.WriteSignature(NULL, 0);
            stFPartFunctions.FactoryReformat(FALSE32, FALSE32, dwBootAreaSize);
        }

        nRetTemp = stVflFunctions.Format(dwBootAreaSize, 0);

        if (nRetTemp != VFL_SUCCESS)
        {
            return WMR_UNRECOVERABLE_ERROR;
        }

        EXAM_RTL_PRINT((TEXT("[FTL:MSG] VFL_Format          [OK]\n")));

        // check if we need a shrinked FTL partition
        if ((*pTotalScts) && (*pdwSectorSize))
        {
            UInt32 dwPageSize, dwPagesPerSuBlks;
            UInt16 wNewFTLSuBlkCnt;
            UInt32 dwRequestedPages;

            dwPagesPerSuBlks = (UInt16)stVflFunctions.GetDeviceInfo(AND_DEVINFO_PAGES_PER_SUBLK);
            dwPageSize = (UInt16)stVflFunctions.GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);

            dwRequestedPages = (*pTotalScts) * ((*pdwSectorSize) / dwPageSize);

            wNewFTLSuBlkCnt = (UInt16)((UInt32)(110 * dwRequestedPages) /
                                       (dwPagesPerSuBlks * 100)) + 23;

            stVflFunctions.SetStruct(AND_STRUCT_VFL_NUM_OF_FTL_SUBLKS, &wNewFTLSuBlkCnt, sizeof(UInt16));
        }

        // choose the FTL we want to use and init it
        if (boolAllowYAFTL == TRUE32)
        {
#if (defined (AND_SUPPORT_YAFTL) && AND_SUPPORT_YAFTL)
            YAFTL_Register(&stFtlFunctions);
            EXAM_RTL_PRINT((TEXT("[FTL:MSG] YAFTL Register  [OK]\n")));
#else
            EXAM_RTL_PRINT((TEXT("[FTL:ERR] YAFTL is not supported - init fail\n")));
            return WMR_UNRECOVERABLE_ERROR;
#endif
        }
        else
        {
#if AND_SUPPORT_LEGACY_FTL
            EXAM_RTL_PRINT((TEXT("[FTL:MSG] FTL Register    [OK]\n")));
            FTL_Register(&stFtlFunctions);
#else
            EXAM_RTL_PRINT((TEXT("[FTL:ERR] Legacy FTL is not supported - init fail\n")));
            return WMR_UNRECOVERABLE_ERROR;
#endif
        }
        boolFTLRegistered = TRUE32;
        nRetTemp = stFtlFunctions.Init(&stVflFunctions);
        if (nRetTemp != FTL_SUCCESS)
        {
            EXAM_RTL_PRINT((TEXT("[FTL:ERR] FTL failed Init\n")));
            return WMR_UNRECOVERABLE_ERROR;
        }
        EXAM_RTL_PRINT((TEXT("[FTL:MSG] FTL Init        [OK]\n")));
        nRetTemp = stFtlFunctions.Format(0);
        if (nRetTemp != FTL_SUCCESS)
        {
            EXAM_RTL_PRINT((TEXT("[FTL:ERR] FTL failed Formatting\n")));
            return WMR_UNRECOVERABLE_ERROR;
        }
        EXAM_RTL_PRINT((TEXT("[FTL:MSG] FTL Format      [OK]\n")));

        /* full reformat, write normal signature */
        // Main signature is set from _selectVFL
        g_scratchANDSignature.dw2ndId = WMR_FORMATTING_VERSION;
        // determine if metadata whitening is needed based on signature
        if (boolMetadataWhitening)
        {
            g_scratchANDSignature.dw2ndId |= WMR_METADATA_SIGNATURE;
        }
        /* 5249095 - this should write FTL build info, not kernel version */
        WMR_MEMSET(g_scratchANDSignature.strBuildInfo, 0, WMR_MAX_STR_BUILD_SIZE);
        WMR_MEMCPY(g_scratchANDSignature.strBuildInfo, WMR_GETVERSION(),
                   WMR_MIN(WMR_STRLEN(WMR_GETVERSION()), (WMR_MAX_STR_BUILD_SIZE - 1)));
        boolStatus = stFPartFunctions.WriteSignature(&g_scratchANDSignature, sizeof(ANDDriverSignature));
        if (boolStatus)
        {
            WMR_MEMCPY(&g_flashANDSignature, &g_scratchANDSignature, sizeof(ANDDriverSignature));
        }
        else
        {
            EXAM_RTL_PRINT((TEXT("[FTL:MSG] Write Signature Failed 0x%08X\n"), g_flashANDSignature.dwMainSignature));
            return WMR_CRITICAL_ERROR;
        }

        EXAM_RTL_PRINT((TEXT("[FTL:MSG] Write Signature OK 0x%08X\n"), g_flashANDSignature.dwMainSignature));
        {
            WMR_MEMSET(&g_scratchANDSignature, 0, sizeof(ANDDriverSignature));
            boolStatus = stFPartFunctions.ReadSignature(&g_scratchANDSignature, sizeof(ANDDriverSignature));
            if (!boolStatus || WMR_MEMCMP(&g_scratchANDSignature, &g_flashANDSignature, sizeof(ANDDriverSignature)))
            {
                EXAM_RTL_PRINT((TEXT("[FTL:MSG] Read back Signature Failed 0x%08X\n"), g_scratchANDSignature.dwMainSignature));
                return WMR_CRITICAL_ERROR;
            }
            else
            {
                EXAM_RTL_PRINT((TEXT("[FTL:MSG] Read back Signature OK\n")));
            }
        }
    }
#endif // ! AND_READONLY
    nRet = stVflFunctions.Open(dwBootAreaSize, 0, 0);

    if (nRet != VFL_SUCCESS)
    {
        EXAM_RTL_PRINT((TEXT("[WMR:ERR] VFL_Open failed\n")));
        WMR_BEEP(1000000);
        return WMR_UNRECOVERABLE_ERROR;
    }

    EXAM_RTL_PRINT((TEXT("[FTL:MSG] VFL_Open            [OK]\n")));

    if (boolFTLRegistered == FALSE32)
    {
#ifndef AND_READONLY
        // Check if we wanted to reformat FTL...
        if (boolRunFTLFormat)
        {
            // change the FTL size inside the vfl and mark the FTL
            // for format
            if ((*pTotalScts) && (*pdwSectorSize))
            {
                UInt32 dwPageSize, dwPagesPerSuBlks;
                UInt16 wNewFTLSuBlkCnt;
                UInt32 dwRequestedPages;

                dwPagesPerSuBlks = (UInt16)stVflFunctions.GetDeviceInfo(AND_DEVINFO_PAGES_PER_SUBLK);
                dwPageSize = (UInt16)stVflFunctions.GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);

                dwRequestedPages = (*pTotalScts) * ((*pdwSectorSize) / dwPageSize);

                wNewFTLSuBlkCnt = (UInt16)((UInt32)(110 * dwRequestedPages) /
                                           (dwPagesPerSuBlks * 100)) + 23;
                stVflFunctions.SetStruct(AND_STRUCT_VFL_NUM_OF_FTL_SUBLKS, &wNewFTLSuBlkCnt, sizeof(UInt16));
            }
            else
            {
                UInt16 wNewFTLSuBlkCnt;
                UInt32 dwSize = sizeof(wNewFTLSuBlkCnt);
                stVflFunctions.GetStruct(AND_STRUCT_VFL_NUM_OF_SUBLKS, &wNewFTLSuBlkCnt, &dwSize);
                stVflFunctions.SetStruct(AND_STRUCT_VFL_NUM_OF_FTL_SUBLKS, &wNewFTLSuBlkCnt, sizeof(UInt16));
            }
        }
#endif

        // check what FTL to use here
        if (stVflFunctions.GetDeviceInfo(AND_DEVINFO_FTL_TYPE) == FTL_TYPE_YAFTL)
        {
#if (defined (AND_SUPPORT_YAFTL) && AND_SUPPORT_YAFTL)
            YAFTL_Register(&stFtlFunctions);
            EXAM_RTL_PRINT((TEXT("[FTL:MSG] YAFTL Register  [OK]\n")));
#else
            EXAM_RTL_PRINT((TEXT("[FTL:ERR] YAFTL is not supported - init fail\n")));
            return WMR_UNRECOVERABLE_ERROR;
#endif
        }
        else
        {
#if AND_SUPPORT_LEGACY_FTL
            EXAM_RTL_PRINT((TEXT("[FTL:MSG] FTL Register    [OK]\n")));
            FTL_Register(&stFtlFunctions);
#else
            EXAM_RTL_PRINT((TEXT("[FTL:ERR] Legacy FTL is not supported - init fail\n")));
            return WMR_UNRECOVERABLE_ERROR;
#endif
        }
        nRet = stFtlFunctions.Init(&stVflFunctions);
        if (nRet != FTL_SUCCESS)
        {
            EXAM_RTL_PRINT((TEXT("[WMR:ERR] FTL Init Fail\n")));
            return WMR_UNRECOVERABLE_ERROR;
        }
#ifndef AND_READONLY
        if (stVflFunctions.GetDeviceInfo(AND_DEVINFO_FTL_NEED_FORMAT) ||
            (boolRunFTLFormat == TRUE32))
        {
            nRet = stFtlFunctions.Format(0);
            if (nRet != FTL_SUCCESS)
            {
                EXAM_RTL_PRINT((TEXT("[WMR:ERR] FTL ReFormat Fail\n")));
                return WMR_UNRECOVERABLE_ERROR;
            }
        }
#endif
    }

    if ((dwFormatType & WMR_INIT_DISABLE_GC_IN_TRIM) == WMR_INIT_DISABLE_GC_IN_TRIM)
    {
        EXAM_RTL_PRINT((TEXT("FTL_Open will get trim GC disabled settings \n")));
        ftlOpenOptions |=  WMR_INIT_DISABLE_GC_IN_TRIM;
    }

    if ((dwFormatType & WMR_INIT_SET_INDEX_CACHE_SIZE) == WMR_INIT_SET_INDEX_CACHE_SIZE)
    {
        EXAM_RTL_PRINT((TEXT("FTL_Open will set index cache to 0x%x \n"),*tmpIndexCache));
        ftlOpenOptions |= WMR_INIT_SET_INDEX_CACHE_SIZE;
        ftlOpenOptions |= ((*tmpIndexCache) << 20);
    }
    
    if ( WMR_INIT_OPEN_READONLY == (dwFormatType & WMR_INIT_OPEN_READONLY) )
    {
        ftlOpenOptions |= WMR_INIT_OPEN_READONLY;
    }
    nRet = stFtlFunctions.Open(pTotalScts, pdwSectorSize, nandFullRestore, FALSE32, 0, ftlOpenOptions);
    if (nRet != FTL_SUCCESS)
    {
#ifdef WMR_ERASE_UNIT_WHEN_OPEN_FAIL
        if (!bFormatAllowed)
        {
            EXAM_RTL_PRINT((TEXT("[WMR:ERR] FTL_Open failed\n"),
                            nSig, WMR_SIGNATURE));
            goto fail_readonly;
        }

        EXAM_RTL_PRINT((TEXT("[WMR:ERR] FTL_Open failed - erasing block 0\n")));
        pLowFuncTbl->Erase(0, 0);
#else
        EXAM_RTL_PRINT((TEXT("[WMR:ERR] FTL_Open failed\n")));
        WMR_BEEP(1000000);
#endif
        return WMR_UNRECOVERABLE_ERROR;
    }
    EXAM_RTL_PRINT((TEXT("[FTL:MSG] FTL_Open            [OK]\n")));

    return WMR_SUCCESS;

 fail_readonly:
    EXAM_RTL_PRINT((TEXT("******************************************************************************\n")));
    EXAM_RTL_PRINT((TEXT("******************************************************************************\n")));
    EXAM_RTL_PRINT((TEXT("AND: NAND initialisation failed due to format mismatch or uninitialised NAND.\n")));
    EXAM_RTL_PRINT((TEXT("AND: Please reboot with reformatting enabled.\n")));
    EXAM_RTL_PRINT((TEXT("******************************************************************************\n")));
    EXAM_RTL_PRINT((TEXT("******************************************************************************\n")));

    return WMR_UNRECOVERABLE_ERROR;
}

BOOL32 WMRGetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 * pdwStructSize)
{
    BOOL32 boolRes = FALSE32;
    switch(dwStructType)
    {
        case AND_STRUCT_WMR_VERSION:	
        {
            UInt32 dwSignature = g_flashANDSignature.dwMainSignature;
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwSignature, sizeof(dwSignature));
            break;
        }
        default:
            EXAM_ERR_PRINT((TEXT("[WMR:ERR]  WMRGetStruct 0x%X is not identified is WMR data struct identifier!\n"), dwStructType));
            boolRes = FALSE32;
    }
    return boolRes;
}


static BOOL32 _andGetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 * pdwStructSize)
{
    BOOL32 boolRes = FALSE32;

    if (dwStructType == LOCKDOWN_EXTRACT_ALL)
    {
        // Mobile Lockdown - Get All Structures
#if AND_COLLECT_STATISTICS
         boolRes = ANDExportAllStructs(pvoidStructBuffer, pdwStructSize);
#endif
    }
    else if (dwStructType == LOCKDOWN_GET_ALL_SIZE)
    {
#if AND_COLLECT_STATISTICS
        UInt32 *dwAllSize = (UInt32*) pvoidStructBuffer;
        if (dwAllSize && pdwStructSize && (*pdwStructSize >= sizeof(UInt32)))
        {
            // Mobile Lockdown - Get Size
            boolRes = ANDExportAllStructs(NULL, dwAllSize);
            *pdwStructSize = sizeof(UInt32);
        }
        else
        {
            boolRes = FALSE32;
        }
#endif
    }
    else
    {
        switch (dwStructType & AND_STRUCT_LAYER_MASK)
        {
        case AND_STRUCT_LAYER_FTL:
        {
            boolRes = stFtlFunctions.GetStruct(dwStructType, pvoidStructBuffer, pdwStructSize);
            break;
        }

        case AND_STRUCT_LAYER_VFL:
        {
            boolRes = stVflFunctions.GetStruct(dwStructType, pvoidStructBuffer, pdwStructSize);
            break;
        }

        case AND_STRUCT_LAYER_FIL:
        {
            boolRes = FIL_GetStruct(dwStructType, pvoidStructBuffer, pdwStructSize);
            break;
        }

        case AND_STRUCT_LAYER_GETALL:
        {
#if AND_COLLECT_STATISTICS
            if (dwStructType == AND_STRUCT_WMR_EXPORT_ALL)
            {
                boolRes = ANDExportAllStructs(pvoidStructBuffer, pdwStructSize);
            }
            else if (dwStructType == AND_STRUCT_WMR_EXPORT_ALL_GET_SIZE)
            {
                UInt32 *dwStructSizeVal = (UInt32*) pvoidStructBuffer;
                if (dwStructSizeVal && pdwStructSize && (*pdwStructSize >= sizeof(UInt32)))
                {
                    boolRes = ANDExportAllStructs(NULL, dwStructSizeVal);
                    *pdwStructSize = sizeof(UInt32);
                }
            }
#endif
            break;
        }
        default:
        {
            EXAM_ERR_PRINT((TEXT("[WMR:ERR]  _andGetStruct 0x%X is not identified !\n"), dwStructType));
            boolRes = FALSE32;
            break;
        }
        }
    }

    return boolRes;
}

static BOOL32 _andPerformFunction(UInt32 dwFunctionSelector, void * pvoidStructBuffer, UInt32 * pdwStructSize)
{
    switch (dwFunctionSelector)
    {
#ifndef AND_READONLY

    case _AND_FUNCTION_COMPLETE_EPOCH_UPDATE:
    {
        WMR_PRINT(ERROR, "Epoch update is deprecated\n");
        return FALSE32;
    }

    case AND_FUNCTION_INDEX_CACHE_UPDATE:
    {
        BOOL32 boolRes = FALSE32;
        if (stFtlFunctions.GetStruct && pdwStructSize)
        {
            boolRes = stFtlFunctions.GetStruct(dwFunctionSelector, NULL, pdwStructSize);
            *pdwStructSize = 0;
        }
        return boolRes;
    }

    case AND_FUNCTION_CHANGE_FTL_TYPE:
    {
        BOOL32 boolRes = FALSE32;
        if (stVflFunctions.SetStruct && pdwStructSize)
        {
            boolRes = stVflFunctions.SetStruct(AND_STRUCT_VFL_NEW_FTLTYPE, pvoidStructBuffer, *pdwStructSize);
        }
        return boolRes;
    }

    case AND_FUNCTION_CHANGE_NUM_OF_SUBLKS:
    {
        BOOL32 boolRes = FALSE32;
        if (stVflFunctions.SetStruct && pdwStructSize)
        {
            boolRes = stVflFunctions.SetStruct(AND_STRUCT_VFL_NEW_NUM_OF_FTL_SUBLKS, pvoidStructBuffer, *pdwStructSize);
        }
        return boolRes;
    }

    case AND_FUNCTION_SET_BURNIN_CODE:
    {
        BOOL32 boolRes = FALSE32;
        if (stVflFunctions.SetStruct && pdwStructSize)
        {
            boolRes = stVflFunctions.SetStruct(AND_STRUCT_VFL_BURNIN_CODE, pvoidStructBuffer, *pdwStructSize);
        }
        return boolRes;
    }

    case AND_FUNCTION_ERASE_SIGNATURE:
    {
        BOOL32 boolRes;
        WMR_MEMSET(&g_flashANDSignature, 0, sizeof(ANDDriverSignature));
        boolRes = stFPartFunctions.WriteSignature(&g_flashANDSignature, sizeof(ANDDriverSignature));
        return boolRes;
    }

    case AND_FUNCTION_SET_POWER_MODE:
    {
        BOOL32 boolRes = FALSE32;
        if (pdwStructSize)
        {
            boolRes = FIL_SetStruct(AND_STRUCT_FIL_POWER_MODE, pvoidStructBuffer, *pdwStructSize);
        }
        return boolRes;
    }

    case AND_FUNCTION_NEURALIZE:
    {
        BOOL32 boolRes;
        boolRes = stFPartFunctions.Neuralize();
        return boolRes;
    }
    
    case AND_FUNCTION_SET_TIMINGS:
    {
        BOOL32 boolRes = FALSE32;
        if (pdwStructSize)
        {
            boolRes = FIL_SetStruct(AND_STRUCT_FIL_SET_TIMINGS, pvoidStructBuffer, *pdwStructSize);
        }
        return boolRes;
    }

    case AND_FUNCTION_SAVE_STATS:
    {
        BOOL32 boolRes = FALSE32;
        if(stFtlFunctions.WriteStats)
        {
            boolRes = stFtlFunctions.WriteStats();
            return boolRes;
        }
    }            
#endif // ! AND_READONLY

    default:
        return FALSE32;
    }
}

BOOL32 WMR_CtrlIO(UInt32 dwCtlrIOType, void * pvoidDataBuffer, UInt32 * pdwDataSize)
{
    if ((dwCtlrIOType & AND_STRUCT_LAYER_MASK) == AND_STRUCT_LAYER_FUNCTIONAL)
    {
        return _andPerformFunction(dwCtlrIOType, pvoidDataBuffer, pdwDataSize);
    }
    else
    {
        return _andGetStruct(dwCtlrIOType, pvoidDataBuffer, pdwDataSize);
    }
}

#if AND_SIMULATOR
void WMR_Close(void)
{
    stFtlFunctions.Close();
    WMR_MEMSET(&stFtlFunctions, 0, sizeof(FTLFunctions));
    stVflFunctions.Close();
    WMR_MEMSET(&stVflFunctions, 0, sizeof(VFLFunctions));
    stFPartFunctions.Close();
    WMR_MEMSET(&stFPartFunctions, 0, sizeof(FPartFunctions));
    boolFPartInitialized = FALSE32;
    // NirW - using Ram disk - killing FIL is a pain and irrelevant...
//  FIL_Close();
//  WMR_MEMSET(&stDeviceInfo, 0, sizeof(stDeviceInfo));
}
#endif

FTLFunctions * WMR_GetFTL(void)
{
    return &stFtlFunctions;
}

VFLFunctions * WMR_GetVFL(void)
{
    return &stVflFunctions;
}

#endif // (!defined(AND_FPART_ONLY) || !AND_FPART_ONLY)

