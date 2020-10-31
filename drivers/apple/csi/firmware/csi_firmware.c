/*
 * Copyright (C) 2012 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#include <platform.h>
#include <platform/memmap.h>
#include <arch.h>
#include <debug.h>
#include <sys/task.h>
#include <sys/boot.h>
#include <lib/env.h>
#include <platform/timer.h>
#include <drivers/a7iop/a7iop.h>
#include <platform/clocks.h>
#include <platform/chipid.h>
#include <platform/soc/hwclocks.h>
#include <drivers/csi.h>
#include <drivers/power.h>

#include "csi_private.h"
#include "csi_firmware.h"

#include <csi_heap_map.h>
#include <csi_platform_config.h>
#include <csi_ipc_protocol.h>
#include <csi_mgmt_protocol.h>


extern const get_fw_info_t fw_info_ans;

static const get_fw_info_t *fw_info_getters[CSI_COPROC_MAX] = {
    &fw_info_ans,
    NULL,
    NULL,
    NULL,
};


csi_status_t
fw_fixup_and_relocate (csi_coproc_t coproc, fw_desc_t *fw_desc)
{
  const get_fw_info_t   *fw_info;
  void                  *heap_base;
  uint32_t              heap_size_max;
  volatile CsiConfig_t  *csi_config;

  fw_info = fw_info_getters[coproc];

  if (fw_info == NULL) {
    return CSI_STATUS_NO_FIRMWARE;
  }

  // Get the csi configuration structure and validate it.
  // This structure contain all the relevant firmware information needed for relocation.
  // Use it as read only until the firmware has been relocated
  if (fw_info->get_csi_config_offset() == VALUE_NOT_FOUND) {
    CSI_LOG (CRITICAL, coproc, "coproc firmware does not specifiy a fix-up location for the csi configuration structure\n");
    return CSI_STATUS_BAD_FIRMWARE;
  }
  csi_config = (volatile CsiConfig_t*)(fw_info->get_fw_image() + fw_info->get_csi_config_offset());
  if (CSI_CONFIG_SIGNATURE != csi_config->signature) {
    CSI_LOG (CRITICAL, coproc, "Invalid csi config signature: 0x%x != 0x%x\n", csi_config->signature, CSI_CONFIG_SIGNATURE);
    return CSI_STATUS_BAD_FIRMWARE;
  }
  if (CSI_CONFIG_VERSION != csi_config->version) {
    CSI_LOG (CRITICAL, coproc, "Invalid csi config version: 0x%x != 0x%x\n", csi_config->version, CSI_CONFIG_VERSION);
    return CSI_STATUS_BAD_FIRMWARE;
  }

  // Validate the firmware size
  fw_desc->fw_address = (void*)(fw_desc->region_base);
  fw_desc->fw_size    = csi_config->fwRuntimeSize;
  if (fw_desc->fw_size > fw_desc->region_size) {
    CSI_LOG (CRITICAL, coproc, "firmware size (0x%x) is too large (max size=0x%x)\n", fw_desc->fw_size, fw_desc->region_size);
    return CSI_STATUS_OUT_OF_RESSOURCES;
  }

  // Validate the alignment. We only support up to a page size, adjust if needed.
  if (!is_power_of_2(csi_config->fwAlignmentRequired)) {
    CSI_LOG (CRITICAL, coproc, "firmware alignment requirement (0x%x) is not a power of 2\n", csi_config->fwAlignmentRequired);
    return CSI_STATUS_BAD_FIRMWARE;
  }
  if (csi_config->fwAlignmentRequired >  PAGE_SIZE) {
    fw_desc->alignment_req = PAGE_SIZE;
    CSI_LOG (DEBUG, coproc, "Requested alignment (0x%x) is greater than a page size (0x%x) - adjusting to 0x%x\n",
             csi_config->fwAlignmentRequired, PAGE_SIZE, fw_desc->alignment_req);
  } else {
    fw_desc->alignment_req = csi_config->fwAlignmentRequired;
  }

  // copy the fixed up firmware, zero'ing the targeted memory first.
  memset (fw_desc->fw_address, 0, fw_desc->fw_size);
  memcpy (fw_desc->fw_address, fw_info->get_fw_image(), fw_info->get_fw_image_size());

  CSI_LOG (CRITICAL, coproc, "firmware: %s-%s %s\n", fw_info->get_name_str(), fw_info->get_build_str(), fw_info->get_version_str());
  CSI_LOG (DEBUG_SPEW, coproc, "  @phys = %p, image size = 0x%x, binary size = 0x%x\n",
           (void *)mem_static_map_physical((uintptr_t)fw_desc->fw_address), fw_info->get_fw_image_size(), fw_desc->fw_size);

  // Get the relocated config structure and validate it.
  // It is now read write
  csi_config = (volatile CsiConfig_t*)(fw_desc->fw_address + fw_info->get_csi_config_offset());
  if (CSI_CONFIG_SIGNATURE != csi_config->signature) {
    CSI_LOG (CRITICAL, coproc, "Invalid csi config signature after relocation: %x != %x\n", csi_config->signature, CSI_CONFIG_SIGNATURE);
    return CSI_STATUS_ERROR;
  }

  // Fix up the soc information
  csi_config->soc          = chipid_get_chip_id();
  csi_config->socRevision  = chipid_get_chip_revision();
  csi_config->nClock       = clock_get_frequency (CLK_NCLK);
  csi_config->ancLinkClock = clock_get_frequency (CLK_ANS_LINK);

  // determine base and max size of heap
  heap_base     = (void*)roundup(((uint64_t)fw_desc->fw_address + fw_desc->fw_size), fw_desc->alignment_req);
  heap_size_max = (uint32_t)(fw_desc->region_size - (uint32_t)(heap_base - fw_desc->fw_address));

  fw_desc->heap_address = NULL;
  fw_desc->heap_size = 0;

  // Set up the heap
  if (csi_config->heapMap.numHeap > 0) {
    // Initialize heap map.
    for (int j = 1; j < (int)csi_config->heapMap.numHeap; j++) {
      csi_config->heapMap.heap[j].pbase = 0;
      csi_config->heapMap.heap[j].size = 0;
    }

    if (heap_size_max < csi_config->heapRequired) {
      CSI_LOG (DEBUG_SPEW, coproc, "requested heap size of (0x%x) exceeds available size (0x%x) - proceeding with smaller heap!\n", csi_config->heapRequired, heap_size_max);
    }

    // the IOP takes remapped address for its heap so substract the region base
    csi_config->heapMap.heap[0].pbase = (uint64_t)(heap_base - fw_desc->region_base);
    csi_config->heapMap.heap[0].size  = ((heap_size_max < csi_config->heapRequired) ? heap_size_max : csi_config->heapRequired);

    // store real address for the iBoot side
    fw_desc->heap_address = (void*)heap_base;
    fw_desc->heap_size    = csi_config->heapMap.heap[0].size;

    CSI_LOG (DEBUG_SPEW, coproc, "Heap @ physical %p (remapped %p), size= 0x%x\n", fw_desc->heap_address, (void *)csi_config->heapMap.heap[0].pbase, csi_config->heapMap.heap[0].size);
  }

  // Fix up whether early-boot tracing is enabled
  uint8_t iboot_debug = 0;
  power_get_nvram(kPowerNVRAMiBootDebugKey, &iboot_debug);
  uint32_t early_debug_val = ((iboot_debug & kPowerNVRAMiBootDebugEarlyTracing) ? 1:0);

  if (early_debug_val) {
    CSI_LOG (CRITICAL, coproc, "Enabling early boot tracing as requested via powernvram.\n");
  }
  csi_config->kDebugEarlyDebug = early_debug_val;

  maybe_do_cache_operation(CACHE_CLEAN, fw_desc->fw_address, fw_desc->fw_size);
  maybe_do_cache_operation(CACHE_CLEAN, heap_base, fw_desc->heap_size);

  return CSI_STATUS_OK;
}


void
fw_print_version (csi_coproc_t coproc, fw_desc_t *fw_desc)
{
  const get_fw_info_t  *fw_info;
  const CsiConfig_t    *csi_config;

  // print version information
  fw_info    = fw_info_getters[coproc];
  csi_config = (CsiConfig_t*)(fw_desc->fw_address + fw_info->get_csi_config_offset());

  CSI_LOG (CRITICAL, coproc, "firmware: %s-%s %s @phys = %p\n",
                             fw_info->get_name_str(), fw_info->get_build_str(), fw_info->get_version_str(),
                             (void *)mem_static_map_physical((uintptr_t)fw_desc->fw_address));
  CSI_LOG (CRITICAL, coproc, "protocol version %x.%x on soc %x.%x\n",
                             csi_mgmt_get_prot_major(csi_config->version), csi_mgmt_get_prot_minor(csi_config->version),
                             csi_config->soc, csi_config->socRevision);
}
