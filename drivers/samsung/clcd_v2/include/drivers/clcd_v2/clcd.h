/*
 * Copyright (C) 2012, 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __SAMSUNG_CLCD_V2_H
#define __SAMSUNG_CLCD_V2_H

void clcd_enable_clocks(bool enable);
void clcd_init(struct display_timing *timing);
bool clcd_get_enable_timing_generator(void);
void clcd_enable_timing_generator(bool enable);
void clcd_install_gamma_table(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut, struct syscfg_wpcl *wpcl);

#endif /* __SAMSUNG_CLCD_V2_H */
