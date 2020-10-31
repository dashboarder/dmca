/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_PCIMAP_H
#define __PLATFORM_PCIMAP_H

/* S5L8970X specific bus/device/function and PCI base address map */

#include <platform/soc/hwregbase.h>

#define PCIE_PORT0_MEMORY_BASE		(PCIE_MEMORY_BASE)
#define PCIE_PORT0_MEMORY_LEN		(0x100000ULL)

#define NVME0_PCI_BDF			(PCI_BDF(1, 0, 0))
#define NVME0_BASE_ADDR			(PCIE_PORT0_MEMORY_BASE)
#define NVME0_SIZE			(0x2000)
#define NVME0_DART_ID			(PCIE_PORT0_DART_ID)


#endif /* ! __PLATFORM_PCIMAP_H */
