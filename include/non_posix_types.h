/*
 * Copyright (C) 2010-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

/**
 * \file	non_posix_types.h
 */

#ifndef __NON_POSIX_TYPES_H
#define __NON_POSIX_TYPES_H

#include <stdint.h>

typedef uint64_t		utime_t;
#define INFINITE_UTIME		0xffffffffffffffffULL

#if defined(__LP64__)
typedef uint64_t 		addr_t;
#else
typedef uint32_t		addr_t;
#endif

#define MB			0x100000

/* just to make NAND code happy */
typedef int8_t			Int8;
typedef int16_t			Int16;
typedef int32_t			Int32;
typedef int64_t			Int64;

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof(x[0]))

#endif /* __NON_POSIX_TYPES_H */
