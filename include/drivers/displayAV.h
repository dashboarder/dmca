/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef DRIVERS_DISPLAYAV_H
#define DRIVERS_DISPLAYAV_H	1


enum {
        kDisplayAxisTypeHorizontal,
        kDisplayAxisTypeVertical,
        kDisplayAxisCount
};

enum {
        kDisplayColorSpacesRGB,
        kDisplayColorSpacesYCbCr422,
        kDisplayColorSpacesYCbCr444
};

enum {
	kDisplayColorDynamicRangeFull,
	kDisplayColorDynamicRangeLimited,
	kDisplayColorDynamicRangeVESA   = kDisplayColorDynamicRangeFull,
	kDisplayColorDynamicRangeCEA    = kDisplayColorDynamicRangeLimited
};

enum {
        kDisplayColorCoefficientITU601,               
        kDisplayColorCoefficientITU709
};

enum {
    kDisplayTestModeOff,
    kDisplayTestModeColorRamp,
    kDisplayTestModeBlackWhiteVLines,
    kDisplayTestModeColorSquare,
    kDisplayTestModeInvalid,
    kDisplayTestModeColorBar32,
    kDisplayTestModeColorBar64,
    kDisplayTestModeWhiteGrayBlackBar32,
    kDisplayTestModeWhiteGrayBlackBar64,
    kDisplayTestModeMobileWhiteBar32,
    kDisplayTestModeMobileWhiteBar64    
};

enum {
    kDisplayColorRGBQuantizationRangeDefault,
    kDisplayColorRGBQuantizationRangeLimited,
    kDisplayColorRGBQuantizationRangeFull
};

struct video_color_data {
        u_int32_t           depth;
        u_int32_t           space; 
        u_int32_t           range; 
        u_int32_t           coefficient;
};

struct video_timing_axis {
        uint32_t            total;
        uint32_t            active;
        uint32_t            sync_width;
        uint32_t            back_porch;
        uint32_t            front_porch;
        int32_t             sync_rate;
        uint32_t            sync_polarity;        
};

struct video_timing_data {
        bool                                interlaced;
        struct video_timing_axis         axis[kDisplayAxisCount];    
};

struct video_link_data {
        bool                            mirror_mode;
        u_int32_t                       test_mode;
        struct video_color_data		color;
        struct video_timing_data	timing;
};


typedef enum {
	kDisplayInfoFrameTypeReserved              = 0x80,
	kDisplayInfoFrameTypeVendorSpecific        = 0x81,
	kDisplayInfoFrameTypeAVI                   = 0x82,
	kDisplayInfoFrameTypeProductDescription    = 0x83,
	kDisplayInfoFrameTypeAudio                 = 0x84,
	kDisplayInfoFrameTypeMPEG                  = 0x85,
	kDisplayInfoFrameTypeMin                   = kDisplayInfoFrameTypeReserved,
	kDisplayInfoFrameTypeMax                   = kDisplayInfoFrameTypeMPEG,
	kDisplayInfoFrameTypeCount                 = 1 + (kDisplayInfoFrameTypeMax - kDisplayInfoFrameTypeMin)
} display_infoframe_type;

struct display_infoframe {
	display_infoframe_type   type;
	uint8_t             version;
	uint8_t             length;
	uint8_t             checksum;
	uint8_t             data[28];
};

#endif
