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
#include <drivers/iic.h>
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

#define N56_AP_BOARD_ID			(0x4)	// AP board ID
#define N56_DEV_BOARD_ID		(0x5)	// DEV board ID

#define N61_AP_BOARD_ID			(0x6)	// AP board ID
#define N61_DEV_BOARD_ID		(0x7)	// DEV board ID

#define N56_DEV2_BOARD_REV		(0xD)
#define N61_DEV2_BOARD_REV		(0xD)

#define N56_DEV3_BOARD_REV		(0xB)
#define N61_DEV3_BOARD_REV		(0xB)

#define N56_PROTO2_BOARD_REV		(0xC)
#define N61_PROTO2_BOARD_REV		(0xC)

#define N56_EVT1_BOARD_REV		(0xB)
#define N61_EVT1_BOARD_REV		(0xB)

#define N56_EVT1_DOE_BOARD_REV		(0xA)
#define N61_EVT1_DOE_BOARD_REV		(0xA)

#define N56_EVT1_CARRIER_BOARD_REV	(0x9)
#define N61_EVT1_CARRIER_BOARD_REV	(0x9)

// Combined Board Id/Board Revision
// IMPORTANT:	IF YOU CHANGE THE ORDER OF THESE ENUMERATORS YOU MUST CHECK
//		FOR ANY > or < RELATIONSHIPS THAT MAY NEED TO ALSO CHANGE.
typedef enum {
	BOARD_TYPE_UNKNOWN	= 0,
//	BOARD_TYPE_N56_DEV1	= 1,	// Deprecated
//	BOARD_TYPE_N56_PROTO1	= 2,	// Deprecated
//	BOARD_TYPE_T133_PROTO1	= 3,	// Deprecated
//	BOARD_TYPE_N56_DEV2	= 4,	// Deprecated
//	BOARD_TYPE_N61_DEV2	= 5,	// Deprecated
//	BOARD_TYPE_N61_DEV1	= 6,	// Deprecated
	BOARD_TYPE_N61_PROTO2	= 7,
	BOARD_TYPE_N56_PROTO2	= 8,
	BOARD_TYPE_N61_EVT1	= 9,
	BOARD_TYPE_N56_EVT1	= 10,
	BOARD_TYPE_N61_DEV3	= 11,
	BOARD_TYPE_N56_DEV3	= 12,
} board_type;

// Display panel type
typedef enum {
	DISPLAY_PANEL_TYPE_UNKNOWN = 0,
	DISPLAY_PANEL_TYPE_D500,
	DISPLAY_PANEL_TYPE_D600,
	DISPLAY_PANEL_TYPE_D500_2LANE,
	// Metadata
	DISPLAY_PANEL_TYPE_COUNT
} display_panel_type;

#if SUB_TARGET_N56
#define	DEFAULT_DISPLAY_PANEL	DISPLAY_PANEL_TYPE_D600
#endif
#if SUB_TARGET_N61
#define	DEFAULT_DISPLAY_PANEL	DISPLAY_PANEL_TYPE_D500
#endif

static const char *display_panel_types[] = {
	"UNKNOWN",
	"D500",
	"D600",
	"D500_2LANE"
};

static board_type iphone7_get_board_type(void);
static uint32_t iphone7_get_board_rev(void);
static void iphone7_get_display_info(void);
uint32_t target_get_display_panel_type(void);

