#include <WMRFeatures.h>
#include <WMROAM.h>
#include <WMRBuf.h>
#include <platform.h>


// Variable instantiations

WMR_BufZoneList_t WMR_BufZoneList;


// Functions

static void ZoneList_Link(WMR_BufZone_t *zone)
{
    // NOP
}


static void ZoneList_Unlink(WMR_BufZone_t *zone)
{
    // NOP
}


extern void WMR_Buf_Init()
{
    // NOP
}


void  *WMR_Buf_Alloc(WMR_BufZone_t *zone, UInt32 size)
{
    // Only need cache-line alignment, so pass it through to the other allocator
    return WMR_Buf_Alloc_ForDMA(zone, size);
}


void *WMR_Buf_Alloc_ForDMA(WMR_BufZone_t *zone, UInt32 size)
{
    void *ret;
    UInt32 oldTotal;

    WMR_ASSERT(BZS_ALLOC == zone->state);

    // Align alloc offset
    oldTotal = zone->totalAlloc;
    zone->totalAlloc = ROUNDUPTO(zone->totalAlloc, CPU_CACHELINE_SIZE);
    zone->waste += (zone->totalAlloc - oldTotal);

    // Set up offset pointer
    ret = (void*)zone->totalAlloc;

    // Move allocator along
    zone->totalAlloc += size;
    zone->numAllocs++;

    return ret;
}


void  WMR_BufZone_Init(WMR_BufZone_t *zone)
{
    zone->baseAddr = NULL;
    zone->maxAddr = NULL;
    zone->totalAlloc = 0;
    zone->numAllocs = 0;
    zone->numRebases = 0;
    zone->waste = 0;
    zone->state = BZS_ALLOC;

    ZoneList_Link(zone);
}


BOOL32 WMR_BufZone_FinishedAllocs(WMR_BufZone_t *zone)
{
    UInt8 *buf;

    WMR_ASSERT(BZS_ALLOC == zone->state);

    // Allocate buffer
    zone->totalAlloc = ROUNDUPTO(zone->totalAlloc, CPU_CACHELINE_SIZE);
    buf = WMR_MALLOC(zone->totalAlloc);
    if (NULL == buf)
    {
        return FALSE32;
    }

    // Update zone
    zone->baseAddr = buf;
    zone->maxAddr = zone->baseAddr + zone->totalAlloc;
    zone->state = BZS_FINISHED;

    return TRUE32;
}


void  WMR_BufZone_Rebase(WMR_BufZone_t *zone, void** buf)
{
    *buf = (void*)(zone->baseAddr + (UInt32)*buf);
    zone->numRebases++;
}


void  WMR_BufZone_FinishedRebases(WMR_BufZone_t *zone)
{
    WMR_ASSERT(zone->numAllocs == zone->numRebases);
}


void  WMR_BufZone_Free(WMR_BufZone_t *zone)
{
    if (zone->baseAddr) {
        WMR_FREE(zone->baseAddr, zone->totalAlloc);
    }

    zone->baseAddr = NULL;
    zone->maxAddr = NULL;
    zone->totalAlloc = 0;
    zone->state = BZS_INIT;

    ZoneList_Unlink(zone);
}

// This function is meaningless on non-Darwin targets
//UInt32 WMR_BufZone_FindDescriptor(UInt8* buf, void **des) // returns offset
