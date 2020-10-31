/*
 * Copyright (C) 2007-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __PLATFORM_CHIPID_H
#define __PLATFORM_CHIPID_H

#include <sys/types.h>

__BEGIN_DECLS

#define	CHIP_REVISION_A0		0x00
#define	CHIP_REVISION_A1		0x01
#define	CHIP_REVISION_B0		0x10
#define	CHIP_REVISION_B1		0x11
#define	CHIP_REVISION_C0		0x20
#define	CHIP_REVISION_C1		0x21

bool		chipid_get_production_mode(void);
bool		chipid_get_current_production_mode(void);
bool		chipid_get_raw_production_mode(void);
void		chipid_clear_production_mode(void);
bool		chipid_get_secure_mode(void);
u_int32_t	chipid_get_security_domain(void);
u_int32_t 	chipid_get_minimum_epoch(void);
u_int32_t	chipid_get_board_id(void);
u_int32_t	chipid_get_chip_id(void);
u_int32_t	chipid_get_chip_revision(void);
u_int32_t	chipid_get_osc_frequency(void);
u_int64_t	chipid_get_ecid_id(void);
u_int64_t	chipid_get_die_id(void);
uint32_t	chipid_get_fuse_revision(void);
bool		chipid_get_fuse_lock(void);
void		chipid_set_fuse_lock(bool locked);
bool		chipid_get_fuse_seal(void);
u_int32_t	chipid_get_memory_density(void);
u_int32_t	chipid_get_memory_manufacturer(void);
u_int32_t	chipid_get_memory_ranks(void);
u_int32_t	chipid_get_memory_width(void);
bool		chipid_get_memory_dqcal(u_int32_t *cal_data);
u_int32_t	chipid_get_package_id(void);
bool		chipid_get_read_done(void);

uint32_t 	chipid_get_pid(void);

__END_DECLS

#endif /* __PLATFORM_CHIPID_H */

