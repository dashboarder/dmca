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

#define INT_SW0				0	// 0x00: Software 0 Interrupt
#define INT_SW1				1	// 0x01: Software 1 Interrupt
#define INT_SW2				2	// 0x02: Software 2 Interrupt
#define INT_SW3				3	// 0x03: Software 3 Interrupt
#define INT_WDT				4	// 0x04: Watchdog Timer Interrupt
#define INT_DWI				5	// 0x05: DWI Interrupt
#define INT_PMGR_PERF_CNTRS		6	// 0x06: PMGR Performace Counters Interrupt
#define INT_THERM_TEMP0			7	// 0x07: Thermal Temp 0 Interrupt
#define INT_THERM_ALARM0		8	// 0x08: Thermal Alarm 0 Interrupt
#define INT_THERM_TEMP1			9	// 0x09: Thermal Temp 1 Interrupt
#define INT_THERM_ALARM1		10	// 0x0a: Thermal Alarm 1 Interrupt

// Reserved 11 - 21

#define INT_CPM_THERM_TEMP		22	// 0x16: CPM Thermal temp Interrupt
#define INT_CPM_THERM_ALARM		23	// 0x17: CPM Thermal alarm Interrupt
#define INT_CPU0_THERM_TEMP		24	// 0x18: CPU0 Thermal temp Interrupt
#define INT_CPU0_THERM_ALARM		25	// 0x19: CPU0 Thermal alarm Interrupt
#define INT_CPU1_THERM_TEMP		26	// 0x1a: CPU1 Thermal temp Interrupt
#define INT_CPU1_THERM_ALARM		27	// 0x1b: CPU1 Thermal alarm Interrupt
#if SUB_PLATFORM_T7001
#define INT_CPU2_THERM_TEMP		28	// 0x1c: CPU2 Thermal temp Interrupt
#define INT_CPU2_THERM_ALARM		29	// 0x1d: CPU2 Thermal alarm Interrupt
#endif

// Reserved 28 - 35 (T7000)
// Reserved 30 - 35 (T7001)

#define INT_ANS_KF_INBOX_EMPTY		36	// 0x24: ANS KF Inbox Empty Interrupt
#define INT_ANS_KF_INBOX_NOTEMPTY	37	// 0x25: ANS KF Inbox Not Empty Interrupt
#define INT_ANS_KF_OUTBOX_EMPTY		38	// 0x26: ANS KF Outbox Empty Interrupt
#define INT_ANS_KF_OUTBOX_NOTEMPTY	39	// 0x27: ANS KF Outbox Not Empty Interrupt
#define INT_ANS_CHNL0			40	// 0x28: ANS Channel 0 Interrupt
#define INT_ANS_CHNL1			41	// 0x29: ANS Channel 1 Interrupt

// Reserved 42 - 46

#define INT_THERM_FS_ACT0		47	// 0x2f: Thermal Failsafe Active 0 Interrupt
#define INT_THERM_FS_ACT1		48	// 0x30: Thermal Failsafe Active 1 Interrupt
#define INT_THERM_FS_EXT0		49	// 0x31: Thermal Failsafe Exit 0 Interrupt
#define INT_THERM_FS_EXT1		50	// 0x32: Thermal Failsafe Exit 1 Interrupt
#define INT_SOC_THERM_FS_ACT0		51	// 0x33: SoC Thermal Failsafe Active 0 Interrupt
#define INT_SOC_THERM_FS_ACT1		52	// 0x34: SoC Thermal Failsafe Active 1 Interrupt

// Reserved 53 - 54

#define INT_CTM_TRIGGER0		55	// 0x37: CTM Trigger 0 Interrupt
#define INT_CTM_TRIGGER1		56	// 0x38: CTM Trigger 1 Interrupt
#define INT_CTM_TRIGGER2		57	// 0x39: CTM Trigger 2 Interrupt
#define INT_CTM_TRIGGER3		58	// 0x3a: CTM Trigger 3 Interrupt
#define INT_COHERENCE_PNT_ERR		59	// 0x3b: Coherence Point Error Interrupt
#define INT_COHERENCE_PNT_PERF_CNTRS	60	// 0x3c: Coherence Point Performance Counters Interrupt

// Reserved 61

