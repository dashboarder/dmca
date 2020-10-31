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
#ifndef __PLATFORM_SOC_MIU_H
#define __PLATFORM_SOC_MIU_H

#include <lib/devicetree.h>
#include <platform/soc/hwregbase.h>

#define rMCCLOCKREGION_TZ0BASEADDR	*(volatile uint32_t *)(AMC_BASE_ADDR + 0x40)
#define rMCCLOCKREGION_TZ0ENDADDR	*(volatile uint32_t *)(AMC_BASE_ADDR + 0x44)
#define rMCCLOCKREGION_TZ0LOCK		*(volatile uint32_t *)(AMC_BASE_ADDR + 0x48)

// CPU Fabric Widget Registers (base address @ CPU_FABRIC_BASE_ADDR)
#define  CPU_Fabric_pl301Wrap0_AMCRDRATELIMIT		(0x0000)	// AMC Read Rate Limit Register
#define  CPU_Fabric_pl301Wrap0_AMCWRALIMIT		(0x0004)	// AMC Write Rate Limit Register
#define  CPU_Fabric_pl301Wrap0_AMCRTRLIMIT		(0x0008)	// AMC Read Transactions Limit Register
#define  CPU_Fabric_pl301Wrap0_AMCWTRLIMIT		(0x000c)	// AMC Write Transactions Limit Register
#define  CPU_Fabric_pl301Wrap0_SPURDRATELIMIT		(0x0040)	// SPU Read Rate Limit Register
#define  CPU_Fabric_pl301Wrap0_SPUWRALIMIT		(0x0044)	// SPU Write Rate Limit Register
#define  CPU_Fabric_pl301Wrap0_SPURTRLIMIT		(0x0048)	// SPU Read Transactions Limit Register
#define  CPU_Fabric_pl301Wrap0_SPUWTRLIMIT		(0x004c)	// SPU Write Transactions Limit Register
#define  CPU_Fabric_pl301Wrap0_SPUWGATHER		(0x0050)	// SPU Write Gather Register
#define  CPU_Fabric_pl301Wrap0_LIOWGATHER		(0x0090)	// LIO Write Gather Register
#define  CPU_Fabric_pl301Wrap0_AUERDRATELIMIT		(0x00c0)	// AUE Read Rate Limit Register
#define  CPU_Fabric_pl301Wrap0_AUEWRALIMIT		(0x00c4)	// AUE Write Rate Limit Register
#define  CPU_Fabric_pl301Wrap0_AUERTRLIMIT		(0x00c8)	// AUE Read Transactions Limit Register
#define  CPU_Fabric_pl301Wrap0_AUEWTRLIMIT		(0x00cc)	// AUE Write Transactions Limit Register
#define  CPU_Fabric_pl301Wrap0_AUEWGATHER		(0x00d0)	// AUE Write Gather Register
#define  CPU_Fabric_pl301Wrap0_ANSRDRATELIMIT		(0x0100)	// ANS Read Rate Limit Register
#define  CPU_Fabric_pl301Wrap0_ANSWRALIMIT		(0x0104)	// ANS Write Rate Limit Register
#define  CPU_Fabric_pl301Wrap0_ANSRTRLIMIT		(0x0108)	// ANS Read Transactions Limit Register
#define  CPU_Fabric_pl301Wrap0_ANSWTRLIMIT		(0x010c)	// ANS Write Transactions Limit Register
#define  CPU_Fabric_pl301Wrap0_ANSWGATHER		(0x0110)	// ANS Write Gather Register
#define  CPU_Fabric_pl301Wrap0_AXI0_ARCHANARBMI0	(0x0408)	// Configured AR channel arbitration value for MI 0

// NRT Fabric Widget Registers (base address @ NRT_FABRIC_BASE_ADDR)
#define NRT_Fabric_pl301Wrap1_AMCRDRATELIMIT		(0x0000)	// AMC Read Rate Limit Register
#define NRT_Fabric_pl301Wrap1_AMCWRALIMIT		(0x0004)	// AMC Write Rate Limit Register
#define NRT_Fabric_pl301Wrap1_AMCRTRLIMIT		(0x0008)	// AMC Read Transactions Limit Register
#define NRT_Fabric_pl301Wrap1_AMCWTRLIMIT		(0x000c)	// AMC Write Transactions Limit Register
#define NRT_Fabric_pl301Wrap1_MSRRDRATELIMIT		(0x0040)	// MSR Read Rate Limit Register
#define NRT_Fabric_pl301Wrap1_MSRWRALIMIT		(0x0044)	// MSR Write Rate Limit Register
#define NRT_Fabric_pl301Wrap1_MSRRTRLIMIT		(0x0048)	// MSR Read Transactions Limit Register
#define NRT_Fabric_pl301Wrap1_MSRWTRLIMIT		(0x004c)	// MSR Write Transactions Limit Register
#define NRT_Fabric_pl301Wrap1_SDIORDRATELIMIT		(0x0080)	// SDIO Read Rate Limit Register
#define NRT_Fabric_pl301Wrap1_SDIOWRALIMIT		(0x0084)	// SDIO Write Rate Limit Register
#define NRT_Fabric_pl301Wrap1_SDIORTRLIMIT		(0x0088)	// SDIO Read Transactions Limit Register
#define NRT_Fabric_pl301Wrap1_SDIOWTRLIMIT		(0x008c)	// SDIO Write Transactions Limit Register

#define rPIO_REMAP_CTL		*(volatile uint32_t *)(AOP_MINIPMGR_BASE_ADDR + MINIPMGR_SECURITY_PIOSYS_ROMADDRREMAP_OFFSET)

enum remap_select {
  REMAP_SRAM = 1,
  REMAP_SDRAM
};

extern void miu_select_remap(enum remap_select sel);
extern void miu_bypass_prep(void);
extern void miu_update_device_tree(DTNode *pmgr_node);

#endif /* ! __PLATFORM_SOC_MIU_H */
