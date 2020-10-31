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

#include "WhimoryBoot.h"
#include "WhimoryBootTypes.h"
#include "WMRConfig.h"
#include "WMROAM.h"
#include "VFLBuffer.h"
#include "FPart.h"
#include "VFL.h"
#include "FTL.h"
#include "FIL.h"
#include "FILTypes.h"
#include "WMRFeatures.h"
#include "ANDStats.h"
#if WMR_UNIT_TEST_ENABLED
#include "WMRTest.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////////////////

static struct WhimorySignature _current_signature;
#ifndef AND_READONLY
static struct WhimorySignature _signature_at_boot; // keep around for debug
#endif //AND_READONLY

static FTLFunctions   *_ftl = NULL;
static VFLFunctions   *_vfl = NULL;
static FPartFunctions *_fpart = NULL;
static LowFuncTbl     *_fil = NULL;

static struct WhimoryDeviceInfo _device_info;

extern Boot_FTL_Registry_t g_ftl_registry[];
extern Boot_VFL_Registry_t g_vfl_registry[];

Boot_FPart_Registry_t g_fpart_registry[] =
{
    { PPN_FPart_MAJOR_VER, PPN_FPart_GetFunctions },
    { 0, NULL },
};

////////////////////////////////////////////////////////////////////////////////
// Static function declarations
////////////////////////////////////////////////////////////////////////////////

#if !(AND_FPART_ONLY)
static FTLFunctions   *
_LookupFTL
    (UInt32 major_ver);

static VFLFunctions   *
_LookupVFL
    (UInt32 major_ver);
#endif //!(AND_FPART_ONLY)

static FPartFunctions *
_LookupFPart
    (UInt32 major_ver);

static void
_GetDeviceInfo
    (LowFuncTbl *fil);

#if (!AND_READONLY) && !(AND_FPART_ONLY)

static WhimoryBootCode
_AllocateInitialSignature
    (void);

static BOOL32
_FillNewSignature
    (struct WhimorySignature *sig,
    UInt32 fpart_major_ver,
    UInt32 fpart_options,
    UInt32 fpart_format_options,
    UInt32 vfl_major_ver,
    UInt32 vfl_options,
    UInt32 ftl_major_ver,
    UInt32 ftl_options,
    UInt32 num_keepout_blocks);

static WhimoryBootCode
_DestructiveFormat
    (struct WhimorySignature *sig,
    UInt32 megabytes_for_ftl);

static BOOL32
_SetFTLPartition
    (UInt32 megabytes_for_ftl);

#endif //(!AND_READONLY) && !(AND_FPART_ONLY)

static BOOL32
_SignatureHasCorrectGeometry
    (struct WhimorySignature *sig);

static BOOL32
_andGetStruct
    (UInt32 selector,
    void * data,
    UInt32 * size);

static BOOL32
_andPerformFunction
    (UInt32 selector,
    void *buffer,
    UInt32 *size);

#if !(AND_FPART_ONLY)
static void
_ClearFILStatistics
    (void);
#endif //!(AND_FPART_ONLY)

////////////////////////////////////////////////////////////////////////////////
// Public function implementations
////////////////////////////////////////////////////////////////////////////////

