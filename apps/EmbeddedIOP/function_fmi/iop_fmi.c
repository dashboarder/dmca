/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>

#include <AssertMacros.h>

#include <platform/soc/hwclocks.h>
#include <platform/soc/hwregbase.h>
#include <platform/soc/pmgr.h>
#include <platform/memmap.h>
#include <platform.h>

#include <sys/task.h>

#include <iop.h>
#include <qwi.h>
#include <EmbeddedIOPProtocol.h>

#include "iop_fmi_protocol.h"

#include <H2fmi_iop.h>

static bool fmi_message_process(int channel, h2fmi_t *fmi);

static int iop_fmi_task(void *cfg);
static void iop_fmi_sleep(int mode);

IOP_FUNCTION(fmi0, iop_fmi_task, 1536, FMI_CONTROL_CHANNEL0);
IOP_FUNCTION(fmi1, iop_fmi_task, 1536, FMI_CONTROL_CHANNEL1);
IOP_SLEEP_HOOK(fmi, iop_fmi_sleep);

static h2fmi_t* g_fmi_table[kIOPFMI_MAX_NUM_OF_BUSES];
static IOPFMI_Command*  g_pCurrentCommand[kIOPFMI_MAX_NUM_OF_BUSES];
static uint32_t g_fmi_count = 0;

