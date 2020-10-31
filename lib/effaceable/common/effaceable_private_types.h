/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _EFFACEABLE_PRIVATE_TYPES_H_
#define _EFFACEABLE_PRIVATE_TYPES_H_

// =============================================================================

#include <AppleEffaceableStorageFormat.h>
#include "effaceable_locker.h"
#include "effaceable_context.h"

// =============================================================================

// grrrr
#ifndef NULL
#define NULL ((void*)0)
#endif

// -----------------------------------------------------------------------------

#define min(a,b)                    \
    ({                              \
        __typeof__ (a) _a = (a);    \
        __typeof__ (b) _b = (b);    \
        _a < _b ? _a : _b;          \
    })

// =============================================================================

#endif /* _EFFACEABLE_PRIVATE_TYPES_H_ */
