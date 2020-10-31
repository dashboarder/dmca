/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/apple/gpio.h>
#include <drivers/power.h>
#include <drivers/gasgauge.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <lib/paint.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/gpiodef.h>
#include <platform/soc/chipid.h>
#include <target.h>
#include <platform/soc/hwclocks.h>
#if WITH_HW_DISPLAY_PMU
#include <drivers/display_pmu.h>
#endif
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/mipi/mipi.h>
#endif

// Board rev encodings. The voltages are treated as active
// low (so 1.8V is a binary 1 and 0 V is a binary 0) so that
// the numbers count naturally
#define BOARD_REV_AP_PROTO1	0x0
#define BOARD_REV_AP_PROTO2	0x1
#define BOARD_REV_AP_EVT	0x2
#define BOARD_REV_AP_EVT_MD	0x3
#define BOARD_REV_AP_CARRIER1	0x4
#define BOARD_REV_AP_CARRIER2	0x5
#define BOARD_REV_AP_DVT_N66	0x6

#define BOARD_REV_DEV1		0x0
#define BOARD_REV_DEV2		0x1
#define BOARD_REV_DEV3		0x2

static uint32_t iphone8_get_board_rev(void);
static void iphone8_assert_display_type(void);

#if WITH_HW_LM3534
static void iphone8_get_backlight_i2c_address(uint32_t oid __unused, void *arg __unused, void *data);

MIB_CONSTANT(kMIBTargetBacklight0I2CBus,	kOIDTypeUInt32, 0);
MIB_FUNCTION(kMIBTargetBacklight0I2CAddress,	kOIDTypeUInt32, iphone8_get_backlight_i2c_address, NULL);
#if TARGET_DISPLAY_D620
MIB_CONSTANT(kMIBTargetBacklight1I2CBus,	kOIDTypeUInt32, 2);
MIB_FUNCTION(kMIBTargetBacklight1I2CAddress,	kOIDTypeUInt32, iphone8_get_backlight_i2c_address, NULL);
#endif
#endif

#if TARGET_DISPLAY_D620
MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 3);
#else
MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
#endif
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

/* <rdar://problem/20423824> Used to indicate to lm3534 driver to over-ride auto-frequency threshold */
MIB_CONSTANT(kMIBTargetBacklightAutoFreqThresh, kOIDTypeUInt32, 0xc8); // This will change threshold from 8mA to 6.592mA

static bool	gpio_board_rev_valid;
static uint32_t	gpio_board_rev;

#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
static mipi_t display_panel_configurations = {
#if TARGET_DISPLAY_D520
	.lanes = 2,
	.esc_div = 8,
	.pll_n = 0x0,
	.pll_m = 0x30,
	.pll_p = 0x0,
	.hsfreq = 0x1b,
	.target_phy_settings = { 4, {{0x44, 1, {0x36}}, {0x30, 1, {0x3F}}, {0x20, 1, {0x45}}, {0x32, 1, {0x28}}}},	//1200 Mhz
#elif TARGET_DISPLAY_D620
	.lanes = 4,
	.esc_div = 8,
	.pll_n = 0x0,
	.pll_m = 0x30,
	.pll_p = 0x0,
	.hsfreq = 0x1b,
	.target_phy_settings = { 4, {{0x44, 1, {0x36}}, {0x30, 1, {0x3F}}, {0x20, 1, {0x45}}, {0x32, 1, {0x28}}}},	//1200 Mhz
#endif
};
#endif

// Read the board rev GPIOs. Note that the encoding is done
// with active-low GPIOs, so we need to invert the values
// we read from the GPIOs
static uint32_t iphone8_get_board_rev(void)
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

		// Invert 
		gpio_board_rev ^= 0xf;

		gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_DFLT);
		gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_DFLT);
		gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_DFLT);
		gpio_configure(GPIO_BOARD_REV3, GPIO_CFG_DFLT);

		gpio_board_rev_valid = true;

