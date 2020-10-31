/*
 * Copyright (c) 2008-2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "ANDTypes.h"
#include "WMROAM.h"
#include "VFLBuffer.h"
#include "FPart.h"
#include "FPartTypes.h"
#include "WMRConfig.h"
#include "WMRExam.h"
#include "ANDStats.h"

static VFLFunctions *stVflFunctions;
static FTLFunctions *stFtlFunctions;

typedef  BOOL32 (*GetStructFunc)(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 * pdwStructSize);

typedef struct {
    UInt32 dwStructLayer;
    UInt32 dwStructType;
    UInt32 dwStructVersion;
} ExportStructParameters;


static ExportStructParameters astrStructsToExport[] = 
{
    // header (must be first)
    { 0, AND_STRUCT_WMR_EXPORT_ALL, AND_EXPORT_STRUCTURE_VERSION},

    // WMR Version (FTL & VFL types)
    {AND_STRUCT_LAYER_WMR, AND_STRUCT_WMR_VERSION, 0},

    // VFL_type
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_GET_TYPE, 0},
    
    // FTL_type ( FTL/yaFTL)
    {AND_STRUCT_LAYER_FTL, AND_STRUCT_FTL_GET_TYPE, 0},

    // All VFL Meta
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 0, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 1, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 2, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 3, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 4, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 5, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 6, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 7, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 8, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 9, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 10, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 11, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 12, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 13, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 14, 0},
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_VFLMETA | 15, 0},

    //FTL geometry
    {AND_STRUCT_LAYER_FTL,AND_STRUCT_FTL_GET_FTL_DEVICEINFO, 0},

    // FTL
    {AND_STRUCT_LAYER_FTL, AND_STRUCT_FTL_FTLCXT, 0},

    // FTL counters
    {AND_STRUCT_LAYER_FTL, AND_STRUCT_FTL_GET_FTL_EC_BINS, 1},
    {AND_STRUCT_LAYER_FTL, AND_STRUCT_FTL_GET_FTL_RC_BINS, 1},

    // FTL - Stat
    {AND_STRUCT_LAYER_FTL, AND_STRUCT_FTL_STATISTICS, 0},

    // VFL - Stat
    {AND_STRUCT_LAYER_VFL, AND_STRUCT_VFL_STATISTICS, 0},

    // FIL - Device Info
    {AND_STRUCT_LAYER_FIL, AND_STRUCT_FIL_DEVICE_INFO, 0},

    // FIL - Stat
    {AND_STRUCT_LAYER_FIL, AND_STRUCT_FIL_STATISTICS, 0},

    // footer (must be last)
    {0, AND_STRUCT_WMR_EXPORT_ALL_END, AND_EXPORT_STRUCTURE_VERSION},

};

#define AND_DUMP_DEBUG (0)

#if AND_DUMP_DEBUG
#define DUMP_VAR(variable) WMR_PRINT(ALWAYS, "%s : %u\n", #variable, (UInt32)(variable))


static void
hexdump(UInt8 *bytes, int count)
{
    int i, j;
    
    for (i = 0; i < count; i += 16) {
        kprintf("%04x:", i);
        for (j = 0; (j < 16) && ((i + j) < count); j++)
            kprintf(" %02.2x", *bytes++);
        kprintf("\n");
    }
}
#else
#define DUMP_VAR(variable)
#endif //AND_DUMP_DEBUG



static GetStructFunc _translateStructLayer (UInt32 dwStructLayer)
{
    switch (dwStructLayer)
    {
        case AND_STRUCT_LAYER_FTL:
            return stFtlFunctions->GetStruct;
        case AND_STRUCT_LAYER_VFL:
            return stVflFunctions->GetStruct;
        case AND_STRUCT_LAYER_FIL:
            return FIL_GetStruct;
        case AND_STRUCT_LAYER_WMR:
            return WMRGetStruct;
        default:
            return NULL;
    }
}


static BOOL32 _appendStructToBuffer(void   *pvoidExportBuffer,
                                    UInt32 *pdwBufferSize,
                                    UInt32 dwStructLayer,
                                    UInt32 dwStructType,
                                    UInt32 dwStructVersion)
{
    UInt32           dwDataSize;
    BOOL32           boolResult;
    GetStructFunc    fGetStruct = _translateStructLayer(dwStructLayer);
    UInt8            *cursor = (UInt8*) pvoidExportBuffer;
    
    
    DUMP_VAR(pvoidExportBuffer);
    if (pdwBufferSize)
    {
        DUMP_VAR(*pdwBufferSize);
    }
    DUMP_VAR(dwStructLayer);
    DUMP_VAR(dwStructType);
    DUMP_VAR(dwStructVersion);
    
    if (fGetStruct)
    {
        if (pvoidExportBuffer)
        {
            // leave room for the struct header
            cursor += sizeof(ANDExportStruct);
        }
        dwDataSize = pdwBufferSize ? (*pdwBufferSize - sizeof(ANDExportStruct)) : 0;
        boolResult = fGetStruct(dwStructType, cursor, &dwDataSize);
        DUMP_VAR(dwDataSize);
    }
    else
    {
        // for header and footer
        boolResult = TRUE32;
        dwDataSize = 0;
    }
    
    if (boolResult)
    {
#if AND_DUMP_DEBUG
        if (pvoidExportBuffer)
        {
            hexdump(pvoidExportBuffer, dwDataSize);
        }
#endif        
        if (pdwBufferSize)
        {
            *pdwBufferSize = dwDataSize + sizeof(ANDExportStruct);
        }
        
        if (pvoidExportBuffer)
        {
            ANDExportStruct strExport;
            
            strExport.dwStructID = dwStructType;
            strExport.dwStructureVersion = dwStructVersion;
            strExport.dwDataSize = dwDataSize;
            strExport.dwIndex = 0; // What is this?
            
            WMR_MEMCPY(pvoidExportBuffer, &strExport, sizeof(ANDExportStruct));
        }   
    }
    else if (pdwBufferSize)
    {
        *pdwBufferSize = 0;
    }
    
    DUMP_VAR(boolResult);
    return boolResult;
}



#define SUB_MIN_ZERO(minuend, subtrahend)  (((minuend) > (subtrahend)) ? ((minuend) - (subtrahend)) : 0)

BOOL32 ANDExportAllStructs(void * pvoidDataBuffer, UInt32 * pdwDataSize)
{
	UInt32 idx;
    ExportStructParameters *nextStruct;
    const UInt32 dwNumStructures = sizeof(astrStructsToExport) / sizeof(astrStructsToExport[0]);
    const UInt32 dwBufferSize = pdwDataSize ? *pdwDataSize : 0;
    UInt8 *cursor             = (UInt8*) pvoidDataBuffer;
    UInt32 dwBufferRead       = 0;
    UInt32 dwTmpStructSize;
    
    if (!pdwDataSize)
    {
        return FALSE32;
    }
    
    // Update globals
    stVflFunctions = WMR_GetVFL();
    stFtlFunctions = WMR_GetFTL();
    
    nextStruct = &astrStructsToExport[0];
    for (idx = 0; idx < dwNumStructures; ++idx)
    {
        dwTmpStructSize = SUB_MIN_ZERO(dwBufferSize, dwBufferRead);
        _appendStructToBuffer(cursor, 
                              &dwTmpStructSize,
                              nextStruct->dwStructLayer,
                              nextStruct->dwStructType,
                              nextStruct->dwStructVersion);
        nextStruct++;
        if (cursor)
        {
            cursor += dwTmpStructSize;
        }
        dwBufferRead += dwTmpStructSize;
    }
    
    if (pdwDataSize)
    {
        *pdwDataSize = dwBufferRead;
    }

    // pdwDataSize tells the whole story, always pass
	return TRUE32;
}