#define INT_GPIO_CLASS0			62	// 0x3e: GPIO Class 0 Interrupt
#define INT_GPIO_CLASS1			63	// 0x3f: GPIO Class 1 Interrupt
#define INT_GPIO_CLASS2			64	// 0x40: GPIO Class 2 Interrupt
#define INT_GPIO_CLASS3			65	// 0x41: GPIO Class 3 Interrupt
#define INT_GPIO_CLASS4			66	// 0x42: GPIO Class 4 Interrupt
#define INT_GPIO_CLASS5			67	// 0x43: GPIO Class 5 Interrupt
#define INT_GPIO_CLASS6			68	// 0x44: GPIO Class 6 Interrupt

// Reserved 61

#define INT_MEM_CONTROLLER		70	// 0x46: Memory Controller MCC Interrupt
#define INT_MCU				71	// 0x47: Memory Controller MCU Interrupt

// Reserved 72

#define INT_CPU0_COMM_TX		73	// 0x49: CPU 0 COMM TX Interrupt
#define INT_CPU0_COMM_RX		74	// 0x4a: CPU 0 COMM RX Interrupt
#define INT_CPU0_PMU_IRQ		75	// 0x4b: CPU 0 PMU IRQ Interrupt
#define INT_CPU1_COMM_TX		76	// 0x4c: CPU 1 COMM TX Interrupt
#define INT_CPU1_COMM_RX		77	// 0x4d: CPU 1 COMM RX Interrupt
#define INT_CPU1_PMU_IRQ		78	// 0x4e: CPU 1 PMU IRQ Interrupt
#if SUB_PLATFORM_T7001
#define INT_CPU2_COMM_TX		79	// 0x4f: CPU 2 COMM TX Interrupt
#define INT_CPU2_COMM_RX		80	// 0x50: CPU 2 COMM RX Interrupt
#define INT_CPU2_PMU_IRQ		81	// 0x51: CPU 2 PMU IRQ Interrupt
#endif

// Reserved 79 - 84 (T7000)
// Reserved 82 - 84 (T7001)

#define INT_CCC_UNIFIED_TRIGGER		85	// 0x55: CCC Unified Trigger Interrupt

// Reserved 86

#define INT_ISP_KF_INBOX_EMPTY		87	// 0x57: ISP KF Inbox Empty Interrupt
#define INT_ISP_KF_INBOX_NOTEMPTY	88	// 0x58: ISP KF Inbox Not Empty Interrupt
#define INT_ISP_KF_OUTBOX_EMPTY		89	// 0x59: ISP KF Outbox Empty Interrupt
#define INT_ISP_KF_OUTBOX_NOTEMPTY	90	// 0x5a: ISP KF Outbox Not Empty Interrupt
#define INT_ISP0			91	// 0x5b: ISP 0 Interrupt
#define INT_ISP1			92	// 0x5c: ISP 1 Interrupt
#define INT_ISP2			93	// 0x5d: ISP 2 Interrupt
#define INT_ISP3			94	// 0x5e: ISP 3 Interrupt
#define INT_ISP_DART			95	// 0x5f: ISP DART Interrupt

// Reserved 96

#define INT_SEP_KF_INBOX_EMPTY		97	// 0x61: SEP KF Inbox Empty Interrupt
#define INT_SEP_KF_INBOX_NOTEMPTY	98	// 0x62: SEP KF Inbox Not Empty Interrupt
#define INT_SEP_KF_OUTBOX_EMPTY		99	// 0x63: SEP KF Outbox Empty Interrupt
#define INT_SEP_KF_OUTBOX_NOTEMPTY	100	// 0x64: SEP KF Outbox Not Empty Interrupt
#define INT_SEP_AUTH_ERROR		101	// 0x65: SEP Authentication Error Interrupt
#define INT_SEP_CRIT_PATH_MONITOR	102	// 0x66: SEP Critical Path Monitor Interrupt
#define INT_DISP0			103	// 0x67: Display 0 Interrupt
#define INT_DISP0_PARTIAL_FRAME		104	// 0x68: Display 0 Partial Frame Interrupt
#define INT_DISP0_PERF_CNTRS		105	// 0x69: Display 0 Performace Counters Interrupt
#define INT_DISP0_DART			106	// 0x6a: Display 0 DART Interrupt
#define INT_DISP0_SMMU			107	// 0x6b: Display 0 SMMU Interrupt
#define INT_DISP0_BACK_END_BLU		108	// 0x6c: Display 0 Back End Back Light Update Interrupt
#define INT_DISP0_BACK_END_VBI		109	// 0x6d: Display 0 Back End VBI Interrupt
#define INT_DISP0_BACK_END_VFTG		110	// 0x6e: Display 0 Back End VFTG Idle Interrupt
#define INT_DISP0_BACK_END_VBL		111	// 0x6f: Display 0 Back End VBL counter Interrupt
#if SUB_PLATFORM_T7000
#define INT_MIPIDSI0			112	// 0x70: MIPI DSI 0 Interrupt
#define INT_MIPIDSI1			113	// 0x71: MIPI DSI 1 Interrupt
#define INT_DISPLAY_PORT		114	// 0x72: Display Port
#define INT_EDP0			INT_DISPLAY_PORT
#define	INT_DPORT0			INT_DISPLAY_PORT
#else
#define INT_DISPLAY_PORT		112	// 0x70: Display Port
#define INT_EDP0			INT_DISPLAY_PORT
#define	INT_DPORT0			INT_DISPLAY_PORT
#endif
#define INT_DISP1			115	// 0x73: Display 1 Interrupt
#define INT_DISP1_PARTIAL_FRAME		116	// 0x74: Display 1 Partial Frame Interrupt
#define INT_DISP1_PERF_CNTRS		117	// 0x75: Display 1 Performace Counters Interrupt
#define INT_DISP1_DART			118	// 0x76: Display 1 DART Interrupt
#define INT_DISP1_SMMU			119	// 0x77: Display 1 SMMU Interrupt
#define INT_VDEC0			120	// 0x78: Video Decoder 0 Interrupt

