/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#ifndef __SAMSUNG_SPI_H
#define __SAMSUNG_SPI_H

#include <platform/soc/hwregbase.h>

#define rSPCLKCON0			(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x00))
#define rSPCON0				(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x04))
#define rSPSTA0				(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x08))
#define rSPPIN0				(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x0C))
#define rSPTDAT0			(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x10))
#define rSPRDAT0			(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x20))
#define rSPPRE0				(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x30))
#define rSPCNT0				(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x34))
#define rSPIDD0				(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x38))
#define rSPIRTO0			(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x3C))
#define rSPIHANGD0			(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x40))
#define rSPISWRST0			(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x44))
#define rSPIVER0			(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x48))
#define rSPTDCNT0			(*(volatile u_int32_t *)(SPI0_BASE_ADDR + 0x4C))

#define rSPCLKCON1			(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x00))
#define rSPCON1				(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x04))
#define rSPSTA1				(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x08))
#define rSPPIN1				(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x0C))
#define rSPTDAT1			(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x10))
#define rSPRDAT1			(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x20))
#define rSPPRE1				(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x30))
#define rSPCNT1				(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x34))
#define rSPIDD1				(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x38))
#define rSPIRTO1			(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x3C))
#define rSPIHANGD1			(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x40))
#define rSPISWRST1			(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x44))
#define rSPIVER1			(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x48))
#define rSPTDCNT1			(*(volatile u_int32_t *)(SPI1_BASE_ADDR + 0x4C))

#define rSPCLKCON2			(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x00))
#define rSPCON2				(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x04))
#define rSPSTA2				(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x08))
#define rSPPIN2				(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x0C))
#define rSPTDAT2			(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x10))
#define rSPRDAT2			(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x20))
#define rSPPRE2				(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x30))
#define rSPCNT2				(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x34))
#define rSPIDD2				(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x38))
#define rSPIRTO2			(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x3C))
#define rSPIHANGD2			(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x40))
#define rSPISWRST2			(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x44))
#define rSPIVER2			(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x48))
#define rSPTDCNT2			(*(volatile u_int32_t *)(SPI2_BASE_ADDR + 0x4C))

#define rSPCLKCON3			(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x00))
#define rSPCON3				(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x04))
#define rSPSTA3				(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x08))
#define rSPPIN3				(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x0C))
#define rSPTDAT3			(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x10))
#define rSPRDAT3			(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x20))
#define rSPPRE3				(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x30))
#define rSPCNT3				(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x34))
#define rSPIDD3				(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x38))
#define rSPIRTO3			(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x3C))
#define rSPIHANGD3			(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x40))
#define rSPISWRST3			(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x44))
#define rSPIVER3			(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x48))
#define rSPTDCNT3			(*(volatile u_int32_t *)(SPI3_BASE_ADDR + 0x4C))

#define rSPCLKCON4			(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x00))
#define rSPCON4				(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x04))
#define rSPSTA4				(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x08))
#define rSPPIN4				(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x0C))
#define rSPTDAT4			(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x10))
#define rSPRDAT4			(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x20))
#define rSPPRE4				(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x30))
#define rSPCNT4				(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x34))
#define rSPIDD4				(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x38))
#define rSPIRTO4			(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x3C))
#define rSPIHANGD4			(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x40))
#define rSPISWRST4			(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x44))
#define rSPIVER4			(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x48))
#define rSPTDCNT4			(*(volatile u_int32_t *)(SPI4_BASE_ADDR + 0x4C))

#if SPI_VERSION == 0
#define SPICON_SHIFT_OFFSET0		(0)
#define SPICON_SHIFT_OFFSET1		(0)
#define SPICON_SHIFT_OFFSET2		(0)
#else
#define SPICON_SHIFT_OFFSET0		(2)
#define SPICON_SHIFT_OFFSET1		(3)
#define SPICON_SHIFT_OFFSET2		(-1)
#endif

#define SPICON_AGD_SHIFT		(0)
#define SPICON_CPHA_SHIFT		(1)
#define SPICON_CPOL_SHIFT		(2)
#define SPICON_MASTER_SHIFT		(3)
#define SPICON_CLK_EN_SHIFT		(4)
#define SPICON_MODE_SHIFT		(5)
#define SPICON_IE_RX_SHIFT		(7)
#define SPICON_IE_TR_SHIFT		(8)
#define SPICON_IE_MME_SHIFT		(9)
#define SPICON_IE_DCE_SHIFT		(10)
#define SPICON_MSBFT_SHIFT		(11 + SPICON_SHIFT_OFFSET0)
#define SPICON_CLK_SEL_SHIFT		(12 + SPICON_SHIFT_OFFSET0)
#define SPICON_BIT_LEN_SHIFT		(13 + SPICON_SHIFT_OFFSET0)
#define SPICON_DMA_SIZE_SHIFT		(15 + SPICON_SHIFT_OFFSET0)
#define SPICON_DATA_SWAP_SHIFT		(16 + SPICON_SHIFT_OFFSET1)
#define SPICON_RX_DELAY_SHIFT		(17 + SPICON_SHIFT_OFFSET1)
#define SPICON_IE_TX_SHIFT		(21)
#define SPICON_DELAY_SEL_SHIFT		(27 + SPICON_SHIFT_OFFSET2)


#if SPI_VERSION == 0
#define SPI_FIFO_BITS			(4)
#define SPSTA_TX_FIFO_BIT_LO		(4)
#define SPSTA_RX_FIFO_BIT_LO		(8)
#else
#define SPI_FIFO_BITS			(5)
#define SPSTA_TX_FIFO_BIT_LO		(6)
#define SPSTA_RX_FIFO_BIT_LO		(11)
#endif

#define SPSTA_TX_FIFO_BIT_HI		(SPSTA_TX_FIFO_BIT_LO + SPI_FIFO_BITS - 1)
#define SPSTA_RX_FIFO_BIT_HI		(SPSTA_RX_FIFO_BIT_LO + SPI_FIFO_BITS - 1)
#define SPI_FIFO_SIZE			(1 << (SPI_FIFO_BITS - 1))

#endif /* __SAMSUNG_SPI_H */
