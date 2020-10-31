/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __L2V_CASSERT_H__
#define __L2V_CASSERT_H__

#define l2v_assert(e) WMR_ASSERT(e)

#define CASSERT(x, name) typedef char __CASSERT_##name[(x) ? 1 : -1]

#define l2v_assert_le(x, y) l2v_assert((x) <= (y))
#define l2v_assert_lt(x, y) l2v_assert((x) < (y))
#define l2v_assert_ge(x, y) l2v_assert((x) >= (y))
#define l2v_assert_gt(x, y) l2v_assert((x) > (y))
#define l2v_assert_eq(x, y) l2v_assert((x) == (y))
#define l2v_assert_ne(x, y) l2v_assert((x) != (y))

#endif // __L2V_CASSERT_H__