#if WITH_ENV
		env_set_uint("board-rev", gpio_board_rev, 0);
#endif
	}

	return gpio_board_rev;
}

#define TRISTATE_PULLDN		(0)
#define TRISTATE_FLOAT		(1)
#define TRISTATE_PULLUP		(2)

// reads a gpio that could be pulled up, pulled down, or floating
static uint32_t iphone8_read_tristate(gpio_t gpio)
{
	uint32_t val1;
	uint32_t val2;

	gpio_configure(gpio, GPIO_CFG_IN);
	gpio_configure_pupdn(gpio, GPIO_PDN);

	spin(100); // Wait 100us

	val1 = gpio_read(gpio);

	gpio_configure_pupdn(gpio, GPIO_PUP);

	spin(100); // Wait 100us

	val2 = gpio_read(gpio);

	gpio_configure(gpio, GPIO_CFG_DFLT);

	if (val1 == 0 && val2 == 0)
		return TRISTATE_PULLDN; // pulled down
	else if (val1 == 0 && val2 == 1)
		return TRISTATE_FLOAT; // floating
	else if (val1 == 1 && val2 == 1)
		return TRISTATE_PULLUP; // pulled up
	else
		panic("unexpected result from gpio %u tristate read %u, %u", gpio, val1, val2);

}

// Display IDs are constructed with the formula (ID1 << 4 | ID0) where
// ID<x> is 0 for grounded, 1 for floating, and 2 for pulled up
// Both pins floating means no card is connected
#define IPHONE8_DISPLAY_ID_TOF_PROX		(1 << 8)

#define IPHONE8_DISPLAY_ID_EMPTY		(TRISTATE_FLOAT << 4  | TRISTATE_FLOAT << 0)

#define IPHONE8_DISPLAY_ID_N71_P1		(TRISTATE_PULLDN << 4 | TRISTATE_PULLDN << 0)
#define IPHONE8_DISPLAY_ID_N71_P1_MUON		(TRISTATE_PULLUP << 4 | TRISTATE_PULLDN << 0)
#define IPHONE8_DISPLAY_ID_N71			(TRISTATE_FLOAT << 4  | TRISTATE_PULLUP << 0)
#define IPHONE8_DISPLAY_ID_N71_TOF_PROX	(IPHONE8_DISPLAY_ID_TOF_PROX | IPHONE8_DISPLAY_ID_N71)

#define IPHONE8_DISPLAY_ID_N66_P1		(TRISTATE_PULLUP << 4 | TRISTATE_PULLUP << 0)
#define IPHONE8_DISPLAY_ID_N66			(TRISTATE_PULLUP << 4 | TRISTATE_FLOAT << 0)
#define IPHONE8_DISPLAY_ID_N66_TOF_PROX		(IPHONE8_DISPLAY_ID_TOF_PROX | IPHONE8_DISPLAY_ID_N66)

#if TARGET_DISPLAY_D520
#define IPHONE8_DISPLAY_ID_DEFAULT_P1		IPHONE8_DISPLAY_ID_N71_P1
#define IPHONE8_DISPLAY_ID_DEFAULT		IPHONE8_DISPLAY_ID_N71
#elif TARGET_DISPLAY_D620
#define IPHONE8_DISPLAY_ID_DEFAULT_P1		IPHONE8_DISPLAY_ID_N66_P1
#define IPHONE8_DISPLAY_ID_DEFAULT		IPHONE8_DISPLAY_ID_N66
#else
#error "Unknown display type"
#endif

static const char* iphone8_get_display_id_pull(uint32_t id)
{
	switch (id) {
		case TRISTATE_PULLDN:
			return "pd";
		case TRISTATE_FLOAT:
			return "op";
		case TRISTATE_PULLUP:
			return "pu";
		default:
			return "?";
	}
}

