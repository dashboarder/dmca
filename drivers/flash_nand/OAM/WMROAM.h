/////
//
// OS Adaptation Module API
//
/////
#include "ANDTypes.h"
#include "WMRFeatures.h"
#include "WMRPlatform.h"

#ifndef _WMR_OAM_H_
#define _WMR_OAM_H_

#define NUMELEMENTS(x)              (sizeof(x) / sizeof((x)[0]))
#define ROUNDDOWNTO(num, gran)      ((num) - ((num) % (gran)))
#define ROUNDUPTO(num, gran)        ((((num) + (gran) - 1) / (gran)) * (gran))
#define IS_ALIGNED_TO(addr, align)  (((UInt32)(addr)) % (align) == 0)
#define IS_ADDRESS_IN_RANGE(addr, start, end)  \
    (((addr) >= (start) && (addr) < (end)) ? TRUE32 : FALSE32)

#define WMR_MIN(x, y)                        (((x) < (y)) ? (x) : (y))
#define WMR_MAX(x, y)                        (((x) < (y)) ? (y) : (x))

#define WMR_CASSERT(x, name) typedef char __WMR_CASSERT_##name[(x) ? 1 : -1]
#define WMR_OFFSETOF(type, field) ((size_t)(&((type *)0)->field))

// Print levels
// Use only the last word in a WMR_PRINT call (e.g. ALWAYS for WMR_PRINT_ALWAYS)
#define WMR_PRINT_ALWAYS  (1UL << 0)

#define WMR_PRINT_INIT    (1UL << 1)
#define WMR_PRINT_IRQ     (1UL << 2)
#define WMR_PRINT_CLOCK   (1UL << 3)
#define WMR_PRINT_POWER   (1UL << 4)
#define WMR_PRINT_ALIGN   (1UL << 5)
#define WMR_PRINT_BUFFER  (1UL << 6)
#define WMR_PRINT_FACTORY (1UL << 7)
#define WMR_PRINT_QUAL    (1UL << 8)
 // WMR_PRINT_QUAL_FATAL allows simulator panics
#define WMR_PRINT_QUAL_FATAL (WMR_PRINT_QUAL | WMR_PRINT_ERROR)

#define WMR_PRINT_READ    (1UL << 10)
#define WMR_PRINT_WRITE   (1UL << 11)
#define WMR_PRINT_ERASE   (1UL << 12)
#define WMR_PRINT_UECC_PANIC (1UL << 13)

#define WMR_PRINT_FTL     (1UL << 20)
#define WMR_PRINT_VFL     (1UL << 21)
#define WMR_PRINT_FIL     (1UL << 22)
#define WMR_PRINT_FTLWARN (1UL << 23)
#define WMR_PRINT_VFLWARN (1UL << 24)
#define WMR_PRINT_EXAM    (1UL << 25)

#define WMR_PRINT_CRYPTO  (1UL << 27)

#define WMR_PRINT_INF     (1UL << 28)
#define WMR_PRINT_LOG     (1UL << 29)
#define WMR_PRINT_MISC    (1UL << 30)
#define WMR_PRINT_ERROR   (1UL << 31)

#define WMR_PRINT_LEVEL (WMR_PRINT_ALWAYS | WMR_PRINT_INIT | WMR_PRINT_ERROR | WMR_PRINT_FACTORY | WMR_PRINT_QUAL | WMR_PRINT_VFLWARN)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

////////////////////////////////////////////////////////////////////////////////
//
// OAM_Init
//
// Call *once* before any use of WMR or OAM functions
//
BOOL32    OAM_Init(void);

////////////////////////////////////////////////////////////////////////////////
//
// gWhimoryInitParameters
//
// Bitmask of initialization parameters to WMR_Init()
//
extern UInt32 gWhimoryInitParameters;
	
////////////////////////////////////////////////////////////////////////////////
//
// WMR_SCM_VERSION
//
// Get the SCM changelist of this build
//
UInt32  WMR_SCM_VERSION(void);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_BUILD_NUMBER
//
// Get the build number of this build
//
UInt32  WMR_BUILD_NUMBER(void);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_PRINT
//
// Use WMR_PRINT_XXX defines to describe your print type
//
#define TEXT(x) x

#if (defined (WMR_DEBUG) && WMR_DEBUG)
#define WMR_DBG_PRINT(x)    { _WMR_PRINT x; }
#define WMR_RTL_PRINT(x)    { _WMR_PRINT x; }
#else
#define WMR_DBG_PRINT(x)
#define WMR_RTL_PRINT(x)    { _WMR_PRINT x; }
#endif

#if (defined(CC_NO_VA_ARGS) && !CC_NO_VA_ARGS)
#define WMR_PRINT(type, format, ...)  \
    do { \
        if (WMR_PRINT_ ## type & WMR_PRINT_LEVEL) { \
            _WMR_PRINT("[NAND] %s:%d " format "", __FUNCTION__, __LINE__, __VA_ARGS__); } \
    } while (0)
