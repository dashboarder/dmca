/*
 * Copyright (C) 2009-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __CLOCK_MANAGEMENT_H__
#define __CLOCK_MANAGEMENT_H__

#include <stdint.h>

void SetParticipateInClockStateManagement(bool enable);
bool ParticipateInClockStateManagement();

#endif // __CLOCK_MANAGEMENT_H__

