/*
 * Copyright (C) 2011-2012 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#include <platform.h>
#include <platform/memmap.h>
#include <arch.h>
#include <debug.h>
#include <sys/task.h>
#include <sys/boot.h>
#include <lib/env.h>
#include <platform/timer.h>
#include <drivers/a7iop/a7iop.h>
#include <drivers/csi.h>

#include "csi_private.h"

// protocol files
#include <csi_system.h>

#include "endpoints/management_ep.h"
#include "endpoints/syslog_ep.h"
#include "endpoints/console_ep.h"
#include "endpoints/crashlog_ep.h"

#include <platform/soc/hwclocks.h>
#include <platform/clocks.h>
#include <platform/soc/pmgr.h>

// XXX <rdar://problem/10422340> addresses the need for a common copy of this file
#include "queue.h"


static const MailboxEndpointConfig endpoint_config[] = MAILBOX_ENDPOINT_INITIALIZERS;

#define MAILBOX_ENDPOINT_INITIALIZER_COUNT (int)(sizeof(endpoint_config)/sizeof(*endpoint_config))

#define EP_QUEUE_ADVANCE(_index, _size) ((_index) = ((((_index) + 1) >= _size) ? 0 : ((_index) + 1)))


// A simple mechanism to put a governor on potentially high volume log scenarios
#define LOG_GOV_PERIOD_USEC   5000000

typedef struct {
  int         occ_count;
  u_int64_t   last_log;
} log_governor_t;


typedef struct {
  bool                      registered;
  csi_coproc_t              coproc;
  uint32_t                  index;
  const char                *name;
  uint32_t                  rcv_queue_size;
  msg_payload_t             *rcv_queue;
  uint32_t                  next_in;
  uint32_t                  next_out;
  uint32_t                  sm_alignment;
  struct task_event         *rcv_not_empty_event;
} endpoint_desc_t;


typedef enum {
  COPROC_STATE_RESET = 0,
  COPROC_STATE_ERROR_FW,
  COPROC_STATE_ERROR_STARTING,
  COPROC_STATE_RUNNING,
  COPROC_STATE_STOPPED,
} coproc_state_t;

typedef struct {
  csi_coproc_t              coproc;
  coproc_state_t            state;
  coproc_feature_t          feature;
  uint32_t                  send_timeout;
  uint32_t                  endpoint_count;
  endpoint_desc_t           endpoints[CSI_MAX_ENDPOINTS];
  fw_desc_t                 fw_desc;
  struct task_event         power_state_event;
  log_governor_t            gov_ep_queue_ovflw;
  log_governor_t            gov_ep_not_reg;
} csi_desc_t;

static csi_desc_t *csi_descs[CSI_COPROC_MAX] = {NULL};


//
// Private functions
//

static endpoint_desc_t *find_endpoint(csi_coproc_t which_coproc, ipc_endpoint_t which_ep);

static int inbox_not_empty_task(void *arg);

static void message_dispatcher(endpoint_desc_t *ep, msg_payload_t msg_payload);

static void *allocate_shared_memory(uint32_t capacity, uint32_t alignment);

static bool check_log_gov(log_governor_t *gov, csi_coproc_t which_coproc);


//
// csi_init
//
csi_status_t
csi_init (csi_coproc_t which_coproc)
{
  int               j;
  csi_desc_t        *csi;
  csi_status_t      status;
  endpoint_desc_t   *next_ep;

  if (which_coproc >= CSI_COPROC_MAX) {
    return CSI_STATUS_UNSUPPORTED;
  }

  // enforce single instance
  REQUIRE(csi_descs[which_coproc]==NULL);
  csi_descs[which_coproc] = malloc(sizeof(csi_desc_t));
  REQUIRE(csi_descs[which_coproc]!=NULL);

  csi = csi_descs[which_coproc];

  csi->coproc       = which_coproc;
  csi->state        = COPROC_STATE_RESET;
  csi->send_timeout = CSI_SEND_MESSAGE_TIMEOUT;

  csi->gov_ep_queue_ovflw.occ_count = 0;
  csi->gov_ep_not_reg.occ_count = 0;

  csi->feature = COPROC_FEATURE_NONE;

  // Create all endpoint structures - all are inactive until registered.

  csi->endpoint_count = 0;
  next_ep = csi->endpoints;

  for (j=CSI_MAX_ENDPOINTS; --j>=0;) {
    next_ep->registered = false;
    next_ep->coproc = csi->coproc;
    next_ep->index = 0;
    next_ep->name = "none";
    next_ep->rcv_queue_size = 0;
    next_ep->rcv_queue = NULL;
    next_ep->next_in = 0;
    next_ep->next_out = 0;
    next_ep->rcv_not_empty_event = NULL;
    next_ep++;
  }

  next_ep = csi->endpoints;

  for (j=0; (j<MAILBOX_ENDPOINT_INITIALIZER_COUNT) && (csi->endpoint_count<CSI_MAX_ENDPOINTS); j++) {
    if (endpoint_config[j].rcv_queue_size != 0) {
      next_ep->index = endpoint_config[j].index;
      next_ep->name = endpoint_config[j].name;
      next_ep->rcv_queue_size = endpoint_config[j].rcv_queue_size;
      next_ep->rcv_queue = malloc(sizeof(*next_ep->rcv_queue) * endpoint_config[j].rcv_queue_size);
      REQUIRE(next_ep->rcv_queue!=NULL);
      next_ep->next_in = 0;
      next_ep->next_out = ~(0);

      next_ep++;
      csi->endpoint_count++;
    }
  }

  // create built-in endpoints
  bi_ep_mgmt_create(csi->coproc, &(csi->power_state_event));
  bi_ep_crashlog_create(csi->coproc);
  bi_ep_console_create(csi->coproc);
  bi_ep_syslog_create(csi->coproc);

  // Find the firmware, prep it, start the coprocessor.
  if (CSI_COPROC_ANS == which_coproc) {
    // region is provided by iBoot
    csi->fw_desc.region_base = ASP_BASE;
    csi->fw_desc.region_size = (uint32_t)(ASP_SIZE);
  } else {
    // Allocate region for all other corpoc
    // XXX: what size is needed - to be added later when/if other coproc are supported.
    csi->fw_desc.region_base = 0x00;
    csi->fw_desc.region_size = 0x00;

    return CSI_STATUS_UNSUPPORTED;
  }

  status = fw_fixup_and_relocate(csi->coproc, &csi->fw_desc);
  if (status != CSI_STATUS_OK) {
    CSI_LOG(CRITICAL, which_coproc, "fw_fixup_and_relocate() error (%s)\n", CSI_STATUS_STR(status));
    csi->state = COPROC_STATE_ERROR_FW;
    return status;
  }

  status = csi_start (which_coproc);
  if (status != CSI_STATUS_OK) {
    return status;
  }

  // create the power state event
  event_init(&(csi->power_state_event), EVENT_FLAG_AUTO_UNSIGNAL, false);

  return CSI_STATUS_OK;
}


csi_status_t
csi_start (csi_coproc_t which_coproc)
{
  csi_desc_t        *csi;
  addr_t            remap_base;
  uint64_t          remap_size;
  int               status;

  csi = csi_descs[which_coproc];

  remap_base = (const addr_t)csi->fw_desc.region_base;
  remap_size = (const uint64_t)csi->fw_desc.region_size;

  status = akf_start(KFW(csi->coproc), remap_base, remap_size);
  if (status != 0) {
    CSI_LOG(CRITICAL, which_coproc, "akf_start() error: %d\n", status);
    csi->state = COPROC_STATE_ERROR_STARTING;
    return CSI_STATUS_ERROR;
  }
  csi_clr_feature (which_coproc, COPROC_FEATURE_SHUTDOWN_IN_PROGRESS);

  CSI_LOG(DEBUG_SPEW, which_coproc, "coproc started: remap base = %p, size = 0x%llx (feature=%x)\n", (void *)remap_base, remap_size, csi_get_feature(which_coproc));
  csi->state = COPROC_STATE_RUNNING;

  // start the message reading task
  task_start(task_create("csi inbox_not_empty_task", inbox_not_empty_task, (void*)csi, kTaskStackSize_inbox_not_empty));

  task_yield(); // give them a chance to run

  return CSI_STATUS_OK;
}


csi_status_t
csi_panic_recover (csi_coproc_t which_coproc)
{
  csi_desc_t        *csi;
  csi_status_t      status;

  csi = csi_descs[which_coproc];

  // stop the AKF to reset it.
  csi_stop (which_coproc);

  // destroy all the allocated shared memory
  // force a power state change to off.
  handle_power_state_ack (mgmt_descs[which_coproc], CSI_PS_OFF);

  clock_reset_device(CLK_ANS);

  // give a chance to all other task to terminate
  task_yield();

  // reset the memory
  status = fw_fixup_and_relocate (csi->coproc, &csi->fw_desc);
  if (status != CSI_STATUS_OK) {
    CSI_LOG(CRITICAL, which_coproc, "panic recovery failed: fw_fixup_and_relocate() error (%s)\n", CSI_STATUS_STR(status));
    csi->state = COPROC_STATE_ERROR_FW;
    return status;
  }

  // restart the AKF
  return csi_start (which_coproc);
}


csi_status_t
csi_late_init (void) {
#if APPLICATION_IBOOT
  const char  *boot_args;
  char        *arg_str;
  uint32_t        verbosity;

  verbosity = 9999;

  boot_args = env_get("boot-args");

  if (boot_args != NULL) {
    arg_str = strstr(boot_args, "csi_syslog_verbosity=");
    if (arg_str != NULL) {
      verbosity = arg_str[strlen("csi_syslog_verbosity=")] - '0';
    }
  }

  for (int j = 0; j < CSI_COPROC_MAX; j++) {
    bi_ep_syslog_verbosity(j, verbosity);
  }
#endif

  return CSI_STATUS_OK;
}

csi_status_t
csi_stop (csi_coproc_t which_coproc)
{
  csi_desc_t        *csi;

  csi = csi_descs[which_coproc];

  akf_stop (KFW(csi->coproc));
  csi->state = COPROC_STATE_STOPPED;

  return CSI_STATUS_OK;
}


csi_status_t
csi_quiesce (enum boot_target target)
{
  csi_status_t  status = CSI_STATUS_OK;

  for (int j = 0; j < CSI_COPROC_MAX; j++) {
    csi_desc_t *csi = csi_descs[j];

    if (NULL != csi) {
      if (csi->state == COPROC_STATE_RUNNING) {
        // put the coproc in suspend to ram for all state
        status = hibernate_iop (csi->coproc);
      }
    }
  }

  return status;
}


csi_status_t
request_iop_power_state (csi_coproc_t which_coproc, csi_power_state_t ps)
{
  csi_desc_t  *csi;

  csi = csi_descs[which_coproc];

  // if the power state event was previously signaled (panic recovery for example)
  // unsignal it was we need to wait for a new transition
  event_unsignal (&(csi->power_state_event));

  // Request the IOP to transition to the specified power state
  // power message are always send thru the management channel
  bi_ep_mgmt_send_ps (which_coproc, ps);

  // Wait for the acknowledge message.
  event_wait (&(csi->power_state_event));

  // make sure the power state is the one we want
  if (bi_ep_mgmt_get_power_state(csi->coproc) != ps) {
    return CSI_STATUS_ERROR;
  }

  return CSI_STATUS_OK;
}


csi_status_t
quiesce_iop (csi_coproc_t which_coproc)
{
  csi_desc_t         *csi;
  csi_status_t       status;

  csi = csi_descs[which_coproc];

  // Request the new power state
  status = request_iop_power_state (which_coproc, CSI_PS_QUIESCE);
  if (CSI_STATUS_OK != status) {
    CSI_LOG(CRITICAL, csi->coproc, "Failed to quiesce\n");
    return CSI_STATUS_ERROR;
  }

  CSI_LOG(CRITICAL, csi->coproc, "quiesced\n");

  return CSI_STATUS_OK;
}


csi_status_t
hibernate_iop (csi_coproc_t which_coproc)
{
  csi_desc_t         *csi;
  csi_status_t       status;

  csi = csi_descs[which_coproc];

  // Request the new power state
  csi_set_feature (which_coproc, COPROC_FEATURE_SHUTDOWN_IN_PROGRESS);
  status = request_iop_power_state (which_coproc, CSI_PS_SLEEP);
  if (CSI_STATUS_OK != status) {
    CSI_LOG(CRITICAL, csi->coproc, "Failed to hibernate\n");
    return CSI_STATUS_ERROR;
  }

  CSI_LOG(CRITICAL, csi->coproc, "in suspend to RAM\n");

  // now turn off the iop
  return csi_stop (which_coproc);
}


csi_status_t
set_send_timeout (csi_coproc_t which_coproc, uint32_t timeout)
{
  csi_desc_t  *csi = csi_descs[which_coproc];

  csi->send_timeout = timeout;

  return CSI_STATUS_OK;
}

uint32_t
get_send_timeout (csi_coproc_t which_coproc)
{
  csi_desc_t  *csi = csi_descs[which_coproc];

  return  csi->send_timeout;
}


csi_status_t
csi_set_feature (csi_coproc_t which_coproc, coproc_feature_t feature)
{
  csi_desc_t  *csi = csi_descs[which_coproc];

  csi->feature = feature;

  return CSI_STATUS_OK;
}

coproc_feature_t
csi_get_feature (csi_coproc_t which_coproc)
{
  csi_desc_t  *csi = csi_descs[which_coproc];

  return csi->feature;
}


csi_status_t
csi_clr_feature (csi_coproc_t which_coproc, coproc_feature_t feature)
{
  csi_desc_t  *csi = csi_descs[which_coproc];

  csi->feature &= ~(feature);

  return CSI_STATUS_OK;
}


void
csi_info (csi_coproc_t which_coproc)
{
  fw_print_version (which_coproc, &(csi_descs[which_coproc]->fw_desc));
}

////////////////////////////////////////////////////////////////////////////////
// allocate shared memory
//
static void *
allocate_shared_memory(uint32_t capacity, uint32_t alignment)
{
  uint32_t       actual_capacity;
  void           *sm;

  // We always allocate page-aligned and with multiples of the page size for alignment
  actual_capacity = roundup(capacity, PAGE_SIZE);
  alignment       = roundup(alignment, PAGE_SIZE);

  // allocate
  sm = memalign(actual_capacity, alignment);
  maybe_do_cache_operation(CACHE_CLEAN, sm, actual_capacity);

  return sm;
}


static void
free_shared_memory(void *sm)
{
  free(sm);
}

static endpoint_desc_t *
find_endpoint(csi_coproc_t which_coproc, ipc_endpoint_t which_ep)
{
  int             j;
  csi_desc_t      *this_csi;
  endpoint_desc_t  *next_ep;

  if (which_coproc < CSI_COPROC_MAX) {
    this_csi = csi_descs[which_coproc];
    if (this_csi != NULL) {
      next_ep = this_csi->endpoints;
      for (j=this_csi->endpoint_count; --j>=0; next_ep++) {
        if (next_ep->index == which_ep) {
          return next_ep;
        }
      }
    }
  }

  return NULL;
}


////////////////////////////////////////////////////////////////////////////////
// inbox_not_empty_task
//
// Wait for and dispatch incoming messages from the IOP.
// Get all the pending message in the mailbox and dispatch them to the appropriate
// endpoint. Message to non existent endpoint are silently discarded.
//
static int
inbox_not_empty_task(void *token)
{
  volatile csi_desc_t  *csi;
  int                  status;
  msg_payload_t        msg;
  endpoint_desc_t      *ep;

  REQUIRE (NULL != (csi = (csi_desc_t*)token));

  if (csi->state != COPROC_STATE_RUNNING) {
    CSI_LOG(CRITICAL, (csi_coproc_t)csi->coproc, "error: inbox_not_empty_task() coproc not running\n");
    return 1;
  }

  // we should only be trying to read message when the AKF is running otherwise data abort can happen.
  while (COPROC_STATE_RUNNING == csi->state) {
    // check the coproc status every 100ms
    while ((status = akf_recv_mbox(KFW(csi->coproc), &msg, 100000)) == 0) {

      // queue the message, the endpoint will ignore it if disabled.
      ep = find_endpoint(csi->coproc, ipc_msg_get_endpoint(&msg));

      if (ep) {
        message_dispatcher(ep, (msg&IPC_MSG_PAYLOAD_MASK));
      } else {
        CSI_LOG(CRITICAL, (csi_coproc_t)csi->coproc, "error: message received for unknown endpoint => message discarded (%d, 0x%llx)\n", ipc_msg_get_endpoint(&msg), msg);
      }
    }
  }

  // coproc has stopped running, terminate task
  return 0;
}

static void
message_dispatcher(endpoint_desc_t *ep, msg_payload_t msg)
{
  // do some basic filtering before signaling
  if (!ep->registered) {
    if (check_log_gov(&csi_descs[ep->coproc]->gov_ep_not_reg, ep->coproc)) {
      CSI_LOG(CRITICAL, ep->coproc, "error: %s endpoint not registered => message discarded\n", ep->name);
    }

  } else {

    // add the message to the queue

    ep->rcv_queue[ep->next_in] = msg;
    EP_QUEUE_ADVANCE(ep->next_in, ep->rcv_queue_size);

    // check for overflow. discard oldest message if it happens.
    if (ep->next_in == ep->next_out) {
      if (check_log_gov(&csi_descs[ep->coproc]->gov_ep_queue_ovflw, ep->coproc)) {
        CSI_LOG(CRITICAL, ep->coproc, "error: %s endpoint queue overflow\n", ep->name);
      }
      EP_QUEUE_ADVANCE(ep->next_out, ep->rcv_queue_size);
    }

    // if this was the first message move the _nextOut pointer to the newly received message
    if ((uint32_t)~(0) == ep->next_out) {
      ep->next_out = 0;
    }

    // signal the endpoint that its rcv queue is not empty
    REQUIRE(ep->rcv_not_empty_event!=NULL);
    event_signal(ep->rcv_not_empty_event);
  }
}


// endpoint support

// NOTE: an unregister method could be added if necessary

csi_status_t
csi_register_endpoint(csi_coproc_t which_coproc, uint32_t which_ep, struct task_event *event, void **return_token, const char **name)
{
  endpoint_desc_t    *ep;

  ep = find_endpoint(which_coproc, (ipc_endpoint_t)which_ep);

  if (ep==NULL) {
    CSI_LOG(CRITICAL, which_coproc, "csi_register_endpoint(): no knowledge of endpoint #%d\n", which_ep);
    return CSI_STATUS_UNSUPPORTED;
  }

  if (ep->registered) {
    CSI_LOG(CRITICAL, which_coproc, "csi_register_endpoint(): %s endpoint already registered\n", ep->name);
    return CSI_STATUS_ERROR;
  }

  if (name != NULL) {
    *name = ep->name;
  }

  ep->rcv_not_empty_event = event;
  ep->registered = true;
  ep->sm_alignment = csi_descs[which_coproc]->fw_desc.alignment_req;
  *return_token = (void*)ep;

  return CSI_STATUS_OK;
}


void *
csi_allocate_shared_memory(void *token, uint32_t capacity)
{
  endpoint_desc_t *ep = (endpoint_desc_t*)token;
  void  *sm;

  sm = allocate_shared_memory(capacity, ep->sm_alignment);

  return sm;
}


void
csi_free_shared_memory(void *sm)
{
  free_shared_memory (sm);

  // remove element from the list
}


csi_status_t
csi_receive_message(void *token, msg_payload_t *msg_payload)
{
  endpoint_desc_t *ep = (endpoint_desc_t*)token;

  REQUIRE(ep != NULL);

  if (!ep->registered) {
    return CSI_STATUS_ERROR;
  }

  // make sure we have a message available
  if (((uint32_t)~(0) == ep->next_out) || (ep->next_out == ep->next_in)) {
    return CSI_STATUS_UNAVAILABLE;
  }

  // get the message from ep input queue and return it.
  *msg_payload = ep->rcv_queue[ep->next_out];
  EP_QUEUE_ADVANCE(ep->next_out, ep->rcv_queue_size);

  return CSI_STATUS_OK;
}


csi_status_t
csi_send_message (void *token, msg_payload_t msg_payload)
{
  endpoint_desc_t   *ep;
  ipc_msg_t         msg_out;
  int               status;
  csi_desc_t        *csi;

  REQUIRE(token != NULL);

  ep = (endpoint_desc_t*)token;
  msg_out = (ipc_msg_t)msg_payload;

  if (!ep->registered) {
    return CSI_STATUS_ERROR;
  }

  // FixMe - some clients already put in the endpoint field - others expect it to be done for them
  if ((msg_out & IPC_MSG_ENDPOINT_MASK) == 0) {
    ipc_msg_set_endpoint(&msg_out, ep->index);
  }

  if (ipc_msg_get_endpoint(&msg_out) != ep->index) {
    return CSI_STATUS_ERROR;
  }

  ipc_msg_set_endpoint(&msg_out, ep->index);

  csi = csi_descs[ep->coproc];

  status = akf_send_mbox(KFW(ep->coproc), msg_out, csi->send_timeout);
  if (-2 == status) {
    // timeout
    CSI_LOG(CRITICAL, ep->coproc, "csi_send_message(): failed to send message %llx in %dus, AKF hanged?\n", msg_payload, csi->send_timeout);
    return CSI_STATUS_TIMEOUT;
  }
  // all other errors are non fatal
  if (0 != status) {
    return CSI_STATUS_ERROR;
  }
  return CSI_STATUS_OK;
}


static bool
check_log_gov(log_governor_t *gov, csi_coproc_t which_coproc)
{
  gov->occ_count++;

  if ((gov->occ_count == 1) || (timer_ticks_to_usecs(timer_get_ticks() - gov->last_log) >= LOG_GOV_PERIOD_USEC)) {
    if (gov->occ_count > 2) {
      CSI_LOG(CRITICAL, which_coproc, "WARNING: The condition logged below has occurred %d times\n", (gov->occ_count-1));
    }
    gov->occ_count = 1;
    gov->last_log = timer_get_ticks();
    return true;
  }

  return false;
}


void maybe_do_cache_operation(int operation, void *address, u_int32_t length)
{
#if WITH_NON_COHERENT_DMA
  void        *aligned_address;
  u_int32_t   aligned_length;

  aligned_address = (void*)((uintptr_t)address & ~(CPU_CACHELINE_SIZE-1));
  aligned_length = roundup((length + (address - aligned_address)), CPU_CACHELINE_SIZE);

  platform_cache_operation(operation, aligned_address, aligned_length);
#endif
}
