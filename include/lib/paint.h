/*
 * Copyright (C) 2007-2013 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __LIB_PAINT_H
#define __LIB_PAINT_H

#include <sys/types.h>
#include <lib/env.h>
#include <lib/image.h>

__BEGIN_DECLS

enum colorspace {
	CS_4BPP,
	CS_8BPP,
	CS_RGB332,
	CS_RGB565,
	CS_ARGB1555,
	CS_ARGB4444,
	CS_RGB888,
	CS_ARGB8888,
	CS_ARGB8101010,
};

#define ColorIndec4BPP(color)		(0)
#define ColorIndex8BPP(color)		(0)
#define ColorRGB332(color)		(((color >> 6) & 0x03) | ((color >> 11) & 0x1C) | ((color >> 16) & 0xE0))
#define ColorRGB565(color)		(((color >> 3) & 0x001F) | ((color >> 5) & 0x07E0) | ((color >> 8) & 0xF800))
#define ColorRGB555(color)		(((color >> 3) & 0x001F) | ((color >> 6) & 0x03E0) | ((color >> 9) & 0x7C00))
#define ColorRGB444(color)		(((color >> 4) & 0x000F) | ((color >> 8) & 0x00F0) | ((color >> 12) & 0x0F00))
#define ColorRGB888(color)		(color & 0x00FFFFFF)
#define ColorRGB101010(color)		(color & 0x3FFFFFFF)
#define ColorA8(color)			((color >> 32)& 0xFF)
#define RGB(r,g,b)			(((uint32_t)((r) & 0xFF) << 16) | ((uint32_t)((g) & 0xFF) << 8) | ((uint32_t)((b) & 0xFF) << 0))
#define RGB_MASK			RBG(0xFF, 0xFF, 0xFF)
#define RGB_R(c)			((uint32_t)(((c) >> 16) & 0xFF))
#define RGB_G(c)			((uint32_t)(((c) >>  8) & 0xFF))
#define RGB_B(c)			((uint32_t)(((c) >>  0) & 0xFF))
#define RGB10(r,g,b)			(((uint32_t)((r) & 0x3FF) << 20) | ((uint32_t)((g) & 0x3FF) << 10) | ((uint32_t)((b) & 0x3FF) << 0))
#define RGB10_MASK			RBG10(0x3FF, 0x3FF, 0x3FF)
#define RGB10_R(c)			((uint32_t)(((c) >> 20) & 0x3FF))
#define RGB10_G(c)			((uint32_t)(((c) >>  10) & 0x3FF))
#define RGB10_B(c)			((uint32_t)(((c) >>  0) & 0x3FF))
#define DCLR_VERSION(major,minor)	((((major) & 0xFF) << 8) | (((minor) & 0xFF) << 0))

#define WpCl_Quotation_Denominator	(16)

#define MAX_NUM_PLANES_PER_CANVAS	2

struct syscfg_wpcl {
	u_int32_t	version;
	u_int32_t	red;
	u_int32_t	green;
	u_int32_t	blue;
};

union  syscfg_DCLr_RGB {
	u_int32_t	rgb;
	struct {
		u_int8_t	blue;
		u_int8_t	green;
		u_int8_t	red;
		u_int8_t	reserved;
	} component;
};

typedef struct syscfg_DClr {
	u_int8_t		minor_version;
	u_int8_t		major_version;
	u_int8_t		reserved_2;
	u_int8_t		reserved_3;

	union syscfg_DCLr_RGB	device_enclosure;

	union syscfg_DCLr_RGB	cover_glass;

	u_int8_t		reserved_C;
	u_int8_t		reserved_D;
	u_int8_t		reserved_E;
	u_int8_t		reserved_F;
} syscfg_DClr_t;

typedef enum {
        clrcColorUnknown = -1,
        clrcColorBlack   = 0,
        clrcColorWhite   = 1,
        clrcColorRed     = 2,
        clrcColorSilver  = 3,
        clrcColorPink    = 4,
        clrcColorBlue    = 5,
        clrcColorYellow  = 6,
        clrcColorGold    = 7,
        clrcColorSparrow = 8,
        clrcColorCount
} clrcColor;

typedef struct syscfg_ClrC {
	u_int8_t		clrcColor;	// clrcColor
	u_int8_t		reserved[15];
} syscfg_ClrC_t;

// Color remapping policy options a target may select
enum color_map_policy {
	COLOR_MAP_POLICY_NONE,		// Common remapping: No color remapping (legacy devices)
	COLOR_MAP_POLICY_INVERT,	// Common remapping: Invert colors
};

// Function pointer typedefs for color remappers.
typedef uint32_t (* map_color_t)(uint64_t color);    // Target specific color remapping

// Color remapping policy data
typedef struct color_policy {
	enum color_map_policy	policy_type;	// Color remapping policy type
	void		       *color_table;	// Pointer to policy-specific color table (e.g., a color_policy_invert_t)
	uint32_t		color_count;	// Number of entries in the color table
	map_color_t		map_color;	// Pointer to color remapping function
} color_policy_t;

// Color remapping table provided by the target for COLOR_MAP_POLICY_INVERT.
typedef struct color_policy_invert {
	uint32_t	cover_glass;	// Cover glass color
	bool		invert;		// Invert bg/fg colors if true
} color_policy_invert_t;

struct plane {
	enum colorspace	cs;
	void		*fb_virt;
	void *		db_virt;     //draw buffer, start: fb_virt + frame size, align to next 4K
	u_int32_t	plane_size;
	u_int32_t	pixel_size;
	u_int32_t	pixel_size_per_channel;
	u_int32_t	line_x;
	u_int32_t	width;
	u_int32_t	height;
	u_int32_t	stride;
	void		(* plot)(struct plane *p, u_int32_t x, u_int32_t y, uint64_t color);
	u_int32_t	(* get_pixel)(struct plane *p, u_int32_t x, u_int32_t y);
	void		(* hline)(struct plane *p, u_int32_t x, u_int32_t y, u_int32_t len, uint64_t color);
	void		(* vline)(struct plane *p, u_int32_t x, u_int32_t y, u_int32_t len, uint64_t color);
};

struct canvas {
	enum colorspace	cs;
	struct plane	planes[MAX_NUM_PLANES_PER_CANVAS];
	uint32_t	num_of_active_planes;
	void		*fb_virt;
	uintptr_t	fb_phys;
	u_int32_t	x;
	u_int32_t	y;
	map_color_t	map_color;
};

int paint_init(void *window, addr_t scratch, size_t scratch_size);
int paint_set_bgcolor(uint16_t bg_red, uint16_t bg_green, uint16_t bg_blue);
int paint_set_picture_for_tag(u_int32_t image_tag);
int paint_set_picture_for_tag_list(u_int32_t image_tag, int image_offset, int image_count);
int paint_set_picture(struct image_info *pict_info);
int paint_set_picture_list(struct image_info *pict_info, int image_offset, int image_count);
int paint_set_picture_from_file(const char *path, uint32_t type);
int paint_update_image(void);

void paint_install_gamma_table(u_int32_t display_id,
				      u_int32_t entries,
				      volatile u_int32_t *red_lut,
				      volatile u_int32_t *green_lut,
				      volatile u_int32_t *blue_lut);

void paint_get_syscfg_wpcl(struct syscfg_wpcl *wpcl);

color_policy_t *paint_color_map_init(enum colorspace cs);
bool paint_color_map_enable(bool enable);
bool paint_color_map_is_desired(void);
bool paint_color_map_is_enabled(void);
bool paint_color_map_is_invalid(void);
enum color_map_policy paint_get_color_policy(void);
int paint_color_map_get_DClr(syscfg_DClr_t *DClr, size_t size, bool report);

int paint_displaytest(void);

struct canvas *get_canvas(void);
void set_canvas(struct canvas *c, void *fb, size_t region_size, uint32_t x, uint32_t y, uint32_t line_x, enum colorspace cs);
void move_canvas(struct canvas *c, void *fb);

struct plane *get_plane(uint32_t plane_num);

void plot(struct plane *p, u_int32_t x, u_int32_t y, uint64_t color);
u_int32_t get_pixel(struct plane *p, u_int32_t x, u_int32_t y);
void fill_rect(struct canvas *c, u_int32_t top_x, u_int32_t top_y, u_int32_t w, u_int32_t h, uint64_t color);


void display_paint_information();
void display_paint_canvas();
void display_paint_plane(uint32_t plane_num);
__END_DECLS

#endif /* __LIB_PAINT_H */
