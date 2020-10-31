/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __ARM7M_H
#define __ARM7M_H

#include <sys/types.h>

__BEGIN_DECLS

extern int	arm7m_init(void);
extern void	arm7m_halt(void);
extern void	arm7m_cache_operation(int operation, void *address, u_int32_t length);

__END_DECLS

#endif
