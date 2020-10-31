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

//
// Private Data
//

static volatile csi_status_t  last_error = CSI_STATUS_OK;


//
// bi_ep_send_message
//
// Send message for the built-in endpoint. If the IOP becomes unreachable
// at some point stop sending message and instead directly fails to avoid
// multiple timeout error.
//
csi_status_t
bi_ep_send_message (void *token, msg_payload_t msg_payload)
{
  csi_status_t  status;

  // make sure the previous send was not a timeout
  if (CSI_STATUS_OK != last_error) {
    // the IOP is not responding, error out right away
    return last_error;
  }

  // send the message
  status = csi_send_message (token, msg_payload);
  if (CSI_STATUS_TIMEOUT == status) {
    // the IOP is not responding, save the error status
    last_error = CSI_STATUS_TIMEOUT;
  }

  return status;
}
