/*
 * Copyright (C) 2007-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DRIVERS_SHA1_H
#define __DRIVERS_SHA1_H

#include <sys/types.h>
#include <sys/hash.h>

#if WITH_CORECRYPTO
#include <corecrypto/ccsha1.h>
#else
#define CCSHA1_OUTPUT_SIZE	(20)			// bytes
#endif

__BEGIN_DECLS

void sha1_calculate(const void *buffer, size_t length, void *result);
#if WITH_CORECRYPTO
const struct ccdigest_info *sha1_get_ccsha1_ccdigest_info();
#endif

__END_DECLS

#endif /* __DRIVERS_SHA1_H */