static uint32_t iphone8_get_display_type(void)
{
	static bool display_id_valid = false;
	static uint32_t display_id;
	uint32_t prox;
	uint32_t board_rev = iphone8_get_board_rev();
	uint32_t id0, id1;

	if (!display_id_valid) {
		// for now, all proto hardware will have D520/D620.
		if (target_config_ap()) {
			// Display types for form-factor devices are determined
			// by the board id and rev and the PROX_SELECT GPIO.
			if (board_rev < BOARD_REV_AP_PROTO2) {
				display_id = IPHONE8_DISPLAY_ID_DEFAULT_P1;

				dprintf(DEBUG_INFO, "Display ID: 0x%x\n", display_id);
			} else {
				prox = iphone8_read_tristate(GPIO_PROX_SELECT);
				// PROX_SELECT must be pulled up (analog prox) or
				// floating (Doppler prox). It cannot be pulled down.
				ASSERT(prox != TRISTATE_PULLDN);
				display_id = IPHONE8_DISPLAY_ID_DEFAULT;
				if (prox == TRISTATE_FLOAT) {
					display_id |= IPHONE8_DISPLAY_ID_TOF_PROX;
				}
				dprintf(DEBUG_INFO, "Display ID: 0x%x (prox=%s)\n", display_id,
					iphone8_get_display_id_pull(prox));
			}
		} else {
			id0 = iphone8_read_tristate(GPIO_DISPLAY_ID0);
			id1 = iphone8_read_tristate(GPIO_DISPLAY_ID1);
			display_id = (id1 << 4) | id0;

			dprintf(DEBUG_INFO, "Display ID: 0x%x (id1/id0=%s/%s)\n", display_id,
				iphone8_get_display_id_pull(id1),
				iphone8_get_display_id_pull(id0));

			switch (display_id) {
				// No display is always supported
				case IPHONE8_DISPLAY_ID_EMPTY:
					display_id = IPHONE8_DISPLAY_ID_DEFAULT;
					break;
#if TARGET_DISPLAY_D520
				// D500 and D520 displays are only supported on targets that support D520 (N71, basically)
				case IPHONE8_DISPLAY_ID_N71_P1:
				case IPHONE8_DISPLAY_ID_N71_P1_MUON:
				case IPHONE8_DISPLAY_ID_N71:
					break;
#elif TARGET_DISPLAY_D620
				// D600 and D620 displays are only supported on targets that support D620 (N66, basically)
				case IPHONE8_DISPLAY_ID_N66_P1:
				case IPHONE8_DISPLAY_ID_N66:
					break;
#endif
				default:
					panic("unsupported display ID 0x%x (id1/id0=%s/%s)",
						display_id,
						iphone8_get_display_id_pull(id1),
						iphone8_get_display_id_pull(id0));
			}
		}
		display_id_valid = true;
	}

	return display_id;
}

int target_get_boot_battery_capacity(void)
{
	int temp=0;
#if WITH_HW_GASGAUGE
	if ( gasgauge_read_temperature(&temp) != 0 ) temp=0; // defensive
#endif

	const uint32_t charge_current=power_get_available_charge_current();
	if ( temp>500 && charge_current>500 ) return 0;   // rely on SOC1 clear only

	return 50;  // @see rdar://16587240
}


int target_precharge_gg_flag_mask(void)
{
#if (PRODUCT_IBSS || PRODUCT_LLB)
	return kHDQRegFlagsMaskSOC1; // not available in iBSS or LLB (env is not there)
#else
	static int gg_flag_mask=0;

	if ( gg_flag_mask==0 ) {
		const char *boot_args = env_get("boot-args");
		if ( boot_args == NULL ) {
			// nothing to do...
		} else {

			char *arg_str= strstr(boot_args, "precharge-gg-flag-mask=");
			if ( arg_str != NULL ) {
				while ( *arg_str!='=' ) arg_str++;

				gg_flag_mask=strtoul( arg_str+1, NULL, 0 );
				if ( gg_flag_mask==0 ) gg_flag_mask=kHDQRegFlagsMaskSOC1;
			}

		}
	}

	return gg_flag_mask;
#endif
}

void target_early_init(void)
{
}

