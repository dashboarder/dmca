/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <drivers/ccc/ccc.h>
#include <platform/soc/hwregbase.h>

void ccc_override_and_lock_iorvbar(addr_t base_address)
{
	rCCC_CPU0_IMPL_IORVBAR = ((base_address >> 11) << 11) | (1 << 0);
	rCCC_CPU1_IMPL_IORVBAR = ((base_address >> 11) << 11) | (1 << 0);

#if SUB_PLATFORM_T7001
	rCCC_CPU2_IMPL_IORVBAR = ((base_address >> 11) << 11) | (1 << 0);

	dprintf(DEBUG_INFO, "IORVBAR:: CPU0:0x%016llx CPU1:0x%016llx CPU2:0x%016llx\n", 
			rCCC_CPU0_IMPL_IORVBAR, rCCC_CPU1_IMPL_IORVBAR, rCCC_CPU2_IMPL_IORVBAR);
#else 
	dprintf(DEBUG_INFO, "IORVBAR:: CPU0:0x%016llx CPU1:0x%016llx\n", rCCC_CPU0_IMPL_IORVBAR, rCCC_CPU1_IMPL_IORVBAR);
#endif /* SUB_PLATFORM_T7001 */
}

void ccc_handle_asynchronous_exception(void)
{
	dprintf(DEBUG_CRITICAL, "CPU0_IMPL: \n");
	dprintf(DEBUG_CRITICAL, "\tFED_ERR_STS:0x%016llx LSU_ERR_STS:0x%016llx MMU_ERR_STS:0x%016llx\n",
					rCCC_CPU0_IMPL_FED_ERR_STS, rCCC_CPU0_IMPL_LSU_ERR_STS, rCCC_CPU0_IMPL_MMU_ERR_STS);
#if defined(rCCC_CPU0_IMPL_MIGSTS_EL1)
	dprintf(DEBUG_CRITICAL, "\tE_FED_ERR_STS:0x%016llx E_LSU_ERR_STS:0x%016llx MIGSTS_EL1:0x%016llx\n",
					rCCC_CPU0_IMPL_E_FED_ERR_STS, rCCC_CPU0_IMPL_E_LSU_ERR_STS, rCCC_CPU0_IMPL_MIGSTS_EL1);
#endif
	dprintf(DEBUG_CRITICAL, "\tL2C_ERR_STS:0x%016llx  L2C_ERR_ADR:0x%016llx L2C_ERR_INF:0x%016llx\n", 
					rCCC_CCC_IMPL_L2C_ERR_STS, rCCC_CCC_IMPL_L2C_ERR_ADR, rCCC_CCC_IMPL_L2C_ERR_INF);
}

void ccc_enable_custom_errors(void)
{
	// rCCC_CPU0_IMPL_CPU_IOACC_CTL_EL3 |= (0x1ULL << 0);	/* enable ns access to CPU_IMPL and HID */
	// rCCC_CCC_IMPL_CPM_IOACC_CTL_EL3 |= (0x7ULL << 0);	/* enable ns access to CPM IMPL, HID, CNTCTL, and CNTRD 
	// 							   CPM Thermal,  CPM APSC, DVFM, PLL, and PSW */
	rCCC_CCC_IMPL_L2C_ERR_STS &= ~(0xffffffffULL);		/* clear old status bits */
	rCCC_CCC_IMPL_L2C_ERR_ADR = 0;				/* clear */
	rCCC_CCC_IMPL_L2C_ERR_INF = 0;				/* clear */
	rCCC_CCC_IMPL_L2C_ERR_STS |= (1ULL << 41);		/* enable PIOErr */
}

void ccc_disable_custom_errors(void)
{
	rCCC_CCC_IMPL_L2C_ERR_STS &= ~(1ULL << 41);		/* disable PIOErr */
}

void ccc_dump_registers(void)
{
//	dprintf(DEBUG_INFO, "CPU_IOACC_CTL_EL3:0x%016llx, CPM_IOACC_CTL_EL3:0x%016llx\n", rCCC_CPU0_IMPL_CPU_IOACC_CTL_EL3, rCCC_CCC_IMPL_CPM_IOACC_CTL_EL3);
	dprintf(DEBUG_INFO, "FED_ERR_STS:0x%016llx  LSU_ERR_STS:0x%016llx MMU_ERR_STS:0x%016llx\n", 
					rCCC_CPU0_IMPL_FED_ERR_STS, rCCC_CPU0_IMPL_LSU_ERR_STS, rCCC_CPU0_IMPL_MMU_ERR_STS);
	dprintf(DEBUG_INFO, "L2C_ERR_STS:0x%016llx  L2C_ERR_ADR:0x%016llx L2C_ERR_INF:0x%016llx\n", 
					rCCC_CCC_IMPL_L2C_ERR_STS, rCCC_CCC_IMPL_L2C_ERR_ADR, rCCC_CCC_IMPL_L2C_ERR_INF);
	dprintf(DEBUG_INFO, "CPU0_IORVBAR:0x%016llx CPU1_IORVBAR:0x%016llx\n", rCCC_CPU0_IMPL_IORVBAR, rCCC_CPU1_IMPL_IORVBAR);
}
