/*
 * Copyright (C) 2007-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#define FASTSIM_H2FMI_BLOCK_PER_PAGE	(3)
#define FASTSIM_H2FMI_BLOCK_SIZE 	(512)
#define FASTSIM_H2FMI_BLOCK_NUMBER 	(16384)

/* Register base and offsets address used for FastSim */
#define FASTSIM_H2FMI_BASE_ADDR		(0x31200000)

#define rFASTSIM_H2FMI_DATA_BUF		(*(volatile u_int32_t *)(FASTSIM_H2FMI_BASE_ADDR + 0x00014))
#define rFASTSIM_H2FMI_DEBUG0		(*(volatile u_int32_t *)(FASTSIM_H2FMI_BASE_ADDR + 0x0001C))
#define rFASTSIM_H2FMI_CMD		(*(volatile u_int32_t *)(FASTSIM_H2FMI_BASE_ADDR + 0x40014))
#define rFASTSIM_H2FMI_ADDR0		(*(volatile u_int32_t *)(FASTSIM_H2FMI_BASE_ADDR + 0x40018))
#define rFASTSIM_H2FMI_ADDR1		(*(volatile u_int32_t *)(FASTSIM_H2FMI_BASE_ADDR + 0x4001C))
#define rFASTSIM_H2FMI_ADDRNUM		(*(volatile u_int32_t *)(FASTSIM_H2FMI_BASE_ADDR + 0x40020))
#define rFASTSIM_H2FMI_STATUS		(*(volatile u_int32_t *)(FASTSIM_H2FMI_BASE_ADDR + 0x40044))

/* CMD */
#define FASTSIM_H2FMI_RESET	(0xFF)

