/*
 * Copyright (C) 2007-2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <string.h>
#include <sys/types.h>

int
memcmp(const void *cs, const void *ct, size_t count)
{
	const unsigned char *su1, *su2;
	signed char res = 0;

	for(su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if((res = *su1 - *su2) != 0)
			break;
	return res;
}

/*
 * memcmp_secure
 *
 * memcmp with constant execution time for a given compare length.
 */
int
memcmp_secure(const void *cs, const void *ct, size_t count)
{
	const unsigned char *su1, *su2;
	unsigned char res = 0;

	for(su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		res |= *su1 ^ *su2;

	return res ? 1 : 0;
}