WhimoryBootCode
WMR_Start
    (UInt32 fpart_major_ver,
    UInt32 fpart_options,
    UInt32 boot_options)   // low-power, etc
{
    ANDStatus status;
    WhimoryBootCode ret_val = kWhimoryOk;
    BOOL32 found_any_signature = FALSE32;

    if (!OAM_Init())
    {
        ret_val = kWhimoryErrSetup;
        goto start_fail;
    }

    WMR_ASSERT(sizeof(struct WhimorySignature) == WHIMORY_SIGNATURE_SIZE);

#ifndef AND_READONLY
    WMR_PRINT(INIT, "Apple PPN NAND Driver, Read/Write\n");
#else
    WMR_PRINT(INIT, "Apple PPN NAND Driver, Read-Only\n");
#endif //AND_READONLY

    status = FIL_Init();
    if (status != FIL_SUCCESS)
    {
        ret_val = kWhimoryErrMissingHardware;
        goto start_fail;
    }

    _fil = FIL_GetFuncTbl();

    // Populate relevant device info
    _GetDeviceInfo(_fil);

    // Verify that there is a device
    if (0 == _device_info.num_ce)
    {
        WMR_PRINT(ERROR, "No NAND device found\n");
        ret_val = kWhimoryErrMissingHardware;
        goto start_fail;
    }

    WMR_PRINT(INIT, "FIL_Init  [OK]\n");

#if WMR_UNIT_TEST_ENABLED
    // FIL Test Hook
    if (!WMR_FIL_Test(_fil))
    {
        ret_val = kWhimoryErrUnitTestFailed;
        goto start_fail;
    }
#endif //WMR_UNIT_TEST_ENABLED

#if AND_LOW_POWER_MOUNT
    // Determine if we need to minimize power
    if (boot_options & kWhimoryBootOptionLowPower)
    {
        UInt32 power_mode = NAND_POWER_MODE_SINGLE_CE;
        FIL_SetStruct(AND_STRUCT_FIL_POWER_MODE,
                      &power_mode,
                      sizeof(power_mode));
    }
#endif //AND_LOW_POWER_MOUNT

    status = BUF_Init(_device_info.main_bytes_per_page,
                      _device_info.total_meta_per_page,
                      WMR_NUM_BUFFERS);

    if (status != BUF_SUCCESS)
    {
        ret_val = kWhimoryErrOutOfMemory;
        goto start_fail;
    }

    _fpart = _LookupFPart(fpart_major_ver);
    if (NULL == _fpart)
    {
        ret_val = kWhimoryErrUnsupportedVersion;
        goto start_fail;
    }

    WMR_ASSERT(NULL != _fpart->Init);
    if (!_fpart->Init(_fil, fpart_options))
    {
        ret_val = kWhimoryErrSetup;
        goto start_fail;
    }

#if WMR_UNIT_TEST_ENABLED
    // FPart Test Hook
    if (!WMR_FPart_Test(_fpart))
    {
        ret_val = kWhimoryErrUnitTestFailed;
        goto start_fail;
    }
#endif //WMR_UNIT_TEST_ENABLED

    // Read the signature to see if we're allowed to talk to this NAND
    WMR_MEMSET(&_current_signature, 0xA5, sizeof(_current_signature));
    found_any_signature = _fpart->ReadSpecialBlock(&_current_signature,
                                                   sizeof(_current_signature),
                                                   AND_SB_TYPE_DRIVER_SIGNATURE);

    if (!found_any_signature)
    {
        WMR_PRINT(INIT, "No signature found.\n");
        ret_val = kWhimoryWarningUnformatted;
        goto start_fail;
    }

    if (WHIMORY_SIGNATURE_MAGIC != _current_signature.magic)
    {
        WMR_PRINT(ERROR, "Signature has invalid magic: 0x%08x\n",
                  _current_signature.magic);
        ret_val = kWhimoryErrCorruptedFormat;
        goto start_fail;
    }

    if (fpart_major_ver != _current_signature.fpart_major_ver)
    {
        WMR_PRINT(ERROR, "FPart major ver in signature is %d\n",
                  _current_signature.fpart_major_ver);
        ret_val = kWhimoryErrCorruptedFormat;
        goto start_fail;
    }

    if (!_SignatureHasCorrectGeometry(&_current_signature))
    {
        WMR_PRINT(ERROR, "Geometry does not match signature\n");
        ret_val = kWhimoryErrCorruptedFormat;
        goto start_fail;
    }

#ifndef AND_READONLY
    // Copy the found signature to a scratch buffer so we can modify it later
    WMR_MEMCPY(&_signature_at_boot,
               &_current_signature,
               sizeof(_current_signature));
#endif //AND_READONLY

    if (WHIMORY_SIGNATURE_VER_PPN_CUR < _current_signature.whimory_ver_at_birth)
    {
        // This device was born from a newer driver and cannot be opened
        WMR_PRINT(ERROR, "Device version %d unsupported\n",
                  _current_signature.whimory_ver_at_birth);
        ret_val = kWhimoryBirthVersionTooNew;
        goto start_fail;
    }

#if !(AND_FPART_ONLY)
    // Make sure that the required FTL and VFL are available
    if (NULL == _LookupVFL(_current_signature.vfl_major_ver))
    {
        ret_val = kWhimoryMajorVersionNotAvailable;
        goto start_fail;
    }

    if (NULL == _LookupFTL(_current_signature.ftl_major_ver))
    {
        ret_val = kWhimoryMajorVersionNotAvailable;
        goto start_fail;
    }
#endif //!AND_FPART_ONLY

    return kWhimoryOk;

 start_fail:

    return ret_val;
}

