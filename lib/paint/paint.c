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
#include <debug.h>
#include <platform.h>
#include <drivers/display.h>
#include <lib/lzss.h>
#include <lib/mib.h>
#include <lib/paint.h>
#include <lib/syscfg.h>
#include <string.h>
#include <sys.h>
#include <sys/types.h>
#include <arch.h>
#include <target.h>
#include <lib/libiboot.h>

#define SYSCFG_DCLR_COLOR_REPORT	1	// Report colors found in 'DClr' syscfg entry
#if DEBUG_BUILD
#define SYSCFG_DCLR_OVERRIDE		1	// Allow overridding 'DClr' syscfg entry from environment for testing
#else
#define SYSCFG_DCLR_OVERRIDE		0 	// Do not honor DClr_Override on non-DEBUG builds.
#endif
#define PANIC_ON_SERIOUS_ERROR		1	// Panic if 'DClr' syscfg processing error
#define PANIC_ON_UNKNOWN_DCLR_COLOR	1	// Panic if 'DClr' syscfg version is unknown or invalid

// Nothing explicitly defines a kMIBTargetOsPictureRotate factor so use
// the value returned by kMIBTargetPictureRotate for everything.
#define kMIBTargetOsPictureRotate	kMIBTargetPictureRotate

struct iboot_image;
static int paint_draw_picture(struct iboot_image *comp_image_buf, size_t comp_image_len);
static int paint_update_image_argb(void);
static int paint_update_image_wide_gammut(void);

static void paint_install_gamma_table_channel(uint32_t entries, volatile uint32_t *channel_lut, const uint8_t *bits);
static uint8_t paint_install_gamma_get_2bits(const uint8_t *bits);

static void plot4(struct plane *p, uint32_t x, uint32_t y, uint64_t color);
static void plot8(struct plane *p, uint32_t x, uint32_t y, uint64_t color);
static void plot16(struct plane *p, uint32_t x, uint32_t y, uint64_t color);
static void plot32(struct plane *p, uint32_t x, uint32_t y, uint64_t color);
static uint32_t get_pixel4(struct plane *p, uint32_t x, uint32_t y);
static uint32_t get_pixel8(struct plane *p, uint32_t x, uint32_t y);
static uint32_t get_pixel16(struct plane *p, uint32_t x, uint32_t y);
static uint32_t get_pixel32(struct plane *p, uint32_t x, uint32_t y);
static void hline4(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color);
static void hline8(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color);
static void hline16(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color);
static void hline32(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color);
static void vline4(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color);
static void vline8(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color);
static void vline16(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color);
static void vline32(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color);
static void set_plane(struct plane *p, uint32_t y, uint32_t line_x, size_t plane_size, addr_t plane_fb, addr_t plane_db, enum colorspace cs);

static void clear_flattened_images(void);

static bool paint_ready;
static struct canvas *paint_canvas;

static uint32_t window_height;
static uint32_t window_width;
static uint32_t window_stride;
static uint32_t window_depth;
static uintptr_t window_framebuffer;

static addr_t compressed_image_base;
static size_t compressed_image_size;

struct flattened_image {
	bool		fi_color;
	uint32_t	fi_width;
	uint32_t	fi_height;
	uint32_t	fi_xstart;
	uint32_t	fi_ystart;
	uint32_t	fi_xstop;
	uint32_t	fi_ystop;
	uint8_t		*fi_buffer;
	uint32_t	fi_src_pos;
};

#define MAX_FLATTENED_IMAGES (0x1000 / sizeof(struct flattened_image))
static uint32_t flattened_images_count;
static struct flattened_image *flattened_images;
static addr_t flattened_images_base;
static addr_t flattened_images_end;

static uint64_t background_rgb;
static bool color_map_desired;
static bool color_map_enabled;
static bool color_map_supported;
static bool color_map_invalid;
static color_policy_t color_policy;
#if DEBUG_BUILD
static struct display_window *paint_window;
#endif
static addr_t last_canvas_addr;
static size_t page_size;

int paint_init(void *window, addr_t scratch, size_t scratch_size)
{
	struct display_window *w = (struct display_window *)window;
	int result = -1;

	paint_ready = false;

	if (w == NULL) {
		goto out;
	}

	paint_canvas = &w->c;

	window_height = w->height;
	window_width  = w->width;
	window_stride = w->stride;
	window_depth  = w->depth;

	//The framebuffer region should be large enough to include all planes used
	window_framebuffer = paint_canvas->fb_phys;

	compressed_image_base = scratch;
	compressed_image_size = window_height * window_stride + 0x1000;

	flattened_images = 0;
	flattened_images = (struct flattened_image *)(compressed_image_base + compressed_image_size);
	flattened_images_base = compressed_image_base + compressed_image_size + 0x1000;
	flattened_images_end = scratch + scratch_size;
	flattened_images_count = 0;

	if (flattened_images_base >= flattened_images_end) {
		dprintf(DEBUG_INFO, "Display Size is too small\n");
		goto out;
	}

	background_rgb = 0x00000000;

	paint_ready = true;

#if DEBUG_BUILD
	paint_window = w;
#endif
	result = 0;

out:
	return result;
}

static void clear_flattened_images(void)
{
	if (!paint_ready)
		return;

	flattened_images_count = 0;
	flattened_images_base = compressed_image_base + compressed_image_size + 0x1000;
}

int paint_set_bgcolor(uint16_t bg_red, uint16_t bg_green, uint16_t bg_blue)
{
	if (!paint_ready) {
		return 0;
	}

	if (paint_canvas->cs == CS_ARGB8101010) {
		background_rgb = RGB10(bg_red, bg_green, bg_blue);
	} else {
		background_rgb = RGB(bg_red, bg_green, bg_blue); 
	}

	return 0;
}

/* image_data is either 32 or 16 bits per sample with the b_alpha most significant.   */
/* The reset of the data is the rgb or grey data having already been alpha scaled.    */
/* The b_alpha value is scale with the back ground color then added to the rgb value. */

struct iboot_image {
	char		image_signature[8];	// 0x00 - 'iBootIm'
	uint32_t	image_adler32;		// 0x08
	uint32_t	image_type;		// 0x0C - 'lzss'
	uint32_t	image_format;		// 0x10 - 'argb' or 'grey'
	uint16_t	image_width;		// 0x14
	uint16_t	image_height;		// 0x16
	int16_t		image_h_offset;		// 0x18
	int16_t		image_v_offset;		// 0x1A
	uint32_t	image_data_length;	// 0x1C
	uint32_t	reserved[8];		// 0x20
	uint8_t		image_data[];		// 0x40
} __attribute__((packed));
typedef struct iboot_image iboot_image;

int paint_set_picture_for_tag(uint32_t image_tag)
{
	struct image_info *pict_info = 0;

	if (mib_get_u32(kMIBTargetApplicationProduct) == MIB_APP_PROD_IBOOT) {
		pict_info = image_find(image_tag);
		if (pict_info == 0) {
			dprintf(DEBUG_INFO, "image tag 0x%08x not found\n", image_tag);
		}
	}

	return paint_set_picture(pict_info);
}

