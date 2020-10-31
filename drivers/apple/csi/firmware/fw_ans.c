/*
 * Copyright (C) 2012 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#include <platform.h>
#include <platform/memmap.h>
#include <arch.h>
#include <debug.h>
#include <sys/task.h>
#include <drivers/a7iop/a7iop.h>
#include <drivers/csi.h>

#include "csi_firmware.h"

#if SUB_PLATFORM_S7002
#include <AppleCSI.s7002ans.release.h>
//#include <AppleCSI.s7002ans.debug.h>
#elif SUB_PLATFORM_T8002
// XXX
// <rdar://problem/18613350> T8002SIM: ASP support
#include <AppleCSI.s7002ans.release.h>
#else 
#include <AppleCSI.s5l8960xans.release.h>
//#include <AppleCSI.s5l8960xans.debug.h>
#endif

// Validate the IOP protocol against the one required by the firmware file
#if !defined(ANS_PROTOCOL_VER_MAJOR) || !defined(ANS_PROTOCOL_VER_MINOR)
#error ERROR: ANS firmware does not specify protocol version
#else
#if (!(ANS_PROTOCOL_VER_MAJOR == APPLE_CSI_PROTOCOL_VER_MAJOR) || (!(ANS_PROTOCOL_VER_MINOR == APPLE_CSI_PROTOCOL_VER_MINOR)))
#error ERROR: the CSI protocol version does not match the one required by the ANS firmware.
#endif
#endif

static const char*  fw_ans_get_name_str()                 { return fwFirmwareTarget; }
static const char*  fw_ans_get_build_str()                { return fwFirmwareBuild; }
static const char*  fw_ans_get_version_str()              { return fwFirmwareVersion; }
static const char*  fw_ans_get_fw_image()                 { return (const char*)&fwFirmwareImage[0]; }
static uint32_t     fw_ans_get_fw_image_size()            { return sizeof(fwFirmwareImage); }
static uint32_t     fw_ans_get_csi_config_offset()        { return ((fwConfigurationOffset!=SYMBOL_NOT_FOUND) ? fwConfigurationOffset : VALUE_NOT_FOUND); }
static uint32_t     fw_ans_get_memory_required()          { return fwAspRequiredMemory; }
static void*        fw_ans_get_reloc_address()            { return (void*)ASP_BASE; }
static uint32_t     fw_ans_get_reloc_size()               { return ASP_SIZE; }

const get_fw_info_t fw_info_ans = {
    fw_ans_get_name_str,
    fw_ans_get_build_str,
    fw_ans_get_version_str,
    fw_ans_get_fw_image,
    fw_ans_get_fw_image_size,
    fw_ans_get_csi_config_offset,
    fw_ans_get_memory_required,
    fw_ans_get_reloc_address,
    fw_ans_get_reloc_size,
};
