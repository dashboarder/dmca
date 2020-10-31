/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
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

#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003

#define INT_SW0				0	// Software 0 Interrupt
#define INT_SW1				1	// Software 1 Interrupt
#define INT_SW2				2	// Software 2 Interrupt
#define INT_SW3				3	// Software 3 Interrupt
#define INT_WDT				4	// Watchdog Timer Interrupt
#define INT_DWI1			5	// DWI Interrupt
#define INT_DWI2			6	// DWI Interrupt
#define INT_PMGR_PERF_CNTRS		7	// PMGR Performace Counters Interrupt
#define INT_THERM_TEMP0			8	// Thermal Temp 0 Interrupt
#define INT_THERM_ALARM0		9	// Thermal Alarm 0 Interrupt
#define INT_THERM_TEMP1			10	// Thermal Temp 1 Interrupt
#define INT_THERM_ALARM1		11	// Thermal Alarm 1 Interrupt
#define INT_THERM_TEMP2			12	// Thermal Temp 2 Interrupt
#define INT_THERM_ALARM2		13	// Thermal Alarm 2 Interrupt

// Reserved 14 - 15

#define INT_CPM_THERM_TEMP		16	// CPM Thermal temp Interrupt
#define INT_CPM_THERM_ALARM		17	// CPM Thermal alarm Interrupt
#define INT_CPU0_THERM_TEMP		18	// CPU0 Thermal temp Interrupt
#define INT_CPU0_THERM_ALARM		19	// CPU0 Thermal alarm Interrupt
#define INT_CPU1_THERM_TEMP		20	// CPU1 Thermal temp Interrupt
#define INT_CPU1_THERM_ALARM		21	// CPU1 Thermal alarm Interrupt

// Reserved 21 - 26

#define INT_THERM_FS_ACT0		27	// Thermal Failsafe Active 0 Interrupt
#define INT_THERM_FS_ACT1		28	// Thermal Failsafe Active 1 Interrupt
#define INT_THERM_FS_EXT0		29	// Thermal Failsafe Exit 0 Interrupt
#define INT_THERM_FS_EXT1		30	// Thermal Failsafe Exit 1 Interrupt
#define INT_SOC_THERM_FS_ACT0		31	// SoC Thermal Failsafe Active 0 Interrupt
#define INT_SOC_THERM_FS_ACT1		32	// SoC Thermal Failsafe Active 1 Interrupt

// Reserved 33 - 34

#define INT_CTM_TRIGGER0		35	// CTM Trigger 0 Interrupt
#define INT_CTM_TRIGGER1		36	// CTM Trigger 1 Interrupt
#define INT_CTM_TRIGGER2		37	// CTM Trigger 2 Interrupt
#define INT_CTM_TRIGGER3		38	// CTM Trigger 3 Interrupt
#define INT_COHERENCE_PNT_ERR		39	// Coherence Point Error Interrupt
#define INT_COHERENCE_PNT_PERF_CNTRS	40	// Coherence Point Performance Counters Interrupt

// Reserved 41

#define INT_GPIO_CLASS0			42	// GPIO Class 0 Interrupt
#define INT_GPIO_CLASS1			43	// GPIO Class 1 Interrupt
#define INT_GPIO_CLASS2			44	// GPIO Class 2 Interrupt
#define INT_GPIO_CLASS3			45	// GPIO Class 3 Interrupt
#define INT_GPIO_CLASS4			46	// GPIO Class 4 Interrupt
#define INT_GPIO_CLASS5			47	// GPIO Class 5 Interrupt
#define INT_GPIO_CLASS6			48	// GPIO Class 6 Interrupt

// Reserved 49-58

#define INT_MEM_CONTROLLER0		59	// Memory Controller MCC Interrupt
#define INT_MEM_CONTROLLER1		60	// Memory Controller MCC Interrupt
// XXX: TBD DSC interrupts

// Reserved 73

#define INT_CPU0_COMM_TX		74	// CPU 0 COMM TX Interrupt
#define INT_CPU0_COMM_RX		75	// CPU 0 COMM RX Interrupt
#define INT_CPU0_PMU_IRQ		76	// CPU 0 PMU IRQ Interrupt
#define INT_CPU1_COMM_TX		77	// CPU 1 COMM TX Interrupt
#define INT_CPU1_COMM_RX		78	// CPU 1 COMM RX Interrupt
#define INT_CPU1_PMU_IRQ		79	// CPU 1 PMU IRQ Interrupt

// Reserved 80 - 85

#define INT_CCC_UNIFIED_TRIGGER		86	// CCC Unified Trigger Interrupt

// Reserved 87

