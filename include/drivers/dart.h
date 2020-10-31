/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _DART_H
#define _DART_H

#include <sys/types.h>

#define DART_PAGE_SIZE	(4096)

typedef uint32_t	dart_iovm_addr_t;

void dart_init(unsigned int dart_id);
void dart_enable_translation(unsigned int dart_id);
void dart_disable_translation(unsigned int dart_id);
void dart_map_page_range(unsigned int dart_id, uintptr_t paddr, dart_iovm_addr_t vaddr, uint32_t pages, bool write_protect);
void dart_unmap_page_range(unsigned int dart_id, dart_iovm_addr_t vaddr, uint32_t pages);
void dart_write_protect_page_range(unsigned int dart_id, dart_iovm_addr_t vaddr, uint32_t pages, bool write_protect);
void dart_assert_unmapped(unsigned int dart_id);

#endif
