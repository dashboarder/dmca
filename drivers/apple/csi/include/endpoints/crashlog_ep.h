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

// crash log type
typedef enum {
  CSI_CRASHLOG_TYPE_NMI = 0,
  CSI_CRASHLOG_TYPE_MSG_CRASH,
  CSI_CRASHLOG_TYPE_MSG_LOG,
} csi_crashlog_t;

//
// Crashlog Object
//
typedef struct {
  csi_coproc_t        coproc;
  struct task_event   msg_event;
  void                *csi_token;
  const char          *name;
  void                *buffer;
  uint32_t            allocated;
} ep_crashlog_desc_t;


//
// Public API
//

void          bi_ep_crashlog_create      (csi_coproc_t which_coproc);
csi_status_t  bi_ep_crashlog_force_event (csi_coproc_t which_coproc, csi_crashlog_t crashlog_type);

csi_status_t  crashlog_quiesce (csi_coproc_t which_coproc, csi_power_state_t ps);
