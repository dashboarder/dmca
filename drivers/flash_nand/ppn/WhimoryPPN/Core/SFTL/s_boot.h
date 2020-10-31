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

#ifndef __S_BOOT_H__
#define __S_BOOT_H__

#include "s_internal.h"

extern Int32 sftl_boot(UInt32 *reported_lba, UInt32 *lba_size, BOOL32 fullRestore, BOOL32 justFmt, UInt32 minorVer, UInt32 opts);

#endif // __S_BOOT_H__
