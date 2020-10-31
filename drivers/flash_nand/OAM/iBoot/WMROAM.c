#include "WMROAM.h"

#include <stdio.h>
#include <lib/heap.h>

#include <sys/task.h>
#include <platform.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwregbase.h>
#include <platform/timer.h>

UInt32 gWhimoryInitParameters = 
#if (defined(WITH_BLOCK_ZERO_SIGNATURE) && WITH_BLOCK_ZERO_SIGNATURE)
WMR_INIT_SIGNATURE_IN_BLOCKZERO |
#endif
0;

static UInt32 boot_buffer_size = DEFAULT_OAM_BOOT_BUFFER_SIZE;
static void * boot_buffer = NULL;

void *
WMR_MALLOC(UInt32 nSize)
{
    void    *p;

    p = memalign(nSize, CPU_CACHELINE_SIZE);
    WMR_MEMSET(p, 0, nSize);
    return p;
}

void*
WMR_BOOT_MALLOC(UInt32 *size)
{
    WMR_ASSERT(NULL != size);
    WMR_ASSERT(NULL == boot_buffer); // only one allocation allowed
    
    if (*size <= boot_buffer_size)
    {
        boot_buffer = malloc(boot_buffer_size);
        WMR_MEMSET(boot_buffer, 0, boot_buffer_size);
    }
    *size = boot_buffer_size;
    return boot_buffer;
}


void
WMR_BOOT_FREE(void * ptr)
{
    WMR_ASSERT(ptr == boot_buffer);
    free(ptr);
    boot_buffer = NULL;
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
#if (defined(WMR_DEBUG) && WMR_DEBUG)
    if (IS_ALIGNED_TO(buffer, CPU_CACHELINE_SIZE) &&
        IS_ALIGNED_TO(length, CPU_CACHELINE_SIZE))
    {
        return TRUE32;
    }
    else
    {
        return FALSE32;
    }
#else //WMR_DEBUG
    return TRUE32;
#endif // !WMR_DEBUG
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

UInt32 WMR_BUS_FREQ_HZ(void)
{
    return clock_get_frequency(CLK_FMI);
}

void  WMR_BUS_FREQ_MHZ(UInt32 * minClockMHz, UInt32 * maxClockMHz)
{
    const UInt32 clkSpeed = WMR_BUS_FREQ_HZ() / 1000000;
    
    if (minClockMHz)
    {
        *minClockMHz = clkSpeed;
    }
    
    if (maxClockMHz)
    {
        *maxClockMHz = clkSpeed;
    }
}

#if FMI_VERSION >= 5
void WMR_DLL_CLOCK_GATE(BOOL32 enable)
{
    clock_gate(CLK_FMI_DLL, enable);
}
#endif

void WMR_CLOCK_GATE(UInt32 device, BOOL32 enable)
{
    int    clock;

    switch (device)
    {
        case 0:
            clock = CLK_FMI0;
            break;
        case 1:
            clock = CLK_FMI1;
            break;
        default:
            panic("invalid WMR_CLOCK_GATE");
            break;
    }
    clock_gate(clock, enable);

#if FMI_VERSION > 1
    switch (device)
    {
        case 0:
            clock = CLK_FMI0BCH;
            break;
        case 1:
            clock = CLK_FMI1BCH;
            break;
        default:
            panic("invalid WMR_CLOCK_GATE");
            break;
    }
    clock_gate(clock, enable);
#endif //FMI_VERSION > 0
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
