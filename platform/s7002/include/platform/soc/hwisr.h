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
#ifndef __PLATFORM_SOC_HWISR_H
#define __PLATFORM_SOC_HWISR_H

#define INT_SW0				0	// Software 0 Interrupt
#define INT_SW1				1	// Software 1 Interrupt
#define INT_SW2				2	// Software 2 Interrupt
#define INT_SW3				3	// Software 3 Interrupt

// Reserved 4 

#define INT_WDT				5	// Watchdog Timer Interrupt

// Reserved 6

#define INT_ANS_KF_INBOX_EMPTY		7	// ANS KF Inbox Empty Interrupt
#define INT_ANS_KF_INBOX_NOTEMPTY	8	// ANS KF Inbox Not Empty Interrupt
#define INT_ANS_KF_OUTBOX_EMPTY		9	// ANS KF Outbox Empty Interrupt
#define INT_ANS_KF_OUTBOX_NOTEMPTY	10	// ANS KF Outbox Not Empty Interrupt
#define INT_ANS_CHNL0			11	// ANS Channel 0 Interrupt

// Reserved 12

#define INT_ENET			13

#define INT_USB_OTG			14	// USB OTG Interrupt
#define INT_USB_20_BATT_CHRG_DET	15	// USB 2.0 Battery Charge Detect Interrupt
#define INT_USBOTG_RESUME		16	// USB OTG Resume Interrupt

// Reserved 17

#define INT_GPIO_CLASS0			18	// GPIO Class 0 Interrupt
#define INT_GPIO_CLASS1			19	// GPIO Class 1 Interrupt
#define INT_GPIO_CLASS2			20	// GPIO Class 2 Interrupt
#define INT_GPIO_CLASS3			21 	// GPIO Class 3 Interrupt
#define INT_GPIO_CLASS4			22	// GPIO Class 4 Interrupt
#define INT_GPIO_CLASS5			23	// GPIO Class 5 Interrupt
#define INT_GPIO_CLASS6			24	// GPIO Class 6 Interrupt

// Reserved 25

#define INT_MEM_CONTROLLER		26	// Memory Controller Interrupt

// Reserved 27

#define INT_CPU0_PMU_IRQ		28	// CPU 0 PMU IRQ Interrupt
#define INT_CPU0_CTI_IRQ		29	// CPU 0 CTI Interrupt
#define INT_CPU0_COMM_TX		30	// CPU 0 COMM TX Interrupt
#define INT_CPU0_COMM_RX		31	// CPU 0 COMM RX Interrupt
#define INT_CPU0_AXI_ERR_IRQ		32	// CPU 0 AXI Error IRQ Interrupt
#define INT_CPU0_CX_ETB_FUKK		33	// CPU 0 CXETB Full Interrupt

#define INT_DISP0			35	// Display Interrupt

// Reserved 46

#define INT_DISP_DART			37	// Display DART Interrupt
#define INT_DISP_SMMU			38	// Display SMMU Interrupt
#define INT_DISP_BACK_END_VBI		39	// Display Back End VBI Interrupt
#define INT_DISP_BACK_END_VFTG_IDLE	40	// Display Back End VFTG Idle Interrupt
#define INT_DISP_BACK_END_VBL_CNTR	41	// Display Back End VBL Counter Interrupt
#define INT_MIPIDSI			42	// MIPI DSI Interrupt

// Reserved 43

#define INT_GFX				44	// GFX Interrupt

// Reserved 45-46

#define INT_MSR				47	// MSR Interrupt
#define INT_MSR_SMMU			48	// MSR SMMU Interrupt
#define INT_MSR_DART			49	// MSR DART Interrupt
#define INT_MSR_SMMU_PERF		50	// MSR SMMU Performance Interrupt
#define INT_MSR_DART_PERF		51	// MSR DART Performance Interrupt

// Reserved 52

#define INT_PIO				53	// PIO IRQ

// Reserved 54

#define INT_DMA_RX_TC			55	// DMA Rx Transfer Complete Interrupt
#define INT_DMA_RX_ERR			56	// DMA Rx Error Interrupt
#define INT_DMA_TX_TC			57	// DMA Tx Transfer Complete Interrupt
#define INT_DMA_TX_ERR			58	// DMA Tx Error Interrupt

// Reserved 59

#define INT_SPI0			60	// SPI 0 Interrupt
#define INT_SPI1			61	// SPI 1 Interrupt

// Reserved 62

#define INT_UART0			63	// UART 0 Interrupt
#define INT_UART1			64	// UART 1 Interrupt
#define INT_UART2			65	// UART 2 Interrupt
#define INT_UART3			66	// UART 3 Interrupt
#define INT_UART4			67	// UART 4 Interrupt

// Reserved 68

#define INT_PWM0			69	// PWM0 Interrupt

// Reserved 70

#define INT_MCA0			71	// Multi-Channel Audio 0 Interrupt
#define INT_MCA1			72	// Multi-Channel Audio 1 Interrupt

// Reserved 73

#define INT_IIC0			74	// I2C 0 Interrupt
#define INT_IIC1			75	// I2C 1 Interrupt

// Reserved 76

#define INT_AES				77	// AES Interrupt

// Reserved 78

#define INT_VDEC			79	// Video Decoder

// Reserved 80

#define INT_SDIO_KF_INBOX_EMPTY		81	// ANS KF Inbox Empty Interrupt
#define INT_SDIO_KF_INBOX_NOTEMPTY	82	// ANS KF Inbox Not Empty Interrupt
#define INT_SDIO_KF_OUTBOX_EMPTY	83	// ANS KF Outbox Empty Interrupt
#define INT_SDIO_KF_OUTBOX_NOTEMPTY	84	// ANS KF Outbox Not Empty Interrupt
#define INT_SDIO_SDHC_WAKEUP		85	// SDIO SDHC Wakeup
#define INT_SDIO_SDHC_INT		86	// SDIO SDHC Tnterrupt
#define INT_SDIO_CARD_INT		87	// SDIO SDHC Interrupt
#define INT_SDIO_IRQ			88	// SDIO IRQ
#define INT_SDIO_WL_HOST_WAKE_INT	89	// SDIO Host Wake Interrupt

// Reserved 90

// 91 - 117 SPU interrupts

#define DBGFIFO_0_NOT_EMPTY		118	// DebugFifo 0 Not Empty Interrupt
#define DBGFIFO_1_NOT_EMPTY		119	// DebugFifo 1 Not Empty Interrupt
#define DBGFIFO_2_NOT_EMPTY		120	// DebugFifo 2 Not Empty Interrupt
#define DBGFIFO_3_NOT_EMPTY		121	// DebugFifo 3 Not Empty Interrupt
#define DBGFIFO_4_NOT_EMPTY		122	// DebugFifo 4 Not Empty Interrupt
#define DBGFIFO_5_NOT_EMPTY		123	// DebugFifo 5 Not Empty Interrupt
#define DBGFIFO_6_NOT_EMPTY		124	// DebugFifo 6 Not Empty Interrupt
#define DBGFIFO_7_NOT_EMPTY		125	// DebugFifo 7 Not Empty Interrupt

// Reserved 126 - 127 


#endif /* ! __PLATFORM_SOC_HWISR_H */
