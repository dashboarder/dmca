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
#ifndef IPIPE_H
#define IPIPE_H

#include <stdint.h>

typedef struct ipipe ipipe_t;

typedef void (ipipe_tx_callback_t)(void *priv, const uint8_t *packet, uint32_t bytes);

ipipe_t *ipipe_init(uint32_t max_packet_size, ipipe_tx_callback_t *tx_callback, void *priv);
void ipipe_quiesce_and_free(ipipe_t *ipipe);
void ipipe_handle_packet(ipipe_t *ipipe, const uint8_t *packet, uint32_t bytes);

#endif
