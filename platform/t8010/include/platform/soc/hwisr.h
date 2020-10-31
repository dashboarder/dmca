/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
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

#if SUB_PLATFORM_T8010

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

// Reserved 21 - 25

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
#define INT_COHERENCE_PNT_ERR		37	// Coherence Point Error Interrupt
#define INT_COHERENCE_PNT_PERF_CNTRS	38	// Coherence Point Performance Counter Interrupt

// Reserved 39-41

#define INT_GPIO_CLASS0			42	// GPIO Class 0 Interrupt
#define INT_GPIO_CLASS1			43	// GPIO Class 1 Interrupt
#define INT_GPIO_CLASS2			44	// GPIO Class 2 Interrupt
#define INT_GPIO_CLASS3			45	// GPIO Class 3 Interrupt
#define INT_GPIO_CLASS4			46	// GPIO Class 4 Interrupt
#define INT_GPIO_CLASS5			47	// GPIO Class 5 Interrupt
#define INT_GPIO_CLASS6			48	// GPIO Class 6 Interrupt

// Reserved 49

#define INT_MINIMPGR_PERF_CTR		50	// MINIPMGR Performance Counter Interrupt
#define INT_GFX_PERF_CTR		51	// GFX Performance Counter Interrupt
#define INT_MEM_CONTROLLER0		52	// Memory Controller MCC0 Interrupt
#define INT_MEM_CONTROLLER1		53	// Memory Controller MCC1 Interrupt

// Reserved 54-55

#define INT_MEM_CTLR_DCS0_AMCX0		56	// Memory Controller DCS0 AMCX 0 Interrupt
#define INT_MEM_CTLR_DCS0_AMCX1		57	// Memory Controller DCS0 AMCX 1 Interrupt
#define INT_MEM_CTLR_DCS0_AMP_CAL_ERR	58	// Memory Controller DCS0 AMP Cal Error 1 Interrupt
#define INT_MEM_CTLR_DCS1_AMCX0		59	// Memory Controller DCS1 AMCX 0 Interrupt
#define INT_MEM_CTLR_DCS1_AMCX1		60	// Memory Controller DCS1 AMCX 1 Interrupt
#define INT_MEM_CTLR_DCS1_AMP_CAL_ERR	61	// Memory Controller DCS1 AMP Cal Error 1 Interrupt
#define INT_MEM_CTLR_DCS2_AMCX0		62	// Memory Controller DCS2 AMCX 0 Interrupt
#define INT_MEM_CTLR_DCS2_AMCX1		63	// Memory Controller DCS2 AMCX 1 Interrupt
#define INT_MEM_CTLR_DCS2_AMP_CAL_ERR	64	// Memory Controller DCS2 AMP Cal Error 1 Interrupt
#define INT_MEM_CTLR_DCS3_AMCX0		65	// Memory Controller DCS3 AMCX 0 Interrupt
#define INT_MEM_CTLR_DCS3_AMCX1		66	// Memory Controller DCS3 AMCX 1 Interrupt
#define INT_MEM_CTLR_DCS3_AMP_CAL_ERR	67	// Memory Controller DCS3 AMP Cal Error 1 Interrupt

// Reserved 68-80

#define INT_CPU0_COMM_TX		81	// CPU 0 COMM TX Interrupt
#define INT_CPU0_COMM_RX		82	// CPU 0 COMM RX Interrupt
#define INT_CPU0_PMU_IRQ		83	// CPU 0 PMU IRQ Interrupt
#define INT_CPU1_COMM_TX		84	// CPU 1 COMM TX Interrupt
#define INT_CPU1_COMM_RX		85	// CPU 1 COMM RX Interrupt
#define INT_CPU1_PMU_IRQ		86	// CPU 1 PMU IRQ Interrupt

// Reserved 87 - 92

#define INT_CCC_UNIFIED_TRIGGER		93	// CCC Unified Trigger Interrupt

// Reserved 94

#define INT_GFX_THROTTLING_TRIGGER	95	// GFX Throttling Trigger Interrupt

// Reserved 96 - 103

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

// Reserved 120 - 122

