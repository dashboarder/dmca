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

#ifndef __S_READ_H__
#define __S_READ_H__

#include "s_internal.h"

extern UInt32 s_read_xlate(UInt32 lba);
extern Int32  s_read_int(UInt32 lba, UInt32 count, UInt8 *buf);
extern Int32  sftl_read(UInt32 lba, UInt32 count, UInt8 *buf);
extern Int32  sftl_read_spans(FTLExtent_t *extents, UInt32 count, UInt8 *buf);

#endif // __S_READ_H__
