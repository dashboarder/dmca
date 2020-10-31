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
#include "endpoints/management_ep.h"
#include "endpoints/crashlog_ep.h"
#include "endpoints/console_ep.h"
#include "endpoints/syslog_ep.h"


//
// Global Data
//

ep_mgmt_desc_t *mgmt_descs[CSI_COPROC_MAX] = {NULL};


//
// Private functions
//

int          ep_mgmt_task (void *arg);

csi_status_t mgmt_msg_handler       (ep_mgmt_desc_t *mgmt, msg_payload_t msg);
csi_status_t handle_hello           (ep_mgmt_desc_t *mgmt, ipc_msg_t msg_in);
csi_status_t handle_power_state     (ep_mgmt_desc_t *mgmt, ipc_msg_t msg_in);
csi_status_t handle_power_state_ack (ep_mgmt_desc_t *mgmt, ipc_msg_t msg_in);
csi_status_t validate_protocol (ep_mgmt_desc_t *mgmt, csi_protocol_version_t  protocol_version);


//
// bi_ep_mgmt_create
//
// create a new management endpoint.
//
void
bi_ep_mgmt_create (csi_coproc_t which_coproc, struct task_event *ps_event)
{
  ep_mgmt_desc_t  *mgmt;

  // enforce single instance
  REQUIRE(mgmt_descs[which_coproc] == NULL);

  mgmt = malloc(sizeof(ep_mgmt_desc_t));
  REQUIRE(mgmt != NULL);

  mgmt->coproc      = which_coproc;
  mgmt->ps_event    = ps_event;
  mgmt->power_state = CSI_PS_OFF;

  mgmt_descs[which_coproc] = mgmt;

  task_start(task_create("csi ep mgmt", ep_mgmt_task, mgmt_descs[which_coproc], kTaskStackSize_endpoints));
}

mgmt_state_t
bi_ep_mgmt_get_state (csi_coproc_t which_coproc)
{
  if (mgmt_descs[which_coproc] == NULL) {
    return MGMT_NONEXISTENT;
  }

  return mgmt_descs[which_coproc]->state;
}

csi_power_state_t
bi_ep_mgmt_get_power_state (csi_coproc_t which_coproc)
{
  if (mgmt_descs[which_coproc] == NULL) {
    return CSI_PS_OFF;
  }

  return mgmt_descs[which_coproc]->power_state;
}

csi_status_t
bi_ep_mgmt_send_ps (csi_coproc_t which_coproc, csi_power_state_t ps)
{
  ep_mgmt_desc_t  *mgmt;
  msg_payload_t   ps_msg;

  mgmt = mgmt_descs[which_coproc];

  csi_mgmt_create_msg (&ps_msg, MGMT_MSG_POWER_STATE, csi_mgmt_create_ps_msg(ps));

  return csi_send_message (mgmt->csi_token, ps_msg);
}

