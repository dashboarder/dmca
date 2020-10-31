/*
 * Copyright (c) 2010-11 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _EFFACEABLE_LOCKER_H_
#define _EFFACEABLE_LOCKER_H_

#include "queue.h"

// =============================================================================

struct _locker;
struct _locker_list;

// -----------------------------------------------------------------------------

typedef struct _locker                     locker_t;
typedef TAILQ_HEAD(_locker_list, _locker)  locker_list_t;

// -----------------------------------------------------------------------------

struct _locker
{
    TAILQ_ENTRY(_locker)                    link;
    AppleEffaceableStorageLockerHeader      header;
    uint8_t                                 data[0];
};

// =============================================================================

#endif /* _EFFACEABLE_LOCKER_H_ */
