/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __CSI_H
#define __CSI_H

#include <sys/task.h>
#include <sys/boot.h>

//
// Public API
//

typedef enum {
  CSI_COPROC_ANS = 0,
  CSI_COPROC_SEP,
  CSI_COPROC_ISP,
  CSI_COPROC_SIO,
  CSI_COPROC_MAX
} csi_coproc_t;

#define IOP_NAME(coproc) \
    (((coproc)==CSI_COPROC_ANS) ? "ANS" : \
      (((coproc)==CSI_COPROC_SEP) ? "SEP" : \
        (((coproc)==CSI_COPROC_ISP) ? "ISP" : \
          (((coproc)==CSI_COPROC_SIO) ? "SIO" : "???"))))

#define KFW(coproc) \
    (((coproc)==CSI_COPROC_ANS) ? KFW_ANS : \
      (((coproc)==CSI_COPROC_SEP) ? KFW_SEP : \
        (((coproc)==CSI_COPROC_ISP) ? 0 : \
          (((coproc)==CSI_COPROC_SIO) ? KFW_SIO : 0))))

typedef enum {
  CSI_STATUS_OK = 0,
  CSI_STATUS_UNSUPPORTED,
  CSI_STATUS_UNAVAILABLE,
  CSI_STATUS_ERROR,
  CSI_STATUS_NO_FIRMWARE,
  CSI_STATUS_BAD_FIRMWARE,
  CSI_STATUS_TIMEOUT,
  CSI_STATUS_PANIC_RECOVER,
  CSI_STATUS_OUT_OF_RESSOURCES,
} csi_status_t;

#define CSI_STATUS_STR(status) \
    (((status)==CSI_STATUS_OK)                      ? "OK" : \
      (((status)==CSI_STATUS_UNSUPPORTED)           ? "UNSUPPORTED" : \
        (((status)==CSI_STATUS_UNAVAILABLE)         ? "UNAVAILABLE" : \
          (((status)==CSI_STATUS_ERROR)             ? "ERROR" : \
            (((status)==CSI_STATUS_NO_FIRMWARE)     ? "NO_FIRMWARE" : \
              (((status)==CSI_STATUS_BAD_FIRMWARE)  ? "BAD_FIRMWARE" : \
                (((status)==CSI_STATUS_TIMEOUT)     ? "TIMEOUT" : "???")))))))


typedef uint64_t msg_payload_t;


csi_status_t csi_init (csi_coproc_t which_coproc);
csi_status_t csi_late_init (void);

csi_status_t csi_quiesce (enum boot_target target);

csi_status_t csi_register_endpoint (csi_coproc_t which_coproc, uint32_t ep, struct task_event *event, void **csi_token, const char **name);

void*        csi_allocate_shared_memory (void *csi_token, uint32_t capacity);
void         csi_free_shared_memory     (void *sm);

csi_status_t csi_receive_message (void *csi_token, msg_payload_t *msg_payload);
csi_status_t csi_send_message    (void *csi_token, msg_payload_t msg_payload);

//
// coprocessor feature and status
//

#define COPROC_FEATURE_NONE                  (0x0000UL)
#define COPROC_FEATURE_SHUTDOWN_IN_PROGRESS  (0x0001UL)
#define COPROC_FEATURE_PANIC_RECOVERY        (0x0002UL)
#define COPROC_FEATURE_ALL                   (0xFFFFFFFFUL)

typedef uint32_t coproc_feature_t;

coproc_feature_t csi_get_feature (csi_coproc_t which_coproc);

static inline bool csi_is_panic_recovery (csi_coproc_t which_coproc)
{
  return (csi_get_feature(which_coproc) & COPROC_FEATURE_PANIC_RECOVERY);
}

static inline bool csi_is_shutdown_in_progress (csi_coproc_t which_coproc)
{
  return (csi_get_feature(which_coproc) & COPROC_FEATURE_SHUTDOWN_IN_PROGRESS);
}

#endif /* !defined(__CSI_H) */
