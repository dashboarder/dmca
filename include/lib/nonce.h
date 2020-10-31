/*
 * Copyright (c) 2012, 2014-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _NONCE_H_
#define _NONCE_H_

// =============================================================================

#include <sys/hash.h>
#include <sys/types.h>

#if HASH_OUTPUT_SIZE > 32	// Bytes
#define NONCE_HASH_OUTPUT_SIZE	(32)
#else
#define NONCE_HASH_OUTPUT_SIZE	HASH_OUTPUT_SIZE
#endif

int mobile_ap_nonce_consume_nonce(uint64_t *nonce);

#endif /* _NONCE_H_ */
