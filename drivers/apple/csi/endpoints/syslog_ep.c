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
#include "endpoints/syslog_ep.h"

#include <csi_ipc_protocol.h>
#include <csi_syslog_defines.h>
#include <csi_syslog_protocol.h>


//
// Syslog Object
//

typedef struct {
  csi_coproc_t        coproc;
  struct task_event   msg_event;
  void                *csi_token;
  const char          *name;
  uint32_t             verbosity;
  syslog_database_t   *buffer;
  uint32_t            allocated;
} ep_syslog_desc_t;

//
// Global Data
//

static ep_syslog_desc_t *syslog_descs[CSI_COPROC_MAX] = {NULL};

const char* syslog_priority_names[] = SYSLOG_PRIORITY_NAMES;
const char* syslog_facility_names[] = SYSLOG_FACILITY_NAMES;

//
// Private functions
//

int   ep_syslog_task (void *arg);
void  syslog_msg_handler (ep_syslog_desc_t *syslog, msg_payload_t msg);


//
// bi_ep_syslog_create
//
// Create a new syslog endpoint
//
void
bi_ep_syslog_create (csi_coproc_t which_coproc)
{
  // enforce single instance
  REQUIRE(syslog_descs[which_coproc]==NULL);
  syslog_descs[which_coproc] = malloc(sizeof(ep_syslog_desc_t));
  REQUIRE(syslog_descs[which_coproc]!=NULL);

  syslog_descs[which_coproc]->coproc = which_coproc;

  task_start(task_create("csi ep syslog", ep_syslog_task, syslog_descs[which_coproc], kTaskStackSize_endpoints));
}


csi_status_t
syslog_quiesce (csi_coproc_t which_coproc, csi_power_state_t ps)
{
  ep_syslog_desc_t *syslog = syslog_descs[which_coproc];

  // free all shared memory if we are not preserving it
  if (!(ps & CSI_PS_SHARED_PRESERVED) && (NULL != syslog->buffer)) {
    csi_free_shared_memory(syslog->buffer);
    syslog->buffer    = NULL;
    syslog->allocated = 0;
  }

  return CSI_STATUS_OK;
}


//
// bi_ep_syslog_verbosity
//
// Set the syslog endpoint verbosity
//
csi_status_t
bi_ep_syslog_verbosity (csi_coproc_t which_coproc, uint32_t verbosity)
{

  ep_syslog_desc_t *syslog = syslog_descs[which_coproc];

  if (syslog==NULL) {
    return CSI_STATUS_UNAVAILABLE;
  }

  if (verbosity > LOG_DEBUG) {
    return CSI_STATUS_ERROR;
  }

  syslog->verbosity = verbosity;

  return CSI_STATUS_OK;
}


//
// syslog endpoint: task
//

int
ep_syslog_task(void *arg)
{
  csi_status_t      result;
  msg_payload_t   rcv_msg;

  ep_syslog_desc_t *syslog = (ep_syslog_desc_t*)arg;

  event_init(&syslog->msg_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

  result = csi_register_endpoint(syslog->coproc, IPC_ENDPOINT_BUILT_IN_SYSLOG, &syslog->msg_event, &syslog->csi_token, &syslog->name);
  REQUIRE(result==CSI_STATUS_OK);

  syslog->verbosity = LOG_DEFAULT;

  syslog->buffer = NULL;
  syslog->allocated = 0;

  for (;;) {
    event_wait(&syslog->msg_event);

    // empty the rcv queue
    while (csi_receive_message(syslog->csi_token, &rcv_msg) == CSI_STATUS_OK) {
      syslog_msg_handler(syslog, rcv_msg);
    }
  }

  return 0;
}

void
syslog_msg_handler(ep_syslog_desc_t *syslog, msg_payload_t msg)
{
  uint32_t      cmd;
  uint32_t      entry;
  uint32_t      type;
  uint32_t      size;
  csi_status_t  status;

  cmd = syslog_get_cmd(&msg);

  switch (cmd) {

    case REQUEST_SYSLOGDB_BUFFER:

      syslog->allocated = ipc_buff_msg_get_size(msg);
      REQUIRE(syslog->buffer==NULL);
      syslog->buffer = csi_allocate_shared_memory(syslog->csi_token, syslog->allocated);
      REQUIRE(syslog->buffer!=NULL);

      // send the response back to the coproc
      uint64_t buffer_phys = mem_static_map_physical((uintptr_t)syslog->buffer);
      status = bi_ep_send_message (syslog->csi_token, syslog_construct_msg(HERE_IS_YOUR_SYSLOGDB_BUFFER, buffer_phys, ipc_buff_msg_get_size(msg)));
      if (status == CSI_STATUS_OK) {
        CSI_EP_LOG(DEBUG_SPEW, syslog->coproc, syslog->name, "buffer request of size %u bytes, allocated @ physical 0x%llx (AP %p)\n",
                          syslog->allocated, buffer_phys, syslog->buffer);
      } else {
        CSI_EP_LOG(CRITICAL, syslog->coproc, syslog->name, "unable to reply to REQUEST_SYSLOGDB_BUFFER cmd.\n");
      }
      break;

    case OUTPUT_SYSLOG_ENTRY:
      entry = (uint32_t)syslog_get_address(&msg);
      maybe_do_cache_operation(CACHE_INVALIDATE, &syslog->buffer->entries[entry], sizeof(syslog_entry_t));
      type  = syslog->buffer->entries[entry].type;
      if (LOG_PRI(type) <= syslog->verbosity) {
        if (LOG_FAC(type) != LOG_KERN) {
          CSI_EP_LOG (SYSLOG, syslog->coproc, syslog->name, "%s: %s\n", syslog_get_facility_name(type), syslog->buffer->entries[entry].message);
        } else {
          CSI_EP_LOG (SYSLOG, syslog->coproc, syslog->name, "task #%d, file: %s: %s\n",
                      syslog->buffer->entries[entry].taskid, syslog->buffer->entries[entry].filename, syslog->buffer->entries[entry].message);
        }
      }
      status = bi_ep_send_message (syslog->csi_token, syslog_construct_msg(OUTPUTTED_SYSLOG_ENTRY, entry, 0));
      break;

    case SYSLOG_ENTRY_TRUNCATED:
      size = (uint32_t)syslog_get_address(&msg); // in this message the address field is used to report the log entry length
      CSI_EP_LOG(CRITICAL, syslog->coproc, syslog->name, "coproc tried to print a 0x%x byte message, but message buffer is only %d bytes long.\n", size, SYSLOG_MESSAGE_SIZE);
      break;

    case SYSLOGDB_OVERFLOW:
      CSI_EP_LOG(CRITICAL, syslog->coproc, syslog->name, "coproc database overflowed.\n");
      break;

    default:
      CSI_EP_LOG(CRITICAL, syslog->coproc, syslog->name, "unimplemented syslog command: %x\n", cmd);
      break;
  }
}