#define INT_ISP_KF_INBOX_EMPTY		88	// ISP KF Inbox Empty Interrupt
#define INT_ISP_KF_INBOX_NOTEMPTY	89	// ISP KF Inbox Not Empty Interrupt
#define INT_ISP_KF_OUTBOX_EMPTY		90	// ISP KF Outbox Empty Interrupt
#define INT_ISP_KF_OUTBOX_NOTEMPTY	91	// ISP KF Outbox Not Empty Interrupt
#define INT_ISP0			92	// ISP 0 Interrupt
#define INT_ISP1			93	// ISP 1 Interrupt
#define INT_ISP2			94	// ISP 2 Interrupt
#define INT_ISP3			95	// ISP 3 Interrupt
#define INT_ISP_DART			96	// ISP DART Interrupt

// Reserved 97

#define INT_SEP_KF_INBOX_EMPTY		98	// SEP KF Inbox Empty Interrupt
#define INT_SEP_KF_INBOX_NOTEMPTY	99	// SEP KF Inbox Not Empty Interrupt
#define INT_SEP_KF_OUTBOX_EMPTY		100	// SEP KF Outbox Empty Interrupt
#define INT_SEP_KF_OUTBOX_NOTEMPTY	101	// SEP KF Outbox Not Empty Interrupt
#define INT_SEP_AUTH_ERROR		102	// SEP Authentication Error Interrupt
#define INT_SEP_CRIT_PATH_MONITOR	103	// SEP Critical Path Monitor Interrupt

// Reserved 104 - 105

// XXX JF: AOP interrupts

// Reserved 138 - 139

#define INT_DISP0			140	// Display 0 Interrupt
#define INT_DISP0_DART			141	// Display 0 DART Interrupt
#define INT_DISP0_SMMU			142	// Display 0 SMMU Interrupt
#define INT_DISP0_BACK_END_BLU		143	// Display 0 Back End Back Light Update Interrupt
#define INT_DISP0_BACK_END_VBI		144	// Display 0 Back End VBI Interrupt
#define INT_DISP0_BACK_END_VFTG		145	// Display 0 Back End VFTG Idle Interrupt
#define INT_DISP0_BACK_END_VBL		146	// Display 0 Back End VBL counter Interrupt
#define INT_MIPIDSI0			147	// MIPI DSI 0 Interrupt
#define INT_MIPIDSI1			148	// MIPI DSI 1 Interrupt
#define INT_DISPLAY_PORT		149	// Display Port
#define INT_EDP0			INT_DISPLAY_PORT
#define	INT_DPORT0			INT_DISPLAY_PORT

// Reserved 151

// XXX: PMP

// Reserved 156

#define INT_VDEC0			157	// Video Decoder 0 Interrupt

// Reserved 158

#define INT_AVE_KF_INBOX_EMPTY		159	// AVE KF Inbox Empty Interrupt
#define INT_AVE_KF_INBOX_NOTEMPTY	160	// AVE KF Inbox Not Empty Interrupt
#define INT_AVE_KF_OUTBOX_EMPTY		161	// AVE KF Outbox Empty Interrupt
#define INT_AVE_KF_OUTBOX_NOTEMPTY	162	// AVE KF Outbox Not Empty Interrupt
#define INT_AVE_DART			163	// AVE DART Interrupt
#define INT_AVE_SMMU			164	// AVE SMMU Interrupt
#define INT_AVE				165	// AVE Interrupt
#define INT_SVE_SYNC			166	// SVE Driver Synchronization Interrupt

// Reserved 167 - 169

#define INT_GFX				170	// GFX Interrupt
#define INT_GFX_SLEEP			171	// GFX Sleep Interrupt
#define INT_GFX_WAKE			172	// GFX Wake Interrupt
#define INT_GFX_CHG			173	// GFX Change Interrupt

// Reserved 174

#define INT_JPEG			175	// JPEG Interrupt
#define INT_JPEG_DART			176	// JPEG DART Interrupt
#define INT_JPEG_DART_PERF		177	// JPEG DART Performance Interrupt

// Reserved 178

#define INT_MSR				179	// MSR Interrupt
#define INT_MSR_SMMU			180	// MSR SMMU Interrupt
#define INT_MSR_DART			181	// MSR DART Interrupt
#define INT_MSR_DAR_PERF		182	// MSR DART Performance Interrupt

// Reserved 183

