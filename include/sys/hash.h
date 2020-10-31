/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple, Inc.
 */
#ifndef __SYS_HASH_H
#define __SYS_HASH_H

#include <sys/types.h>

__BEGIN_DECLS

#if WITH_SHA2_384
#if !WITH_CORECRYPTO && !HOST_TEST
#error "WITH_SHA2_384 requires WITH_CORECRYPTO"
#endif

#include <corecrypto/ccsha2.h>

#define HASH_OUTPUT_SIZE	CCSHA384_OUTPUT_SIZE	// bytes

#else   // Assume SHA1

#include <drivers/sha1.h>

#define HASH_OUTPUT_SIZE	CCSHA1_OUTPUT_SIZE	// bytes

#endif

void hash_calculate(const void *in_ptr, size_t in_len, void *out_ptr, size_t out_len);

__END_DECLS

#endif
