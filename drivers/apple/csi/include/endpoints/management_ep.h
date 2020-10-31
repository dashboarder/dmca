/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#pragma once

#include <csi_ipc_protocol.h>
#include <csi_mgmt_protocol.h>


// FixMe - auto-pinger not implemented
// pinger
typedef enum {
  PINGER_DISABLED = 0x0000,
  PINGER_OK       = 0x0001,
  PINGER_TIMEOUT  = 0x1000,
  PINGER_SEQ_ERR  = 0x1001,
  PINGER_XMIT_ERR = 0x1002,
} pinger_status_t;

typedef struct {
  uint32_t        xmt_count;
  uint32_t        replied_to_count;
  uint32_t        xmt_err_count;
  uint32_t        rcv_err_count;
  uint32_t        timeout_err_count;
} ping_stats_t;

typedef struct {
  uint32_t           ping_period_usec;
  uint32_t           report_period_usec;
  pinger_status_t    status;

  csi_ping_seqnum_t  xmt_seq_number;
  csi_ping_seqnum_t  expected_rcv_seq_number;
  csi_ping_seqnum_t  rcv_seq_number;

  ping_stats_t       stats;
} pinger_t;


//
// Management Object
//

typedef enum {
  MGMT_INITIALIZED = 0,
  MGMT_QUIESCED,
  MGMT_WAITING_FOR_ACK,
  MGMT_VERSION_OK,

  // error status
  MGMT_NONEXISTENT                  = 0x1000,
  MGMT_UNSUPPORTED_PROTOCOL_VERSION = 0x1001,
  MGMT_MBOX_ERROR                   = 0x1002,
} mgmt_state_t;

typedef struct {
  csi_coproc_t                coproc;
  struct task_event           msg_event;
  void                        *csi_token;
  const char                  *name;

  csi_protocol_version_t      remoteVersion;
  mgmt_state_t                state;

  // pinger
  pinger_t                    pinger;

  volatile bool               ping_test_waiting;
  volatile csi_ping_seqnum_t  ping_test_seqnum;
  volatile ipc_msg_t          ping_test_ack;

  // power
  struct task_event           *ps_event;
  csi_power_state_t           power_state;
} ep_mgmt_desc_t;

extern ep_mgmt_desc_t *mgmt_descs[];


//
// Endpoint API
//

void         bi_ep_mgmt_create    (csi_coproc_t which_coproc, struct task_event *ps_event);
csi_status_t bi_ep_mgmt_send_ps   (csi_coproc_t which_coproc, csi_power_state_t ps);
csi_status_t bi_ep_mgmt_send_ping (csi_coproc_t which_coproc, int ping_count);
mgmt_state_t bi_ep_mgmt_get_state (csi_coproc_t which_coproc);

csi_power_state_t bi_ep_mgmt_get_power_state (csi_coproc_t which_coproc);

// to support panic recovery
csi_status_t handle_power_state_ack (ep_mgmt_desc_t *mgmt, ipc_msg_t msg_in);
