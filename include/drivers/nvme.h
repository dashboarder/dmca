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
#ifndef DRIVERS_NVME_H
#define DRIVERS_NVME_H

#include <sys/types.h>
#include <drivers/pci.h>
#if WITH_DEVICETREE
#include <lib/devicetree.h>
#endif

// NVMe for iOS says we can assume IDENTIFY.MDTS >= 8, which translates to 256 blocks
#define NVME_BOOT_MAX_TRANSFER_BLOCKS	(256)

#define NVME_NAMESPACE_NAND		1
#define NVME_NAMESPACE_FIRMWARE		2
#define NVME_NAMESPACE_SYSCFG		3
#define NVME_NAMESPACE_NVRAM		5
#define NVME_NAMESPACE_EFFACEABLE	6
#define NVME_NAMESPACE_PANICLOG		7

#define NVME_BLOCK_SIZE			(1<<12)

#define NVME_SUCCESS			(0)
// Error codes from -1 to -4095 are reserved for NVMe errors
//     - Bits 7:0 get the SC field from the completion
//     - Bits 11:8 get the SCT field from the completion
//     - See the NVMe spec for details on errors from -1 to -4095
// Error codes from 5000 to 5999 for PCI errors
#define NVME_ERR_PROBE_FAILED		(-5000)
#define NVME_ERR_INVALID_CLASS_CODE	(-5001)
#define NVME_ERR_INVALID_BAR		(-5002)
#define NVME_ERR_INVALID_REG		(-5003)
// Error codes from 6000 to 6999 for controller initialization errors
#define NVME_ERR_DISABLE_TIMEOUT	(-6000)
#define NVME_ERR_ENABLE_TIMEOUT		(-6001)
#define NVME_ERR_SHUTDOWN_TIMEOUT	(-6002)
#define NVME_ERR_CFS			(-6003)
#define NVME_ERR_POLL_REG_TIMEOUT	(-6004)
#define NVME_ERR_INVALID_DSTRD		(-6100)
#define NVME_ERR_INVALID_MPSMIN		(-6101)
#define NVME_ERR_INVALID_MQES		(-6102)
#define NVME_ERR_INVALID_SQES		(-6103)
#define NVME_ERR_INVALID_CQES		(-6104)
#define NVME_ERR_INVALID_DDRREQALIGN	(-6150)
#define NVME_ERR_INVALID_DDRREQSIZE	(-6151)
// Error codes from 7000 to 7999 for command processing errors
#define NVME_ERR_INVALID_CMPL_SQID	(-7001)
#define NVME_ERR_INVALID_CMPL_SQHD	(-7002)
#define NVME_ERR_IO_CMD_TIMEOUT		(-7003)
#define NVME_ERR_ADMIN_CMD_TIMEOUT	(-7004)
#define NVME_ERR_CMPL_CID_MISMATCH	(-7005)
// Error codes from 8000 to 8999 are generic driver errors
#define NVME_ERR_NOT_ENABLED		(-8000)

typedef struct nvme_namespace_params_t {
	bool		formatted;
	uint64_t	num_blocks;
	uint32_t	block_size;
} nvme_namespace_params_t;

int nvme_init(int nvme_id, pci_device_t bridge, uint32_t dart_id);
void nvme_quiesce(int nvme_id);
void nvme_quiesce_all(void);
int nvme_read_blocks(int nvme_id, uint32_t nsid, void *ptr, uint32_t block, uint32_t count);
int nvme_write_blocks(int nvme_id, uint32_t nsid, const void *ptr, uint32_t block, uint32_t count);
int nvme_identify_namespace(int nvme_id, uint32_t nsid, nvme_namespace_params_t *params_out);
uint32_t nvme_get_max_transfer_blocks(int nvme_id);
int nvme_init_mass_storage(int nvme_id);
int nvme_init_mass_storage_panic(int nvme_id);

#if WITH_DEVICETREE
void nvme_update_devicetree (int nvme_id, dt_node_t *node);
#endif

#endif // DRIVERS_NVME_H
