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
#ifndef __DRIVERS_FLASH_NAND_H
#define __DRIVERS_FLASH_NAND_H

#include <sys/types.h>

__BEGIN_DECLS

int 	flash_nand_init(void);
int 	flash_nand_id(void);

#if WITH_HW_BOOTROM_NAND
/* page offset into block zero from which the bootblock should be read */
#define NAND_BOOTBLOCK_OFFSET	2

int	nand_read_bootblock(u_int8_t interface, u_int8_t cs, void *buffer, size_t *size);
#endif

__END_DECLS

#endif /* __DRIVERS_FLASH_NAND_H */
