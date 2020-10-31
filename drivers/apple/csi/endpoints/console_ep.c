/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <platform.h>
#include <platform/timer.h>
#include <arch.h>
#include <debug.h>
#include <sys/task.h>
#include <drivers/csi.h>
#include <drivers/a7iop/a7iop.h>

#include "csi_private.h"
#include "endpoints/console_ep.h"

#include <csi_ipc_protocol.h>
#include <csi_console_defines.h>
#include <csi_console_protocol.h>


//
// Global Data
//

static ep_console_desc_t *console_descs[CSI_COPROC_MAX] = {NULL};

//
// Private functions
//

int   ep_console_task     (void *arg);
void  console_msg_handler (ep_console_desc_t *console, msg_payload_t msg);


//
// bi_ep_console_create
//
void
bi_ep_console_create (csi_coproc_t which_coproc)
{
  // enforce single instance
  REQUIRE(console_descs[which_coproc]==NULL);
  console_descs[which_coproc] = malloc(sizeof(ep_console_desc_t));
  REQUIRE(console_descs[which_coproc]!=NULL);

  console_descs[which_coproc]->coproc = which_coproc;

  task_start(task_create("csi ep console", ep_console_task, console_descs[which_coproc], kTaskStackSize_endpoints));
}


csi_status_t
console_quiesce (csi_coproc_t which_coproc, csi_power_state_t ps)
{
  ep_console_desc_t *console = console_descs[which_coproc];

  // free the sahred memory
  if (!(ps & CSI_PS_SHARED_PRESERVED) && (NULL != console->buffers)) {
    csi_free_shared_memory(console->buffers);
    console->buffers   = NULL;
    console->allocated = 0;
  }

  return CSI_STATUS_OK;
}


//
// bi_ep_console_send_custom_cmd
//
csi_status_t
bi_ep_console_send_custom_cmd (csi_coproc_t which_coproc, uint32_t payload)
{
  ep_console_desc_t   *console;
  ipc_msg_t           resp;

  console = console_descs[which_coproc];

  if (console == NULL) {
    return CSI_STATUS_UNAVAILABLE;
  }

  ipc_console_create_custom_cmd(&resp, payload);

  return bi_ep_send_message (console->csi_token, resp);
}


//
// bi_ep_console_set_verbosity
//
csi_status_t
bi_ep_console_set_verbosity (csi_coproc_t which_coproc, uint32_t verbosity)
{
  ep_console_desc_t   *console;
  ipc_msg_t           resp;

  console = console_descs[which_coproc];

  if (console == NULL) {
    return CSI_STATUS_UNAVAILABLE;
  }

  console->verbosity = verbosity;
  ipc_console_create_flow_enable_cmd(&resp, console->verbosity);

  return bi_ep_send_message (console->csi_token, resp);
}


//
// Console Message handler
//

