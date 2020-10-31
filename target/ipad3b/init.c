/*
 * Copyright (C) 2011-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/display.h>
#include <drivers/flash_nor.h>
#include <drivers/iic.h>
#include <drivers/power.h>
#include <lib/devicetree.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/memmap.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <target.h>
#include <target/powerconfig.h>

#define P101_PROTO0_BOARD		(0)
#define P101_PROTO2_BOARD		(1)
#define P101_EVT_BOARD			(2)

static u_int32_t ipad3b_get_board_rev(void);
static void amelia_init();

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  90);
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/displayport/displayport.h>
static dp_t dp;
#endif

void target_early_init(void)
{
}

void target_late_init(void)
{
	if (target_config_ap()) {
		// Deprecate Proto0/1
		// Since board revs are shared between dev boards and enclosures,
		// depreate only enclosures
		if (ipad3b_get_board_rev() < P101_PROTO2_BOARD)
			platform_not_supported();
	}
}

void target_init(void)
{
	// Cold-boot defaults for GPIOs where pinconfig.h differs,
	// and it can't wait for the AppleARMFunction creation.

	unsigned int i;

	// <rdar://problem/11580048> P102 use 18.5mA for IRATE setting, not OTP value of 21mA
	if (ipad3b_get_board_rev() <= P101_EVT_BOARD) {
		const uint8_t pmu_data[] = {
			0x23,0x03,		// bank 3
			0xd2,0x01,		// enter test mode
			0x23,0x01,		// bank 1
			0x89,0x21,		// WLED_CTRL4 16kHz, 18.5mA
			0x23,0x03,		// bank 3
			0xd2,0x00,		// exit test mode
			0x23,0x00		// bank 0
		};

		for (i = 0; i < sizeof(pmu_data); i += 2)
			iic_write(0, 0x78, &pmu_data[i], 2);
	}

	// lower VDD_UNDER to 2.9V
	{
		uint8_t pmu_data[] = {
			0x23,0x03,		// bank 3
			0xd2,0x01,		// enter test mode
			0x23,0x01,		// bank 1
			0xd5,0x00,		// TRIM/CONF73
			0x23,0x03,		// bank 3
			0xd2,0x00,		// exit test mode
			0x23,0x00		// bank 0
		};


		for (i = 0; i < sizeof(pmu_data); i += 2) {
		    	if (i == 6) {
				iic_read(0, 0x78, &pmu_data[i], 1, &pmu_data[i+1], 1, IIC_NORMAL);
				pmu_data[i+1] = (pmu_data[i+1] & ~7) | 2;	// VDD_FAULT = 2.9V
			}
			iic_write(0, 0x78, &pmu_data[i], 2);
		}
	}

#if WITH_HW_FLASH_NOR
	flash_nor_init(SPI_NOR0);
#endif
}

void target_quiesce_hardware(void)
{
}

void target_poweroff(void)
{
}

int target_bootprep(enum boot_target target)
{
	switch (target) {
		case BOOT_DARWIN:
		case BOOT_DARWIN_RESTORE:
		case BOOT_DIAGS:
			break;

		default:
			; // do nothing
	}

	return 0;
}

bool target_should_recover(void)
{
	return platform_get_request_dfu2() && power_has_usb();
}

bool target_should_poweron(bool *cold_button_boot)
{
#if WITH_HW_POWER
	if (power_get_boot_flag() == kPowerBootFlagColdButton) *cold_button_boot = true;
#else
	*cold_button_boot = false;
#endif

	return !*cold_button_boot || platform_get_request_dfu1();
}

bool target_should_poweroff(bool at_boot)
{
	return platform_get_request_dfu1() && (!at_boot || !power_has_usb());
}

#if APPLICATION_IBOOT
void target_watchdog_tickle(void)
{
	u_int32_t value = gpio_read(GPIO_WDOG_TICKLE);
	gpio_write(GPIO_WDOG_TICKLE, value ^ 1);
}
#endif // APPLICATION_IBOOT

void * target_get_display_configuration(void)
{
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
	dp.mode	=		0x1;
	dp.type	=		0x1;
	dp.min_link_rate =	0x6;
	dp.max_link_rate =	0x6;
	dp.lanes =		0x4;
	dp.ssc =		0x0;
	dp.alpm =		0x0;
	dp.vrr_enable =		0x0;
	dp.vrr_on =		0x0;
	return ((void *)(&dp));
#else
	return NULL;
#endif
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	// XXX follow up on 'display-timing' below setting below

	env_set("boot-device", "nand0", 0);
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "ipad3", 0);
}

#endif

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
	DTNode		*node;
#if WITH_HW_DISPLAY_EDP	
	DTNode		*clcd_node = NULL, *backlight_node = NULL, *lcd_node = NULL;
#endif	
	uint32_t	propSize, propCnt;
	char		*propName;
	void		*propData;
	
	// Update the DWI node with the actual voltages
	if (FindNode(0, "arm-io/dwi", &node)) {
		propName = "voltages-buck0";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			propCnt = propSize / sizeof(u_int32_t);
			platform_get_cpu_voltages(propCnt, propData);
			platform_convert_voltages(0, propCnt, propData);
		}
		propName = "voltages-buck2";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			propCnt = propSize / sizeof(u_int32_t);
			platform_get_soc_voltages(propCnt, propData);
			platform_convert_voltages(2, propCnt, propData);
		}
	}

#if WITH_HW_DISPLAY_EDP
        if (FindNode(0, DP_DTPATH, &node)) {
                extern int edp_update_device_tree(DTNode *edp_node, DTNode *lcd_node, DTNode *clcd_node, DTNode *backlight_node);
        	FindNode(0, "arm-io/clcd", &clcd_node);
        	FindNode(0, DP_DTPATH "/lcd", &lcd_node);
                FindNode(0, "backlight", &backlight_node);
                edp_update_device_tree(node, lcd_node, clcd_node, backlight_node);
        }
#endif

	// Update the bottom prox calibration data
	if (FindNode(0, "arm-io/i2c1/prox", &node)) {
		propName = "prox-calibration-blob";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CPCl', propData, propSize);
		}
	}
	
	// Update the backlight calibration data
	if (FindNode(0, "backlight", &node)) {
		propName = "backlight-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('BLCl', propData, propSize);
		}
	}

	// Update the compass calibration data
	if (FindNode(0, "arm-io/i2c1/compass", &node)) {
		propName = "compass-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CPAS', propData, propSize);
		}
	}

	// Update als node with calibration data
	if (FindNode(0, "arm-io/i2c2/als", &node)) {
		propName = "backlight-leakage-channel0";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ALC0', propData, propSize);
		}
		propName = "backlight-leakage-channel1";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ALC1', propData, propSize);
		}
		propName = "alsCalibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('AlsC', propData, propSize);
		}
		propName = "als-colorCfg";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ClrC', propData, propSize);
		}
	}

	// Update the gyro calibration data
	if (FindNode(0, "arm-io/i2c2/gyro", &node)) {
		propName = "low-temp-offset";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LTGO', propData, propSize);
		}
		propName = "high-temp-offset";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('HTGO', propData, propSize);
		}
		propName = "gyro-sensitivity-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GSCl', propData, propSize);
		}
		propName = "gyro-sens-matrix-inverse";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GSCi', propData, propSize);
		}
		propName = "gyro-temp-table";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GYTT', propData, propSize);
		}
		propName = "gyro-interrupt-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GICl', propData, propSize);
		}
	}

	// Update the gyro1 calibration data
	if (FindNode(0, "arm-io/i2c2/gyro1", &node)) {
		propName = "low-temp-offset";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LTGO', propData, propSize);
		}
		propName = "high-temp-offset";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('HTGO', propData, propSize);
		}
		propName = "gyro-sensitivity-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GSCl', propData, propSize);
		}
		propName = "gyro-sens-matrix-inverse";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GSCi', propData, propSize);
		}
		propName = "gyro-temp-table";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GYTT', propData, propSize);
		}
		propName = "gyro-interrupt-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GICl', propData, propSize);
		}
	}

	// Update accelerometer calibration data
	if (FindNode(0, "arm-io/i2c2/accelerometer", &node)) {
		propName = "low-temp-accel-offset";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LTAO', propData, propSize);
		}
		propName = "accel-sensitivity-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ASCl', propData, propSize);
		}
	}

	// Update the speaker calibration data
	if (FindNode(0, "arm-io/i2c0/audio-speaker0", &node)) {
		propName = "speaker-rdc";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SRdc', propData, propSize);
		}
		propName = "speaker-config";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint8_t vpbr[2];
			if (syscfgCopyDataForTag('VPBR', vpbr, sizeof(vpbr)) == 2) {
			    ((uint8_t *)propData)[12] = (((uint8_t *)propData)[12] & ~0x1f) | ((vpbr[0] - 6) & 0x1f);
			    ((uint8_t *)propData)[32] = (((uint8_t *)propData)[12] & ~0x1f) | ((vpbr[1] - 6) & 0x1f);
			}
			uint8_t vbst[2];
			if (syscfgCopyDataForTag('VBST', vbst, sizeof(vbst)) == 2) {
			    ((uint8_t *)propData)[16] = vbst[0];
			    ((uint8_t *)propData)[36] = vbst[1];
			}
		}
	}

	// Update camera calibration data
	if (FindNode(0, "arm-io/isp", &node)) {
		propName = "camera-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CmCl', propData, propSize);
		}
	}

	// Dev1 audio config
	if (target_config_dev()) {
		if (ipad3b_get_board_rev() < P101_PROTO2_BOARD) {
			// disable speaker nodes
			if (FindNode(0, "arm-io/i2c0/audio-speaker0", &node)) {
				propName = "reg";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "AAPL,ignore", kPropNameLength);
				}
			}
			if (FindNode(0, "arm-io/i2c0/audio-speaker1", &node)) {
				propName = "reg";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "AAPL,ignore", kPropNameLength);
				}
			}

			// restore built-in speaker
			if (FindNode(0, "arm-io/spi1/audio-codec", &node)) {
				propName = "function-spkr_mute_xxx";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "function-spkr_mute", kPropNameLength);
				}
				propName = "aout1-speaker_xxx";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "aout1-speaker", kPropNameLength);
				}
				propName = "aout2-speaker_xxx";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "aout2-speaker", kPropNameLength);
				}
			} 
		}
	}

	return 0;
}

#endif

static u_int32_t ipad3b_get_board_rev(void)
{
	u_int32_t gpio_board_rev;

	gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_IN);

	gpio_configure_pupdn(GPIO_BOARD_REV0, GPIO_PUP);
	gpio_configure_pupdn(GPIO_BOARD_REV1, GPIO_PUP);
	gpio_configure_pupdn(GPIO_BOARD_REV2, GPIO_PUP);

	spin(100); // Wait 100us

	gpio_board_rev = 
		(gpio_read(GPIO_BOARD_REV2) << 2) |
		(gpio_read(GPIO_BOARD_REV1) << 1) |
		(gpio_read(GPIO_BOARD_REV0) << 0);

	gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_DFLT);

	return (gpio_board_rev & 0x7);
}
