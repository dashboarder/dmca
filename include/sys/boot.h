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
#ifndef __SYS_BOOT_H
#define __SYS_BOOT_H

#include <compiler.h>
#include <lib/image.h>

__BEGIN_DECLS

/* boot routines */
enum boot_target {
	BOOT_UNKNOWN = 0,
	BOOT_HALT,
	BOOT_IBOOT,
	BOOT_DARWIN,         // Darwin restore / boot
	BOOT_DARWIN_RESTORE, // resume from RAM
	BOOT_DIAGS,
	BOOT_TSYS,
	BOOT_SECUREROM,
	BOOT_MONITOR,
	BOOT_DALI,
};

/* boot devices */
enum boot_device {
     	BOOT_DEVICE_NOR = 0,
     	BOOT_DEVICE_SPI,
	BOOT_DEVICE_NAND,
	BOOT_DEVICE_NVME,
	BOOT_DEVICE_USBDFU,
	BOOT_DEVICE_TBTDFU,
	BOOT_DEVICE_XMODEM,
};

/* boot flags */
#define BOOT_FLAG_TEST_MODE (1 << 0)

void prepare_and_jump(enum boot_target, void *ptr, void *arg) __noreturn;

/* routines to boot various systems */
void find_boot_images(void);
int boot_diagnostics(void);
int boot_tsys(void);
int boot_iboot(void);
int mount_bootfs(void);
int mount_bootfs_from_device(const char *device);

int mount_and_boot_system(void);
int mount_and_upgrade_system(void);
int boot_upgrade_system(void);
int restore_system(void);

#if WITH_BOOT_STAGE
int boot_check_stage(void);
int boot_set_stage(uint8_t boot_stage);
void boot_check_panic(void);
int boot_clear_error_count(void);
int boot_error_count(uint32_t *boot_fail_count, uint32_t *panic_fail_count);
#endif

__END_DECLS

#endif