#define INT_DOCK_CHANNELS		123	// Dock Channels Interrupt
#define INT_SCM_SPI			124	// SCM SPI Interrupt
#define INT_AOP_UART0			125	// AOP UART0 Interrupt
#define INT_AOP_UART1			126	// AOP UART1 Interrupt
#define INT_AOP_I2CM			127	// AOP I2CM Interrupt
#define INT_AOP_GPIO_IRQ0		128	// AOP GPIO IRQ0 Interrupt
#define INT_AOP_GPIO_IRQ1		129	// AOP GPIO IRQ1 Interrupt
#define INT_AOP_GPIO_IRQ2		130	// AOP GPIO IRQ2 Interrupt
#define INT_AOP_GPIO_IRQ3		131	// AOP GPIO IRQ3 Interrupt
#define INT_AOP_GPIO_IRQ4		132	// AOP GPIO IRQ4 Interrupt
#define INT_AOP_GPIO_IRQ5		133	// AOP GPIO IRQ5 Interrupt
#define INT_AOP_GPIO_IRQ6		134	// AOP GPIO IRQ6 Interrupt
#define INT_WAKEUP_AP			135	// AOP Wakeup AP Interrupt
#define INT_CPU_WAKEUP_AP		136	// AOP CPU Wakeup AP Interrupt
#define INT_SCM_RX_BUFFER_FULL0		137	// AOP SCM Buffer Full 0 Interrupt
#define INT_SCM_RX_BUFFER_FULL1		138	// AOP SCM Buffer Full 1 Interrupt
#define INT_SCM_MAX_COUNT0		139	// AOP SCM Max Count 0 Interrupt
#define INT_SCM_MAX_COUNT1		140	// AOP SCM Max Count 1 Interrupt
#define INT_SCM_SLAVE_ERROR		141	// AOP SCM Slave Error Interrupt
#define INT_AOP_UART2			142	// AOP UART2 Interrupt
#define INT_AOP_MCA0			143	// AOP MCA0 Interrupt
#define INT_AOP_HPD_DETECT		144	// AOP HPD Detect Interrupt
#define INT_AOP_KF_INBOX_EMPTY		145	// AOP KF Inbox Empty Interrupt
#define INT_AOP_KF_INBOX_NOTEMPTY	146	// AOP KF Inbox Not Empty Interrupt
#define INT_AOP_KF_OUTBOX_EMPTY		147	// AOP KF Outbox Empty Interrupt
#define INT_AOP_KF_OUTBOX_NOTEMPTY	148	// AOP KF Outbox Not Empty Interrupt
#define INT_AOP_AP_WATCHDOG		149	// AOP MCA0 Interrupt
#define INT_AOP_AP_WAKEUP_TIME		150	// AOP MCA0 Interrupt
#define INT_AOP_CDBGPWRUPREQ		151	// AOP CDBGPWRUPREQ Interrupt
#define INT_AOP_HPD_ERROR		152	// AOP HPD Error Interrupt

// Reserved 153

#define INT_DISP0			154	// Display 0 Interrupt
#define INT_DISP0_PIO_DMA		155	// Display 0 PIO DMA Interrupt
#define INT_DISP0_DART			156	// Display 0 DART Interrupt
#define INT_DISP0_SMMU			157	// Display 0 SMMU Interrupt
#define INT_DISP0_BACK_END_BLU		158	// Display 0 Back End Back Light Update Interrupt
#define INT_DISP0_BACK_END_VBI		159	// Display 0 Back End VBI Interrupt
#define INT_DISP0_BACK_END_VFTG		160	// Display 0 Back End VFTG Idle Interrupt
#define INT_DISP0_BACK_END_VBL		161	// Display 0 Back End VBL counter Interrupt
#define INT_DISPLAY_PORT_TX0		162	// Display Port TX 0 Interrupt
#define INT_MIPIDSI0			163	// MIPI DSI 0 Interrupt
#define INT_EDP0			INT_DISPLAY_PORT
#define	INT_DPORT0			INT_DISPLAY_PORT
#define INT_DISP0_BACK_END_VBI2		164	// Display 0 Back End - VBI2 Interrupt
#define INT_DISP0_SUBFRAME_TIMESTAMP	165	// Display 0 Sub Frame Line Count TimeStamp Interrupt
#define INT_DISP0_BACK_END_PARAM_FIFO	166	// Display 0 Back End - Param FIFO completion Interrupt
#define INT_PMP_KF_INBOX_EMPTY		167	// PMP KF Inbox Empty Interrupt
#define INT_PMP_KF_INBOX_NOTEMPTY	168	// PMP KF Inbox Not Empty Interrupt
#define INT_PMP_KF_OUTBOX_EMPTY		169	// PMP KF Outbox Empty Interrupt
#define INT_PMP_KF_OUTBOX_NOTEMPTY	170	// PMP KF Outbox Not Empty Interrupt