#if !(AND_FPART_ONLY)
WhimoryBootCode
WMR_Open
    (UInt32 *total_sectors,
    UInt32 *sector_size,
    UInt32 *index_cache, 
    UInt32 boot_options)
{
    WhimoryBootCode ret_val = kWhimoryOk;
    Int32 status;

    WMR_ASSERT(NULL != _fil);
    WMR_ASSERT(NULL != _fpart);
    WMR_ASSERT(_current_signature.magic == WHIMORY_SIGNATURE_MAGIC);
    WMR_ASSERT(WHIMORY_SIGNATURE_VER_PPN_CUR >= _current_signature.whimory_ver_at_birth);

    // Check VFL and FTL software availability first
    _vfl = _LookupVFL(_current_signature.vfl_major_ver);
    if (NULL == _vfl)
    {
        ret_val = kWhimoryErrUnsupportedVersion;
        goto open_fail;
    }

    WMR_ASSERT(NULL != _vfl->GetMinorVersion);
    if (_current_signature.vfl_minor_ver != _vfl->GetMinorVersion())
    {
        WMR_PRINT(ERROR, "VFL minor version - disk %d sw %d\n",
                  _current_signature.vfl_minor_ver, _vfl->GetMinorVersion());
        ret_val = kWhimoryMinorVersionMismatch;
        goto open_fail;
    }

    _ftl = _LookupFTL(_current_signature.ftl_major_ver);
    if (NULL == _ftl)
    {
        ret_val = kWhimoryErrUnsupportedVersion;
        goto open_fail;
    }

    WMR_ASSERT(NULL != _ftl->GetMinorVersion);
    if (_current_signature.ftl_minor_ver != _ftl->GetMinorVersion())
    {
        WMR_PRINT(ERROR, "FTL minor version - disk %d sw %d\n",
                  _current_signature.ftl_minor_ver, _ftl->GetMinorVersion());
        ret_val = kWhimoryMinorVersionMismatch;
        goto open_fail;
    }

    status = _vfl->Init(_fpart);
    if (status != VFL_SUCCESS)
    {
        WMR_PRINT(ERROR, "VFL_Init [Failed]\n");
        ret_val = kWhimoryErrSetup;
        goto open_fail;
    }

    status = _vfl->Open(_current_signature.num_keepout_blocks,
                        _current_signature.vfl_minor_ver,
                        _current_signature.vfl_options);

    if (status != VFL_SUCCESS)
    {
        WMR_PRINT(ERROR, "VFL_Open [Failed]\n");
        ret_val = kWhimoryErrCorruptedFormat;
        goto open_fail;
    }

    WMR_PRINT(INIT, "VFL_Open    [OK]\n");

    _ClearFILStatistics();

#if WMR_UNIT_TEST_ENABLED
    // VFL Test Hook
    if (!WMR_VFL_Test(_vfl))
    {
        ret_val = kWhimoryErrUnitTestFailed;
        goto open_fail;
    }
#endif //WMR_UNIT_TEST_ENABLED

    status = _ftl->Init(_vfl);
    if (status != FTL_SUCCESS)
    {
        WMR_PRINT(ERROR, "FTL_Init [Failed]\n");
        ret_val = kWhimoryErrSetup;
        goto open_fail;
    }
    
    if ((boot_options & kWhimoryBootOptionDisableGCInTrim) == kWhimoryBootOptionDisableGCInTrim)
    {
        _current_signature.ftl_options |= WMR_INIT_DISABLE_GC_IN_TRIM;
    }

    if ((boot_options & kWhimorySetIndexCacheSize) == kWhimorySetIndexCacheSize)
    {
        _current_signature.ftl_options |= WMR_INIT_SET_INDEX_CACHE_SIZE;
        _current_signature.ftl_options |= ((*index_cache) << 20);
    }
#ifdef AND_SIMULATOR
    if(boot_options & (WMR_INIT_IGNORE_ERASE_GAP))
    {
        _current_signature.ftl_options |= WMR_INIT_IGNORE_ERASE_GAP;
    }
#endif
    status = _ftl->Open(total_sectors,
                        sector_size,
                        ((boot_options & kWhimoryBootOptionForceFtlRecovery) ? TRUE32 : FALSE32),
                        ((boot_options & kWhimoryBootOptionFreshOffFTLFormat) ? TRUE32 : FALSE32),
                        _current_signature.ftl_minor_ver,
                        _current_signature.ftl_options|((boot_options & kWhimoryBootOptionReadOnly) ? kWhimoryBootOptionReadOnly : 0));
    if (status != FTL_SUCCESS)
    {
        WMR_PRINT(ERROR, "FTL_Open [Failed]\n");
        ret_val = kWhimoryErrCorruptedFormat;
        goto open_fail;
    }
    WMR_PRINT(INIT, "FTL_Open    [OK]\n");

#if WMR_UNIT_TEST_ENABLED
    // FTL Test Hook
    if (!WMR_FTL_Test(_ftl))
    {
        ret_val = kWhimoryErrUnitTestFailed;
        goto open_fail;
    }
#endif //WMR_UNIT_TEST_ENABLED

    return kWhimoryOk;

 open_fail:
    WMR_PRINT(ERROR, "FAILED: 0x%08x\n", ret_val);

    return ret_val;
}

#ifndef AND_READONLY
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
    UInt32 megabytes_for_ftl)
{
    WhimoryBootCode boot_code = kWhimoryErrSetup;

    // Should have previously attempted to FIL_Init
    _fil = FIL_GetFuncTbl();
    WMR_ASSERT(NULL != _fil);

    // Populate relevant device info
    _GetDeviceInfo(_fil);

    // Fill-out a signature and get function pointers
    if (_FillNewSignature(&_current_signature,
                          fpart_major_ver,
                          fpart_options,
                          fpart_format_options,
                          vfl_major_ver,
                          vfl_options,
                          ftl_major_ver,
                          ftl_options,
                          num_keepout_blocks))
    {
        // Write that format without maintaining user data
        boot_code = _DestructiveFormat(&_current_signature, megabytes_for_ftl);
    }

    return boot_code;
}

