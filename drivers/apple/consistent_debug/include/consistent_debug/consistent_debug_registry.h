/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DRIVERS_CONSISTENT_DEBUG_REGISTRY_H
#define __DRIVERS_CONSISTENT_DEBUG_REGISTRY_H


// Consistent Debugging Root Structure Definition.
// See documentation at <TODO: CONSISTENT DEBUGGING WIKI LINK>


// Debug Root Pointer Definitions:
// 
// The Debug Root Pointer is used to find the location of the
// Debug Registry. It is currently implemented as a 32-bit PMGR
// scratch register that's reset on boot.



// Debug Root Pointer Scratch Register Format:
//   We have 64 bit addresses but a 32-bit scratch register, so
//   do a bit of encoding to at least futureproof the use of a
//   32-bit register to the foreseeable future.
//
// Bit: ---31..30-|---- 29...0 -----
//      |  TYPE   |    ADDR        |
//      -------------------------
// 
// TYPE:
//     0b00: ADDR is (physical address >> 12). This is good as
//     long as our physical address space is within 42 bits,
//     which like it will be true for a while.
// 
//     -other-: Reserved for future encodings. Expected use
//      cases are for larger-sized  granularity, or indexing off
//      another known location like SRAM_BASE/SDRAM_BASE.
// 
// Helper functions for producing these encodings are in
// consistent_debug_helper_fns.h


// Debug Registry Definitions:

typedef enum {
    DBG_PROCESSOR_AP = 1,
    DBG_COPROCESSOR_ANS,
    DBG_COPROCESSOR_SEP,
    DBG_COPROCESSOR_SIO,
    DBG_COPROCESSOR_ISP,
    DBG_COPROCESSOR_OSCAR,
    DBG_NUM_PROCESSORS
} dbg_processor_t;

#define DEBUG_REGISTRY_MAX_RECORDS 512

#define DEBUG_RECORD_ID_LONG(a, b,c ,d, e, f, g, h)  ( ((unsigned long long)((((h) << 24) & 0xFF000000) | (((g) << 16) & 0x00FF0000) | (((f) << 8) & 0x0000FF00) | ((e) & 0x00FF)) << 32) | \
                                      (unsigned long long)((((d) << 24) & 0xFF000000) | (((c) << 16) & 0x00FF0000) | (((b) << 8) & 0x0000FF00) | ((a) & 0x00FF)) )
#define DEBUG_RECORD_ID_SHORT(a,b,c,d) DEBUG_RECORD_ID_LONG(a,b,c,d,0,0,0,0)
/* The Debug Registry lives near the top of SDRAM in a reserved
   region. Its size is fixed at compile time and defined as
   DEBUG_REGISTRY_MAX_RECORDS. This region holds pointers to
   where the actual structures are. This allows more costly data
   structures to be allocated at runtime and as needed.
 
   iOS and iBoot will provide APIs for registering records here.
   Note that ONLY the AP will be allowed to register entries in
   the Consistent Debug Registry. An IOP that wishes to have an
   entry have an AP-side handler reserve a record. Once a record
   is reserved, anyone may own it.
 
 *   |---------------------------|
 *   |     record_id             |
 *   |---------------------------|
 *   |        length             | 
 *   |---------------------------|
 *   |        physaddr           |
 *   |---------------------------|
 *   |     next_record_id        |
     .......
                                  */
typedef struct {
    uint64_t record_id; // 64-bit unique ID identifying the record
    uint64_t length;    // Length of the payload
    uint64_t physaddr;  // System physical address of entry
} dbg_record_header_t;

// Life cycle of a record_id:
// Starts out as DbgIdUnusedEntry. When reserved, it becomes
// kDbgIdReservedEntry. Once the correct address/length has been
// filled in, the owner can write the appropriate record ID.
//
// When freeing an entry, the agent should write
// kDbgIdFreeReqEntry to the field. Then, the external reader is
// expected to notice the record_id change and stop using this
// debug region. After the reader no longer is using these
// resources, it will write in kDbgIdFreeAckEntry. Then, the
// consistent debug driver will respond by zeroing the entry.

#define kDbgIdUnusedEntry 0x0ULL
#define kDbgIdReservedEntry DEBUG_RECORD_ID_LONG('R','E','S','E','R','V','E', 'D')
#define kDbgIdFreeReqEntry  DEBUG_RECORD_ID_LONG('F','R','E','E','-','R','E','Q')
#define kDbgIdFreeAckEntry  DEBUG_RECORD_ID_LONG('F','R','E','E','-','A','C','K')


#define kDbgIdTopLevelHeader DEBUG_RECORD_ID_SHORT('D','B','G','H')

/*
    The first entry in the debug area is a "top level header",
    which has a different layout than following records. It
    describes the size and number of records. Although these
    parameters are derivable by looking up the top level
    header's magic record ID, this approach lends to more robust
    code run-time parsing of the debug area that doesn't have to
    be constantly updated for minor format changes.
*/
typedef struct {
    uint64_t record_id;             // = kDbgIdTopLevelHeader
    uint32_t num_records;           // = DEBUG_REGISTRY_MAX_RECORDS
    uint32_t record_size_bytes;     // = sizeof(dbg_record_header_t)
} dbg_top_level_header_t;


/* 
 *      Shared Memory Console Descriptors:
 *      Record ID: One per SHMConsole
 */

