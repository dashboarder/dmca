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

#define IPHONE5_DEV3_BOARD		(0xF)
#define IPHONE5_DEV5_PROTO0_BOARD	(0xE)
#define IPHONE5_DEV6_PROTO1_BOARD	(0xD)
#define IPHONE5_PROTO2A_BOARD		(0xC)
#define IPHONE5_PROTO2B_PROTO2C_BOARD	(0xB)
#define IPHONE5_DEV7_PROTO3_BOARD	(0xA)
#define IPHONE5_EVT1_BOARD		(0x9)
#define IPHONE5_DOE1_BOARD		(0x8)
#define IPHONE5_EVT3_BOARD		(0x6)

static u_int32_t iphone5_get_board_rev(void);
static u_int32_t display_config;

#define IPHONE5_GRAPE_ID_N94	(0x0)
#define IPHONE5_GRAPE_ID_DS401	(0x1)
#define IPHONE5_GRAPE_ID_DS402	(0x2)
#define IPHONE5_GRAPE_ID_PROTO0	(0x8)

static u_int32_t iphone5_get_grape_id(void);

MIB_CONSTANT(kMIBTargetBacklight0I2CBus,	kOIDTypeUInt32, 0);
MIB_CONSTANT(kMIBTargetBacklight0I2CAddress,	kOIDTypeUInt32, 0xC6);
MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

void target_early_init(void)
{
}

