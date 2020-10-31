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
#ifndef __APPLE_DISPLAY_BACKEND_H
#define __APPLE_DISPLAY_BACKEND_H

void adbe_enable_clocks(bool enable);
void adbe_init(struct display_timing *timing);
void adbe_enable_timing_generator(bool enable);
bool adbe_get_enable_timing_generator(void);
void adbe_install_gamma_table(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut, struct syscfg_wpcl *wpcl);

#endif /* __APPLE_DISPLAY_BACKEND_H */
