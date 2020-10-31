// *****************************************************************************
//
// File: H2fmi_dma_iboot.c
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

#include "H2fmi_dma.h"
#include <drivers/dma.h>
#include <platform/timer.h>
#include <sys/task.h>
#include "WMROAM.h"

#define H2FMI_DMA_POLLING_ENABLED (1)

#if !H2FMI_DMA_POLLING_ENABLED

static struct task_event dma_events[DMA_CHANNEL_COUNT];
static BOOL32 first_time = TRUE32;


static void h2fmi_dma_done_handler(void *arg)
{
    UInt32 channel = (UInt32)arg;
    
    event_signal(&dma_events[channel]);
}

#endif // !H2FMI_DMA_POLLING_ENABLED


// no timeout -  rdar://6403538
BOOL32 h2fmi_dma_execute_cmd(UInt32 cmd, int dma_channel,
			   void *src, void *dst, UInt32 length,
			   UInt32 word_size, UInt32 burst_size,
               struct dma_aes_config *config)
{
    struct dma_segment sgl;
    BOOL32             ret;
        
    sgl.paddr  = (UInt32)src;
    sgl.length = length;
        
    ret = h2fmi_dma_execute_async(cmd,
                              dma_channel,
                              &sgl,
                              dst,
                              length,
                              word_size,
                              burst_size,
                              config);
    if( !ret )
    {
        return FALSE32;
    }

    h2fmi_dma_wait(dma_channel, 0);

    return TRUE32;    
}


BOOL32 h2fmi_dma_execute_async(UInt32 cmd, Int32 dma_channel,
             struct dma_segment *sgl, void *fifo, UInt32 length, 
             UInt32 word_size, UInt32 burst_size,
             struct dma_aes_config *config)
{
    int ret;

#if !H2FMI_DMA_POLLING_ENABLED

    if (first_time )
    {
        UInt32 i;
        first_time = FALSE32;
        
        for (i = 0; i < DMA_CHANNEL_COUNT; i++)
        {
            event_init(&dma_events[i], EVENT_FLAG_AUTO_UNSIGNAL, false);
        }
    }
    
    WMR_ASSERT(!dma_events[dma_channel].signalled);
    if (dma_events[dma_channel].signalled)
    {
        /* This dma channel already has a completion pending.  Clear it now so things
         * don't get out of sync, but if this happens it's because sometime in the past
         * someone called h2fmi_dma_execute_async without calling either h2fmi_dma_wait or h2fmi_dma_cancel.
         */
        WMR_PRINT(ERROR, "%s called when completion already pending.  Recovering, but this is a bug.", __FUNCTION__);
        event_unsignal(&dma_events[dma_channel]);
    }

#endif // !H2FMI_DMA_POLLING_ENABLED
    
    dma_set_aes(dma_channel, config);
    
    ret =  dma_execute_async(cmd,
                             dma_channel,
                             sgl,
                             fifo,
                             length,
                             word_size,
                             burst_size,
#if H2FMI_DMA_POLLING_ENABLED
                             NULL,
                             NULL);
#else // H2FMI_DMA_POLLING_ENABLED
                             h2fmi_dma_done_handler,
                             (void*)dma_channel);
#endif // !H2FMI_DMA_POLLING_ENABLED

    if (ret != 0)
    {
        WMR_PRINT(ERROR, "WMR_DMA_START_ASYNC_FAILED: direction=%d, channel=%d, length=%d\n",
                  cmd, dma_channel, length);
        return FALSE32;        
    }
    else
    {
        return TRUE32;
    }
}


BOOL32 h2fmi_dma_wait(int dma_channel, UInt32 timeoutUs)
{
    BOOL32  ret = TRUE32;

#if H2FMI_DMA_POLLING_ENABLED

    const UInt64 startUs = WMR_CLOCK_NATIVE();

    dma_use_int(dma_channel, FALSE32);

    while (!dma_poll(dma_channel))
    {
        if ((0 != timeoutUs) && WMR_HAS_TIME_ELAPSED_US(startUs, timeoutUs))
        {
            ret = dma_poll(dma_channel);
            break;
        }

        WMR_YIELD();
    }

    dma_use_int(dma_channel, TRUE32);

#else // H2FMI_DMA_POLLING_ENABLED

    if (timeoutUs )
    {
        ret = event_wait_timeout(&dma_events[dma_channel], timeoutUs);
    }
    else
    {
        // Wait forever if timeout is zero
        event_wait(&dma_events[dma_channel]);
        ret = TRUE32;
    }

#endif // !H2FMI_DMA_POLLING_ENABLED

    if (!ret)
    {
        WMR_PRINT(ERROR, "Timeout waiting for DMA: channel=%d\n", dma_channel);
        WMR_PANIC("dma timeout");
    }

    return ret;   
}

void h2fmi_dma_cancel(int dma_channel)
{
    dma_cancel(dma_channel);
#if !H2FMI_DMA_POLLING_ENABLED
    event_unsignal(&dma_events[dma_channel]);
#endif // !H2FMI_DMA_POLLING_ENABLED
}


// ********************************** EOF **************************************

