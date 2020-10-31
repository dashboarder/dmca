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
#ifndef TBT_CONTROL_PORT_H
#define TBT_CONTROL_PORT_H

#include <stdint.h>
#include <drivers/thunderbolt/nhi.h>
#include "uuid.h"

typedef struct tbt_cp tbt_cp_t;
typedef void (*tbt_xd_callback_t)(tbt_cp_t *, const uint8_t *packet, size_t len, void *priv);

tbt_cp_t *tbt_cp_create(nhi_t *nhi);
void tbt_cp_quiesce_and_free(tbt_cp_t *cp);
void *tbt_cp_register_xd_service(tbt_cp_t *cp, tbt_xd_callback_t req_cb, tbt_xd_callback_t resp_cb, void *priv);
void tbt_cp_unregister_xd_service(tbt_cp_t *cp, void *handle);

bool tbt_cp_read(tbt_cp_t *cp, uint64_t topology_id, uint8_t space, uint8_t port, uint16_t index, uint32_t *buffer, uint16_t quadlets);
bool tbt_cp_write(tbt_cp_t *cp, uint64_t topology_id, uint8_t space, uint8_t port, uint16_t index, const uint32_t *buffer, uint16_t quadlets);

bool tbt_cp_packet_available(tbt_cp_t *cp);
void tbt_cp_process_packet(tbt_cp_t *cp);
uint8_t tbt_cp_next_pdf_seq(tbt_cp_t *cp, uint8_t pdf);
bool tbt_cp_send(tbt_cp_t *cp, uint8_t pdf, uint8_t *packet, size_t len);

uint64_t tbt_cp_packet_get_route_string(const uint8_t *packet);
void tbt_cp_packet_set_route_string(uint8_t *packet, uint64_t route_string);

#endif
