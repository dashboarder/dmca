/*
 * Copyright (c) 2010-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* ========================================================================== */

/*! 
  @header nand_part_interface

  @abstract portable core for (mostly boot-related) nand partitioning logic

  @copyright Apple Inc.

  @updated 2012-02-21 
*/

/* ========================================================================== */

#ifndef _NAND_PART_INTERFACE_H
#define _NAND_PART_INTERFACE_H

__BEGIN_DECLS

/* ========================================================================== */

// disable significant amounts of the logging for tiny build profile
#if (defined(WITH_NAND_PART_BUILD_TINY) && WITH_NAND_PART_BUILD_TINY)
# define NAND_PART_EXTENDED_LOGGING 0
#else
# define NAND_PART_EXTENDED_LOGGING 1
#endif

// enable for local development only; DO NOT SUBMIT with this enabled to trunk
#define NAND_PART_LOCAL_DEBUG_ONLY 0


/* ========================================================================== */
/* property keys                                                              */

#define kNandPartCoreVersionMajorKey  "VersionMajor"
#define kNandPartCoreVersionMinorKey  "VersionMinor"
#define kNandPartCoreGenerationKey    "Generation"

#define kNandPartCoreIsFormattedKey   "isFormatted"
#define kNandPartCoreIsNewbornKey     "isNewborn"
#define kNandPartCoreIsRebornKey      "isReborn"

#define kNandPartCoreMaxSparesKey     "maxSpares"


/* ========================================================================== */

enum _NandPartLogLevel {

        kNandPartLogLevelAlways = 0,

        kNandPartLogLevelBug    = 1,
        kNandPartLogLevelError  = 2,
        kNandPartLogLevelInfo   = 3,
        kNandPartLogLevelWarn   = 4,
        kNandPartLogLevelDebug  = 5,
        kNandPartLogLevelSpew   = 6,

        kNandPartLogLevelNever
};

typedef enum _NandPartLogLevel NandPartLogLevel;

extern const char * kNandPartLogTagBug;
extern const char * kNandPartLogTagError;
extern const char * kNandPartLogTagInfo;
extern const char * kNandPartLogTagWarn;
extern const char * kNandPartLogTagDebug;
extern const char * kNandPartLogTagSpew;


/* ========================================================================== */

struct _NandPartGeometry {

        bool                    isPpn;

        uint32_t                ceCount;

        uint32_t                blocksPerCE;
        uint32_t                blockStride;

        uint32_t                pagesPerMLCBlock;
        uint32_t                pagesPerSLCBlock;

        uint32_t                bytesPerBootPage;
        uint32_t                bytesPerFullPage;
        uint32_t                bytesPerFullPageMeta;

        uint32_t                bitsPerCauAddr;
        uint32_t                bitsPerPageAddr;
        uint32_t                bitsPerBlockAddr;
};

typedef struct _NandPartGeometry NandPartGeometry;


/* ========================================================================== */

struct _NandPartSpec {
        uint32_t                type;
        uint32_t                content;
        uint32_t                depth;
        uint32_t                width;
        uint32_t                flags;
};

typedef struct _NandPartSpec NandPartSpec;


/* ========================================================================== */

struct _NandPartParams {

        bool                    isBootPartErasable;
        bool                    isBootPartWritable;
        bool                    isFormatEnabled;
        bool                    isDebugEnabled;
        bool                    isHostTraceEnabled;
        bool                    isClientTraceEnabled;
        bool                    isPoolAllowed;
        uint32_t                maxMajorVersion;
        NandPartSpec            partitionSpecs[kNandPartEntryMaxCount];
};

typedef struct _NandPartParams NandPartParams;


/* ========================================================================== */

struct _NandPartProvider {
                                
        void *                  context;
        NandPartGeometry        geometry;
        NandPartParams          params;

