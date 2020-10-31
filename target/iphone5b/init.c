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
#if WITH_HW_DISPLAY_PMU
#include <drivers/display_pmu.h>
#endif
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

// Board ID/Board Rev matrix:

// Board ID: N48 MLB: 1010, N49 MLB: 1110
//	Board Rev 1111: PreProto+Tristar1+Chestnut Combined Rail (Not supported by SW)
//	Board Rev 1101: Proto1+Tristar1+Chestnut Split Rail+d404 display panel
//	Board Rev 1100: Proto1+Tristar2+Chestnut Split Rail+d404 display panel
//	Board Rev 1011: EVT1+Tristar2+Chestnut Split Rail+d404 display panel+L81 audio
//	Board Rev 1010: EVT2+Tristar2+Chestnut Split Rail+d404 display panel+L81 audio
//	Board Rev 1001: DVT+Tristar2+Chestnut Split Rail+d404 display panel+L81 audio

// Board ID: N48 Dev: 1011, N49 DEV: 1111
//	Board Rev 1111: Dev1+Tristar1+Chestnut Combined Rail+d401 display panel
//	Board Rev 1101: Proto1+Tristar1+Chestnut Split Rail+d404 display panel
//	Board Rev 1100: Proto1+Tristar2+Chestnut Split Rail+d404 display panel
//	Board Rev 1011: EVT1+Tristar2+Chestnut Split Rail+d404 display panel+L81 audio
//	Board Rev 1010: EVT2+Tristar2+Chestnut Split Rail+d404 display panel+L81 audio
//	Board Rev 1001: DVT+Tristar2+Chestnut Split Rail+d404 display panel+L81 audio

#define N48_AP_BOARD_ID			(0xA)	// AP board ID
#define N48_DEV_BOARD_ID		(0xB)	// DEV board ID

#define N49_AP_BOARD_ID			(0xE)	// AP board ID
#define N49_DEV_BOARD_ID		(0xF)	// DEV board ID

#define DEV1_BOARD			(0xF)	// Dev1 + Tristar1 + Chestnut Combined Rail
#define DEV1A_BOARD			(0xE)	// Dev1 + Tristar2 + Chestnut Combined Rail
#define DEV1B_BOARD			(0xC)	// Dev1 + Tristar2 + Chestnut Split Rail
#define DEV1C_BOARD			(0xB)	// Dev1 + Tristar2 + Chestnut Split Rail + L81 audio
#define PROTO1A_BOARD			(0xD)	// Proto1 + Tristar1 + Chestnut Split Rail
#define PROTO1B_BOARD			(0xC)	// Proto1 + Tristar2 + Chestnut Split Rail
#define EVT1_BOARD			(0xB)	// EVT1 + Tristar2 + Chestnut Split Rail + L81 audio
#define EVT2_BOARD			(0xA)	// EVT2 + Tristar2 + Chestnut Split Rail + L81 audio
#define DVT_BOARD			(0x9)	// DVT + Tristar2 + Chestnut Split Rail + L81 audio

static u_int32_t iphone5b_get_board_rev(void);
static bool iphone5b_with_chestnut_split_rail(void);
static uint32_t display_config;

MIB_CONSTANT(kMIBTargetBacklight0I2CBus,	kOIDTypeUInt32, 0);
MIB_CONSTANT(kMIBTargetBacklight0I2CAddress,	kOIDTypeUInt32, 0xC6);
MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

// Don't panic for these targets because of <rdar://problem/14236421>.
MIB_CONSTANT(kMIBTargetPanicOnUnknownDclrVersion,	kOIDTypeBoolean, false);

void target_early_init(void)
{
}

void target_late_init(void)
{
	if (iphone5b_get_board_rev() > DEV1_BOARD) {
		platform_not_supported();
	}
}

