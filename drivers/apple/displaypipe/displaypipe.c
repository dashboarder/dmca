/*
 * Copyright (C) 2009-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <assert.h>
#include <drivers/display.h>
#include <lib/env.h>
#include <lib/paint.h>
#include <platform.h>
#include <platform/chipid.h>
#include <platform/clocks.h>
#include <platform/int.h>
#include <platform/memmap.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <target.h>
#include <sys.h>
#include <sys/task.h>

//Front End
#if WITH_DFE_ADFE
# include <drivers/adfe/adfe.h>
#elif WITH_DFE_ADFE_V2
# include <drivers/adfe_v2/adfe.h>
#else
# error "unknown displayfrontend"
#endif // WITH_DFE_ADFE

//Back End
#if WITH_DBE_CLCD
# include <drivers/clcd_v2/clcd.h>
#elif WITH_DBE_RGBOUT
# include <drivers/rgbout/rgbout.h>
#elif WITH_DBE_ADBE
# include <drivers/adbe/adbe.h>
#else
# error "unknown display-backend"
#endif

//Transport
#if WITH_HW_DISPLAY_HDMI
#include <drivers/hdmi.h>
#elif (WITH_HW_DISPLAY_EDP || WITH_HW_DISPLAY_DISPLAYPORT)
#include <drivers/displayport/displayport.h>
#include <drivers/displayport.h>
#endif

#include <platform/soc/display_timings.h>

#ifndef LINEAR_STRIDE_ALIGNMENT
#define LINEAR_STRIDE_ALIGNMENT		64
#endif

#define LINEAR_STRIDE_ALIGNMENT_MASK	(LINEAR_STRIDE_ALIGNMENT - 1)

#ifndef TARGET_FB_MULT
#define TARGET_FB_MULT 30
#endif

static u_int32_t display_default_color;
static u_int64_t display_frame_us = 16667;
static bool display_enabled;
static bool display_enabled_valid;

static struct display_window *main_window;
static u_int32_t *red_lut = NULL, *green_lut = NULL, *blue_lut = NULL;

static struct display_timing *timing_info;

static const int32_t timing_list_size = sizeof(timing_list) / sizeof(struct display_timing);

static void display_backend_enable_clocks(bool enable);
static void display_backend_init(struct display_timing *timing);
static void display_backend_enable_timing_generator(bool enable);
static void display_backend_install_gamma_table(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut, struct syscfg_wpcl *wpcl);
static int display_set_timings(struct display_timing *timing);
static void display_set_timing_info();

static void display_set_timing_info()
{
	const char		*env;
	int32_t			cnt;

	if (timing_info != NULL)
		return;
		
	env = env_get("display-timing");
	if (env == 0) env = "";

	for (cnt = 0; cnt < timing_list_size; cnt++) {
		if (strcmp(env, timing_list[cnt].display_name)) continue;

		timing_info = timing_list + cnt;
		timing_info->display_config = target_get_display_configuration();
	}
}

int display_init(void)
{
	const char		*env;
	enum colorspace		color = CS_4BPP;
	int			result;
	u_int32_t		display_id;
	int32_t			cnt, stride;
	struct syscfg_wpcl 	display_wpcl;
	u_int64_t		tmp = 1;

	dprintf(DEBUG_SPEW, "display_init()\n");

	/* clear the display memory */
	bzero((void *)DISPLAY_BASE, DISPLAY_SIZE);

	display_set_timing_info();

	if (timing_info == 0) {
		dprintf(DEBUG_INFO, "Failed to find display timing info, bailing display_init()\n");
		return -1;
	}

	// Make sure clocks (both display front-end and back-end) are on
	display_backend_enable_clocks(true);

	display_set_timings(timing_info);

	adfe_init(timing_info);

	// Setup back-end (includes dither, color-manager)
	display_backend_init(timing_info);

	if (timing_info->pixel_clock) {
		// Calculate display frame interval (in usecs) based on timing info
		tmp = 1000000ULL;
		tmp *= (timing_info->v_back_porch + timing_info->v_front_porch + timing_info->v_pulse_width + timing_info->v_active);
		tmp *= (timing_info->h_back_porch + timing_info->h_front_porch + timing_info->h_pulse_width + timing_info->h_active);
		tmp /= timing_info->pixel_clock;
	}

	display_frame_us = tmp;
	env = env_get("display-color-space");
	if (env == 0) env = "RGB888";

	if (!strcmp(env, "RGB888")) color = CS_RGB888;
	else if (!strcmp(env, "RGB565")) color = CS_RGB565;
	else if (!strcmp(env, "ARGB8101010")) color = CS_ARGB8101010;

	main_window = display_create_window(0, 0, timing_info->h_active, timing_info->v_active, color);
	if (main_window == 0) return -1;

	/* add an environment variable with the address of the framebuffer */
	env_set_uint("framebuffer", (uintptr_t)main_window->c.fb_phys, 0);

	// request the desired pixel clock
	clock_set_frequency(timing_info->host_clock_id, 0, 0, 0, 0, timing_info->pixel_clock);

	switch (timing_info->display_type) {
#if WITH_HW_DISPLAY_PINOT
		case DISPLAY_TYPE_PINOT :
		{
			extern int pinot_init(struct display_timing *timing, enum colorspace color, u_int32_t *display_id);
			result = pinot_init(timing_info, main_window->cs, &display_id);
			break;
		}
#endif

#if WITH_HW_DISPLAY_SUMMIT
		case DISPLAY_TYPE_SUMMIT :
		{
			extern int summit_init(struct display_timing *timing, enum colorspace color, u_int32_t *display_id);
			result = summit_init(timing_info, main_window->cs, &display_id);
			break;
		}
#endif

#if WITH_HW_DISPLAY_EDP
		case DISPLAY_TYPE_EDP :
		{
			extern int edp_init(struct display_timing *timing, enum colorspace color, u_int32_t *display_id);
			result = edp_init(timing_info, main_window->cs, &display_id);
			break;
		}
#endif

#if (WITH_HW_DISPLAY_DISPLAYPORT && !WITH_HW_MCU)
    
		case DISPLAY_TYPE_DP :
		{
			result = displayport_init_with_timing_info(timing_info);
			break;
		}
#endif
#if WITH_HW_DISPLAY_HDMI
		case DISPLAY_TYPE_HDMI :
		{
			result = hdmi_init_with_timing_info(timing_info);
			break;
		}
#endif

		default :
			display_id = 0;
			result = 0;
	}

	if (result == 0) {
		if (!display_enabled_valid)
			display_enabled = display_get_enable();

		if (!display_enabled) {
			display_set_enable(true, &display_default_color);
		}

		red_lut = (u_int32_t *)malloc(257 * sizeof(u_int32_t));
		green_lut = (u_int32_t *)malloc(257 * sizeof(u_int32_t));
		blue_lut = (u_int32_t *)malloc(257 * sizeof(u_int32_t));

	        // Generate linear LUT
	        stride = 256 >> (10 - (timing_info->display_depth / 3));
	        red_lut[0] = 1;
	        for (cnt = 1; cnt < 257; cnt++) {
	                if ((cnt % stride) == 1) red_lut[cnt - 1]--;
	                red_lut[cnt] = red_lut[cnt - 1] + 4;
	                green_lut[cnt - 1] = blue_lut[cnt - 1] = red_lut[cnt - 1];
	        }
	        green_lut[cnt - 1] = blue_lut[cnt - 1] = red_lut[cnt - 1];

#if PRODUCT_IBOOT
		paint_install_gamma_table(display_id, 257, red_lut, green_lut, blue_lut);
#endif
		paint_get_syscfg_wpcl(&display_wpcl);
		dprintf(DEBUG_SPEW, "paint_install_gamma_table: Found WpCl version = 0x%08x, red = 0x%08x, green = 0x%08x, blue = 0x%08x\n", display_wpcl.version, display_wpcl.red, display_wpcl.green, display_wpcl.blue);

		display_backend_install_gamma_table(red_lut, green_lut, blue_lut, &display_wpcl);

	} else {
		display_set_enable(false, NULL);
		timing_info = NULL;
	}

	return result;
}