static bool	gpio_board_rev_valid;
static uint32_t	gpio_board_rev;
static display_panel_type  display_panel;
static bool	display_panel_missing;
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
static mipi_t display_panel_configurations[] = {
	{
		.lanes = 0,
		.esc_div = 0,
		.pll_n = 0,
		.pll_m = 0,
		.pll_p = 0,
		.hsfreq = 0,
		.target_phy_settings = { 0, {}},	//0 Mhz
		 
	},
	{
		.lanes = 3,
		.esc_div = 6,
		.pll_n = 0,
		.pll_m = 0,
		.pll_p = 0,
		.hsfreq = 0,
		.target_phy_settings = { 4, {{0x44, 1, {0x52}}, {0x30, 1, {0x3F}}, {0x20, 1, {0x45}}, {0x32, 1, {0x28}}}},	//843 Mhz
		 
	},
	{
		.lanes = 4,
		.esc_div = 8,
		.pll_n = 0,
		.pll_m = 0,
		.pll_p = 0,
		.hsfreq = 0,
		.target_phy_settings = { 4, {{0x44, 1, {0x36}}, {0x30, 1, {0x3F}}, {0x20, 1, {0x45}}, {0x32, 1, {0x28}}}},	//1200 Mhz
	},
	{
		.lanes = 2,
		.esc_div = 8,
		.pll_n = 0,
		.pll_m = 0,
		.pll_p = 0,
		.hsfreq = 0,
		.target_phy_settings = { 4, {{0x44, 1, {0x36}}, {0x30, 1, {0x3F}}, {0x20, 1, {0x45}}, {0x32, 1, {0x28}}}},	//1200 Mhz
	},
};
#endif

MIB_CONSTANT(kMIBTargetBacklight0I2CBus,	kOIDTypeUInt32, 0);
MIB_CONSTANT(kMIBTargetBacklight0I2CAddress,	kOIDTypeUInt32, 0xC6);
#if SUB_TARGET_N56
MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 3);
MIB_CONSTANT(kMIBTargetBacklight1I2CBus,	kOIDTypeUInt32, 1);
MIB_CONSTANT(kMIBTargetBacklight1I2CAddress,	kOIDTypeUInt32, 0xC6);
#else
MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
#endif
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

static uint32_t iphone7_get_board_rev(void)
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

static board_type iphone7_get_board_type(void)
{
	static board_type	board_type = BOARD_TYPE_UNKNOWN;

	if (board_type != BOARD_TYPE_UNKNOWN) {
		return board_type;
	}

	uint32_t board_id = platform_get_board_id();
	uint32_t board_rev = iphone7_get_board_rev();

	switch (board_id) {
#if SUB_TARGET_N56
	case N56_AP_BOARD_ID:
		switch (board_rev) {
		case N56_PROTO2_BOARD_REV:
			board_type = BOARD_TYPE_N56_PROTO2;
			return board_type;

		default:
			if (board_rev > N56_EVT1_BOARD_REV) {
				// Deprecated or unsupported board revision
				goto fail;
			}

		/* FALL THROUGH */

		case N56_EVT1_BOARD_REV:
			board_type = BOARD_TYPE_N56_EVT1;
			return board_type;
		}
#endif	// SUB_TARGET_N56
#if SUB_TARGET_N61
	case N61_AP_BOARD_ID: {
		switch (board_rev) {
		case N61_PROTO2_BOARD_REV:
			board_type = BOARD_TYPE_N61_PROTO2;
			return board_type;

		default:
			if (board_rev > N61_EVT1_BOARD_REV) {
				// Deprecated or unsupported board revision
				goto fail;
			}

		/* FALL THROUGH */

		case N61_EVT1_BOARD_REV:
			board_type = BOARD_TYPE_N61_EVT1;
			return board_type;
		}
	}
#endif	// SUB_TARGET_N61
#if SUB_TARGET_N56
	case N56_DEV_BOARD_ID: {
		switch (board_rev) {
		case N56_DEV3_BOARD_REV:
			board_type = BOARD_TYPE_N56_DEV3;
			return board_type;
		default:
			goto fail;
		}
	}
#endif	// SUB_TARGET_N56
#if SUB_TARGET_N61
	case N61_DEV_BOARD_ID: {
		switch (board_rev) {
		case N61_DEV3_BOARD_REV:
			board_type = BOARD_TYPE_N61_DEV3;
			return board_type;
		default:
			goto fail;
		}
	}
#endif	// SUB_TARGET_N61
	default:
		goto fail;
	}

fail:
	platform_not_supported();
	return BOARD_TYPE_UNKNOWN;
}


// The panel id pin mappings for development boards are described by the
// following id pin states:

//	ID1	ID0	Panel	Switch	Board	Status
//	----	----	-----	------	-----	-------------
//	Open	Open	D403	Sage2	N56/N61	Deprecated
//	PU	PD	D500	Sage2	N61	Deprecated
//	Open	PD	D500	Meson	N61	Valid for N61
//	PD	PU	D600	Sage2	N56	Deprecated
//	PD	Open	D600	Meson	N56	Valid for N56

