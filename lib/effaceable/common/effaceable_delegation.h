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

#ifndef _EFFACEABLE_DELEGATION_H_
#define _EFFACEABLE_DELEGATION_H_

// =============================================================================

#define isFnAvailable(_me, _it, _fn)    (0 != context(_me)->_it->_fn)
#define delegateFn(_me, _it, _fn, ...)  context(_me)->_it->_fn(context(_me)->_it, ## __VA_ARGS__)

// =============================================================================

#endif /* _EFFACEABLE_DELEGATION_H_ */