void display_clear(void)
{
	if (main_window) {
		fill_rect(&main_window->c, 0, 0, main_window->width, main_window->height, display_default_color);
	}
}


int display_quiesce(bool clear_display)
{
	int result = 0;

	//enable clocks
	display_backend_enable_clocks(true);
	if (!display_get_enable()) {
		//disable the clock
		display_backend_enable_clocks(false);
		return ENXIO;
	}

	display_set_timing_info();

	if (timing_info == NULL) return result;

	if (clear_display) {
		display_clear();

		display_delay_frames(2);
	}
	adfe_disable_error_handler();

	if (red_lut)
		free(red_lut);
	if (green_lut)
		free(green_lut);
	if (blue_lut)
		free(blue_lut);
		
	red_lut = NULL;
	green_lut = NULL;
	blue_lut = NULL;
	
#if WITH_HW_DISPLAY_PINOT
	if (timing_info->display_type == DISPLAY_TYPE_PINOT) {
		extern int pinot_quiesce(void);
		result = pinot_quiesce();
		if (result != 0) return result;
	}
#endif

#if WITH_HW_DISPLAY_SUMMIT
	if (timing_info->display_type == DISPLAY_TYPE_SUMMIT) {
		extern int summit_quiesce(void);
		result = summit_quiesce();
		if (result != 0) return result;
	}
#endif

#if  WITH_HW_DISPLAY_EDP
	if (timing_info->display_type == DISPLAY_TYPE_EDP) {
		extern int edp_quiesce(void);
		result = edp_quiesce();
		result = 0;
		if (result != 0) return result;
	}
#endif

#if WITH_HW_DISPLAY_DISPLAYPORT
	if (timing_info->display_type == DISPLAY_TYPE_DP) {
		displayport_quiesce();
		result = 0;
	}
#endif

#if WITH_HW_DISPLAY_HDMI
	if (timing_info->display_type == DISPLAY_TYPE_HDMI) {
		hdmi_quiesce();
		result = 0;
	}
#endif

	display_set_enable(false, NULL);

	if (main_window) {
		free(main_window);
		main_window = NULL;
	}

	return result;
}

