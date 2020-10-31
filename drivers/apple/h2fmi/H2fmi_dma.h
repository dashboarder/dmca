// *****************************************************************************
//
// File: H2fmi_dma.h
//
// *****************************************************************************
//
// Notes:
//
//
// *****************************************************************************
//
// Copyright (C) 2008 Apple Computer, Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Computer, Inc.
//
// *****************************************************************************
#include "H2fmi_private.h"
#include "H2fmi_dma_types.h"

#ifndef _H2FMI_DMA_H_
#define _H2FMI_DMA_H_

BOOL32 h2fmi_dma_execute_cmd(UInt32 cmd, int dma_channel,
			   void *src, void *dst, UInt32 length,
			   UInt32 word_size, UInt32 burst_size,
               struct dma_aes_config *config);


BOOL32 h2fmi_dma_execute_async(UInt32 cmd, Int32 dma_channel,
             struct dma_segment *sgl, void *fifo, UInt32 length, 
             UInt32 word_size, UInt32 burst_size,
             struct dma_aes_config *config);

BOOL32 h2fmi_dma_wait(int dma_channel, UInt32 timeoutUs);

void h2fmi_dma_cancel(int dma_channel);

#endif //_H2FMI_DMA_H_

// ********************************** EOF **************************************
