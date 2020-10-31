/*
 *  Copyright (C) 2009, 2013  Apple, Inc. All rights reserved.
 *  
 *  This document is the property of Apple Inc.
 *  It is considered confidential and proprietary.
 *
 *  This document may not be reproduced or transmitted in any form,
 *  in whole or in part, without the express written permission of
 *  Apple Inc.
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys.h>
#include <sys/types.h>
#include <assert.h>
#include <lib/devicetree.h>
#include "DeviceTreePatcher.h"

struct MemoryMapFileInfo32 {
  uint32_t paddr;
  uint32_t length;
};

struct MemoryMapFileInfo64 {
  uint64_t paddr;
  uint64_t length;
};

DeviceTreePatcher::DeviceTreePatcher(char *base, size_t devtree_size)
  : _base(base), _treesize(devtree_size),
      _alloc_idx(0) {
}

uint32_t
DeviceTreePatcher::GetNumAddressCells(const char *node_name)
{
  DTNodePtr node;
  if (!FindNode(0, node_name, &node)) {
    fprintf(stderr, "GetNumAddressCells: couldn't find \"%s\" node.\n", node_name);
    return false;
  }

  char addrCellsProp[] = "#address-cells";
  char *addrCellsPropp = &addrCellsProp[0];
  uint32_t *addrCellsData, addrCellsPropSize;

  if (!FindProperty(node, &addrCellsPropp, (void**)&addrCellsData, &addrCellsPropSize)) {
    return 0x1;
  } else {
    return *addrCellsData;
  }
}

bool DeviceTreePatcher::AllocateMemoryRange(const char *name,
					    uint64_t pmaddr,
					    uint64_t size) {
  set_device_tree((addr_t) _base, _treesize);
  bool use_wide_addrs;
  uint32_t chosenAddrCells = GetNumAddressCells("chosen");

  switch(chosenAddrCells) {
    case 0x1:
      use_wide_addrs = false;
      break;
    case 0x2:
      use_wide_addrs = true;
      break;
    default:
      fprintf(stderr, "AllocateMemoryRange: chosen->#address-cells = %x, which is not valid.\n", chosenAddrCells);
      return false;
  }

  // Find the memory-map node.
  DTNodePtr memory_map;
  if (!FindNode(0, "chosen/memory-map", &memory_map)) {
    fprintf(stderr, "AllocateMemoryRange: failed to find the /chosen/memory-map node\n");
    return false;
  }

  // Find an unused memory map node
  union {
    MemoryMapFileInfo32 pd32;
    MemoryMapFileInfo64 pd64;
  } *propData;

  char mapName[kPropNameLength];
  snprintf(mapName, kPropNameLength, "MemoryMapReserved-%d", _alloc_idx);
  char *propName = mapName;

  uint32_t propSize;
  if (!FindProperty(memory_map,
		    &propName,
		    (void **) &propData,
		    &propSize)) {
    fprintf(stderr, "AllocateMemoryRange: failed to find a /chosen/memory-map:%s property\n", mapName);
    return false;
  }
  
  fprintf(stderr, "Section %2u: 0x%llx, %llx %s\n",
	  _alloc_idx, pmaddr, size, name);
  strlcpy(propName, name, kPropNameLength);

  if (use_wide_addrs) {
    assert(propSize == 16);
    propData->pd64.paddr = pmaddr;
    propData->pd64.length = size;
  } else {
    assert(propSize == 8);
    propData->pd32.paddr = (uint32_t)pmaddr;
    propData->pd32.length = (uint32_t)size;
  }

  assert(size == (uint32_t)size);
  ++_alloc_idx;
  return true;
}

bool DeviceTreePatcher::GetPropertyBuffer(const char *node_name,
					  const char *property_name,
					  void **ret_data,
					  uint32_t *ret_size) {
  DTNodePtr node;
  if (!FindNode(0, node_name, &node)) {
    fprintf(stderr, "Couldn't find node %s\n", node_name);
    return false;
  }
  void *data;
  uint32_t size;
  if (!FindProperty(node, const_cast<char **>(&property_name), &data, &size)) {
    fprintf(stderr, "Couldn't find property %s:%s\n", node_name, property_name);
    return false;
  }
  *ret_data = data;
  *ret_size = size;
  return true;
}

bool DeviceTreePatcher::SetPropertyInt(const char *node_name,
				       const char *property_name,
				       uint64_t value) {
  void *data;
  uint32_t size;
  if (!GetPropertyBuffer(node_name, property_name, &data, &size)) return false;
  // Little-endian properties.
  char *p = (char *) data;
  for (uint32_t i = 0; i < size; ++i) {
    p[i] = value & 255;
    value >>= 8;
  }
  if (value != 0) {
    fprintf(stderr,
	    "Property %s:%s size %u too small for value 0x%" PRIX64 "\n",
	    node_name, property_name, (unsigned) size, value);
    return false;
  }
  return true;
}

bool DeviceTreePatcher::SetPropertyTwoQuads(const char *node_name, const char *property_name, uint64_t *values)
{
  void *data;
  uint32_t size;
  uint64_t value;
  if (!GetPropertyBuffer(node_name, property_name, &data, &size)) return false;

  if (size != 16) {
    fprintf(stderr,
	    "Property %s:%s size %u too small for value 0x%016llx,0x%016llx\n",
	    node_name, property_name, (unsigned) size, values[0], values[1]);
    return false;
  }

  // Little-endian properties.
  char *p = (char *) data;
  value = values[0];
  for (uint32_t i = 0; i < size/2; ++i) {
    p[i] = value & 255;
    value >>= 8;
  }

  value = values[1];
  for (uint32_t i = 0; i < size/2; ++i) {
    p[i+8] = value & 255;
    value >>= 8;
  }

  if (value != 0) {
    fprintf(stderr,
	    "Property %s:%s size %u too small for value 0x%" PRIX64 "\n",
	    node_name, property_name, (unsigned) size, value);
    return false;
  }
  return true;

}

bool DeviceTreePatcher::SetPropertyString(const char *node_name,
					  const char *property_name,
					  const char *s) {
  void *data;
  uint32_t size;
  if (!GetPropertyBuffer(node_name, property_name, &data, &size)) return false;
  char *p = (char *) data;
  uint32_t i;
  for (i = 0; i < size; ++i) {
    p[i] = s[i];
    if (s[i] == '\0') break;
  }
  if (i == size && s[i] != '\0') {
    fprintf(stderr,
	    "Property %s:%s size %u too small for string of length %u\n",
	    node_name, property_name, (unsigned) size,
	    (unsigned) strlen(s) + 1);
    return false;
  }
  return true;
}