// Reserved 121 - 122

#define INT_AVE_KF_INBOX_EMPTY		123	// 0x7b: AVE KF Inbox Empty Interrupt
#define INT_AVE_KF_INBOX_NOTEMPTY	124	// 0x7c: AVE KF Inbox Not Empty Interrupt
#define INT_AVE_KF_OUTBOX_EMPTY		125	// 0x7d: AVE KF Outbox Empty Interrupt
#define INT_AVE_KF_OUTBOX_NOTEMPTY	126	// 0x7e: AVE KF Outbox Not Empty Interrupt
#define INT_AVE_DART			127	// 0x7f: AVE DART Interrupt
#define INT_AVE_SMMU			128	// 0x80: AVE SMMU Interrupt
#define INT_AVE				129	// 0x81: AVE Interrupt
#define INT_SVE_SYNC			130	// 0x82: SVE Driver Synchronization Interrupt

// Reserved 131 - 133

#define INT_GFX				134	// 0x86: GFX Interrupt
#define INT_GFX_SLEEP			135	// 0x87: GFX Sleep Interrupt
#define INT_GFX_WAKE			136	// 0x88: GFX Wake Interrupt
#define INT_GFX_CHG			137	// 0x89: GFX Change Interrupt

// Reserved 138

#define INT_JPEG			139	// 0x8b: JPEG Interrupt
#define INT_JPEG_DART			140	// 0x8c: JPEG DART Interrupt
#define INT_JPEG_DART_PERF		141	// 0x8d: JPEG DART Performance Interrupt

// Reserved 142

#define INT_MSR				143	// 0x8f: MSR Interrupt
#define INT_MSR_SMMU			144	// 0x90: MSR SMMU Interrupt
#define INT_MSR_DART			145	// 0x91: MSR DART Interrupt
#define INT_MSR_DAR_PERF		146	// 0x92: MSR DART Performance Interrupt

// Reserved 147

#define INT_SIO_KF_INBOX_EMPTY		148	// 0x94: SIO KF Inbox Empty Interrupt
#define INT_SIO_KF_INBOX_NOTEMPTY	149	// 0x95: SIO KF Inbox Not Empty Interrupt
#define INT_SIO_KF_OUTBOX_EMPTY		150	// 0x96: SIO KF Outbox Empty Interrupt
#define INT_SIO_KF_OUTBOX_NOTEMPTY	151	// 0x97: SIO KF Outbox Not Empty Interrupt
#define INT_SPI0			152	// 0x98: SPI 0 Interrupt
#define INT_SPI1			153	// 0x99: SPI 1 Interrupt
#define INT_SPI2			154	// 0x9a: SPI 2 Interrupt
#define INT_SPI3			155	// 0x9b: SPI 3 Interrupt

// Reserved 156 - 157

