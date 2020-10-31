/*
 * Copyright (C) 2010-2011, 2013-2014 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <debug.h>
#include <compiler.h>
#include <lib/mib.h>
#include <lib/partition.h>
#include <lib/blockdev.h>

#include "partition_private.h"

struct gpt_pth {
        char            sig[8];
#define PTH_SIGNATURE   "EFI PART"
        uint32_t        revision;
#define PTH_REVISION    0x00010000
        uint32_t        hsize;
        uint32_t        hcrc;
        uint32_t        reserved;
        uint64_t        this_lba;
        uint64_t        backup_lba;
        uint64_t        first_lba;
        uint64_t        last_lba;
        uint8_t         uuid[16];
        uint64_t        ptab_lba;
        uint32_t        ptab_entrycount;
        uint32_t        ptab_entrysize;
        uint32_t        ptab_crc;
};

struct gpt_pte {
        uint8_t         ptype_uuid[16];
        uint8_t         unique_uuid[16];
        uint64_t        first_lba;
        uint64_t        last_lba;
        uint64_t        attributes;
#define PATTR_SYSTEM            (1ULL<<0)
#define PATTR_READONLY          (1ULL<<60)
#define PATTR_HIDDEN            (1ULL<<62)
#define PATTR_NOMOUNT           (1ULL<<63)
        uint16_t        name[36];
};


/*
 * Known UUIDs.  Note that these are in the inexplicable
 * pre-swapped form.
 */

static uint8_t  ptype_hfs[] __unused = {
        0x00, 0x53, 0x46, 0x48, 0x00, 0x00, 0xaa, 0x11,
        0xaa, 0x11, 0x00, 0x30, 0x65, 0x43, 0xec, 0xac
};

static uint8_t  ptype_bdp[] __unused = {
        0xa2, 0xa0, 0xd0, 0xeb, 0xe5, 0xb9, 0x33, 0x44,
        0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7
};

int
gpt_scan(struct blockdev *dev, struct partition_entry *entry_list)
{
        unsigned int    entry, part;
        off_t           cursor;
        struct gpt_pth  *pth;
        struct gpt_pte  *pte;
        const char      *reason;
        size_t          cacheLineSize = mib_get_size(kMIBPlatformCacheLineSize);

        pth = NULL;
        pte = NULL;
        entry = 0;
        reason = NULL;

        /* allocate buffers */
        pth = memalign(sizeof(*pth), cacheLineSize);
        pte = memalign(sizeof(*pte), cacheLineSize);

        /* read the partition table header from LBA 1 */
        if (sizeof(*pth) != blockdev_read(dev, pth, dev->block_size, sizeof(*pth))) {
                reason = "read error";
                goto out;
        }
        if (memcmp(pth->sig, PTH_SIGNATURE, 8)) {
                reason = "bad signature";
                goto out;
        }
        if (PTH_REVISION != pth->revision) {
                reason = "bad revision";
                goto out;
        }
        if (pth->ptab_entrysize < sizeof(*pte)) {
                reason = "entries too small";
                goto out;
        }

        /* set up to scan partition table entries */
        cursor = pth->ptab_lba << dev->block_shift;

        part = 0;
        while ((entry < MAX_PARTITIONS) && (part++ < pth->ptab_entrycount)) {
                /* read an entry */
                if (sizeof(*pte) != blockdev_read(dev, pte, cursor, sizeof(*pte))) {
                        reason = "read error";
                        goto out;
                }
                cursor += pth->ptab_entrysize;

                /* skip no-mount entries */
                if (pte->attributes & PATTR_NOMOUNT) {
                        continue;
                }

                /* assume not valid */
                entry_list[entry].valid = false;
                
                /* handle recognised partition types */
                dprintf(DEBUG_SPEW, "gpt: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
                    pte->ptype_uuid[0], pte->ptype_uuid[1], pte->ptype_uuid[2], pte->ptype_uuid[3],
                    pte->ptype_uuid[4], pte->ptype_uuid[5], pte->ptype_uuid[6], pte->ptype_uuid[7],
                    pte->ptype_uuid[8], pte->ptype_uuid[9], pte->ptype_uuid[10], pte->ptype_uuid[11],
                    pte->ptype_uuid[12], pte->ptype_uuid[13], pte->ptype_uuid[14], pte->ptype_uuid[15]);
//#if WITH_HFS
                if (!memcmp(pte->ptype_uuid, ptype_hfs, 16)) {
                        entry_list[entry].valid = true;
                        entry_list[entry].id = 0xaf;            /* HFS+ */
                }
//#endif
                if (!entry_list[entry].valid)
                        continue;
                
                /* calculate partition layout */
                entry_list[entry].offset = (off_t)pte->first_lba << dev->block_shift;
                entry_list[entry].len = (off_t)(pte->last_lba - pte->first_lba + 1) << dev->block_shift;
                entry++;
        }
        
out:
        if (NULL != pte)
                free(pte);
        if (NULL != pth)
                free(pth);
        if (NULL != reason)
                dprintf(DEBUG_INFO, "gpt: %s\n", reason);
        return entry;
}
