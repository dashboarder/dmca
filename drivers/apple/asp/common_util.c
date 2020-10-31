 /*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#define ASP_TARGET_RTXC 1
#define CASSERT(x, name) typedef char __ASP_CASSERT_##name[(x) ? 1 : -1]
#include <stdint.h>
#include <csi_ipc_protocol.h>
#include <arch.h>
#include <aspcore_mbox.h>
#include <aspcore_protocol.h>
#include <debug.h>
#include <drivers/asp.h>
#include <drivers/csi.h>
#include <lib/env.h>
#include <lib/heap.h>
#include <lib/partition.h>
#include <platform.h>
#include <stdio.h>
#include <sys.h>
#include <sys/menu.h>
#include "common_util.h"
#include <target/aspnandconfig.h>

extern asp_t asp;
extern aspproto_nand_geom_t nand_info;
extern struct blockdev *asp_nand_dev;
extern struct blockdev *asp_nvram_dev;
extern struct blockdev *asp_firmware_dev;
extern struct blockdev *asp_llb_dev;
extern struct blockdev *asp_effaceable_dev;
extern struct blockdev *asp_syscfg_dev;
extern struct blockdev *asp_paniclog_dev;

char *asp_names[] = {"unknown", "NAND", "NVRAM", "FIRMWARE", "LLB", "EFFACEABLE", "SYSCFG", "PANICLOG"};


aspproto_cmd_t * asp_get_cmd_for_tag(uint32_t tag)
{

	uint8_t * base = (uint8_t *) asp.asp_command;
	uint8_t * offset = NULL;

	if (tag > ASP_NUM_TAGS)
	{
		panic("Invalid tag %d given to asp_get_cmd_for_tag!\n", tag);
	}

	offset = base + (tag * ASPPROTO_CMD_LINES * CACHELINE_BYTES);

    memset(offset, 0x00, ASPPROTO_CMD_LINES * CACHELINE_BYTES);
    ((aspproto_cmd_t *)offset)->tag = tag;

	return ((aspproto_cmd_t *)offset);

}

#if WITH_NON_COHERENT_DMA

int asp_send_command(uint32_t tag)
{
    uint32_t  cmd_mem_size;
    void      *cmd_mem_base;
    int       return_val;

    cmd_mem_size = ASPPROTO_CMD_LINES * CACHELINE_BYTES;
    cmd_mem_base = (void*)((uintptr_t)asp.asp_command + (tag * cmd_mem_size));

    platform_cache_operation(CACHE_CLEAN, cmd_mem_base, cmd_mem_size);

    if (csi_send_message(asp.csi_token, ASPMBOX_MAKE_CMD_NEW1(tag)) != CSI_STATUS_OK)
    {
        return ASPPROTO_CMD_STATUS_TIMEOUT;
    }

    return_val = asp_wait_for_completion(tag);
    platform_cache_operation(CACHE_INVALIDATE, cmd_mem_base, cmd_mem_size);

    return return_val;
}

#else

int asp_send_command(uint32_t tag)
{
    uint64_t  payload;

    payload = ASPMBOX_MAKE_CMD_NEW1(tag);

    if (csi_send_message(asp.csi_token, payload) != CSI_STATUS_OK)
    {
        return ASPPROTO_CMD_STATUS_TIMEOUT;
    }

    return asp_wait_for_completion(tag);
}
#endif


int asp_wait_for_completion(uint32_t tag)
{

    msg_payload_t msg;
    uint32_t      rx_type;
    uint32_t      rx_tag;
    uint32_t      status;
    csi_status_t  result;

again:

#if ASP_ENABLE_TIMEOUTS
    if (!event_wait_timeout(&asp.msg_event, ASP_IO_TIMEOUT_US))
    {
        dprintf(DEBUG_CRITICAL, "ASP - I/O timed out for tag %d!", tag);
        asp.state = ASP_STATE_TIMEOUT;
        return ASPPROTO_CMD_STATUS_TIMEOUT;
    }
#else
    event_wait(&asp.msg_event);
#endif

    result = csi_receive_message(asp.csi_token, &msg);

    if (result != CSI_STATUS_OK)
    {
        // REVISIT - Change this after defining ASPPROTO_CMD_STATUS_ERROR
        return -1;
    }

    rx_type = ASPMBOX_GET_TYPE(msg);

    switch (rx_type)
    {
        case ASPMBOX_TYPE_CMD_FIN :
        {
            rx_tag = ASPMBOX_FIN_GET_TAG(msg);

            if (tag != rx_tag)
            {
                // REVISIT - Change this after defining ASPPROTO_CMD_STATUS_ERROR
                status = -1;
            }
            else
            {
                status = ASPMBOX_FIN_GET_ERRCODE(msg);
            }

            break;
        }

        case ASPMBOX_TYPE_CMD_UNSOLICITED :
        {
            // ignore and try again
            goto again;
            break;
        }

        default :
        {
            dprintf(DEBUG_CRITICAL, "Invalid message type  %d received", rx_type);
            status = -1;
        }
    }

    return status;

}

bool asp_set_writable(void)
{

    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op  = ASPPROTO_CMD_SETWRITABLE;

    asp.writable = false;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {

        dprintf(DEBUG_CRITICAL, "ASP setWritable Failed\n");
        return false;

    }

    asp.writable = true;
    printf("******IMPORTANT NOTE: IBOOT CAN NOW WRITE TO NAND AND POTENTIALLY CHANGE STATE OF THE DRIVE\n");
    printf("                      REBOOT IF THIS IS NOT WHAT YOU INTENDED\n");

    return true;

}

bool asp_get_geometry(void)
{

    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op  = ASPPROTO_CMD_GET_GEOM;

    while ( 1 )
    {
        if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
        {
            dprintf(DEBUG_CRITICAL, "Unable to read ASP geometry\n");
            return false;
        }

        if ( command->geom.pendingLoad != 0 )
        {
            dprintf(DEBUG_CRITICAL, "ASP: Pending load is set in geometry. Will retry in 50ms\n" );
            task_sleep ( 50 * 1000 ); // sleep for 50mS.
        }

        else
        {
            break;
        }
    }

    asp.num_lbas                  = command->geom.num_lbas;
    asp.bytes_per_lba             = command->geom.bytes_per_lba;
    asp.lastUserBand              = command->geom.lastUserBand;
    asp.lastBand                  = command->geom.lastBand;
    asp.numVirtualSLCBonfireBands = command->geom.numVirtualSLCBonfireBands;
    asp.firstIntermediateBand     = command->geom.firstIntermediateBand;
    asp.lastIntermediateBand      = command->geom.lastIntermediateBand;
    if (command->geom.utilFormatted)    asp.util_formatted  = true;
    if (command->geom.lbaFormatted)     asp.lba_formatted   = true;

    memcpy(asp.chip_id, command->geom.chip_id, ANC_MAX_BUSSES * ANC_NAND_ID_SIZE);
    memcpy(asp.mfg_id, command->geom.mfg_id, ANC_MAX_BUSSES * ANC_NAND_ID_SIZE);
    dprintf(DEBUG_CRITICAL, "ASP Block Device %d lbas, %d bytes per lba, utilFormatted:%d lbaFormatted:%d\n",
        asp.num_lbas, asp.bytes_per_lba, asp.util_formatted, asp.lba_formatted);

    if (command->geom.total_user_lbas == 0)
        return false;   //asp init failed, don't publish a block device

    if (asp.state != ASP_STATE_READY) asp.state = ASP_STATE_INITIALIZED;

    return true;

}

bool asp_create_block_device(uint8_t type)
{

    struct blockdev *   blkdevice;
    uint32_t result;

    if (((asp.util_formatted == false) && (type!=ASP_NVRAM)) ||
    	((asp.lba_formatted == false) && (type==ASP_NAND)))
    {
        dprintf(DEBUG_CRITICAL, "Cannot create blockdevice(%d) as media is not formatted!\n",type);
        return false;
    }

    blkdevice = calloc(1, sizeof(struct blockdev));

    if (!blkdevice)
    {
        dprintf(DEBUG_CRITICAL, "Unable to allocate blockdev\n");
        return false;
    }

    switch (type)
    {

        case ASP_NAND:
            result = construct_blockdev(blkdevice, "asp_nand", (uint64_t)asp.num_lbas * asp.bytes_per_lba, asp.bytes_per_lba);
        break;

        case ASP_NVRAM:
            result = construct_blockdev(blkdevice, "nvram", (uint64_t)ASP_NVRAM_NUMBLKS * ASP_NVRAM_BLKSZ, ASP_NVRAM_BLKSZ);
        break;

        case ASP_FIRMWARE:
            result = construct_blockdev(blkdevice, "asp_fw", (uint64_t)ASP_FIRMWARE_NUMBLKS * ASP_FIRMWARE_BLKSZ, ASP_FIRMWARE_BLKSZ);
        break;

        case ASP_LLB:
            result = construct_blockdev(blkdevice, "asp_llb", (uint64_t)ASP_LLB_NUMBLKS * ASP_LLB_BLKSZ, ASP_LLB_BLKSZ);
        break;

        case ASP_EFFACEABLE:
            result = construct_blockdev(blkdevice, "asp_effaceable", (uint64_t)ASP_EFFACEABLE_NUMBLKS * ASP_EFFACEABLE_BLKSZ, ASP_EFFACEABLE_BLKSZ);
        break;

        case ASP_SYSCFG:
            result = construct_blockdev(blkdevice, "nand_syscfg", (uint64_t)ASP_SYSCFG_NUMBLKS * ASP_SYSCFG_BLKSZ, ASP_SYSCFG_BLKSZ);
        break;

        case ASP_PANICLOG:
            result = construct_blockdev(blkdevice, "paniclog", (uint64_t)ASP_PANICLOG_NUMBLKS * ASP_PANICLOG_BLKSZ, ASP_PANICLOG_BLKSZ);
            break;

        default:
            result = -1;
        break;

    }

    if (result != 0)
    {
        dprintf(DEBUG_CRITICAL, "Failed to contruct blockdev\n");
        free(blkdevice);
        return false;
    }

    blkdevice->read_block_hook  = &asp_read_block;

    if (type == ASP_NVRAM)
    {
        blkdevice->write_block_hook = &asp_write_block;
    }

#if !RELEASE_BUILD
    if (type == ASP_LLB)
    {
        blkdevice->erase_hook = &asp_erase_block;
    }
#endif

    if (type == ASP_PANICLOG)
    {
        blkdevice->write_block_hook = &asp_write_block;
    }

#if ASP_ENABLE_WRITES
    else
    {
        blkdevice->write_block_hook = &asp_write_block;

    }
#endif

    if (register_blockdev(blkdevice) != 0)
    {
        dprintf(DEBUG_CRITICAL, "Failed to register blockdev\n");
        free(blkdevice);
        return false;
    }

    asp_set_blkdev_for_type(type, blkdevice);

    return true;

}

bool asp_set_photoflow_mode(core_flow_mode_e slc_mode)
{

    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op  = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode = CORE_DEBUG_SET_PHOTOFLOW_MODE;
    command->tunnel.options.value = (uint32_t) slc_mode;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Unable to asp_enable_slc\n");
        return false;
    }
    return true;
}

bool asp_enable_bg(void)
{

    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op  = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode = CORE_DEBUG_ENABLE_BG;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Unable to asp_enable_bg\n");
        return false;
    }
    return true;
}

bool asp_disable_bg(void)
{

    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op  = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode = CORE_DEBUG_DISABLE_BG;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Unable to asp_disable_bg\n");
        return false;
    }
    return true;
}

bool asp_set_dies_in_parallel(uint32_t mlc_slc_write_dies,
                              uint32_t mlc_read_dies,
                              uint32_t mlc_erase_dies,
                              uint32_t mlc_mlc_write_dies,
                              uint32_t tlc_slc_write_dies,
                              uint32_t tlc_read_dies,
                              uint32_t tlc_erase_dies,
                              uint32_t tlc_tlc_write_dies,
                              CorePowerState_e power_level)
{
    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    if (nand_info.num_bus==0)
    {
        return false;
    }

    if (CORE_POWERSTATE_HIGH_POWER == power_level) {
        command->tunnel.core_opcode = CORE_DEBUG_SET_MAX_DIES_IN_PARALLEL;
        dprintf(DEBUG_SPEW, "setting for MLC: (SLC_WRITE=%d,READ=%d,ERASE=%d,MLC_WRITE=%d) high power parallel per channel\n",
                mlc_slc_write_dies / nand_info.num_bus, 
                mlc_read_dies / nand_info.num_bus, 
                mlc_erase_dies / nand_info.num_bus, 
                mlc_mlc_write_dies / nand_info.num_bus);

        dprintf(DEBUG_SPEW, "setting for TLC: (SLC_WRITE=%d,READ=%d,ERASE=%d,TLC_WRITE=%d) high power parallel per channel\n",
                tlc_slc_write_dies / nand_info.num_bus, 
                tlc_read_dies / nand_info.num_bus, 
                tlc_erase_dies / nand_info.num_bus, 
                tlc_tlc_write_dies / nand_info.num_bus);
    } else if (CORE_POWERSTATE_LOW_POWER == power_level) {
        command->tunnel.core_opcode = CORE_DEBUG_SET_LOW_POWER_MAX_DIES_IN_PARALLEL;
        dprintf(DEBUG_SPEW, "setting for MLC: (SLC_WRITE=%d,READ=%d,ERASE=%d,MLC_WRITE=%d) low power parallel per channel\n",
                mlc_slc_write_dies / nand_info.num_bus, 
                mlc_read_dies / nand_info.num_bus, 
                mlc_erase_dies / nand_info.num_bus, 
                mlc_mlc_write_dies / nand_info.num_bus);

        dprintf(DEBUG_SPEW, "setting for TLC: (SLC_WRITE=%d,READ=%d,ERASE=%d,TLC_WRITE=%d) low power parallel per channel\n",
                tlc_slc_write_dies / nand_info.num_bus, 
                tlc_read_dies / nand_info.num_bus, 
                tlc_erase_dies / nand_info.num_bus, 
                tlc_tlc_write_dies / nand_info.num_bus);
    } else {
        dprintf(DEBUG_CRITICAL, "Powerlevel 0x%x is not configurable\n", power_level);
        return false;
    }
    command->op                                                 = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.buffer_paddr                                = 0;
    command->tunnel.bufferLen                                   = 0;
    command->tunnel.options.mask                                = (1 << ANC_MAX_BUSSES) - 1;
    command->tunnel.options.dies_in_parallel.mlc_mlc_write_dies = mlc_mlc_write_dies / nand_info.num_bus;
    command->tunnel.options.dies_in_parallel.mlc_slc_write_dies = mlc_slc_write_dies / nand_info.num_bus;
    command->tunnel.options.dies_in_parallel.mlc_erase_dies     = mlc_erase_dies / nand_info.num_bus;
    command->tunnel.options.dies_in_parallel.mlc_read_dies      = mlc_read_dies / nand_info.num_bus;
    command->tunnel.options.dies_in_parallel.tlc_tlc_write_dies = tlc_tlc_write_dies / nand_info.num_bus;
    command->tunnel.options.dies_in_parallel.tlc_slc_write_dies = tlc_slc_write_dies / nand_info.num_bus;
    command->tunnel.options.dies_in_parallel.tlc_erase_dies     = tlc_erase_dies / nand_info.num_bus;
    command->tunnel.options.dies_in_parallel.tlc_read_dies      = tlc_read_dies / nand_info.num_bus;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Unable to set dies in parallel\n");
        return false;
    }
    return true;
}

bool asp_set_power_state(CorePowerState_e powerState) {
    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op  = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode = CORE_DEBUG_SET_POWER_STATE;
    command->tunnel.options.value = powerState;

    dprintf(DEBUG_CRITICAL, "setting asp to high power mode\n");
    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "setting asp to high power failed\n");
        return false;
    }
    return true;
}

bool asp_set_indirection_memory(uint32_t indirection_memory, uint32_t legacy_memory) {
    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op  = ASPPROTO_CMD_CORE_DEBUG;
    command->tunnel.core_opcode = CORE_DEBUG_SET_INDIRECTION_MEMORY;
    command->tunnel.options.indirection_memory.heapIndMemory = indirection_memory;
    command->tunnel.options.indirection_memory.legacyIndMemory = legacy_memory;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "setting indirection memory failed\n");
        return false;
    }
    return true;
}

bool asp_test_scratchpad(void)
{
    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op  = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode = NAND_DEBUG_TEST_SCRATCHPAD;
    command->tunnel.options.mask = (1<<ANC_MAX_BUSSES)-1;

    dprintf(DEBUG_CRITICAL, "testing scratchpad\n");
    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "scratchpad failed\n");
        return false;
    }
    return true;
}

#if !RELEASE_BUILD && WITH_MENU
int asp_update_ppn_firmware(const void *fw_buffer, size_t fw_length)
{
    int err = 0;
    void *fw_buffer_aligned = NULL;

    asp_disable_bg();

    //allocate buffer
    fw_buffer_aligned = memalign(fw_length, 4096);
    if(!fw_buffer_aligned)
    {
        dprintf(DEBUG_CRITICAL, "Unable to allocate data_buffer\n");
        return -1;
    }
    memcpy(fw_buffer_aligned, fw_buffer, fw_length);

    //set up the tunnel command
    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op                     = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode          = NAND_DEBUG_PPN_FIRMWARE_UPDATE;
    command->tunnel.buffer_paddr    = VADDR_TO_PADDR32(fw_buffer_aligned);
    command->tunnel.bufferLen       = fw_length;
    command->tunnel.options.mask    = (1 << ANC_MAX_BUSSES) - 1;
    command->tunnel.options.value   = 0;

    dprintf(DEBUG_CRITICAL, "Begin PPN firmware update\n");

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "ERROR: PPN firmware update failed\n\n");
        asp.state = ASP_STATE_ERROR_INVALID_PPNFW;
        err = -1;
    }
    else
    {
        if (asp.state == ASP_STATE_ERROR_INVALID_PPNFW)
        {
            dprintf(DEBUG_CRITICAL, "PPN firmware successfully recovered, reboot required\n\n");
        }
        else
        {
            dprintf(DEBUG_CRITICAL, "PPN firmware update succeeded\n\n");
        }
        asp_set_default_dies_in_parallel();
    }

    asp_enable_bg();

    free(fw_buffer_aligned);
    return err;
}
#endif // #if !RELEASE_BUILD && WITH_MENU

#if !RELEASE_BUILD
int asp_erase_block(struct blockdev * _dev, off_t offset, uint64_t len)
{
    block_addr start_block = 0;
    uint32_t num_blocks = 0, returned_blocks = 0, blk_size = 0;
    uint8_t *data_buffer = NULL;
    int type = -1;

    if(len == 0)
    {
        len = _dev->total_len;
        printf("len = 0 will erase entire dev.\n");
    }
    if((offset + len) > _dev->total_len)
    {
        printf("Invalid parameter. Size should be > 0 and (Size + Offset) should be < %lld\n", _dev->total_len);
        return -1;
    }

    type = asp_type_from_blkdev(_dev);
    switch (type)
    {
        case ASP_LLB:
            blk_size = ASP_LLB_BLKSZ;
        break;

#if ASP_ENABLE_WRITES
        case ASP_NVRAM:
            blk_size = ASP_NVRAM_BLKSZ;
        break;

        case ASP_NAND:
            blk_size = ASP_NAND_BLKSZ;
        break;

        case ASP_FIRMWARE:
            blk_size = ASP_FIRMWARE_BLKSZ;
        break;

        case ASP_EFFACEABLE:
            blk_size = ASP_EFFACEABLE_BLKSZ;
        break;

        case ASP_SYSCFG:
            blk_size = ASP_SYSCFG_BLKSZ;
        break;
#endif // ASP_ENABLE_WRITES

        default:
            dprintf(DEBUG_CRITICAL, "Wrong type %d for asp_erase_block!\n", type);
            return -1;
        break;
    }

    if((0 != (len % blk_size)) || (0 != (offset % blk_size)))
    {
        printf("len and offset are expected to be multiple of block sizez: %d\n", blk_size);
        return -1;
    }
    start_block = offset / blk_size;
    num_blocks = len / blk_size;

    data_buffer = memalign(num_blocks * blk_size, 4096);
    if(!data_buffer)
    {
        printf("Could not allocate data buffer\n");
        return -1;
    }
    //erasing is actually writing out 0's.
    memset(data_buffer, 0x00, num_blocks * blk_size);

    returned_blocks = asp_write_block(_dev, data_buffer, start_block, num_blocks);

    if (returned_blocks != num_blocks)
    {
        printf("Error. Expected to erase %d blocks. Erased %d blocks\n", num_blocks, returned_blocks);
        free(data_buffer);
        return -1;
    }

    free(data_buffer);
    return len;
}
#endif


int asp_read_block(struct blockdev *_dev, void *ptr, block_addr block, uint32_t count)
{

    uintptr_t           paddr  = mem_static_map_physical((addr_t)ptr);
    uint32_t            paddr32 = paddr >> ASP_NAND_BLK_ALIGN_SHIFT;
    int                 type = -1;
    aspproto_cmd_t *    command;
    uint8_t *           bounce_buffer = NULL;
    uint32_t            num_blks_allowed;
    uint32_t            blk_size;
    int                 total_count;
    int                 return_count;
    void                *vaddr;

    type = asp_type_from_blkdev(_dev);
    if (asp.state != ASP_STATE_READY && type != ASP_NVRAM)
    {
        dprintf(DEBUG_CRITICAL, "ASP is not ready! Current state is %d\n", asp.state);
        return_count = -1;
        goto ReadBlockCleanUp;
    }
    else if (csi_is_panic_recovery(asp.coproc))
    {
        asp_panic_recover();
        dprintf(DEBUG_CRITICAL, "ASP is panicked; ignoring read\n");
        return_count = -1;
        goto ReadBlockCleanUp;
    }

    vaddr = ptr;

    switch (type)
    {

        case ASP_NAND:
            command = asp_get_cmd_for_tag(ASP_TAG_NAND);
            command->op = ASPPROTO_CMD_READ;
            num_blks_allowed = asp.num_lbas;
            blk_size = ASP_NAND_BLKSZ;
        break;

        case ASP_NVRAM:
            command = asp_get_cmd_for_tag(ASP_TAG_NVRAM);
            command->op = ASPPROTO_CMD_READ_NVRAM;
            num_blks_allowed = ASP_NVRAM_NUMBLKS;
            blk_size = ASP_NVRAM_BLKSZ;
        break;

        case ASP_FIRMWARE:
            command = asp_get_cmd_for_tag(ASP_TAG_FIRMWARE);
            command->op = ASPPROTO_CMD_READ_FW;
            num_blks_allowed = ASP_FIRMWARE_NUMBLKS;
            blk_size = ASP_FIRMWARE_BLKSZ;
        break;

        case ASP_LLB:
            command = asp_get_cmd_for_tag(ASP_TAG_LLB);
            command->op = ASPPROTO_CMD_READ_LLB;
            num_blks_allowed = ASP_LLB_NUMBLKS;
            blk_size = ASP_LLB_BLKSZ;
        break;

        case ASP_EFFACEABLE:
            command = asp_get_cmd_for_tag(ASP_TAG_EFFACEABLE);
            command->op = ASPPROTO_CMD_READ_EFFACEABLE;
            num_blks_allowed = ASP_EFFACEABLE_NUMBLKS;
            blk_size = ASP_EFFACEABLE_BLKSZ;
        break;

        case ASP_SYSCFG:
            command = asp_get_cmd_for_tag(ASP_TAG_SYSCFG);
            command->op = ASPPROTO_CMD_READ_SYSCFG;
            num_blks_allowed = ASP_SYSCFG_NUMBLKS;
            blk_size = ASP_SYSCFG_BLKSZ;
            // syscfg starts at block 2
            if (block != 2)
            {
                dprintf(DEBUG_CRITICAL, "Attempted to read nand syscfg at non-zero offset\n");
                return_count = -1;
                goto ReadBlockCleanUp;
            }
            block = 0; // Real Syscfg is always the 1st block in the blockdevice
        break;

        case ASP_PANICLOG:
            command = asp_get_cmd_for_tag(ASP_TAG_PANICLOG);
            command->op = ASPPROTO_CMD_READ_PANICLOG;
            num_blks_allowed = ASP_PANICLOG_NUMBLKS;
            blk_size = ASP_PANICLOG_BLKSZ;

            if(block != 0) {
                return_count = -1;
                goto ReadBlockCleanUp;
            }
            break;

        default:
            dprintf(DEBUG_CRITICAL, "Wrong type %d for asp_read_block!\n", type);
            return_count = -1;
            goto ReadBlockCleanUp;
        break;

    }

    if ((paddr & (4096 - 1)) != 0)
    {
        bounce_buffer = memalign(count * blk_size, 4096);

        if (!bounce_buffer)
        {
            dprintf(DEBUG_CRITICAL, "Failed to allocate bounce buffer\n");
            return_count = -1;
            goto ReadBlockCleanUp;
        }

        paddr32 = VADDR_TO_PADDR32(bounce_buffer);
        vaddr   = bounce_buffer;
    }

    command->flags.all = 0;
    command->flags.noAesKey = 1;

    if (count > num_blks_allowed)
    {
    	dprintf(DEBUG_CRITICAL, "Trimming supported read size of %d 4KB blks to "
    			"%d 4KB blocks for NAND dev type %d\n",	count, num_blks_allowed, type);
    	count = num_blks_allowed;
    }

    total_count = count;

    if (type == ASP_NVRAM || type == ASP_SYSCFG)
    {
        count = count * 2; //Since NVRAM and SYSCFG blkdev publishes 8KB while NAND blksz is 4KB
    }

#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, vaddr, count * ASP_NAND_BLKSZ);
#endif

    if (type != ASP_PANICLOG)
    {
        return_count = total_count;
    }
    else
    {
        return_count = 0;
    }

    while (count)
    {
        uint32_t lbas = MIN(count, MAX_SGL_ENTRIES);
        uint32_t lba;
        int ret;
        command->lba = block;

        for (lba = 0; lba < lbas; lba++)
        {
            command->sglAndIv[lba] = paddr32++;
        }

        command->count = lbas;
        count -= lbas;
        block += lbas;

        if (type == ASP_NVRAM && !asp.util_formatted)
        {
            ; // ignore writes
        }
        else if ((ret=asp_send_command(command->tag)) != ASPPROTO_CMD_STATUS_SUCCESS)
        {
            if (type == ASP_NVRAM || type == ASP_SYSCFG) // failed NVRAM reads return all FFs, not errors
            {
                memset(ptr, 0xff, total_count * blk_size);
            }
            else
            {
                dprintf(DEBUG_CRITICAL, "Failed to read LBA %d from bdev %s, return code=%d\n", command->lba, asp_names[type],ret);
                return_count = -1;
                goto ReadBlockCleanUp;
            }
        }
        else if (type == ASP_PANICLOG)
        {
            return_count += command->count;
        }
   }

#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_INVALIDATE, vaddr, total_count * ASP_NAND_BLKSZ);
#endif

    if (type == ASP_NVRAM && !asp.util_formatted)
    {
        // fake reads return all 0xffs
        memset(ptr, 0xff, total_count * blk_size);
    }

ReadBlockCleanUp:

    if (bounce_buffer)
    {
        // Buffer wasn't 4KB-aligned
        memcpy(ptr, bounce_buffer, total_count * blk_size);
        free(bounce_buffer);
    }

    return return_count;

}


int asp_write_block(struct blockdev *_dev, const void *ptr, block_addr block, uint32_t count)
{

    uintptr_t paddr   = mem_static_map_physical((addr_t)ptr);
    uint32_t  paddr32 = paddr >> ASP_NAND_BLK_ALIGN_SHIFT;
    int       total_count;
    int type = -1;
    aspproto_cmd_t * command;
    uint32_t num_blks_allowed;
    uint32_t blk_size;
	uint8_t *bounce_buffer = NULL;
    void    *vaddr = (void *)ptr;

    type = asp_type_from_blkdev(_dev);
    if (asp.state != ASP_STATE_READY && type != ASP_NVRAM)
    {
        dprintf(DEBUG_CRITICAL, "ASP is not ready! Current state is %d\n", asp.state);
    	return -1;
    }
    else if (csi_is_panic_recovery(asp.coproc))
    {
        asp_panic_recover();

        dprintf(DEBUG_CRITICAL, "ASP is panicked; ignoring read\n");
        return -1;
    }

    switch (type)
    {

        case ASP_NVRAM:
            command = asp_get_cmd_for_tag(ASP_TAG_NVRAM);
            command->op = ASPPROTO_CMD_WRITE_NVRAM;
            num_blks_allowed = ASP_NVRAM_NUMBLKS;
            blk_size = ASP_NVRAM_BLKSZ;
        break;

        case ASP_LLB:
            command = asp_get_cmd_for_tag(ASP_TAG_LLB);
            command->op = ASPPROTO_CMD_WRITE_LLB;
            num_blks_allowed = ASP_LLB_NUMBLKS;
            blk_size = ASP_LLB_BLKSZ;
        break;

#if ASP_ENABLE_WRITES
        case ASP_NAND:
            command = asp_get_cmd_for_tag(ASP_TAG_NAND);
            command->op = ASPPROTO_CMD_WRITE;
            num_blks_allowed = asp.num_lbas;
            blk_size = ASP_NAND_BLKSZ;
        break;

        case ASP_FIRMWARE:
            command = asp_get_cmd_for_tag(ASP_TAG_FIRMWARE);
            command->op = ASPPROTO_CMD_WRITE_FW;
            num_blks_allowed = ASP_FIRMWARE_NUMBLKS;
            blk_size = ASP_FIRMWARE_BLKSZ;
        break;

        case ASP_EFFACEABLE:
            command = asp_get_cmd_for_tag(ASP_TAG_EFFACEABLE);
            command->op = ASPPROTO_CMD_WRITE_EFFACEABLE;
            num_blks_allowed = ASP_EFFACEABLE_NUMBLKS;
            blk_size = ASP_EFFACEABLE_BLKSZ;
        break;
#endif
        case ASP_PANICLOG:
            command = asp_get_cmd_for_tag(ASP_TAG_PANICLOG);
            command->op = ASPPROTO_CMD_WRITE_PANICLOG;
            num_blks_allowed = ASP_PANICLOG_NUMBLKS;
            blk_size = ASP_PANICLOG_BLKSZ;
            break;

        default:
            dprintf(DEBUG_CRITICAL, "Unsupported type %d for asp_write_block!\n", type);
            return -1;
        break;

    }

#if ASP_ENABLE_WRITES
    if ((type != ASP_NVRAM)
        && (asp.writable == false))
    {
        printf("Need to make system writable. Execute 'asp setwritable' first\n");
        return -1;
    }
#endif

    if ((paddr & (4096 - 1)) != 0)
    {
        bounce_buffer = memalign(count * blk_size, 4096);

        if (!bounce_buffer)
        {
            dprintf(DEBUG_CRITICAL, "Failed to allocate bounce buffer\n");
            return -1;
        }

        paddr32 = VADDR_TO_PADDR32(bounce_buffer);
        memcpy(bounce_buffer, ptr, count * blk_size);
        vaddr = bounce_buffer;
    }

#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_CLEAN, vaddr, count * blk_size);
#endif

    command->flags.all = 0;
    command->flags.noAesKey = 1;

    if (count > num_blks_allowed)
    {
    	dprintf(DEBUG_CRITICAL, "Trimming supported read size of %d 4KB blks to "
    			"%d 4KB blocks for NAND dev type %d\n",	count, num_blks_allowed, type);
    	count = num_blks_allowed;
    }

    total_count = count;

    if (type == ASP_NVRAM)
    {
        count = count * 2; //Since NVRAM blkdev publishes 8KB while NAND blksz is 4KB
    }

    while (count)
    {
        uint32_t lbas = MIN(count, MAX_SGL_ENTRIES);
        uint32_t lba;
        command->lba = block;

        for (lba = 0; lba < lbas; lba++)
        {
            command->sglAndIv[lba] = paddr32++;
        }

        command->count = lbas;
        count -= lbas;
        block += lbas;

        if (type == ASP_NVRAM && !asp.util_formatted)
        {
            ; // ignore writes
        }
        else if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
        {
            dprintf(DEBUG_CRITICAL, "Failed to write LBA %d\n", block);
            return -1;
        }
    }

#if WITH_NON_COHERENT_DMA
    platform_cache_operation(CACHE_INVALIDATE, vaddr, total_count * blk_size);
#endif

    if (bounce_buffer)
    {
        // Buffer wasn't 4KB-aligned
        free(bounce_buffer);
    }

    return total_count;

}


bool asp_sync(void)
{

    uint32_t status;
    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op  = ASPPROTO_CMD_SHUTDOWNNOTIFY;

    status = asp_send_command(command->tag);

    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to sync: 0x%08x\n", status);
        return false;
    }

    dprintf(DEBUG_INFO, "ASP sync complete\n");
    return true;

}


#if defined(ASP_ENABLE_NEURALIZE) && ASP_ENABLE_NEURALIZE
bool asp_neuralize(void)
{

    uint32_t status;
    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op  = ASPPROTO_CMD_NEURALIZE;

    status = asp_send_command(command->tag);

    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to neuralize: 0x%08x\n", status);
        return false;
    }

    dprintf(DEBUG_INFO, "ASP neuralize complete\n");
    return true;
}
#endif


int asp_type_from_blkdev (struct blockdev *device)
{

    int type = -1;

    if (device == asp_nand_dev)
    {
        type = ASP_NAND;
    }

	else if (device == asp_nvram_dev)
    {
        type = ASP_NVRAM;
    }

    else if (device == asp_firmware_dev)
    {
        type = ASP_FIRMWARE;
    }

    else if (device == asp_llb_dev)
    {
        type = ASP_LLB;
    }

    else if (device == asp_effaceable_dev)
    {
        type = ASP_EFFACEABLE;
    }

    else if (device == asp_syscfg_dev)
    {
        type = ASP_SYSCFG;
    }

    else if (device == asp_paniclog_dev)
    {
        type = ASP_PANICLOG;
    }

    else
    {
        dprintf(DEBUG_CRITICAL, "Wrong type %d for asp_type_from_blkdev!\n", type);
        type = -1;
    }

    return type;

}


struct blockdev * asp_get_blkdev_for_type (int type)
{

    struct blockdev * device = NULL;

    switch (type)
    {

        case ASP_NAND:
            device = asp_nand_dev;
        break;

		case ASP_NVRAM:
			device = asp_nvram_dev;
		break;

        case ASP_FIRMWARE:
            device = asp_firmware_dev;
        break;

        case ASP_LLB:
            device = asp_llb_dev;
        break;

        case ASP_EFFACEABLE:
            device = asp_effaceable_dev;
        break;

        case ASP_SYSCFG:
            device = asp_syscfg_dev;
        break;

        case ASP_PANICLOG:
            device = asp_paniclog_dev;
        break;

        default:
            dprintf(DEBUG_CRITICAL, "Wrong type %d for asp_blktype_from_dev!\n", type);
            device = NULL;
            break;

    }

    return device;

}


int asp_set_blkdev_for_type (int type, struct blockdev * device)
{

    switch (type)
    {

        case ASP_NAND:
            asp_nand_dev = device;
        break;

		case ASP_NVRAM:
			asp_nvram_dev = device;
		break;

        case ASP_FIRMWARE:
            asp_firmware_dev = device;
        break;

        case ASP_LLB:
            asp_llb_dev = device;
        break;

        case ASP_EFFACEABLE:
            asp_effaceable_dev = device;
        break;

        case ASP_SYSCFG:
            asp_syscfg_dev = device;
        break;

        case ASP_PANICLOG:
            asp_paniclog_dev = device;
        break;

        default:
            dprintf(DEBUG_CRITICAL, "Wrong type %d for asp_set_blkdev_for_type!\n", type);
            return -1;

    }

    return 0;

}

int asp_panic_recover(void)
{
    if (asp.state != ASP_STATE_PANIC_RECOVERY)
    {
        asp.state = ASP_STATE_PANIC_RECOVERY;
        dprintf(DEBUG_CRITICAL, "Initializing tags\n");
        if (!asp_init_tags())
        {
            dprintf(DEBUG_CRITICAL, "Unable to init tags\n");
            return -1;
        }

        asp_reinit();

        dprintf(DEBUG_CRITICAL, "Opening ASP\n");
        if (!asp_send_open())
        {
            dprintf(DEBUG_CRITICAL, "Unable to open ASP\n");
            return -1;
        }
    }
    return 0;
}

void asp_reinit(void) {

    asp_set_indirection_memory(ASPNAND_INDIRECTION_MEMORY, ASPNAND_LEGACY_INDIRECTION_MEMORY);
    asp_set_default_dies_in_parallel();
}