int paint_set_picture_for_tag_list(uint32_t image_tag, int image_offset, int image_count)
{
	struct image_info *pict_info = 0;

	if (mib_get_u32(kMIBTargetApplicationProduct) == MIB_APP_PROD_IBOOT) {
		pict_info = image_find(image_tag);
		if (pict_info == 0) {
			dprintf(DEBUG_INFO, "image tag 0x%08x not found\n", image_tag);
		}
	}

	return paint_set_picture_list(pict_info, image_offset, image_count);
}

int paint_set_picture(struct image_info *pict_info)
{
	return paint_set_picture_list(pict_info, 0, 1);
}

int paint_set_picture_list(struct image_info *pict_info, int image_offset, int image_count)
{
	void *comp_image_buf;
	size_t comp_image_len;

	/* must be initialised */
	if (!paint_ready) {
		return 0;
	}

	/* if no image, clear the display and all images */
	if (pict_info == 0) {
		clear_flattened_images();
		return 0;
	}

	/* if we're out of space, we can't do anything */
	if (flattened_images_count >= MAX_FLATTENED_IMAGES) {
		return 0;
	}

	comp_image_len = pict_info->imageAllocation;
	if (comp_image_len > compressed_image_size) {
		dprintf(DEBUG_INFO, "compressed image too large\n");
		return -1;
	}
	comp_image_buf = (uint8_t *)(compressed_image_base);

	/* accept any type (pass NULL) */
	if (image_load(pict_info, NULL, 0, NULL, &comp_image_buf, &comp_image_len) < 0) {
		dprintf(DEBUG_INFO, "cannot load image\n");
		return -1;
	}

	// before drawing picture, consolidate environment based on image validation result
	security_consolidate_environment();

	for (int idx = 0; idx < image_offset + image_count; idx++) {
		iboot_image *ind_comp_image = comp_image_buf;
		size_t ind_comp_image_len;

		if (ind_comp_image->image_data_length == 0) {
			// old format
			ind_comp_image_len = comp_image_len;
		} else if ((comp_image_len >= sizeof(iboot_image)) && (comp_image_len >= (sizeof(iboot_image) + ind_comp_image->image_data_length))) {
			ind_comp_image_len = sizeof(iboot_image) + ind_comp_image->image_data_length;
		} else {
			dprintf(DEBUG_INFO, "image length invalid\n");
			return -1;
		}

		if (idx >= image_offset) {
			int ret = paint_draw_picture(ind_comp_image, ind_comp_image_len);
			if (ret != 0)
				return ret;
		}

		comp_image_buf = ((uint8_t *)comp_image_buf) + ind_comp_image_len;
		comp_image_len -= ind_comp_image_len;
	}

	return 0;
}

int paint_set_picture_from_file(const char *path, uint32_t type)
{
	addr_t		comp_image_buf;
	size_t		comp_image_len;

	/* must be initialised */
	if (!paint_ready) {
		return 0;
	}

	/* if we're out of space, we can't do anything */
	if (flattened_images_count >= MAX_FLATTENED_IMAGES) {
		return 0;
	}

	comp_image_buf = compressed_image_base;
	comp_image_len = compressed_image_size;
	if (image_load_file(path, &comp_image_buf, &comp_image_len, &type, 1, NULL, 0)) {
		dprintf(DEBUG_INFO, "cannot load image\n");
		return -1;
	}

	// before drawing picture, consolidate environment based on image validation result
	security_consolidate_environment();

	return paint_draw_picture((void *)compressed_image_base, comp_image_len);
}

static int paint_draw_picture(iboot_image *comp_image, size_t comp_image_len)
{
	uint8_t *image_buf, *rot_buf;
	uint32_t image_max_len, image_len, image_expected_len, data;
	uint32_t *image_src32, *image_dst;
	uint16_t *image_src16;
	int32_t imagex, imagey, imagexs, imagexe, imageys, imageye;
	int32_t imagex_offset, imagey_offset;
	int32_t posx, posy, rposx, rposy;
	int32_t picture_scale;
	int32_t picture_rotate;
	bool image_color;

	image_max_len = flattened_images_end - flattened_images_base;
	image_buf = (uint8_t *)(flattened_images_base);

	if ((comp_image_len < sizeof(iboot_image)) ||
	    (strncmp(comp_image->image_signature, "iBootIm", 8) != 0) ||
	    (comp_image->image_type != 'lzss') ||
	    ((comp_image->image_format != 'argb') && (comp_image->image_format != 'grey'))) {
		dprintf(DEBUG_INFO, "image not valid\n");
		return -1;
	}

	image_color = false;
	if (comp_image->image_format == 'argb') image_color = true;

	picture_scale = mib_get_u32(kMIBTargetPictureScale);
	ASSERT((picture_scale == 1) || (picture_scale == 2));
	picture_rotate = mib_get_s32(kMIBTargetPictureRotate);

	if (picture_rotate == 0) {
		imagex = comp_image->image_width * picture_scale;
		imagey = comp_image->image_height * picture_scale;
		imagex_offset = comp_image->image_h_offset * picture_scale;
		imagey_offset = comp_image->image_v_offset * picture_scale;
	} else if ((picture_rotate == 90) || (picture_rotate == -90)) {
		imagex = comp_image->image_height * picture_scale;
		imagey = comp_image->image_width * picture_scale;
		imagex_offset = comp_image->image_v_offset * picture_scale * ((picture_rotate == 90) ? -1 : 1);
		imagey_offset = comp_image->image_h_offset * picture_scale * ((picture_rotate == 90) ? 1 : -1);
	} else {
		panic("kMIBTargetPictureRotate value is unsupported: %d", picture_rotate);
	}

	imagexs = ((window_width - imagex) / 2) + imagex_offset;
	imagexe = imagexs + imagex;
	imageys = ((window_height - imagey) / 2) + imagey_offset;
	imageye = imageys + imagey;

	if ((imagexs < 0) || (imageys < 0) || (imagexe > (int32_t)window_width) || (imageye > (int32_t)window_height)) {
		dprintf(DEBUG_INFO, "image does not fit on screen\n");
		return -1;
	}

	image_expected_len = (uint32_t)(imagex / picture_scale) * (uint32_t)(imagey / picture_scale) * (image_color ? 4 : 2);

	if (picture_rotate != 0) {
		if ((image_expected_len * 2) > image_max_len) {
			dprintf(DEBUG_INFO, "image is too large to be rotate\n");
			return -1;
		}

		rot_buf = image_buf;
		image_buf += image_expected_len;
		image_max_len -= image_expected_len;
	}

	image_len = decompress_lzss(image_buf, image_max_len, comp_image->image_data, comp_image_len - sizeof(iboot_image));

	if (image_len != image_expected_len) {
		dprintf(DEBUG_INFO, "image data size does not match expected image size\n");
		return -1;
	}

	if (picture_scale == 2) {
		image_src16 = (uint16_t *)image_buf + (imagex / picture_scale) * (imagey / picture_scale) - 1;
		image_src32 = (uint32_t *)image_buf + (imagex / picture_scale) * (imagey / picture_scale) - 1;
		if (image_color) image_dst = (uint32_t *)image_buf + imagex * imagey - 1;
		else image_dst = (uint32_t *)image_buf + (imagex / 2) * imagey - 1;
		for (posy = 0; posy < (imagey / picture_scale); posy++) {
			for (posx = 0; posx < (imagex / picture_scale); posx++) {
				if (image_color) {
					data = *(image_src32--);
					*(image_dst - 0) = data;
					*(image_dst - 1) = data;
					*(image_dst - imagex - 0) = data;
					*(image_dst - imagex - 1) = data;
					image_dst -= 2;
				} else {
					data = *(image_src16--);
					data |= data << 16;
					*(image_dst) = data;
					*(image_dst - imagex / 2) = data;
					image_dst -= 1;
				}
			}
			image_dst -= imagex / (image_color ? 1 : 2);
		}
	}

	if ((picture_rotate == 90) || (picture_rotate == -90)) {
                image_src16 = (uint16_t *)image_buf;
                image_src32 = (uint32_t *)image_buf;
		for (posx = 0; posx < imagex; posx++) {
			if (picture_rotate == 90) rposx = imagex - 1 - posx;
			else rposx = posx;
			for (posy = 0; posy < imagey; posy++) {
				if (picture_rotate == 90) rposy = posy;
				else rposy = imagey - 1 - posy;
				if (image_color) {
					((uint32_t *)rot_buf)[rposy * imagex + rposx] = *(image_src32++);
				} else {
					((uint16_t *)rot_buf)[rposy * imagex + rposx] = *(image_src16++);
				}
			}
		}

		image_buf = rot_buf;
	}

	flattened_images[flattened_images_count].fi_color  = image_color;
	flattened_images[flattened_images_count].fi_width  = imagex;
	flattened_images[flattened_images_count].fi_height = imagey;
	flattened_images[flattened_images_count].fi_xstart = imagexs;
	flattened_images[flattened_images_count].fi_ystart = imageys;
	flattened_images[flattened_images_count].fi_xstop  = imagexs + imagex - 1;
	flattened_images[flattened_images_count].fi_ystop  = imageys + imagey - 1;
	flattened_images[flattened_images_count].fi_buffer = image_buf;

	flattened_images_base += image_len * picture_scale * picture_scale;

	flattened_images_count++;

	return 0;
}

