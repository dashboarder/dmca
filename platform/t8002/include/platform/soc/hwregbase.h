/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
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

/* T8002 Reg Base Defs */

// Memory Subsystem
#define SECUREROM_BASE_ADDR		(IO_BASE + 0x00000000)		// SecureROM access through PIO
#define PIO_BASE_ADDR			(IO_BASE + 0x00100000)		// PIO Config
#define CPU_FABRIC_BASE_ADDR		(IO_BASE + 0x00101000)		// CPU Fabric Config
#define NRT_FABRIC_BASE_ADDR		(IO_BASE + 0x00102000)		// NRT Fabric Config
#define RT_FABRIC_BASE_ADDR		(IO_BASE + 0x00103000)		// RT Fabric Config

#define DCS_REG_VERSION			(0x10)

// AMCSYS Config
#define AMC_BASE_ADDR			(IO_BASE + 0x00200000)		// AMC Config
#define NUM_AMCCS			(0)

#define DCS_BASE_ADDR			(IO_BASE + 0x00300000) 	// DCS

#define DCS_SPACING 			(0x40000)
#define SPLLCTRL_SPACING 		(0x0000C000)
#define DCSH_AON_SPACING 		(0x00010000)
#define AMP_CA_SPACING 		(0x00020000)
#define AMP_DQ_SPACING 		(0x00008000)
#define AMP_SPACING 			(DCS_SPACING)
#define AMP_BASE_ADDR 			(DCS_BASE_ADDR + AMP_CA_SPACING)
#define AMPH_BASE_ADDR 		(AMP_BASE_ADDR + 3 * AMP_DQ_SPACING)

// MSR Subsystem
#define MSR_BASE_ADDR			(IO_BASE + 0x01100000)		// MSR Config
#define MSR_SMMU_BASE_ADDR		(IO_BASE + 0x01104000)		// MSR SMMU
#define MSR_DART_BASE_ADDR		(IO_BASE + 0x01108000)		// MSR DART
#define VDEC_BASE_ADDR			(IO_BASE + 0x01200000)		// VXD394 Video Decoder
//XXX
//#define SGX544_SC_BASE_ADDR		(IO_BASE + 0x02800000)		// SGX544-SC Space

// Display Subsystem
#define DISP_ADP_BASE_ADDR		(IO_BASE + 0x03200000)		// Display pipe configuration
#define DISP0_BASE_ADDR			DISP_ADP_BASE_ADDR
#define DISP_ADP_SMMU_BASE_ADDR		(IO_BASE + 0x03300000)		// Display pipe SMMU configuration
#define DISP_ADE_BASE_ADDR		(IO_BASE + 0x03400000)		// DBE Timing controller
#define	DISP0_ADBE_BASE_ADDR		DISP_ADE_BASE_ADDR
//XXX
//#define DISP_AAP_BASE_ADDR		(IO_BASE + 0x03440000)		// DBE Ambient-Adaptive Pixel
#define DISP_DITHER_BASE_ADDR		(IO_BASE + 0x03540000)		// DBE Dither
#define DISP_MIPI_DSI_BASE_ADDR		(IO_BASE + 0x03800000)		// MIPI DSI Core configuration

// ANS Subsystem
#define ANS_PL301_BASE_ADDR		(IO_BASE + 0x05010000)		// ANS PL301 Configuration
#define ANS_PL301_TLIMIT0_BASE_ADDR	(IO_BASE + 0x05020000)		// ANS Cortex-A7 Transaction Limit
#define ANS_PL301_TLIMIT1_BASE_ADDR	(IO_BASE + 0x05030000)		// ANS NAND controoler Transaction Limit
#define ANS_AKF_BASE_ADDR		(IO_BASE + 0x05040000)		// ANS Cortex-A7 and wrapper
// ANS KF Debug registers
#define ANS_CFG_BASE_ADDR		(IO_BASE + 0x05080000)		// ANS Config
#define ANS_PPN_N_PL_BASE_ADDR		(IO_BASE + 0x05081000)
#define ANS_ANC_BASE_ADDR		(IO_BASE + 0x05800000)		// NAND Controller

// SDIO Subsystem
#define SDIO_PL301_BASE_ADDR		(IO_BASE + 0x06010000)		// SDIO PL301 Configuration
#define SDIO_PL301_TLIMIT0_BASE_ADDR	(IO_BASE + 0x06020000)		// SDIO Cortex-A7 Transaction Limit
#define SDIO_PL301_TLIMIT1_BASE_ADDR	(IO_BASE + 0x06030000)		// SDIO controller Transaction Limit
#define SDIO_AKF_BASE_ADDR		(IO_BASE + 0x06080000)		// SDIO Cortex-A7 and wrapper
// SDIO KF Debug registers
#define SDIO_CFG_BASE_ADDR		(IO_BASE + 0x060C0000)		// SDIO Config

