/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <platform/soc/hwregbase.h>
#include <platform/soc/reconfig.h>
#include <sys.h>

#define ENCODE_RECONFIG_CONFIG(start, count) ((((count) & 0x3ff) << 16) | ((start & 0x3ff) << 0))


typedef struct {
	uint32_t mem_reconfig_count;
	uint32_t soc_reconfig_count;
	uint32_t akf_reconfig_count;
} reconfig_stat_t;

static reconfig_stat_t reconfig_stat;

void reconfig_append_command(reconfig_type_t type, uint32_t cmd, uint32_t data, uint32_t mask)
{
	uint32_t reconfig_offset = 0;
	uint32_t *reconfig_count_ptr = NULL;

	// First, a basic check that reconfig is being programmed in the right order (mem -> soc -> akf)
	switch (type)
	{
	case RECONFIG_TYPE_MEM:
		ASSERT(reconfig_stat.soc_reconfig_count == 0);
		ASSERT(reconfig_stat.akf_reconfig_count == 0);
		reconfig_offset = reconfig_stat.mem_reconfig_count;
		reconfig_count_ptr = &reconfig_stat.mem_reconfig_count;
		break;
	case RECONFIG_TYPE_SOC:
		ASSERT(reconfig_stat.mem_reconfig_count != 0);
		ASSERT(reconfig_stat.akf_reconfig_count == 0);
		reconfig_offset = reconfig_stat.mem_reconfig_count + reconfig_stat.soc_reconfig_count;
		reconfig_count_ptr = &reconfig_stat.soc_reconfig_count;
		break;
	case RECONFIG_TYPE_AKF:
		ASSERT(reconfig_stat.mem_reconfig_count != 0);
		ASSERT(reconfig_stat.soc_reconfig_count != 0);
		reconfig_offset = reconfig_stat.mem_reconfig_count + reconfig_stat.soc_reconfig_count + reconfig_stat.akf_reconfig_count;
		reconfig_count_ptr = &reconfig_stat.akf_reconfig_count;
		break;
	default:
		panic("Unknown reconfig type: %d", type);
	}

	if(reconfig_offset >= RECONFIG_RAM_SIZE_IN_WORDS - 1)
	{
		panic("Out of reconfig space!");
	}

	rRECONFIG_RAM_CMD(reconfig_offset) = cmd;
	rRECONFIG_RAM_DATA(reconfig_offset) = data;
	rRECONFIG_RAM_MASK(reconfig_offset) = mask;

	(*reconfig_count_ptr)++;
}

void reconfig_commit()
{
	// Encode start/range
	rRECONFIG_CONFIG_MEM = ENCODE_RECONFIG_CONFIG(0, reconfig_stat.mem_reconfig_count);
	rRECONFIG_CONFIG_SOC = ENCODE_RECONFIG_CONFIG(reconfig_stat.mem_reconfig_count, reconfig_stat.soc_reconfig_count);
	rRECONFIG_CONFIG_AKF = ENCODE_RECONFIG_CONFIG(reconfig_stat.mem_reconfig_count + reconfig_stat.soc_reconfig_count, reconfig_stat.akf_reconfig_count);


	// Enable Reconfig engine
	rRECONFIG_CONFIGURATION |= (1 << 16);
}

void reconfig_dbg_dump_contents(void)
{
	printf("==== MEM RECONFIG COMMANDS ====\n");
	printf("INDEX\t\tCMD\t\tDATA\t\tMASK\n");
	for(uint32_t i = 0; i < reconfig_stat.mem_reconfig_count; i++)
	{
		printf("%d\t\t0x%08x\t\t0x%08x\t\t0x%08x\n", i, rRECONFIG_RAM_CMD(i), rRECONFIG_RAM_DATA(i), rRECONFIG_RAM_MASK(i));
	}
	printf("==== SOC RECONFIG COMMANDS ====\n");
	printf("INDEX\t\tCMD\t\tDATA\t\tMASK\n");
	for(uint32_t i = reconfig_stat.mem_reconfig_count; i < reconfig_stat.mem_reconfig_count + reconfig_stat.soc_reconfig_count; i++)
	{
		printf("%d\t\t0x%08x\t\t0x%08x\t\t0x%08x\n", i, rRECONFIG_RAM_CMD(i), rRECONFIG_RAM_DATA(i), rRECONFIG_RAM_MASK(i));
	}
	printf("==== AKF RECONFIG COMMANDS ====\n");
	printf("INDEX\t\tCMD\t\tDATA\t\tMASK\n");
	for(uint32_t i = reconfig_stat.mem_reconfig_count + reconfig_stat.soc_reconfig_count; i < reconfig_stat.mem_reconfig_count + reconfig_stat.akf_reconfig_count; i++)
	{
		printf("%d\t\t0x%08x\t\t0x%08x\t\t0x%08x\n", i, rRECONFIG_RAM_CMD(i), rRECONFIG_RAM_DATA(i), rRECONFIG_RAM_MASK(i));
	}
	printf("reconfig_commit: RECONFIG_CONFIG_MEM = 0x%x, CONFIG_SOC = 0x%x, CONFIG_AKF=0x%x\n", rRECONFIG_CONFIG_MEM, rRECONFIG_CONFIG_SOC, rRECONFIG_CONFIG_AKF);
}
