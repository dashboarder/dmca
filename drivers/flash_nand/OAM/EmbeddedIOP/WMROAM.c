#include "WMRConfig.h"
#include "ANDTypes.h"
#include "WMROAM.h"

#include <stdio.h>
#include <lib/heap.h>
#include <sys.h>
#include <sys/task.h>
#include <platform.h>
#include <platform/soc/hwclocks.h>
#include <platform/timer.h>

void *
WMR_MALLOC(UInt32 nSize)
{
    void    *p;

    p = memalign(nSize, CPU_CACHELINE_SIZE);
    WMR_MEMSET(p, 0, nSize);
    return p;
}

BOOL32
OAM_Init(void)
{   
    return TRUE32;
}

void OAM_Beep(UInt32 dwMs)
{
}

void    WMR_FREE(void * ptr, UInt32 size)
{
    free(ptr);
}

BOOL32  WMR_CHECK_BUFFER(const void * buffer, UInt32 length)
{
    return (((UInt32) buffer % CPU_CACHELINE_SIZE) == 0) ? TRUE32 : FALSE32;
}

BOOL32
WMR_FILL_STRUCT(void *pDstBuff, UInt32 *pdwDstBuffSize, const void *pSrcBuff, UInt32 dwSrcBuffSize)
{
    BOOL32 boolRes = TRUE32;
    
    if (!pDstBuff && pdwDstBuffSize)
    {
        // Just return the buffer size
        *pdwDstBuffSize = dwSrcBuffSize;
    }
    else if (!pdwDstBuffSize || (*pdwDstBuffSize < dwSrcBuffSize))
    {
        // Supplied buffer is too small
        UInt32 dwPrintedSize = pdwDstBuffSize ? *pdwDstBuffSize : (UInt32) ~0;
        WMR_PRINT(ERROR,"IOCtl on buffer of size %d with %d bytes of src data!\n", 
                       dwPrintedSize, dwSrcBuffSize);
        boolRes = FALSE32;
    }
    else
    {   
        // We have a valid buffer pointer and buffer size
        WMR_MEMCPY(pDstBuff, pSrcBuff, dwSrcBuffSize);
        *pdwDstBuffSize = dwSrcBuffSize;
    }

    return boolRes;
}

void   _WMR_PRINT(const char * format, ...)
{
    va_list args;

    // retrieve the variable arguments
    va_start(args, format);

    vprintf(format, args);

    va_end(args);
}

void
WMR_BEEP(UInt32 milliseconds)
{
    printf("WMR_BEEP\n");
}

const char * WMR_GETVERSION(void)
{
    static char version[] = "version string";

    return version;
}

BOOL32
WMR_I_CAN_HAZ_DEBUGGER(void)                                                                                                 
{
    return FALSE32;
}

void WMR_WAIT_US(UInt64 microseconds)
{
    spin(microseconds);
}

void WMR_SLEEP_US(UInt32 microseconds)
{
    task_sleep(microseconds);
}

void WMR_YIELD(void)
{
    // Task yield is not available to libraries, so can't build it into Whimory
    task_yield();
}

void
WMR_DEADLOOP(void)
{
    volatile int i = 1;
    while (i)
    {
	(void) 0;
    }
}

void
WMR_CLOCK_RESET_DEVICE(UInt32 device)
{
    int    clock;

    if( device == 0 )
    {
        clock = CLK_FMI0;
    }
    else if( device == 1 )
    {
        clock = CLK_FMI1;
    }
    else
    {
        panic("invalid WMR_CLOCK_RESET_DEVICE");
    }

    clock_reset_device(clock);
}