static uint32_t map_bgvalue(uint32_t bgvalue)
{
	if (color_map_enabled && (paint_canvas->map_color != NULL)) {
		return paint_canvas->map_color(bgvalue);
	} else {
		return bgvalue;
	}

}

static void update_framebuffer(void)
{
	uint32_t i;
	size_t frame_size;

	if (!paint_ready) return;

	//copy final pixels from draw buffer to frame buffer
	for (i = 0; i < paint_canvas->num_of_active_planes; i++) {
		frame_size = paint_canvas->y * paint_canvas->planes[i].line_x * paint_canvas->planes[i].pixel_size / 8;
		memcpy( paint_canvas->planes[i].fb_virt, paint_canvas->planes[i].db_virt, frame_size );
	}
}

int paint_update_image(void)
{
	if (!paint_ready) {
		return 0;
	}

	if (paint_canvas->cs == CS_ARGB8101010)
		return paint_update_image_wide_gammut();
	return paint_update_image_argb();
}

static int paint_update_image_argb(void)
{
	uint8_t b_alpha;
	uint32_t cnt, x, y, rgbvalue, bgvalue, bg_mask = 0x00FFFFFF;
	struct flattened_image *fi;
	uint32_t images_xstart = UINT32_MAX;
	uint32_t images_xstop = 0;
	uint32_t images_ystart = UINT32_MAX;
	uint32_t images_ystop = 0;
	void (* plot_func)(struct plane *p, u_int32_t x, u_int32_t y, uint64_t color);

	/* If background color is white and there is an image, make the background black */
	if ((flattened_images_count != 0) && (background_rgb == 0x00FFFFFF)) {
		bg_mask = 0x00000000;
	}

	bgvalue = background_rgb & bg_mask;
	bgvalue = map_bgvalue(bgvalue);

	fill_rect(paint_canvas, 0, 0, window_width, window_height, bgvalue);

	for (cnt = 0; cnt < flattened_images_count; cnt++) {
		flattened_images[cnt].fi_src_pos = 0;
		if (flattened_images[cnt].fi_xstart < images_xstart)
			images_xstart = flattened_images[cnt].fi_xstart;
		if (flattened_images[cnt].fi_xstop > images_xstop)
			images_xstop = flattened_images[cnt].fi_xstop;
		if (flattened_images[cnt].fi_ystart < images_ystart)
			images_ystart = flattened_images[cnt].fi_ystart;
		if (flattened_images[cnt].fi_ystop > images_ystop)
			images_ystop = flattened_images[cnt].fi_ystop;
	}

	// moving the plot function to the stack saves us some time
	// because of -fno-strict-aliasing
	plot_func = paint_canvas->planes[0].plot;

	for (y = images_ystart; y <= images_ystop; y++) {
		for (x = images_xstart; x <= images_xstop; x++) {

			bgvalue = background_rgb & bg_mask;

			for (cnt = 0; cnt < flattened_images_count; cnt++) {
				fi = flattened_images + cnt;

				if ((y < fi->fi_ystart) || (y > fi->fi_ystop)) continue;
				if ((x < fi->fi_xstart) || (x > fi->fi_xstop)) continue;

				if (fi->fi_color) {
					rgbvalue = ((uint32_t *)fi->fi_buffer)[fi->fi_src_pos++];
				} else {
					rgbvalue = ((uint16_t *)fi->fi_buffer)[fi->fi_src_pos++];
					rgbvalue = (rgbvalue << 16) | ((rgbvalue & 0x00FF) << 8) | ((rgbvalue & 0x00FF) << 0);
				}

				if (rgbvalue == 0xFFFFFFFF) {
					rgbvalue = 0xFF000000;
				}

				b_alpha = rgbvalue >> 24;

				if (b_alpha != 0xFF && b_alpha != 0) {
					bgvalue = (((((bgvalue & 0x00FF00FF) * b_alpha) + 0x00FF00FF) >> 8) & 0x00FF00FF) |
						(((((bgvalue & 0x0000FF00) * b_alpha) + 0x0000FF00) >> 8) & 0x0000FF00);

					bgvalue += rgbvalue;
				} else if (b_alpha == 0) {
					bgvalue = rgbvalue;
				}
			}

			bgvalue = map_bgvalue(bgvalue);
			plot_func(&(paint_canvas->planes[0]), x, y, bgvalue);
		}
	}

	update_framebuffer();

	return 0;
}

