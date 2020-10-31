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
#include <lib/paint.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/chipid.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/memmap.h>
#include <platform/soc/hwregbase.h>
#include <platform/soc/pmgr.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <target.h>
#include <platform/soc/display_color_manager_tables.h>

///////////////////////////////////////////////////////////
// CHESTNUT LDO assignment
//
//				LCM		SAGE		GRAPE
//
// DEV1				1		2		3
// N51-PROTO2			1		1		2
// N53-PROTO2/2A		2		1		3		
// DEV2/EVT1			2		1		3
//
///////////////////////////////////////////////////////////

#define N51_N53_DEV1_PROTO2_BOARD	(0xF)
#define N53_PROTO2A_BOARD		(0xE)
#define N51_N53_DEV2_BOARD		(0xE)
#define N51_EVT1_MAIN_BOARD		(0xE)
#define N51_EVT1_MESA_BOARD		(0xD)
#define N53_EVT1_MAIN_BOARD		N51_EVT1_MESA_BOARD
#define N51_N53_DEV3_BOARD		(0xD)		// XXX we got this wrong, should have matched EVT1C. 
#define N51_N53_EVT1A_BOARD		(0xC)
#define N51_N53_EVT1B_BOARD		(0xB)		// presumably
#define N51_N53_EVT1C_BOARD		(0xA)
#define N51_N53_EVT2_BOARD		(0x9)

static uint32_t iphone6_get_board_rev(void);

static bool	gpio_board_rev_valid;
static uint32_t	gpio_board_rev;

static uint32_t display_config;

MIB_CONSTANT(kMIBTargetBacklight0I2CBus,	kOIDTypeUInt32, 0);
MIB_CONSTANT(kMIBTargetBacklight0I2CAddress,	kOIDTypeUInt32, 0xC6);
MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);
MIB_CONSTANT(kMIBTargetDisplayCMEngammaTablePtr,	kOIDTypeStruct, (void *)linear_rgb_engamma_tables);
MIB_CONSTANT(kMIBTargetDisplayCMEngammaTableCount,	kOIDTypeUInt32, ARRAY_SIZE(linear_rgb_engamma_tables));
MIB_CONSTANT(kMIBTargetDisplayCMDegammaTablePtr,	kOIDTypeStruct, (void *)linear_rgb_degamma_tables);
MIB_CONSTANT(kMIBTargetDisplayCMDegammaTableCount,	kOIDTypeUInt32, ARRAY_SIZE(linear_rgb_degamma_tables));
MIB_CONSTANT(kMIBTargetDisplayCMMatrixTablePtr,		kOIDTypeStruct, (void *)linear_identity_matrix_tables);
MIB_CONSTANT(kMIBTargetDisplayCMMatrixTableCount,	kOIDTypeUInt32, ARRAY_SIZE(linear_identity_matrix_tables));

static uint32_t iphone6_get_board_rev(void)
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

static bool is_dev1_board()
{
	// DEV config: board-id == 1
	// DEV1 board-rev == 0xF
	if (((rPMGR_SCRATCH0 >> 16) & 0xf) == 1)
		 return (iphone6_get_board_rev() == N51_N53_DEV1_PROTO2_BOARD);

	return false;
}

static bool is_n51_proto2_board()
{
	// N51 board-id == 0
	// Proto2 board-rev == 0xF
	if (((rPMGR_SCRATCH0 >> 16) & 0xf) == 0)
		 return (iphone6_get_board_rev() == N51_N53_DEV1_PROTO2_BOARD);

	return false;
}

static bool is_n53_proto2_board()
{
	// N53 board-id == 2
	// Proto2a board-rev == 0xF
	if (((rPMGR_SCRATCH0 >> 16) & 0xf) == 2)
		 return (iphone6_get_board_rev() == N51_N53_DEV1_PROTO2_BOARD);

	return false;
}

static bool is_mesa_supported()
{
	// X162 INT line needs to be configured active high only on supported boards
	// Any DEV board and N51_EVT1_MESA_BOARD and all EVT2 boards and forward.

	// Return true if DEV board. 
	if ( (((rPMGR_SCRATCH0 >> 16) & 0xf) == 1) || (iphone6_get_board_rev() <= N51_EVT1_MESA_BOARD) ) 
		return true;
	else 
		return false;
}

void target_early_init(void)
{
}

void target_late_init(void)
{
}

