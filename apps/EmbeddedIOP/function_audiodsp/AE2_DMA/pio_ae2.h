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

typedef enum {
	kPIOReceive = 0,
	kPIOTransmit = 1,
	kNumPIOs = 2,
} ePIOType;

enum { kBurstSize = 4 };

typedef struct
{
	ePIOType mWhichDevice;
	int_handler mInterruptHandler;
	void * mInterruptHandlerData;
	DMALinkedListItem mCurrentDMAItem;
	uint32_t mErrorCount[kAudioDevice_Last];
} internal_pio_object_t;

void handleAudioDeviceInterrupt();

/*
 setup pio.  Set the DMA linked list for doing PIO operations.
 preps the pio_object for doing it's first pio op
 */
void setupPIO(internal_pio_object_t * pio_object, void *src, void *dst, DMALinkedListItem *chain, size_t bytesToTransfer);

/*
 Startup PIO
 */
void startPIO(internal_pio_object_t * pio_object);

/*
 Stop PIO
 */
void stopPIO(internal_pio_object_t * pio_object);

#endif /* __PL080DMAC_AE2__ */