#define INT_SIO_KF_INBOX_EMPTY		184	// SIO KF Inbox Empty Interrupt
#define INT_SIO_KF_INBOX_NOTEMPTY	185	// SIO KF Inbox Not Empty Interrupt
#define INT_SIO_KF_OUTBOX_EMPTY		186	// SIO KF Outbox Empty Interrupt
#define INT_SIO_KF_OUTBOX_NOTEMPTY	187	// SIO KF Outbox Not Empty Interrupt
#define INT_SPI0			188	// SPI 0 Interrupt
#define INT_SPI1			189	// SPI 1 Interrupt
#define INT_SPI2			190	// SPI 2 Interrupt
#define INT_SPI3			191	// SPI 3 Interrupt
#define INT_UART0			192	// UART 0 Interrupt
#define INT_UART1			193	// UART 1 Interrupt
#define INT_UART2			194	// UART 2 Interrupt
#define INT_UART3			195	// UART 3 Interrupt
#define INT_UART4			196	// UART 4 Interrupt
#define INT_UART5			197	// UART 5 Interrupt
#define INT_UART6			198	// UART 6 Interrupt
#define INT_UART7			199	// UART 7 Interrupt
#define INT_UART8			200	// UART 8 Interrupt
#define INT_MCA0			201	// Multi-Channel Audio 0 Interrupt
#define INT_MCA1			202	// Multi-Channel Audio 1 Interrupt
#define INT_MCA2			203	// Multi-Channel Audio 2 Interrupt
#define INT_MCA3			204	// Multi-Channel Audio 3 Interrupt
#define INT_MCA4			205	// Multi-Channel Audio 4 Interrupt
#define INT_IIC0			206	// I2C 0 Interrupt
#define INT_IIC1			207	// I2C 1 Interrupt
#define INT_IIC2			208	// I2C 2 Interrupt
#define INT_IIC3			209	// I2C 3 Interrupt
#define INT_PWM				210	// PWM Interrupt
#define INT_AES0			211	// AES 0 Interrupt

// Reserved 212 - 213

#define INT_USB_OTG			214	// USB OTG Interrupt
#define INT_USB2HOST0_EHCI		215	// USB EHCI 0 Interrupt
#define INT_USB2HOST0_OHCI		216	// USB OHCI 0 Interrupt
#define INT_USB2HOST1_EHCI		217	// USB EHCI 1 Interrupt
#define INT_USB2HOST2_EHCI		218	// USB EHCI 2 Interrupt
#define INT_USBOTG_RESUME		219	// USB OTG Resume Interrupt
#define INT_USB_20_BATT_CHRG_DET	220	// USB 2.0 Battery Charge Detect Interrupt
#define INT_USB2HOST0_PHY_RESUME	221	// USB Host 0 Resume Interrupt
#define INT_USB2HOST1_PHY_RESUME	222	// USB Host 1 Resume Interrupt
#define INT_USB2HOST2_PHY_RESUME	223	// USB Host 2 Resume Interrupt
#define INT_OTG_IN_EP0			224	// OTG IN Endpoint 0 Interrupt
#define INT_OTG_IN_EP1			225	// OTG IN Endpoint 1 Interrupt
#define INT_OTG_IN_EP2			226	// OTG IN Endpoint 2 Interrupt
#define INT_OTG_IN_EP3			227	// OTG IN Endpoint 3 Interrupt
#define INT_OTG_IN_EP4			228	// OTG IN Endpoint 4 Interrupt
#define INT_OTG_IN_EP5			229	// OTG IN Endpoint 5 Interrupt
#define INT_OTG_IN_EP6			230	// OTG IN Endpoint 6 Interrupt
#define INT_OTG_IN_EP7			231	// OTG IN Endpoint 7 Interrupt
#define INT_OTG_IN_EP8			232	// OTG IN Endpoint 8 Interrupt
#define INT_OTG_OUT_EP0			233	// OTG OUT Endpoint 0 Interrupt
#define INT_OTG_OUT_EP1			234	// OTG OUT Endpoint 1 Interrupt
#define INT_OTG_OUT_EP2			235	// OTG OUT Endpoint 2 Interrupt
#define INT_OTG_OUT_EP3			236	// OTG OUT Endpoint 3 Interrupt
#define INT_OTG_OUT_EP4			237	// OTG OUT Endpoint 4 Interrupt
#define INT_OTG_OUT_EP5			238	// OTG OUT Endpoint 5 Interrupt
#define INT_OTG_OUT_EP6			239	// OTG OUT Endpoint 6 Interrupt
#define INT_OTG_OUT_EP7			240	// OTG OUT Endpoint 7 Interrupt
#define INT_OTG_OUT_EP8			241	// OTG OUT Endpoint 8 Interrupt

// Reserved 242 - 243