#define INT_UART0			158	// 0x9e: UART 0 Interrupt
#define INT_UART1			159	// 0x9f: UART 1 Interrupt
#define INT_UART2			160	// 0xa0: UART 2 Interrupt
#define INT_UART3			161	// 0xa1: UART 3 Interrupt
#define INT_UART4			162	// 0xa2: UART 4 Interrupt
#define INT_UART5			163	// 0xa3: UART 5 Interrupt
#define INT_UART6			164	// 0xa4: UART 6 Interrupt
#define INT_UART7			165	// 0xa5: UART 7 Interrupt
#define INT_UART8			166	// 0xa6: UART 8 Interrupt

// Reserved 167 - 168

#define INT_MCA0			169	// 0xa9: Multi-Channel Audio 0 Interrupt
#define INT_MCA1			170	// 0xab: Multi-Channel Audio 1 Interrupt
#define INT_MCA2			171	// 0xac: Multi-Channel Audio 2 Interrupt
#define INT_MCA3			172	// 0xad: Multi-Channel Audio 3 Interrupt
#define INT_MCA4			173	// 0xae: Multi-Channel Audio 4 Interrupt
#define INT_IIC0			174	// 0xaf: I2C 0 Interrupt
#define INT_IIC1			175	// 0xb0: I2C 1 Interrupt
#define INT_IIC2			176	// 0xb1: I2C 2 Interrupt
#define INT_IIC3			177	// 0xb2: I2C 3 Interrupt
#define INT_PWM				178	// 0xb3: PWM Interrupt
#define INT_AES0			179	// 0xb4: AES 0 Interrupt

// Reserved 180 - 181

#define INT_USB_OTG			182	// 0xb6: USB OTG Interrupt
#define INT_USB2HOST0_EHCI		183	// 0xb7: USB EHCI 0 Interrupt
#define INT_USB2HOST0_OHCI		184	// 0xb8: USB OHCI 0 Interrupt
#define INT_USB2HOST1_EHCI		185	// 0xb9: USB EHCI 1 Interrupt
#define INT_USB2HOST2_EHCI		186	// 0xba: USB EHCI 2 Interrupt
#define INT_USBOTG_RESUME		187	// 0xbb: USB OTG Resume Interrupt
#define INT_USB_20_BATT_CHRG_DET	188	// 0xbc: USB 2.0 Battery Charge Detect Interrupt
#define INT_USB2HOST0_PHY_RESUME	189	// 0xbd: USB Host 0 Resume Interrupt
#define INT_USB2HOST1_PHY_RESUME	190	// 0xbe: USB Host 1 Resume Interrupt
#define INT_USB2HOST2_PHY_RESUME	191	// 0xbf: USB Host 2 Resume Interrupt
#define INT_OTG_IN_EP0			192	// 0xc0: OTG IN Endpoint 0 Interrupt
#define INT_OTG_IN_EP1			193	// 0xc1: OTG IN Endpoint 1 Interrupt
#define INT_OTG_IN_EP2			194	// 0xc2: OTG IN Endpoint 2 Interrupt
#define INT_OTG_IN_EP3			195	// 0xc3: OTG IN Endpoint 3 Interrupt
#define INT_OTG_IN_EP4			196	// 0xc4: OTG IN Endpoint 4 Interrupt
#define INT_OTG_IN_EP5			197	// 0xc5: OTG IN Endpoint 5 Interrupt
#define INT_OTG_IN_EP6			198	// 0xc6: OTG IN Endpoint 6 Interrupt
#define INT_OTG_IN_EP7			199	// 0xc7: OTG IN Endpoint 7 Interrupt
#define INT_OTG_IN_EP8			200	// 0xc8: OTG IN Endpoint 8 Interrupt
#define INT_OTG_OUT_EP0			201	// 0xc9: OTG OUT Endpoint 0 Interrupt
#define INT_OTG_OUT_EP1			202	// 0xca: OTG OUT Endpoint 1 Interrupt
#define INT_OTG_OUT_EP2			203	// 0xcb: OTG OUT Endpoint 2 Interrupt
#define INT_OTG_OUT_EP3			204	// 0xcc: OTG OUT Endpoint 3 Interrupt
#define INT_OTG_OUT_EP4			205	// 0xcd: OTG OUT Endpoint 4 Interrupt
#define INT_OTG_OUT_EP5			206	// 0xce: OTG OUT Endpoint 5 Interrupt
#define INT_OTG_OUT_EP6			207	// 0xcf: OTG OUT Endpoint 6 Interrupt
#define INT_OTG_OUT_EP7			208	// 0xd0: OTG OUT Endpoint 7 Interrupt
#define INT_OTG_OUT_EP8			209	// 0xd1: OTG OUT Endpoint 8 Interrupt

