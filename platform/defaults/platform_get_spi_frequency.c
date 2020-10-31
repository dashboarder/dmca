/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <platform.h>

u_int32_t platform_get_spi_frequency(void)
{
#if SUPPORT_FPGA
	return 2500000;
#else
	return 12000000;
#endif
}