//
// mgmt endpoint: task
//
int
ep_mgmt_task (void *arg)
{
  csi_status_t    result;
  msg_payload_t   rcv_msg;

  ep_mgmt_desc_t *mgmt = (ep_mgmt_desc_t*)arg;

  event_init(&mgmt->msg_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

  result = csi_register_endpoint(mgmt->coproc, IPC_ENDPOINT_BUILT_IN_MGMT, &mgmt->msg_event, &mgmt->csi_token, &mgmt->name);
  REQUIRE(result==CSI_STATUS_OK);

  mgmt->state = MGMT_QUIESCED;
  mgmt->remoteVersion = MGMT_UNSUPPORTED_VERSION;

  // FixMe - pinger_init(&mgmt->pinger);

  mgmt->ping_test_waiting = false;
  mgmt->ping_test_seqnum = (csi_ping_seqnum_t)0x1234;


  for (;;) {
    event_wait(&mgmt->msg_event);
    // empty the rcv queue
    while(csi_receive_message(mgmt->csi_token, &rcv_msg) == CSI_STATUS_OK) {
      mgmt_msg_handler(mgmt, rcv_msg);
    }
  }

  return 0;
}


csi_status_t
mgmt_msg_handler (ep_mgmt_desc_t *mgmt, msg_payload_t msg)
{
  csi_mgmt_msg_t  type;
  csi_status_t    status;

  type   = csi_mgmt_get_type(msg);
  status = CSI_STATUS_OK;

  // get the type and dispatch to the appropriate handler
  switch (type) {
    case MGMT_MSG_HELLO:
      status = handle_hello(mgmt, msg);
      break;

    case MGMT_MSG_HELLO_ACK:
      CSI_EP_LOG(CRITICAL, mgmt->coproc, mgmt->name, "unsupported HELLO_ACK message received: %llx\n", msg);
      break;

    case MGMT_MSG_PING:
      CSI_EP_LOG(CRITICAL, mgmt->coproc, mgmt->name, "unsupported PING message received: %llx\n", msg);
      break;

    case MGMT_MSG_PING_ACK:
      // in lieu of autonomous pinger
      if (!mgmt->ping_test_waiting) {
        CSI_EP_LOG(CRITICAL, mgmt->coproc, mgmt->name, "unexpected ping ack received\n");
      }
      mgmt->ping_test_ack = msg;
      mgmt->ping_test_waiting = false;
      break;

    case MGMT_MSG_POWER_STATE:
      status = handle_power_state(mgmt, msg);
      break;

    case MGMT_MSG_POWER_STATE_ACK:
      status = handle_power_state_ack(mgmt, msg);
      break;

    case MGMT_MSG_NOP:
      // nop message are silently discarded
      status = CSI_STATUS_OK;
      break;

    case MGMT_MSG_ENDPOINT_STATUS:
      // these should never happen

    default:
      // all other message type are currently unsupported
      // ignore them to support upward compatibility
      CSI_EP_LOG(CRITICAL, mgmt->coproc, mgmt->name, "invalid or unsupported message type received: %d\n", type);
      break;
  }

  if (CSI_STATUS_OK != status) {
    CSI_EP_LOG(CRITICAL, mgmt->coproc, mgmt->name, "failed to process message of type %d\n", type);
  }

  return status;
}


csi_status_t
handle_hello (ep_mgmt_desc_t *mgmt, ipc_msg_t msg_in)
{
  csi_status_t    status;
  msg_payload_t   msg_out;

  // Check status of the endpoint.
  // If we are not waiting for a hello message (quiesced state) ignore it
  if ((MGMT_QUIESCED != mgmt->state) && (MGMT_UNSUPPORTED_PROTOCOL_VERSION != mgmt->state)) {
    return CSI_STATUS_ERROR;
  }
  mgmt->power_state = CSI_PS_ON;

  // Validate the other side protocol version
  status = validate_protocol(mgmt, csi_mgmt_get_prot(msg_in));
  if (CSI_STATUS_OK == status) {
    // supported version - send our version in the ACK message
    if (csi_is_panic_recovery(mgmt->coproc)) {
      // set the panic recovery feature flag
      csi_mgmt_create_msg (&msg_out, MGMT_MSG_HELLO_ACK, csi_mgmt_create_hello_msg (CSI_FEATURE_PANIC_RECOVERY, APPLE_CSI_PROTOCOL_VERSION));
    } else {
      // no feature support by default.
      csi_mgmt_create_msg (&msg_out, MGMT_MSG_HELLO_ACK, csi_mgmt_create_hello_msg (CSI_FEATURE_NONE, APPLE_CSI_PROTOCOL_VERSION));
    }

    CSI_EP_LOG (DEBUG_SPEW, mgmt->coproc, mgmt->name, "coproc running, protocol version %x.%x\n",
                csi_mgmt_get_prot_major(mgmt->remoteVersion), csi_mgmt_get_prot_minor(mgmt->remoteVersion));

  } else {
    // unsupported version - signal the other side we do not support them
    CSI_EP_LOG (CRITICAL, mgmt->coproc, mgmt->name, "coproc running unsupported version %x.%x\n",
                csi_mgmt_get_prot_major(mgmt->remoteVersion), csi_mgmt_get_prot_minor(mgmt->remoteVersion));

    csi_mgmt_create_msg (&msg_out, MGMT_MSG_HELLO_ACK, MGMT_UNSUPPORTED_VERSION);
  }

  return bi_ep_send_message (mgmt->csi_token, msg_out);
}


csi_status_t
handle_power_state (ep_mgmt_desc_t *mgmt, ipc_msg_t msg_in)
{
  msg_payload_t      ps_msg;
  csi_power_state_t  ps;

  // the IOP is requesting a power state change.
  // Used for debugging purpose, might be removed later.
  ps = csi_mgmt_get_ps_state (msg_in);

  csi_mgmt_create_msg (&ps_msg, MGMT_MSG_POWER_STATE, csi_mgmt_create_ps_msg(ps));

  return bi_ep_send_message (mgmt->csi_token, ps_msg);
}


csi_status_t
handle_power_state_ack (ep_mgmt_desc_t *mgmt, ipc_msg_t msg_in)
{
  csi_power_state_t  ps = csi_mgmt_get_ps_state (msg_in);

  // transition to ON means the IOP is now running
  // not used in iBoot.
  if (CSI_PS_ON == ps) {
    return CSI_STATUS_OK;
  }

  // if the power state is not ON quiesce built in endpoints.
  syslog_quiesce(mgmt->coproc, ps);
  console_quiesce(mgmt->coproc, ps);
  crashlog_quiesce(mgmt->coproc, ps);

  // set the new power state
  mgmt->power_state = ps;
  mgmt->state       = MGMT_QUIESCED;

  CSI_EP_LOG(DEBUG, mgmt->coproc, mgmt->name, "IOP state changed to 0x%x (mgmt state=%x)\n", mgmt->power_state, mgmt->state);

  // signal the power state change event
  event_signal(mgmt->ps_event);

  return CSI_STATUS_OK;
}


csi_status_t
validate_protocol (ep_mgmt_desc_t *mgmt, csi_protocol_version_t  protocol_version)
{
  // store the remote protocol version for future use.
  mgmt->remoteVersion = protocol_version;

  // Validate the protocol version
  // Current version support: major has to match.
  if (APPLE_CSI_PROTOCOL_VERSION == protocol_version) {
    // valid version
    mgmt->state = MGMT_VERSION_OK;
    return CSI_STATUS_OK;
  } else {
    // unsupported version
    mgmt->state = MGMT_UNSUPPORTED_PROTOCOL_VERSION;
    return CSI_STATUS_UNSUPPORTED;
  }
}


csi_status_t
bi_ep_mgmt_send_ping(csi_coproc_t which_coproc, int ping_count)
{
  int                 j;
  ep_mgmt_desc_t      *mgmt;
  u_int64_t           cum_start_ticks;
  u_int64_t           per_ping_start_ticks;
  ipc_msg_t           ping_msg;
  csi_status_t        status;

  mgmt = mgmt_descs[which_coproc];

  if (mgmt==NULL) {
    return CSI_STATUS_UNAVAILABLE;
  }

  cum_start_ticks = timer_get_ticks();

  for ( j=ping_count; --j>=0; ) {
    mgmt->ping_test_waiting = true;
    mgmt->ping_test_seqnum++;

    csi_mgmt_create_msg (&ping_msg, MGMT_MSG_PING, csi_mgmt_create_ping_msg(mgmt->ping_test_seqnum, (csi_ping_timestamp_t)timer_get_ticks()));
    status = bi_ep_send_message (mgmt->csi_token, ping_msg);
    if (CSI_STATUS_OK != status) {
      return status;
    }

    per_ping_start_ticks = timer_get_ticks();

    while (mgmt->ping_test_waiting) {
      if (timer_ticks_to_usecs(timer_get_ticks()-per_ping_start_ticks) >= 500000) {
        // ping ack not received within expected period
        // FixMe - expected period is warped for fastsim's notion of time
        mgmt->ping_test_waiting = false;
        return CSI_STATUS_TIMEOUT;
      }
      task_yield();
    }

    if (csi_mgmt_get_ping_seqnum(mgmt->ping_test_ack) != mgmt->ping_test_seqnum) {
      return CSI_STATUS_ERROR;
    }
  }

  if (ping_count == 1) {
    CSI_EP_LOG(CRITICAL, mgmt->coproc, mgmt->name, "ping rtt: %lld usec\n", timer_ticks_to_usecs(timer_get_ticks()-cum_start_ticks));
  } else {
    CSI_EP_LOG(CRITICAL, mgmt->coproc, mgmt->name, "%d pings cumulative rtt: %lld usec\n", ping_count, timer_ticks_to_usecs(timer_get_ticks()-cum_start_ticks));
  }

  return CSI_STATUS_OK;
}
