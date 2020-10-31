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

#ifndef __S_STATS_H__
#define __S_STATS_H__

#include "s_internal.h"

#ifndef AND_READONLY

extern void s_stats_from_buf(UInt8 *buf, UInt32 size);
extern UInt32 s_stats_to_buf(UInt32 *buf);

extern void s_stats_insert(void);

extern void s_stats_update(void);

#else // ->!AND_READONLY

#define s_stats_from_buf(x, y)
#define s_stats_to_buf(x) 0
#define s_stats_insert()
#define s_stats_update()

#endif // !AND_READONLY

#endif // __S_STATS_H__