#define PCIE_LINK0			244	// PCIE Link 0 Interrupt
#define PCIE_LINK0_DART			245	// PCIE Link 0 DART Interrupt
#define PCIE_LINK0_MMU			246	// PCIE Link 0 MMU Interrupt
#define PCIE_LINK1			247	// PCIE Link 1 Interrupt
#define PCIE_LINK1_DART			248	// PCIE Link 1 DART Interrupt
#define PCIE_LINK1_MMU			249	// PCIE Link 1 MMU Interrupt
#define PCIE_LINK2			250	// PCIE Link 1 Interrupt
#define PCIE_LINK2_DART			251	// PCIE Link 1 DART Interrupt
#define PCIE_LINK2_MMU			252	// PCIE Link 1 MMU Interrupt
#define PCIE_LINK3			253	// PCIE Link 1 Interrupt
#define PCIE_LINK3_DART			254	// PCIE Link 1 DART Interrupt
#define PCIE_LINK3_MMU			255	// PCIE Link 1 MMU Interrupt

#define PCIE_MSI0			256	// PCIE MSI 0 Interrupt
#define PCIE_MSI1			257	// PCIE MSI 1 Interrupt
#define PCIE_MSI2			258	// PCIE MSI 2 Interrupt
#define PCIE_MSI3			259	// PCIE MSI 3 Interrupt
#define PCIE_MSI4			260	// PCIE MSI 4 Interrupt
#define PCIE_MSI5			261	// PCIE MSI 5 Interrupt
#define PCIE_MSI6			262	// PCIE MSI 6 Interrupt
#define PCIE_MSI7			263	// PCIE MSI 7 Interrupt
#define PCIE_MSI8			264	// PCIE MSI 8 Interrupt
#define PCIE_MSI9			265	// PCIE MSI 9 Interrupt
#define PCIE_MSI10			266	// PCIE MSI 10 Interrupt
#define PCIE_MSI11			267	// PCIE MSI 11 Interrupt
#define PCIE_MSI12			268	// PCIE MSI 12 Interrupt
#define PCIE_MSI13			269	// PCIE MSI 13 Interrupt
#define PCIE_MSI14			270	// PCIE MSI 14 Interrupt
#define PCIE_MSI15			271	// PCIE MSI 15 Interrupt
#define PCIE_MSI16			272	// PCIE MSI 16 Interrupt
#define PCIE_MSI17			273	// PCIE MSI 17 Interrupt
#define PCIE_MSI18			274	// PCIE MSI 18 Interrupt
#define PCIE_MSI19			275	// PCIE MSI 19 Interrupt
#define PCIE_MSI20			276	// PCIE MSI 20 Interrupt
#define PCIE_MSI21			277	// PCIE MSI 21 Interrupt
#define PCIE_MSI22			278	// PCIE MSI 22 Interrupt
#define PCIE_MSI23			279	// PCIE MSI 23 Interrupt
#define PCIE_MSI24			280	// PCIE MSI 24 Interrupt
#define PCIE_MSI25			281	// PCIE MSI 25 Interrupt
#define PCIE_MSI26			282	// PCIE MSI 26 Interrupt
#define PCIE_MSI27			283	// PCIE MSI 27 Interrupt
#define PCIE_MSI28			284	// PCIE MSI 28 Interrupt
#define PCIE_MSI29			285	// PCIE MSI 29 Interrupt
#define PCIE_MSI30			286	// PCIE MSI 30 Interrupt
#define PCIE_MSI31			287	// PCIE MSI 31 Interrupt

#elif SUB_PLATFORM_S8001

#define INT_SW0				0	// Software 0 Interrupt
#define INT_SW1				1	// Software 1 Interrupt
#define INT_SW2				2	// Software 2 Interrupt
#define INT_SW3				3	// Software 3 Interrupt
#define INT_WDT				4	// Watchdog Timer Interrupt
#define INT_DWI1			5	// DWI Interrupt
#define INT_DWI2			6	// DWI Interrupt
#define INT_PMGR_PERF_CNTRS		7	// PMGR Performace Counters Interrupt
#define INT_THERM_TEMP0			8	// Thermal Temp 0 Interrupt
#define INT_THERM_ALARM0		9	// Thermal Alarm 0 Interrupt
#define INT_THERM_TEMP1			10	// Thermal Temp 1 Interrupt
#define INT_THERM_ALARM1		11	// Thermal Alarm 1 Interrupt
#define INT_THERM_TEMP2			12	// Thermal Temp 2 Interrupt
#define INT_THERM_ALARM2		13	// Thermal Alarm 2 Interrupt
#define INT_THERM_TEMP3			14	// Thermal Temp 3 Interrupt
#define INT_THERM_ALARM3		15	// Thermal Alarm 3 Interrupt
#define INT_CPM_THERM_TEMP		16	// CPM Thermal temp Interrupt
#define INT_CPM_THERM_ALARM		17	// CPM Thermal alarm Interrupt
#define INT_CPU0_THERM_TEMP		18	// CPU0 Thermal temp Interrupt
#define INT_CPU0_THERM_ALARM		19	// CPU0 Thermal alarm Interrupt
#define INT_CPU1_THERM_TEMP		20	// CPU1 Thermal temp Interrupt
#define INT_CPU1_THERM_ALARM		21	// CPU1 Thermal alarm Interrupt