int
ep_console_task (void *arg)
{
  csi_status_t      result;
  msg_payload_t   rcv_msg;

  ep_console_desc_t *console = (ep_console_desc_t*)arg;

  event_init(&console->msg_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

  result = csi_register_endpoint(console->coproc, IPC_ENDPOINT_BUILT_IN_CONSOLE, &console->msg_event, &console->csi_token, &console->name);
  REQUIRE(result==CSI_STATUS_OK);

  console->verbosity = 1;
  console->buffers   = NULL;
  console->allocated = 0;
  console->scratch   = malloc(CONSOLE_BUFFER_SIZE+1);
  REQUIRE(console->scratch!=NULL);

  for (;;) {
    event_wait(&console->msg_event);

    // empty the rcv queue
    while (csi_receive_message(console->csi_token, &rcv_msg) == CSI_STATUS_OK) {
      console_msg_handler(console, rcv_msg);
    }
  }

  return 0;
}


void
console_msg_handler (ep_console_desc_t *console, msg_payload_t msg)
{
  uint64_t            paddr;
  ipc_msg_t           resp;
  uint32_t            cmd;
  uint32_t            rd_off;
  uint32_t            wr_off;
  uint32_t            size;
  console_buffer_t    *buffer_desc;
  csi_status_t        status;

  // the console endpoint use the reserved 2 bits of the buffer message to encode commands
  // decode the command and respond to it
  cmd = ipc_msg_extract(msg, IPC_MSG_BUFF_MSG_FLAG_RV_LEN, IPC_MSG_BUFF_MSG_FLAG_RV_POS);

  switch (cmd) {

    case REQUEST_NEW_BUFFER:
      // allocate a console buffer of the requested size.
      console->allocated = ipc_buff_msg_get_size(msg);
      console->buffers = csi_allocate_shared_memory(console->csi_token, console->allocated);
      REQUIRE(console->buffers!=NULL);
      paddr = mem_static_map_physical((uintptr_t)console->buffers);

      // send the new buffer response back to the IOP
      ipc_buff_create_msg(&resp, paddr, console->allocated);
      resp |= ipc_msg_set(HERE_IS_A_NEW_BUFFER, IPC_MSG_BUFF_MSG_FLAG_RV_LEN, IPC_MSG_BUFF_MSG_FLAG_RV_POS);
      status = bi_ep_send_message (console->csi_token, resp);
      if (CSI_STATUS_OK != status) {
        CSI_EP_LOG(CRITICAL, console->coproc, console->name, "unable to reply to REQUEST_NEW_BUFFER: %x\n", status);
        break;
      }

      // this transaction occurs when the console endpoint is initially up and ready
      // and so is an appropriate time to communicate console verbosity to the IOP
      ipc_console_create_flow_enable_cmd(&resp, console->verbosity);
      status = bi_ep_send_message (console->csi_token, resp);
      if (CSI_STATUS_OK != status) {
        CSI_EP_LOG(CRITICAL, console->coproc, console->name, "unable to send console verbosity: %x\n", status);
      }
      break;

    case OUTPUT_BUFFER:
      buffer_desc = &((console_buffer_t*)(console->buffers))[ipc_console_iocmd_get_channel(&msg)];

      rd_off = ipc_console_iocmd_get_rdptr(&msg);
      wr_off = ipc_console_iocmd_get_wrptr(&msg);

      if (rd_off<=wr_off) {
        size = wr_off - rd_off;
        REQUIRE(size<=CONSOLE_BUFFER_SIZE);
        maybe_do_cache_operation(CACHE_INVALIDATE, &buffer_desc->buffer[rd_off], size);
        memcpy(console->scratch,  &buffer_desc->buffer[rd_off], size);
      } else {
        size = CONSOLE_BUFFER_SIZE - rd_off;
        REQUIRE((size+wr_off)<=CONSOLE_BUFFER_SIZE);
        maybe_do_cache_operation(CACHE_INVALIDATE, &buffer_desc->buffer[rd_off], size);
        memcpy(console->scratch,  &buffer_desc->buffer[rd_off], size);
        maybe_do_cache_operation(CACHE_INVALIDATE, buffer_desc->buffer, wr_off);
        memcpy((console->scratch+size), buffer_desc->buffer, wr_off);
        size += wr_off;
      }
      console->scratch[size] = '\0';

      CSI_EP_LOG(CONSOLE, console->coproc, console->name, "%s\n", console->scratch);

      // only send message if the other side is not shutting down. Otherwise this
      // might create a lock up in suspend to RAM situation.
      if (!csi_is_shutdown_in_progress(console->coproc)) {
        ipc_console_create_iocmd(&resp, DONE_WITH_BUFFER, ipc_console_iocmd_get_channel(&msg), wr_off, wr_off);
        status = bi_ep_send_message (console->csi_token, resp);
      }
      break;

    case BUFFER_OVERFLOWED:
      CSI_EP_LOG(CONSOLE, console->coproc, console->name, "Console buffer overflow - message truncated\n");
      break;

    default:
      CSI_EP_LOG(CRITICAL, console->coproc, console->name, "unimplemented console command: %x\n", cmd);
      break;
  }
}