#else //CC_NO_VA_ARGS
#define WMR_PRINT(type, format, args ...)  \
    do { \
        if (WMR_PRINT_ ## type & WMR_PRINT_LEVEL) { \
            _WMR_PRINT("[NAND] %s:%d " format "", __FUNCTION__, __LINE__, ## args); } \
    } while (0)
#endif //CC_NO_VA_ARGS

void   _WMR_PRINT(const char * format, ...);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_MALLOC
//
// Allocate a WMR internal buffer (cache aligned & padded)
//
void   *WMR_MALLOC(UInt32 size);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_FREE
//
// Release internal buffer
//
void    WMR_FREE(void * ptr, UInt32 size);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_BOOT_MALLOC
//
// Allocate up to one boot-time-only buffer that will be freed with 
// WMR_BOOT_FREE() before the NAND driver finishes opening
// 
// The size argument is required. The incoming integer value is the minimum 
// required size. The outgoing value is the actual size of the buffer.
//
void   *WMR_BOOT_MALLOC(UInt32 *size);
    
////////////////////////////////////////////////////////////////////////////////
//
// WMR_BOOT_FREE
//
// Free the buffer allocated by WMR_BOOT_MALLOC()
//
void    WMR_BOOT_FREE(void * ptr);
    
////////////////////////////////////////////////////////////////////////////////
//
// WMR_CHECK_BUFFER
//
// Check if a given buffer is same for I/O DMA
//
BOOL32  WMR_CHECK_BUFFER(const void * buffer, UInt32 length);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_FILL_STRUCT
//
// Safely fill an ioctl buffer. Returns false if dest buffer size is too small
// or unspecified
//
    BOOL32 WMR_FILL_STRUCT(void *pDstBuff, UInt32 *pdwDstBuffSize, const void *pSrcBuff, UInt32 dwSrcBuffSize);
    
////////////////////////////////////////////////////////////////////////////////
//
// WMR_GETVERSION
//
// gets version string (will be used for signature
//
const char * WMR_GETVERSION(void);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_I_CAN_HAZ_DEBUGGER
//
// indictes whether a unit is capable of being debugged
//
BOOL32 WMR_I_CAN_HAZ_DEBUGGER(void);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_BEEP
//
// Use a piezo sound (if available) to alert the user to an error
//
void    WMR_BEEP(UInt32 milliseconds);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_DEADLOOP
//
// Alert the user that the system has stopped and deadloop
//
//
void    WMR_DEADLOOP(void);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_CHKSUM
//
// Calculate a checksum on the given buffer
//
UInt32  WMR_CHKSUM(const void * buf, UInt32 length);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_WAIT_US
//
// Wait the given number of microseconds (may spin or deschedule)
//
void WMR_WAIT_US(UInt64 microseconds);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_SLEEP_US
//
// Sleep the given number of microseconds.  On threading/tasking systems, this
// relinquishes cpu control to the scheduler.  On simpler systems, this simply
// waits the specified amount of time.
//
void WMR_SLEEP_US(UInt32 microseconds);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_YIELD
//
// Yield cpu control to the scheduler on threading/tasking systems.  On simpler
// systems, this simply returns immediately.
//
void WMR_YIELD(void);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_BUS_FREQ_MHZ
//
// Return the minimum and maximum speeds that the system bus connected to the
// the FMC may operate during NAND accesses
//
void   WMR_BUS_FREQ_MHZ(UInt32 * minClockMHz, UInt32 * maxClockMHz);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_BUS_FREQ_HZ
//
// Return a precise version of the fastest operating frequency of the FMC
//
UInt32 WMR_BUS_FREQ_HZ(void);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_GET_SIGNATURE_MAJOR_VER
//
// Return signature byte indicating the major version
//
UInt8 WMR_GET_SIGNATURE_MAJOR_VER(void);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_GET_SIGNATURE_LAYOUT
//
// Return signature byte indicating the minor version
//
UInt8 WMR_GET_SIGNATURE_MINOR_VER(void);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_ACQUIRE_LOCK
//
// Acquire a lock allocated to the NAND driver
//
void WMR_ACQUIRE_LOCK(void);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_RELEASE_LOCK
//
// Releas a lock acquired through WMR_ACQUIRE_LOCK
//
void WMR_RELEASE_LOCK(void);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_CLOCK_GATE
//
// Gates/Ungates the clock for device
//
void WMR_CLOCK_GATE(UInt32 device, BOOL32 enable);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_DLL_CLOCK_GATE
//
// Gates/Ungates the clock for NAND DLL
//
void WMR_DLL_CLOCK_GATE(BOOL32 enable);

////////////////////////////////////////////////////////////////////////////////
//
// WMR_CLOCK_RESET_DEVICE
//
// Clock reset device
//
void WMR_CLOCK_RESET_DEVICE(UInt32 device);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WMR_OAM_H_ */

