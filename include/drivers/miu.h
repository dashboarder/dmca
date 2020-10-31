/*
 * Copyright (C) 2007-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DRIVERS_MIU_H
#define __DRIVERS_MIU_H

#include <sys/types.h>

__BEGIN_DECLS

int  mcu_initialize_dram(bool resume);
int  mcu_suspend_dram(void);
void mcu_adjust_performance(void);
u_int64_t mcu_get_memory_size(void);

__END_DECLS

#endif /* __DRIVERS_MIU_H */