WhimoryBootCode
WMR_Update
    (const BOOL32 keep_backward_compatibility,
    const BOOL32 save_data,
    UInt32 vfl_major_ver,
    UInt32 vfl_options,
    UInt32 ftl_major_ver,
    UInt32 ftl_options,
    UInt32 num_keepout_blocks)
{
    // This function is like destructive format, but carries forward some
    // signature information instead of generating a new signature

    WhimoryBootCode boot_code = kWhimoryOk;

    WMR_ASSERT(WHIMORY_SIGNATURE_MAGIC == _current_signature.magic);
    WMR_ASSERT(WHIMORY_SIGNATURE_VER_PPN_CUR >= _current_signature.whimory_ver_at_birth);

    // We don't support these types of graceful transitions yet
    WMR_ASSERT(!keep_backward_compatibility);
    WMR_ASSERT(!save_data);

    _vfl = _LookupVFL(vfl_major_ver);
    if (NULL == _vfl)
    {
        boot_code = kWhimoryErrUnsupportedVersion;
        goto update_error;
    }

    _current_signature.vfl_major_ver = vfl_major_ver;
    _current_signature.vfl_minor_ver = _vfl->GetMinorVersion();
    _current_signature.vfl_options = vfl_options;

    _ftl = _LookupFTL(ftl_major_ver);
    if (NULL == _ftl)
    {
        boot_code = kWhimoryErrUnsupportedVersion;
        goto update_error;
    }

    _current_signature.ftl_major_ver = ftl_major_ver;
    _current_signature.ftl_minor_ver = _ftl->GetMinorVersion();
    _current_signature.ftl_options = ftl_options;

    if (_current_signature.num_keepout_blocks != num_keepout_blocks)
    {
        WMR_PRINT(ERROR, "Warning: changing keepout from %d to %d\n",
                  _current_signature.num_keepout_blocks, num_keepout_blocks);
        _current_signature.num_keepout_blocks = num_keepout_blocks;
    }

    boot_code = _DestructiveFormat(&_current_signature, 0);

    if (kWhimoryOk != boot_code)
    {
        // On failure, revert to old setting
        WMR_MEMCPY(&_current_signature, &_signature_at_boot, sizeof(_current_signature));
    }

    return boot_code;

 update_error:

    WMR_MEMCPY(&_current_signature, &_signature_at_boot, sizeof(_current_signature));
    return boot_code;
}

WhimoryBootCode
WMR_Partition
    (UInt32 megabytes_for_ftl)
{
    WhimoryBootCode boot_code = kWhimoryOk;
    Int32 nand_status;

    WMR_ASSERT(NULL != _fpart);
    // Assert that we have a valid signature in memory
    WMR_ASSERT(_current_signature.magic == WHIMORY_SIGNATURE_MAGIC);

    _vfl = _LookupVFL(_current_signature.vfl_major_ver);
    if (NULL == _vfl)
    {
        boot_code = kWhimoryErrUnsupportedVersion;
        goto partition_fail;
    }

    _ftl = _LookupFTL(_current_signature.ftl_major_ver);
    if (NULL == _ftl)
    {
        boot_code = kWhimoryErrUnsupportedVersion;
        goto partition_fail;
    }

    // Assume VFL is already formatted
    if (ANDErrorCodeOk != (nand_status = _vfl->Init(_fpart)))
    {
        boot_code = kWhimoryErrSetup;
        goto partition_fail;
    }

    nand_status = _vfl->Open(_current_signature.num_keepout_blocks,
                             _current_signature.vfl_minor_ver,
                             _current_signature.vfl_options);
    if (ANDErrorCodeOk != nand_status)
    {
        WMR_PRINT(ERROR, "VFL Open failed\n");
        boot_code = kWhimoryErrCorruptedFormat;
        goto partition_fail;
    }

    _ClearFILStatistics();

    if (!_SetFTLPartition(megabytes_for_ftl))
    {
        boot_code = kWhimoryErrUnsupportedVersion;
        goto partition_fail;
    }

    // Format the FTL with the new size, but don't open it
    if (ANDErrorCodeOk != (nand_status = _ftl->Init(_vfl)))
    {
        boot_code = kWhimoryErrSetup;
        goto partition_fail;
    }
    nand_status = _ftl->Format(_current_signature.ftl_options);
    if (ANDErrorCodeOk != nand_status)
    {
        WMR_PRINT(ERROR, "FTL Format failed\n");
        boot_code = kWhimoryErrCorruptedFormat;
        goto partition_fail;
    }

    if (0 != (WHIMORY_FTL_PARTITION_VALID_BIT & _current_signature.megabytes_for_ftl))
    {
        // if there's a pending partition flag in the signature, clear it
        _current_signature.megabytes_for_ftl &= ~WHIMORY_FTL_PARTITION_VALID_BIT;

        WMR_PRINT(INIT, "Updating signature\n");
        if (!_fpart->WriteSpecialBlock(&_current_signature,
                                    sizeof(_current_signature),
                                    AND_SB_TYPE_DRIVER_SIGNATURE))
        {
            WMR_PRINT(ERROR, "Failed to write signature\n");
            boot_code = kWhimoryErrCorruptedFormat;
            goto partition_fail;
        }
        WMR_MEMCPY(&_signature_at_boot, &_current_signature, sizeof(_current_signature));
    }

    // Tear down so we can call WMR_Open()
    _ftl->Close();
    _vfl->Close();

    return kWhimoryOk;

 partition_fail:

    return boot_code;
}

