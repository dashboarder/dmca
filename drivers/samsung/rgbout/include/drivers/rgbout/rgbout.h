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
#ifndef __SAMSUNG_RGBOUT_H
#define __SAMSUNG_RGBOUT_H

void rgbout_enable_clocks(bool enable);
void rgbout_init(struct display_timing *timing);
void rgbout_enable_timing_generator(bool enable);
bool rgbout_get_enable_timing_generator(void);
void rgbout_install_gamma_table(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut, struct syscfg_wpcl *wpcl);

#endif /* __SAMSUNG_RGBOUT_H */
