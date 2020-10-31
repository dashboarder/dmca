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

//
// Console object
//

typedef struct {
  csi_coproc_t         coproc;
  struct task_event    msg_event;
  void                 *csi_token;
  const char           *name;
  uint32_t             verbosity;
  void                 *buffers;
  uint32_t             allocated;
  char                 *scratch;
} ep_console_desc_t;


//
// Console APIs
//

void          bi_ep_console_create          (csi_coproc_t which_coproc);
csi_status_t  bi_ep_console_send_custom_cmd (csi_coproc_t which_coproc, uint32_t payload);
csi_status_t  bi_ep_console_set_verbosity   (csi_coproc_t which_coproc, uint32_t verbosity);

csi_status_t  console_quiesce (csi_coproc_t which_coproc, csi_power_state_t ps);
