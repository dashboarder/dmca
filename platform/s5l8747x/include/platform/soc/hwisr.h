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

#define EINT_G6			0	// External Interrupt 6
#define EINT_G5			1	// External Interrupt 5		
#define EINT_G4			2	// External Interrupt 4 	
#define EINT_G3			3	// External Interrupt 3	 	
#define INT_SWI			4	// SW Interrupt
#define INT_CMMSTX		5	// COMMS TX Interrupt	
#define INT_CMMSRX		6	// COMMS RX Interrupt
#define INT_TIMERFIQ		7	// TIMER FIQ Interrupt
#define INT_TIMERIRQ		8	// TIMER IRQ Interrupt 
#define INT_SPI0		9	// SPI0 Interrupt
#define INT_SPI1		10	// SPI1 Interrupt
#define INT_RESERVED11		11	// Reserved
#define INT_VP			12	// VP Interrupt
#define INT_RGBOUT		13	// RGBOUT Interrupt
#define INT_RESERVED14		14	// Reserved
#define INT_RESERVED15		15	// Reserved
#define INT_DMA0		16	// DMA0 Interrupt 
#define INT_DMA1		17	// DMA1 Interrupt
#define INT_USB_OTG0		18	// USB OTG0 Interrupt
#define INT_USB_OTG1		19	// USB OTG1 Interrupt
#define INT_RESERVED20		20	// Reserved
#define INT_IIC0		21	// IIC0 Interrupt
#define INT_IIC1		22	// IIC1 Interrupt
#define INT_IIC2		23	// IIC2 Interrupt
#define INT_UART0 		24	// UART0 Interrupt
#define INT_UART1 		25	// UART1 Interrupt
#define INT_UART2		26	// UART2 Interrupt
#define INT_RESERVED27		27	// Reserved
#define INT_RESERVED28		28	// Reserved
#define INT_RESERVED29		29	// Reserved
#define INT_TVOUT		30	// TVOUT Interrupt
#define EINT_G2			31	// External Interrupt 2
#define EINT_G1			32	// External Interrupt 1
#define EINT_G0			33	// External Interrupt 0		
#define INT_RESERED34		34	// Reserved
#define INT_MFC_DEC		35	// MFC DEC Interrupt	 	
#define INT_PKE			36	// PKE Interrupt
#define INT_SCALER		37	// SCALER Interrupt
#define INT_MIXER		38	// MIXER Interrupt	
#define INT_AES			39	// AES Interrupt
#define INT_SHA1		40	// SHA1 Interrupt 
#define INT_PRNG		41	// PRNG Interrupt
#define INT_SHA2		42	// SHA2 Interrupt
#define INT_RESERVED43		43	// Reserved
#define INT_ADC			44	// ADC Interrupt
#define INT_RESERVED45		45	// Reserved
#define INT_I2S0		46	// I2S Interrupt
#define INT_RESERVED47		47	// Reserved
#define INT_RESERVED48		48	// Reserved 
#define INT_RESERVED49		49	// Reserved
#define INT_SUSPEND		50	// SUSPEND Interrupt
#define INT_WDT			51	// WDT Interrupt
#define INT_SPDIF		52	// SPDIF Interrupt
#define INT_RESERVED53		53	// Reserved
#define INT_RESERVED54		54	// Reserved
#define INT_RESERVED55		55	// Reserved
#define INT_RESERVED56 		56	// Reserved
#define INT_RESERVED57 		57	// Reserved
#define INT_RESERVED58		58	// Reserved
#define INT_HDMI_CEC		59	// HDMI CEC Interrupt
#define INT_HDMI		60	// HDMI Interrupt
#define INT_CLKRSTGEN		61	// CLKRSTGEN Interrupt
#define INT_PMU			62	// PMU Interrupt
#define INT_ARF			63	// ARF Interrupt

#endif /* ! __PLATFORM_SOC_HWISR_H */
