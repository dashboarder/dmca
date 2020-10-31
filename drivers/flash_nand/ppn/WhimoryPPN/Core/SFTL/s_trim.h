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

#ifndef __S_TRIM_H__
#define __S_TRIM_H__

#include "FTL.h"
#include "s_internal.h"

// Lifecycle
extern BOOL32 s_trim_init(void);
extern void   s_trim_close(void);

// Trimmers
extern Int32 sftl_unmap(FTLExtent_t *extents, UInt32 numExtents);

// Write collision
extern void  s_trim_apply(BOOL32 token);
extern void  s_trim_writeCollide(UInt32 lba, UInt32 span);

// Vulnerability management
extern void s_trim_markVulnerable(UInt32 vba);
extern void s_trim_clearVulnerables(void);
extern void s_trim_checkVulnerable(UInt32 sb);

#endif // __S_TRIM_H__

