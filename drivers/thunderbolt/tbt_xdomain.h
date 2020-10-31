/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef TBT_XDOMAIN_H
#define TBT_XDOMAIN_H

#include "tbt_control_port.h"
#include "uuid.h"

typedef struct tbt_xd_discovery tbt_xd_discovery_t;

tbt_xd_discovery_t *tbt_xd_discovery_create(tbt_cp_t *cp, const uint32_t *configuration_rom, uint16_t dwords, uint32_t generation);
void tbt_xd_discovery_quiesce_and_free(tbt_xd_discovery_t *xdd);
void tbt_xd_discovery_send_rom_changed_request(tbt_xd_discovery_t *xdd, uint64_t route);

#endif
