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
#include <csi_platform_defines.h>
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
#include <target/aspnandconfig.h>
#if !SUPPORT_FPGA
#include <target/powerconfig.h>
#endif
#include "common_util.h"


//
// Defines
//
#define MY_ASP_PROTOCOL_VERSION 10
CASSERT(ASP_PROTOCOL_VERSION == MY_ASP_PROTOCOL_VERSION, protocol_versions_match);

//
// Prototypes
//

static bool asp_get_nand_info(void);

//
// Globals
//

asp_t asp;
aspproto_nand_geom_t nand_info;
struct blockdev *asp_nand_dev;
struct blockdev *asp_nvram_dev;
struct blockdev *asp_firmware_dev;
struct blockdev *asp_llb_dev;
struct blockdev *asp_effaceable_dev;
struct blockdev *asp_syscfg_dev;
struct blockdev *asp_paniclog_dev;

static int asp_set_atv(void)
{
    aspproto_cmd_t *    command;

    command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    command->op = ASPPROTO_CMD_SETOPTIONS;
    command->opts.isATV = true;

    if ((asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)){
        dprintf(DEBUG_CRITICAL, "Unable to set atv mode\n");
        return -1;
    }
    return 0;
}

int asp_init(void)
{

    csi_status_t result;

    asp.state           = ASP_STATE_INITIALIZING;
    asp.writable        = false;
    asp_nand_dev        = NULL;
	asp_nvram_dev		= NULL;
    asp_firmware_dev    = NULL;
    asp_llb_dev         = NULL;
    asp_effaceable_dev  = NULL;
    asp_syscfg_dev      = NULL;
    asp_paniclog_dev    = NULL;


    event_init(&asp.msg_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

    result = csi_register_endpoint(CSI_COPROC_ANS, IPC_ENDPOINT_ASP, &asp.msg_event, &asp.csi_token, &asp.ep_name);

    if (result != CSI_STATUS_OK)
    {
        asp.state = ASP_STATE_ERROR_CRITICAL;
        dprintf(DEBUG_CRITICAL, "Failed to register ASP endpoint");
        return -1;
    }

#if !ASP_ENABLE_TIMEOUTS
    dprintf(DEBUG_CRITICAL, "ASP Timeouts disabled!\n");
#endif

    if (!asp_wait_for_ready())
    {
        asp.state = ASP_STATE_ERROR_CRITICAL;
        return -1;
    }

    if (!asp_init_tags())
    {
        asp.state = ASP_STATE_ERROR_CRITICAL;
        return -1;
    }

    if (!asp_get_geometry())
    {
        // need to create dummy NVRAM device anyways to prevet iBoot panic
        asp_create_block_device(ASP_NVRAM);
        asp.state = ASP_STATE_ERROR_NOT_REGISTERED;
        return -1;
    }

    if (!asp_get_nand_info())
    {
        dprintf(DEBUG_CRITICAL, "asp_get_nand_info failed\n");
    }

    asp_reinit();

    if (!asp_create_block_device(ASP_NVRAM))
    {
        asp.state = ASP_STATE_ERROR_NOT_REGISTERED;
        return -1;
    }

#if TARGET_POWER_NO_BATTERY
    if (asp_set_atv() != 0) {
        return -1;
    }
#endif

    asp_create_block_device(ASP_FIRMWARE);

    asp_create_block_device(ASP_LLB);

    asp_create_block_device(ASP_EFFACEABLE);

    asp_create_block_device(ASP_SYSCFG);

    asp_create_block_device(ASP_PANICLOG);

    asp.state = ASP_STATE_READY;

    return 0;

}

bool asp_wait_for_ready(void)
{

    msg_payload_t msg;
    uint32_t      tag;
    csi_status_t  result;

#if ASP_ENABLE_TIMEOUTS
if (!event_wait_timeout(&asp.msg_event, ASP_READY_TIMEOUT_US))
    {
        dprintf(DEBUG_CRITICAL, "ASP timed out waiting for hardware to become ready!");
        asp.state = ASP_STATE_TIMEOUT;
        return false;
    }
#else
    event_wait(&asp.msg_event);
#endif

    result = csi_receive_message(asp.csi_token, &msg);
    if (result != CSI_STATUS_OK)
    {
        return false;
    }

    tag = ASPMBOX_FIN_GET_TAG(msg);
    if (tag != ASPMBOX_TAG_AWAKE)
    {
        return false;
    }

    else
    {
        if (ASPMBOX_FIN_GET_ERRCODE(msg) != ASP_PROTOCOL_VERSION)
        {
            dprintf(DEBUG_CRITICAL, "iBoot ASP protocol version %d aspcore protocol version %lld",
                    ASP_PROTOCOL_VERSION, ASPMBOX_FIN_GET_ERRCODE(msg));
            return false;
        }
    }

    return true;

}


bool asp_init_tags(void)
{

    addr_t   paddr;
    uint64_t payload;
    uint32_t i;

    asp.asp_command = csi_allocate_shared_memory(asp.csi_token,
                                                 ASP_NUM_TAGS * ASPPROTO_CMD_LINES * CACHELINE_BYTES);

    if (!asp.asp_command)
    {
        dprintf(DEBUG_CRITICAL, "Unable to allocate ASP command memory");
        asp.asp_command = NULL;
        return false;
    }

    paddr = mem_static_map_physical((addr_t)asp.asp_command);

    payload = ASPMBOX_MAKE_CMD_SETBASE((uint64_t)paddr, ASP_NUM_TAGS * ASPPROTO_CMD_LINES);

    dprintf(DEBUG_CRITICAL, "Using paddr %p, setbase message 0x%016llx\n", (void *)paddr, payload);

    if (csi_send_message(asp.csi_token, payload) != CSI_STATUS_OK)
    {
        dprintf(DEBUG_CRITICAL, "Unable to send ASPMBOX_MASK_CMD_SETBASE message\n");
        return false;
    }

    for (i = 0; i < ASP_NUM_TAGS; i++)
    {
        aspproto_cmd_t *cmd;

        payload = ASPMBOX_MAKE_CMD_SETADDR(i, (i * ASPPROTO_CMD_LINES), ASPPROTO_CMD_LINES);

        if (csi_send_message(asp.csi_token, payload) != CSI_STATUS_OK)
        {
            dprintf(DEBUG_CRITICAL, "Unable to send ASPMBOX_MAKE_CMD_SETADDR message\n");
            return false;
        }

        cmd = asp_get_cmd_for_tag(i);
        cmd->tag = i;
    }

    return true;

}


static bool asp_get_nand_info(void)
{
    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op = ASP_PROTO_CMD_DEVICE_INFO;

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Unable to read ASP device info\n");
        return false;
    }

    if(command->nand_info.size > sizeof(nand_info))
    {
        panic ("nand info size exceeded\n");
    }
    memcpy(&nand_info, command->nand_info.data, command->nand_info.size);

    return true;
}

