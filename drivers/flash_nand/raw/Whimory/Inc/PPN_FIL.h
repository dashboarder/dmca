/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _PPN_FIL_H_
#define _PPN_FIL_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "FILTypes.h"

typedef UInt8 PPNStatusType;
#define PPN_READ_STATUS_INVALID_DATA    (((PPNStatusType)1) << 0)
#define PPN_READ_STATUS_REFRESH         (((PPNStatusType)1) << 1)
#define PPN_READ_STATUS_RETIRE          (((PPNStatusType)1) << 2)
#define PPN_READ_STATUS_CLEAN           (((PPNStatusType)1) << 3)
#define PPN_READ_STATUS_GEB             (((PPNStatusType)1) << 4)
#define PPN_READ_STATUS_VALID           (((PPNStatusType)1) << 6)
#define PPN_READ_STATUS_WRONG_BITS_PER_CELL (((PPNStatusType)1) << 7)    

#define PPN_OTHERS_STATUS_OP_FAILED     (((PPNStatusType)1) << 0)
#define PPN_OTHERS_STATUS_NOT_SUPPORTED (((PPNStatusType)1) << 2)
#define PPN_OTHERS_STATUS_GEB           (((PPNStatusType)1) << 4)
#define PPN_OTHERS_STATUS_VALID         (((PPNStatusType)1) << 6)

#if !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM
#define PPN_PROGRAM_STATUS_FAIL           (((PPNStatusType)1) << 0)
#define PPN_PROGRAM_STATUS_NOT_PROGRAMMED (((PPNStatusType)1) << 2)

#define PPN_PROGRAM_STATUS_GEB          (((PPNStatusType)1) << 4)

#define PPN_ERASE_STATUS_FAIL           (((PPNStatusType)1) << 0)
#define PPN_ERASE_STATUS_RETIRE         (((PPNStatusType)1) << 2)
#define PPN_ERASE_STATUS_GEB            (((PPNStatusType)1) << 4)
#endif // !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM

typedef UInt32 PPNOptions;
#define PPN_OPTIONS_REPORT_HEALTH        (((UInt32)1) << 0)
#define PPN_OPTIONS_GET_PAGE_RMA_INFO    (((UInt32)1) << 1)

// simulator debug options (allocate from the end)
#if (defined (AND_SIMULATOR) && AND_SIMULATOR)
#define PPN_OPTIONS_IGNORE_BAD_BLOCK     (((UInt32)1) << 31)
#define PPN_OPTIONS_GENERATE_BAD_BLOCK   (((UInt32)1) << 30)
#define PPN_OPTIONS_TAKE_SNAPSHOT        (((UInt32)1) << 29)
#else
#define PPN_OPTIONS_IGNORE_BAD_BLOCK     (0) // do not set any bits if we are running outside the simulator
#define PPN_OPTIONS_GENERATE_BAD_BLOCK   (0) // do not set any bits if we are running outside the simulator
#define PPN_OPTIONS_TAKE_SNAPSHOT        (0) // do not set any bits if we are running outside the simulator
#endif

#define PPN_NO_OPTIONS                   (0)

typedef UInt32 PPNCommand;
#define PPN_COMMAND_NONE                 (0UL)
#define PPN_COMMAND_READ                 (1UL)
#if !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM || WMR_IBOOT_WRITE_ENABLE
#define PPN_COMMAND_PROGRAM              (2UL)
#define PPN_COMMAND_ERASE                (3UL)
#endif // !(defined(AND_READONLY)) || AND_SUPPORT_NVRAM
#define PPN_COMMAND_CAUBBT               (4UL)

#define PPN_MAX_CES_PER_BUS         2
#define PPN_MAX_PAGES_PER_CE        128
#define PPN_MAX_PAGES               128

#define PPN_MAX_MEM_BUFFERS         256

#define PPN_VERSION_1_0_0      (0x00010000)
#define PPN_VERSION_1_0_3      (0x00010003)
#define PPN_VERSION_1_5_0      (0x00010500)
#define PPN_VERSION_2_0_0      (0x00020000)
#define PPN_VERSION_ERROR      (0x00FEFEFE)
#define PPN_VERSION_BAD_FW     (0x00FFFFFF)
#define PPN_VERSION_UNSUPPORTED PPN_VERSION_2_0_0

typedef UInt16 ChipEnableType;
typedef UInt8 ChipEnableIndexType;
typedef UInt32 PageAddressType;

typedef struct
{
    UInt16 length;
    UInt16 column;
    PageAddressType row;
} RowColLenAddressType;

typedef struct
{
    FILDeviceInfo *device_info;
    void          *handle; //opaque to caller
                           // at least for now, i am adding a bus identifier
    UInt16 bus_num;
} BusContext;

typedef struct
{
    UInt16 offset; // offset inside the mem_entry
    UInt8 idx;     // what mem entry this is in
} PPNMemoryIndex;

typedef struct
{
    void     *data;
    void     *meta;
    UInt16 num_of_lbas;      // lba is a 4KB chunk - limited to 255 entries
} PPNMemoryEntry;

typedef struct
{
    ChipEnableType ce;
    UInt16 pages;
    UInt16 offset;
} PPNCommandCeInfo;

typedef struct
{
    RowColLenAddressType addr;
    ChipEnableIndexType ceIdx;
    UInt8 lbas;
    PPNStatusType status;
} PPNCommandEntry;

typedef struct
{
    // This section is shared between the A8/A9 and the IOP.

    PPNCommand command;
    PPNOptions options;
    UInt16 num_pages;
    UInt16 lbas;
    PPNStatusType page_status_summary;   // logical OR of all page statuses

    PPNCommandCeInfo ceInfo[PPN_MAX_CES_PER_BUS];
    PPNCommandEntry entry[PPN_MAX_PAGES];

    // This section is used by the A8/A9 only

    BusContext context;

    PPNMemoryIndex mem_index[PPN_MAX_PAGES];
    PPNMemoryEntry mem_buffers[PPN_MAX_MEM_BUFFERS];
    UInt16 mem_buffers_count;
} PPNCommandStruct;

typedef struct
{
    UInt16 channel;
    ChipEnableIndexType chip_enable_idx;
    ChipEnableType physical_chip_enable;
} PPNChipEnableStruct;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _PPN_FIL_H_ */
