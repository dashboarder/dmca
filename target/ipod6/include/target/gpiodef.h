/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __TARGET_GPIODEF_H
#define __TARGET_GPIODEF_H

/* iPod6,x specific gpio -> pin mappings */

#include <platform/gpio.h>

/* BOARD_REV mapping */
#define BOARD_REV_DEV4				0x0e
#define BOARD_REV_PROTO2A			0x0e
#define BOARD_REV_PROTO2B			0x0d
#define BOARD_REV_PROTO2X_CARDINAL		0x0c
#define BOARD_REV_PROTO2B_ALT_CARBON 		0x0b
#define BOARD_REV_PROTO2D 			 0x0a
#define BOARD_REV_PROTO2F 			 0x09
#define BOARD_REV_EVT 				 0x08
#define BOARD_REV_EVT_ALT_CARBON		 0x07
#define BOARD_REV_DVTb_ALT_CARBON		 0x06
#define BOARD_REV_DVTb_CARDINAL			0x05
#define BOARD_REV_PVT_OPA2376			0x03

extern uint32_t ipod6_get_board_rev();

/* define target-specific gpios in a generic fashion here. */
#define GPIO_ALT_BOOST_ID	GPIOC(GPIO_AP,  5, 3)		// GPIO11
#define GPIO_SPU_TO_OPAL_CS_L	GPIOC(GPIO_SPU, 5, 1)		//  41 : SPU_SPI_CS_TRIG[10]	-> SCM_SPU_TO_OPAL_CS_L

/* which IICs to initialize */
#define IICS_MASK		(3)

#define TRISTAR_IIC_BUS         (0)

#define DISPLAY_PMU_IIC_BUS	(1)

/* Miscellaneous Pins */
#define	GPIO_PMU_LCD_RST	(11)
#define	PMU_LCD_PWR_VCI		(5)
#define	PMU_LCD_PWR_DVDD	(0xe)
#define PMU_LDO_OPAL		(0x8)

/* 
 * support of video or command mode
 */
extern bool product_target_is_display_in_video_mode();
extern uint32_t product_target_get_ulps_in_delay(void);
extern uint32_t product_target_get_ulps_end_delay(void);
extern uint32_t product_target_get_ulps_out_delay(void);
extern bool product_target_no_burst_mode();

#define TARGET_DISP_VIDEO_MODE		product_target_is_display_in_video_mode()
#define TARGET_DSI_ULPS_IN_DELAY	product_target_get_ulps_in_delay()
#define TARGET_DSI_ULPS_END_DELAY	product_target_get_ulps_end_delay()
#define TARGET_DSI_ULPS_OUT_DELAY	product_target_get_ulps_out_delay()
#define TARGET_NO_BURST_MODE		product_target_no_burst_mode()
#endif /* ! __TARGET_GPIODEF_H */
