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

#include <sys/types.h>
#include <stdlib.h>

/*
 * Minimal new/delete.
 */
void *
operator new(size_t size) throw()
{
	return(malloc(size));
}

void *
operator new[](size_t size) throw()
{
	return(malloc(size));
}

void
operator delete(void *ptr) throw()
{
	free(ptr);
}

void
operator delete[](void *ptr) throw()
{
	free(ptr);
}