static int
iop_fmi_task(void *cfg)
{
    struct iop_channel_config *channel = (struct iop_channel_config *)cfg;
    h2fmi_t* iop_fmi = (h2fmi_t*) malloc(sizeof(h2fmi_t));
    struct task_event* fmi_message_event = (struct task_event*) malloc(sizeof(struct task_event));
    int fmi_channel;
    
    check(kIOPFMI_COMMAND_SIZE == sizeof(IOPFMI_Command));

    /**
     * Ensure everything is zero in case we get panicked right away 
     * by host processor (so our panic handlers don't dereference 
     * invalid pointers) 
     */
    memset(iop_fmi,0,sizeof(*iop_fmi));
    iop_fmi->bus_id = (UInt32)-1;

    dprintf(DEBUG_SPEW, "**(%p) FMI task starting\n", iop_fmi);

    /* register the allocated FMI in the table */
    g_fmi_table[g_fmi_count++] = iop_fmi;
        
    /* establish the host communications channel */
    event_init(fmi_message_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
    dprintf(DEBUG_SPEW, "**(%p) opening fmi channel\n", iop_fmi);
    fmi_channel = qwi_instantiate_channel(
        "fmi command",
        QWI_ROLE_CONSUMER,
        channel->ring_size,
        (void *)mem_static_map_cached(channel->ring_base),
        (qwi_channel_hook)event_signal,
        fmi_message_event);

    for (;;) {
        dprintf(DEBUG_SPEW, "**(%p) waiting for message on fmi channel\n", iop_fmi);
        while (fmi_message_process(fmi_channel, iop_fmi))
        {
            // eat all available messages
        }
        event_wait(fmi_message_event);
    }

    return(0);
}

static bool
fmi_message_process(int channel, h2fmi_t *fmi)
{
    uint32_t         message;
    IOPFMI_Command**  ppCommand;
    IOPFMI_Command*  pCommand;
    const bool       is_ppn = fmi->is_ppn;

    dprintf(DEBUG_SPEW, "**(%p) handling host message\n", fmi);
    
    /* look to see if there's an item waiting for us */
    if (qwi_receive_item(channel, &message) == -1)
        return(false);
    
    dprintf(DEBUG_SPEW, "**(%p) received fmi message\n", fmi);

    /* find the command structure based on the message */
    pCommand = mem_static_map_cached(message);
    if ( ((UInt32)-1)==fmi->bus_id )
    {
        ppCommand = NULL;
    }
    else
    {
        ppCommand = &g_pCurrentCommand[fmi->bus_id];
        *ppCommand = pCommand;
    }

    /*
     * Flush any cached item contents we might have lying around - we are guaranteed
     * that the command size is a multiple of our cacheline size.
     */
    WMR_PREPARE_READ_BUFFER((void *)pCommand, (sizeof(*pCommand)));

    if (0 != (pCommand->iopfmi.state & kIOPFMI_STATE_WAKING_UP))
    {
        if (pCommand->iopfmi.opcode != kIOPFMI_OPCODE_SET_CONFIG)
        {
            h2fmi_restoreFmiRegs(fmi);
        }
    }

    if (0 != (pCommand->iopfmi.state & kIOPFMI_STATE_POWER_CHANGED))
    {
#if SUPPORT_PPN
        if (is_ppn)
        {
            h2fmi_ppn_iop_power_state_changed(fmi);
        }
#endif
    }

    /* 
     * TODO: make this part of the API and push this
     * architecture-specific command handling down into the s5l8920x
     * platform directory.
     */
    switch (pCommand->iopfmi.opcode) {
    case kIOPFMI_OPCODE_SET_CONFIG:
        dprintf(DEBUG_SPEW, "**(%p) SET_CONFIG\n", fmi);
        h2fmi_iop_set_config(fmi, &pCommand->set_config);
        break;

    case kIOPFMI_OPCODE_POST_RESET_OPER:
        dprintf(DEBUG_SPEW, "**(%p) POST RESET_OPERATIONS\n", fmi);
        WMR_ASSERT(is_ppn);

#if SUPPORT_PPN
        h2fmi_ppn_iop_post_rst_pre_pwrstate_operations(fmi, &pCommand->post_reset_oper);
#else
        WMR_PANIC("SUPPORT_PPN");
#endif
        break;

    case kIOPFMI_OPCODE_SET_FEATURES:
        dprintf(DEBUG_SPEW, "**(%p) POST RESET_OPERATIONS\n", fmi);
        WMR_ASSERT(is_ppn);

#if SUPPORT_PPN
        h2fmi_ppn_iop_set_feature_list(fmi, &pCommand->set_features);
#else
        WMR_PANIC("SUPPORT_PPN");
#endif
        break;

    case kIOPFMI_OPCODE_RESET_EVERYTHING:
        dprintf(DEBUG_SPEW, "**(%p) RESET_EVERYTHING\n", fmi);

        if (is_ppn)
        {
#if SUPPORT_PPN
            h2fmi_ppn_iop_reset_everything(fmi, &pCommand->reset_everything);
#else
            WMR_PANIC("SUPPORT_PPN");
#endif
        }
        else
        {
            h2fmi_iop_reset_everything(fmi, &pCommand->reset_everything);
        }
        break;

    case kIOPFMI_OPCODE_ERASE_SINGLE:
        dprintf(DEBUG_SPEW, "**(%p) ERASE_SINGLE\n", fmi);

        if (is_ppn)
        {
#if SUPPORT_PPN
            h2fmi_ppn_iop_erase_single(fmi, &pCommand->erase_single);
#else
            WMR_PANIC("SUPPORT_PPN");
#endif
        }
        else
        {
            h2fmi_iop_erase_single(fmi, &pCommand->erase_single);
        }
        break;

    case kIOPFMI_OPCODE_ERASE_MULTIPLE:
        dprintf(DEBUG_SPEW, "**(%p) ERASE_MULTIPLE\n", fmi);
        if (is_ppn)
        {
            WMR_PANIC("Erase Multiple called on PPN device with Legacy FIL");
        }
        else
        {
            h2fmi_iop_erase_multiple(fmi, &pCommand->erase_multiple);
        }
        break;

    case kIOPFMI_OPCODE_READ_SINGLE:
        dprintf(DEBUG_SPEW, "**(%p) READ_SINGLE\n", fmi);
        if (is_ppn)
        {
#if SUPPORT_PPN
            h2fmi_ppn_iop_read_single(fmi, &pCommand->io_single);
#else
            WMR_PANIC("SUPPORT_PPN");
#endif
        }
        else
        {
            h2fmi_iop_read_single(fmi, &pCommand->io_single);
        }
        break;

    case kIOPFMI_OPCODE_READ_MULTIPLE:
        dprintf(DEBUG_SPEW, "**(%p) READ_MULTIPLE\n", fmi);
        if (is_ppn)
        {
            WMR_PANIC("Legacy FIL used for PPN read!");
        }
        else
        {
            h2fmi_iop_read_multiple(fmi, &pCommand->io_multiple);
        }
        break;

    case kIOPFMI_OPCODE_READ_RAW:
        dprintf(DEBUG_SPEW, "**(%p) READ_RAW\n", fmi);
        if (is_ppn)
        {
#if SUPPORT_PPN
            h2fmi_ppn_iop_read_raw(fmi, &pCommand->io_raw);
#else
            WMR_PANIC("SUPPORT_PPN");
#endif
        }
        else
        {
            h2fmi_iop_read_raw(fmi, &pCommand->io_raw);
        }
        break;

    case kIOPFMI_OPCODE_READ_BOOTPAGE:
        dprintf(DEBUG_SPEW, "**(%p) READ_BOOTLOADER\n", fmi);
        if (is_ppn)
        {
#if SUPPORT_PPN
            h2fmi_ppn_iop_read_bootpage(fmi, &pCommand->io_bootpage);
#else
            WMR_PANIC("SUPPORT_PPN");
#endif
        }
        else
        {
            h2fmi_iop_read_bootpage(fmi, &pCommand->io_bootpage);
        }
        break;

    case kIOPFMI_OPCODE_WRITE_SINGLE:
        dprintf(DEBUG_SPEW, "**(%p) WRITE_SINGLE\n", fmi);
        if (is_ppn)
        {
#if SUPPORT_PPN
            h2fmi_ppn_iop_write_single(fmi, &pCommand->io_single);
#else
            WMR_PANIC("SUPPORT_PPN");
#endif
        }
        else
        {
            h2fmi_iop_write_single(fmi, &pCommand->io_single);
        }
        break;

    case kIOPFMI_OPCODE_WRITE_MULTIPLE:
        dprintf(DEBUG_SPEW, "**(%p) WRITE_MULTIPLE\n", fmi);
        if (is_ppn)
        {
            WMR_PANIC("WRITE_MULTIPLE on PPN device using legacy FIL");
        }
        else
        {
            h2fmi_iop_write_multiple(fmi, &pCommand->io_multiple);
        }
        break;

     case kIOPFMI_OPCODE_WRITE_RAW:
         dprintf(DEBUG_SPEW, "**(%p) WRITE_RAW\n", fmi);
         if (is_ppn)
         {
             WMR_PANIC("WRITE_RAW on PPN device");
         }
         else
         {
             h2fmi_iop_write_raw(fmi, &pCommand->io_raw);
         }
         break;

     case kIOPFMI_OPCODE_WRITE_BOOTPAGE:
         dprintf(DEBUG_SPEW, "**(%p) WRITE_BOOTLOADER\n", fmi);
         if (is_ppn)
         {
#if SUPPORT_PPN
             h2fmi_ppn_iop_write_bootpage(fmi, &pCommand->io_bootpage);
#else
            WMR_PANIC("SUPPORT_PPN");
#endif
         }
         else
         {
             h2fmi_iop_write_bootpage(fmi, &pCommand->io_bootpage);
         }
         break;

    case kIOPFMI_OPCODE_READ_CAU_BBT:
        dprintf(DEBUG_SPEW, "**(%p) READ_CAU_BBT\n", fmi);
        WMR_ASSERT(is_ppn);
#if SUPPORT_PPN
        h2fmi_ppn_iop_read_cau_bbt(fmi, &pCommand->io_ppn);
#else
        WMR_PANIC("SUPPORT_PPN");
#endif
        break;

    case kIOPFMI_OPCODE_UPDATE_FIRMWARE:
        dprintf(DEBUG_SPEW, "**(%p) UPDATE_FIRMWARE\n", fmi);
        WMR_ASSERT(is_ppn);
#if SUPPORT_PPN
        h2fmi_ppn_iop_update_firmware(fmi, &pCommand->update_firmware);
#else
        WMR_PANIC("SUPPORT_PPN");
#endif

    case kIOPFMI_OPCODE_PPN_READ:
        dprintf(DEBUG_SPEW, "**(%p) PPN_READ\n", fmi);
        WMR_ASSERT(is_ppn);
#if SUPPORT_PPN
        h2fmi_ppn_iop_read_multiple(fmi, &pCommand->io_ppn);
#else
        WMR_PANIC("SUPPORT_PPN");
#endif
        break;

    case kIOPFMI_OPCODE_PPN_WRITE:
        dprintf(DEBUG_SPEW, "**(%p) PPN_WRITE\n", fmi);
        WMR_ASSERT(is_ppn);
#if SUPPORT_PPN
        h2fmi_ppn_iop_write_multiple(fmi, &pCommand->io_ppn);
#else
        WMR_PANIC("SUPPORT_PPN");
#endif
        break;

    case kIOPFMI_OPCODE_PPN_ERASE:
        dprintf(DEBUG_SPEW, "**(%p) PPN_ERASE\n", fmi);
        WMR_ASSERT(is_ppn);
#if SUPPORT_PPN
        h2fmi_ppn_iop_erase_multiple(fmi, &pCommand->io_ppn);
#else
        WMR_PANIC("SUPPORT_PPN");
#endif
        break;

    case kIOPFMI_OPCODE_PPN_SET_POWER:
        dprintf(DEBUG_SPEW, "**(%p) PPN_SET_POWER\n", fmi);
        WMR_ASSERT(is_ppn);
#if SUPPORT_PPN
        h2fmi_ppn_iop_set_power(fmi, &pCommand->set_power);
#endif
        break;

    case kIOPFMI_OPCODE_READ_CHIP_IDS:
        dprintf(DEBUG_SPEW, "**(%p) READ_CHIP_IDS\n", fmi);
        h2fmi_iop_read_chip_ids(fmi, &pCommand->read_chip_ids);
        break;

#if SUPPORT_PPN
    case kIOPFMI_OPCODE_GET_FAILURE_INFO:
        dprintf(DEBUG_SPEW, "**(%p) GET_FAILURE_INFO\n", fmi);
        h2fmi_ppn_iop_get_failure_info(fmi, &pCommand->get_failure_info);
        break;

    case kIOPFMI_OPCODE_GET_CONTROLLER_INFO:
        dprintf(DEBUG_SPEW, "**(%p) GET_CONTROLLER_INFO\n", fmi);
        WMR_ASSERT(is_ppn);
        h2fmi_ppn_iop_get_controller_info(fmi, &pCommand->get_controller_info);
        break;

    case kIOPFMI_OPCODE_GET_TEMPERATURE:
        dprintf(DEBUG_SPEW, "**(%p) GET_TEMPERATURE\n", fmi);
        WMR_ASSERT(is_ppn);
        h2fmi_ppn_iop_get_temperature(fmi, &pCommand->get_temperature);
        break;

   case kIOPFMI_OPCODE_GET_DIE_INFO:
       dprintf(DEBUG_SPEW, "**(%p) GET_DIE_INFO\n", fmi);
       WMR_ASSERT(is_ppn);
       h2fmi_ppn_iop_get_die_info(fmi, &pCommand->get_die_info);
       break;
#endif

    default:
        dprintf(DEBUG_CRITICAL, "**(%p) ERROR: unrecognised fmi opcode 0x%x", fmi, 
                pCommand->iopfmi.opcode);
        pCommand->iopfmi.status = kIOPFMI_STATUS_PARAM_INVALID;
        break;
    }

    dprintf(DEBUG_SPEW, "**(%p) done processing fmi message with status 0x%08x\n", fmi, pCommand->iopfmi.status);

    WMR_PREPARE_WRITE_BUFFER((void *)pCommand, sizeof(*pCommand));

    qwi_send_item(channel, message);

    dprintf(DEBUG_SPEW, "**(%p) signaled completion of fmi message to host\n", fmi);

    if ( NULL!=ppCommand )
    {
        *ppCommand = NULL;
    }

    return(true);
}

static void iop_fmi_sleep(int mode)
{
    uint32_t idx;
    for (idx = 0; idx < g_fmi_count; idx++)
    {
        if (IOP_SLEEP_MODE_SLEEPING == mode)
        {
            h2fmi_iop_sleep(g_fmi_table[idx]);
        }
        else if (IOP_SLEEP_MODE_WAKING == mode)
        {
            h2fmi_iop_wake(g_fmi_table[idx]);
        }
        else
        {
            dprintf(DEBUG_CRITICAL, "ERROR: unrecognized sleep mode value\n");
        }
    }
}

static void
do_fmi_panic(void *arg __unused)
{
	uint32_t	i;

    printf("g_fmi_count: %d, currentTick: 0x%llx\n",g_fmi_count,WMR_CLOCK_TICKS());

    for ( i=0; i<g_fmi_count; i++ )
    {
        IOPFMI_Command* pCommand = ( ((UInt32)-1)==g_fmi_table[i]->bus_id ? NULL : g_pCurrentCommand[g_fmi_table[i]->bus_id] );
        if (NULL == pCommand)
        {
            printf("Not executing command\n");
            // Skip HW Regs when idle in case we're gated
            dump_fmi_state(g_fmi_table[i], i, FALSE32, FALSE32);
        }
        else
        {
            BOOL32 withECC;

            switch (pCommand->iopfmi.opcode)
            {
                case kIOPFMI_OPCODE_READ_BOOTPAGE:
                    // fall-through
                case kIOPFMI_OPCODE_WRITE_BOOTPAGE:
                    withECC = TRUE32;
                    break;

                default:
                    withECC = (g_fmi_table[i]->is_ppn ? FALSE32 : TRUE32);
                    break;
            }

            printf("Is executing command @ %p opCode: %d\n", pCommand, pCommand->iopfmi.opcode);    
            dump_fmi_state(g_fmi_table[i], i, TRUE32, withECC);
        }
    }
    
}

PANIC_HOOK(fmi, do_fmi_panic, NULL);