// Reserved 22 - 25

#define INT_THERM_FS_ACT0		26	// Thermal Failsafe Active 0 Interrupt
#define INT_THERM_FS_ACT1		27	// Thermal Failsafe Active 1 Interrupt
#define INT_THERM_FS_EXT0		28	// Thermal Failsafe Exit 0 Interrupt
#define INT_THERM_FS_EXT1		29	// Thermal Failsafe Exit 1 Interrupt
#define INT_SOC_THERM_FS_ACT0		30	// SoC Thermal Failsafe Active 0 Interrupt
#define INT_SOC_THERM_FS_ACT1		31	// SoC Thermal Failsafe Active 1 Interrupt
#define INT_PMGR_EVENT			32	// PMGR Event Interrupt
#define INT_CTM_TRIGGER0		33	// CTM Trigger 0 Interrupt
#define INT_CTM_TRIGGER1		34	// CTM Trigger 1 Interrupt
#define INT_CTM_TRIGGER2		35	// CTM Trigger 2 Interrupt
#define INT_CTM_TRIGGER3		36	// CTM Trigger 3 Interrupt
#define INT_COHERENCE_PNT0_ERR		37	// Coherence Point Error Interrupt
#define INT_COHERENCE_PNT0_PERF_CNTRS	38	// Coherence Point Performance Counters Interrupt
#define INT_COHERENCE_PNT1_ERR		39	// Coherence Point Error Interrupt
#define INT_COHERENCE_PNT1_PERF_CNTRS	40	// Coherence Point Performance Counters Interrupt

// Reserved 41

#define INT_GPIO_CLASS0			42	// GPIO Class 0 Interrupt
#define INT_GPIO_CLASS1			43	// GPIO Class 1 Interrupt
#define INT_GPIO_CLASS2			44	// GPIO Class 2 Interrupt
#define INT_GPIO_CLASS3			45	// GPIO Class 3 Interrupt
#define INT_GPIO_CLASS4			46	// GPIO Class 4 Interrupt
#define INT_GPIO_CLASS5			47	// GPIO Class 5 Interrupt
#define INT_GPIO_CLASS6			48	// GPIO Class 6 Interrupt

// Reserved 49

#define INT_MINI_PMGR_PERF_CNTRS	50	// MINIPMGR PerfCounter Interrupt
#define INT_GFX_PERF_CNTRS		51	// GFX Perf Counter Interrupt
#define INT_MEM_CONTROLLER0		52	// Memory Controller MCC Interrupt
#define INT_MEM_CONTROLLER0_1		53	// Memory Controller MCC Interrupt
#define INT_MEM_CONTROLLER1		54	// Memory Controller MCC Interrupt
#define INT_MEM_CONTROLLER1_1		55	// Memory Controller MCC Interrupt

// Reserved 56 - 80

#define INT_CPU0_COMM_TX		57	// CPU 0 COMM TX Interrupt
#define INT_CPU0_COMM_RX		58	// CPU 0 COMM RX Interrupt
#define INT_CPU0_PMU_IRQ		59	// CPU 0 PMU IRQ Interrupt
#define INT_CPU1_COMM_TX		60	// CPU 1 COMM TX Interrupt
#define INT_CPU1_COMM_RX		61	// CPU 1 COMM RX Interrupt
#define INT_CPU1_PMU_IRQ		62	// CPU 1 PMU IRQ Interrupt

// Reserved 87 - 92

#define INT_CCC_UNIFIED_TRIGGER		93	// CCC Unified Trigger Interrupt

// Reserved 94 - 103

#define INT_ISP_KF_INBOX_EMPTY		104	// ISP KF Inbox Empty Interrupt
#define INT_ISP_KF_INBOX_NOTEMPTY	105	// ISP KF Inbox Not Empty Interrupt
#define INT_ISP_KF_OUTBOX_EMPTY		106	// ISP KF Outbox Empty Interrupt
#define INT_ISP_KF_OUTBOX_NOTEMPTY	107	// ISP KF Outbox Not Empty Interrupt
#define INT_ISP0			108	// ISP 0 Interrupt
#define INT_ISP1			109	// ISP 1 Interrupt
#define INT_ISP2			110	// ISP 2 Interrupt
#define INT_ISP3			111	// ISP 3 Interrupt
#define INT_ISP_DART			112	// ISP DART Interrupt