static void iphone7_get_display_info(void)
{
// Display types for form-factor devices are determined by the board id.
	if (target_config_ap()) {
		display_panel = DEFAULT_DISPLAY_PANEL;
		return;
	}

// For development boards we have to sense which type of display daughter card
// is plugged in. However, for boards with the boot configuration set to NVMe
// we can't do this because one of the display sense pins serves double duty
// as the S3E reset pin (AP_TO_S3E_RESET_L). Therefore, for boards that boot
// from NVMe, force the display type to correspond to the board id.

	uint32_t	boot_config = platform_get_boot_config();
	if ((boot_config == BOOT_CONFIG_NVME0)
	 || (boot_config == BOOT_CONFIG_NVME0_TEST)) {
		display_panel = DEFAULT_DISPLAY_PANEL;
		return;
	}

// Display panel id sense pins
#define DISPLAY_TO_AP_ID0	GPIO( 1, 1)	// 9: ULPI_DATA[2] -> DISPLAY_TO_AP_ID0
#define DISPLAY_TO_AP_ID1	GPIO( 0, 6)	// 6: ULPI_DATA[4] -> DISPLAY_TO_AP_ID1

	const char *id0;
	const char *id1;

	gpio_configure(DISPLAY_TO_AP_ID0, GPIO_CFG_IN);
	gpio_configure(DISPLAY_TO_AP_ID1, GPIO_CFG_IN);

	gpio_configure_pupdn(DISPLAY_TO_AP_ID0, GPIO_PUP);
	gpio_configure_pupdn(DISPLAY_TO_AP_ID1, GPIO_PUP);

	display_panel_missing = false;

	spin(100); // Wait 100us

	if (!gpio_read(DISPLAY_TO_AP_ID1)) {
		// ID1 has an external pull down.
		id1 = "pd";

		if (!gpio_read(DISPLAY_TO_AP_ID0)) {
			// ID0 has an external pull down which is an
			// invalid configuration.
			id0 = "pd";
		} else {
			// ID0 could be externally pulled up or it could be
			// floating. Change the polarity of the internal pull.
			gpio_configure_pupdn(DISPLAY_TO_AP_ID0, GPIO_PDN);
			spin(100); // Wait 100us

			if (gpio_read(DISPLAY_TO_AP_ID0)) {
				// ID0 has an external pull up which is an
				// invalid configuration.
				id0 = "pu";
			} else {
				// ID0 is open.
				id0 = "open";
				display_panel  = DISPLAY_PANEL_TYPE_D600;
			}
		}
	} else {
		// ID1 could be externally pulled high or it could be
		// floating. Change the polarity of the internal pull.
		gpio_configure_pupdn(DISPLAY_TO_AP_ID1, GPIO_PDN);
		spin(100); // Wait 100us

		if (gpio_read(DISPLAY_TO_AP_ID1)) {
			// ID1 has an external pull up which is an
			// an invalid configuration.
			id1 = "pu";
			id0 = "any";
		} else {
			// ID1 is open.
			id1 = "open";

			if (!gpio_read(DISPLAY_TO_AP_ID0)) {
				// ID0 has an external pull down.
				id0 = "pd";
				display_panel  = DISPLAY_PANEL_TYPE_D500;
			} else {
				// ID0 could be externally pulled up or it could be
				// floating. Change the polarity of the internal pull.
				gpio_configure_pupdn(DISPLAY_TO_AP_ID0, GPIO_PDN);
				spin(100); // Wait 100us

				if (gpio_read(DISPLAY_TO_AP_ID0)) {
					// ID0 has an external pull up which is an
					// invalid configuration.
					id0 = "pu";
				} else {
					// ID0 is open.
					id0 = "open";

					// Both ID1 and ID0 are open. That means
					// no panel is installed or an unsupported
					// panel that doesn't have id sense pins
					// is installed. We can't tell which at
					// this point.
					display_panel_missing = true;
				}
			}
		}
	}

	dprintf(DEBUG_INFO, "%spanel id pin state: id1=%s, id0=%s\n",
		(display_panel !=DISPLAY_PANEL_TYPE_UNKNOWN) ? "" : "Unknown", id1, id0);

	ASSERT(display_panel < DISPLAY_PANEL_TYPE_COUNT);

	dprintf(DEBUG_INFO, "Display panel = %s\n", display_panel_types[display_panel]);

#if SUB_TARGET_N56
	ASSERT(display_panel_missing || (display_panel == DISPLAY_PANEL_TYPE_D600));
#endif
#if SUB_TARGET_N61
	ASSERT(display_panel_missing || (display_panel == DISPLAY_PANEL_TYPE_D500));
#endif

	// If no panel was found, assume the default panel for now.
	// If any panel id is actually read from the display, we'll panic
	// when updating the device tree.
	if (display_panel_missing) {
		display_panel = DEFAULT_DISPLAY_PANEL;
	}

	gpio_configure(DISPLAY_TO_AP_ID0, GPIO_CFG_DFLT);
	gpio_configure(DISPLAY_TO_AP_ID1, GPIO_CFG_DFLT);
}