void target_init(void)
{
#if WITH_HW_FLASH_NOR
	flash_nor_init(SPI_NOR0);
#endif

	// All AP and PMU GPIOS for the radios are set by pinconfig.h 
	// and powerconfig.h.  While the AP settings are compatible with wake,
	// the BB PMU will be in reset thanks to powerconfig.h providing
	// dedicated cold boot settings.  The AppleARMFunctions in the device
	// tree will take care of the rest for a cold boot.

	if (iphone5b_get_board_rev() == EVT1_BOARD) {
		// <rdar://problem/12982626> Switch Tristar to Pulsing Con Detect mode for n5x and n48/n49
		iic_write(0, 0x35, (uint8_t[]){ 0x1e, 0x30 }, 2);
	}
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
	display_config = 0x00000964;
	return ((void *)(&display_config));
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-device", "nand0", 0);
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "n41", 0);
}

#endif

bool target_has_tristar2(void)
{
	u_int32_t board_rev = iphone5b_get_board_rev();
	return board_rev != DEV1_BOARD && board_rev != PROTO1A_BOARD;
}

#if WITH_HW_CHESTNUT
uint8_t target_get_lcm_ldos(void)
{
//	N48 dev and ap boards have different display PMU rail topologes:

//	Split rail topology (ap boards with D404 display module)
//		LDO1: PP5V7_SAGE_AVDDH
//		LDO2: PP5V7_TO_LCD_AVDDH
//		LDO3: PP5V1_GRAPE_VDDH

//	Combined rail topology (dev boards with D401 display module)
//		LDO1: PP5V7_SAGE_AND_LCM_AVDDH
//		LDO2: PP5V1_GRAPE_VDDH
//		LDO3: Unused

	if (iphone5b_with_chestnut_split_rail()) {
		return (DISPLAY_PMU_LDO(0) | DISPLAY_PMU_LDO(1));
	}
	// The combined rail topology requires only LDO1 to be enabled.
	return DISPLAY_PMU_LDO(0);
}
#endif

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
#if WITH_HW_DISPLAY_PINOT
	DTNode		*clcd_node, *backlight_node;
#endif
	DTNode		*node;
	uint32_t	propSize;
	char		*propName, *propStr;
	void		*propData;

#if WITH_HW_DISPLAY_PINOT
	// Find the CLCD node
	FindNode(0, "arm-io/clcd", &clcd_node);

	if (FindNode(0, "arm-io/mipi-dsim/lcd", &node)) {
		extern int pinot_update_device_tree(DTNode *pinot_node, DTNode *clcd_node, DTNode *backlight_node);

		FindNode(0, "backlight", &backlight_node);
		pinot_update_device_tree(node, clcd_node, backlight_node);

		// Set Chestnut LDO based upon split/combined rail topology.
		propName = "function-lcd_ldo";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (iphone5b_with_chestnut_split_rail())
				((uint8_t *)propData)[8] = 1;	// V02 -> PP5V7_CHESTNUT_LDO2
			else
				((uint8_t *)propData)[8] = 0;	// V01 -> PP5V7_CHESTNUT_LDO1
		}
	}
