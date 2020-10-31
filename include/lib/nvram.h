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
#ifndef __LIB_NVRAM_H
#define __LIB_NVRAM_H

#include <sys/types.h>
#include <lib/devicetree.h>

__BEGIN_DECLS

int nvram_load(void);
int nvram_save(void); // write out the environment
int nvram_set_panic(const void *base, size_t length); // copy panic log to staging
int nvram_update_devicetree(dt_node_t *node, const char *propname); // copy nvram content to devicetree

__END_DECLS

#endif

