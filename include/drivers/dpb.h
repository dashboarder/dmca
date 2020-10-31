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
#ifndef __DRIVERS_DPB_H
#define __DRIVERS_DPB_H

#include <sys/types.h>

__BEGIN_DECLS

#if DPB_VERSION < 2
void dpb_init(void);
#else
void dpb_init(uint32_t display_width, uint32_t display_height);
#endif
void dpb_install_gamma_table(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut, struct syscfg_wpcl *wpcl);

__END_DECLS

#endif /* __DRIVERS_DPB_H */
