/*
 * Copyright (C) 2007-2009 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __LIB_LIBIBOOT_H
#define __LIB_LIBIBOOT_H

#include <sys/types.h>

__BEGIN_DECLS

/* for powers of 2 only */
#define ROUNDUP(a, b) (((a) + ((b) - 1)) & (~((b) - 1)))
#define ROUNDDOWN(a, b) (((a) / (b)) * (b))

/* some of our own stuff */
void hexdump(const void *_ptr, size_t len);

#define is_pow2(n) ((((n)-1) & (n)) == 0)
int log2_int(unsigned int val);

__END_DECLS

#endif
