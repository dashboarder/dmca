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

#pragma once

#include <platform.h>
#include <platform/memmap.h>
#include <arch.h>
#include <debug.h>
#include <sys/task.h>
#include <drivers/a7iop/a7iop.h>
#include <drivers/csi.h>

#include "csi_private.h"


#define VALUE_NOT_FOUND ((uint32_t)-1)

typedef const char* (*get_name_str_t)(void);
typedef const char* (*get_build_str_t)(void);
typedef const char* (*get_version_str_t)(void);
typedef const char* (*get_fw_image_t)(void);
typedef uint32_t    (*get_fw_image_size_t)(void);
typedef uint32_t    (*get_csi_config_offset_t)(void);
typedef uint32_t    (*get_fw_mem_req_t)(void);
typedef void*       (*get_reloc_address_t)(void);
typedef uint32_t    (*get_reloc_size_t)(void);

typedef struct {
  get_name_str_t                  get_name_str;
  get_build_str_t                 get_build_str;
  get_version_str_t               get_version_str;
  get_fw_image_t                  get_fw_image;
  get_fw_image_size_t             get_fw_image_size;
  get_csi_config_offset_t         get_csi_config_offset;
  get_fw_mem_req_t                get_fw_memory_required;
  get_reloc_address_t             get_reloc_address;
  get_reloc_size_t                get_reloc_size;
} get_fw_info_t;