// LIO Subsystem
#define LIO_MISC_BASE_ADDR		(IO_BASE + 0x07000000)		// Miscellaneous registers
#define LIO_AUD_MUX_BASE_ADDR		(IO_BASE + 0x07004000)		// Audio I2S Switch registers
#define LIO_PWM_BASE_ADDR		(IO_BASE + 0x07010000)		// PWM Device registers
#define LIO_SMB_0_BASE_ADDR		(IO_BASE + 0x07020000)		// I2C Device registers
#define LIO_SPI_BASE_ADDR		(IO_BASE + 0x07200000)		// SPI Device registers
#define LIO_UART_BASE_ADDR		(IO_BASE + 0x07230000)		// UART Device registers
#define LIO_MCA_BASE_ADDR		(IO_BASE + 0x07260000)		// MCA Device registers
#define LIO_AES_BASE_ADDR		(IO_BASE + 0x07280000)		// AES
#define LIO_DMAC_BASE_ADDR		(IO_BASE + 0x07300000)		// PL080 DMAC registers

// PMS Subsystem
#define GPIO_BASE_ADDR			(IO_BASE + 0x07500000)		// GPIO Device Registers
#define PMGR_BASE_ADDR			(IO_BASE + 0x07600000)		// Power Manager
#define AIC_BASE_ADDR			(IO_BASE + 0x07700000)		// Apple Interrupt Controller
#define GLBTIMER_BASE_ADDR		(IO_BASE + 0x07710000)		// Global Timer

#define	rAIC_TMR_CNT				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x2014))
#define	rAIC_TMR_ISR				(*(volatile u_int32_t *)(AIC_BASE_ADDR + 0x2018))
#define		AIC_TMRISR_PCT		(1 << 0)
#define		AIC_TMRISR_ETS		(1 << 1)
#define		AIC_TMRISR_STS		(1 << 2)

// AOP Subsystem
// SPU KF Debug registers
#define AOP_GPIO_BASE_ADDR		(IO_BASE + 0x08088000)		// AOP GPIO
#define AOP_MINIPMGR_BASE_ADDR	(IO_BASE + 0x08100000)
// XXX Need to be fixed
#define AOP_SRAM			(IO_BASE + 0x88000000)
#define MEMORY_CALIB_SAVE_BASE_ADDR	AOP_SRAM // Temporarily save cold boot memcal results in AOP_SRAM

 // Dockchannels
#define DOCKCHANNELS_BASE_ADDR		(IO_BASE + 0x08380000)		// Dock channel registers

// AOP PL080 DMA
#define AOP_PL080_BASE_ADDR		(IO_BASE + 0x08650000)

// AUE Subsystem
#define AUE_USBCONT_BASE_ADDR		(IO_BASE + 0x09000000)		// USB Control
#define AUE_USBOTG_BASE_ADDR		(IO_BASE + 0x09100000)		// USBOTG

/* iBoot Specific Defs */

#define AKF_VERSION			(2)

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
#define IICS_COUNT			(3)

#define AES_AP_BASE_ADDR		(LIO_AES_BASE_ADDR)

#define DSIM_BASE_ADDR			(DISP_MIPI_DSI_BASE_ADDR)
#define DSIM_LANE_COUNT			(1)
#define DSIM_VERSION			(3)

#define ADBE_DISPLAYPIPE_BASE_ADDR	(DISP0_BASE_ADDR)
#define DISP_VERSION			(5)
#define ADP_VERSION			(1)

#define PMGR_WDG_VERSION		(1)

#define GPIOC_COUNT			(2)
#define GPIO_VERSION			(5)
#define GPIO_0_GROUP_COUNT		(18)
#define GPIO_GROUP_COUNT		(GPIO_0_GROUP_COUNT)
#define GPIO_1_BASE_ADDR		(AOP_GPIO_BASE_ADDR)
#define GPIO_1_GROUP_COUNT		(12)
#define GPIO_AP				(GPIOC_0)
#define GPIO_AOP			(GPIOC_1)

#define AIC_VERSION			2
#define AIC_INT_COUNT			(256)

#define USBOTG_BASE_ADDR		(AUE_USBOTG_BASE_ADDR)
#define AUSB_CTL_REG_BASE_ADDR		(AUE_USBCONT_BASE_ADDR)
#define AUSB_CTL_USB20PHY_REG_OFFSET	(0x30)
#define AUSB_USB20PHY_ONLY_VERSION	(1)

#define ASEP_AKF_BASE_ADDR		(IO_BASE + 0x0AA00000)		// ASEP AKF control Registers

#endif  /* ! __PLATFORM_SOC_HWREGBASE_H */
