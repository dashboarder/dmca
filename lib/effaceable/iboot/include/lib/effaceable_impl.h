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

#ifndef _EFFACEABLE_IMPL_H_
#define _EFFACEABLE_IMPL_H_

// =============================================================================

#include <effaceable_contract.h>

// =============================================================================

__BEGIN_DECLS

// -----------------------------------------------------------------------------

extern EffaceableReturn setupEffaceableSystemContract(effaceable_system_t * system);
extern void registerEffaceableStorage(effaceable_storage_t * storage);

// -----------------------------------------------------------------------------

__END_DECLS

// =============================================================================

#endif /* _EFFACEABLE_IMPL_H_ */
