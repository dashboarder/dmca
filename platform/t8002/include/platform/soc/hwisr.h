/*
 * Copyright (C) 2013 - 2014 Apple Inc. All rights reserved.
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

#define INT_PMGR_PERF_CNTRS		5	// PMGR Performace Counters Interrupt
#define INT_THERM_TEMP0			6	// Thermal Temp 0 Interrupt
#define INT_THERM_ALARM0		7	// Thermal Alarm 0 Interrupt
#define INT_UVDC			8	// UVDC Interrupt
#define INT_UVDC_TH			9	// UVDC TH interrupt
#define INT_DROOP			10	// Droop Interrupt
#define INT_DROOP_TH			11	// Droop TH interrupt

// Reserved 12 - 13

#define INT_MIPIDPHY			14 	// MIPID PHY Interrupt
#define INT_CTM_TRIGGER0 		15 	// CTM Trigger 0 Interrupt
#define INT_CTM_TRIGGER1		16 	// CTM Trigger 1 Interrupt
#define INT_CTM_TRIGGER2		17	// CTM Trigger 2 Interrupt
#define INT_CTM_TRIGGER3		18	// CTM Trigger 3 Interrupt

// Reserved 19

#define INT_MINI_PMGR_PERF_CNTRS	20	// MINIPMGR PerfCounter Interrupt
#define INT_MINI_PMGR_WDT		21	// MINIPMGR Watchdog Interrupt

// Reserved 22

#define INT_CPU0_AXI_ERR_IRQ		23	// CPU 0 AXI Error IRQ Interrupt
#define INT_CPU0_CX_ETB_FUKK		24	// CPU 0 CXETB Full Interrupt 

// Reserved 25

#define INT_CPU0_PMU_IRQ		26	// CPU 0 PMU IRQ Interrupt
#define INT_CPU0_CTI_IRQ		27	// CPU 0 CTI Interrupt
#define INT_CPU0_COMM_RX		28	// CPU 0 COMM RX Interrupt
#define INT_CPU0_COMM_TX		29	// CPU 0 COMM TX Interrupt

// Reserved 30

#define INT_CPU1_PMU_IRQ		31	// CPU 1 PMU IRQ Interrupt
#define INT_CPU1_CTI_IRQ		32	// CPU 1 CTI Interrupt
#define INT_CPU1_COMM_RX		33	// CPU 1 COMM RX Interrupt
#define INT_CPU1_COMM_TX		34	// CPU 1 COMM TX Interrupt

// Reserved 35

#define INT_GPIO_CLASS0			36	// GPIO Class 0 Interrupt
#define INT_GPIO_CLASS1			37	// GPIO Class 1 Interrupt
#define INT_GPIO_CLASS2			38	// GPIO Class 2 Interrupt
#define INT_GPIO_CLASS3			39 	// GPIO Class 3 Interrupt
#define INT_GPIO_CLASS4			40	// GPIO Class 4 Interrupt
#define INT_GPIO_CLASS5			41	// GPIO Class 5 Interrupt
#define INT_GPIO_CLASS6			42	// GPIO Class 6 Interrupt

// Reserved 43

#define INT_MEM_MCU			44	// Memory Controller MCU Interrupt
#define INT_DCS_AMCX0			45	// Memory Controller DCS AMCX0 Interrupt
#define INT_DCS_AMCX1			46	// Memory Controller DCS AMCX1 Interrupt
#define INT_DCS_AMP_CAL_ERR		47	// Memory Controller DCS AMP Cal Error Interrupt

// Reserved 48

#define INT_GFX				49	// GFX Interrupt

// Reserved 50

#define INT_ENET			51	// Ethernet IP Interrupt
#define INT_USB_OTG			52	// USB OTG Interrupt
#define INT_USB_20_BATT_CHRG_DET	53	// USB 2.0 Battery Charge Detect Interrupt
#define INT_USBOTG_RESUME		54	// USB OTG Resume Interrupt
#define INT_USB_HOST			55	// USB Host Interrupt
#define INT_SDIO_KF_INBOX_EMPTY		56	// SDIO KF Inbox Empty Interrupt
#define INT_SDIO_KF_INBOX_NOTEMPTY	57	// SDIO KF Inbox Not Empty Interrupt
#define INT_SDIO_KF_OUTBOX_EMPTY	58	// SDIO KF Outbox Empty Interrupt
#define INT_SDIO_KF_OUTBOX_NOTEMPTY	59	// SDIO KF Outbox Not Empty Interrupt
#define INT_SDIO_SDHC_WAKEUP		60	// SDIO SDHC Wakeup
#define INT_SDIO_SDHC_INT		61	// SDIO SDHC Tnterrupt
#define INT_SDIO_CARD_INT		62	// SDIO SDHC Interrupt
#define INT_SDIO_IRQ			63	// SDIO IRQ
#define INT_SDIO_WL_HOST_WAKE_INT	64	// SDIO Host Wake Interrupt

// Reserved 65

#define INT_ANS_KF_INBOX_EMPTY 		66	// ANS KF Inbox Empty Interrupt
#define INT_ANS_KF_INBOX_NOTEMPTY 	67	// ANS KF Inbox Not Empty Interrupt
#define INT_ANS_KF_OUTBOX_EMPTY		68	// ANS KF Outbox Empty Interrupt
#define INT_ANS_KF_OUTBOX_NOTEMPTY	69	// ANS KF Outbox Not Empty Interrupt
#define INT_ANS_CHNL0			70	// ANS Channel 0 Interrupt

// Reserved 71

#define INT_PIO				72	// PIO IRQ

// Reserved 73

#define INT_DMA_RX_TC			74	// DMA Rx Transfer Complete Interrupt
#define INT_DMA_RX_ERR			75	// DMA Rx Error Interrupt
#define INT_DMA_TX_TC			76	// DMA Tx Transfer Complete Interrupt
#define INT_DMA_TX_ERR			77	// DMA Tx Error Interrupt

// Reserved 78

#define INT_SPI0			79	// SPI 0 Interrupt
#define INT_SPI1			80	// SPI 1 Interrupt
#define INT_UART0			81	// UART 0 Interrupt
#define INT_UART1			82	// UART 1 Interrupt
#define INT_UART2			83	// UART 2 Interrupt
#define INT_UART3			84	// UART 3 Interrupt
#define INT_UART4			85	// UART 4 Interrupt
#define INT_UART5			86	// UART 5 Interrupt
#define INT_UART6			87	// UART 6 Interrupt
#define INT_UART7			88	// UART 7 Interrupt
#define INT_PWM0			89	// PWM0 Interrupt
#define INT_MCA0			90	// Multi-Channel Audio 0 Interrupt
#define INT_MCA1			91	// Multi-Channel Audio 1 Interrupt
#define INT_MCA2			92	// Multi-Channel Audio 2 Interrupt
#define INT_IIC0			93	// I2C 0 Interrupt
#define INT_IIC1			94	// I2C 1 Interrupt
#define INT_IIC2			95	// I2C 2 Interrupt
#define INT_AES				96	// AES Interrupt

// Reserved 97

#define INT_MSR				98	// MSR Interrupt
#define INT_MSR_SMMU			99	// MSR SMMU Interrupt
#define INT_MSR_DART			100	// MSR DART Interrupt
#define INT_MSR_SMMU_PERF		101	// MSR SMMU Performance Interrupt
#define INT_MSR_DART_PERF		102	// MSR DART Performance Interrupt

// Reserved 103

#define INT_ADP				104	// Display All ADP
#define INT_DISP_DART			105	// Display DART Interrupt
#define INT_DISP_SMMU			106	// Display SMMU Interrupt
#define INT_DISP_BACK_END_VBI		107	// Display Back End VBI Interrupt
#define INT_DISP_BACK_END_VFTG_IDLE	108	// Display Back End VFTG Idle Interrupt
#define INT_DISP_BACK_END_VBL_CNTR	109	// Display Back End VBL Counter Interrupt
#define INT_PIO_DMA			110	// PIO DMA Interrupt

// Reserved 111

#define INT_VDEC			112	// Video Decoder

// Reserved 113

#define INT_JPEG			114	// MSR Interrupt
#define INT_JPEG_DART			115	// MSR SMMU Interrupt
#define INT_JPEG_DART_PERF		116	// MSR DART Interrupt

// Reserved 117

#define INT_AVE_KF_INBOX_EMPTY 		118	// AVE KF Inbox Empty Interrupt
#define INT_AVE_KF_INBOX_NOTEMPTY 	119	// AVE KF Inbox Not Empty Interrupt
#define INT_AVE_KF_OUTBOX_EMPTY		120	// AVE KF Outbox Empty Interrupt
#define INT_AVE_KF_OUTBOX_NOTEMPTY	121	// AVE KF Outbox Not Empty Interrupt
#define INT_AVE_SMMU			122	// AVE SMMU Interrupt
#define INT_AVE_DART			123	// AVE DART Interrupt
#define INT_AVE 			124	// AVE Interrupt
#define INT_SVE 			125	// SVE Driver Synchronization Interrupt

// AVE Reserved 126-127 

// Reserved 128

#define INT_ISP_KF_INBOX_EMPTY 		129	// ISP KF Inbox Empty Interrupt
#define INT_ISP_KF_INBOX_NOTEMPTY 	130	// ISP KF Inbox Not Empty Interrupt
#define INT_ISP_KF_OUTBOX_EMPTY		131	// ISP KF Outbox Empty Interrupt
#define INT_ISP_KF_OUTBOX_NOTEMPTY	132	// ISP KF Outbox Not Empty Interrupt
#define INT_ISP0			133	// ISP 0 Interrupt
#define INT_ISP1			134	// ISP 1 Interrupt
#define INT_ISP2			135	// ISP 2 Interrupt
#define INT_ISP3			136	// ISP 3 Interrupt
#define INT_ISP_DART 			137	// ISP DART Interrupt

// Reserved 138

#define INT_SEP_KF_INBOX_EMPTY		139	// SEP KF Inbox Empty Interrupt
#define INT_SEP_KF_INBOX_NOTEMPTY	140	// SEP KF Inbox Not Empty Interrupt
#define INT_SEP_KF_OUTBOX_EMPTY		141	// SEP KF Outbox Empty Interrupt
#define INT_SEP_KF_OUTBOX_NOTEMPTY	142	// SEP KF Outbox Not Empty Interrupt
#define INT_SEP_CRIT_PATH_MONITOR	143	// SEP Critical Path Monitor Interrupt
#define INT_SEP_AUTH_ERROR		144	// SEP Authentication Error Interrupt

// Reserved 145 - 147

#define INT_DOCKCHANNELS_AP		148	// DOCKCHANNELS Interrupt:q

// AOP Interrupts 146 - 211

// Reserved 221

#define INT_PDMC			222	// PDMC Interrupt

// Reserved 223 - 224

// Marconi Interrupts 225 - 248

// Reserved 249 - 255

#endif /* ! __PLATFORM_SOC_HWISR_H */
