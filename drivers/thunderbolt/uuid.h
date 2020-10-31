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
#ifndef UUID_H
#define UUID_H

typedef struct uuid {
	uint32_t	time_low;
	uint16_t	time_mid;
	uint16_t	time_hi_and_version;
	uint8_t		clock_seq_hi_and_reserved;
	uint8_t		clock_seq_low;
	uint8_t		node[6];
} uuid_t;

void uuid_host_to_network(const uuid_t *source, uint8_t *dest);
void uuid_network_to_host(const uint8_t *src, uuid_t *dest);
void uuid_host_to_tbt(const uuid_t *source, uint8_t *dest);
void uuid_tbt_to_host(const uint8_t *source, uuid_t *dest);
void uuid_generate_v5(const uint8_t *hash, uuid_t *dest);
const uuid_t *uuid_get_device_uuid(void);
void uuid_print(const uuid_t *src);

#endif
