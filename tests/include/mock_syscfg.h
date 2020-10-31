/*
 * Copyright (C) 2014-2015 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef MOCK_SYSCFG_H
#define MOCK_SYSCFG_H

#include <stdint.h>

void mock_syscfg_add(uint32_t tag, void *data, uint32_t size);
void mock_syscfg_reset(void);

#endif