        void                    (*lock_interface)(void * context);
        void                    (*unlock_interface)(void * context);
        void *                  (*alloc_mem)(void * context, unsigned size, bool align);
        int                     (*compare_mem)(void * context, const void * left, const void * right, unsigned int size);
        void *                  (*move_mem)(void * context, void * dest, const void * src, unsigned size);
        void *                  (*set_mem)(void * context, void * mem, uint8_t val, unsigned size);
        void                    (*free_mem)(void * context, void * mem, unsigned size);
        void                    (*read_random)(void * context, void * buf, unsigned size);
        bool                    (*read_boot_page)(void * context, uint32_t ce, uint32_t addr, void * buf, bool * is_clean);
        bool                    (*read_full_page)(void * context, uint32_t ce, uint32_t addr, void * buf, void * meta, bool * is_clean);
        bool                    (*write_boot_page)(void * context, uint32_t ce, uint32_t addr, void * buf);
        bool                    (*write_full_page)(void * context, uint32_t ce, uint32_t addr, void * buf, void * meta);
        bool                    (*erase_block)(void * context, uint32_t ce, uint32_t addr, uint32_t block);
        bool                    (*is_block_factory_bad)(void * context, uint32_t ce, uint32_t block);
        bool                    (*validate_spare_list)(void * context, uint32_t * ce_list, uint32_t * block_list, unsigned count);
        bool                    (*request_spare_blocks)(void * context, uint32_t * ce_list, uint32_t * block_list, unsigned count, unsigned * quantity);
        bool                    (*set_data_property)(void * context, const char * key, void * buf, unsigned size);
        bool                    (*set_number_property)(void * context, const char * key, uint64_t num, unsigned bits);
        bool                    (*set_string_property)(void * context, const char * key, const char * cstring);
        bool                    (*set_boolean_property)(void * context, const char * key, bool val);
        bool                    (*wait_for_fbbt_service)(void * context);
        bool                    (*create_partition_device)(void * context, NandPartEntry * entry, uint32_t idx);
        bool                    (*publish_partition_device)(void * context, uint32_t idx);
        void                    (*vlogf_message)(void * context, NandPartLogLevel lvl, const char * fmt, va_list ap);
};

typedef struct _NandPartProvider NandPartProvider;


/* ========================================================================== */

struct _NandPartInterface {

        void *                  core;

        bool                    (*load_ptab)(void * core);
        bool                    (*erase_boot_partition)(void * core);
        bool                    (*write_boot_partition)(void * core, void * llbBuf, unsigned llbSize);
        bool                    (*is_block_bad)(void * core, uint32_t partIndex, uint32_t partBank, uint32_t partBlock);
        const NandPartEntry *   (*get_entry)(void * core, uint32_t partIndex);
        uint32_t                (*get_bank_width)(void * core, uint32_t partIndex);
        uint32_t                (*get_block_depth)(void * core, uint32_t partIndex);
        uint32_t                (*get_pages_per_block)(void * core, uint32_t partIndex);
        uint32_t                (*get_bytes_per_page)(void * core, uint32_t partIndex);
        bool                    (*read_page)(void * core, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, uint32_t pageInBlock, void * buf, bool * isClean);
        bool                    (*write_page)(void * core, uint32_t partIndex, uint32_t partBank, uint32_t partBlock, uint32_t pageInBlock, void * buf);
        bool                    (*erase_block)(void * core, uint32_t partIndex, uint32_t partBank, uint32_t partBlock);
        bool                    (*request_ptab_diff)(void * core, uint32_t partIndex, void * buf, unsigned size);
        bool                    (*provide_ptab_diff)(void * core, uint32_t partIndex, const void * buf, unsigned size);
        bool                    (*format_ptab)(void * core);
        unsigned                (*get_ptab_bytes)(void * core, void * buf, unsigned size);
};

typedef struct _NandPartInterface NandPartInterface;

#define _npiExec(npi, fn, args...) npi->fn(npi->core, ##args)

#define npi_load_ptab(npi) _npiExec(npi, load_ptab)
#define npi_erase_boot_partition(npi)  _npiExec(npi, erase_boot_partition)
#define npi_write_boot_partition(npi, buf, size)  _npiExec(npi, write_boot_partition, buf, size)
#define npi_is_block_bad(npi, part, bank, block)  _npiExec(npi, is_block_bad, part, bank, block)
#define npi_get_entry(npi, part)  _npiExec(npi, get_entry, part)
#define npi_get_bank_width(npi, part)  _npiExec(npi, get_bank_width, part)
#define npi_get_block_depth(npi, part)  _npiExec(npi, get_block_depth, part)
#define npi_get_pages_per_block(npi, part)  _npiExec(npi, get_pages_per_block, part)
#define npi_get_bytes_per_page(npi, part)  _npiExec(npi, get_bytes_per_page, part)
#define npi_read_page(npi, part, bank, block, page, buf, clean)  _npiExec(npi, read_page, part, bank, block, page, buf, clean)
#define npi_write_page(npi, part, bank, block, page, buf)  _npiExec(npi, write_page, part, bank, block, page, buf)
#define npi_erase_block(npi, part, bank, block)  _npiExec(npi, erase_block, part, bank, block)
#define npi_request_ptab_diff(npi, part, buf, size)  _npiExec(npi, request_ptab_diff, part, buf, size)
#define npi_provide_ptab_diff(npi, part, buf, size)  _npiExec(npi, provide_ptab_diff, part, buf, size)
#define npi_format_ptab(npi)  _npiExec(npi, format_ptab)
#define npi_get_ptab_bytes(npi, buf, size)  _npiExec(npi, get_ptab_bytes, buf, size)


/* ========================================================================== */

extern bool nand_part_init(NandPartInterface ** interface, NandPartProvider * provider);
extern void nand_part_dump(NandPartInterface * interface);

/* ========================================================================== */

__END_DECLS

#endif /* _NAND_PART_INTERFACE_H */