static uint64_t paint_draw_plot_convert_argb_to_wide_gammut(struct flattened_image *fi, uint32_t bgvalue)
{
	uint8_t b_alpha;
	uint32_t rgbvalue;
	uint64_t tmp = bgvalue;

	if (fi->fi_color) {
		rgbvalue = ((uint32_t *)fi->fi_buffer)[fi->fi_src_pos++];
	} else {
		rgbvalue = ((uint16_t *)fi->fi_buffer)[fi->fi_src_pos++];
		rgbvalue = (rgbvalue << 16) | ((rgbvalue & 0x00FF) << 8) | ((rgbvalue & 0x00FF) << 0);
	}

	if (rgbvalue == 0xFFFFFFFF) {
		rgbvalue = 0xFF000000;
	}

	b_alpha = rgbvalue >> 24;

	if (b_alpha != 0xFF && b_alpha != 0) {
		tmp = (((((tmp & 0x00FF00FF) * b_alpha) + 0x00FF00FF) >> 8) & 0x00FF00FF) |
			(((((tmp & 0x0000FF00) * b_alpha) + 0x0000FF00) >> 8) & 0x0000FF00);

			tmp += rgbvalue;
	} else if (b_alpha == 0) {
		tmp = rgbvalue;
	}
	//Till now, we have a 32 pixel, convert it to 101010
	tmp = (((uint64_t) b_alpha) << 32) | RGB10(RGB_R(tmp) * 4, RGB_G(tmp) * 4, RGB_B(tmp) * 4);
	
	return tmp;
}

static int paint_update_image_wide_gammut(void)
{
	uint32_t cnt, x, y, bgvalue, bg_mask = 0x3FFFFFFF;
	struct flattened_image *fi;
	uint32_t images_xstart = UINT32_MAX;
	uint32_t images_xstop = 0;
	uint32_t images_ystart = UINT32_MAX;
	uint32_t images_ystop = 0;
	void (* plot_func_p0)(struct plane *p, u_int32_t x, u_int32_t y, uint64_t color);
	void (* plot_func_p1)(struct plane *p, u_int32_t x, u_int32_t y, uint64_t color);

	/* If background color is white and there is an image, make the background black */
	if ((flattened_images_count != 0) && (background_rgb == 0x3FFFFFFF)) {
		bg_mask = 0x00000000;
	}

	bgvalue = background_rgb & bg_mask;
	bgvalue = map_bgvalue(bgvalue);

	fill_rect(paint_canvas, 0, 0, window_width, window_height, bgvalue);

	for (cnt = 0; cnt < flattened_images_count; cnt++) {
		flattened_images[cnt].fi_src_pos = 0;
		if (flattened_images[cnt].fi_xstart < images_xstart)
			images_xstart = flattened_images[cnt].fi_xstart;
		if (flattened_images[cnt].fi_xstop > images_xstop)
			images_xstop = flattened_images[cnt].fi_xstop;
		if (flattened_images[cnt].fi_ystart < images_ystart)
			images_ystart = flattened_images[cnt].fi_ystart;
		if (flattened_images[cnt].fi_ystop > images_ystop)
			images_ystop = flattened_images[cnt].fi_ystop;
	}

	// moving the plot function to the stack saves us some time
	// because of -fno-strict-aliasing
	plot_func_p0 = paint_canvas->planes[0].plot;
	plot_func_p1 = paint_canvas->planes[1].plot;

	for (y = images_ystart; y <= images_ystop; y++) {
		for (x = images_xstart; x <= images_xstop; x++) {

			bgvalue = background_rgb & bg_mask;

			for (cnt = 0; cnt < flattened_images_count; cnt++) {
				fi = flattened_images + cnt;

				if ((y < fi->fi_ystart) || (y > fi->fi_ystop)) continue;
				if ((x < fi->fi_xstart) || (x > fi->fi_xstop)) continue;

				bgvalue = paint_draw_plot_convert_argb_to_wide_gammut(fi, bgvalue);
			}

			bgvalue = map_bgvalue(bgvalue);
			plot_func_p0(&(paint_canvas->planes[0]), x, y, bgvalue);
			plot_func_p1(&(paint_canvas->planes[1]), x, y, bgvalue);
		}
	}

	update_framebuffer();

	return 0;
}

/* after the fact get the information about the current display */
int display_get_info(struct display_info *info)
{
	uint32_t rotate = ((360 + mib_get_s32(kMIBTargetOsPictureRotate)) / 90) % 4;
	uint32_t os_picture_scale = mib_get_u32(kMIBTargetOsPictureScale);
	uint32_t depth;

	if (paint_ready) {
		info->stride = window_stride;
		info->width = window_width;
		info->height = window_height;
		depth = (paint_canvas->cs == CS_ARGB8101010) ? 30 : window_depth;
		info->depth = depth | (rotate << 8) | ((os_picture_scale - 1) << 16);
		info->framebuffer = window_framebuffer;
	} else {
		info->stride = 640 * 4;
		info->width = 640;
		info->height = 1136;

		/* 
		 * First setting scale as workaround for <rdar://problem/11342009>.  
		 * This lets us use the display properly even if the iBoot display system 
		 * is not active (e.g. fastsim support). 
		 */
		info->depth = 32 | ((os_picture_scale - 1) << 16);
		info->framebuffer = (uintptr_t)mib_get_addr(kMIBTargetDisplayBaseAddress);
	}

	return 0;
}

void paint_install_gamma_table(uint32_t display_id, 
			       uint32_t entries,
			       volatile uint32_t *red_lut,
			       volatile uint32_t *green_lut,
			       volatile uint32_t *blue_lut)
{
	unsigned int cnt;
	const uint8_t *gamma_data = 0;
	uint32_t gamma_table_count;
	const display_gamma_table *target_gamma_table_base;
	const display_gamma_table *target_gamma_table = 0;
#ifndef PAINT_UNITTEST
	static struct syscfg_glcl syscfg_glcl;

	cnt = syscfgCopyDataForTag('GLCl', (uint8_t *)&syscfg_glcl, sizeof(syscfg_glcl));
	if (cnt == sizeof(syscfg_glcl)) {
		if (syscfg_glcl.major_vers != 1) {
			dprintf(DEBUG_CRITICAL, "paint_install_gamma_table: Unknown version %u.%u\n",
					syscfg_glcl.major_vers, syscfg_glcl.minor_vers);
		} else {
			dprintf(DEBUG_CRITICAL, "int_install_gamma_table: Found syscfg gamma table\n");
			gamma_data = syscfg_glcl.gamma_data;
		}
	}

	gamma_table_count = mib_get_u32(kMIBTargetDisplayGammaTableCount);
#else
	gamma_table_count = 0;
#endif
	if (gamma_table_count != 0) {
#ifndef PAINT_UNITTEST
		target_gamma_table_base = (const display_gamma_table *)mib_get_ptr(kMIBTargetDisplayGammaTablePtr);
#else
		target_gamma_table_base = 0;
#endif
		for (cnt = 0; gamma_data == 0 && cnt < gamma_table_count; cnt++) {
			target_gamma_table = target_gamma_table_base + cnt;
			if ((display_id & target_gamma_table->display_id_mask) == target_gamma_table->display_id) {
				dprintf(DEBUG_CRITICAL, "paint_install_gamma_table: Found Gamma table 0x%08x / 0x%08x\n",
						target_gamma_table->display_id, target_gamma_table->display_id_mask);
				gamma_data = target_gamma_table->display_gamma_data;
			}
		}
	}

	if (gamma_data != 0) {
		paint_install_gamma_table_channel(entries, red_lut, gamma_data + kGammaChannelBufferSize * 0);
		paint_install_gamma_table_channel(entries, green_lut, gamma_data + kGammaChannelBufferSize * 1);
		paint_install_gamma_table_channel(entries, blue_lut, gamma_data + kGammaChannelBufferSize * 2);
	} else {
		printf("paint_install_gamma_table: No Gamma table found for display_id: 0x%08x\n", display_id);
	}

}