int target_get_boot_battery_capacity(void) {
	
	if ( iphone7_get_board_rev() <= N56_EVT1_CARRIER_BOARD_REV ) {	// XXX: also N56 carrier, etc.
		int temp=0;
#if WITH_HW_GASGAUGE
		if ( gasgauge_read_temperature(&temp) != 0 ) temp=0; // defensive
#endif

		const uint32_t charge_current=power_get_available_charge_current();
		if ( temp>500 && charge_current>500 ) return 0;   // rely on SOC1 clear only

		return 50;  // @see rdar://16587240, rdar://18649980
	}

	return 50; // old limit
}

uint32_t target_get_display_panel_type(void)
{
	if (display_panel == DISPLAY_PANEL_TYPE_UNKNOWN) {
		iphone7_get_display_info();
	}
	return display_panel;
}

static void target_fixup_power(void)
{
#if WITH_HW_POWER
int pmu_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm);
int pmu_get_data(int dev, uint16_t reg, uint8_t *byte);

	int rc;
	uint8_t val;

	bool fix_ana_trim=false;
	rc = pmu_get_data(0, 0x1, &val);		// read PMU.OTP version
	if ( rc < 0 ) {
		dprintf(DEBUG_INFO, "PMU: cannot read OTP Version (%d)\n", rc);			
	} else if ( val < 0x0d ) {
		rc = pmu_get_data(0, 0x01d7, &val);	// read kD2186_BUCK6_ANA_TRIM4
		if ( rc<0 ) {
			dprintf(DEBUG_INFO, "PMU: cannot read kD2186_BUCK6_ANA_TRIM4 (%d)\n", rc);			
		} else {
			fix_ana_trim=true;
		}
	}

	rc=pmu_set_data(0, 0x7000, 0x1d, 0); 	// test mode
	if ( rc<0 ) {
		dprintf(DEBUG_INFO, "PMU: cannot enter test mode (%d)\n", rc);			
	} else {

	//  <rdar://problem/16076898> for N61 EVT, N56 Proto 2 and earlier		
		if ( iphone7_get_board_type() <= BOARD_TYPE_N61_EVT1 ) {
			rc=pmu_set_data(0, 0x0028, 0x23, 1);
			if ( rc<0 ) dprintf(DEBUG_INFO, "cannot change VDD_FAULT (%d)\n", rc);
		}

	//	<rdar://problem/16775541> Remove N56 EVT iBoot Overwrites for PMU 
	// to set high gain mode for kD2186_BUCK6_ANA_TRIM4 0x02 has to be OR'd in to preserve exiting bits
		if ( fix_ana_trim ) {
			rc=pmu_set_data(0, 0x01d7, ( val | 0x02 ), 1); // merge in high gain mode
			if ( rc<0 ) dprintf(DEBUG_INFO, "cannot change kD2186_BUCK6_ANA_TRIM4 (%d)\n", rc);
		}

		rc=pmu_set_data(0, 0x7000, 0x00, 0); 
		if ( rc<0 ) panic("PMU: cannot exit test mode (%d)", rc);
	}


	uint8_t data[2] = { 0xa, 0 };
    rc=iic_read(1, 0xea, data, 1, &data[1], 1, IIC_NORMAL);
    if ( rc<0 ) {
		dprintf(DEBUG_INFO, "tigris: cannot read VBUS_ILIM (%d)\n", rc);
    } else if ( data[1]!=0x23 ) {
    	data[1]=0x23;
    	
		rc=iic_write(1, 0xea, data, sizeof(data));
		if ( rc<0 ) dprintf(DEBUG_INFO, "tigris: cannot set VBUS_ILIM (%d)\n", rc);
    }