#endif

	// Update the highland-park node with acoustic transducer scale data
	if (FindNode(0, "arm-io/highland-park", &node)) {
		propName = "at-scale";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ATSc', propData, propSize);
		}
	}

	// Update the codec and speaker nodes with acoustic transducer scale data
	{
		uint8_t atscData[20];
		
		if (syscfgCopyDataForTag('ATSc', atscData, sizeof(atscData)) > 0) {
			// Update codec
			if (FindNode(0, "arm-io/spi3/audio-codec", &node)) {
				propName = "at-scale-imic";
				if (FindProperty(node, &propName, &propData, &propSize)) {
				    memcpy(propData, &atscData[0], sizeof(uint32_t));
				}
				propName = "at-scale-smic";
				if (FindProperty(node, &propName, &propData, &propSize)) {
				    memcpy(propData, &atscData[4], sizeof(uint32_t));
				}
				propName = "at-scale-fmic";
				if (FindProperty(node, &propName, &propData, &propSize)) {
				    memcpy(propData, &atscData[8], sizeof(uint32_t));
				}
				propName = "at-scale-rcvr";
				if (FindProperty(node, &propName, &propData, &propSize)) {
				    memcpy(propData, &atscData[12], sizeof(uint32_t));
				}
			}
			// Update speaker
			if (FindNode(0, "arm-io/i2c0/audio-speaker", &node)) {
				propName = "at-scale";
				if (FindProperty(node, &propName, &propData, &propSize)) {
				    memcpy(propData, &atscData[16], sizeof(uint32_t));
				}
			}
		}
	}

	// Update the speaker calibration data
	if (FindNode(0, "arm-io/i2c0/audio-speaker", &node)) {
		propName = "speaker-rdc";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SRdc', propData, propSize);
		}
		propName = "speaker-config";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint8_t vpbr;
			if (syscfgCopyDataForTag('VPBR', &vpbr, sizeof(vpbr)) == 1) {
			    ((uint8_t *)propData)[12] = (((uint8_t *)propData)[12] & ~0x1f) | (vpbr & 0x1f);
			}
			uint8_t vbst;
			if (syscfgCopyDataForTag('VBST', &vbst, sizeof(vbst)) == 1) {
			    ((uint8_t *)propData)[16] = vbst;
			}
		}
	}

	switch ( iphone5b_get_board_rev() ) {
		case DEV1_BOARD:
		case DEV1A_BOARD:
		case PROTO1A_BOARD:
		case PROTO1B_BOARD:	// DEV1B_BOARD same as PROTO1B_BOARD
		{
			static const uint8_t l20cfg[]={0x90,0x00,0x01,0x90,0x62,0x03,0x12,0x1e,0x02,0x04,0x0A,0x05,0x6b,0x63,0x06,0x32,0x74,0x03,0x00,0x00 };

			if (FindNode(0, "arm-io/i2c0/audio-speaker", &node)) {
				propName = "compatible";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propData, "audio-control,cs35l20", kPropNameLength);
				}
				
				propName = "speaker-config";
				if (FindProperty(node, &propName, &propData, &propSize)) {
				 	memcpy(propData, (void*)l20cfg, sizeof(l20cfg));
				}

				propName = "l67_func-ext_active";
				if (FindProperty(node, &propName, &propData, &propSize)) {
				    	strlcpy(propName, "function-ext_active", kPropNameLength);
				}

				propName = "l67_func-ext_master";
				if (FindProperty(node, &propName, &propData, &propSize)) {
				    	strlcpy(propName, "function-ext_master", kPropNameLength);
				}
			}

			// MCLK provides 6MHz for L20 on I2S2_MCLK pin
			if (FindNode(0, "arm-io/i2s2/audio-bluetooth", &node)) {
				propName = "reg";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					((uint32_t*)propData)[2] = 0x005b8d80;
				}
			}

			if (FindNode(0, "arm-io/spi3/audio-codec", &node)) {
				propName = "compatible";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propData, "audio-control,cs42l67", propSize);
				}
				propName = "function-xsp_active";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "function-vsp_active", kPropNameLength);
				}
				propName = "function-xsp_master";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "function-vsp_master", kPropNameLength);
				}
				propName = "function-l67_xsp_active";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "function-xsp_active", kPropNameLength);
				}
				propName = "function-l67_xsp_master";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "function-xsp_master", kPropNameLength);
				}
				propName = "function-xsp_sidetone_active";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					propName[0] = '~';
				}
				propName = "l67_aout2-hac";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "aout2-hac", kPropNameLength);
				}
				propName = "mic3-intmic";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "ain1-intmic", kPropNameLength);
				}
				propName = "mic2-extmic";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "ain2-extmic", kPropNameLength);
				}
				propName = "mic1-intmic";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "ain5-intmic", kPropNameLength);
				}
				propName = "mic4-intmic";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "ain6-intmic", kPropNameLength);
				}
			}
			if (FindNode(0, "arm-io/i2s0/audio-codec", &node)) {
				propName = "compatible";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propData, "audio-data,cs42l67", propSize);
				}
				propName = "reg";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					((uint8_t *)propData)[0] = 0x11;
				}
				propName = "l67_func-clock_src_system";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "function-clock_src_system", kPropNameLength);
				}
				propName = "l67_func-clock_src_baseband";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "function-clock_src_baseband", kPropNameLength);
				}
			}
			if (FindNode(0, "arm-io/mca0/audio-codec-reference", &node)) {
				propName = "AAPL,ignore";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					propName[0] = '~';
				}
			}
			if (FindNode(0, "arm-io/mca0/audio-codec-sidetone", &node)) {
				propName = "compatible";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "AAPL,ignore", kPropNameLength);
				}
			}
			if (FindNode(0, "arm-io/mca1/audio-codec-voice", &node)) {
				propName = "reg";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					((uint8_t *)propData)[0] = 0x01;
				}
				propName = "l67_clock-sources";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "clock-sources", kPropNameLength);
				}
				propName = "l67_func-clock_src_route_i2sM";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "function-clock_src_route_i2sM", kPropNameLength);
				}
				propName = "l67_func-clock_src_route_bb2w";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					strlcpy(propName, "function-clock_src_route_bb2w", kPropNameLength);
				}
			}

		} break;
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

	// Update the compass calibration data
	if (FindNode(0, "arm-io/i2c1/compass1", &node)) {
		propName = "compass-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CPAS', propData, propSize);
		}
	}

	// Update the als calibration data
	if (FindNode(0, "arm-io/i2c2/als", &node)) {
		propName = "alsCalibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('AlsC', propData, propSize);
		}
	}

	// Update the gyro calibration data
	if (FindNode(0, "arm-io/i2c0/gyro", &node)) {
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
	if (FindNode(0, "arm-io/i2c1/accelerometer", &node)) {
		propName = "low-temp-accel-offset";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LTAO', propData, propSize);
		}
		propName = "accel-sensitivity-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ASCl', propData, propSize);
		}
		propName = "accel-interrupt-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('AICl', propData, propSize);
		}
	}

	// Update accelerometer calibration data
	if (FindNode(0, "arm-io/i2c1/accelerometer1", &node)) {
		propName = "low-temp-accel-offset";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LTAO', propData, propSize);
		}
		propName = "accel-sensitivity-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ASCl', propData, propSize);
		}
		propName = "accel-interrupt-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('AICl', propData, propSize);
		}
	}

	// Update prox calibration data
	if (FindNode(0, "arm-io/spi1/multi-touch", &node)) {
		// Set multi-touch compatibility based upon split/combined rail topology.
		propName = "compatible";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (iphone5b_with_chestnut_split_rail())
				propStr = "multi-touch,n48";
			else
				propStr = "multi-touch,n41";

			memset(propData, 0, propSize);
			strlcpy(propData, propStr, propSize);
		}

		// Set Chestnut LDO based upon split/combined rail topology.
		propName = "function-power_ana";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (iphone5b_with_chestnut_split_rail())
				((uint8_t *)propData)[8] = 2;	// V03 -> PP5V1_CHESTNUT_LDO3
			else
				((uint8_t *)propData)[8] = 1;	// V02 -> PP5V1_CHESTNUT_LDO2
		}

		propName = "prox-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('PxCl', propData, propSize);
		}
	}

	// vibrator calibration
	if (FindNode(0, "arm-io/pwm/vibrator", &node)) {
		propName = "duty-cycle";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint32_t pct;
			if (syscfgCopyDataForTag('VLSg', (void *)&pct, sizeof(pct)) > 0) {
				((uint32_t *)propData)[0] = ((uint64_t)pct << 32) / 100;
			}
		}
		propName = "intensity-config";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint32_t pct;
			if (syscfgCopyDataForTag('VLWk', (void *)&pct, sizeof(pct)) > 0) {
				((uint32_t *)propData)[1] = ((uint64_t)pct << 32) / 100;
			}
		}
	}

	// Update charger calibration data
	if (FindNode(0, "charger", &node)) {
		propName = "usb-input-limit-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CBAT', propData, propSize);
		}
	}

	// TriStar 1 on older systems
	if (!target_has_tristar2()) {
		if (FindNode(0, "arm-io/i2c0/tristar", &node)) {
			propName = "compatible";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				strlcpy(propData, "tristar,cbtl1608", kPropNameLength);
			}
		}
	}