// Reserved 171

#define INT_VDEC0			172	// Video Decoder 0 Interrupt

// Reserved 173

#define INT_AVE_KF_INBOX_EMPTY		174	// AVE KF Inbox Empty Interrupt
#define INT_AVE_KF_INBOX_NOTEMPTY	175	// AVE KF Inbox Not Empty Interrupt
#define INT_AVE_KF_OUTBOX_EMPTY		176	// AVE KF Outbox Empty Interrupt
#define INT_AVE_KF_OUTBOX_NOTEMPTY	177	// AVE KF Outbox Not Empty Interrupt
#define INT_AVE_SMMU			178	// AVE SMMU Interrupt
#define INT_AVE_DART			179	// AVE DART Interrupt
#define INT_AVE				180	// AVE Interrupt
#define INT_SVE_SYNC			181	// SVE Driver Synchronization Interrupt

// Reserved 182 - 184

#define INT_GFX				185	// GFX Interrupt
#define INT_GFX_SLEEP			186	// GFX Sleep Interrupt
#define INT_GFX_WAKE			187	// GFX Wake Interrupt
#define INT_GFX_CHG			188	// GFX Change Interrupt
#define INT_GHX_KF_INBOX_EMPTY		189	// GHX KF Inbox Empty Interrupt
#define INT_GHX_KF_INBOX_NOTEMPTY	190	// GHX KF Inbox Not Empty Interrupt
#define INT_GHX_KF_OUTBOX_EMPTY		191	// GHX KF Outbox Empty Interrupt
#define INT_GHX_KF_OUTBOX_NOTEMPTY	192	// GHX KF Outbox Not Empty Interrupt
#define INT_GHX_FIRMWARE		193	// GHX Firmware Only Interrupt

// Reserved 194

#define INT_GPU_TIMER			195	// GPU Timer Interrupt
#define INT_JPEG0			196	// JPEG0 Interrupt
#define INT_JPEG0_DART			197	// JPEG0 DART Interrupt
#define INT_JPEG0_DART_PERF		198	// JPEG0 DART Performance Interrupt
#define INT_JPEG1			199	// JPEG1 Interrupt
#define INT_JPEG1_DART			200	// JPEG1 DART Interrupt
#define INT_JPEG1_DART_PERF		201	// JPEG1 DART Performance Interrupt

// Reserved 202 - 203

#define INT_MSR				204	// MSR Interrupt
#define INT_MSR_PIO_DMA			205	// MSR PIO DMA Interrupt
#define INT_MSR_SMMU			206	// MSR SMMU Interrupt
#define INT_MSR_DART			207	// MSR DART Interrupt
#define INT_MSR_DART_PERF		208	// MSR DART Performance Interrupt

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
#define INT_HFD				238	// HFD Interrupt

// Reserved 239 - 240

#define INT_USB_OTG			241	// USB OTG Interrupt
#define INT_USB2HOST0_EHCI		242	// USB EHCI 0 Interrupt
#define INT_USB2HOST0_OHCI		243	// USB OHCI 0 Interrupt
#define INT_USB2HOST1_EHCI		244	// USB EHCI 1 Interrupt

// Reserved 245

#define INT_USBOTG_RESUME		246	// USB OTG Resume Interrupt
#define INT_USB_20_BATT_CHRG_DET	247	// USB 2.0 Battery Charge Detect Interrupt
#define INT_USB2HOST0_PHY_RESUME	248	// USB Host 0 Resume Interrupt
#define INT_USB2HOST1_PHY_RESUME	249	// USB Host 1 Resume Interrupt

// Reserved 250

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
#define PCIE_LINK2			276	// PCIE Link 1 Interrupt
#define PCIE_LINK2_DART			277	// PCIE Link 1 DART Interrupt
#define PCIE_LINK2_MMU			278	// PCIE Link 1 MMU Interrupt
#define PCIE_LINK3			279	// PCIE Link 1 Interrupt
#define PCIE_LINK3_DART			280	// PCIE Link 1 DART Interrupt
#define PCIE_LINK3_MMU			281	// PCIE Link 1 MMU Interrupt

// Reserved 282 - 287

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
