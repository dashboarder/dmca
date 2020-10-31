/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __SWIFTER_PMU_H
#define __SWIFTER_PMU_H

#define SWIFTER_PMU_SHUTDOWN_REG        0x010
#define SWIFTER_PMU_SUSPEND_REG         0x018
#define SWIFTER_PMU_RESTART_REG         0x020

#define SWIFTER_PMU_NVRAM_START         0x100

#define SWIFTER_PMU_EVENT_REG           0x200
#define SWIFTER_PMU_EVENT_HIB           (1 << 0)
#define SWFITER_PMU_EVENT_HOME          (1 << 1)

#endif