#endif // ! AND_READONLY
#endif // ! AND_FPART_ONLY

void
WMR_Close
    (void)
{
    if (NULL != _ftl)
    {
        _ftl->Close();
    }
    _ftl = NULL;

    if (NULL != _vfl)
    {
        _vfl->Close();
    }
    _vfl = NULL;

    if (NULL != _fpart)
    {
        _fpart->Close();
    }
    _fpart = NULL;

    BUF_Close();

    // Clear static variables

#ifndef AND_READONLY
    WMR_MEMSET(&_signature_at_boot, 0, sizeof(_signature_at_boot));
#endif //!AND_READONLY
    WMR_MEMSET(&_current_signature, 0, sizeof(_current_signature));
    WMR_MEMSET(&_device_info, 0, sizeof(_device_info));
}

BOOL32
_andGetStruct
    (UInt32 selector,
    void   *data,
    UInt32 *size)
{
    BOOL32 result = FALSE32;

    if (LOCKDOWN_EXTRACT_ALL == selector)
    {
        // Mobile Lockdown - Get All Structures
#if AND_COLLECT_STATISTICS
        result = ANDExportAllStructs(data, size);
#endif //AND_COLLECT_STATISTICS
    }
    else if (LOCKDOWN_GET_ALL_SIZE == selector)
    {
#if AND_COLLECT_STATISTICS
        UInt32 *target_size = (UInt32*)data;
        if ((NULL != target_size) &&
            (NULL != size) &&
            (*size >= sizeof(*target_size)))
        {
            result = ANDExportAllStructs(NULL, target_size);
            *size = sizeof(*target_size);
        }
#endif //AND_COLLECT_STATISTICS
    }
    else
    {
        switch (selector & AND_STRUCT_LAYER_MASK)
        {
            case AND_STRUCT_LAYER_FTL:
            {
                if ((NULL != _ftl) && (NULL != _ftl->GetStruct))
                {
                    result = _ftl->GetStruct(selector, data, size);
                }
                break;
            }

            case AND_STRUCT_LAYER_VFL:
            {
                if ((NULL != _vfl) && (NULL != _vfl->GetStruct))
                {
                    result = _vfl->GetStruct(selector, data, size);
                }
                break;
            }

            case AND_STRUCT_LAYER_FIL:
            {
                result = FIL_GetStruct(selector, data, size);
                break;
            }

            case AND_STRUCT_LAYER_GETALL:
            {
#if AND_COLLECT_STATISTICS
                if (AND_STRUCT_WMR_EXPORT_ALL == selector)
                {
                    result = ANDExportAllStructs(data, size);
                }
                else if (AND_STRUCT_WMR_EXPORT_ALL_GET_SIZE == selector)
                {
                    UInt32 *target_size = (UInt32*)data;
                    if ((NULL != target_size) &&
                        (NULL != size) &&
                        (*size >= sizeof(*target_size)))
                    {
                        result = ANDExportAllStructs(NULL, target_size);
                        *size = sizeof(*target_size);
                    }
                }
#endif //AND_COLLECT_STATISTICS
                break;
            }

            default:
            {
                WMR_PRINT(ERROR, "not implemented: 0x%08x\n", selector);
                break;
            }
        }
    }

    return result;
}
BOOL32
_andPerformFunction
    (UInt32 selector,
    void *buffer,
    UInt32 *size)
{
    BOOL32 result = FALSE32;

    switch (selector)
    {
#ifndef AND_READONLY
        case AND_FUNCTION_INDEX_CACHE_UPDATE:
        {
            if ((NULL != _ftl) &&
                (NULL != _ftl->GetStruct) &&
                (NULL != size))
            {
                result = _ftl->GetStruct(selector, NULL, size);
                *size = 0;
            }
        }
        break;

        case AND_FUNCTION_SET_BURNIN_CODE:
        {
            if ((NULL != _vfl) &&
                (NULL != _vfl->SetStruct) &&
                (NULL != buffer) &&
                (NULL != size))
            {
                result = _vfl->SetStruct(AND_STRUCT_VFL_BURNIN_CODE, buffer, *size);
            }
        }
        break;

        case AND_FUNCTION_SET_POWER_MODE:
        {
            if ((NULL != buffer) &&
                (NULL != size))
            {
                result = FIL_SetStruct(AND_STRUCT_FIL_POWER_MODE, buffer, *size);
            }
        }
        break;

        case AND_FUNCTION_NEURALIZE:
        {
            if (NULL != _fpart)
            {
                result = _fpart->Neuralize();
            }
        }
        break;

        case AND_FUNCTION_SET_TIMINGS:
        {
            if ((NULL != buffer) &&
                (NULL != size))
            {
                result = FIL_SetStruct(AND_STRUCT_FIL_SET_TIMINGS, buffer, *size);
            }
        }
        break;
            
        case AND_FUNCTION_SAVE_STATS:
        {
        if(_ftl->WriteStats)
            {
                result = _ftl->WriteStats();
            }
        }
        break;
        
        case _AND_FUNCTION_COMPLETE_EPOCH_UPDATE:
        {
            WMR_PRINT(ERROR, "Epoch update is deprecated\n");
            result = FALSE32;
        
        }
        break;

#endif // ! AND_READONLY
        default:
        {
            WMR_PANIC("Unknown selector: 0x%08x\n", selector);
            result = FALSE32;
        }
    }

    if (!result)
    {
        WMR_PRINT(ERROR, "selector 0x%08x failed\n", selector);
    }

    return result;
}