#if WITH_HW_DISPLAY_PMU
	display_pmu_update_device_tree("arm-io/i2c0/display-pmu");
#endif

	return 0;
}

#endif

static bool		gpio_board_rev_valid;
static u_int32_t	gpio_board_rev;

static u_int32_t iphone5b_get_board_rev(void)
{

	if (!gpio_board_rev_valid) {
		gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV3, GPIO_CFG_IN);

		gpio_configure_pupdn(GPIO_BOARD_REV0, GPIO_PDN);
		gpio_configure_pupdn(GPIO_BOARD_REV1, GPIO_PDN);
		gpio_configure_pupdn(GPIO_BOARD_REV2, GPIO_PDN);
		gpio_configure_pupdn(GPIO_BOARD_REV3, GPIO_PDN);

		spin(100); // Wait 100us

		gpio_board_rev = 
			(gpio_read(GPIO_BOARD_REV3) << 3) |
			(gpio_read(GPIO_BOARD_REV2) << 2) |
			(gpio_read(GPIO_BOARD_REV1) << 1) |
			(gpio_read(GPIO_BOARD_REV0) << 0);

		gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_DFLT);
		gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_DFLT);
		gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_DFLT);
		gpio_configure(GPIO_BOARD_REV3, GPIO_CFG_DFLT);

		gpio_board_rev_valid = true;
	}

	return gpio_board_rev;
}