void paint_get_syscfg_wpcl(struct syscfg_wpcl *wpcl)
{
	int size;

#ifndef PAINT_UNITTEST
	size = syscfgCopyDataForTag('WpCl', (u_int8_t *)wpcl, sizeof(struct syscfg_wpcl));
#else
	size = 0;
#endif
	if ((size != sizeof(struct syscfg_wpcl)) || (0 == wpcl->version)) {
		wpcl->version = 0;
		wpcl->red = wpcl->green = wpcl->blue =  (1 << WpCl_Quotation_Denominator);
	}
}

#if DEBUG_BUILD

static uint32_t displaytest_colors_argb[] = {
 	0x00FF0000, 0x00000000, 0x0000FF00, 0x00000000, 0x000000FF, 0x00000000, 0x00FFFFFF, 0x00000000
 };
#define DISPLAYTEST_COLORS_COUNT_ARGB (sizeof(displaytest_colors_argb) / sizeof(displaytest_colors_argb[0]))

static uint32_t displaytest_colors_wide_gammut[] = {
	0x3FF00000, 0x00000000, 0x000FFC00, 0x00000000, 0x000003FF, 0x00000000, 0x3FFFFFFF, 0x00000000
};
#define DISPLAYTEST_COLORS_COUNT_WG (sizeof(displaytest_colors_wide_gammut) / sizeof(displaytest_colors_wide_gammut[0]))

int paint_displaytest(void)
{
	uint32_t cnt, displaytes_color_count;
	uint32_t *displaytest_colors;

	if (!paint_ready) {
		return 0;
	}
	if (paint_canvas->cs == CS_ARGB8101010) {
		displaytest_colors = displaytest_colors_wide_gammut;
		displaytes_color_count = DISPLAYTEST_COLORS_COUNT_WG;
	} else {
		displaytest_colors = displaytest_colors_argb;
		displaytes_color_count = DISPLAYTEST_COLORS_COUNT_ARGB;
	}

	for (cnt = 0; cnt < displaytes_color_count; cnt++) {
		fill_rect(paint_canvas, cnt, cnt, window_width - (2 * cnt), window_height - (2 * cnt), displaytest_colors[cnt]);
	}

	update_framebuffer();
	return 0;
}

#endif

static uint32_t pig_offset;
static uint32_t pig_bitpos;
static uint8_t  pig_data;

static void paint_install_gamma_table_channel(uint32_t entries, volatile uint32_t *channel_lut, const uint8_t *bits)
{
	uint32_t cnt;
	uint16_t lasts;
	uint8_t  lastc;
	int8_t    nibble;

	nibble = 0;
	lastc = 0;
	lasts = 0;

	channel_lut[0] = lasts;

	pig_offset = 0;
	pig_bitpos = 0;
	pig_data = 0;

	for (cnt = 1; cnt < entries; cnt++) {
		switch (paint_install_gamma_get_2bits(bits)) {
			case 0: nibble = 0; break;
			case 1: nibble = 1; break;
			case 3: nibble = -1; break;
			case 2:
				nibble = paint_install_gamma_get_2bits(bits) | (paint_install_gamma_get_2bits(bits) << 2);
				if ((nibble & 8) == 8) nibble |= 0xF0;
				break;
		}

		if (cnt == 1) nibble += 8;

		lastc += nibble;
		lasts += lastc;
		channel_lut[cnt] = lasts;
	}
}

static uint8_t paint_install_gamma_get_2bits(const uint8_t *bits)
{
	uint8_t bits2;

	if (pig_bitpos == 0) {
		pig_data = bits[pig_offset++];
	}

	bits2 = (pig_data >> pig_bitpos) & 3;

	pig_bitpos += 2;
	if (pig_bitpos == 8) pig_bitpos = 0;

	return bits2;
}

void fill_rect(struct canvas *c, uint32_t top_x, uint32_t top_y, uint32_t w, uint32_t h, uint64_t color)
{
	uint32_t y;
	
	if (top_x >= c->x || top_y >= c->y)
		return;
	if (top_x + w > c->x)
		w = c->x - top_x;
	if (top_y + h > c->y)
		h = c->y - top_y;

	for (y = top_y; y < top_y + h; y++) {
		struct plane *p0 = &(c->planes[0]);
		struct plane *p1 = &(c->planes[1]);
		p0->hline(p0, top_x, y, w, color);
		if (p1 && p1->hline)  p1->hline(p1, top_x, y, w, color);
	}
}

void plot(struct plane *p, uint32_t x, uint32_t y, uint64_t color)
{
	p->plot(p, x, y, color);
}

uint32_t get_pixel(struct plane *p, uint32_t x, uint32_t y)
{
	return p->get_pixel(p, x, y);
}


struct canvas *get_canvas(void)
{
	return paint_canvas;
}

struct plane *get_plane(uint32_t plane_num)
{
	if (!paint_canvas)
		return NULL;

	RELEASE_ASSERT(plane_num < MAX_NUM_PLANES_PER_CANVAS);
	return (&(paint_canvas->planes[plane_num]));
}


#if SYSCFG_DCLR_OVERRIDE
// Testing function to permit specification of 'DClr' syscfg data in
// environment variables for devices that don't actually have 'DClr' yet.
static int dclrOverride(uint8_t *DClr, size_t length)
{
	unsigned int	i, j;
	const char	*env;
	size_t		envLength;
	uint8_t		c;

	if ((DClr == NULL) || (length != sizeof(syscfg_DClr_t))) {
		return -1;
	}

	env = env_get("DClr_override");
	if (env == NULL) {
		return -1;
	}

	envLength = strlen(env);
	if (envLength != (length * 2)) {
		dprintf(DEBUG_CRITICAL, "DClr: DClr_override length is %zu should be %zu\n", envLength, length * 2);
		return -1;
	}

	for (i = 0; i < length; ++i) {
		DClr[i] = 0;
		for (j = 0; j < 2; ++j) {
			c = *env++;
			if (c >= '0' && c <= '9')
				c -= '0';
			else if (c >= 'A' && c <= 'F')
				c -= 'A' - 10;
			else if (c >= 'a' && c <= 'f')
				c -= 'a' - 10;
			else {
				dprintf(DEBUG_CRITICAL, "DClr: DClr_override: not a hex digit: %c\n", c);
				return -1;
			}

			DClr[i] += c << (4 * (1 - j));
		}
	}
	return length;
}
#endif	// SYSCFG_DCLR_OVERRIDE

#if SYSCFG_DCLR_COLOR_REPORT
static void report_color(const char *title, u_int8_t r, u_int8_t g, u_int8_t b)
{
	dprintf(DEBUG_INFO, "DClr: %s R/G/B = %d/%d/%d (0x%08X)\n", title, r, g, b, RGB(r, g, b));
}
#endif