#endif	
}

void target_early_init(void)
{
	// Get the display panel type.
	iphone7_get_display_info();

	uint32_t panel_type_index;
	uint32_t target_mipi_p, target_mipi_m, target_mipi_s;
	//Horrible hack to support suspend to ram. Should remove when supporting only one display per target
	uint32_t pixel_clock = 0; 

	panel_type_index = target_get_display_panel_type();

	if (panel_type_index == DISPLAY_PANEL_TYPE_D500) {
		// D500 @ 843 Mhz
		target_mipi_p = 4;
		target_mipi_m = 281;
		target_mipi_s = 1;
		pixel_clock = 88890000;
	} else {
		// D600 @ 1200 MHz
		target_mipi_p = 2;
		target_mipi_m = 100;
		target_mipi_s = 0;
		pixel_clock = 200000000;
	}

#if PRODUCT_LLB || PRODUCT_IBSS
	clock_set_frequency(CLK_MIPI, 0, target_mipi_p, target_mipi_m, target_mipi_s, 0);
#endif
	//Remove when only 1 display panel is supported
	clock_set_frequency(CLK_VCLK0, 0, 0, 0, 0, pixel_clock);

// Fuse Rev { 0x4 or 0x5} then USB Operating condition 3.0V
// Fuse Rev is NOT {0x4} or NOT { 0x5} and CFG Bit 492=0 then USB Operating condition is 3.3V
// Fuse Rev is NOT {0x4} or NOT {0x5} and CFG Bit 492=1 then USB Operating condition is 3.0V
#if PRODUCT_LLB || PRODUCT_IBSS
	if ( chipid_get_chip_revision() == CHIP_REVISION_B0 ) {
		const uint32_t fuse=chipid_get_fuse_revision();

		// LDO1, kD2186_LDO1_VSEL=0x0308
		if ( fuse==0x4 || fuse==0x5 || (rCFG_FUSE15&(1<<12))!=0 ) {
			printf("*** setting PP33_USB t0 3.0V ***\n");
			power_set_ldo(1, 3000); // USB 3.0V, 
		} else {
			// <rdar://15932593> has set LDO1 to 3.225 i pmu_ldo_cold_setup
			printf("*** setting PP33_USB t0 3.225V ***\n");
		}
	}

	target_fixup_power();

#endif
#if APPLICATION_IBOOT
	// <rdar://15943528> revs of N61/N56 boards before EVT builds (ie. < BOARD_TYPE_N61_EVT) 
	// turn on LDO9 for rear and front camera 
	if (iphone7_get_board_type() < BOARD_TYPE_N61_EVT1)
		power_enable_ldo(9, true);	// LDO9, kD2186_ACTIVE4:(1<<1)
#endif

}

void target_late_init(void)
{
	clock_gate(CLK_UART7, 0);
}