void target_init(void)
{
#if WITH_HW_FLASH_NOR
	flash_nor_init(SPI_NOR0);
#endif
	if(is_mesa_supported()) {
		// Configure the X162 INT line to be active high.
		// Currently doing this for all DEV boards and EVT1 MESA board
		gpio_configure(GPIO_X162_INT, GPIO_CFG_IN);
		gpio_configure_pupdn(GPIO_X162_INT, GPIO_PDN);
	}
	if (iphone6_get_board_rev() == N51_N53_EVT1A_BOARD) {
		// <rdar://problem/12982626> Switch Tristar to Pulsing Con Detect mode for n5x and n48/n49
		iic_write(0, 0x35, (uint8_t[]){ 0x1e, 0x30 }, 2);
	}
	if ((iphone6_get_board_rev() <= N51_N53_EVT1A_BOARD) && (iphone6_get_board_rev() > N51_N53_EVT1C_BOARD)) {
		// LDO9 (PP2V8_CAM) to 2.85V <rdar://problem/12660594> (Amber >= A1 version)
		// NOTE: Boards with Amber A0 version is handled via power_init pmu_ldo_cold_setup_a0
		iic_write(0, 0xe8, (uint8_t[]){ 0x03, 0x48, 0x5A }, 3);
	}
	if (iphone6_get_board_rev() <= N51_N53_EVT1C_BOARD) {
		// <rdar://problem/13255750> Override pin config to support SOCHOT0 throttler circuit on N5x E1C+
		gpio_configure_pupdn(GPIO_SOCHOT0_IN, GPIO_NO_PUPDN);
		
		// <rdar://problem/13180654> N5x | Set LDO9 to 2.80V for power throttler circuit
		// LDO9_VSEL = 2.8V  (0.6V + 25mV*LDO9_VSEL)
		iic_write(0, 0xe8, (uint8_t[]){ 0x03, 0x48, 0x58 }, 3);
		
		// <rdar://problem/13766537> N5x EVT2 | Change LDO3 and LDO13 voltages for Amber OTP-AP and older
		// For OTP-AP and older, which is determined by PMU register 0x0001 containing a value 0x16 or lower
		// - We need to overwrite LDO3 with 0x60 (used to be 0x64)
		// - And we need to overwrite LDO13 with 0x64 (used to be 0x60)

		uint8_t addr[2] = {0x00,0x01};
		uint8_t data;
		if (iic_read(0, 0xe8, &addr, sizeof(addr), &data, sizeof(data), IIC_NORMAL) == 0) {
			if (data <= 0x16) {
				iic_write(0, 0xe8, (uint8_t[]){ 0x03, 0x18, 0x60 }, 3);
				iic_write(0, 0xe8, (uint8_t[]){ 0x03, 0x68, 0x64 }, 3);
			}
		}
	}
	else { // pre-EVT2 HW
		// Hack to support Mesa on older HW
		// // <rdar://problem/13503359> N5x EVT2 and beyond - increase drive strength of Mesa SPI_CLK to 8mA (x4), slow slew
		{
			uintptr_t mesa_spi_clk_gpio;
			uint32_t mesa_spi_clk_gpio_val;
		
			mesa_spi_clk_gpio = (uintptr_t)(GPIO_BASE_ADDR + 0x130);		// SPI2_SCLK		-> AP_TO_NAVAJO_SPI2_CLK
			mesa_spi_clk_gpio_val = *(volatile uint32_t *)mesa_spi_clk_gpio;
			mesa_spi_clk_gpio_val &= ~(3 << 10);					// reset drive strength
			mesa_spi_clk_gpio_val |= (1 << 10);					// set drive strength to X2
			*(volatile uint32_t *)mesa_spi_clk_gpio = mesa_spi_clk_gpio_val;
		}
	}
	
	// <rdar://problem/14181542> N5x AP_DEV: Enable GPIO5 for WLAN_CLK32K instead of CLK32K_OUT
	if (((rPMGR_SCRATCH0 >> 16) & 0xf) == 1) {
		iic_write(0, 0xe8, (uint8_t[]){ 0x04, 0x04, 0x49 }, 3);	// GPIO5 Out 32K, Push-Pull, VBUCK3
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
#endif // WITH_HW_POWER

	return !*cold_button_boot || platform_get_request_dfu1();
}

bool target_should_poweroff(bool at_boot)
{
	return platform_get_request_dfu1() && (!at_boot || !power_has_usb());
}

#if APPLICATION_IBOOT
void target_watchdog_tickle(void)
{
	uint32_t value = gpio_read(GPIO_WDOG_TICKLE);
	gpio_write(GPIO_WDOG_TICKLE, value ^ 1);
}
#endif // APPLICATION_IBOOT

void * target_get_display_configuration(void)
{
	display_config = 0x00000da2;
	return ((void *)(&display_config));
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-device", "asp_nand", 0);
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "n51", 0);
}

#endif // WITH_ENV

bool target_has_tristar2(void)
{
	return iphone6_get_board_rev() != N51_N53_DEV1_PROTO2_BOARD;
}

#if WITH_HW_CHESTNUT
uint8_t target_get_lcm_ldos(void)
{
	if (is_n51_proto2_board())
		return DISPLAY_PMU_LDO(0);

	return (DISPLAY_PMU_LDO(0) | DISPLAY_PMU_LDO(1));
}
#endif

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
#if WITH_HW_DISPLAY_PINOT
	DTNode		*disp0_node, *backlight_node;
#endif
	DTNode		*node;
	uint32_t	propSize;
	char		*propName, *propStr;
	void		*propData;

