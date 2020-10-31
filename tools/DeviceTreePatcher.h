/*
 *  Copyright (C) 2009,2013  Apple, Inc. All rights reserved.
 *  
 *  This document is the property of Apple Inc.
 *  It is considered confidential and proprietary.
 *
 *  This document may not be reproduced or transmitted in any form,
 *  in whole or in part, without the express written permission of
 *  Apple Inc.
 */

#ifndef DEVICE_TREE_PATCHER_H
#define DEVICE_TREE_PATCHER_H	1

#include <stdint.h>

// Helper for patching a device tree. Beware that this modifies
// globals (e.g gDeviceTree) as a side-effect of its methods.
class DeviceTreePatcher {
 public:
  DeviceTreePatcher(char *base, size_t devtree_size);

  // Use one of the reserved chosen/memory-map nodes to record the location
  // of a loaded or allocated memory section. Returns true on success, or
  // false if this fails to claim an unused reserved node.
  bool AllocateMemoryRange(const char *name, uint64_t pmaddr, uint64_t size);

  // Set an integer property. Returns true on success, or false if the
  // property cannot be found, or the value is too large to fit.
  bool SetPropertyInt(const char *node, const char *property, uint64_t value);

  // Set an integer property. Returns true on success, or false if the
  // property cannot be found, or the value is too large to fit.
  bool SetPropertyTwoQuads(const char *node, const char *property, uint64_t *value);

  // Set a string property. Returns true on success, or false if the
  // property cannot be found, or the value is too large to fit.
  bool SetPropertyString(const char *node, const char *property, const char *s);

  uint32_t GetNumAddressCells(const char *node_name);
  
 private:
  // Get a pointer to the storage area for a property. Returns true
  // and fills in 'data' and 'size' if found. Returns false and does
  // not modify the return arguments if not found.
  bool GetPropertyBuffer(const char *node_name,
			 const char *property_name,
			 void **data,
			 uint32_t *size);

  char *_base;
  size_t _treesize;
  uint32_t _alloc_idx;
};

#endif  // DEVICE_TREE_PATCHER_H
