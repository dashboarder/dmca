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

#include "clock_management.h"
#include <sys.h>

static bool sParticipateInClockManagement = false;

void SetParticipateInClockStateManagement(bool enable)
{
	sParticipateInClockManagement = enable;
}

bool ParticipateInClockStateManagement()
{
	return sParticipateInClockManagement;
}