#if WITH_HW_DISPLAY_PINOT
	// Find the DISP0 (display-subsystem 0) node
	FindNode(0, "arm-io/disp0", &disp0_node);

	if (FindNode(0, "arm-io/mipi-dsim/lcd", &node)) {
		extern int pinot_update_device_tree(DTNode *pinot_node, DTNode *clcd_node, DTNode *backlight_node);

		FindNode(0, "backlight", &backlight_node);
		pinot_update_device_tree(node, disp0_node, backlight_node);

		// Fix up LCM LDO for dev1 and n51proto2
		if (is_dev1_board() || is_n51_proto2_board()) {
			propName = "function-lcd_ldo";
			if (FindProperty(node, &propName, &propData, &propSize))
				((uint8_t *)propData)[8] = 0;			// V01 -> PP5V7_LCM_AVDDH_CHESTNUT
		}
 	}
#endif

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

	// Update the als calibration data
	if (FindNode(0, "arm-io/i2c1/als", &node)) {
		propName = "alsCalibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('AlsC', propData, propSize);
		}
	}

	// Update the backlight calibration data
	if (FindNode(0, "backlight", &node)) {
		propName = "backlight-calibration";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('BLCl', propData, propSize);
	}

	// Update the compass calibration data
	if (FindNode(0, "arm-io/uart2/oscar/compass", &node)) {
		propName = "compass-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CPAS', propData, propSize);
		}
	}

	// Update the X162 calibration data
	if (FindNode(0, "arm-io/spi2/mesa", &node)) {
		propName = "calibration-blob";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('FSCl', propData, propSize);
		}
		propName = "modulation-ratio";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('NvMR', propData, propSize);
		}
	}
           
	// Update the gyro calibration data
	if (FindNode(0, "arm-io/uart2/oscar/gyro", &node)) {
		propName = "gyro-orientation";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('GRot', propData, propSize);
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
	if (FindNode(0, "arm-io/uart2/oscar/accelerometer", &node)) {
		propName = "accel-orientation";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ARot', propData, propSize);
		}
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
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('PxCl', propData, propSize);
	}

	// Update charger calibration data
	if (FindNode(0, "charger", &node)) {
		propName = "usb-input-limit-calibration";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('CBAT', propData, propSize);
	}

	// Update vibrator calibration data
	if (FindNode(0, "arm-io/i2c0/pmu/vib-pwm/vibrator", &node)) {
		uint32_t strong_vset;
		uint32_t weak_vset;

		propName = "reg";
		if (FindProperty(node, &propName, &propData, &propSize)) {
		    if (syscfgCopyDataForTag('VLSg', (void *)&strong_vset, sizeof(strong_vset)) > 0)
			((uint32_t *)propData)[0] = strong_vset;
		}
		propName = "intensity-config";
		if (FindProperty(node, &propName, &propData, &propSize)) {
		    if (syscfgCopyDataForTag('VLWk', (void *)&weak_vset, sizeof(strong_vset)) > 0)
			((uint32_t *)propData)[1] = (((uint64_t)weak_vset << 32) + (strong_vset-1)) / strong_vset;  // round up
		}
	}

	// Multi-touch support
	if (FindNode(0, "arm-io/spi1/multi-touch", &node)) {
		propName = "compatible";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			propStr = NULL;

			if (is_dev1_board())
				propStr = "multi-touch,n41";
			else if (is_n51_proto2_board())
				propStr = "multi-touch,n51-p251";

			if (propStr != NULL) {
				memset(propData, 0, propSize);
				strlcpy(propData, propStr, propSize);
			}
		}

		// Set Grape LDOs for proto2 board
		if (is_n51_proto2_board()) {
			propName = "function-power_ana";
			if (FindProperty(node, &propName, &propData, &propSize))
				((uint8_t *)propData)[8] = 0x01;		// V02 -> PP5V1_GRAPE_VDDH
		}
	}

	// Display-pmu Sage LDO
	if (is_dev1_board()) {
		if (FindNode(0, "arm-io/i2c0/display-pmu", &node)) {
			propName = "sage-ldo";
			if (FindProperty(node, &propName, &propData, &propSize))
				*(uint32_t *)propData = 0x01;			// V02 -> PP5V7_SAGE_AVDDH_CHESTNUT
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

	// Pre-E1C Camera AVDD LDO is coming from PMU LDO9
	if ((target_config_ap() && iphone6_get_board_rev() > N51_N53_EVT1C_BOARD) ||
	    (!target_config_ap() && iphone6_get_board_rev() > N51_N53_DEV3_BOARD)) {
		if (FindNode(0, "arm-io/isp", &node)) {
			propName = "function-cam_avdd_ldo";
			if (FindProperty(node, &propName, &propData, &propSize))
				propName[0] = '~';
		}
	}

#if WITH_HW_DISPLAY_PMU
	display_pmu_update_device_tree("arm-io/i2c0/display-pmu");
#endif

	return 0;
}

#endif // WITH_DEVICETREE

#if WITH_PAINT

// The default background is expected to to be black and the artwork is expected
// to be white. This arrangement will be inverted depending upon the cover glass
// color of the device.

// Sample DClr_override values for testing devices without DClr syscfg entries (enclosure/cover glass):
// gray/black:		setenv DClr_override 000200009B9899003C3B3B0000000000
// gold/white:		setenv DClr_override 00020000B3C5D400E3E4E10000000000
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
