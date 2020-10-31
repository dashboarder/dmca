/*                                                                              
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <drivers/anc_boot.h>
#include <lib/blockdev.h>
#include <platform/memmap.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "util_boot.h"
#include "anc_bootrom.h"
#include "anc_llb.h"


#define BOOT_NUM_ELEMENT    (12)
#define BOOT_NUM_PSLOTS     (32)
typedef struct {
    uint32_t page  : 8;
    uint32_t slot  : 8;
    uint32_t pslot : 8;
    uint32_t RFU   : 8;
} UtilBoot_location_t;

typedef struct {
    UtilBoot_location_t location[UTIL_NUM_ELEMENT];
    uint32_t        prevUtilDMversion;
    uint32_t        prevUtilDMslot;
    bool            BCopyPresent;
    uint8_t         minorVersion;
} UtilBoot_t;

typedef struct {
    UtilBootDM_t    s;
    uint32_t        version;
    uint32_t        RFU;           // was used by mxcfg
    uint16_t        erases[UTIL_NUM_PSLOTS];
    uint8_t         clogMajor;
    uint8_t         clogMinor;
    uint8_t         pendDefect;    // defect pending (shuffle in progress).
    uint8_t         pad[4096 - sizeof(UtilBootDM_t) - 11 - UTIL_NUM_PSLOTS*2];
}UtilDM_t;

extern anc_ppn_device_params_t *anc_geom;
#define CASSERT(x, name) typedef char __ASP_CASSERT_##name[(x) ? 1 : -1]

CASSERT(sizeof(UtilDM_t)==4096, sizeUtilDMcorrect);

extern int anc_num_channel;

#define quantup(num, gran)           (((num) + (gran) - 1) / (gran))
#define ROUNDUPTO(num, gran)        ((((num) + (gran) - 1) / (gran)) * (gran))

static UtilDM_t* UtilDM;
static UtilDM_t* UtilDM_backup;

static UtilBoot_t Util;

#define META_LBA_INDEX 0 
static uint32_t    meta[4];

// prototypes
uint32_t    Boot_Slip         (uint32_t slot);
void        Boot_BandDip      (uint32_t pslot, uint32_t* band, uint32_t* dip);
static int anc_read_fw_block   (struct blockdev *_dev, void *data, block_addr req_offset, uint32_t max_size);
bool Boot_PostProcessDM(UtilDM_t *p, uint32_t slot, uint32_t page, uint32_t *meta);
#if WITH_LLB_NVRAM
bool Boot_Read(void *buffer, uint32_t page, uint32_t slot, uint32_t *meta);
uint32_t Boot_Find_Fast_Scan(void *buffer, uint32_t page, uint32_t slot);
void Boot_Find_Fast(void *buffer);
void Boot_Find_UtilDM(void *buffer);
static int anc_read_nvram_block   (struct blockdev *_dev, void *data, block_addr req_offset, uint32_t max_size);
#endif
#if WITH_LLB_BLKDEV
static int anc_read_llb_block   (struct blockdev *_dev, void *data, block_addr req_offset, uint32_t max_size);
#endif

// POR equivalent.  Resets the Nand but does not do discovery. Zeros out the UtilDM->s.utilMajor to indicate UtilDM not loaded.
bool anc_reset(int resetMode) {
    bool  err;
    
    UtilDM->s.utilMajor = 0;                       // indicate UtilDM not loaded.
    meta[1]=0;
    meta[2]=0;
    meta[3]=0;
    err = anc_bootrom_init(true, resetMode);
    if (err != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

// Entry point from LLB
bool anc_firmware_init(void)
{
    bool             err;
    struct blockdev *bdev;
#if WITH_LLB_NVRAM
    struct blockdev *nv_bdev;
#endif
#if WITH_LLB_BLKDEV
    struct blockdev *llb_bdev;
#endif


    UtilDM = memalign(sizeof(UtilDM_t), CPU_CACHELINE_SIZE);
    UtilDM_backup = memalign(sizeof(UtilDM_t), CPU_CACHELINE_SIZE);

    if (!UtilDM) {
        dprintf(DEBUG_CRITICAL, "Failed to malloc UtilDM\n");
        return false;
    }

    if (!UtilDM_backup) {
        dprintf(DEBUG_CRITICAL, "Failed to malloc UtilDM_backup\n");
        return false;
    }

    bzero(UtilDM, sizeof(UtilDM_t));
    bzero(UtilDM_backup, sizeof(UtilDM_t));

    uint32_t         block_size;
    uint32_t         size;

    err = anc_llb_init();
    if (err != 0)
    {
        dprintf(DEBUG_CRITICAL,"Failed to reset ANC\n");
        return false;
    }

    // Load UtilDM
    meta[META_LBA_INDEX] = BOOT_LBA_TOKEN_UTILDM;
    if (anc_llb_read_phys_page(BOOT_BAND_UTILDM, BOOT_DIP_UTILDM, BOOT_PAGE_UTILDM, BOOT_SIZE_UTILDM, UtilDM_backup, &meta[0])!=BOOT_SIZE_UTILDM) {
        UtilDM->s.utilMajor = 0;
        return(false);
    }

    Boot_PostProcessDM(UtilDM_backup,0,0,meta);
    dprintf(DEBUG_CRITICAL,"Loaded UtilDM  revision = %2d.%2d\n",UtilDM->s.utilMajor, UtilDM->s.utilMinor);

    if (UtilDM->s.eSize[BOOT_ELEMENT_FW] == 0)
    {
        dprintf(DEBUG_CRITICAL,"No firmware region programmed; not creating block device\n");
        return false;
    }

    bdev = malloc(sizeof(struct blockdev));
    if (!bdev)
    {
        dprintf(DEBUG_CRITICAL,"Failed to allocate blockdev\n");
        return false;
    }

    block_size = BOOT_BYTES_PER_SEC * UtilDM->s.secPerPage;
    size = ROUNDUPTO(UtilDM->s.eSize[BOOT_ELEMENT_FW] * BOOT_BYTES_PER_SEC, block_size);

    if (construct_blockdev(bdev, 
                           "anc_firmware",
                           size,
                           block_size) != 0)
    {
        dprintf(DEBUG_CRITICAL,"Failed to construct block device\n");
        free(bdev);
        return false;
    }

    bdev->read_block_hook = &anc_read_fw_block;
    if (register_blockdev(bdev) != 0)
    {
        dprintf(DEBUG_CRITICAL,"Failed to register bdev\n");
        free(bdev);
        return false;
    }

    // ANC DMA has a minimum 16byte alignment requirement
    // But if the CPU isn't coherent with ANC, we need to increase the alignment requirement
    #if WITH_NON_COHERENT_DMA
    blockdev_set_buffer_alignment(bdev, __max(ANC_MIN_ALIGNMENT, CPU_CACHELINE_SIZE));
    #else
    blockdev_set_buffer_alignment(bdev, ANC_MIN_ALIGNMENT);
    #endif


#if 0
    uint32_t *buf;
    uint32_t  i;
    buf = (uint32_t *)INSECURE_MEMORY_BASE;
    dprintf(DEBUG_CRITICAL,"Starting test at %p\n", buf);
    memset(buf, 0, 520192);
    anc_read_fw_block(NULL, buf, 0, 127);
    for (i = 0; i < 520192 / 4; i++)
    {
        if (buf[i] != i)
        {
            panic("Miscompare at 0x%08X got 0x%08X", i, buf[i]);
        }
    }
    dprintf(DEBUG_CRITICAL,"compare passed\n");

    anc_read_fw_block(NULL, buf, 126, 1);
    i = 126 * 1024;
    uint32_t j;
    for (j = 0; j < 1024; j++, i++)
    {
        if (buf[j] != i)
        {
            panic("Miscompare expected 0x%08X got 0x%08X", i, buf[j]);
        }
    }
#endif

#if WITH_LLB_NVRAM
    // keep trying to find the latest
    Boot_Find_UtilDM(UtilDM_backup);
    Boot_Find_Fast(UtilDM_backup);
    nv_bdev = malloc(sizeof(struct blockdev));
    if (!nv_bdev)
    {
        dprintf(DEBUG_CRITICAL,"Failed to allocate nvram blockdev\n");
        return false;
    }


    if (construct_blockdev(nv_bdev, 
                           "nvram",
                           8192,
                           8192) != 0)
    {
        dprintf(DEBUG_CRITICAL,"Failed to construct block device\n");
        free(nv_bdev);
        return false;
    }

    nv_bdev->read_block_hook = &anc_read_nvram_block;
    if (register_blockdev(nv_bdev) != 0)
    {
        dprintf(DEBUG_CRITICAL,"Failed to register bdev\n");
        free(nv_bdev);
        return false;
    }
#endif
#if WITH_LLB_BLKDEV
    llb_bdev = malloc(sizeof(struct blockdev));
    if (!llb_bdev)
    {
        dprintf(DEBUG_CRITICAL,"Failed to allocate LLB blockdev\n");
        return false;
    }

    block_size = BOOT_BYTES_PER_SEC * UtilDM->s.secPerPage;
    size = ROUNDUPTO(UtilDM->s.eSize[BOOT_ELEMENT_LLB] * BOOT_BYTES_PER_SEC, block_size);

    if (construct_blockdev(llb_bdev, 
                           "anc_llb",
                           size,
                           block_size) != 0)
    {
        dprintf(DEBUG_CRITICAL,"Failed to construct block device\n");
        free(llb_bdev);
        return false;
    }

    llb_bdev->read_block_hook = &anc_read_llb_block;

    // ANC DMA has a minimum 16byte alignment requirement
    // But if the CPU isn't coherent with ANC, we need to increase the alignment requirement
    #if WITH_NON_COHERENT_DMA
    blockdev_set_buffer_alignment(llb_bdev, __max(ANC_MIN_ALIGNMENT, CPU_CACHELINE_SIZE));
    #else
    blockdev_set_buffer_alignment(llb_bdev, ANC_MIN_ALIGNMENT);
    #endif


    if (register_blockdev(llb_bdev) != 0)
    {
        dprintf(DEBUG_CRITICAL,"Failed to register bdev\n");
        free(llb_bdev);
        return false;
    }
#endif

    return true;
}


// LLB loader.  Will load UtilDM on first read request which will reset the UtilDM->s.utilMajor to be current.
size_t anc_read_llb(void* data, size_t max_size) {
    uint32_t  page;
    uint32_t  size, thisSize;
    uint32_t  secPerPage;
    bool      err;
    
    if(0 == UtilDM->s.utilMajor) {
        err = anc_bootrom_init(false, 0);      // do discovery
        if(err) {
            return(0);
        }
        meta[META_LBA_INDEX] = BOOT_LBA_TOKEN_UTILDM;
        if (!anc_bootrom_read_phys_page(BOOT_BAND_UTILDM, BOOT_DIP_UTILDM, BOOT_PAGE_UTILDM, BOOT_SIZE_UTILDM, UtilDM, &meta[0])) {
            UtilDM->s.utilMajor = 0;
            return(false);
        }
        dprintf(DEBUG_CRITICAL,"Loaded UtilDM  revision = %2d.%2d\n",UtilDM->s.utilMajor, UtilDM->s.utilMinor);
    }
    

    meta[META_LBA_INDEX] = BOOT_LBA_TOKEN_LLB;
    secPerPage = UtilDM->s.secPerPage; 
    page = BOOT_PAGE_LLB;               // LLB doesn't start at page=0
    
    size = (uint32_t) max_size / BOOT_BYTES_PER_SEC;
    dprintf(DEBUG_CRITICAL,"Reading LLB for size=%d\n", size);
    if(size > UtilDM->s.eSize[BOOT_ELEMENT_LLB]) {
        size = UtilDM->s.eSize[BOOT_ELEMENT_LLB];
        dprintf(DEBUG_CRITICAL,"Trimmed size to written size of %5d\n", size);
    }
    
    max_size = size;  // record for result to caller

    while(size) {
        if(size > secPerPage) {
            thisSize = secPerPage;
        } else {
            thisSize = size;
        }
        
        if(!anc_bootrom_read_phys_page(BOOT_BAND_LLB, BOOT_DIP_LLB, page, thisSize, data, &meta[0])) {
            break;
        }
        size -= thisSize;
        page++;
        data += thisSize * BOOT_BYTES_PER_SEC;
    }

    return( (max_size - size) * BOOT_BYTES_PER_SEC );
}
        

// the code below this comment is required for loading the FW element.





// clone of UtilDM_Slip from product code
uint32_t Boot_Slip(uint32_t pslot) {
    uint32_t    i;
    
    i = 0;
    while(i < UtilDM->s.numDefects && (pslot >= UtilDM->s.defects[i]) ) {
        pslot++;
        i++;
    }  
    return(pslot);
}



// clone of UtilDM_BandDip
// Converts the physical slot to a band,dip that is consistent with Nand engine's usage.  returns dip and writes band to bandPtr
void Boot_BandDip(uint32_t pslot, uint32_t* bandPtr, uint32_t* dipPtr) {
    uint32_t bus;
    uint32_t die;
    uint32_t plane;
    uint32_t cau;

    uint32_t slot_itr;
    uint32_t band;
    uint32_t num_bus;
    uint32_t die_per_bus;
    uint32_t cau_per_die;

    if (anc_get_dies_per_channel() * anc_num_channel == UtilDM->s.numDie) {
        num_bus = anc_num_channel;
    } else {
        // geom_num_bus is incorrect because a bus didn't come up.
        num_bus = UtilDM->s.numDie / anc_get_dies_per_channel();
    }
    die_per_bus = UtilDM->s.numDie / anc_num_channel;
    cau_per_die = UtilDM->s.numPlanes;

    bus   = 0;
    die   = 0;
    plane = 0;
    band  = 0;
    for (slot_itr = 0; slot_itr < pslot; slot_itr++) {
        bus++;

        if(bus >= num_bus) {
            bus = 0;
            die++;

            if (die >= die_per_bus){
                die = 0;
                plane++;

                if (plane >= cau_per_die) {
                    plane=0;
                    band++;
                }
            }
        }
    }
    cau = (die * cau_per_die) + plane;

    *bandPtr = band;
    *dipPtr  = anc_get_dip(bus, cau);
}



// leveraged core UtilDM_PostProcess
bool Boot_PostProcessDM(UtilDM_t *p, uint32_t page, uint32_t slot, uint32_t *meta)
{
    uint32_t wasGrownDef;
    uint32_t newV;
    uint32_t currV;
    bool     result = false;

    if (meta[0] == BOOT_ERR_BLANK) {
        dprintf(DEBUG_INFO,"UTIL - [%2d,%2d] BLANK! \n", slot, page);
    }
    else if (p->s.utilMajor != BOOT_UTIL_MAJOR) {
        dprintf(DEBUG_INFO,"UTIL - [%2d,%2d] !!!!! Found UtilDM with different major version =0x%08x ", slot, page,
               p->s.utilMajor);
        return false;
    } else if (p->s.utilMinor < Util.minorVersion) {
        dprintf(DEBUG_INFO,"UTIL - [%2d,%2d] !!!!! Found UtilDM with major version 0x%08x but minor version 0x%08x (less than minimum 0x%08x)",
               slot, page, p->s.utilMajor, p->s.utilMinor, Util.minorVersion);
        return false;
    } else {
        newV  = p->version;
        currV = UtilDM->version;

        if (newV >= currV) {
            Util.prevUtilDMversion             = currV;
            Util.prevUtilDMslot                = Util.location[BOOT_ELEMENT_UTILDM].slot;
            Util.location[BOOT_ELEMENT_UTILDM].slot = slot;
            Util.location[BOOT_ELEMENT_UTILDM].page = page;
            
            dprintf(DEBUG_INFO,"UTIL - [%2d,%2d] Adopting version=0x%08x numDefects=%2d numSlots=%2d\n", slot, 
                   page, newV, p->s.numDefects,
                   p->s.numSlots);
            wasGrownDef = UtilDM->s.numDefects;
            memcpy(UtilDM, p, sizeof(UtilDM_t));
            
            if (wasGrownDef != UtilDM->s.numDefects) {
                result = true;
            }
            
            if (slot == BOOT_SLOT_FAST_B) {
                Util.BCopyPresent = true;
            } else {
                Util.BCopyPresent = false; // Always expect a B copy on the right of A
            }
        } else if (!Util.BCopyPresent && (slot == BOOT_SLOT_FAST_B) && ((currV == (newV + 1)))) {
            
            // This is the case where we have a newer copy on the left of the older copy  <rdar://problem/15261567>
            // accept B copy if it's version is 1 less than A copy, consider B slot, the previous slot
            Util.prevUtilDMversion = newV;
            Util.prevUtilDMslot    = slot;
            
            dprintf(DEBUG_INFO,"UTIL - [%2d,%2d] Adopting B copy version=0x%08x numDefects=%2d numSlots=%2d", slot, 
                   page, newV, p->s.numDefects,
                   p->s.numSlots);
            Util.BCopyPresent = true;
            
        } else {
            dprintf(DEBUG_INFO,"UTIL - [%2d,%2d] Ignoring version=0x%08x numDefects=%2d numSlots=%2d", slot, 
                   page, newV, p->s.numDefects,
                   p->s.numSlots);
        }
    }

    return result;
}

static int anc_read_fw_block   (struct blockdev *_dev, void *data, block_addr offset, uint32_t sectors) {

    uint32_t    slot;
    uint32_t    slotB;      // offset for the B copy of FW.  0 indicates not doing retry
    uint32_t    pslot;
    uint32_t    page;
    uint32_t    band;
    uint32_t    dip;
    uint32_t    thisSize;
    uint32_t    secPerPage;
    uint32_t    pagePerSlot;
    uint32_t    err;
    uint32_t    max_size;

    // Incoming sectors are in terms of page size (to prevent the client from
    // attempting a sub-page read).
    sectors *= UtilDM->s.secPerPage;
    offset *= UtilDM->s.secPerPage;

    dprintf(DEBUG_INFO,"Reading sectors from FW element from offset=%4d for a size=%4d\n", offset, sectors);;
    
    if(BOOT_UTIL_MAJOR != UtilDM->s.utilMajor) {
        dprintf(DEBUG_CRITICAL,"Exiting anc_read_fw because UtilDM->s.utilMajor is incorrect\n");
        return(0);
    }
    if(offset > UtilDM->s.eSize[BOOT_ELEMENT_FW]) {
        dprintf(DEBUG_CRITICAL,"Requested offset exceeds size of firmware\n");
        return(0);
    }
    
    if(sectors + offset > UtilDM->s.eSize[BOOT_ELEMENT_FW]) {
        dprintf(DEBUG_CRITICAL,"Attempt to read past end of firmware\n");
        return(0);
    }
    
    meta[META_LBA_INDEX] = BOOT_LBA_TOKEN_FW;
    secPerPage = UtilDM->s.secPerPage;       
    pagePerSlot = UtilDM->s.numPages;
    
    page = BOOT_PAGE_FW;               // starting location for FW in [slot,page]
    slot = BOOT_SLOT_FW;

    while(offset >= secPerPage) {
        offset -= secPerPage;
        page++;
        if(page >= pagePerSlot) {
            page = 0;
            slot++;
        }
    }
    // lba now converted to page/slot
    
    err = 0;
    slotB = 0;

    max_size = sectors;                    // save for result
    while(sectors) {
        pslot = Boot_Slip(slot+slotB);
        Boot_BandDip(pslot, &band, &dip);

        thisSize = sectors;
        if(thisSize > secPerPage) {
            thisSize = secPerPage;
        } 
        
        if (anc_llb_read_phys_page(band, dip, page, thisSize, data, &meta[0])!=secPerPage) {
            dprintf(DEBUG_CRITICAL,"Failed to read page\n");
            if(!slotB) {
                slotB = UtilDM->s.sizeFWslots;
            } else {
                break;
            }
        } else {
            slotB = 0;
            sectors -= thisSize;
            data += thisSize * BOOT_BYTES_PER_SEC;
            page++;
            if(page >= pagePerSlot) {
                page = 0;
                slot++;
            }
        }
    }
    if(err) {
        dprintf(DEBUG_CRITICAL,"Encountered error = %d\n", err);
    }

    return (max_size - sectors) / secPerPage;
}

#if WITH_LLB_BLKDEV
// LLB loader.  Will load UtilDM on first read request which will reset the UtilDM->s.utilMajor to be current.
static int anc_read_llb_block   (struct blockdev *_dev, void *data, block_addr offset, uint32_t sectors) {
    uint32_t  page;
    uint32_t  size, thisSize, max_size, secRead;
    uint32_t  secPerPage;
    bool      err;
    
    if(0 == UtilDM->s.utilMajor) {
        err = anc_bootrom_init(false, 0);      // do discovery
        if(err) {
            return(0);
        }
        meta[META_LBA_INDEX] = BOOT_LBA_TOKEN_UTILDM;
        if (anc_llb_read_phys_page(BOOT_BAND_UTILDM, BOOT_DIP_UTILDM, BOOT_PAGE_UTILDM, BOOT_SIZE_UTILDM, UtilDM, &meta[0])!=BOOT_SIZE_UTILDM) {
            UtilDM->s.utilMajor = 0;
            return(false);
        }
        dprintf(DEBUG_CRITICAL,"Loaded UtilDM  revision = %2d.%2d\n",UtilDM->s.utilMajor, UtilDM->s.utilMinor);
    }
    

    meta[META_LBA_INDEX] = BOOT_LBA_TOKEN_LLB;
    secPerPage = UtilDM->s.secPerPage; 
    page = BOOT_PAGE_LLB + offset;
    
    size = sectors * secPerPage;
    dprintf(DEBUG_SPEW,"Reading LLB for size=%d, page=%u\n", sectors, (unsigned int)page);
    if(size > UtilDM->s.eSize[BOOT_ELEMENT_LLB]) {
        size = UtilDM->s.eSize[BOOT_ELEMENT_LLB];
        dprintf(DEBUG_CRITICAL,"Trimmed size to written size of %5d\n", size);
    }
    
    max_size = size;  // record for result to caller

    while(size) {
        if(size > secPerPage) {
            thisSize = secPerPage;
        } else {
            thisSize = size;
        }
        
        secRead = anc_llb_read_phys_page(BOOT_BAND_LLB, BOOT_DIP_LLB, page, thisSize, data, &meta[0]);
        if ((secRead == 0) || (secRead<thisSize && size>secPerPage)) { // only allow partial NAND page at the end of read
            break;
        }
        size -= thisSize;
        page++;
        data += thisSize * BOOT_BYTES_PER_SEC;
    }

    return( (max_size - size) / secPerPage );
}
#endif        

#if WITH_LLB_NVRAM
static int anc_read_nvram_block   (struct blockdev *_dev, void *data, block_addr offset, uint32_t sectors) {

    uint32_t    slot;
    uint32_t    slotB;      // offset for the B copy of FW.  0 indicates not doing retry
    uint32_t    pslot;
    uint32_t    page;
    uint32_t    band;
    uint32_t    dip;
    uint32_t    thisSize;
    uint32_t    secPerPage;
    uint32_t    pagePerSlot;
    uint32_t    err;
    uint32_t    max_size;

    // Incoming sectors are in terms of page size (to prevent the client from
    // attempting a sub-page read).
    sectors *= 2; // blocksize is 8k not 4k

    dprintf(DEBUG_INFO,"Reading sectors from NVRAM element from offset=%4d for a size=%4d\n", offset, sectors);
    
    if(BOOT_UTIL_MAJOR != UtilDM->s.utilMajor) {
        dprintf(DEBUG_CRITICAL,"Exiting anc_read_fw because UtilDM->s.utilMajor is incorrect\n");
        return(0);
    }
    if(offset > 0) {
        dprintf(DEBUG_CRITICAL,"Requested offset exceeds size of nvram\n");
        return(0);
    }
    
    if(sectors + offset > UtilDM->s.eSize[BOOT_ELEMENT_NVRAM]) {
        dprintf(DEBUG_CRITICAL,"Attempt to read past end of nvram\n");
        return(0);
    }
    
    meta[META_LBA_INDEX] = BOOT_LBA_TOKEN_NVRAM;
    secPerPage = UtilDM->s.secPerPage;       
    pagePerSlot = UtilDM->s.numPages;
    
    page = Util.location[BOOT_ELEMENT_NVRAM].page;               // starting location for NVRAM in [slot,page]
    slot = Util.location[BOOT_ELEMENT_NVRAM].slot;

    err = 0;
    slotB = 0;

    max_size = sectors;                    // save for result
    if (page==0 && slot==0) {// NVRAM not populated
        sectors=0;
    }
    while(sectors) {
        pslot = Boot_Slip(slot+slotB);
        Boot_BandDip(pslot, &band, &dip);

        thisSize = sectors;
        if(thisSize > secPerPage) {
            thisSize = secPerPage;
        } 
        
        if(!anc_llb_read_phys_page(band, dip, page, thisSize, data, &meta[0])) {
            dprintf(DEBUG_CRITICAL,"Failed to read page\n");
            if(!slotB) {
                slotB = 1;
            } else {
                break;
            }
        } else {
            slotB = 0;
            sectors -= thisSize;
            
            data += thisSize * BOOT_BYTES_PER_SEC;
            page++;
            if(page >= pagePerSlot) {
                page=0;
                slot++;
                if (sectors) { // still more to read?
                    err=1;
                    break;
                }
            }
        }
    }
    if(err) {
        dprintf(DEBUG_CRITICAL,"Encountered error = %d\n", err);
    }

    return (max_size - sectors) / 2;
}




uint32_t Boot_Find_Fast_Scan(void *buffer, uint32_t page, uint32_t slot) {
    uint32_t  element;
    uint32_t  ElementSizePages;

    if (0 == page) {
        UtilDM->version    = 1;
    }

    for (; page < UtilDM->s.numPages;) {
        meta[0]          = BOOT_LBA_TOKEN_UNKNOWN;
        if (Boot_Read(buffer,page,slot,meta)) {
            if ((meta[0] >= BOOT_LBA_TOKEN_FAST_FIRST) && (meta[0] <= BOOT_LBA_TOKEN_FAST_LAST)) {
                
                element = meta[0] - BOOT_LBA_TOKEN_ELEMENTS;
                dprintf(DEBUG_INFO,"UTIL_MINOR - [%2d,%2d] Found %d\n", slot, page, element);

                if (0 == UtilDM->s.eSize[element]) {
                    dprintf(DEBUG_CRITICAL,"UTIL - Element of unknown size!!! \n");
                    page++;
                } else {
                    if (element == BOOT_ELEMENT_UTILDM) {
                        Boot_PostProcessDM(buffer,page,slot,meta);
                    } else {
                        Util.location[element].slot = slot;
                        Util.location[element].page = page;
                        dprintf(DEBUG_INFO,"UTIL_MINOR - Found location [%2d,%2d] for element %d\n", Util.location[element].slot,
                               Util.location[element].page,element);
                    }
                    
                    page += quantup(UtilDM->s.eSize[element], UtilDM->s.secPerPage);
                }
            } else if (!((meta[0] >= BOOT_LBA_TOKEN_FAST_FIRST) && (meta[0] <= BOOT_LBA_TOKEN_FAST_LAST)) && (meta[0] < BOOT_LBA_TOKEN_UNKNOWN)) {
                element          = meta[0] - BOOT_LBA_TOKEN_ELEMENTS;
                ElementSizePages = 0;
                
                if (element < UTIL_NUM_ELEMENT) {
                    ElementSizePages = quantup(UtilDM->s.eSize[element], UtilDM->s.secPerPage);
                }
                
                if ((ElementSizePages == 0) || ((page + ElementSizePages) > UtilDM->s.numPages)) {
                    dprintf(DEBUG_CRITICAL,"UTIL - [%2d,%2d] Something Broken, UtilDM->s.eSize[%d]=%d!!\n", slot, page, element,
                           ElementSizePages);
                    page = 0;
                    break;
                    
                } else {
                    dprintf(DEBUG_CRITICAL,"Skipping unknown element %d\n", element);
                }
                
                page += ElementSizePages;
            }
        } 
        else
        {
            if ( BOOT_ERR_BLANK != meta[0]) { // something ugly ?, either unknown LBA value or unc                                     
                dprintf(DEBUG_CRITICAL,"UTIL - [%2d,%2d] Something Broken !! meta=%x\n", slot, page, meta[0]);
                page = 0;
            } else {
                dprintf(DEBUG_INFO,"UTIL_MINOR - [%2d,%2d] Blank !!\n", slot, page);
            }
            break;
        }
    }
    return page;
}


// passed in a single sector seg with buffer allocated                                                                             
void Boot_Find_Fast(void *buffer) {
    uint32_t  page;
    uint32_t  slot, tmpslot;
    uint32_t element;

    tmpslot = slot = Util.location[BOOT_ELEMENT_UTILDM].slot;// slot with the highest version UtilDM                                                                                             
    if ((BOOT_SLOT_FAST_B == slot) && (BOOT_SLOT_FAST_A == Util.prevUtilDMslot) && (Util.prevUtilDMversion == UtilDM->version)) { // NORMAL state                                                                                                                     
        slot                = BOOT_SLOT_FAST_A;
        page                = Boot_Find_Fast_Scan(buffer, 1, slot);
        
        if (page) {
            slot = BOOT_SLOT_FAST_B; // read copy B                                                                      
            meta[0]      = BOOT_LBA_TOKEN_UNKNOWN;

            if (!Boot_Read(buffer,page-1,slot,meta)) {
                dprintf(DEBUG_INFO,"UTIL - Fast Copy B last page broken!\n");
                page = 0;
            } else if (page < UtilDM->s.numPages) { // last written page of B is ok, check next page is blank                       
                slot = BOOT_SLOT_FAST_B;
                meta[0]       = BOOT_LBA_TOKEN_UNKNOWN;
                if (!Boot_Read(buffer,page,slot,meta) && meta[0]!=BOOT_ERR_BLANK) {
                    dprintf(DEBUG_CRITICAL,"UTIL - Fast Copy B next location not Blank!!\n");
                    page = 0;
                }
            }
        }
    } else {
        // spew enough data to know why we failed the IF term.                                                                     
        dprintf(DEBUG_CRITICAL,"UTIL - FastA:%2d slot:%2d prevSlot:%2d prevV:%08x currV:%08x\n", 
                BOOT_SLOT_FAST_A, slot, Util.prevUtilDMslot, 
                Util.prevUtilDMversion, UtilDM->version);
        
        
        if ((BOOT_SLOT_LLB != Util.prevUtilDMslot) && !(!Util.BCopyPresent && (BOOT_SLOT_FAST_B == Util.prevUtilDMslot))) {
            /*Scan the older copy first*/
            slot = Util.prevUtilDMslot;
            Boot_Find_Fast_Scan(buffer, 0, slot);
        }
        
        slot = tmpslot;
        Boot_Find_Fast_Scan(buffer, 0, slot);
        page           = 0;
    }

    for (element = BOOT_ELEMENT_FAST_FIRST; element <= BOOT_ELEMENT_FAST_LAST; element++) {
        dprintf(DEBUG_INFO,"UTIL - Location [%2d,%2d] for element %d\n", Util.location[element].slot, Util.location[element].page,
               element);
    }
}