// Reserved 113

#define INT_SEP_KF_INBOX_EMPTY		114	// SEP KF Inbox Empty Interrupt
#define INT_SEP_KF_INBOX_NOTEMPTY	115	// SEP KF Inbox Not Empty Interrupt
#define INT_SEP_KF_OUTBOX_EMPTY		116	// SEP KF Outbox Empty Interrupt
#define INT_SEP_KF_OUTBOX_NOTEMPTY	117	// SEP KF Outbox Not Empty Interrupt
#define INT_SEP_AUTH_ERROR		118	// SEP Authentication Error Interrupt
#define INT_SEP_CRIT_PATH_MONITOR	119	// SEP Critical Path Monitor Interrupt

// Reserved 120

// XXX JF: AOP interrupts

// Reserved 153

#define INT_DISP0			154	// Display 0 Interrupt
#define INT_DISP0_DART			155	// Display 0 DART Interrupt
#define INT_DISP0_SMMU			156	// Display 0 SMMU Interrupt
#define INT_DISP0_BACK_END_BLU		157	// Display 0 Back End Back Light Update Interrupt
#define INT_DISP0_BACK_END_VBI		158	// Display 0 Back End VBI Interrupt
#define INT_DISP0_BACK_END_VFTG		159	// Display 0 Back End VFTG Idle Interrupt
#define INT_DISP0_BACK_END_VBL		160	// Display 0 Back End VBL counter Interrupt
#define INT_DISPLAY_PORT0		161	// Display Port 0
#define INT_EDP0			INT_DISPLAY_PORT0
#define	INT_DPORT0			INT_DISPLAY_PORT0

// Reserved 162

#define INT_DISP1			163	// Display 1 Interrupt
#define INT_DISP1_DART			164	// Display 1 DART Interrupt
#define INT_DISP1_SMMU			165	// Display 1 SMMU Interrupt
#define INT_DISP1_BACK_END_BLU		166	// Display 1 Back End Back Light Update Interrupt
#define INT_DISP1_BACK_END_VBI		167	// Display 1 Back End VBI Interrupt
#define INT_DISP1_BACK_END_VFTG		168	// Display 1 Back End VFTG Idle Interrupt
#define INT_DISP1_BACK_END_VBL		169	// Display 1 Back End VBL counter Interrupt
#define INT_DISPLAY_PORT1		170	// Display Port 1

// Reserved 171

// XXX: PMP

// Reserved 176

#define INT_VDEC0			177	// Video Decoder 0 Interrupt

// Reserved 178

#define INT_AVE_KF_INBOX_EMPTY		179	// AVE KF Inbox Empty Interrupt
#define INT_AVE_KF_INBOX_NOTEMPTY	180	// AVE KF Inbox Not Empty Interrupt
#define INT_AVE_KF_OUTBOX_EMPTY		181	// AVE KF Outbox Empty Interrupt
#define INT_AVE_KF_OUTBOX_NOTEMPTY	182	// AVE KF Outbox Not Empty Interrupt
#define INT_AVE_DART			183	// AVE DART Interrupt
#define INT_AVE_SMMU			184	// AVE SMMU Interrupt
#define INT_AVE				185	// AVE Interrupt
#define INT_SVE_SYNC			186	// SVE Driver Synchronization Interrupt

// Reserved 187 - 189

#define INT_GFX				190	// GFX Interrupt
#define INT_GFX_SLEEP			191	// GFX Sleep Interrupt
#define INT_GFX_WAKE			192	// GFX Wake Interrupt
#define INT_GFX_CHG			193	// GFX Change Interrupt
#define INT_GFX_FW			194	// GFX Change Interrupt

#define	INT_SRS				195 	// SRS Interrupt
#define INT_SRS_STG1			196 	// SRS STG1 Interrupt
#define INT_SRS_BWM			197 	// SRS BWM Interrupt
#define INT_SRS_NM			198 	// SRS NM Interrupt
#define INT_SRS_DART			199 	// SRS DART Interrupt
#define INT_SRS_STT			200 	// SRS STT Interrupt

#define INT_JPEG			201	// JPEG Interrupt
#define INT_JPEG_DART			202	// JPEG DART Interrupt
#define INT_JPEG_DART_PERF		203	// JPEG DART Performance Interrupt