void target_late_init(void)
{
	iphone8_get_board_rev();

	if (target_config_ap() && iphone8_get_board_rev() < BOARD_REV_AP_PROTO2) {
		dprintf(DEBUG_INFO, "Proto1 is deprecated\n");
		platform_not_supported();
	}
	if (target_config_dev() && iphone8_get_board_rev() < BOARD_REV_DEV3) {
		dprintf(DEBUG_INFO, "Dev1 and Dev2 are deprecated\n");
		platform_not_supported();
	}

	power_set_gpio(GPIO_PMU_NAND_LOW_POWER_MODE, 1, 1);
}

bool iphone8_use_stockholm_gpio(void)
{
#if SUB_TARGET_N71
	if (target_config_ap() && iphone8_get_board_rev() < BOARD_REV_AP_PROTO2)
		return true;
#endif
	return false;
}

void target_init(void)
{
	uint32_t board_rev = iphone8_get_board_rev();
#if WITH_HW_FLASH_NOR
	flash_nor_init(SPI_NOR0);
#endif

	dprintf(DEBUG_INFO, "board rev 0x%x (raw 0x%x)\n", board_rev, board_rev ^ 0xf);
	iphone8_get_display_type();

	// All devboards and N71 Proto1 use a GPIO to enable Stockholm
#if (PRODUCT_IBSS || PRODUCT_LLB)
	bool use_stockholm_gpio = iphone8_use_stockholm_gpio();
	if (use_stockholm_gpio) {
		static const uint32_t fixup_list[] = {
			GPIO_ID(151),	CFG_IN | PULL_DOWN | SLOW_SLEW,	// 151 : GPIO[42] -> AP_TO_STOCKHOLM_EN
			UINT32_MAX, 	UINT32_MAX
		};

		dprintf(DEBUG_INFO, "Fixing up GPIOs for DEV/pre-PROTO2 devices\n");

		gpio_fixup_pinconfig(fixup_list);
	}
	if(target_config_dev() || iphone8_get_board_rev() < BOARD_REV_AP_EVT_MD) {
		/* <rdar://problem/20505384> GPIO 42 is OFF for pre-EVT-MD revisions */
		static const uint32_t fixup_list[] = {
			GPIO_ID(151), 	CFG_OUT_0 | SLOW_SLEW,	// 151 : GPIO[42] (controls new external Orb LDO, replacement of LDO8)
			UINT32_MAX, UINT32_MAX
		};
		dprintf(DEBUG_INFO, "Disabling LDO via GPIO42\n");

		gpio_fixup_pinconfig(fixup_list);
	} else {
		/* EVT-MD and Future will turn off LDO8 */
#if WITH_HW_POWER
int pmu_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm);

		// in addition we need to disable LDO8 now that no one is using it <rdar://problem/20217268> [N71/N66] turn off LDO8 for EVT-MD
		int rc=pmu_set_data(0, 0x0310, 0 , 1);
		if ( rc<0 ) dprintf(DEBUG_INFO, "cannot change kD2255_PWRONOFF_LDO8_EN (%d)\n", rc);

#endif
	}
#if SUB_TARGET_N66 || SUB_TARGET_N66M
	if(target_config_ap() && iphone8_get_board_rev() < BOARD_REV_AP_DVT_N66) {
		/* <rdar://problem/21977200> N66 IO Spreadsheet v27a (for N66 DVT and older only) */
		static const uint32_t fixup_list[] = {
			GPIO_ID(41), 	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,
			GPIO_ID(42), 	CFG_FUNC0 | PULL_DOWN | DRIVE_X4 | SLOW_SLEW,
			UINT32_MAX, UINT32_MAX
		};
		dprintf(DEBUG_INFO, "Applying SPI2 workaround from v27a\n");
		gpio_fixup_pinconfig(fixup_list);
	}
#endif
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