int paint_color_map_get_DClr(syscfg_DClr_t *DClr, size_t size, bool report)
{
	int	result;
	bool	die = false;

	// Validate parameters.
	if ((DClr == NULL) || (size != sizeof(*DClr))) {
		dprintf(DEBUG_CRITICAL, "DClr: paint_color_map_get_DClr: Bad parameters\n");
		die = PANIC_ON_SERIOUS_ERROR;
		goto fail;
	}

#if SYSCFG_DCLR_OVERRIDE
	result = dclrOverride((uint8_t *)DClr, size);
	if (result == sizeof(*DClr)) {
		dprintf(DEBUG_INFO, "DClr: Overriding syscfg data from environment\n");
	} else
#endif
	{
		// Look up the 'DClr' tag from syscfg.
#ifndef PAINT_UNITTEST
		result = syscfgCopyDataForTag('DClr', (uint8_t *)DClr, sizeof(*DClr));
#else
		result = 0;
#endif
		if (result < 0) {
#ifndef PAINT_UNITTEST
			result = target_dclr_from_clrc((uint8_t *)DClr, sizeof(*DClr));
#else
		result = 0;
#endif
			if (result < 0) {
				dprintf(DEBUG_INFO,
					"DClr: syscfg entry not found -- color remapping disabled\n");
				return 0;
			}
		}
	}

	if (result != sizeof(*DClr)) {
		dprintf(DEBUG_CRITICAL, "DClr: syscfgCopyDataForTag returned invalid length: %d\n",
			result);
		// Defer panic to bootprep if an attempt is made to boot an OS with this error enabled.
		color_map_invalid |= PANIC_ON_SERIOUS_ERROR;
		goto fail;
	}

	switch (DCLR_VERSION(DClr->major_version, DClr->minor_version)) {
		case DCLR_VERSION(2,0):
			if (report) {
#if SYSCFG_DCLR_COLOR_REPORT
				report_color("Device enclosure",
					     DClr->device_enclosure.component.red,
					     DClr->device_enclosure.component.green,
					     DClr->device_enclosure.component.blue);
				report_color("Device cover glass",
					     DClr->cover_glass.component.red,
					     DClr->cover_glass.component.green,
					     DClr->cover_glass.component.blue);
#endif
			}
			break;

		default:
			dprintf(DEBUG_CRITICAL, "DClr: Unknown version: %d.%d\n",
				DClr->major_version, DClr->minor_version);
			// Defer panic to bootprep if an attempt is made to boot an OS with this error enabled.
			color_map_invalid |= mib_get_bool(kMIBTargetPanicOnUnknownDclrVersion);
			goto fail;
	}

	return result;

fail:
	if (die) {
		panic("DClr: Fatal error");
	}
	return 0;
}

// Standard color remapping function for COLOR_MAP_POLICY_INVERT
static uint32_t invert_color_rbg888(uint64_t color)
{
	color ^= 0x00FFFFFF;
	return color;
}

static uint32_t invert_color_rbg101010(uint64_t color)
{
	color ^= 0x3FFFFFFF;
	return color;
}

void color_map_init(struct canvas *c)
{
	color_policy_t *target_policy;
	syscfg_DClr_t	DClr;
	int		result;
	bool		die = false;

	// Assume color remapping is not supported.
	color_map_desired = false;
	color_map_enabled = false;
	color_map_supported = false;
	color_map_invalid = false;

	// Must have a canvas.
	if (c == NULL) {
		dprintf(DEBUG_CRITICAL, "Bad parameter: color_map_init(NULL)\n");
		die = PANIC_ON_SERIOUS_ERROR;
		goto fail;
	}

	// We only support the RGB888 colorspace and ARGB8101010.
	if ((c->cs != CS_RGB888) && (c->cs != CS_ARGB8101010)) {
		dprintf(DEBUG_CRITICAL, "DClr: Only the RBG888 colorspace is supported\n");
		goto fail;
	}

	// Ask the target what it's color remapping policy is.
#ifndef PAINT_UNITTEST
	target_policy = target_color_map_init(c->cs, &color_policy);
#else
	target_policy = 0;
#endif
	if (target_policy == NULL) {
		goto fail;
	}

	switch (target_policy->policy_type) {
	case COLOR_MAP_POLICY_NONE:	// Standard policy: No color remapping
		return;

	case COLOR_MAP_POLICY_INVERT:	// Standard policy: Invert colors
	{
		color_policy_invert_t  *color_table;	// Target color table
		uint32_t		cover_glass;	// Cover glass color
		uint8_t			r, g, b;	// Color components
		unsigned int		i;

		// Validate what the target gave us.
		if ((target_policy->color_table == NULL) || (target_policy->color_count == 0)) {
			dprintf(DEBUG_CRITICAL, "target_color_map_init() returned invalid color table\n");
			die = PANIC_ON_SERIOUS_ERROR;
			goto fail;
		}

		// Look up the device's 'DClr' syscfg entry.
		result = paint_color_map_get_DClr(&DClr, sizeof(DClr), true);
		if (result != sizeof(DClr)) {
			goto fail;
		}

		// Get the cover glass color.
		switch (DCLR_VERSION(DClr.major_version, DClr.minor_version)) {

			case DCLR_VERSION(2,0):
				r = DClr.cover_glass.component.red;
				g = DClr.cover_glass.component.green;
				b = DClr.cover_glass.component.blue;
				cover_glass = ColorRGB888(DClr.cover_glass.rgb);
				break;

			default:
				// Should have been caught in color_map_get_DClr().
				// Defer panic to bootprep if an attempt is made to
				// boot an OS with this error enabled.
				color_map_invalid |= PANIC_ON_SERIOUS_ERROR;
				goto fail;
		}

		// Get the target's inversion color table.
		color_table = (color_policy_invert_t *)(target_policy->color_table);

		// Match the cover glass color against the expected colors for this target.
		for (i = 0; i < target_policy->color_count; ++i) {
			if (cover_glass == color_table[i].cover_glass) {
				break;
			}
		}

		if (i >= target_policy->color_count) {
			dprintf(DEBUG_CRITICAL,
				"DClr: Unsupported cover glass color: R/G/B = %d/%d/%d (0x%08X)\n",
				r, g, b, cover_glass);
			// Defer panic to bootprep if an attempt is made to boot an OS with this error enabled.
			color_map_invalid |= PANIC_ON_UNKNOWN_DCLR_COLOR;
			goto fail;
		}

		dprintf(DEBUG_INFO, "DClr: Color mapping policy is to %s colors\n",
			color_table[i].invert ? "invert" : "keep");

		// If the target policy is to invert the colors, then set the
		// color remapping functions. Otherwise, leave the function
		// pointers NULL to keep the source background/foreground colors.
		if (color_table[i].invert) {
			if (c->cs == CS_ARGB8101010) c->map_color = invert_color_rbg101010;
			else c->map_color = invert_color_rbg888;
			color_map_desired = true;
		}

		color_map_supported = true;
		return;
	}

	default:
		dprintf(DEBUG_CRITICAL, "target_color_map_init() returned unknown policy type: 0x%08X\n",
			target_policy->policy_type);
		die = PANIC_ON_SERIOUS_ERROR;
		goto fail;
	}

fail:
	color_policy.policy_type = COLOR_MAP_POLICY_NONE;
	color_map_desired = false;
	color_map_enabled = false;
	color_map_supported = false;

	if (die) {
		panic("DClr: Fatal error");
	}
}

