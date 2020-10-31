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

#ifndef _WHIMORY_BOOT_TYPES_H_
#define _WHIMORY_BOOT_TYPES_H_

#define WHIMORY_SIGNATURE_SIZE (3 * 512)
struct WhimorySignature
{
    UInt32 magic;
#define WHIMORY_SIGNATURE_MAGIC (0x776D7278) // 'wmrx'
    UInt32 _deprecated_epoch; // 0 for new devices
    UInt32 whimory_ver_at_birth; // SW older than birth epoch may not open this drive
#define WHIMORY_SIGNATURE_VER_PPN_CUR (6)
    UInt32 ftl_major_ver;
    UInt32 ftl_minor_ver;
    UInt32 ftl_options;
    UInt32 vfl_major_ver;
    UInt32 vfl_minor_ver;
    UInt32 vfl_options;
    UInt32 fpart_major_ver;
    UInt32 fpart_minor_ver;
    UInt32 fpart_options; // debug-only: need these to find signature
    UInt32 num_keepout_blocks;

    UInt32 num_chip_enables;

    // Informational
#define WHIMORY_SW_VER_MAX_LENGTH (128)
    UInt8 sw_ver_str[WHIMORY_SW_VER_MAX_LENGTH];
    UInt32 fpart_format_options; // options used for initial format

#define WHIMORY_FTL_PARTITION_VALID_BIT (1UL << 31)
#define WHIMORY_FTL_PARTITION_SIZE_MASK (0x7fffffffUL)
    UInt32 megabytes_for_ftl;

    // Reserve to 1.5KB
    UInt8 reserved[WHIMORY_SIGNATURE_SIZE - (16 * sizeof(UInt32)) - 128];
};

struct WhimoryDeviceInfo
{
    UInt32 num_ce;
    UInt32 main_bytes_per_page;
    UInt32 total_meta_per_page;
    UInt32 lba_per_page;
};

typedef FTLFunctions * (*FTL_Registrar)(void);
typedef struct
{
    UInt32 major_ver;
    FTL_Registrar registrar;
} Boot_FTL_Registry_t;

typedef VFLFunctions * (*VFL_Registrar)(void);
typedef struct
{
    UInt32 major_ver;
    VFL_Registrar registrar;
} Boot_VFL_Registry_t;

typedef FPartFunctions * (*FPart_Registrar)(void);
typedef struct
{
    UInt32 major_ver;
    FPart_Registrar registrar;
} Boot_FPart_Registry_t;

#endif //_WHIMORY_BOOT_TYPES_H_