void * target_get_display_configuration(void)
{
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
	return ((void *)(&display_panel_configurations));
#else
	return NULL;
#endif
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	// boot-device is set in platform's init.c
	env_set("display-color-space","RGB888", 0);
#if TARGET_DISPLAY_D520
	env_set("display-timing", "D520", 0);
	env_set("adbe-tunables", "D520", 0);
	env_set("adfe-tunables", "D520", 0);
#elif TARGET_DISPLAY_D620
	env_set("display-timing", "D620", 0);
	env_set("adbe-tunables", "D620", 0);
	env_set("adfe-tunables", "D620", 0);
#endif
}

#endif

#if WITH_HW_CHESTNUT
uint8_t target_get_lcm_ldos(void)
{
	return (DISPLAY_PMU_LDO(0) | DISPLAY_PMU_LDO(1));
}
#endif

#if WITH_HW_LM3534

// Do we use the legacy LM3534 (Meson) backlight, or the POR LM3539 (Muon) one?
static bool iphone8_use_lm3534(void)
{
	if (target_config_dev()) {
		switch (iphone8_get_display_type()) {
			case IPHONE8_DISPLAY_ID_N66_P1:
			case IPHONE8_DISPLAY_ID_N71_P1:
				// LM3534
				return true;
			default:
				// LM3539
				return false;
		}
	} else {
		if (iphone8_get_board_rev() < BOARD_REV_AP_EVT) {
			return true;
		} else {
			return false;
		}
	}
}

static void iphone8_get_backlight_i2c_address(uint32_t oid __unused, void *arg __unused, void *data)
{
	uint32_t *resptr = (uint32_t *)data;

	if (iphone8_use_lm3534()) {
		*resptr = 0xc6;
	} else {
		*resptr = 0xc4;
	}
}


uint8_t target_lm3534_gpr(uint32_t ctlr)
{
	// auto mode, ramp disable, chip enable
	uint8_t	gpr = (1 << 4) | (1 << 1) | (1 << 0);

	return gpr;
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
	char		*propName;
	void		*propData;

	// Start with hacks for prototypes and devboard, POR stuff is below the hacks

	if (FindNode(0, "arm-io/uart3/stockholm", &node)) {
		propName = "function-enable";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			char		*propName_gpio;
			void		*propData_gpio;
			uint32_t	propSize_gpio;

			propName_gpio = "function-enable-gpio";
			if (FindProperty(node, &propName_gpio, &propData_gpio, &propSize_gpio)) {
				bool use_stockholm_gpio = iphone8_use_stockholm_gpio();

				if (use_stockholm_gpio) {
					// Fixup AP_TO_STOCKHOLM_EN for older boards.
					propName[0] = '~';
					propName_gpio[15] = 0;
				} else {
					propName_gpio[0] = '~';
				}
			}
		}
	}

	// GPIO[42] is an additional enable for the rear camera's AVDD LDO on some
	// configs of Proto2 and newer. On Proto1 it was the stockholm enable GPIO
	// <rdar://problem/19129172>
	//
	// Additionally, this GPIO is repurposed for a DOE (<rdar://problem/20234031>)
	// For this DOE, we remove this function from the camera as well,
	// and drive GPIO42 high at boot (see target_init())
	if (target_config_dev() || iphone8_get_board_rev() < BOARD_REV_AP_PROTO2) {
		if (FindNode(0, "arm-io/isp", &node)) {
			propName = "function-cam_avdd_gpio";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
		}
	}
	/* Based on <rdar://problem/20505384>, BOARD_REV_AP_PROTO2 needs to keep function-cam_avdd_gpio */
	if(target_config_ap() && (iphone8_get_board_rev() == BOARD_REV_AP_PROTO2 || iphone8_get_board_rev() == BOARD_REV_AP_EVT)) {
		if (FindNode(0, "arm-io/isp", &node)) {
			propName = "function-cam_avdd_gpio";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				// [0-7]: String
				// [8-11]: GPIO number
				// [12-15]: GPIO pin setting
				((uint8_t*)propData)[8] = 151;
			}
		}
	}

