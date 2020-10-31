/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __AMC_REGS_H
#define __AMC_REGS_H

#include <platform/soc/hwregbase.h>

#if (AMC_REG_VERSION == 1 || AMC_REG_VERSION == 2)
#include "amc_regs_v12.h"
#elif AMC_REG_VERSION == 3
#include "amc_regs_v3.h"
#elif (AMC_REG_VERSION == 4 || AMC_REG_VERSION == 5)
#include "amc_regs_v45.h"
#else
#error "amc_regs header missing"
#endif

#endif /* __AMC_REGS_H */