bool paint_color_map_enable(bool enable)
{
	bool result = color_map_enabled;
	if (!color_map_supported) {
		enable = false;
	}
	color_map_enabled = enable;
	dprintf(DEBUG_SPEW, "DClr: Color remapping %sabled\n",
		color_map_enabled ? "en" : "dis");

	return result;
}

bool paint_color_map_is_desired(void)
{
	return color_map_desired;
}

bool paint_color_map_is_enabled(void)
{
	return color_map_enabled;
}

bool paint_color_map_is_invalid(void)
{
	return color_map_invalid;
}

enum color_map_policy paint_get_color_policy(void)
{
	enum color_map_policy	policy = color_policy.policy_type;

	switch (policy) {
		case COLOR_MAP_POLICY_NONE:
			break;

		case COLOR_MAP_POLICY_INVERT:
			if (!color_map_desired) {
				policy = COLOR_MAP_POLICY_NONE;
			}
			break;

		default:
			break;
	}

	return policy;
}

static addr_t alloc_display_valid_region_memory(size_t frame_size_in_bytes)
{
	RELEASE_ASSERT(page_size != 0);
	RELEASE_ASSERT(last_canvas_addr != 0);

 	addr_t addr = last_canvas_addr;
 	last_canvas_addr += (frame_size_in_bytes + page_size - 1) & ~((addr_t)(page_size - 1));
 
	return addr;
}

void display_init_alloc_mem(void * fb)
{
 	page_size = mib_get_size(kMIBPlatformPageSize);
 	last_canvas_addr = (addr_t) fb;
}

void set_canvas(struct canvas *c, void *fb, size_t region_size, uint32_t x, uint32_t y, uint32_t line_x, enum colorspace cs)
{	
	struct plane *p = &(c->planes[0]);
	struct plane *p1 = &(c->planes[1]);


	switch(cs) {
		case CS_4BPP:
			c->num_of_active_planes = 1;
			p->pixel_size = 4;
			p->pixel_size_per_channel = 8;
			p->width = x;
			p->height = y;
			p->stride = line_x * 1; // In bytes
			p->plot = &plot4;
			p->get_pixel = &get_pixel4;
			p->hline = &hline4;
			p->vline = &vline4;
			break;
		case CS_8BPP:
		case CS_RGB332:
			c->num_of_active_planes = 1;
			p->pixel_size = 8;
			p->pixel_size_per_channel = 8;
			p->width = x;
			p->height = y;
			p->stride = line_x * 4; // In bytes
			p->plot = &plot8;
			p->get_pixel = &get_pixel8;
			p->hline = &hline8;
			p->vline = &vline8;
			break;
		case CS_RGB565:
		case CS_ARGB1555:
		case CS_ARGB4444:
			c->num_of_active_planes = 1;
			p->pixel_size = 16;
			p->pixel_size_per_channel = 8;
			p->width = x;
			p->height = y;
			p->stride = line_x * 4; // In bytes
			p->plot = &plot16;
			p->get_pixel = &get_pixel16;
			p->hline = &hline16;
			p->vline = &vline16;
			break;
		case CS_RGB888:
		case CS_ARGB8888:
			c->num_of_active_planes = 1;
			p->pixel_size = 32;
			p->pixel_size_per_channel = 8;
			p->width = x;
			p->height = y;
			p->stride = line_x * 4; // In bytes
			p->plot = &plot32;
			p->get_pixel = &get_pixel32;
			p->hline = &hline32;
			p->vline = &vline32;
			break;
		//10-bit RGB components are stored in packed 4 bytes (same as RGB101010) in RGB10 plane.
		//8-bit Alpha is stored in packed format in A8 plane
		case CS_ARGB8101010:
			c->num_of_active_planes = 2;
			p->pixel_size = 32;
			p->pixel_size_per_channel = 10;
			p->width = x;
			p->height = y;
			p->stride = line_x * 4; // In bytes
			p->plot = &plot32;
			p->get_pixel = &get_pixel32;
			p->hline = &hline32;
			p->vline = &vline32;
			p1->pixel_size = 8;
			p1->pixel_size_per_channel = 8;
			p1->width = x;
			p1->height = y;
			p1->stride = line_x * 1; // In bytes
			p1->plot = &plot8;
			p1->get_pixel = &get_pixel8;
			p1->hline = &hline8;
			p1->vline = &vline8;
			break;
	}
	c->fb_virt = fb;
	c->fb_phys = (uintptr_t)fb;
	c->x = x;
	c->y = y;
	c->cs = cs;
	c->map_color = NULL;

	//as pixel operations take time, directly drawing on canvas will generate blinking on animation
	//so move the time consuming process on seperated buffer, and then copy the final result into canvas
	//this process requires 2 buffers. The allocation will give the memory for HW used framebuffers, followed by the copies
	display_init_alloc_mem(fb);

 	addr_t plane_0_fb_addr, plane_0_db_addr, plane_1_fb_addr, plane_1_db_addr;
 	size_t plane_0_frame_size_in_bytes , plane_1_frame_size_in_bytes;

	plane_0_frame_size_in_bytes = y * line_x * c->planes[0].pixel_size/8; 
	if (c->num_of_active_planes > 1) plane_1_frame_size_in_bytes = y * line_x * c->planes[1].pixel_size/8; 

	//Allocate plane0 framebuffer
	plane_0_fb_addr = alloc_display_valid_region_memory(plane_0_frame_size_in_bytes);
	//if needed, allocate plane1 framebuffer
	if (c->num_of_active_planes > 1) plane_1_fb_addr = alloc_display_valid_region_memory(plane_1_frame_size_in_bytes);

	//Allocate plane0 buffer for drawing
 	plane_0_db_addr = alloc_display_valid_region_memory(plane_0_frame_size_in_bytes);
	//Allocate plane1 buffer for drawing, if needed
	if (c->num_of_active_planes > 1) plane_1_db_addr = alloc_display_valid_region_memory(plane_1_frame_size_in_bytes);

	set_plane(&(c->planes[0]), y, line_x, plane_0_frame_size_in_bytes, plane_0_fb_addr, plane_0_db_addr, cs);
	if (c->num_of_active_planes > 1) set_plane(&(c->planes[1]), y, line_x, plane_1_frame_size_in_bytes, plane_1_fb_addr, plane_1_db_addr, cs);

	// Initialize color remapping if supported on this device.
	color_map_init(c);
}

static void set_plane(struct plane *p, uint32_t y, uint32_t line_x, size_t plane_size, addr_t plane_fb, addr_t plane_db, enum colorspace cs)
{
  	p->line_x = line_x;
  	p->cs = cs;
 	p->fb_virt = (void *) plane_fb;
	p->plane_size = plane_size;
  	p->db_virt = (void *) plane_db;
}

static void plot4(struct plane *p, uint32_t x, uint32_t y, uint64_t color)
{
	panic("plot4 not implemented yet\n");
}

static void plot8(struct plane *p, uint32_t x, uint32_t y, uint64_t color)
{
	if (p->cs == CS_8BPP) color = ColorIndex8BPP(color);
	else color = ColorRGB332(color);

	((uint8_t *)p->db_virt)[y * p->line_x + x] = color;
}

