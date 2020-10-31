/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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
#define INT_WDT				4	// Watchdog Timer Interrupt
#define INT_DWI				5	// DWI Interrupt
#define INT_PMGR_PERF_CNTRS		6	// PMGR Performace Counters Interrupt
#define INT_THERM_TEMP0			7	// Thermal Temp 0 Interrupt
#define INT_THERM_ALARM0		8	// Thermal Alarm 0 Interrupt
#define INT_THERM_TEMP1			9	// Thermal Temp 1 Interrupt
#define INT_THERM_ALARM1		10	// Thermal Alarm 1 Interrupt

// Reserved 11 - 19

#define INT_THERM_FS_ACT0		20	// Thermal Failsafe Active 0 Interrupt
#define INT_THERM_FS_ACT1		21	// Thermal Failsafe Active 1 Interrupt
#define INT_THERM_FS_EXT0		22	// Thermal Failsafe Exit 0 Interrupt
#define INT_THERM_FS_EXT1		23	// Thermal Failsafe Exit 1 Interrupt
#define INT_SOC_THERM_FS_ACT0		24	// SoC Thermal Failsafe Active 0 Interrupt
#define INT_SOC_THERM_FS_ACT1		25	// SoC Thermal Failsafe Active 1 Interrupt

// Reserved 26 - 31

#define INT_ISP_KF_INBOX_EMPTY		32	// ISP KF Inbox Empty Interrupt
#define INT_ISP_KF_INBOX_NOTEMPTY	33	// ISP KF Inbox Not Empty Interrupt
#define INT_ISP_KF_OUTBOX_EMPTY		34	// ISP KF Outbox Empty Interrupt
#define INT_ISP_KF_OUTBOX_NOTEMPTY	35	// ISP KF Outbox Not Empty Interrupt
#define INT_ISP0			36	// ISP 0 Interrupt
#define INT_ISP1       			37	// ISP 1 Interrupt
#define INT_ISP2			38	// ISP 2 Interrupt
#define INT_ISP3       			39	// ISP 3 Interrupt
#define INT_ISP_DART			40	// ISP DART Interrupt

// Reserved 41 - 43

#define INT_SEP_KF_INBOX_EMPTY		44	// SEP KF Inbox Empty Interrupt
#define INT_SEP_KF_INBOX_NOTEMPTY	45	// SEP KF Inbox Not Empty Interrupt
#define INT_SEP_KF_OUTBOX_EMPTY		46	// SEP KF Outbox Empty Interrupt
#define INT_SEP_KF_OUTBOX_NOTEMPTY	47	// SEP KF Outbox Not Empty Interrupt

// Reserved 48 - 51

#define INT_ANS_KF_INBOX_EMPTY		52	// ANS KF Inbox Empty Interrupt
#define INT_ANS_KF_INBOX_NOTEMPTY	53	// ANS KF Inbox Not Empty Interrupt
#define INT_ANS_KF_OUTBOX_EMPTY		54	// ANS KF Outbox Empty Interrupt
#define INT_ANS_KF_OUTBOX_NOTEMPTY	55	// ANS KF Outbox Not Empty Interrupt
#define INT_ANS_CHNL0			56	// ANS Channel 0 Interrupt
#define INT_ANS_CHNL1			57	// ANS Channel 1 Interrupt

// Reserved 58 - 63

#define INT_DISP0			64	// Display 0 Interrupt
#define INT_DISP0_PERF_CNTRS		65	// Display 0 Performace Counters Interrupt 
#define INT_DISP0_BACK_END_VBI		66	// Display 0 Back End VBI Interrupt
#define INT_DISP0_BACK_END_MISC		67	// Display 0 Back End Misc Interrupt
#define INT_MIPIDSI			68	// MIPI DSI Interrupt
#define INT_EDP0			69	// Display Port
#define INT_DPORT0			INT_EDP0

// Reserved 70 - 73

#define INT_DISP1			74	// Display 1 Interrupt
#define INT_DISP1_PERF_CNTRS		75	// Display 1 Performace Counters Interrupt 
#define INT_DISP1_BACK_END		76	// Display 1 Back End Interrupt

// Reserved 77 - 79

#define INT_VDEC			80	// Video Decoder Interrupt

// Reserved 81

#define INT_VENC			82	// Video Encoder Interrupt

// Reserved 83

#define INT_GFX				84	// GFX Interrupt
#define INT_GFX_SLEEP			85	// GFX Sleep Interrupt
#define INT_GFX_WAKE			86	// GFX Wake Interrupt
#define INT_GFX_CHG			87	// GFX Change Interrupt

// Reserved 88 - 89

#define INT_ADSP_F0			90	// ADSP F0 Interrupt
#define INT_ADSP_F1			91	// ADSP F1 Interrupt
#define INT_ADSP_T0			92	// ADSP T0 Interrupt
#define INT_ADSP_T1			93	// ADSP T1 Interrupt

// Reserved 94

#define INT_JPEG			95	// JPEG Interrupt
#define INT_JPEG_ERR			96	// JPEG Error Interrupt
#define INT_JPEG_DART			97	// JPEG DART Interrupt
#define INT_JPEG_DART_PERF		98	// JPEG DART Performance Interrupt

// Reserved 99

