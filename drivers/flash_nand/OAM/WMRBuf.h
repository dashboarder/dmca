#ifndef __WMR_BUF_H__
#define __WMR_BUF_H__

#include "WMRFeatures.h"
#include "WMRTypes.h"
#include "WMRBufTypes.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


// Variables
extern WMR_BufZoneList_t WMR_BufZoneList;


// Functions
extern void   WMR_Buf_Init(void); // module-level initialization

extern void  *WMR_Buf_Alloc(WMR_BufZone_t *zone, UInt32 size);
extern void  *WMR_Buf_Alloc_ForDMA(WMR_BufZone_t *zone, UInt32 size);

extern void   WMR_BufZone_Init(WMR_BufZone_t *zone);
extern BOOL32 WMR_BufZone_FinishedAllocs(WMR_BufZone_t *zone);
extern void   WMR_BufZone_Rebase(WMR_BufZone_t *zone, void** buf);
extern void   WMR_BufZone_FinishedRebases(WMR_BufZone_t *zone);
extern void   WMR_BufZone_Free(WMR_BufZone_t *zone);
extern UInt32 WMR_BufZone_FindDescriptor(UInt8* buf, void **des); // returns offset


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __WMR_BUF_H__