BOOL32
WMR_PPN_CtrlIO
    (UInt32 selector,
    void *data,
    UInt32 *size)
{
    if ((selector & AND_STRUCT_LAYER_MASK) == AND_STRUCT_LAYER_FUNCTIONAL)
    {
        return _andPerformFunction(selector, data, size);
    }
    else
    {
        return _andGetStruct(selector, data, size);
    }
}

BOOL32
WMR_PPNGetStruct
    (UInt32 struct_type,
    void *data,
    UInt32 *size)
{
    BOOL32 result = FALSE32;

    switch (struct_type)
    {
        case AND_STRUCT_WMR_VERSION:
        {
            UInt32 dwSignature = WHIMORY_SIGNATURE_MAGIC;
            result = WMR_FILL_STRUCT(data,
                                     size,
                                     &dwSignature,
                                     sizeof(dwSignature));
        }
        break;

        default:
        {
            WMR_PRINT(ERROR, "unknown struct id: 0x%08x\n", struct_type);
        }
        break;
    }
    return result;
}

FTLFunctions *
WMR_PPN_GetFTL
    (void)
{
    WMR_ASSERT(NULL != _ftl);

    return _ftl;
}

VFLFunctions *
WMR_PPN_GetVFL
    (void)
{
    WMR_ASSERT(NULL != _vfl);

    return _vfl;
}

FPartFunctions *
WMR_PPN_GetFPart
    (void)
{
    WMR_ASSERT(NULL != _fpart);

    return _fpart;
}

////////////////////////////////////////////////////////////////////////////////
// Static function implementations
////////////////////////////////////////////////////////////////////////////////

#if !defined(AND_READONLY) && !(AND_FPART_ONLY)
BOOL32
_FillNewSignature
    (struct WhimorySignature *sig,
    UInt32 fpart_major_ver,
    UInt32 fpart_options,
    UInt32 fpart_format_options,
    UInt32 vfl_major_ver,
    UInt32 vfl_options,
    UInt32 ftl_major_ver,
    UInt32 ftl_options,
    UInt32 num_keepout_blocks)
{
    WMR_MEMSET(sig, 0, sizeof(*sig));
    sig->magic = WHIMORY_SIGNATURE_MAGIC;
    sig->_deprecated_epoch = 0;
    sig->whimory_ver_at_birth = WHIMORY_SIGNATURE_VER_PPN_CUR;

    // Get an FPart
    sig->fpart_major_ver = fpart_major_ver;
    _fpart = _LookupFPart(fpart_major_ver);
    if (NULL == _fpart)
    {
        return FALSE32;
    }
    WMR_ASSERT(NULL != _fpart->GetMinorVersion);
    sig->fpart_minor_ver = _fpart->GetMinorVersion();
    sig->fpart_options = fpart_options;
    sig->fpart_format_options = fpart_format_options;

    // Get a VFL
    sig->vfl_major_ver = vfl_major_ver;
    _vfl = _LookupVFL(vfl_major_ver);
    if (NULL == _vfl)
    {
        return FALSE32;
    }
    WMR_ASSERT(NULL != _vfl->GetMinorVersion);
    sig->vfl_minor_ver = _vfl->GetMinorVersion();
    sig->vfl_options = vfl_options;

    // Get an FTL
    sig->ftl_major_ver = ftl_major_ver;
    _ftl = _LookupFTL(ftl_major_ver);
    if (NULL == _ftl)
    {
        return FALSE32;
    }
    WMR_ASSERT(NULL != _ftl->GetMinorVersion);
    sig->ftl_minor_ver = _ftl->GetMinorVersion();

    // Stash the number of keepout blocks
    sig->num_keepout_blocks = num_keepout_blocks;

    // Keep critical geometry information for devices with
    // multiple removable packages
    sig->num_chip_enables = _device_info.num_ce;

    // For debugging, record what software version formatted this device
    WMR_FILL_SW_VERSION(sig->sw_ver_str, sizeof(sig->sw_ver_str));

    return TRUE32;
}