// Reserved 204

#define INT_MSR				205	// MSR Interrupt
#define INT_MSR_SMMU			206	// MSR SMMU Interrupt
#define INT_MSR_DART			207	// MSR DART Interrupt
#define INT_MSR_DAR_PERF		208	// MSR DART Performance Interrupt

// Reserved 209

#define INT_SIO_KF_INBOX_EMPTY		210	// SIO KF Inbox Empty Interrupt
#define INT_SIO_KF_INBOX_NOTEMPTY	211	// SIO KF Inbox Not Empty Interrupt
#define INT_SIO_KF_OUTBOX_EMPTY		212	// SIO KF Outbox Empty Interrupt
#define INT_SIO_KF_OUTBOX_NOTEMPTY	213	// SIO KF Outbox Not Empty Interrupt
#define INT_SPI0			214	// SPI 0 Interrupt
#define INT_SPI1			215	// SPI 1 Interrupt
#define INT_SPI2			216	// SPI 2 Interrupt
#define INT_SPI3			217	// SPI 3 Interrupt
#define INT_UART0			218	// UART 0 Interrupt
#define INT_UART1			219	// UART 1 Interrupt
#define INT_UART2			220	// UART 2 Interrupt
#define INT_UART3			221	// UART 3 Interrupt
#define INT_UART4			222	// UART 4 Interrupt
#define INT_UART5			223	// UART 5 Interrupt
#define INT_UART6			224	// UART 6 Interrupt
#define INT_UART7			225	// UART 7 Interrupt
#define INT_UART8			226	// UART 8 Interrupt
#define INT_MCA0			227	// Multi-Channel Audio 0 Interrupt
#define INT_MCA1			228	// Multi-Channel Audio 1 Interrupt
#define INT_MCA2			229	// Multi-Channel Audio 2 Interrupt
#define INT_MCA3			230	// Multi-Channel Audio 3 Interrupt
#define INT_MCA4			231	// Multi-Channel Audio 4 Interrupt
#define INT_IIC0			232	// I2C 0 Interrupt
#define INT_IIC1			233	// I2C 1 Interrupt
#define INT_IIC2			234	// I2C 2 Interrupt
#define INT_IIC3			235	// I2C 3 Interrupt
#define INT_PWM				236	// PWM Interrupt
#define INT_AES0			237	// AES 0 Interrupt

// Reserved 238 - 240

#define INT_USB_OTG			241	// USB OTG Interrupt
#define INT_USB2HOST0_EHCI		242	// USB EHCI 0 Interrupt
#define INT_USB2HOST0_OHCI		243	// USB OHCI 0 Interrupt
#define INT_USB2HOST1_EHCI		244	// USB EHCI 1 Interrupt
#define INT_USB2HOST2_EHCI		245	// USB EHCI 2 Interrupt
#define INT_USBOTG_RESUME		246	// USB OTG Resume Interrupt
#define INT_USB_20_BATT_CHRG_DET	247	// USB 2.0 Battery Charge Detect Interrupt
#define INT_USB2HOST0_PHY_RESUME	248	// USB Host 0 Resume Interrupt
#define INT_USB2HOST1_PHY_RESUME	249	// USB Host 1 Resume Interrupt
#define INT_USB2HOST2_PHY_RESUME	250	// USB Host 2 Resume Interrupt
#define INT_OTG_IN_EP0			251	// OTG IN Endpoint 0 Interrupt
#define INT_OTG_IN_EP1			252	// OTG IN Endpoint 1 Interrupt
#define INT_OTG_IN_EP2			253	// OTG IN Endpoint 2 Interrupt
#define INT_OTG_IN_EP3			254	// OTG IN Endpoint 3 Interrupt
#define INT_OTG_IN_EP4			255	// OTG IN Endpoint 4 Interrupt
#define INT_OTG_IN_EP5			256	// OTG IN Endpoint 5 Interrupt
#define INT_OTG_IN_EP6			257	// OTG IN Endpoint 6 Interrupt
#define INT_OTG_IN_EP7			258	// OTG IN Endpoint 7 Interrupt
#define INT_OTG_IN_EP8			259	// OTG IN Endpoint 8 Interrupt
#define INT_OTG_OUT_EP0			260	// OTG OUT Endpoint 0 Interrupt
#define INT_OTG_OUT_EP1			261	// OTG OUT Endpoint 1 Interrupt
#define INT_OTG_OUT_EP2			262	// OTG OUT Endpoint 2 Interrupt
#define INT_OTG_OUT_EP3			263	// OTG OUT Endpoint 3 Interrupt
#define INT_OTG_OUT_EP4			264	// OTG OUT Endpoint 4 Interrupt
#define INT_OTG_OUT_EP5			265	// OTG OUT Endpoint 5 Interrupt
#define INT_OTG_OUT_EP6			266	// OTG OUT Endpoint 6 Interrupt
#define INT_OTG_OUT_EP7			267	// OTG OUT Endpoint 7 Interrupt
#define INT_OTG_OUT_EP8			268	// OTG OUT Endpoint 8 Interrupt

