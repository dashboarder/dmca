/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

// Author:  Daniel J. Post (djp), dpost@apple.com

#ifndef __SFTL_GEOM_H__
#define __SFTL_GEOM_H__

#include "s_internal.h"

#define s_g_bytes_per_page (sftl.__geom.bytes_per_page+0)
#define s_g_bytes_per_lba (sftl.__geom.bytes_per_lba+0)
#define s_g_bytes_per_lba_meta (sftl.__geom.bytes_per_lba_meta+0)
#define s_g_vbas_per_sb (sftl.__geom.vbas_per_sb+0)
#define s_g_vbas_per_page (sftl.__geom.vbas_per_page+0)
#define s_g_max_pages_per_btoc (sftl.__geom.max_pages_per_btoc+0)
#define s_g_max_sb (sftl.__geom.max_sb+0)
#define s_g_vfl_max_sb (sftl.__geom.vfl_max_sb+0)
#define s_g_num_banks (sftl.__geom.num_banks+0)
#define s_g_vbas_per_stripe (sftl.__geom.vbas_per_stripe+0)

extern void s_geom_init(void);

#ifndef AND_READONLY

extern UInt32 (*s_g_mul_bytes_per_lba)(UInt32 num);
extern UInt32 (*s_g_mul_bytes_per_lba_meta)(UInt32 num);
extern UInt32 (*s_g_addr_to_vba)(UInt32 sb, UInt32 vbaOfs);
extern UInt32 (*s_g_vba_to_sb)(UInt32 vba);
extern UInt32 (*s_g_vba_to_vbaOfs)(UInt32 vba);
extern UInt32 (*s_g_div_bytes_per_page)(UInt32 num);
extern UInt32 (*s_g_mul_vbas_per_page)(UInt32 num);
extern UInt32 (*s_g_div_vbas_per_stripe)(UInt32 num);
extern UInt32 (*s_g_mod_vbas_per_stripe)(UInt32 num);

#else // AND_READONLY

#define s_g_mul_bytes_per_lba(x) ((x) * sftl.__geom.bytes_per_lba)
#define s_g_mul_bytes_per_lba_meta(x) ((x) * sftl.__geom.bytes_per_lba_meta)
#define s_g_addr_to_vba(x, y) (((x) * sftl.__geom.vbas_per_sb) + (y))
#define s_g_vba_to_sb(x) ((x) / sftl.__geom.vbas_per_sb)
#define s_g_vba_to_vbaOfs(x) ((x) % sftl.__geom.vbas_per_sb)
#define s_g_div_bytes_per_page(x) ((x) / sftl.__geom.bytes_per_page)
#define s_g_mul_vbas_per_page(x) ((x) * sftl.__geom.vbas_per_page)
#define s_g_div_vbas_per_stripe(x) ((x) / sftl.__geom.vbas_per_stripe)
#define s_g_mod_vbas_per_stripe(x) ((x) % sftl.__geom.vbas_per_stripe)

#endif // AND_READONLY

#endif // __SFTL_GEOM_H__

