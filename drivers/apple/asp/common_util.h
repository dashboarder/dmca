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

#ifndef __common_util_h__
#define __common_util_h__

//#define ASP_ENABLE_NEURALIZE    1

#define ASP_ENABLE_WRITES 0

#if !SUPPORT_FPGA
#define ASP_ENABLE_TIMEOUTS 1
#else
#define ASP_ENABLE_TIMEOUTS 0
#endif

#define ASP_READY_TIMEOUT_US    (90 * 1000 * 1000)
#define ASP_IO_TIMEOUT_US       (90 * 1000 * 1000)


typedef struct
{
    csi_coproc_t        coproc;
    struct task_event   msg_event;
    void *              csi_token;
    void *              asp_command;
    uint32_t            num_lbas;
    uint32_t            bytes_per_lba;
    bool                lba_formatted;
    bool                util_formatted;
    int                 state;
    bool                writable;
    const char *        ep_name;
    uint32_t            lastUserBand;
    uint32_t            lastBand;
    uint32_t            numVirtualSLCBonfireBands;
    uint32_t            firstIntermediateBand;
    uint32_t            lastIntermediateBand;
    uint8_t             chip_id[ANC_MAX_BUSSES][ANC_NAND_ID_SIZE];
    uint8_t             mfg_id[ANC_MAX_BUSSES][ANC_NAND_ID_SIZE];
} asp_t;

enum
{
    ASP_TAG_GENERIC = 0,
    ASP_TAG_NAND,
    ASP_TAG_NVRAM,
    ASP_TAG_FIRMWARE,
    ASP_TAG_LLB,
    ASP_TAG_EFFACEABLE,
    ASP_TAG_SYSCFG,
    ASP_TAG_PANICLOG,
    ASP_NUM_TAGS
};

//
// Definitions
//

enum
{
    ASP_NAND_BLKSZ 			= (4 * 1024),
    ASP_NVRAM_BLKSZ 		= (8 * 1024),
    ASP_FIRMWARE_BLKSZ 		= (4 * 1024),
    ASP_LLB_BLKSZ 			= (4 * 1024),
    ASP_EFFACEABLE_BLKSZ 	= (4 * 1024),
    ASP_SYSCFG_BLKSZ		= (8 * 1024),
    ASP_PANICLOG_BLKSZ		= (4 * 1024),
};


enum
{
    ASP_NAND_NUMBLKS 		= 0,  // For general NAND, size should be looked up from geometry
    ASP_NVRAM_NUMBLKS 		= 1,
    ASP_FIRMWARE_NUMBLKS 	= 2048,
#if defined (ASP_LLB_OVERRIDE_NUM_BLKS)
    ASP_LLB_NUMBLKS 		= ASP_LLB_OVERRIDE_NUM_BLKS,
# else
    ASP_LLB_NUMBLKS             = 253,
#endif
    ASP_EFFACEABLE_NUMBLKS 	= 1,
    ASP_SYSCFG_NUMBLKS		= 18,
    ASP_PANICLOG_NUMBLKS    = 256,
};


enum
{
    ASP_NAND = 1,
    ASP_NVRAM,
    ASP_FIRMWARE,
    ASP_LLB,
    ASP_EFFACEABLE,
    ASP_SYSCFG,
    ASP_PANICLOG,
};


enum
{
    ASP_STATE_INITIALIZING = 0,
    ASP_STATE_INITIALIZED,
    ASP_STATE_READY,
    ASP_STATE_TIMEOUT,
    ASP_STATE_ERROR_NOT_REGISTERED,
    ASP_STATE_ERROR_NOT_WRITABLE,
    ASP_STATE_ERROR_NEURALIZE,
    ASP_STATE_ERROR_INVALID_PPNFW,
    ASP_STATE_ERROR_CRITICAL,
    ASP_STATE_PANIC_RECOVERY,
};


enum
{
    ASP_FORMAT_UTIL,
    ASP_FORMAT_LBA,
    ASP_FORMAT_ALL,
};

#define MAX_SGL_ENTRIES         (512)
//Calculate number of cache lines needed for the command and round them up
#define ASPPROTO_CMD_LINES      ((((MAX_SGL_ENTRIES * sizeof(uint32_t)) + sizeof(aspproto_cmd_t))/CACHELINE_BYTES) + 1)

#define MIN(a,b)                (((a) < (b)) ? (a) : (b))
#define MAX(a,b)                (((a) < (b)) ? (b) : (a))

#define ASP_NAND_BLK_ALIGN_SHIFT (12)
#define ASP_NAND_BLK_ALIGNMENT   (1 << ASP_NAND_BLK_ALIGN_SHIFT)
#define VADDR_TO_PADDR32(buffer) (mem_static_map_physical((addr_t)(buffer)) >> ASP_NAND_BLK_ALIGN_SHIFT)

#define ALIGN_UP(val, n)	((((val) + (n) - 1) / (n)) * (n))

int asp_send_command(uint32_t tag);
int asp_wait_for_completion(uint32_t tag);
bool asp_set_writable(void);
bool asp_get_geometry(void);
bool asp_create_block_device(uint8_t type);
bool asp_set_dies_in_parallel(uint32_t mlc_slc_write_dies, 
                              uint32_t mlc_read_dies,
                              uint32_t mlc_erase_dies,
                              uint32_t mlc_write_dies,
                              uint32_t tlc_slc_write_dies, 
                              uint32_t tlc_read_dies,
                              uint32_t tlc_erase_dies,
                              uint32_t tlc_tlc_write_dies,
                              CorePowerState_e power_level);
bool asp_set_power_state(CorePowerState_e powerState);
bool asp_set_indirection_memory(uint32_t indirection_memory, uint32_t legacy_memory);
bool asp_test_scratchpad(void);
#if !RELEASE_BUILD && WITH_MENU 
int asp_update_ppn_firmware(const void *fw_buffer, size_t fw_length);
#endif // #if !RELEASE_BUILD && WITH_MENU 
aspproto_cmd_t * asp_get_cmd_for_tag(uint32_t tag);
int asp_read_block(struct blockdev *_dev, void *ptr, block_addr block, uint32_t count);
int asp_write_block(struct blockdev *_dev, const void *ptr, block_addr block, uint32_t count);
#if !RELEASE_BUILD
int asp_erase_block(struct blockdev * _dev, off_t offset, uint64_t len);
#endif
bool asp_sync(void);
int asp_type_from_blkdev (struct blockdev *device);
struct blockdev * asp_get_blkdev_for_type (int type);
int asp_set_blkdev_for_type (int type, struct blockdev * device);
bool asp_set_photoflow_mode(core_flow_mode_e slc_mode);
bool asp_enable_bg(void);
bool asp_disable_bg(void);
bool asp_send_open(void);
int asp_panic_recover(void);
bool asp_init_tags(void);
void asp_reinit(void);
bool asp_wait_for_ready(void);
#if defined(ASP_ENABLE_NEURALIZE) && ASP_ENABLE_NEURALIZE
bool asp_neuralize(void);
#endif // ASP_ENABLE_NEURALIZE

#endif // __common_util_h__

