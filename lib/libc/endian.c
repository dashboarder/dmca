/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <stdlib.h>

#if ARCH_ARMv7

u_int32_t swap32(u_int32_t x)
{
	u_int32_t y;

	__asm__ volatile("rev %0, %1" : "=r" (y) : "r" (x));

	return y;
}

u_int16_t swap16(u_int16_t x)
{
	u_int16_t y;

	__asm__ volatile("rev16 %0, %1" : "=r" (y) : "r" (x));

	return y;
}

#else

u_int32_t swap32(u_int32_t x)
{
	return ((((x) << 24) & 0xff000000) | (((x) << 8) & 0x00ff0000) | (((x) >> 8) & 0x0000ff00) | (((x) >> 24) & 0x000000ff));
}

u_int16_t swap16(u_int16_t x)
{
	return ((((x) << 8) & 0xff00) | (((x) >> 8) & 0x00ff));
}

#endif

u_int32_t htonl(u_int32_t hostlong)
{
	return swap32(hostlong);
}

u_int32_t ntohl(u_int32_t netlong)
{
	return swap32(netlong);
}

u_int16_t htons(u_int16_t hostshort)
{
	return swap16(hostshort);
}

u_int16_t ntohs(u_int16_t netshort)
{
	return swap16(netshort);
}
