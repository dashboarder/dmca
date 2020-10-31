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
#ifndef __CSI_PRIVATE_H
#define __CSI_PRIVATE_H

#include <drivers/csi.h>
#include <debug.h>

// NOTE:  The following header file is expected to be found via the search paths expressed in
// rules.mk via the GLOBAL_INCLUDES macro.
#include <csi_prot_common.h>
#include <csi_platform_defines.h>
#include <csi_system.h>

// build-time CSI protocol version range check
// Make sure you could this in sync with the protoc
#define EXPECTED_RANGE_MIN_PROTOCOL_VER_MAJOR 9
#define EXPECTED_RANGE_MAX_PROTOCOL_VER_MAJOR 9

#if !((EXPECTED_RANGE_MIN_PROTOCOL_VER_MAJOR <= APPLE_CSI_PROTOCOL_VER_MAJOR) && (EXPECTED_RANGE_MAX_PROTOCOL_VER_MAJOR >= APPLE_CSI_PROTOCOL_VER_MAJOR))
#error ERROR: This iBoot is not compatible with your AppleStorageProcessor firmware
#endif


// explicitly specify task stack sizes
// empirically derived stack high watermark measurements
// csi inbox_not_e:  512
// csi ep mgmt:      416
// csi ep crashlog:  1728
// csi ep console:   448
// csi ep syslog:    1424

// request 8 kb to cover with a healthy margin
// further tuning will be handled with <rdar://problem/15208515> Analyze stack usage and tune stack size for csi iBoot tasks

#define kTaskStackSize_inbox_not_empty  0x2000
#define kTaskStackSize_endpoints        0x2000




// default send message time out
#define CSI_SEND_MESSAGE_TIMEOUT  (10 * 1000 * 1000) // 10s


#define roundup(x,y)        ((((x)+(y)-1)/(y))*(y))

#define is_power_of_2(x)    (((x)==((x)&~((x)-1)))&&((x)!=0))


#ifndef REQUIRE
#define REQUIRE(_expr)                                      \
            do {                                            \
                if (__builtin_expect(!(_expr), 0))          \
                    panic("%s:%s:%u: REQUIRE failed: %s\n", \
                        __FILE__,                           \
                        __PRETTY_FUNCTION__,                \
                        __LINE__, #_expr);                  \
            } while(0);
#endif


#define COPROC_STATE(s) \
  (((s)==MGMT_INITIALIZED) ?                          "Initialized" : \
    (((s)==MGMT_QUIESCED) ?                           "Quiesced" : \
      (((s)==MGMT_WAITING_FOR_ACK) ?                  "Waiting For Hello" : \
        (((s)==MGMT_VERSION_OK) ?                     "Running" : \
          (((s)==MGMT_UNSUPPORTED_PROTOCOL_VERSION) ? "Running - Unsupported Protocol Version" : \
            (((s)==MGMT_NONEXISTENT) ?                "Uninitialized" : "???"))))))



  // CS_LOG levels are passed through to dprintf() and compared against DEBUG_LEVEL.
  // <debug.h> defines { DEBUG_SILENT DEBUG_CRITICAL DEBUG_INFO DEBUG_SPEW }
  // one may use DEBUG_LEVEL to enable output and (DEBUG_LEVEL+1) to stifle output

  // these levels should probably be enabled by default
#define CSI_LOG_LEVEL_CRITICAL    DEBUG_CRITICAL    // indication of a serious problem

  // these levels *may* be enabled by default
#define CSI_LOG_LEVEL_CRASHLOG    DEBUG_CRITICAL    // coproc crashlog output
#define CSI_LOG_LEVEL_CONSOLE     DEBUG_INFO        // coproc console output
#define CSI_LOG_LEVEL_SYSLOG      DEBUG_CRITICAL    // coproc syslog output
#define CSI_LOG_LEVEL_DEBUG_SPEW  DEBUG_SPEW        // verbose debug output

  // these levels are disabled by default
#if DEBUG_BUILD
#define CSI_LOG_LEVEL_DEBUG       DEBUG_INFO
#else
#define CSI_LOG_LEVEL_DEBUG       DEBUG_CRITICAL        // every little operational detail - TMI - useful for debugging only
#endif


  // csi log formatting macros
#define CSI_LOG_RAW(lev, x...) \
  do { dprintf(CSI_LOG_LEVEL_##lev, x); } while(0)

#define CSI_LOG(lev, cop, x...) \
  do { dprintf(CSI_LOG_LEVEL_##lev, "%s: ", IOP_NAME(cop)); dprintf(CSI_LOG_LEVEL_##lev, x); } while(0)

#define CSI_EP_LOG(lev, cop, ep_name, x...) \
  do { dprintf(CSI_LOG_LEVEL_##lev, "[%s %s] ", IOP_NAME(cop), ep_name); dprintf(CSI_LOG_LEVEL_##lev, x); } while(0)


  // firmware api

typedef struct {
  uint32_t                  alignment_req;
  void                      *fw_address;
  uint32_t                  fw_size;
  void                      *heap_address;
  uint32_t                  heap_size;
  addr_t                    region_base;
  uint32_t                  region_size;
} fw_desc_t;

csi_status_t  fw_fixup_and_relocate (csi_coproc_t which_coproc, fw_desc_t *fw_desc);
void          fw_print_version (csi_coproc_t coproc, fw_desc_t *fw_desc);

csi_status_t  csi_start (csi_coproc_t which_coproc);
csi_status_t  csi_stop  (csi_coproc_t which_coproc);
csi_status_t  csi_panic_recover (csi_coproc_t which_coproc);
csi_status_t  quiesce_iop   (csi_coproc_t which_coproc);
csi_status_t  hibernate_iop (csi_coproc_t which_coproc);
csi_status_t  request_iop_power_state (csi_coproc_t which_coproc, csi_power_state_t ps);
csi_status_t  set_send_timeout (csi_coproc_t which_coproc, uint32_t timeout);
uint32_t      get_send_timeout (csi_coproc_t which_coproc);
void          csi_info (csi_coproc_t which_coproc);


csi_status_t  csi_set_feature (csi_coproc_t which_coproc, coproc_feature_t feature);
csi_status_t  csi_clr_feature (csi_coproc_t which_coproc, coproc_feature_t feature);

void          maybe_do_cache_operation(int operation, void *address, u_int32_t length);


//
// built-in endpoint api
//

csi_status_t  bi_ep_send_message (void *token, msg_payload_t msg_payload);

#endif /* !defined(__CSI_PRIVATE_H) */