#define INT_MSR				100	// MSR Interrupt
#define INT_MSR_SMMU			101	// MSR SMMU Interrupt
#define INT_MSR_DART			102	// MSR DART Interrupt
#define INT_MSR_DAR_PERF		103	// MSR DART Performance Interrupt

// Reserved 104-107

#define INT_GPIO_CLASS0			108	// GPIO Class 0 Interrupt
#define INT_GPIO_CLASS1			109	// GPIO Class 1 Interrupt
#define INT_GPIO_CLASS2			110	// GPIO Class 2 Interrupt
#define INT_GPIO_CLASS3			111 	// GPIO Class 3 Interrupt
#define INT_GPIO_CLASS4			112	// GPIO Class 4 Interrupt
#define INT_GPIO_CLASS5			113	// GPIO Class 5 Interrupt
#define INT_GPIO_CLASS6			114	// GPIO Class 6 Interrupt

// Reserved 115-127

#define INT_MEM_CONTROLLER		128	// Memory Controller Interrupt
#define INT_MCU_CHNL0_PHY		129	// MCU Channel 0 PHY
#define INT_MCU_CHNL1_PHY		130	// MCU Channel 0 PHY

// Reserved 131

#define INT_SIO_KF_INBOX_EMPTY		132	// SIO KF Inbox Empty Interrupt
#define INT_SIO_KF_INBOX_NOTEMPTY	133	// SIO KF Inbox Not Empty Interrupt
#define INT_SIO_KF_OUTBOX_EMPTY		134	// SIO KF Outbox Empty Interrupt
#define INT_SIO_KF_OUTBOX_NOTEMPTY	135	// SIO KF Outbox Not Empty Interrupt
#define INT_SPI0			136	// SPI 0 Interrupt
#define INT_SPI1			137	// SPI 1 Interrupt
#define INT_SPI2			138	// SPI 2 Interrupt
#define INT_SPI3			139	// SPI 3 Interrupt
#define INT_UART0			140	// UART 0 Interrupt
#define INT_UART1			141	// UART 1 Interrupt
#define INT_UART2			142	// UART 2 Interrupt
#define INT_UART3			143	// UART 3 Interrupt
#define INT_UART4			144	// UART 4 Interrupt
#define INT_UART5			145	// UART 5 Interrupt
#define INT_UART6			146	// UART 6 Interrupt
#define INT_UART7			147	// UART 7 Interrupt
#define INT_MCA0			148	// Multi-Channel Audio 0 Interrupt
#define INT_MCA1			149	// Multi-Channel Audio 1 Interrupt
#define INT_MCA2			150	// Multi-Channel Audio 2 Interrupt
#define INT_MCA3			151	// Multi-Channel Audio 3 Interrupt
#define INT_MCA4			152	// Multi-Channel Audio 4 Interrupt
#define INT_SEC_UART0			153	// Secure UART 0 Interrupt
#define INT_SEC_UART1			154	// Secure UART 1 Interrupt
#define INT_IIC0			155	// I2C 0 Interrupt
#define INT_IIC1			156	// I2C 1 Interrupt
#define INT_IIC2			157	// I2C 2 Interrupt
#define INT_IIC3			158	// I2C 3 Interrupt
#define INT_PWM				159	// PWM Interrupt
#define INT_AES0			160	// AES 0 Interrupt

// Reserved 161

#define INT_USB_OTG			162	// USB OTG Interrupt
#define INT_USB2HOST0_EHCI		163	// USB EHCI 0 Interrupt
#define INT_USB2HOST0_OHCI		164	// USB OHCI 0 Interrupt
#define INT_USB2HOST1_EHCI		165	// USB EHCI 1 Interrupt

// Reserved 166 - 167

#define INT_USBOTG_RESUME		168	// USB OTG Resume Interrupt
#define INT_USB_20_BATT_CHRG_DET	169	// USB 2.0 Battery Charge Detect Interrupt
#define INT_USB2HOST0_PHY_RESUME	170	// USB Host 0 Resume Interrupt
#define INT_USB2HOST1_PHY_RESUME	171	// USB Host 1 Resume Interrupt

// Reserved 172

#define INT_COHERENCE_PNT_ERR		173	// Coherence Point Error Interrupt
#define INT_COHERENCE_PNT_PERF_CNTRS	174	// Coherence Point Performance Counters Interrupt

// Reserved 175

#define INT_CPU0_COMM_TX		176	// CPU 0 COMM TX Interrupt
#define INT_CPU0_COMM_RX		177	// CPU 0 COMM RX Interrupt
#define INT_CPU0_PMU_IRQ		178	// CPU 0 PMU IRQ Interrupt
#define INT_CPU1_COMM_TX		179	// CPU 1 COMM TX Interrupt
#define INT_CPU1_COMM_RX		180	// CPU 1 COMM RX Interrupt
#define INT_CPU1_PMU_IRQ		181	// CPU 1 PMU IRQ Interrupt

// Reserved 182 - 184

#define INT_CTM_TRIGGER0		185	// CTM Trigger 0 Interrupt
#define INT_CTM_TRIGGER1		186	// CTM Trigger 1 Interrupt
#define INT_CTM_TRIGGER2		187	// CTM Trigger 2 Interrupt
#define INT_CTM_TRIGGER3		188	// CTM Trigger 3 Interrupt

// Reserved 189 - 191

#endif /* ! __PLATFORM_SOC_HWISR_H */