// Reserved 210 - 211

#define PCIE_LINK0			212	// 0xd4: PCIE Link 0 Interrupt
#define PCIE_LINK0_DART			213	// 0xd5: PCIE Link 0 DART Interrupt
#define PCIE_LINK0_MMU			214	// 0xd6: PCIE Link 0 MMU Interrupt
#define PCIE_LINK1			215	// 0xd7: PCIE Link 1 Interrupt
#define PCIE_LINK1_DART			216	// 0xd8: PCIE Link 1 DART Interrupt
#define PCIE_LINK1_MMU			217	// 0xd9: PCIE Link 1 MMU Interrupt
#if SUB_PLATFORM_T7001
#define PCIE_LINK2			218	// 0xda: PCIE Link 1 Interrupt
#define PCIE_LINK2_DART			219	// 0xdb: PCIE Link 1 DART Interrupt
#define PCIE_LINK2_MMU			220	// 0xdc: PCIE Link 1 MMU Interrupt
#define PCIE_LINK3			221	// 0xdd: PCIE Link 1 Interrupt
#define PCIE_LINK3_DART			222	// 0xde: PCIE Link 1 DART Interrupt
#define PCIE_LINK3_MMU			223	// 0xdf: PCIE Link 1 MMU Interrupt
#endif

// Reserved 218 - 223 (T7000)

#define PCIE_MSI0			224	// 0xe0: PCIE MSI 0 Interrupt
#define PCIE_MSI1			225	// 0xe1: PCIE MSI 1 Interrupt
#define PCIE_MSI2			226	// 0xe2: PCIE MSI 2 Interrupt
#define PCIE_MSI3			227	// 0xe3: PCIE MSI 3 Interrupt
#define PCIE_MSI4			228	// 0xe4: PCIE MSI 4 Interrupt
#define PCIE_MSI5			229	// 0xe5: PCIE MSI 5 Interrupt
#define PCIE_MSI6			230	// 0xe6: PCIE MSI 6 Interrupt
#define PCIE_MSI7			231	// 0xe7: PCIE MSI 7 Interrupt
#define PCIE_MSI8			232	// 0xe8: PCIE MSI 8 Interrupt
#define PCIE_MSI9			233	// 0xe9: PCIE MSI 9 Interrupt
#define PCIE_MSI10			234	// 0xea: PCIE MSI 10 Interrupt
#define PCIE_MSI11			235	// 0xeb: PCIE MSI 11 Interrupt
#define PCIE_MSI12			236	// 0xec: PCIE MSI 12 Interrupt
#define PCIE_MSI13			237	// 0xed: PCIE MSI 13 Interrupt
#define PCIE_MSI14			238	// 0xee: PCIE MSI 14 Interrupt
#define PCIE_MSI15			239	// 0xef: PCIE MSI 15 Interrupt
#define PCIE_MSI16			240	// 0xf0: PCIE MSI 16 Interrupt
#define PCIE_MSI17			241	// 0xf1: PCIE MSI 17 Interrupt
#define PCIE_MSI18			242	// 0xf2: PCIE MSI 18 Interrupt
#define PCIE_MSI19			243	// 0xf3: PCIE MSI 19 Interrupt
#define PCIE_MSI20			244	// 0xf4: PCIE MSI 20 Interrupt
#define PCIE_MSI21			245	// 0xf5: PCIE MSI 21 Interrupt
#define PCIE_MSI22			246	// 0xf6: PCIE MSI 22 Interrupt
#define PCIE_MSI23			247	// 0xf7: PCIE MSI 23 Interrupt
#define PCIE_MSI24			248	// 0xf8: PCIE MSI 24 Interrupt
#define PCIE_MSI25			249	// 0xf9: PCIE MSI 25 Interrupt
#define PCIE_MSI26			250	// 0xfa: PCIE MSI 26 Interrupt
#define PCIE_MSI27			251	// 0xfb: PCIE MSI 27 Interrupt
#define PCIE_MSI28			252	// 0xfc: PCIE MSI 28 Interrupt
#define PCIE_MSI29			253	// 0xfd: PCIE MSI 29 Interrupt
#define PCIE_MSI30			254	// 0xfe: PCIE MSI 30 Interrupt
#define PCIE_MSI31			255	// 0xff: PCIE MSI 31 Interrupt

#endif /* ! __PLATFORM_SOC_HWISR_H */
