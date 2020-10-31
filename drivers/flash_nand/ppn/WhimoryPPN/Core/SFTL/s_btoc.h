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

#ifndef __S_BTOC_H__
#define __S_BTOC_H__

#include "s_internal.h"


extern BOOL32  s_btoc_init(void);
extern void    s_btoc_close(void);

#ifndef AND_READONLY
// Write flow:
extern BOOL32  s_btoc_isFull(void);
extern UInt32  s_btoc_update_size(Int32 remaining);
extern void    s_btoc_add_data(UInt32 vba, UInt32 lba, UInt32 count, WeaveSeq_t weaveSeq, UInt32 userWeaveSeq);
extern BOOL32  s_btoc_cross_data(void);
#endif // AND_READONLY

// Read flow:
extern BOOL32  s_btoc_read(UInt32 sb, s_btoc_entry_t *btoc, UInt32 *len, BOOL32 stopEarly, BOOL32 scrubOnUECC); // return: BTOC good=true

// BTOC cache:
extern s_btoc_entry_t *s_btoc_alloc(UInt32 sb);
extern void    s_btoc_dealloc(s_btoc_entry_t *BTOC);

#ifndef AND_READONLY
extern UInt32  s_btoc_getSrc(UInt32 destVpn);
extern void    s_btoc_setSrc(UInt32 destVpn, UInt32 srcVpn);
extern s_btoc_entry_t *s_btoc_search(UInt32 sb);
#else // AND_READONLY
#define s_btoc_getSrc(x)
#define s_btoc_setSrc(x)
#define s_btoc_search(x)
#endif // AND_READONLY

#endif // __S_BTOC_H__

