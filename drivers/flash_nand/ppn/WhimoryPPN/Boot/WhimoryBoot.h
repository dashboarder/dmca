// Copyright (C) 2010 Apple Inc. All rights reserved.
//
// This document is the property of Apple Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Inc.
//
//

#ifndef _WHIMORY_BOOT_H_
#define _WHIMORY_BOOT_H_

#include "WMRTypes.h"
#include "FTL.h"
#include "VFL.h"
#include "FPart.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
////////////////////////////////////////////////////////////////////////////////
// Public structure definitions
////////////////////////////////////////////////////////////////////////////////

typedef enum _WhimoryBootCode
{
    kWhimoryOk = 0,

    kWhimoryWarning = (1 << 29),
    kWhimoryWarningUnformatted,
    kWhimoryEpochMismatch_Deprecated,
    kWhimoryMinorVersionMismatch,
    kWhimoryMajorVersionNotAvailable,
    kWhimoryBirthVersionTooNew,

    kWhimoryError = (1 << 30),
    kWhimoryErrMissingHardware,
    kWhimoryErrOutOfMemory,
    kWhimoryErrUnsupportedVersion,
    kWhimoryErrSetup,
    kWhimoryErrUnitTestFailed,
    kWhimoryErrCorruptedFormat,
} WhimoryBootCode;

#define kWhimoryBootOptionLowPower          (1 << 0)
#define kWhimoryBootOptionForceFtlRecovery  (1 << 1)
#define kWhimoryBootOptionFreshOffFTLFormat (1 << 2)    
#define kWhimoryBootOptionDisableGCInTrim   (1 << 3)
#define kWhimorySetIndexCacheSize           (1 << 4)    
#define kWhimoryBootOptionReadOnly          (1 << 16) // Must align with WMR_INIT_OPEN_READONLY @ bit 16.

////////////////////////////////////////////////////////////////////////////////
// Public function declarations
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// WMR_Start
//
// WMR_Start will attempt to locate a signature. It will call OAM_Init()
// and FIL_Init(), so those fuctions should be idempotent.
// Possible non-fatal outcomes:
//   kWhimoryWarningUnformatted - Run WMR_Initial_Format()
//
// On kWhimoryOk or warnings, callers can request WMR_PPN_GetFPart()
//
////////////////////////////////////////////////////////////////////////////////

WhimoryBootCode
WMR_Start
    (UInt32 fpart_major_ver,
    UInt32 fpart_options,
    UInt32 boot_options);

////////////////////////////////////////////////////////////////////////////////
// WMR_Open
//
// WMR_Open will open a block device if the media is properly formatted
// and return appropriate warnings if the device needs to be formatted or
// repaired.
//
////////////////////////////////////////////////////////////////////////////////

WhimoryBootCode
WMR_Open
    (UInt32 *total_sectors,
    UInt32 *sector_size,
    UInt32 *index_cache, 
    UInt32 boot_options);

////////////////////////////////////////////////////////////////////////////////
// WMR_Format
//
// Format the media with a given set of preferred versions
//
////////////////////////////////////////////////////////////////////////////////

WhimoryBootCode
WMR_Initial_Format
    (UInt32 fpart_major_ver,
    UInt32 fpart_options,
    UInt32 fpart_format_options,
    UInt32 vfl_major_ver,
    UInt32 vfl_options,
    UInt32 ftl_major_ver,
    UInt32 ftl_options,
    UInt32 num_keepout_blocks,
    UInt32 megabytes_for_ftl);

////////////////////////////////////////////////////////////////////////////////
// WMR_Partition
//
// Re-size the portion of the VFL that is visible to FTL.
// A value of 0 gives the entire media to FTL.
//
////////////////////////////////////////////////////////////////////////////////

WhimoryBootCode
WMR_Partition
    (UInt32 megabytes_for_ftl);

////////////////////////////////////////////////////////////////////////////////
// WMR_Update
//
// Update the physical format if newer format is supported by this software.
// Optionally require backward compatiblity and saving user data
//
////////////////////////////////////////////////////////////////////////////////

WhimoryBootCode
WMR_Update
    (const BOOL32 keep_backward_compatibility,
    const BOOL32 save_data,
    UInt32 vfl_major_ver,
    UInt32 vfl_options,
    UInt32 ftl_major_ver,
    UInt32 ftl_options,
    UInt32 num_keepout_blocks);

////////////////////////////////////////////////////////////////////////////////
// WMR_Close
//
// Gracefully shut down the block device, deallocate all buffers, and return
// static variables to initial state such that WMR_Open can be called again.
//
////////////////////////////////////////////////////////////////////////////////

void
WMR_Close
    (void);

////////////////////////////////////////////////////////////////////////////////
// WMR_PPN_CtrlIO
//
// Generic IOCtl interface to all layers of whimory. Caller must specifiy data
// buffer size in incoming size pointer.
//
////////////////////////////////////////////////////////////////////////////////

BOOL32
WMR_PPN_CtrlIO
    (UInt32 ctrlio_type,
    void   *data,
    UInt32 *size);

////////////////////////////////////////////////////////////////////////////////
// WMR_PPNGetStruct
//
// Get a parameter from RAM
//
////////////////////////////////////////////////////////////////////////////////

BOOL32
WMR_PPNGetStruct
    (UInt32 ctrlio_type,
    void   *data,
    UInt32 *size);

////////////////////////////////////////////////////////////////////////////////
// WMR_PPNGetCurrentSignature
//
// Get the signature representing the current software state
//
////////////////////////////////////////////////////////////////////////////////

const struct WhimorySignature *
WMR_PPNGetSignature
    (void);

////////////////////////////////////////////////////////////////////////////////
// Accessors
//
// Get a parameter from RAM
//
////////////////////////////////////////////////////////////////////////////////
FTLFunctions   *WMR_PPN_GetFTL
    (void);
VFLFunctions   *WMR_PPN_GetVFL
    (void);
FPartFunctions *WMR_PPN_GetFPart
    (void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_WHIMORY_BOOT_H_
