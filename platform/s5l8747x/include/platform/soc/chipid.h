/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __PLATFORM_SOC_CHIPID_H
#define __PLATFORM_SOC_CHIPID_H

#include <platform/chipid.h>
#include <platform/soc/hwregbase.h>

#define	rEFUSE_READ_DONE		(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x00))
#define	rCHIPIDL			(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x04))
#define	rCHIPIDH			(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x08))
#define	rDIEIDL				(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x0C))
#define	rDIEIDH				(*(volatile u_int32_t *)(CHIPID_BASE_ADDR + 0x10))

extern bool     chipid_get_ecid_image_personalization_required(void);

#endif /* __PLATFORM_SOC_CHIPID_H */