WhimoryBootCode
_AllocateInitialSignature
    (void)
{
    // if there is no block allocated to signature, allocate one
    if (!_fpart->IsSpecialBlockTypeAllocated(AND_SB_TYPE_DRIVER_SIGNATURE))
    {
        SpecialBlockAddress signature_block;
        if (!_vfl->AllocateSpecialBlock(&signature_block,
                                        AND_SB_TYPE_DRIVER_SIGNATURE))
        {
            WMR_PRINT(ERROR, "AllocateSpecialBlock for signature failed\n");
            return kWhimoryErrSetup;
        }
        if (!_fpart->AllocateSpecialBlockType(&signature_block,
                                              1,
                                              AND_SB_TYPE_DRIVER_SIGNATURE))
        {
            WMR_PRINT(ERROR, "AllocateSpecialBlockType failed\n");
            return kWhimoryErrSetup;
        }
    }
    return kWhimoryOk;
}

WhimoryBootCode
_DestructiveFormat
    (struct WhimorySignature *sig,
    UInt32 megabytes_for_ftl)
{
    Int32 nand_status;

    // Init and format FPart
    if (!_fpart->Init(_fil, sig->fpart_options))
    {
        return kWhimoryErrOutOfMemory;
    }

    // Format VFL and open it so FTL can use it
    if (ANDErrorCodeOk != (nand_status = _vfl->Init(_fpart)))
    {
        WMR_PRINT(LOG, "_vfl->Init: 0x%08X\n", nand_status);
        return kWhimoryErrOutOfMemory;
    }

    WMR_PRINT(INIT, "Starting VFL Format\n");
    if (ANDErrorCodeOk != (nand_status = _vfl->Format(sig->num_keepout_blocks,
                                                      sig->vfl_options)))
    {
        WMR_PRINT(LOG, "_vfl->Format: 0x%08X\n", nand_status);
        return kWhimoryErrCorruptedFormat;
    }

    if (ANDErrorCodeOk != (nand_status = _vfl->Open(sig->num_keepout_blocks,
                                                    sig->vfl_minor_ver,
                                                    sig->vfl_options)))
    {
        WMR_PRINT(LOG, "_vfl->Open: 0x%08X\n", nand_status);
        return kWhimoryErrCorruptedFormat;
    }

    // Allocate a signature before any persistent FPart blocks
    // so that subsequent searches will find it first
    WMR_PRINT(INIT, "Allocating signature\n");
    if (ANDErrorCodeOk != _AllocateInitialSignature())
    {
        return kWhimoryErrCorruptedFormat;
    }

    WMR_PRINT(INIT, "Starting FPart Format\n");
    // Allocate persistent special blocks owned by FPart
    if (!_fpart->Format(_vfl, sig->fpart_format_options))
    {
        WMR_PRINT(ERROR, "Failed to format FPart\n");
        return kWhimoryErrSetup;
    }

    _ClearFILStatistics();    
    
    // If there is a request for non-zero burnin-size,
    // set that before formatting FTL
    if ((megabytes_for_ftl > 0) &&
        !_SetFTLPartition(megabytes_for_ftl))
    {
        return kWhimoryErrCorruptedFormat;
    }

    // Format the FTL, but don't open it
    if (ANDErrorCodeOk != (nand_status = _ftl->Init(_vfl)))
    {
        WMR_PRINT(LOG, "_ftl->Init: 0x%08X\n", nand_status);
        return kWhimoryErrOutOfMemory;
    }
    WMR_PRINT(INIT, "Starting FTL Format\n");
    if (ANDErrorCodeOk != (nand_status = _ftl->Format((sig->ftl_options) | (WMR_INIT_RUN_PRODUCTION_FORMAT))))
    {
        WMR_PRINT(LOG, "_ftl->Format: 0x%08X\n", nand_status);
        return kWhimoryErrCorruptedFormat;
    }

    // Write signature to tell WMR_Open() about the details
    WMR_PRINT(INIT, "Writing final signature\n");
    if (!_fpart->WriteSpecialBlock(sig,
                                   sizeof(*sig),
                                   AND_SB_TYPE_DRIVER_SIGNATURE))
    {
        WMR_PRINT(ERROR, "Failed to write signature\n");
        return kWhimoryErrCorruptedFormat;
    }
    WMR_MEMCPY(&_signature_at_boot, sig, sizeof(*sig));

    // Close so that init can be called in WMR_Open()
    _vfl->Close();
    _ftl->Close();
    return kWhimoryOk;
}

