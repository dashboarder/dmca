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

#ifndef __AE2_DMA_H__
#define __AE2_DMA_H__


typedef void * dma_object_t;

/*
 * dma_object_t is an object that can be used to control a dma
 */

typedef enum {
	kDirectionNone = 0,
	kDirectionIn   = 1,
	kDirectionOut  = 2
} DMADirection;

typedef struct LLI
{
    uint32_t   source;
    uint32_t   destination;
    struct LLI *next;
    uint32_t   control;
} DMALinkedListItem;

typedef enum {
	kI2S_0 = 0,
	kI2S_1,
	kI2S_2,
	kI2S_3,
	kMCA_0,
	kMCA_1,
	kAudioDevice_Last
} AudioDevice_Index;

// create a dma object that will transfer to/from a buffer from/to a device, depending
// on the direction.
dma_object_t create_dma_object(void *buffer, AudioDevice_Index device, DMALinkedListItem *chain, DMADirection direction, size_t bytesToTransfer);
void destroy_dma_object(dma_object_t dma);

void setupInterruptHandler(dma_object_t dma, int_handler handler, void *arg);
void setupErrorHandler(dma_object_t dma, int_handler handler, void *arg);

typedef enum {
	kFrameError = 0,
	kRXOverrun,
	kRXUnderrun,
	kTXOverrun,
	kTXUnderrun,
	kError_last
} Error_Index;

static const char* const kErrorTypeStr[kError_last] = {
	"Frame_Error",
	"RX_Overrun",
	"RX_Underrun",
	"TX_Overrun",
	"TX_Underrun",
};

uint32_t getErrorCount(dma_object_t dma, Error_Index which);

void startDMAObject(dma_object_t dma);
void stopDMAObject(dma_object_t dma, bool immediate);

#endif /* __AE2_DMA_H__ */

