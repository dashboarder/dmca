/*
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

// Author:  Daniel J. Post (djp), dpost@apple.com

#include "s_geom.h"

#ifndef AND_READONLY
UInt32 (*s_g_mul_bytes_per_lba)(UInt32 num);
UInt32 (*s_g_mul_bytes_per_lba_meta)(UInt32 num);
UInt32 (*s_g_addr_to_vba)(UInt32 sb, UInt32 vbaOfs);
UInt32 (*s_g_vba_to_sb)(UInt32 vba);
UInt32 (*s_g_vba_to_vbaOfs)(UInt32 vba);
UInt32 (*s_g_div_bytes_per_page)(UInt32 num);
UInt32 (*s_g_mul_vbas_per_page)(UInt32 num);
UInt32 (*s_g_div_vbas_per_stripe)(UInt32 num);
UInt32 (*s_g_mod_vbas_per_stripe)(UInt32 num);

UInt32 s_g_mul_bytes_per_lba_shift(UInt32 num)
{
    return num << sftl.__geom.bytes_per_lba_shift;
}

UInt32 s_g_mul_bytes_per_lba_meta_shift(UInt32 num)
{
    return num << sftl.__geom.bytes_per_lba_meta_shift;
}

UInt32 s_g_addr_to_vba_shift(UInt32 sb, UInt32 vbaOfs)
{
    return (sb << sftl.__geom.vbas_per_sb_shift) + vbaOfs;
}

UInt32 s_g_vba_to_sb_shift(UInt32 vba)
{
    return vba >> sftl.__geom.vbas_per_sb_shift;
}

UInt32 s_g_vba_to_vbaOfs_shift(UInt32 vba)
{
    return vba & ((1 << sftl.__geom.vbas_per_sb_shift) - 1);
}

UInt32 s_g_div_bytes_per_page_shift(UInt32 num)
{
    return num >> sftl.__geom.bytes_per_page_shift;
}

UInt32 s_g_mul_vbas_per_page_shift(UInt32 num)
{
    return num << sftl.__geom.vbas_per_page_shift;
}

UInt32 s_g_div_vbas_per_stripe_shift(UInt32 num)
{
    return num >> sftl.__geom.vbas_per_stripe_shift;
}

UInt32 s_g_mod_vbas_per_stripe_and(UInt32 num)
{
    return num & (sftl.__geom.vbas_per_stripe-1);
}

UInt32 s_g_mul_bytes_per_lba_mul(UInt32 num)
{
    return num * sftl.__geom.bytes_per_lba;
}

UInt32 s_g_mul_bytes_per_lba_meta_mul(UInt32 num)
{
    return num * sftl.__geom.bytes_per_lba_meta;
}

UInt32 s_g_addr_to_vba_mul(UInt32 sb, UInt32 vbaOfs)
{
    return (sb * sftl.__geom.vbas_per_sb) + vbaOfs;
}

UInt32 s_g_vba_to_sb_div(UInt32 vba)
{
    return vba / sftl.__geom.vbas_per_sb;
}

UInt32 s_g_vba_to_vbaOfs_div(UInt32 vba)
{
    return vba % sftl.__geom.vbas_per_sb;
}

UInt32 s_g_div_bytes_per_page_div(UInt32 num)
{
    return num / sftl.__geom.bytes_per_page;
}

UInt32 s_g_mul_vbas_per_page_mul(UInt32 num)
{
    return num * sftl.__geom.vbas_per_page;
}

UInt32 s_g_div_vbas_per_stripe_div(UInt32 num)
{
    return num / sftl.__geom.vbas_per_stripe;
}

UInt32 s_g_mod_vbas_per_stripe_mod(UInt32 num)
{
    return num % sftl.__geom.vbas_per_stripe;
}

BOOL32 s_can_ffs(UInt32 num)
{
    return 0 == (num & (num - 1));
}

UInt32 s_ffs(UInt32 num)
{
    UInt32 shift = 0;
    WMR_ASSERT(0 == (num & (num - 1)));

    while (num > 1) {
        shift++;
        num >>= 1;
    }

    return shift;
}
#endif // AND_READONLY

void s_geom_init(void)
{
    sftl.__geom.pages_per_sb       = sftl.vfl->GetDeviceInfo(AND_DEVINFO_PAGES_PER_SUBLK);
    sftl.__geom.max_sb             = sftl.vfl->GetDeviceInfo(AND_DEVINFO_NUM_OF_USER_SUBLK);
    sftl.__geom.vfl_max_sb         = sftl.vfl->GetDeviceInfo(AND_DEVINFO_BLOCKS_PER_CAU);
    sftl.__geom.bytes_per_page     = sftl.vfl->GetDeviceInfo(AND_DEVINFO_BYTES_PER_PAGE);
    sftl.__geom.vbas_per_page      = sftl.vfl->GetDeviceInfo(AND_DEVINFO_FIL_LBAS_PER_PAGE);
    sftl.__geom.vbas_per_sb        = sftl.__geom.pages_per_sb * sftl.__geom.vbas_per_page;
    sftl.__geom.bytes_per_lba      = sftl.__geom.bytes_per_page / sftl.__geom.vbas_per_page;
    sftl.__geom.bytes_per_lba_meta = sftl.vfl->GetDeviceInfo(AND_DEVINFO_FIL_META_BUFFER_BYTES);
    sftl.__geom.num_banks          = sftl.vfl->GetDeviceInfo(AND_DEVINFO_NUM_OF_BANKS);
    sftl.__geom.vbas_per_stripe    = s_g_num_banks * s_g_vbas_per_page;

#ifndef AND_READONLY
    // Bytes per lba:
    if (s_can_ffs(sftl.__geom.bytes_per_lba)) {
        sftl.__geom.bytes_per_lba_shift = s_ffs(sftl.__geom.bytes_per_lba);
        s_g_mul_bytes_per_lba = &s_g_mul_bytes_per_lba_shift;
    } else {
        s_g_mul_bytes_per_lba = &s_g_mul_bytes_per_lba_mul;
    }

    // Bytes per page:
    if (s_can_ffs(sftl.__geom.bytes_per_page)) {
        sftl.__geom.bytes_per_page_shift = s_ffs(sftl.__geom.bytes_per_page);
        s_g_div_bytes_per_page = &s_g_div_bytes_per_page_shift;
    } else {
        s_g_div_bytes_per_page = &s_g_div_bytes_per_page_div;
    }

    // Bytes per lba META:
    if (s_can_ffs(sftl.__geom.bytes_per_lba_meta)) {
        sftl.__geom.bytes_per_lba_meta_shift = s_ffs(sftl.__geom.bytes_per_lba_meta);
        s_g_mul_bytes_per_lba_meta = &s_g_mul_bytes_per_lba_meta_shift;
    } else {
        s_g_mul_bytes_per_lba_meta = &s_g_mul_bytes_per_lba_meta_mul;
    }

    // VBAs per sb:
    if (s_can_ffs(sftl.__geom.vbas_per_sb)) {
        sftl.__geom.vbas_per_sb_shift = s_ffs(sftl.__geom.vbas_per_sb);
        s_g_addr_to_vba = &s_g_addr_to_vba_shift;
        s_g_vba_to_sb = &s_g_vba_to_sb_shift;
        s_g_vba_to_vbaOfs = &s_g_vba_to_vbaOfs_shift;
    } else {
        s_g_addr_to_vba = &s_g_addr_to_vba_mul;
        s_g_vba_to_sb = &s_g_vba_to_sb_div;
        s_g_vba_to_vbaOfs = &s_g_vba_to_vbaOfs_div;
    }

    // VBAs per page:
    if (s_can_ffs(sftl.__geom.vbas_per_page)) {
        sftl.__geom.vbas_per_page_shift = s_ffs(sftl.__geom.vbas_per_page);
        s_g_mul_vbas_per_page = &s_g_mul_vbas_per_page_shift;
    } else {
        s_g_mul_vbas_per_page = &s_g_mul_vbas_per_page_mul;
    }

    // VBAS per stripe:
    if (s_can_ffs(sftl.__geom.vbas_per_stripe)) {
        sftl.__geom.vbas_per_stripe_shift = s_ffs(sftl.__geom.vbas_per_stripe);
        s_g_div_vbas_per_stripe = &s_g_div_vbas_per_stripe_shift;
        s_g_mod_vbas_per_stripe = &s_g_mod_vbas_per_stripe_and;
    } else {
        s_g_div_vbas_per_stripe = &s_g_div_vbas_per_stripe_div;
        s_g_mod_vbas_per_stripe = &s_g_mod_vbas_per_stripe_mod;
    }
#endif // AND_READONLY

    // Sanity-check meta size:
    if (sizeof(PageMeta_t) != sftl.__geom.bytes_per_lba_meta) {
        WMR_PANIC("sftl meta struct size (%d) not equal to bytes per metadata (%d)", sizeof(PageMeta_t), sftl.__geom.bytes_per_lba_meta);
    }
}


