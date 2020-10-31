/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __APPLE_DISPLAY_FRONTEND_H
#define __APPLE_DISPLAY_FRONTEND_H

void adfe_init(struct display_timing *timing);
void adfe_set_ui_layer(u_int32_t pixel_pipe, enum colorspace color, struct plane *plane0, struct plane *plane1, u_int32_t width, u_int32_t height);
void adfe_enable_error_handler(void);
void adfe_disable_error_handler(void);
void adfe_set_background_color(u_int32_t color);
void adfe_activate_window(void);
bool adfe_get_enable(void);
void adfe_set_axis_flip(uint32_t pixel_pipe, bool flipX, bool flipY);

#endif /* __APPLE_DISPLAY_FRONTEND_H */
