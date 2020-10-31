/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_SOC_DVFMPERF_H
#define __PLATFORM_SOC_DVFMPERF_H
#include <platform/soc/chipid.h>

enum chipid_voltage_index dvfmperf_get_voltage_index(uint32_t index, enum chipid_voltage_type voltage_type);

#endif /* __PLATFORM_SOC_DVFMPERF_H */