void target_late_init(void)
{
	if (iphone5_get_board_rev() > IPHONE5_DEV7_PROTO3_BOARD) {
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
	display_config =       0x00000964; 
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

	// Updte the speaker calibration data
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
	// XXX The I2C1 als node can be removed once we deprecate proto HW
	if (FindNode(0, "arm-io/i2c1/als_i2c1", &node)) {
		propName = "alsCalibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('AlsC', propData, propSize);
		}
		propName = "als-colorCfg";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ClrC', propData, propSize);
		}
	}
	if (FindNode(0, "arm-io/i2c2/als", &node)) {
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

	// Update prox calibration data
	if (FindNode(0, "arm-io/spi1/multi-touch", &node)) {
		propName = "prox-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('PxCl', propData, propSize);
		}
	}

	// HACKs for multi-touch support
	if (FindNode(0, "arm-io/spi1/multi-touch", &node)) {
		propName = "compatible";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			propStr = NULL;

			switch (iphone5_get_grape_id()) {
				case IPHONE5_GRAPE_ID_N94 :
					propStr = "multi-touch,n94";
					break;

				case IPHONE5_GRAPE_ID_PROTO0 :
					propStr = "multi-touch,n41-proto0";
					break;

				case IPHONE5_GRAPE_ID_DS401 :
					propStr = "multi-touch,n41-d401";
					break;

				default :
					break;
			}

			// Replace the data if an override was found
			if (propStr != NULL) {
				memset(propData, 0, propSize);
				strlcpy(propData, propStr, propSize);
			}
		}
	}

	// pre-EVT1 Grape clock config
	if (iphone5_get_board_rev() > IPHONE5_EVT1_BOARD) {
	    if (FindNode(0, "arm-io/spi1/multi-touch", &node)) {
		    propName = "function-clock_enable";
		    if (FindProperty(node, &propName, &propData, &propSize)) {
			propName[0] = '~';
		    }
		    propName = "function-clock_enable-pmu";
		    if (FindProperty(node, &propName, &propData, &propSize)) {
			propName[21] = '\0';
		    }
	    }

	    // configure PMU GPIO1 to be Grape clock (off until enabled)
	    // configure SoC PWM2 to be disabled (
	    power_gpio_configure(0, 0x11);
	    gpio_configure(GPIO(8, 5), GPIO_CFG_IN);
	}

	// pre-DOE1 and pre-DEV8 don't use I2C2
	// <rdar://problem/10941423> I2C2 Hardware configuration for N41&N42
	if (iphone5_get_board_rev() > IPHONE5_DOE1_BOARD) {
		if (FindNode(0, "arm-io/i2c2", &node)) {
			propName = "compatible";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				memset(propData, 0, propSize);
				strlcpy(propData, "none", propSize);
			}
		}
	}

	// pre-DOE1 vibrator and accelerometer config
	if (iphone5_get_board_rev() > IPHONE5_DOE1_BOARD) {
	    if (FindNode(0, "arm-io/i2c1/accelerometer", &node)) {
		    propName = "interrupts";
		    if (FindProperty(node, &propName, &propData, &propSize)) {
			((uint32_t *)propData)[0] = 0x44;
		    }
	    }
    
	    // add pull-down to TMR32_PWM1 (now accel int.)
	    gpio_configure_pupdn(GPIO(8, 4), GPIO_PDN);

	    // remove PWM vibrator control
	    if (FindNode(0, "arm-io/pwm/vibrator", &node)) {
		    propName = "reg";
		    if (FindProperty(node, &propName, &propData, &propSize)) {
			strlcpy(propName, "AAPL,ignore", kPropNameLength);
		    }
	    }
	    
	    // enable PMU vibrator control
	    if (FindNode(0, "arm-io/i2c0/pmu/vib-pwm", &node)) {
		    propName = "AAPL,ignore";
		    if (FindProperty(node, &propName, &propData, &propSize)) {
			propName[0] = '~';
		    }
	    }

	    // transform vibrator node back to PMU control
	    if (FindNode(0, "arm-io/i2c0/pmu/vib-pwm/vibrator", &node)) {
		    uint32_t strong_vset = 0x35;
		    uint32_t weak_vset = 0xFF;

		    syscfgCopyDataForTag('VLSg', (void *)&strong_vset, sizeof(strong_vset));
		    syscfgCopyDataForTag('VLWk', (void *)&weak_vset, sizeof(strong_vset));

		    propName = "reg";
		    if (FindProperty(node, &propName, &propData, &propSize)) {
			((uint32_t *)propData)[0] = strong_vset;
		    }
		    propName = "default-hz";
		    if (FindProperty(node, &propName, &propData, &propSize)) {
			propName[0] = '~';
		    }
		    propName = "duty-cycle";
		    if (FindProperty(node, &propName, &propData, &propSize)) {
			propName[0] = '~';
		    }
		    propName = "enable-ms";
		    if (FindProperty(node, &propName, &propData, &propSize)) {
			propName[0] = '~';
		    }
		    propName = "function-enable";
		    if (FindProperty(node, &propName, &propData, &propSize)) {
			propName[0] = '~';
		    }
		    propName = "intensity-config";
		    if (FindProperty(node, &propName, &propData, &propSize)) {
			((uint32_t *)propData)[0] = 'ampl';
			((uint32_t *)propData)[1] = (((uint64_t)weak_vset << 32) + (strong_vset-1)) / strong_vset;  // round up
		    }
	    }
	}

	// vibrator calibration (DOE1 and newer only)
	if (iphone5_get_board_rev() <= IPHONE5_DOE1_BOARD) {
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
	}

	// Hack for EVT2 and older
	if (iphone5_get_board_rev() <= IPHONE5_EVT3_BOARD) {
		// Update codec's mic 4 inversion
		if (FindNode(0, "arm-io/spi3/audio-codec", &node)) {
			propName = "mic4a-invpol";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
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

	return 0;
}

#endif

static bool		gpio_board_rev_valid;
static u_int32_t	gpio_board_rev;

static u_int32_t iphone5_get_board_rev(void)
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

static bool		gpio_grape_id_valid;
static u_int32_t	gpio_grape_id;

static u_int32_t iphone5_get_grape_id(void)
{
	if (!gpio_grape_id_valid) {
		if (target_config_ap()) {
			gpio_grape_id = IPHONE5_GRAPE_ID_DS402;
		} else {
			gpio_configure(GPIO_GRAPE_ID0, GPIO_CFG_IN);
			gpio_configure(GPIO_GRAPE_ID1, GPIO_CFG_IN);
			gpio_configure(GPIO_GRAPE_ID2, GPIO_CFG_IN);

			gpio_configure_pupdn(GPIO_GRAPE_ID0, GPIO_PDN);
			gpio_configure_pupdn(GPIO_GRAPE_ID1, GPIO_PDN);
			gpio_configure_pupdn(GPIO_GRAPE_ID2, GPIO_PDN);

			spin(100); // Wait 100us

			gpio_grape_id = 
				(gpio_read(GPIO_GRAPE_ID2) << 0) |
				(gpio_read(GPIO_GRAPE_ID1) << 1) |
				(gpio_read(GPIO_GRAPE_ID0) << 2);

			gpio_configure(GPIO_GRAPE_ID0, GPIO_CFG_DFLT);
			gpio_configure(GPIO_GRAPE_ID1, GPIO_CFG_DFLT);
			gpio_configure(GPIO_GRAPE_ID2, GPIO_CFG_DFLT);
		}
		gpio_grape_id_valid = true;
	}

	return gpio_grape_id;
}

#if WITH_PAINT

// N41 & N42 don't support the DClr syscfg key necessary for varying the iBoot
// logo and background color. Therefore we will synthesize a DClr from the
// the information contained in the ClrC syscfg key along with the knowledge
// than N41s and N42s only come in black or white.
//
// Note: The device enclosure colors are not currently used for anything and
//       the values below are taken from N51. If at some point the enclosure
//       color starts being using for something, then these values will need
//       to be set to the actual alues corresponding to the N41/N42 enclosures.

static const syscfg_DClr_t iphone5_DClr[] = {
	[clrcColorBlack] = {
		.minor_version		= 0,
		.major_version		= 2,
		.device_enclosure.rgb	= RGB(153, 152, 155),	// Value taken from n51
		.cover_glass.rgb	= RGB( 59,  59,  60)
	},
	[clrcColorWhite] = {
		.minor_version		= 0,
		.major_version		= 2,
		.device_enclosure.rgb	= RGB(215, 217, 216),	// Value taken from n51
		.cover_glass.rgb	= RGB(225, 228, 227)
	},
};

int target_dclr_from_clrc(uint8_t *buffer, size_t size)
{
	int			result;
	syscfg_ClrC_t		ClrC;

	if ((buffer == NULL) || (size != sizeof(syscfg_DClr_t))) {
		return -1;
	}

	// Look up the 'ClrC' tag from syscfg.
	result = syscfgCopyDataForTag('ClrC', (uint8_t *)&ClrC, sizeof(ClrC));
	if (result < 0) {
		dprintf(DEBUG_INFO,
			"DClr: ClrC syscfg entry not found -- color remapping disabled\n");
		return result;
	}

	if (ClrC.clrcColor < ARRAY_SIZE(iphone5_DClr)) {
		dprintf(DEBUG_INFO,
			"DClr: Synthesizing values from ClrC (%d)\n", ClrC.clrcColor);
		memcpy(buffer, &iphone5_DClr[ClrC.clrcColor], size);
		return size;
	} else {
		dprintf(DEBUG_INFO,
			"DClr: Unsupported ClrC value (%d) -- color remapping disabled\n",
			ClrC.clrcColor);
		return -1;
	}
}


// The default background is expected to to be black and the artwork is expected
// to be white. This arrangement will be inverted depending upon the cover glass
// color of the device.

// Sample DClr_override values for testing devices without DClr syscfg entries (enclosure/cover glass):
// gray/black:		setenv DClr_override 000200009B9899003C3B3B0000000000
// silver/white:	setenv DClr_override 00020000D8D9D700E3E4E10000000000

static color_policy_invert_t target_cover_glass_color_table[] = {
	{ RGB( 59,  59,  60), false },	// Black - black background, white logo
	{ RGB(225, 228, 227), true  },	// White - white background, black logo
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

#endif // WITH_PAINT