static void plot16(struct plane *p, uint32_t x, uint32_t y, uint64_t color)
{
	if (p->cs == CS_ARGB1555) color = ColorRGB555(color);
	else if (p->cs == CS_ARGB4444) color = ColorRGB444(color);
	else color = ColorRGB565(color);

	((uint16_t *)p->db_virt)[y * p->line_x + x] = color;
}

static void plot32(struct plane *p, uint32_t x, uint32_t y, uint64_t color)
{
	if (p->cs == CS_ARGB8101010) color = ColorRGB101010(color);
	else color = ColorRGB888(color);

	((uint32_t *)p->db_virt)[y * p->line_x + x] = color;
}

static uint32_t get_pixel4(struct plane *p, uint32_t x, uint32_t y)
{
	panic("get_pixel4 not implemented yet\n");

	return 0;
}

static uint32_t get_pixel8(struct plane *p, uint32_t x, uint32_t y)
{
	panic("get_pixel8 not implemented yet\n");

	return 0;
}

static uint32_t get_pixel16(struct plane *p, uint32_t x, uint32_t y)
{
	panic("get_pixel16 not implemented yet\n");

	return 0;

}

static uint32_t get_pixel32(struct plane *p, uint32_t x, uint32_t y)
{
	return ((uint32_t *)p->fb_virt)[y * p->line_x + x] & 0x00FFFFFF;
}

static void hline4(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color)
{
	panic("hline4 not implemented yet\n");
}

static void hline8(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color)
{
	uint8_t *ptr;

	if (p->cs == CS_8BPP) color = ColorIndex8BPP(color);
	else if (p->cs == CS_ARGB8101010) color = ColorA8(color);
	else color = ColorRGB332(color);

	ptr = &((uint8_t *)p->db_virt)[y * p->line_x + x];
	while (len > 0) {
		*ptr = color;
		ptr++;
		len--;
	}
}

static void hline16(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color)
{
	uint16_t *ptr;

	if (p->cs == CS_ARGB1555) color = ColorRGB555(color);
	else if (p->cs == CS_ARGB4444) color = ColorRGB444(color);
	else color = ColorRGB565(color);

	ptr = &((uint16_t *)p->db_virt)[y * p->line_x + x];
	while (len > 0) {
		*ptr = color;
		ptr++;
		len--;
	}
}

static void hline32(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color)
{
	uint32_t *ptr;

	if (p->cs == CS_ARGB8101010) color = ColorRGB101010(color);
	else color = ColorRGB888(color);

	ptr = &((uint32_t *)p->db_virt)[y * p->line_x + x];
	while (len > 0) {
		*ptr = color;
		ptr++;
		len--;
	}
}

static void vline4(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color)
{
	panic("vline4 not implemented yet\n");
}

static void vline8(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color)
{
	uint8_t *ptr;

	if (p->cs == CS_8BPP) color = ColorIndex8BPP(color);
	else color = ColorRGB332(color);

	ptr = &((uint8_t *)p->db_virt)[y * p->line_x + x];
	while (len > 0) {
		*ptr = color;
		ptr += p->line_x;
		len--;
	}
}

static void vline16(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color)
{
	uint16_t *ptr;

	if (p->cs == CS_ARGB1555) color = ColorRGB555(color);
	else if (p->cs == CS_ARGB4444) color = ColorRGB444(color);
	else color = ColorRGB565(color);

	ptr = &((uint16_t *)p->db_virt)[y * p->line_x + x];
	while (len > 0) {
		*ptr = color;
		ptr += p->line_x;
		len--;
	}
}

static void vline32(struct plane *p, uint32_t x, uint32_t y, uint32_t len, uint64_t color)
{
	uint32_t *ptr;

	if (p->cs == CS_ARGB8101010) color = ColorRGB101010(color);
	else color = ColorRGB888(color);

	ptr = &((uint32_t *)p->db_virt)[y * p->line_x + x];
	while (len > 0) {
		*ptr = color;
		ptr += p->line_x;
		len--;
	}
}

#if DEBUG_BUILD
static char *cs_to_string(enum colorspace cs) {

	switch (cs) {
	case CS_4BPP: return "4BPP";
	case CS_8BPP: return "8BPP";
	case CS_RGB332: return "RGB332";
	case CS_RGB565: return "RGB565";
	case CS_ARGB1555: return "ARGB1555";
	case CS_ARGB4444: return "ARGB4444";
	case CS_RGB888: return "RGB888";
	case CS_ARGB8888: return "ARGB8888";
	case CS_ARGB8101010: return "ARGB8101010";
	}
}

void dump_paint_plane(struct plane *p, uint32_t plane_num)
{
	printf("\t\tPlane %d\n", plane_num);
	printf("\t\t\tcs %s\n", cs_to_string(p->cs));
	printf("\t\t\tfb_virt %p\n", p->fb_virt);
	printf("\t\t\tdb_virt %p\n", p->db_virt);
	printf("\t\t\tplane_size %u\n", p->plane_size);
	printf("\t\t\tpixel_size %u\n", p->pixel_size);
	printf("\t\t\tpixel_size_per_channel %u\n", p->pixel_size_per_channel);
	printf("\t\t\tline_x %u\n", p->line_x);
	printf("\t\t\twidth %u\n", p->width);
	printf("\t\t\theight %u\n", p->height);
	printf("\t\t\tstride %u\n", p->stride);
	printf("\t\t\tplot %p\n", p->plot);
	printf("\t\t\tget_pixel %p\n", p->get_pixel);
	printf("\t\t\thline %p\n", p->hline);
	printf("\t\t\tvline %p\n", p->vline);

}

void dump_paint_canvas(struct canvas *c)
{
	printf("\tCanvas:\n");
	printf("\t\tcs %s\n", cs_to_string(c->cs));
	printf("\t\tnum_of_active_planes %u\n", c->num_of_active_planes);
	printf("\t\tfb_virt %p\n", c->fb_virt);
	printf("\t\tfb_phys %lu\n", c->fb_phys);
	printf("\t\tx %u\n", c->x);
	printf("\t\ty %u\n", c->y);
	printf("\t\tmap_color %p\n", c->map_color);
	for (uint32_t i = 0; i < c->num_of_active_planes; i++) {
		dump_paint_plane(&(c->planes[i]), i);
	}
}

void dump_paint_information(struct display_window *w)
{
	printf("Window:\n");
		printf("\tactive\t%d\n", w->active);
		printf("\tcs\t%d\n", w->cs);
		printf("\tdepth\t%d\n", w->depth);
		printf("\tpos_x\t%d\n", w->pos_x);
		printf("\tpos_y\t%d\n", w->pos_y);
		printf("\twidth\t%d\n", w->width);
		printf("\theight\t%d\n", w->height);
		printf("\tstride\t%d\n", w->stride);
		dump_paint_canvas(&(w->c));
}

void display_paint_plane(uint32_t plane_num)
{
	struct plane *p = &(paint_canvas->planes[plane_num]);
	dump_paint_plane(p, plane_num);
}

void display_paint_canvas()
{
	dump_paint_canvas(paint_canvas);
}

void display_paint_information()
{
	dump_paint_information(paint_window);
}
#endif // #if DEBUG_BUILD
