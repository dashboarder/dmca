/*
 * Copyright (C) 2010, 2015 Apple Inc. All rights reserved.
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
#include <platform/int.h>
#include <platform/soc/hwisr.h>

#include <drivers/display.h>
#include <drivers/displayport/displayport.h>
#include <drivers/displayport.h>

#if defined(WITH_MENU) && WITH_MENU

static struct video_link_data dp_video_data;

int do_dp(int argc, struct cmd_arg *args)
{
        if (argc < 2) {
                puts("not enough arguments.\n");
                goto usage;
        }

        if (strcmp("start", args[1].str) == 0) {
		static struct display_timing timing_list[] =
		{
#if SUB_TARGET_J1 || SUB_TARGET_J2 || SUB_TARGET_J2A
			{ "ipad3",	0, 252200000, 264, 2048, 5, 150, 5, 1536, 9, 3, 1, 0, 0, 0, 0, 24, DISPLAY_TYPE_EDP, NULL },
#endif
#if WITH_HW_DISPLAY_DISPLAYPORT
			{ "720p",	0, 00000000, 243, 1280, 220, 110, 40, 720, 20, 5, 5, 0, 0, 0, 0, 24, DISPLAY_TYPE_DP, NULL },
#endif
		};
#if SUB_TARGET_J1 || SUB_TARGET_J2 || SUB_TARGET_J2A
		dp_t dp = {
			.mode	=		0x1,
			.type	=		0x1,
			.min_link_rate =	0x6,
			.max_link_rate =	0x6,
			.lanes =		0x4,
			.ssc =		0x0,
			.alpm =		0x0,
			.vrr_enable =		0x0,
			.vrr_on =		0x0,
		};
#elif WITH_HW_DISPLAY_DISPLAYPORT
		dp_t dp = {
			.mode	=		0x1,
			.type	=		0x1,
			.min_link_rate =	0x6,
			.max_link_rate =	0x6,
			.lanes =		0x2,
			.ssc =		0x0,
			.alpm =		0x0,
			.vrr_enable =		0x0,
		};
#else
		dp_t dp;
#endif
		timing_list[0].display_config = (void *)&dp;
		
                displayport_init(&dp);

                bzero(&dp_video_data, sizeof(struct video_link_data));

                dp_video_data.mirror_mode = false;
                dp_video_data.test_mode = 0;
                dp_video_data.color.depth = 8;
                dp_video_data.color.space = kDisplayColorSpacesRGB;
                dp_video_data.color.range = kDisplayColorDynamicRangeVESA;
                dp_video_data.color.coefficient = kDisplayColorCoefficientITU601;
                dp_video_data.timing.axis[0].total = timing_list[0].h_active + timing_list[0].h_back_porch + timing_list[0].h_front_porch + timing_list[0].h_pulse_width;
                dp_video_data.timing.axis[0].active = timing_list[0].h_active;
                dp_video_data.timing.axis[0].sync_width = timing_list[0].h_pulse_width;
                dp_video_data.timing.axis[0].back_porch = timing_list[0].h_back_porch;
                dp_video_data.timing.axis[0].front_porch = timing_list[0].h_front_porch;
                dp_video_data.timing.axis[0].sync_rate = 0;
                dp_video_data.timing.axis[0].sync_polarity = 0;
                dp_video_data.timing.axis[1].total = timing_list[0].v_active + timing_list[0].v_back_porch + timing_list[0].v_front_porch + timing_list[0].v_pulse_width;
                dp_video_data.timing.axis[1].active = timing_list[0].v_active;
                dp_video_data.timing.axis[1].sync_width = timing_list[0].v_pulse_width;
                dp_video_data.timing.axis[1].back_porch = timing_list[0].v_back_porch;
                dp_video_data.timing.axis[1].front_porch = timing_list[0].v_front_porch;
                dp_video_data.timing.axis[1].sync_rate = 60 << 16;
                dp_video_data.timing.axis[1].sync_polarity = 0;

        } else if (strcmp("stop", args[1].str) == 0) {
                displayport_quiesce();
        } else if (strcmp("video", args[1].str) == 0) {
                if (strcmp("logo", args[2].str) == 0) {
                        dp_video_data.mirror_mode = false;
                        dp_video_data.test_mode = 0;

                        dp_controller_start_video(&dp_video_data);
                }
                if (strcmp("bist", args[2].str) == 0) {

                        dp_video_data.mirror_mode = false;
                        dp_video_data.test_mode = 1;

                        dp_controller_start_video(&dp_video_data);
                }
                if (strcmp("mirror", args[2].str) == 0) {
                        dp_video_data.mirror_mode = true;
                        dp_video_data.test_mode = 0;

                        dp_controller_start_video(&dp_video_data);
                }
	} else if (strcmp("dpcd", args[1].str) == 0) {
		uint8_t data[16];
		int len = args[3].n;
		uint32_t intr = 0;

		if (len > 16)
			len = 16;
		
		printf("dpcd read %d bytes from  0x%x\n", len,  args[2].n);
#if WITH_HW_DISPLAY_DISPLAYPORT
		intr = INT_DPORT0;
#elif WITH_HW_DISPLAY_EDP
		intr = INT_EDP0;
#endif 
		//cuz AppleTV only defines display in iBoot but declares drivers in iBEC
		if (intr) mask_int(intr);
		dp_controller_read_bytes_dpcd(args[2].n, data, len);
		if (intr) unmask_int(intr);

		for (int i=0; i < len; i++) {
			printf("0x%02x ", data[i]);
		}
		printf("\n");
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
        printf("%s dpcd <addr> <len>. Len is max 16 bytes\n", args[0].str);
        
        return -1;                
}

MENU_COMMAND_DEVELOPMENT(dp, do_dp, "displayport stuff", NULL);

#endif