void target_init(void)
{
#if WITH_HW_FLASH_NOR
	flash_nor_init(SPI_NOR0);
#endif
#if PRODUCT_IBSS || PRODUCT_LLB
	if (iphone7_get_board_type() < BOARD_TYPE_N61_EVT1) {
		static const uint32_t fixup_list[] = {
			GPIO_ID(51),	CFG_IN | SLOW_SLEW,		//  51 : GPIO[14] -> BB_TO_AP_GPS_SYNC
			GPIO_ID(66),	CFG_DISABLED | SLOW_SLEW,	//  66 : GPIO[22] -> BB_TO_AP_HSIC1_WAKE
			UINT32_MAX, 	UINT32_MAX
		};

		dprintf(DEBUG_INFO, "Fixing up GPIOs for pre-EVT devices\n");

		gpio_fixup_pinconfig(fixup_list);
	}
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

#if APPLICATION_IBOOT
void target_watchdog_tickle(void)
{
	uint32_t value = gpio_read(GPIO_WDOG_TICKLE);
	gpio_write(GPIO_WDOG_TICKLE, value ^ 1);
}
#endif // APPLICATION_IBOOT

void * target_get_display_configuration(void)
{
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
	
	return ((void *)(&(display_panel_configurations[target_get_display_panel_type()])));
#else
	return NULL;
#endif
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	uint32_t panel_type_index;

	// boot-device is set in platform's init.c
	env_set("display-color-space","RGB888", 0);
	panel_type_index = target_get_display_panel_type();
	env_set("display-timing", display_panel_types[panel_type_index], 0);
	env_set("adbe-tunables", display_panel_types[panel_type_index], 0);
	env_set("adfe-tunables", display_panel_types[panel_type_index], 0);
}

#endif

#if WITH_HW_CHESTNUT
uint8_t target_get_lcm_ldos(void)
{
	return (DISPLAY_PMU_LDO(0) | DISPLAY_PMU_LDO(1));
}
#endif

#if WITH_HW_LM3534
uint8_t target_lm3534_gpr(uint32_t ctlr)
{
	uint8_t	gpr = (1 << 1) | (1 << 0);		// (ramp disable) | (chip enable)

	switch (target_get_display_panel_type()) {
		case DISPLAY_PANEL_TYPE_D500:
			gpr |= (1 << 4);		// (ovp_select_21v)
			break;

		case DISPLAY_PANEL_TYPE_D600:
			if (ctlr == 1)
				gpr |= (1 << 4);	// (ovp_select_21v)
			break;

		case DISPLAY_PANEL_TYPE_D500_2LANE:
			gpr |= (1 << 4);		// (ovp_select_21v)
			break;

		default:
			break;
	}

	return gpr;
}
#endif

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
#if WITH_HW_DISPLAY_PINOT
	DTNode		*disp0_node, *backlight_node;
#endif
	board_type	board = iphone7_get_board_type();
	DTNode		*node;
	uint32_t	propSize;
	char		*propName;
	char		*propStr;
	void		*propData;
	const char	*nodeName;

#if WITH_HW_DISPLAY_PINOT
	// Find the DISP0 (display-subsystem 0) node
	FindNode(0, "arm-io/disp0", &disp0_node);

	if (FindNode(0, "arm-io/mipi-dsim/lcd", &node)) {
		extern int pinot_update_device_tree(DTNode *pinot_node, DTNode *clcd_node, DTNode *backlight_node);

		FindNode(0, "backlight", &backlight_node);
		pinot_update_device_tree(node, disp0_node, backlight_node);

		if (target_config_dev() && display_panel_missing) {
			// Get the panel id that was actually read (if any).
			propName = "lcd-panel-id";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				// If we really read a panel id then an
				// unsupported panel is plugged into the board.
				if (((u_int32_t *)propData)[0] != 0) {
					panic("Unsupported lcd-panel-id: 0x%08x",
					      ((u_int32_t *)propData)[0]);
				}
			}
		}
	}
#endif

#if WITH_HW_MIPI_DSIM
	// Find the mipi node
	if (FindNode(0, "arm-io/mipi-dsim", &node)) {
		extern int mipi_update_device_tree(DTNode *mipi_node);
		mipi_update_device_tree(node);
	}
