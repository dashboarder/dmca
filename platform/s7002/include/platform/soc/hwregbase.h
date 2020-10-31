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
#ifndef __PLATFORM_SOC_HWREGBASE_H
#define __PLATFORM_SOC_HWREGBASE_H

#include <platform/memmap.h>

/* S7002 Reg Base Defs */

// Memory Subsystem
#define SECUREROM_BASE_ADDR		(IO_BASE + 0x00000000)		// SecureROM access through PIO
#define PIO_BASE_ADDR			(IO_BASE + 0x00100000)		// PIO Config
#define CPU_FABRIC_BASE_ADDR		(IO_BASE + 0x00101000)		// PL301 Config
#define NRT_FABRIC_BASE_ADDR		(IO_BASE + 0x00102000)		// PL301 Config
#define CPU_BASE_ADDR			(IO_BASE + 0x00103000)		// AP Config
#define AMC_BASE_ADDR			(IO_BASE + 0x00200000)		// AMC Config
#define AMPS_DQ_BASE_ADDR		(IO_BASE + 0x00300000)		// AMP DQ DRAM Phy
#define AMPS_CA_BASE_ADDR		(IO_BASE + 0x00310000)		// AMP CA DRAM Phy

// MSR Subsystem
#define MSR_BASE_ADDR			(IO_BASE + 0x01000000)		// MSR Config
#define MSR_SMMU_BASE_ADDR		(IO_BASE + 0x01004000)		// MSR SMMU
#define MSR_DART_BASE_ADDR		(IO_BASE + 0x01008000)		// MSR DART
#define VDEC_BASE_ADDR			(IO_BASE + 0x02000000)		// VXD394 Video Decoder
#define SGX544_SC_BASE_ADDR		(IO_BASE + 0x02800000)		// SGX544-SC Space

// Display Subsystem
#define DISP_ADP_BASE_ADDR		(IO_BASE + 0x03200000)		// Display pipe configuration
#define DISP0_BASE_ADDR			DISP_ADP_BASE_ADDR
#define DISP_ADP_SMMU_BASE_ADDR		(IO_BASE + 0x03300000)		// Display pipe SMMU configuration
#define DISP_ADE_BASE_ADDR		(IO_BASE + 0x03400000)		// DBE Timing controller
#define	DISP0_ADBE_BASE_ADDR		DISP_ADE_BASE_ADDR
#define DISP_AAP_BASE_ADDR		(IO_BASE + 0x03440000)		// DBE Ambient-Adaptive Pixel
#define DISP_DITHER_BASE_ADDR		(IO_BASE + 0x034C0000)		// DBE Dither
#define DISP_MIPI_DSI_BASE_ADDR		(IO_BASE + 0x03600000)		// MIPI DSI Core configuration

// ANS Subsystem
#define ANS_PL301_BASE_ADDR		(IO_BASE + 0x04010000)		// ANS PL301 Configuration
#define ANS_PL301_TLIMIT0_BASE_ADDR	(IO_BASE + 0x04020000)		// ANS Cortex-A7 Transaction Limit
#define ANS_PL301_TLIMIT1_BASE_ADDR	(IO_BASE + 0x04030000)		// ANS NAND controoler Transaction Limit
#define ANS_AKF_BASE_ADDR		(IO_BASE + 0x04040000)		// ANS Cortex-A7 and wrapper
// ANS KF Debug registers
#define ANS_CFG_BASE_ADDR		(IO_BASE + 0x04080000)		// ANS Config
#define ANS_PPN_N_PL_BASE_ADDR	(IO_BASE + 0x04081000)
#define ANS_ANC_BASE_ADDR		(IO_BASE + 0x04800000)		// NAND Controller

// SDIO Subsystem
#define SDIO_PL301_BASE_ADDR		(IO_BASE + 0x05010000)		// SDIO PL301 Configuration
#define SDIO_PL301_TLIMIT0_BASE_ADDR	(IO_BASE + 0x05020000)		// SDIO Cortex-A7 Transaction Limit
#define SDIO_PL301_TLIMIT1_BASE_ADDR	(IO_BASE + 0x05030000)		// SDIO controller Transaction Limit
#define SDIO_AKF_BASE_ADDR		(IO_BASE + 0x05040000)		// SDIO Cortex-A7 and wrapper
// SDIO KF Debug registers
#define SDIO_CFG_BASE_ADDR		(IO_BASE + 0x05080000)		// SDIO Config

