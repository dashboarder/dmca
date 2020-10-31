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

#ifndef __S_WEARLEV_H__
#define __S_WEARLEV_H__

#include "s_internal.h"

#define s_wearlev_cross_block() do { sftl.wearLev.blocksSince++; } while(0)
extern void s_wearlev_search(void);

#endif // __S_WEARLEV_H__