#if TARGET_DISPLAY_D520
	// This mess is dictated by <rdar://problem/18862199> Update Merge Personalities for N71
	// See also <rdar://problem/19147664> Implement compatible field updates for N66 doppler prox
	// See also XXX new radar here
	if (FindNode(0, "arm-io/spi2/multi-touch", &node)) {
		uint32_t display_type = iphone8_get_display_type();

		propName = "compatible";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (display_type == IPHONE8_DISPLAY_ID_N71_P1) {
				strlcpy(propData, "multi-touch,t162", propSize);
			} else if (display_type == IPHONE8_DISPLAY_ID_N71_P1_MUON) {
				strlcpy(propData, "multi-touch,t162", propSize);
			} else if (display_type == IPHONE8_DISPLAY_ID_N71) {
				strlcpy(propData, "multi-touch,n71", propSize);
			} else if (display_type == IPHONE8_DISPLAY_ID_N71_TOF_PROX) {
				strlcpy(propData, "multi-touch,n71,2", propSize);
			} else {
				panic("Unknown/unsupported display 0x%x", display_type);
			}
		}
	}
#endif
#if TARGET_DISPLAY_D620
	// <rdar://problem/19147664> Implement compatible field updates for N66 doppler prox
	if (FindNode(0, "arm-io/spi2/multi-touch", &node)) {
		uint32_t display_type = iphone8_get_display_type();

		propName = "compatible";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			if (display_type == IPHONE8_DISPLAY_ID_N66_P1) {
				strlcpy(propData, "multi-touch,n66-p1", propSize);
			} else if (display_type == IPHONE8_DISPLAY_ID_N66) {
				strlcpy(propData, "multi-touch,n66", propSize);
			} else if (display_type == IPHONE8_DISPLAY_ID_N66_TOF_PROX) {
				strlcpy(propData, "multi-touch,n66,2", propSize);
			} else {
				panic("Unknown/unsupported display 0x%x", display_type);
			}
		}
	}
#endif

	if (iphone8_use_lm3534()) {
		if (dt_find_node(0, "arm-io/i2c0/lm3539", &node)) {
			if (dt_has_prop(node, "reg-lm3534")) {
				dt_remove_prop(node, "reg");
				dt_remove_prop(node, "compatible");
				dt_rename_prop(node, "reg-lm3534", "reg");
				dt_rename_prop(node, "compatible-lm3534", "compatible");
				dt_set_prop_str(node, "name", "lm3534");
			}
		}
#if SUB_TARGET_N66 || SUB_TARGET_N66M
		if (dt_find_node(0, "arm-io/i2c1/lm3539-1", &node)) {
			if (dt_has_prop(node, "reg-lm3534")) {
				dt_remove_prop(node, "reg");
				dt_remove_prop(node, "compatible");
				dt_rename_prop(node, "reg-lm3534", "reg");
				dt_rename_prop(node, "compatible-lm3534", "compatible");
				dt_set_prop_str(node, "name", "lm3534-1");
			}
		}
		if (dt_find_node(0, "arm-io/i2c2/lm3539-1", &node)) {
			if (dt_has_prop(node, "reg-lm3534")) {
				dt_remove_prop(node, "reg");
				dt_remove_prop(node, "compatible");
				dt_rename_prop(node, "reg-lm3534", "reg");
				dt_rename_prop(node, "compatible-lm3534", "compatible");
				dt_set_prop_str(node, "name", "lm3534-1");
			}
		}
#endif
	}

	// POR stuff below

#if WITH_HW_DISPLAY_PINOT
	// Find the DISP0 (display-subsystem 0) node
	FindNode(0, "arm-io/disp0", &disp0_node);

	if (FindNode(0, "arm-io/mipi-dsim/lcd", &node)) {
		extern int pinot_update_device_tree(DTNode *pinot_node, DTNode *clcd_node, DTNode *backlight_node);

		FindNode(0, "backlight", &backlight_node);
		pinot_update_device_tree(node, disp0_node, backlight_node);
	}
