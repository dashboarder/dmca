// *****************************************************************************
//
// File: H2FIL.c
//
// *****************************************************************************
//
// Notes:
//
// *****************************************************************************
//
// Copyright (C) 2008-2010 Apple, Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Computer, Inc.
//
// *****************************************************************************

#include "H2fmi.h"
#include "H2fmi_ppn.h"
#include <WMROAM.h>
#include <WMRFeatures.h>
#include <FIL.h>

/*****************************************************************************/
/* Static variables definitions                                              */
/*****************************************************************************/

/*****************************************************************************/
/* Code Implementation                                                       */
/*****************************************************************************/

static LowFuncTbl stLowFuncTbl =
{
    .GetDeviceInfo              = h2fmiGetDeviceInfo,
    .Reset                      = h2fmiReset,
    .CalcCurrentTiming          = h2fmiCalcCurrentTimings,
    .SetWhiteningState          = h2fmiSetWhiteningState,
    .SetWhiteningMetadataState  = h2fmiSetWhiteningMetadataState,
    .RegisterCurrentTransaction = h2fmiRegisterCurrentTransaction,

#if (defined(AND_SUPPORT_BLOCK_STORAGE) && AND_SUPPORT_BLOCK_STORAGE)
    .SetDeviceInfo              = h2fmiSetDeviceInfo,
    .ReadWithECC                = h2fmiReadSinglePage,
    .ReadNoECC                  = h2fmiReadNoECC,
    .ReadMultiplePages          = h2fmiReadSequentialPages,
    .ReadScatteredPages         = h2fmiReadScatteredPages,
#endif

#if (AND_FPART_ONLY || AND_SUPPORT_BLOCK_STORAGE)
    .ReadMaxECC                 = h2fmiReadSinglePageMaxECC,
#endif

#if ((defined(AND_SUPPORT_NVRAM) && AND_SUPPORT_NVRAM) || (defined(AND_SUPPORT_FW_AREA) && AND_SUPPORT_FW_AREA))
    .ReadBLPage = h2fmiReadBootpage,
#endif

#if !H2FMI_READONLY
    .WriteMultiplePages  = h2fmiWriteMultiplePages,
    .Write               = h2fmiWriteSinglePage,
    .WriteScatteredPages = h2fmiWriteScatteredPages,
    .WriteMaxECC         = h2fmiWriteSinglePageMaxECC,
#endif // !H2FMI_READONLY

#if !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM
    .Erase              = h2fmiEraseSingleBlock,
    .EraseMultiple      = NULL,
    .WriteBLPage        = h2fmiWriteBootpage,
#endif // !defined(AND_READONLY)
};

#if WITH_PPN
static LowFuncTbl stLowFuncTblPpn = {

#if (AND_SUPPORT_BLOCK_STORAGE || AND_SUPPORT_NVRAM || AND_SUPPORT_FW_AREA)
    .GetDeviceInfo              = h2fmiGetDeviceInfo,
    .Reset                      = h2fmiReset,
    .CalcCurrentTiming          = h2fmiCalcCurrentTimings,
    .SetWhiteningState          = h2fmiSetWhiteningState,
    .SetWhiteningMetadataState  = h2fmiSetWhiteningMetadataState,
    .RegisterCurrentTransaction = h2fmiRegisterCurrentTransaction,
    .ReadWithECC                = h2fmiPpnReadSinglePage,
#endif

#if (defined(AND_SUPPORT_BLOCK_STORAGE) && AND_SUPPORT_BLOCK_STORAGE)
    .SetDeviceInfo      = h2fmiSetDeviceInfo,
    .ReadNoECC          = NULL,
    .ReadMultiplePages  = NULL,
    .ReadScatteredPages = NULL,
    .ReadMaxECC         = NULL,
#elif (defined(AND_FPART_ONLY) && AND_FPART_ONLY)
    .ReadNoECC          = NULL,
    .ReadMaxECC         = NULL,
#endif
  
#if ((defined(AND_SUPPORT_NVRAM) && AND_SUPPORT_NVRAM) || (defined(AND_SUPPORT_FW_AREA) && AND_SUPPORT_FW_AREA))
    .ReadBLPage = h2fmiPpnReadBootpage,
#endif

#if !H2FMI_READONLY
    .WriteMultiplePages  = NULL,
    .Write               = h2fmiPpnWriteSinglePage,
    .WriteScatteredPages = NULL,
    .WriteMaxECC         = NULL,
#endif // !H2FMI_READONLY

#if !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM
    .Erase              = h2fmiPpnEraseSingleBlock,
    .EraseMultiple      = NULL,
    .WriteBLPage        = h2fmiPpnWriteBootpage,
#endif
    .PerformCommandList = h2fmiPpnPerformCommandList,
};
#endif //WITH_PPN

extern h2fmi_t g_fmi0;

/*****************************************************************************/
/* Code Implementation                                                       */
/*****************************************************************************/
Int32
FIL_Init(void)
{
    Int32 nFILRet;
    static BOOL32 bInitDone = FALSE32;

    if (TRUE32 == bInitDone)
    {
        h2fmiPrintConfig();
        return FIL_SUCCESS;
    }

    nFILRet   = h2fmiInit();
    if (nFILRet != FIL_SUCCESS)
    {
        return nFILRet;
    }

#if !WITH_PPN
    if (g_fmi0.is_ppn)
    {
        WMR_PRINT(ERROR, "[FIL:ERR] Build does not support PPN!\n");
        return FIL_UNSUPPORTED_ERROR;
    }
#endif // WITH_PPN

    bInitDone = TRUE32;
    return FIL_SUCCESS;
}

LowFuncTbl*
FIL_GetFuncTbl()
{
    LowFuncTbl *ppn = NULL;

#if WITH_PPN
    ppn = &stLowFuncTblPpn;
#endif // WITH_PPN

    return (g_fmi0.is_ppn ? ppn : &stLowFuncTbl);
}

BOOL32 FIL_GetStruct(UInt32 dwStructType, void* pvoidStructBuffer, UInt32* pdwStructSize)
{
    WMR_PRINT(INF, "[FIL:INF] FIL_GetStruct(0x%08X)\n", dwStructType);

    return h2fmiGetStruct(dwStructType, pvoidStructBuffer, pdwStructSize);
}

#if AND_SUPPORT_BLOCK_STORAGE
BOOL32 FIL_SetStruct(UInt32 dwStructType, void* pvoidStructBuffer, UInt32 dwStructSize)
{
    WMR_PRINT(INF,
              "[FIL:INF] FIL_SetStruct(0x%08X, %d)\n", dwStructType, dwStructSize);

    return h2fmiSetStruct(dwStructType, pvoidStructBuffer, dwStructSize);
}
#endif //AND_SUPPORT_BLOCK_STORAGE

void FIL_Close(void)
{
    // do nothing
}
/*
void FIL_SwitchToPpn(void)
{
    stLowFuncTbl.GetDeviceInfo = h2fmi_ppn_get_device_info;
}
*/
