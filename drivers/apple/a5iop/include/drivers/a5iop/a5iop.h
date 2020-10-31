/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __A5IOP_H
#define __A5IOP_H

#include <sys/types.h>

__BEGIN_DECLS

extern void	a5iop_init(void);
extern void	a5iop_cache_operation(int operation, void *address, u_int32_t length);
extern void	a5iop_sleep(u_int32_t going_idle);

__END_DECLS

#endif