bool Boot_Read(void *buffer, uint32_t page, uint32_t slot, uint32_t *meta)
{
    uint32_t band, dip;
    uint32_t pslot = Boot_Slip(slot);//+slotB);
    Boot_BandDip(pslot, &band, &dip);

    return (anc_llb_read_phys_page(band, dip, page, 1, buffer, &meta[0]));
}

void Boot_Find_UtilDM(void *buffer)
{

    uint32_t firstSlot         = Util.location[BOOT_ELEMENT_UTILDM].slot;
    uint32_t slot, page;
    
    UtilDM->s.numSlots = 32; //UTIL_BLIND_SEARCH;
    page=0;

    for (slot = BOOT_SLOT_FAST_A; slot < UtilDM->s.numSlots;) {
        
        if (slot == UtilDM->pendDefect) {
            slot++;
            continue;
        }
        meta[0]=BOOT_LBA_TOKEN_UNKNOWN;
        Boot_Read(buffer,page,slot,meta);
        
        if (Boot_PostProcessDM(buffer,page,slot,meta)) {
            dprintf(DEBUG_INFO,"UTIL - defect growth\n");                         // don't assume that we're aligned.
            slot = BOOT_SLOT_FAST_A; // start over
            Util.location[BOOT_ELEMENT_UTILDM].slot = firstSlot;        // and assimlate again.
        } else {
            slot++;
        }
    }

    if (0 == UtilDM->version) {                                     // found nothing?
        dprintf(DEBUG_CRITICAL,"UTIL - Found no UtilDM!!! Nand should be blank\n");
        UtilDM->s.numSlots   = 0;                                   // then reset numSlots
        UtilDM->s.numDefects = 0;
    }
}

#endif