bool display_get_enable(void)
{
	bool display_enable;

#if WITH_DBE_CLCD
	display_enable = clcd_get_enable_timing_generator();
#elif WITH_DBE_RGBOUT
	display_enable = rgbout_get_enable_timing_generator();
#elif WITH_DBE_ADBE
	display_enable = adbe_get_enable_timing_generator();
#else
# error "missing display_backend_enable_timing_generator"
#endif
	display_enabled_valid = true;

	return display_enable;
}

void display_set_enable(bool enable, u_int32_t *color)
{
	if (!display_enabled_valid)
		display_enabled = display_get_enable();

	if (enable == display_enabled) return;

	if (enable) {
		if (color != NULL) {
			display_default_color = *color;
		}

		fill_rect(&main_window->c, 0, 0, main_window->width, main_window->height, display_default_color);

		display_backend_enable_timing_generator(true);

		display_enabled = true;

	} else {

		display_backend_enable_timing_generator(false);

		display_enabled = false;
	}
}

bool display_set_rotation(bool rotate180)
{
	#if WITH_DFE_ADFE_V2
	adfe_set_axis_flip(0, rotate180, rotate180);
	return true;
	#else
	return false;
	#endif
}

void display_delay_frames(u_int32_t frames)
{
	task_sleep(frames * display_frame_us);
}

