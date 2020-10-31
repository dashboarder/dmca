/*
 * Copyright (C) 2007-2015 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DRIVERS_DISPLAY_H
#define __DRIVERS_DISPLAY_H

#include <sys/types.h>
#include <lib/paint.h>

__BEGIN_DECLS

struct display_window {
	bool active;
	enum colorspace cs;
	uint32_t depth;
	uint32_t pos_x, pos_y;
	uint32_t width, height;
	uint32_t stride;
	struct canvas c;
	void *ptr; // device specific data
};

#define kGammaChannelBufferSize (72)
#define kGammaBufferSize (kGammaChannelBufferSize * 3)

struct display_gamma_table {
	uint32_t display_id;
	uint32_t display_id_mask;
	uint8_t  display_gamma_data[kGammaBufferSize];
};
typedef struct display_gamma_table display_gamma_table;

struct syscfg_glcl {
	uint8_t	minor_vers;
	uint8_t	major_vers;
	uint8_t reserved[2];
	uint8_t	gamma_data[kGammaBufferSize];
};

int display_init(void);
int display_quiesce(bool clear_display);
void display_clear(void);

bool display_get_enable(void);
void display_set_enable(bool enable, uint32_t *color);
bool display_set_rotation(bool rotate180);
void display_delay_frames(uint32_t frames);
void display_set_background_color(uint32_t color);

struct display_window *display_create_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, enum colorspace color);
void display_destroy_window(struct display_window *win);
void display_move_window(struct display_window *win, uint32_t x, uint32_t y);
void display_activate_window(struct display_window *win);
void display_deactivate_window(struct display_window *win);
void display_set_window_fb(struct display_window *win, void *fb);

struct display_info {
	addr_t   framebuffer;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t depth;
};

/* after the fact get the information about the current display */
int display_get_info(struct display_info *info);

struct display_timing
{
	const char	*display_name;

	uint32_t	host_clock_id;
	uint32_t	pixel_clock;

	uint32_t	dot_pitch;

	uint32_t	h_active;
	uint32_t	h_back_porch;
	uint32_t	h_front_porch;
	uint32_t	h_pulse_width;
	uint32_t	v_active;
	uint32_t	v_back_porch;
	uint32_t	v_front_porch;
	uint32_t	v_pulse_width;

	bool		neg_vclk;
	bool		neg_hsync;
	bool		neg_vsync;
	bool		neg_vden;

	uint32_t	display_depth;
	uint32_t	display_type;
	void		*display_config;
};

#define DISPLAY_TYPE_DUMB	(0)
#define DISPLAY_TYPE_SMART	(1)
#define DISPLAY_TYPE_MERLOT	(2)
#define DISPLAY_TYPE_PINOT	(3)
#define DISPLAY_TYPE_TMDS	(4)
#define DISPLAY_TYPE_DP 	(5)
#define DISPLAY_TYPE_EDP 	(6)
#define DISPLAY_TYPE_HDMI	(7)
#define	DISPLAY_TYPE_SUMMIT	(8)

__END_DECLS

#endif /* __DRIVERS_DISPLAY_H */
