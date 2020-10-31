/*
 * Copyright (C) 2009-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _CLOCK_STEPPING_H_
#define _CLOCK_STEPPING_H_

typedef enum
{
	kClockRequestMessageProcess,
	kClockRequestPowerManager,
	kClockRequestLoopback,
	kClockRequestMax
} clockrequest_reason_t;

typedef enum
{
	kClockValueLow,
	kClockValueHigh,
	kClockValueMax
} clockvalue_t;

void SetClockState(clockrequest_reason_t reason, clockvalue_t value);

#endif // _CLOCK_STEPPING_H_

