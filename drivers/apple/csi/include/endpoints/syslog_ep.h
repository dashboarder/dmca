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

#if DEBUG_BUILD
#define LOG_DEFAULT LOG_DEBUG
#else
#define LOG_DEFAULT LOG_WARNING
#endif


//
// Endpoint APIs
//

void          bi_ep_syslog_create    (csi_coproc_t which_coproc);
csi_status_t  bi_ep_syslog_verbosity (csi_coproc_t which_coproc, uint32_t verbosity);

csi_status_t  syslog_quiesce (csi_coproc_t which_coproc, csi_power_state_t ps);
