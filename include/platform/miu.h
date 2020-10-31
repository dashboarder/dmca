/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_MIU_H
#define __PLATFORM_MIU_H

#include <sys/types.h>

__BEGIN_DECLS

int miu_init(void);
int miu_initialize_dram(bool resume);
int miu_initialize_internal_ram(void);
void miu_suspend(void);

__END_DECLS

#endif