// SPU Subsystem
#define SPU_SRAM_A_BASE_ADDR		(IO_BASE + 0x06000000)		// SPU Local SRAM-A
#define SPU_SRAM_B_BASE_ADDR		(IO_BASE + 0x06040000)		// SPU Local SRAM-B
#define SPU_SRAM_SIZE				(0x48000)			// SPU SRAM size is 288 KB
#define SPU_CPU_PL301_BASE_ADDR		(IO_BASE + 0x06410000)		// SPU CPU PL301 Configuration
#define SPU_SRAM_PL301_BASE_ADDR	(IO_BASE + 0x06411000)		// SPU SRAM PL301 Configuration
#define SPU_MEM_PL301_BASE_ADDR		(IO_BASE + 0x06412000)		// SPU MEM PL301 Configuration
#define SPU_AKF_BASE_ADDR		(IO_BASE + 0x06500000)		// SPU Cortex-A7 and wrapper
// SPU KF Debug registers
#define SPU_SPI_BASE_ADDR		(IO_BASE + 0x06600000)		// SPU SPI Device
#define SPU_SEQUENCER_BASE_ADDR		(IO_BASE + 0x06700000)		// SPU Sequencer configuration
#define SPU_SMB_BASE_ADDR		(IO_BASE + 0x06800000)		// SPU I2C 
#define SPU_GPIO_BASE_ADDR		(IO_BASE + 0x06810000)		// SPU GPIO
#define SPU_UART_BASE_ADDR		(IO_BASE + 0x06820000)		// SPU UART
#define SPU_SOC_RECONFIG_BASE_ADDR	(IO_BASE + 0x06900000)		// SPU SOC ReConfig
#define SPU_AIC_WRAP_BASE_ADDR		(IO_BASE + 0x06A00000)		// AIC 
#define SPU_AIC_BASE_ADDR		(IO_BASE + 0x06B00000)		// AIC 
#define SPU_PMGR_BASE_ADDR		(IO_BASE + 0x06C00000)		// PMGR
#define SPU_DBG_FIFO_0_BASE_ADDR	(IO_BASE + 0x06E00000)		// Debug FIFO [0] registers
#define SPU_DBG_FIFO_1_BASE_ADDR	(IO_BASE + 0x06E03000)		// Debug FIFO [1] registers
#define SPU_DBG_FIFO_2_BASE_ADDR	(IO_BASE + 0x06E06000)		// Debug FIFO [2] registers
#define SPU_DBG_FIFO_3_BASE_ADDR	(IO_BASE + 0x06E09000)		// Debug FIFO [3] registers
#define SPU_DBG_FIFO_4_BASE_ADDR	(IO_BASE + 0x06E0C000)		// Debug FIFO [4] registers
#define SPU_DBG_FIFO_5_BASE_ADDR	(IO_BASE + 0x06E0F000)		// Debug FIFO [5] registers
#define SPU_DBG_FIFO_6_BASE_ADDR	(IO_BASE + 0x06E12000)		// Debug FIFO [6] registers
#define SPU_DBG_FIFO_7_BASE_ADDR	(IO_BASE + 0x06E15000)		// Debug FIFO [7] registers
#define SPU_DBG_WRAP_BASE_ADDR		(IO_BASE + 0x06EB0000)		// Debug sub-system config space

// LIO Subsystem
#define LIO_MISC_BASE_ADDR		(IO_BASE + 0x07000000)		// Miscellaneous registers
#define LIO_AUD_MUX_BASE_ADDR		(IO_BASE + 0x07004000)		// Audio I2S Switch registers
#define LIO_PWM_BASE_ADDR		(IO_BASE + 0x07010000)		// PWM Device registers
#define LIO_SMB_0_BASE_ADDR		(IO_BASE + 0x07020000)		// I2C Device registers
#define LIO_GPIO_BASE_ADDR		(IO_BASE + 0x07100000)		// GPIO
#define LIO_SPI_BASE_ADDR		(IO_BASE + 0x07200000)		// SPI Device registers
#define LIO_UART_BASE_ADDR		(IO_BASE + 0x07230000)		// UART Device registers
#define LIO_MCA_BASE_ADDR		(IO_BASE + 0x07240000)		// MCA Device registers
#define LIO_DMAC_BASE_ADDR		(IO_BASE + 0x07300000)		// PL080 DMAC registers
#define LIO_AES_BASE_ADDR		(IO_BASE + 0x07380000)		// AES

