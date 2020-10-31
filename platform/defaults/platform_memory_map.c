/*
 * Copyright (C) 2009-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <sys.h>
#include <platform/memmap.h>

/*
 * Default platform memory map; takes advantage of common platform
 * defintions about memory sizes and locations.
 *
 * Note that this definition should only be used for simple applications
 * where memory is all cached and identity-mapped.  More complicated
 * memory arrangements should be described in platform-specific code,
 * rather than complicating this table.
 *
 * For some platforms/targets, SDRAM_LEN is not constant and so we 
 * have to compute a maximum via other means.
 * 
 */
#ifndef SDRAM_MAX_LEN
# ifndef SDRAM_BANK_COUNT
#  define SDRAM_BANK_COUNT	1
# endif
# define SDRAM_MAX_LEN		(SDRAM_BANK_LEN * SDRAM_BANK_COUNT)
#endif

#ifndef SRAM_MAX_LEN
# ifndef SRAM_BANK_COUNT
#  define SRAM_BANK_COUNT	1
# endif
# define SRAM_MAX_LEN		(SRAM_BANK_LEN * SRAM_BANK_COUNT)
#endif

struct mem_static_map_entry mem_static_map_entries[] = {
        /* cached         uncached        physical        size */
#ifdef SDRAM_BASE
        { SDRAM_BASE,     MAP_NO_ENTRY,     SDRAM_BASE,     SDRAM_MAX_LEN },
#endif
#ifdef SRAM_BASE
        { SRAM_BASE,      MAP_NO_ENTRY,     SRAM_BASE,      SRAM_MAX_LEN },
#endif
        {MAP_NO_ENTRY, MAP_NO_ENTRY, MAP_NO_ENTRY, 0}
};