// Reserved 269

#define PCIE_LINK0			270	// PCIE Link 0 Interrupt
#define PCIE_LINK0_DART			271	// PCIE Link 0 DART Interrupt
#define PCIE_LINK0_MMU			272	// PCIE Link 0 MMU Interrupt
#define PCIE_LINK1			273	// PCIE Link 1 Interrupt
#define PCIE_LINK1_DART			274	// PCIE Link 1 DART Interrupt
#define PCIE_LINK1_MMU			275	// PCIE Link 1 MMU Interrupt
#define PCIE_LINK2			276	// PCIE Link 2 Interrupt
#define PCIE_LINK2_DART			277	// PCIE Link 2 DART Interrupt
#define PCIE_LINK2_MMU			278	// PCIE Link 2 MMU Interrupt
#define PCIE_LINK3			279	// PCIE Link 3 Interrupt
#define PCIE_LINK3_DART			280	// PCIE Link 3 DART Interrupt
#define PCIE_LINK3_MMU			281	// PCIE Link 3 MMU Interrupt
#define PCIE_LINK4			282	// PCIE Link 4 Interrupt
#define PCIE_LINK4_DART			283	// PCIE Link 4 DART Interrupt
#define PCIE_LINK4_MMU			284	// PCIE Link 4 MMU Interrupt
#define PCIE_LINK5			285	// PCIE Link 5 Interrupt
#define PCIE_LINK5_DART			286	// PCIE Link 5 DART Interrupt
#define PCIE_LINK5_MMU			287	// PCIE Link 5 MMU Interrupt
#define PCIE_MSI0			288	// PCIE MSI 0 Interrupt
#define PCIE_MSI1			289	// PCIE MSI 1 Interrupt
#define PCIE_MSI2			290	// PCIE MSI 2 Interrupt
#define PCIE_MSI3			291	// PCIE MSI 3 Interrupt
#define PCIE_MSI4			292	// PCIE MSI 4 Interrupt
#define PCIE_MSI5			293	// PCIE MSI 5 Interrupt
#define PCIE_MSI6			294	// PCIE MSI 6 Interrupt
#define PCIE_MSI7			295	// PCIE MSI 7 Interrupt
#define PCIE_MSI8			296	// PCIE MSI 8 Interrupt
#define PCIE_MSI9			297	// PCIE MSI 9 Interrupt
#define PCIE_MSI10			298	// PCIE MSI 10 Interrupt
#define PCIE_MSI11			299	// PCIE MSI 11 Interrupt
#define PCIE_MSI12			300	// PCIE MSI 12 Interrupt
#define PCIE_MSI13			301	// PCIE MSI 13 Interrupt
#define PCIE_MSI14			302	// PCIE MSI 14 Interrupt
#define PCIE_MSI15			303	// PCIE MSI 15 Interrupt
#define PCIE_MSI16			304	// PCIE MSI 16 Interrupt
#define PCIE_MSI17			305	// PCIE MSI 17 Interrupt
#define PCIE_MSI18			306	// PCIE MSI 18 Interrupt
#define PCIE_MSI19			307	// PCIE MSI 19 Interrupt
#define PCIE_MSI20			308	// PCIE MSI 20 Interrupt
#define PCIE_MSI21			309	// PCIE MSI 21 Interrupt
#define PCIE_MSI22			310	// PCIE MSI 22 Interrupt
#define PCIE_MSI23			311	// PCIE MSI 23 Interrupt
#define PCIE_MSI24			312	// PCIE MSI 24 Interrupt
#define PCIE_MSI25			313	// PCIE MSI 25 Interrupt
#define PCIE_MSI26			314	// PCIE MSI 26 Interrupt
#define PCIE_MSI27			315	// PCIE MSI 27 Interrupt
#define PCIE_MSI28			316	// PCIE MSI 28 Interrupt
#define PCIE_MSI29			317	// PCIE MSI 29 Interrupt
#define PCIE_MSI30			318	// PCIE MSI 30 Interrupt
#define PCIE_MSI31			319	// PCIE MSI 31 Interrupt

#else
#error Unsupported Platform
#endif

#endif /* ! __PLATFORM_SOC_HWISR_H */