bool asp_send_open(void)
{
    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);
    int              status;

    command->op = ASPPROTO_CMD_OPEN;

    status = asp_send_command(command->tag);
    if (status != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Unable to open ASP: 0x%08x\n", status);
        return false;
    }

    return true;
}

int asp_nand_open(void)
{
    if (asp.state != ASP_STATE_READY)
    {
        return -1;
    }

    if (!asp_send_open())
    {
        return -1;
    }

    // NOTE: Need to get NAND geometry again after sending an OPEN
    // command to ASP to ensure that the lbaFormatted field is
    // correctly updated after CLOG replay
    if (!asp_get_geometry())
    {
        asp.state = ASP_STATE_ERROR_NOT_REGISTERED;
        return -1;
    }

    if (!asp_create_block_device(ASP_NAND))
    {
        asp.state = ASP_STATE_ERROR_NOT_REGISTERED;
        return -1;
    }

    partition_scan_and_publish_subdevices("asp_nand");

    return 0;

}

int asp_set_default_dies_in_parallel(void)
{
    asp_set_dies_in_parallel(ASPNAND_SLC_WRITE_DIES_IN_PARALLEL,
                             ASPNAND_READ_DIES_IN_PARALLEL,
                             ASPNAND_ERASE_DIES_IN_PARALLEL,
                             ASPNAND_MLC_WRITE_DIES_IN_PARALLEL,
                             ASPNAND_TLC_SLC_WRITE_DIES_IN_PARALLEL,
                             ASPNAND_TLC_READ_DIES_IN_PARALLEL,
                             ASPNAND_TLC_ERASE_DIES_IN_PARALLEL,
                             ASPNAND_TLC_TLC_WRITE_DIES_IN_PARALLEL,
                             CORE_POWERSTATE_HIGH_POWER);

    asp_set_dies_in_parallel(ASPNAND_SLC_WRITE_LOW_POWER_DIES_IN_PARALLEL,
                             ASPNAND_READ_LOW_POWER_DIES_IN_PARALLEL,
                             ASPNAND_ERASE_LOW_POWER_DIES_IN_PARALLEL,
                             ASPNAND_MLC_WRITE_LOW_POWER_DIES_IN_PARALLEL,
                             ASPNAND_TLC_SLC_WRITE_LOW_POWER_DIES_IN_PARALLEL,
                             ASPNAND_TLC_READ_LOW_POWER_DIES_IN_PARALLEL,
                             ASPNAND_TLC_ERASE_LOW_POWER_DIES_IN_PARALLEL,
                             ASPNAND_TLC_TLC_WRITE_LOW_POWER_DIES_IN_PARALLEL,
                             CORE_POWERSTATE_LOW_POWER);

    asp_set_power_state(CORE_POWERSTATE_HIGH_POWER);

    return 0;
}

int asp_disable_uid(void)
{
    aspproto_cmd_t * command = asp_get_cmd_for_tag(ASP_TAG_GENERIC);

    command->op = ASPPROTO_CMD_NAND_DEBUG;
    command->tunnel.opcode = NAND_DEBUG_DISABLE_UID;
    command->tunnel.buffer_paddr = 0;
    command->tunnel.bufferLen    = 0;
    command->tunnel.options.mask = (1 << ANC_MAX_BUSSES) - 1;

    dprintf(DEBUG_CRITICAL, "Disabling ANS UID keys\n");

    if (asp_send_command(command->tag) != ASPPROTO_CMD_STATUS_SUCCESS)
    {
        dprintf(DEBUG_CRITICAL, "Failed to disable UID keys\n");
        return -1;
    }
    return 0;
}

