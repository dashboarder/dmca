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

#ifndef _EFFACEABLE_NAND_H_
#define _EFFACEABLE_NAND_H_

// =============================================================================

#include <sys/types.h>

// =============================================================================

void effaceable_nand_init(NandPartInterface * npi, uint32_t idx);

// =============================================================================

#endif /* _EFFACEABLE_NAND_H_ */
