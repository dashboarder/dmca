/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __LIB_ENV_H
#define __LIB_ENV_H

#include <sys/types.h>

__BEGIN_DECLS

/* systemwide environment variables */
struct env_var {
	char *str;
	size_t u;

	uint32_t flags;
};

#define ENV_PERSISTENT 0x1	/* saved with 'saveenv' or not */
#define ENV_SHADOW     0x2	// hidden from env_get or not

const char *env_get(const char *name);
size_t env_get_uint(const char *name, size_t default_val);
bool env_get_bool(const char *name, bool default_val);
const struct env_var *env_get_etc(const char *name);

int env_set(const char *name, const char *val, uint32_t flags);
int env_set_uint(const char *name, size_t val, uint32_t flags);
int env_unset(const char *name);

/* supplied by code that wants to blacklist environment variables */
bool env_blacklist(const char *name, bool write);
bool env_blacklist_nvram(const char *name);

/* used by the nvram system */
size_t env_serialize(uint8_t *buf, size_t buf_len);
int env_unserialize(const uint8_t *buf, size_t buf_len);

/* get an ip address from the environment */
int env_get_ipaddr(const char *name, uint32_t *ip);
int env_get_ethaddr(const char *name, uint8_t ethaddr[6]);

__END_DECLS

#endif

