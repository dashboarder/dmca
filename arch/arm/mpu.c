/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <arch.h>
#include <platform/memmap.h>
#include <sys.h>

void arm_mpu_init(void)
{
	/* set everything as fully cached, except for the hardware register region */
	arm_write_iprot_region_0(0x0000003f);
	arm_write_dprot_region_0(0x0000003f);
#ifdef PERIPH_BASE
	arm_write_dprot_region_1((PERIPH_BASE | (0x1F - ((PERIPH_LEN == 0) ? 0 : (__builtin_clz(PERIPH_LEN) + 1))) << 1) | 1);
#endif
	arm_write_ins_prot_register(0x3); // region 0 full access
	arm_write_data_prot_register(0xf); // region 0 & 1 full access
	arm_write_cacheable_registers(0x1, 0x1); // region 0 data and instruction is cacheable
	arm_write_bufferable_register(0x1); // data region 0 is write bufferable
}