#endif

#if WITH_HW_MIPI_DSIM
	// Find the mipi node
	if (FindNode(0, "arm-io/mipi-dsim", &node)) {
		extern int mipi_update_device_tree(DTNode *mipi_node);
		mipi_update_device_tree(node);
	}
#endif

	// Update the speaker calibration data
	if (FindNode(0, "arm-io/i2c1/audio-speaker", &node)) {
		propName = "speaker-rdc";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SRdc', propData, propSize);
		}
		propName = "speaker-calib";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SpCl', propData, propSize);
		}
		propName = "speaker-config";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			/*
			 VBCA Value: 0x00000009 0x0000000A 0x00000005 0x00000009 0x00000001
				<Spkr VBST> <Spkr VPBR> <Arc VBST> <Arc VPBR> <Version>
			*/
			uint8_t vbca[20];
			if (syscfgCopyDataForTag('VBCA', vbca, sizeof(vbca)) == sizeof(vbca)) {
				((uint8_t *)propData)[12] = (((uint8_t *)propData)[12] & ~0x1f) | ( (vbca[4]) & 0x1f); // Spkr VPBR
				((uint8_t *)propData)[16] =  vbca[0]; // Spkr VBST
			}
		}

		propName = "acoustic-trim-gains";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ATGa', propData, propSize);
		}
	}

	if (FindNode(0, "arm-io/i2c1/audio-actuator", &node)) {
		propName = "actuator-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('TCal', propData, propSize);
		}
		propName = "speaker-config";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			/*
			 VBCA Value: 0x00000009 0x0000000A 0x00000005 0x00000009 0x00000001
				<Spkr VBST> <Spkr VPBR> <Arc VBST> <Arc VPBR> <Version>
			*/
			uint8_t vbca[20];
			if (syscfgCopyDataForTag('VBCA', vbca, sizeof(vbca)) == sizeof(vbca)) {
				((uint8_t *)propData)[12] = (((uint8_t *)propData)[12] & ~0x1f) | (vbca[12] & 0x1f);   // Arc VPBR
				((uint8_t *)propData)[16] =  vbca[8];  // Arc VBST
			}
		}
	}

	// Update the codec with acoustic transducer scale data
	if (FindNode(0, "arm-io/spi1/audio-codec", &node)) {
		propName = "acoustic-trim-gains";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('ATGa', propData, propSize);
		}

		if (target_config_dev() || iphone8_get_board_rev() < BOARD_REV_AP_EVT) {
			// For DEV and pre-EVT AP, switch in the old device tree properties
			if (dt_remove_prop(node, "smic-mic")) {
				dt_rename_prop(node, "smic-mic-preevt", "smic-mic");
			}

			if (dt_remove_prop(node, "lmic-mic")) {
				dt_rename_prop(node, "lmic-mic-preevt", "lmic-mic");
			}

			if (dt_remove_prop(node, "smic-micbias")) {
				dt_rename_prop(node, "smic-micbias-preevt", "smic-micbias");
			}

			if (dt_remove_prop(node, "lmic-micbias")) {
				dt_rename_prop(node, "lmic-micbias-preevt", "lmic-micbias");
			}
		} else {
			// For EVT+ AP, delete pre-evt device tree properties
			dt_remove_prop(node, "smic-mic-preevt");
			dt_remove_prop(node, "lmic-mic-preevt");
			dt_remove_prop(node, "smic-micbias-preevt");
			dt_remove_prop(node, "lmic-micbias-preevt");
		}
	}

	// Update the als calibration data
	if (FindNode(0, "arm-io/i2c2/als", &node)) {
		propName = "alsCalibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LSCI', propData, propSize);
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
	if (FindNode(0, "arm-io/aop/iop-aop-nub/compass", &node)) {
		propName = "compass-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CPAS', propData, propSize);
		}
		propName = "compass-orientation";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CRot', propData, propSize);
		}
	}

	// Update the gyro calibration data
	if (FindNode(0, "arm-io/aop/iop-aop-nub/gyro", &node)) {
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
	if (FindNode(0, "arm-io/aop/iop-aop-nub/accel", &node)) {
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

	// Update pressure sensor calibration data
	if (FindNode(0, "arm-io/aop/iop-aop-nub/pressure", &node)) {
		propName = "pressure-offset-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SPPO', propData, propSize);
		}
	}

	// Update prox calibration data
	if (FindNode(0, "arm-io/spi2/multi-touch", &node)) {
		propName = "prox-calibration";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('PxCl', propData, propSize);
		propName = "orb-gap-cal";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('OrbG', propData, propSize);
		propName = "orb-accel-cal";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('OICo', propData, propSize);
		propName = "orb-force-cal";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('OFCl', propData, propSize);
		propName = "orb-dynamic-accel-cal";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('FDAC', propData, propSize);
	}
	
	// Stockholm calibration
	if (FindNode(0, "arm-io/uart3/stockholm", &node)) {
		propName = "calibration";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('NFCl', propData, propSize);
	}

	// Update the X162 calibration data
	if (FindNode(0, "arm-io/spi3/mesa", &node)) {
		propName = "calibration-blob";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('FSCl', propData, propSize);
		}
		propName = "modulation-ratio";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('NvMR', propData, propSize);
		}
	}