#endif

	if (FindNode(0, "arm-io/spi2/multi-touch", &node)) {
		propName = "compatible";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			propStr = NULL;
			if (target_config_ap()) {
				// AP Boards are assigned a display
				switch (board) {
				case BOARD_TYPE_N61_PROTO2:
				case BOARD_TYPE_N61_EVT1:
					propStr = "multi-touch,n61";
					break;
				case BOARD_TYPE_N56_PROTO2:
				case BOARD_TYPE_N56_EVT1:
					propStr = "multi-touch,n56";
					break;
				default:
					panic("Unknown display for board type: %d", board);
					break;
				}
			} else {
				// Dev boards have to figure out the display type
				if (target_get_display_panel_type() == DISPLAY_PANEL_TYPE_D500) {
					// D500: N61
					propStr = "multi-touch,n61";
				} else {
					// D600: N56
					propStr = "multi-touch,n56";
				}
			}
			if (propStr != NULL) {
				memset(propData, 0, propSize);
				strlcpy(propData, propStr, propSize);
			}
		}
	}

	// <rdar://problem/15618064> Move mesa I2C to 12C1
	// <rdar://problem/15937046> N61/N56 Mesa EEPROM moved from i2c2 to i2c1
	if (board < BOARD_TYPE_N56_PROTO2) {
		// Mesa EEPROM is on I2C2 so delete arm-io/i2c1/mesa-eeprom.
		nodeName = "arm-io/i2c1/mesa-eeprom";
		if (FindNode(0, nodeName, &node)) {
			propName = "name";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				dprintf(DEBUG_INFO,"Mesa EEPROM on I2C2 - deleting DT node %s\n",
					nodeName);
				propName[0] = '~';
			} else {
				panic("%s has no 'name' property", nodeName);
			}
		} else {
			panic("Unable to find device tree node %s", nodeName);
		}
	} else {
		// Mesa EEPROM is on I2C1 so delete arm-io/i2c2/mesa-eeprom.
		nodeName = "arm-io/i2c2/mesa-eeprom";
		if (FindNode(0, nodeName, &node)) {
			propName = "name";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				dprintf(DEBUG_INFO,"Mesa EEPROM on I2C1 - deleting DT node %s\n",
					nodeName);
				propName[0] = '~';
			} else {
				panic("%s has no 'name' property", nodeName);
			}
		} else {
			panic("Unable to find device tree node %s", nodeName);
		}
	}

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

	// Update the codec and speaker nodes with acoustic transducer scale data
	{
		uint8_t atscData[20];

		if (syscfgCopyDataForTag('ATSc', atscData, sizeof(atscData)) > 0) {
			// Update codec
			if (FindNode(0, "arm-io/spi1/audio-codec", &node)) {
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
			if (FindNode(0, "arm-io/i2c1/audio-speaker", &node)) {
				propName = "at-scale";
				if (FindProperty(node, &propName, &propData, &propSize)) {
				    memcpy(propData, &atscData[16], sizeof(uint32_t));
				}
			}
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

		// Remove the properties for the second backlight controller
		// if we have anything other than a D600 display panel.
		if (target_get_display_panel_type() != DISPLAY_PANEL_TYPE_D600) {
			propName = "dual-backlight-controllers";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
			propName = "function-backlight_enable1";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
			propName = "function-backlight_enable1";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
		}
	}

	// Update the compass calibration data
	if (FindNode(0, "arm-io/uart8/oscar/compass", &node)) {
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
	if (FindNode(0, "arm-io/uart8/oscar/gyro", &node)) {
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
	if (FindNode(0, "arm-io/uart8/oscar/accelerometer", &node)) {
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
	if (FindNode(0, "arm-io/uart8/oscar/pressure", &node)) {
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

#if SUB_TARGET_N56
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

	// Update the baseband time sync GPIO.
	// This signal is on GPIO14 (pin 51) for pre-EVT devices.
	if (board < BOARD_TYPE_N61_EVT1) {
		if (FindNode(0, "baseband", &node)) {
			propName = "function-bb_ap_time_sync";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				if (propSize == (4 * sizeof(uint32_t))) {
					((uint32_t*)propData)[2] = 51;
				}
			}
		}
	}

	// WLAN Fiji B0 workarounds -- !!!FIXME!!! Remove this when Fiji B0 is deprecated
	// <rdar://problem/16594586> Remove Fiji B0 PCIe hacks
	if (FindNode(0, "arm-io/apcie/pci-bridge1/wlan", &node)) {
		if (chipid_get_chip_revision() > CHIP_REVISION_B0) {
			// Disable workarounds for Fiji B1 and later.
			propName = "acpie-l1ss-workaround";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
			propName = "pci-wake-l1pm-disable";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
		} else {
			// Fix up the hacks for Fiji B0 and earlier
			propName = "pci-l1pm-control";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
			propName = "pci-l1pm-control-b0";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[16] = '\0';
			}
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
