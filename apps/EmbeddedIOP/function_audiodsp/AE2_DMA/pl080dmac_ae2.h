/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __PL080DMAC_AE2__
#define __PL080DMAC_AE2__

#include <stdint.h>
#include <platform/int.h>
#include "ae2_dma.h"

#define kDMACClockGating  0x341A0004

#define kDMAChannelsPerController 8

#define kDMACBase         0x341A3000
#define kDMAC0Base        0x341A3100

#define kDMACIntStatus    0x0
#define kDMACIntTCStatus  0x4
#define kDMACIntTCClear   0x8
#define kDMACIntErrStatus 0xC
#define kDMACIntErrClear  0x10

#define kDMACConfiguration 0x30

#define kDMACSourceOffset 0x0
#define kDMACDestOffset   0x4
#define kDMACLLIOffset    0x8
#define kDMACCtrlOffset   0xC
#define kDMACConfigOffset 0x10
#define kDMACRegisterSize 0x20

#define DMACConfiguration_M2_Mask   (1 << 2)
#define DMACConfiguration_M2_Little (0 << 2)
#define DMACConfiguration_M2_Big    (1 << 2)
#define DMACConfiguration_M1_Mask   (1 << 1)
#define DMACConfiguration_M1_Little (0 << 1)
#define DMACConfiguration_M1_Big    (1 << 1)
#define DMACConfiguration_E_Mask    (1 << 0)
#define DMACConfiguration_E_Disable (0 << 0)
#define DMACConfiguration_E_Enable  (1 << 0)

#define DMACCxConfiguration_Halt	   (1 << 18)
#define DMACCxConfiguration_Active     (1 << 17)
#define DMACCxConfiguration_ITC_Enable (1 << 15)
#define DMACCxConfiguration_IError_Enable (1 << 14)
#define DMACCxConfiguration_E_Enable   (1 <<  0)

#define DMACCxControl_I_Mask           (1 << 31)
#define DMACCxControl_I_Disable        (0 << 31)
#define DMACCxControl_I_Enable         (1 << 31)
#define DMACCxControl_SI_Mask          (1 << 26)
#define DMACCxControl_SI_No_Increment  (0 << 26)
#define DMACCxControl_SI_Increment     (1 << 26)
#define DMACCxControl_DI_Mask          (1 << 27)
#define DMACCxControl_DI_No_Increment  (0 << 27)
#define DMACCxControl_DI_Increment     (1 << 27)
#define DMACCxControl_D_Master1        (0 << 25)
#define DMACCxControl_S_Master1        (0 << 24)
#define DMACCxControl_SWidth_Halfword  (1 << 18)
#define DMACCxControl_DWidth_Halfword  (1 << 21)
#define DMACCxControl_DBSize_1         (0 << 15)
#define DMACCxControl_DBSize_4         (1 << 15)
#define DMACCxControl_SBSize_1         (0 << 12)
#define DMACCxControl_SBSize_4         (1 << 12)
#define DMACCxControl_TransferSizeMask (0x0FFF)

#define DMACCxConfiguration_FlowCntrl_Memory_to_Memory_DMA     (0 << 11)
#define DMACCxConfiguration_FlowCntrl_Memory_to_Peripheral_DMA (1 << 11)
#define DMACCxConfiguration_FlowCntrl_Peripheral_to_Memory_DMA (2 << 11)


void handleAudioDeviceDMAInterrupt();

void handleAudioDeviceDMAInterruptError();

/*
 Get a free DMA channel
*/
uint32_t acquireDMAChannel(void);

/*
 Free a DMA channel
*/
void freeDMAChannel(uint32_t dmaChannel);

/*
 Make sure the pl080 clock is enabled and initialize DMA channels 0 and 1
 */
void configureDMA(uint32_t dmaChannel);

/*
 Startup DMA from address src to destination dst
 If chain is not NULL, will setup that in the LLI to auto-restart DMA from the chain
 */
void setupDMA(uint32_t dmaChannel, void *src, void *dst, DMALinkedListItem *chain, DMADirection direction, uint32_t peripheral, size_t bytesToTransfer);

/*
 Startup DMA from address src to destination dst
 If chain is not NULL, will setup that in the LLI to auto-restart DMA from the chain
 */
void startDMA(uint32_t dmaChannel);

/*
 Halts the channel, then blocks until the DMA is complete and then fully disables it
 */
void disableDMA(uint32_t dmaChannel);

/*
 Blocks until the DMA is complete and then fully disables it
 */
void disableDMAImmediate(uint32_t dmaChannel);

#endif /* __PL080DMAC_AE2__ */

