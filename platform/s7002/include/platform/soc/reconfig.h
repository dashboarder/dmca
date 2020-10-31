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
#ifndef __PLATFORM_RECONFIG_H
#define __PLATFORM_RECONFIG_H

#include <sys/types.h>

#define rRECONFIG_CONFIGURATION  	*(volatile uint32_t *)(SPU_SOC_RECONFIG_BASE_ADDR + 0x0000)
#define rRECONFIG_CONFIG_MEM  		*(volatile uint32_t *)(SPU_SOC_RECONFIG_BASE_ADDR + 0x0004)
#define rRECONFIG_TIMEOUT_MEM 		*(volatile uint32_t *)(SPU_SOC_RECONFIG_BASE_ADDR + 0x0008)
#define rRECONFIG_INTERRUPT_MEM 	*(volatile uint32_t *)(SPU_SOC_RECONFIG_BASE_ADDR + 0x000C)
#define rRECONFIG_CONFIG_SOC  		*(volatile uint32_t *)(SPU_SOC_RECONFIG_BASE_ADDR + 0x0010)
#define rRECONFIG_TIMEOUT_SOC 		*(volatile uint32_t *)(SPU_SOC_RECONFIG_BASE_ADDR + 0x0014)
#define rRECONFIG_INTERRUPT_SOC 	*(volatile uint32_t *)(SPU_SOC_RECONFIG_BASE_ADDR + 0x0018)
#define rRECONFIG_CONFIG_AKF  		*(volatile uint32_t *)(SPU_SOC_RECONFIG_BASE_ADDR + 0x001C)
#define rRECONFIG_TIMEOUT_AKF 		*(volatile uint32_t *)(SPU_SOC_RECONFIG_BASE_ADDR + 0x0020)
#define rRECONFIG_INTERRUPT_AKF 	*(volatile uint32_t *)(SPU_SOC_RECONFIG_BASE_ADDR + 0x0024)


#define rRECONFIG_RAM_CMD(i)  		*(volatile uint32_t *)(SPU_SOC_RECONFIG_BASE_ADDR + 0x1000 + (i * 4))
#define rRECONFIG_RAM_DATA(i) 		*(volatile uint32_t *)(SPU_SOC_RECONFIG_BASE_ADDR + 0x2000 + (i * 4))
#define rRECONFIG_RAM_MASK(i) 		*(volatile uint32_t *)(SPU_SOC_RECONFIG_BASE_ADDR + 0x3000 + (i * 4))

#define RECONFIG_RAM_CMD_READ		(1 << 0)

// Removing this debug region now that reconfig engine/code seems to be working fine
// Do not use this space since SPU may have reclaimed it
#if 0
// last page of SPU SRAM allocated to RECONFIG for reconfig debug purposes
#define RECONFIG_DEBUG_SPACE		(SPU_SRAM_A_BASE_ADDR + SPU_SRAM_SIZE - PAGE_SIZE)
#endif

typedef enum {RECONFIG_TYPE_MEM, RECONFIG_TYPE_SOC, RECONFIG_TYPE_AKF} reconfig_type_t;


/**
 * Adds a reconfig command of the given type to the reconfig buffer
 *
 * Note that the reconfig driver expects you to program in all the RECONFIG_TYPE_MEM commands,
 * followed by SOC commands, followed by AKF commands. The driver will panic if you attempt to program
 * the block in a different sequence.
 * 
 **/
void reconfig_append_command(reconfig_type_t type, uint32_t cmd, uint32_t data, uint32_t mask);

/**
 * Program the RECONFIG_CONFIG_{SOC/MEM/AKF} registers with the start / counts, and enables the reconfig block.
 *
 * Call this after you've fully reprogrammed the reconfig memory using reconfig_append_command()
 **/
void reconfig_commit();


/**
 * Prints the contents of the reconfig buffer via printf. For debugging purposes only
 **/
void reconfig_dbg_dump_contents(void);

#endif /* __PLATFORM_RECONFIG_H */