#if SUB_TARGET_N66 || SUB_TARGET_N66M
	// Update rear camera tilt and rotation data
	if (FindNode(0, "arm-io/isp", &node)) {
		propName = "back-camera-tilt-and-rotation";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('BCTR', propData, propSize);
		}
	}
#endif

	// Update cover glass type
	if (FindNode(0, "product", &node)) {
		propName = "cover-glass";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CGSp', propData, propSize);
		}
	}

	// Update charger calibration data
	if (FindNode(0, "charger", &node)) {
		propName = "usb-input-limit-calibration";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('CBAT', propData, propSize);
	}

	// Update Vibe calibration data
	if (FindNode(0, "arm-io/i2c1/vib-pwm", &node)) {
		propName = "calibration-data";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			const uint32_t count=syscfgCopyDataForTag('VbCl', propData, propSize);
			if ( count!=propSize ) *((uint32_t*)propData)=0x00a46d0d;
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
// gray/black:		setenv DClr_override 00020000B9B5B4003C3B3B0000000000
// gold/white:		setenv DClr_override 00020000B5CCE100E3E4E10000000000
// silver/white:	setenv DClr_override 00020000D8D9D700E3E4E10000000000

static color_policy_invert_t target_cover_glass_color_table[] = {
	{ RGB( 39,  39,  40), false },	// Blacker - black background, white logo
	{ RGB(228, 231, 232), true  },	// Whiter  - white background, black logo
	{ RGB( 59,  59,  60), false },	// Black   - black background, white logo
	{ RGB(225, 228, 227), true  },	// White   - white background, black logo
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

#if !WITH_HW_POWER

bool power_needs_precharge(void)
{
	return false;
}

void power_cancel_buttonwait(void)
{
}

bool power_do_chargetrap(void)
{
	return false;
}

bool power_is_suspended(void)
{
	return false;
}

void power_will_resume(void)
{
}

bool power_has_usb(void)
{
	return false;
}

int power_read_dock_id(unsigned *id)
{
	return -1;
}

bool power_get_diags_dock(void)
{
	return false;
}

uint32_t power_get_boot_battery_level(void)
{
	return 0;
}

int power_get_nvram(uint8_t key, uint8_t *data)
{
	return -1;
}

int power_set_nvram(uint8_t key, uint8_t data)
{
	return -1;
}

int power_set_soc_voltage(unsigned mv, int override)
{
	return -1;
}

bool force_usb_power = false;

void power_set_usb_state(bool configured, bool suspended)
{
}

int power_backlight_enable(uint32_t backlight_level)
{
	return 0;
}

#endif /* ! WITH_HW_POWER */