BOOL32
_SetFTLPartition
    (UInt32 megabytes_for_ftl)
{
    UInt32 num_superblocks = 0;
    UInt32 num_superblocks_data_size = sizeof(num_superblocks);
    UInt16 superblocks_for_ftl;
    BOOL32 result = FALSE32;

    if (!_vfl->GetStruct(AND_STRUCT_VFL_NUM_OF_SUBLKS,
                         &num_superblocks,
                         &num_superblocks_data_size))
    {
        WMR_PRINT(ERROR, "VFL parameters unavailable\n");
        return FALSE32;
    }

    if (megabytes_for_ftl > 0)
    {
        WMR_ASSERT(num_superblocks > 0);

        WMR_ASSERT(NULL != _ftl);
        superblocks_for_ftl = _ftl->ConvertUserMBtoFTLSuperblocks(_vfl, megabytes_for_ftl);

        WMR_PRINT(INIT, "Set to %dMB using %d virtual blocks\n",
                  megabytes_for_ftl,
                  superblocks_for_ftl);
        WMR_ASSERT(superblocks_for_ftl <= num_superblocks);
    }
    else
    {
        WMR_ASSERT(num_superblocks > 0);
        // All the superblocks go to FTL
        superblocks_for_ftl = num_superblocks;

        WMR_PRINT(INIT, "full size FTL\n");
    }

    result = _vfl->SetStruct(AND_STRUCT_VFL_NUM_OF_FTL_SUBLKS,
                             &superblocks_for_ftl,
                             sizeof(superblocks_for_ftl));

    if (!result)
    {
        WMR_PRINT(ERROR, "unsupported\n");
    }

    return result;
}

#endif //!defined(AND_READONLY) && !(AND_FPART_ONLY)

const struct WhimorySignature *
WMR_PPNGetSignature
    (void)
{
    return &_current_signature;
}

static BOOL32
_SignatureHasCorrectGeometry
    (struct WhimorySignature *sig)
{
    if (sig->num_chip_enables != _device_info.num_ce)
    {
        return FALSE32;
    }

    return TRUE32;
}

#if !(AND_FPART_ONLY)

FTLFunctions *
_LookupFTL
    (UInt32 major_ver)
{
    UInt32 idx;

    for (idx = 0; NULL != g_ftl_registry[idx].registrar; idx++)
    {
        if (major_ver == g_ftl_registry[idx].major_ver)
        {
            return g_ftl_registry[idx].registrar();
        }
    }
    WMR_PRINT(ERROR, "Unsupported FTL major ver: %d\n", major_ver);
    return NULL;
}

VFLFunctions *
_LookupVFL
    (UInt32 major_ver)
{
    UInt32 idx;

    for (idx = 0; NULL != g_vfl_registry[idx].registrar; idx++)
    {
        if (major_ver == g_vfl_registry[idx].major_ver)
        {
            return g_vfl_registry[idx].registrar();
        }
    }
    WMR_PRINT(ERROR, "Unsupported VFL major ver: %d\n", major_ver);
    return NULL;
}

#endif // ! AND_FPART_ONLY

FPartFunctions *
_LookupFPart
    (UInt32 major_ver)
{
    UInt32 idx;

    for (idx = 0; NULL != g_fpart_registry[idx].registrar; idx++)
    {
        if (major_ver == g_fpart_registry[idx].major_ver)
        {
            return g_fpart_registry[idx].registrar();
        }
    }
    WMR_PRINT(ERROR, "Unsupported FPart major ver: %d\n", major_ver);
    return NULL;
}

void
_GetDeviceInfo
    (LowFuncTbl *fil)
{
    _device_info.num_ce = fil->GetDeviceInfo(AND_DEVINFO_NUM_OF_CS);
    _device_info.main_bytes_per_page = fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    _device_info.total_meta_per_page = fil->GetDeviceInfo(AND_DEVINFO_BYTES_PER_SPARE);
    _device_info.lba_per_page = fil->GetDeviceInfo(AND_DEVINFO_FIL_LBAS_PER_PAGE);
}


#if !(AND_FPART_ONLY)
static void
_ClearFILStatistics(void)
{
    FILStatistics fil_stats;

    WMR_MEMSET(&fil_stats, 0, sizeof(fil_stats));
    FIL_SetStruct(AND_STRUCT_FIL_STATISTICS, &fil_stats, sizeof(fil_stats));
}
#endif //!(AND_FPART_ONLY)