// AUE Subsystem
#define AUE_USBCONT_BASE_ADDR		(IO_BASE + 0x08000000)		// USB Control
#define AUE_USBOTG_BASE_ADDR		(IO_BASE + 0x08100000)		// USBOTG
#define AUE_AUSB_WIDGETS_BASE_ADDR	(IO_BASE + 0x08200000)		// AUSB Widgets
#define AUE_AUSB_AFIFO_WDGTS_BASE_ADDR	(IO_BASE + 0x08300000)		// AUSB Async Fifo Widgets
#define AUE_PL301_BASE_ADDR		(IO_BASE + 0x08800000)		// AUE PL301 Configuration

/* iBoot Specific Defs */

#define AKF_VERSION			(1)

#define UART0_BASE_ADDR			(LIO_UART_BASE_ADDR)
#define UART1_BASE_ADDR			(LIO_UART_BASE_ADDR + 0x04000)
#define UART2_BASE_ADDR			(LIO_UART_BASE_ADDR + 0x08000)
#define UART3_BASE_ADDR			(LIO_UART_BASE_ADDR + 0x0C000)
#define UART4_BASE_ADDR			(LIO_UART_BASE_ADDR + 0x10000)
#define UARTS_COUNT			(5)
#define UART_VERSION			(1)

#define SPI0_BASE_ADDR			(LIO_SPI_BASE_ADDR)
#define SPI1_BASE_ADDR			(LIO_SPI_BASE_ADDR + 0x04000)
#define SPI_VERSION			(1)
#define SPIS_COUNT			(2)

#define IIC_BASE_ADDR			(LIO_SMB_0_BASE_ADDR)
#define IIC_SPACING			(0x00004000)
#define IICS_COUNT			(2)

#define DSIM_BASE_ADDR			(DISP_MIPI_DSI_BASE_ADDR)
#define DSIM_LANE_COUNT			(1)
#define DSIM_VERSION			(3)

#define ADBE_DISPLAYPIPE_BASE_ADDR	(DISP0_BASE_ADDR)
#define DISP_VERSION			(5)
#define ADP_VERSION			(1)

#define AMP_BASE_ADDR			(AMPS_DQ_BASE_ADDR)
#define AMP_SPACING			(0)
#define AMP_CA_SPACING			(0x00010000)

#if !CONFIG_SIM
#define AMP_REG_OFFSET			(4)
#else
 // FIXME M7 fastsim has an Alcatraz-y AMP until <rdar://problem/15464244> M7: update AMC/AMP for M7
#define AMP_REG_OFFSET			(0)
#endif

#define PMGR_BASE_ADDR			(SPU_PMGR_BASE_ADDR)
#define CHIPID_BASE_ADDR		(PMGR_BASE_ADDR + 0x2A000)
#define PMGR_WDG_VERSION		(1)

#define GPIOC_COUNT			(2)
#define GPIO_VERSION			(4)
#define GPIO_BASE_ADDR			(LIO_GPIO_BASE_ADDR)
#define GPIO_0_GROUP_COUNT		(18)
#define GPIO_GROUP_COUNT		(GPIO_0_GROUP_COUNT)
#define GPIO_1_BASE_ADDR		(SPU_GPIO_BASE_ADDR)
#define GPIO_1_GROUP_COUNT		(10)
#define GPIO_AP				(GPIOC_0)
#define GPIO_SPU			(GPIOC_1)

#define AIC_BASE_ADDR			(SPU_AIC_BASE_ADDR)
#define AIC_VERSION			(1)
#define AIC_INT_COUNT			(192)

#define PL080DMAC_BASE_ADDR		(LIO_DMAC_BASE_ADDR)
#define PL080DMAC_SPACING		(0x4000)
#define PL080DMAC_COUNT			(2)
#define PL080DMAC_CHANNEL_COUNT		(8)

#define USBOTG_BASE_ADDR		(AUE_USBOTG_BASE_ADDR)
#define AUSB_CTL_REG_BASE_ADDR		(AUE_USBCONT_BASE_ADDR)
#define AUSB_CTL_USB20PHY_REG_OFFSET	(0x30)
#define AUSB_USB20PHY_ONLY_VERSION	(1)
#define AUSB_PL301_WIDGET_BASE_ADDR	(AUE_PL301_BASE_ADDR)

#define AES_S7002_BASE_ADDR		(LIO_AES_BASE_ADDR)
#define AES_S7002_VERSION		(1)

#define DBGFIFO_0_BASE_ADDR		(SPU_DBG_FIFO_0_BASE_ADDR)
#define DBGFIFO_SPACING			(0x3000)
#define DBGFIFO_COUNT			(8)

#define RECONFIG_RAM_SIZE_IN_WORDS	(1024)

#endif  /* ! __PLATFORM_SOC_HWREGBASE_H */