#define DbgIdConsoleHeaderForIOP(which_dbg_processor, which_num) (DEBUG_RECORD_ID_LONG('C','O','N',0,0,0,which_dbg_processor,which_num))

#define kDbgIdConsoleHeaderAP DbgIdConsoleHeaderForIOP(DBG_PROCESSOR_AP, 0)
#define kDbgIdConsoleHeaderANS DbgIdConsoleHeaderForIOP(DBG_COPROCESSOR_ANS, 0)
#define kDbgIdConsoleHeaderSIO DbgIdConsoleHeaderForIOP(DBG_COPROCESSOR_SIO, 0)
#define kDbgIdConsoleHeaderSEP DbgIdConsoleHeaderForIOP(DBG_COPROCESSOR_SEP, 0)
#define kDbgIdConsoleHeaderISP DbgIdConsoleHeaderForIOP(DBG_COPROCESSOR_ISP, 0)
#define kDbgIdConsoleHeaderOscar DbgIdConsoleHeaderForIOP(DBG_COPROCESSOR_OSCAR, 0)


/* 
 * Coprocessor Progress Reports
 *
 * The CPR is a standardized structure across all coprocessors
 * that gives brief info as to its state.
 *
 * Each coprocessor is responsible for updating its state. The
   ring buffer is supposed to wrap around when full, ensuring
   that the current state is always present.

*/

#define DbgIdCPRHeaderForIOP(which_dbg_processor, which_num) (DEBUG_RECORD_ID_LONG('C','P','R',0,0,0,which_dbg_processor,which_num))

#define kDbgIdCPRHeaderBase DbgIdCPRHeaderForIOP(0, 0)
#define kDbgIdCPRHeaderAP DbgIdCPRHeaderForIOP(DBG_PROCESSOR_AP, 0)
#define kDbgIdCPRHeaderANS DbgIdCPRHeaderForIOP(DBG_COPROCESSOR_ANS, 0)
#define kDbgIdCPRHeaderSIO DbgIdCPRHeaderForIOP(DBG_COPROCESSOR_SIO, 0)
#define kDbgIdCPRHeaderSEP DbgIdCPRHeaderForIOP(DBG_COPROCESSOR_SEP, 0)
#define kDbgIdCPRHeaderISP DbgIdCPRHeaderForIOP(DBG_COPROCESSOR_ISP, 0)
#define kDbgIdCPRHeaderOscar DbgIdCPRHeaderForIOP(DBG_COPROCESSOR_OSCAR, 0)

typedef enum {
    DBG_CPR_STATE_POWEROFF,
    DBG_CPR_STATE_BOOTING, // In the bootloader
    DBG_CPR_STATE_RUNNING, // Somewhere in the kernel
    DBG_CPR_STATE_QUIESCING,
    DBG_CPR_STATE_ASLEEP,
    DBG_CPR_STATE_CRASHED // Panicked / Crashed
} cp_state_t;

typedef enum {
    DBG_CPR_AP_PANICKED_IN_IBOOT,
    DBG_CPR_AP_PANICKED_IN_DARWIN,
} cp_ap_crash_arg_t;

typedef struct {
    uint64_t timestamp;
    uint32_t cp_state;          // One of the cp_state_t enumerations
    uint32_t cp_state_arg;      // IOP-defined supplemental value
} dbg_cpr_state_entry_t;

#define CPR_MAX_STATE_ENTRIES 16 // Arbitrary value

// This second-level struct should be what the Debug Registry record (e.g. kDbgIdCPRHeaderANS) points to.
typedef struct {
    uint32_t rdptr;
    uint32_t wrptr;
    uint32_t num_cp_state_entries;
    uint32_t checksum;
    dbg_cpr_state_entry_t cp_state_entries[CPR_MAX_STATE_ENTRIES];
} dbg_cpr_t;

/* 
 * Panic String Locator
 *
 * Points to the location / length of the panic string.
*/

#define DbgIdPanicHeaderForIOP(which_dbg_processor, which_num) (DEBUG_RECORD_ID_LONG('P','A','N','I','C',0,which_dbg_processor,which_num))


#define kDbgIdPanicHeaderAP DbgIdPanicHeaderForIOP(DBG_PROCESSOR_AP, 0)


/*
 * Memory Map Hints:
 *
 * Useful regions (such as those defined in platform/memmap.h) start with magic "MAP"
 *
*/

// XNU's PANIC_BASE will be recorded here. This can allow Astris to read the kernel's 
#define kDbgIdXNUPanicRegion (DEBUG_RECORD_ID_LONG('M','A','P','P','A','N','I','C'))
// XNU's physical load address for Astris's use in coredumps
#define kDbgIdXNUKernelRegion (DEBUG_RECORD_ID_LONG('M','A','P','K','E','R','N','L'))

typedef struct {
    dbg_top_level_header_t      top_level_header;
    dbg_record_header_t         records[DEBUG_REGISTRY_MAX_RECORDS];

    // Stuff the AP's Progress Report buffer at the end of this
    // structure. It's currently the only processor that doesn't
    // have some easier form of persistent memory that survives the
    // iBoot->iOS handoff (e.g. ANS has its private heap)
    dbg_cpr_t                     ap_cpr_region;
} dbg_registry_t;


#endif