///////////////////////////////////////////////////////////////////////////////////
//With iBoot inside display memory (1 plane Color)
//                         PANIC_BASE    -------------------------------
//                                       |          iBoot              |
//                                       |-----------------------------|
//                                       |         (unused)            |
//                                       |-----------------------------|
//                                       |         db_virt (draw)      |
//                                       |-----------------------------|
//                                       |            fb               |
//                                       |-----------------------------|<---- region_valid
//                                       |         images              |
//                                       |-----------------------------|<---- scratch_base + 4K
//                                       |         flatten images      |
//                      DISPLAY_BASE --> ------------------------------ <---- scratch_base
//With iBoot outside display memory (1 plane Color)
//                         PANIC_BASE    -------------------------------
//                                       |         (unused)            |
//                                       |-----------------------------|
//                                       |         db_virt (draw)      |
//                                       |-----------------------------|
//                                       |            fb               |
//                                       |-----------------------------|<---- region_valid
//                                       |         images              |
//                                       |-----------------------------|<---- scratch_base + 4K
//                                       |         flatten images      |
//                      DISPLAY_BASE --> ------------------------------ <---- scratch_base
//With iBoot outside display memory (2 plane Color)
//                         PANIC_BASE    -------------------------------
//                                       |         (unused)            |
//                                       |-----------------------------|
//                                       |    plane 1 db_virt (draw)   |
//                                       |-----------------------------|
//                                       |    plane 0 db_virt (draw)   |
//                                       |-----------------------------|
//                                       |      plane 1  fb            |
//                                       |-----------------------------|
//                                       |      plane 0  fb            |
//                                       |-----------------------------|<---- region_valid
//                                       |         images              |
//                                       |-----------------------------|<---- scratch_base + 4K
//                                       |         flatten images      |
//                      DISPLAY_BASE --> ------------------------------ <---- scratch_base
///////////////////////////////////////////////////////////////////////////////////
struct display_window *display_create_window(u_int32_t x, u_int32_t y, u_int32_t width, u_int32_t height, enum colorspace color)
{
	struct display_window *w;
	u_int32_t stridelen_in_pixels = width, depth;
	uint32_t stridelen_in_bytes;
	addr_t fb_base;
	size_t fb_size;
	size_t fb_region, region_valid;
	uint32_t num_of_planes;
	uint32_t plane1_depth;

	switch (color) {
		case CS_RGB565 :
			depth = 16;
			num_of_planes = 1;
			break;

		case CS_RGB888 :
			depth = 32;
			num_of_planes = 1;
			break;

		case CS_ARGB8101010 :
			//the RGB plane is 32
			depth = 32; 
			num_of_planes = 2;
			//Alpha plane is 8
			plane1_depth = 8;
			break;

		default :
			return 0;
	}

	w = malloc(sizeof(struct display_window));

	w->active = false;
	w->cs = color;
	w->depth = depth;
	w->pos_x = x;
	w->pos_y = y;
	w->width = width;
	w->height = height;
	stridelen_in_bytes = (width * depth) / 8;
	stridelen_in_bytes = (stridelen_in_bytes + LINEAR_STRIDE_ALIGNMENT_MASK) & ~LINEAR_STRIDE_ALIGNMENT_MASK;
	w->stride = stridelen_in_bytes;
	stridelen_in_pixels = stridelen_in_bytes / (depth/8);

	// Calculate the frame buffer size rounded up to a page
	fb_size = (w->height * w->stride + 0xFFF) & ~0xFFF;

	//If we have more planes, more region_valid memory is required. Adjust the size accordingly
	if (num_of_planes > 1) {
		size_t plane1_stride_in_bytes = (width * (plane1_depth/8));
		plane1_stride_in_bytes = (plane1_stride_in_bytes  + LINEAR_STRIDE_ALIGNMENT_MASK) & ~LINEAR_STRIDE_ALIGNMENT_MASK;
		size_t plane1_fb_size = (plane1_stride_in_bytes + 0xFFF) & ~0xFFF;
		fb_size += plane1_fb_size;
	}


