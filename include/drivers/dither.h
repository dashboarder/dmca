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
#ifndef __DRIVERS_DITHER_H
#define __DRIVERS_DITHER_H

#include <sys/types.h>

__BEGIN_DECLS

#define	DITHER_NONE		(0 << 0)
#define	DITHER_BLUE_NOISE	(1 << 0)
#define	DITHER_SPATIO_TEMPORAL	(1 << 1)
#define DITHER_ERROR_DIFFUSION	(1 << 2)

#if DITHER_VERSION == 4
void dither_init(uint32_t display_width, uint32_t display_height, uint32_t display_depth);
#else
void dither_init(uint32_t display_depth);
#endif
void dither_set_enable(bool enable);

__END_DECLS

#endif /* __DRIVERS_DITHER_H */
