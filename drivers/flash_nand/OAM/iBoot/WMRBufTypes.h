#ifndef __WMR_BUFTYPES_H__
#define __WMR_BUFTYPES_H__

#include <WMRTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Types
typedef enum {
    BZS_INIT,
    BZS_ALLOC,
    BZS_FINISHED,
    BZS_FREE,
} WMR_BZS_State_e;

typedef struct {
    UInt8* baseAddr;
    UInt8* maxAddr;

    UInt32 totalAlloc;
    UInt32 numAllocs;
    UInt32 numRebases;
    UInt32 waste;

    WMR_BZS_State_e state;
} WMR_BufZone_t;


typedef struct {
} WMR_BufZoneList_t;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __WMR_BUFTYPES_H__


