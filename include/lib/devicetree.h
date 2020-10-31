/*
 * Copyright (C) 2008-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _BOOTX_DEVICE_TREE_H_
#define _BOOTX_DEVICE_TREE_H_

#include <stdint.h>
#include <sys.h>
#include <sys/types.h>

__BEGIN_DECLS

#define kPropNameLength (32)
#define kPropValueMaxLength (0x100000)
typedef struct DTProperty DTProperty, *DTPropertyPtr;

int dt_load(void);
int dt_load_file(const char *path);

extern addr_t gDeviceTreeAddr;
extern size_t gDeviceTreeSize;

typedef struct dtnode dt_node_t;

void dt_init(void);
dt_node_t *dt_deserialize(void *dtaddr, size_t dtlength);
void dt_serialize(dt_node_t *dt, void *buffer, size_t bufferlen);
size_t dt_get_size(void);
dt_node_t *dt_get_root(void);
bool dt_find_node(dt_node_t *root, const char *path, dt_node_t **node);
const char *dt_get_node_name(dt_node_t *node);
bool dt_get_prop(dt_node_t *node, char **prop_name, void **prop_data, uint32_t *prop_size);
bool dt_find_prop(dt_node_t *node, const char *prop_name, void **prop_data, uint32_t *prop_size);
bool dt_has_prop(dt_node_t *node, const char *prop_name);
void dt_set_prop(dt_node_t *node, const char *name, const void *data, uint32_t size);
void dt_set_prop_32(dt_node_t *node, const char *name, uint32_t value);
void dt_set_prop_64(dt_node_t *node, const char *name, uint64_t value);
void dt_set_prop_addr(dt_node_t *node, const char *name, uintptr_t value);
void dt_set_prop_str(dt_node_t *node, const char *name, const char *str);
bool dt_remove_prop(dt_node_t *node, const char *name);
bool dt_rename_prop(dt_node_t *node, const char *name, const char *new_name);
void dt_seal(void);
int dt_num_children(dt_node_t *node);

dt_node_t *dt_get_child(dt_node_t *node, int n);
dt_node_t *dt_get_next(dt_node_t *node);

// compatibility with legacy code
typedef dt_node_t DTNode;
typedef dt_node_t * DTNodePtr;
#define FindNode dt_find_node
#define FindProperty dt_get_prop

__END_DECLS

#endif /* ! _BOOTX_DEVICE_TREE_H_ */