	//on 32 bit systems, the fb_base has to account for 1 fb + iBoot region.  The iBoot region goes away for OS purposes yet the fb remains
	region_valid = fb_region = (fb_size * TARGET_FB_MULT)/10;
#ifdef PROTECTED_REGION_SIZE
	if(fb_region < fb_size * 2 + PROTECTED_REGION_SIZE)  fb_region = fb_size * 2 + PROTECTED_REGION_SIZE ;
	region_valid = fb_region - PROTECTED_REGION_SIZE;
#endif
	// Calculate the frame buffer base assuming multiple buffers (as defined by TARGET_FB_MULT) from PANIC_BASE
	// N.B We use PANIC_BASE since DISPLAY_BASE is used for image manipulation region
	fb_base = PANIC_BASE - fb_region;

	platform_init_display_mem(&fb_base, &fb_size);
#ifdef DISPLAY_BASE_NONALIGNED
	set_canvas(&w->c, (void *)(PANIC_BASE - fb_region), region_valid, width, height, stridelen_in_pixels, color);
#else
	set_canvas(&w->c, (void *)fb_base, region_valid, width, height, stridelen_in_pixels, color);
#endif
	paint_init(w, DISPLAY_BASE, fb_base - DISPLAY_BASE);

	adfe_set_ui_layer(0, color, get_plane(0), get_plane(1), width, height);

	display_activate_window(w);

	return w;
}

void display_set_background_color(u_int32_t color)
{
	adfe_set_background_color(color);
}

void display_activate_window(struct display_window *win)
{
	u_int32_t layer = 0;

	if (layer > 1) return;

	win->active = true;

	adfe_activate_window();
}

static int display_set_timings(struct display_timing *timing)
{
	int result = 0;
#if WITH_HW_DISPLAY_DISPLAYPORT
	result = displayport_set_timings(timing);
#endif
#if WITH_HW_DISPLAY_HDMI
	result = hdmi_set_timings(timing);
#endif
	return result;
}


static void display_backend_enable_clocks(bool enable)
{
#if WITH_DBE_CLCD
	clcd_enable_clocks(enable);
#elif WITH_DBE_RGBOUT
	rgbout_enable_clocks(enable);
#elif WITH_DBE_ADBE
	adbe_enable_clocks(enable);
#else
# error "missing display_backend_enable_clocks"
#endif	
}

static void display_backend_init(struct display_timing *timing)
{
#if WITH_DBE_CLCD
	clcd_init(timing);
#elif WITH_DBE_RGBOUT
	rgbout_init(timing);
#elif WITH_DBE_ADBE
	adbe_init(timing);
#else
# error "missing display_backend_init"
#endif	
}

static void display_backend_install_gamma_table(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut, struct syscfg_wpcl *wpcl)
{
#if WITH_DBE_CLCD
	clcd_install_gamma_table(red_lut, green_lut, blue_lut, wpcl);
#elif WITH_DBE_RGBOUT
	rgbout_install_gamma_table(red_lut, green_lut, blue_lut, wpcl);
#elif WITH_DBE_ADBE
	adbe_install_gamma_table(red_lut, green_lut, blue_lut, wpcl);
#else
# error "missing display_backend_load_gamma_table"
#endif
}

static void display_backend_enable_timing_generator(bool enable)
{
#if WITH_DBE_CLCD
	clcd_enable_timing_generator(enable);
#elif WITH_DBE_RGBOUT
	rgbout_enable_timing_generator(enable);
#elif WITH_DBE_ADBE
	adbe_enable_timing_generator(enable);
#else
# error "missing display_backend_enable_timing_generator"
#endif
}