static bool iphone5b_with_chestnut_split_rail(void)
{
	if ((platform_get_board_id() == N48_DEV_BOARD_ID)
	 || (platform_get_board_id() == N49_DEV_BOARD_ID)) {
		u_int32_t board_rev = iphone5b_get_board_rev();
		if ((board_rev == DEV1_BOARD) || (board_rev == DEV1A_BOARD)) {
			return false;
		}
	}

	return true;
}

#if WITH_PAINT

// The default background is expected to to be black and the artwork is expected
// to be white. This arrangement will be inverted depending upon the cover glass
// color of the device.

// Sample DClr_override values for testing devices without DClr syscfg entries (enclosure/cover glass):
// white/black:		setenv DClr_override 00020000F7F4F5003C3B3B0000000000
// yellow/black:	setenv DClr_override 0002000089F1FA003C3B3B0000000000
// pink/black:		setenv DClr_override 000200007A76FE003C3B3B0000000000
// blue/black:		setenv DClr_override 00020000E0AB46003C3B3B0000000000
// green/black:		setenv DClr_override 0002000077E8A1003C3B3B0000000000

static color_policy_invert_t target_cover_glass_color_table[] = {
	{ RGB( 59,  59,  60), false },	// Black - black background, white logo
};

color_policy_t *target_color_map_init(enum colorspace cs, color_policy_t *color_policy)
{
	// Must have a color policy structure passed in.
	if (color_policy == NULL)
		goto fail;

	// We only support the RGB888 colorspace.
	if (cs != CS_RGB888)
		goto fail;

	color_policy->policy_type  = COLOR_MAP_POLICY_INVERT;
	color_policy->color_table  = (void *)target_cover_glass_color_table;
	color_policy->color_count  = ARRAY_SIZE(target_cover_glass_color_table);
	color_policy->map_color    = NULL;	// Use standard remapper

	return color_policy;

fail:
	return NULL;
}
#endif	// WITH_PAINT

