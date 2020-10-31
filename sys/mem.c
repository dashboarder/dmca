/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
/*
 * Generic memory mapping functions.
 */

#include <sys/types.h>
#include <sys.h>

static struct mem_static_map_entry *
mem_static_find_virtual(uintptr_t addr, ptrdiff_t *offset)
{
        struct mem_static_map_entry     *mp;

        for (mp = mem_static_map_entries; mp->size != 0; mp++) {
                /* is this an entry in a cached mapping? */
                if ((MAP_NO_ENTRY != mp->cached_base) &&
                    (addr >= mp->cached_base) &&
                    (addr <= (mp->cached_base + mp->size - 1))) {
                        *offset = addr - mp->cached_base;
                        return(mp);
                }

                /* is this an entry in an uncached mapping? */
                if ((mp->uncached_base != MAP_NO_ENTRY) &&
                    (addr >= mp->uncached_base) &&
                    (addr <= (mp->uncached_base + mp->size - 1))) {
                        *offset = addr - mp->uncached_base;
                        return(mp);
                }
        }
        return(NULL);
}

static struct mem_static_map_entry *
mem_static_find_physical(uintptr_t addr)
{
        struct mem_static_map_entry     *mp;

        for (mp = mem_static_map_entries; mp->size != 0; mp++) {
                /* is this an entry in a physical mapping? */
                if ((MAP_NO_ENTRY != mp->physical_base) &&
                    (addr >= mp->physical_base) &&
                    (addr <= (mp->physical_base + mp->size - 1)))
                    return(mp);
        }
        return(NULL);
}

void *
mem_static_map_cached(uintptr_t ptr)
{
        struct mem_static_map_entry     *mp;
        ptrdiff_t                       offset;

        mp = mem_static_find_virtual(ptr, &offset);
        if (NULL != mp)
                return((void *)(mp->cached_base + offset));
	return(MAP_FAILED);
}

void *
mem_static_map_uncached(uintptr_t ptr)
{
        struct mem_static_map_entry     *mp;
        ptrdiff_t                       offset;

        mp = mem_static_find_virtual(ptr, &offset);
        if (NULL != mp)
                return((void *)(mp->uncached_base + offset));
	return(MAP_FAILED);
}

uintptr_t
mem_static_map_physical(uintptr_t ptr)
{
        struct mem_static_map_entry     *mp;
        ptrdiff_t                       offset;

        mp = mem_static_find_virtual(ptr, &offset);
        if (NULL != mp)
                return(mp->physical_base + offset);
	return(MAP_NO_ENTRY);
}
