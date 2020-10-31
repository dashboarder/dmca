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


#include <debug.h>
#include <sys.h>
#include <sys/menu.h>
#include <sys/task.h>

#include <drivers/display.h>
#include <drivers/hdmi.h>

#if defined(WITH_MENU) && WITH_MENU

static struct video_link_data hdmi_video_data;

int do_hdmi(int argc, struct cmd_arg *args)
{
	static struct display_timing timing_list[] =
	{
#if WITH_HW_DISPLAY_HDMI
		{ "720p",	0, 74250000, 243, 1280, 220, 110, 40, 720, 20, 5, 5, 0, 0, 0, 0, 24, DISPLAY_TYPE_HDMI, NULL},
#endif
	};
        if (argc < 2) {
                puts("not enough arguments.\n");
                goto usage;
        }

        if (strcmp("start", args[1].str) == 0) {
		
		printf("MY PIXEL CLOCK %d\n", timing_list[0].pixel_clock);
                hdmi_init(&timing_list[0]);

                bzero(&hdmi_video_data, sizeof(struct video_link_data));

                hdmi_video_data.mirror_mode = false;
                hdmi_video_data.test_mode = 0;
                hdmi_video_data.color.depth = 8;
                hdmi_video_data.color.space = kDisplayColorSpacesRGB;
                hdmi_video_data.color.range = kDisplayColorDynamicRangeVESA;
                hdmi_video_data.color.coefficient = kDisplayColorCoefficientITU601;
                hdmi_video_data.timing.axis[0].total = timing_list[0].h_active + timing_list[0].h_back_porch + timing_list[0].h_front_porch + timing_list[0].h_pulse_width;
                hdmi_video_data.timing.axis[0].active = timing_list[0].h_active;
                hdmi_video_data.timing.axis[0].sync_width = timing_list[0].h_pulse_width;
                hdmi_video_data.timing.axis[0].back_porch = timing_list[0].h_back_porch;
                hdmi_video_data.timing.axis[0].front_porch = timing_list[0].h_front_porch;
                hdmi_video_data.timing.axis[0].sync_rate = 0;
                hdmi_video_data.timing.axis[0].sync_polarity = 0;
                hdmi_video_data.timing.axis[1].total = timing_list[0].v_active + timing_list[0].v_back_porch + timing_list[0].v_front_porch + timing_list[0].v_pulse_width;
                hdmi_video_data.timing.axis[1].active = timing_list[0].v_active;
                hdmi_video_data.timing.axis[1].sync_width = timing_list[0].v_pulse_width;
                hdmi_video_data.timing.axis[1].back_porch = timing_list[0].v_back_porch;
                hdmi_video_data.timing.axis[1].front_porch = timing_list[0].v_front_porch;
                hdmi_video_data.timing.axis[1].sync_rate = 60 << 16;
                hdmi_video_data.timing.axis[1].sync_polarity = 0;

        } else if (strcmp("stop", args[1].str) == 0) {
                hdmi_quiesce();
        } else if (strcmp("video", args[1].str) == 0) {
                if (strcmp("logo", args[2].str) == 0) {
                        hdmi_video_data.mirror_mode = false;
                        hdmi_video_data.test_mode = 0;

                        hdmi_controller_start_video(&hdmi_video_data, timing_list[0].pixel_clock);
                }
                if (strcmp("bist", args[2].str) == 0) {

                        hdmi_video_data.mirror_mode = false;
                        hdmi_video_data.test_mode = 1;

                        hdmi_controller_start_video(&hdmi_video_data, timing_list[0].pixel_clock);
                }
                if (strcmp("mirror", args[2].str) == 0) {
                        hdmi_video_data.mirror_mode = true;
                        hdmi_video_data.test_mode = 0;

                        hdmi_controller_start_video(&hdmi_video_data, timing_list[0].pixel_clock);
                }
        } else if (strcmp("readReg", args[1].str) == 0) {
                hdmi_read_reg(args[2].n);
        } else if (strcmp("writeReg", args[1].str) == 0) {
                hdmi_write_reg(args[2].n, args[3].n);
        } else {
                puts("unrecognized command.\n");
                goto usage;
        }

        return 0;

usage:
        printf("%s start\n", args[0].str);
        printf("%s video logo\n", args[0].str);
        printf("%s video bist\n", args[0].str);
        printf("%s video mirror\n", args[0].str);
        printf("%s stop\n", args[0].str);
        printf("%s readReg\n", args[0].str);
        printf("%s writeReg\n", args[0].str);
        
        return -1;                
}

MENU_COMMAND_DEVELOPMENT(hdmi, do_hdmi, "hdmi stuff", NULL);

#endif
