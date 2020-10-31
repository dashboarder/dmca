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
#include <drivers/display.h>
#if WITH_HW_DISPLAY_PMU
#include <drivers/display_pmu.h>
#endif
#if WITH_HW_DOCKFIFO_UART || WITH_HW_DOCKFIFO_BULK
#include <drivers/dockfifo/dockfifo.h>
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
#include <platform/soc/chipid.h>
#include <platform/soc/pmgr.h>
#include <platform/dmin.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <target.h>
#include <target/cjay.h>

#define BOARD_TYPE_N27_AP		(0xA)
#define BOARD_TYPE_N28_AP		(0xC)
#define BOARD_TYPE_N27A_AP		(0x2)
#define BOARD_TYPE_N28A_AP		(0x4)

#define BOARD_TYPE_N27_DEV		(0xB)
#define BOARD_TYPE_N28_DEV		(0xD)
#define BOARD_TYPE_N27A_DEV		(0x3)
#define BOARD_TYPE_N28A_DEV		(0x5)

// Display panel type
typedef enum {
	DISPLAY_PANEL_TYPE_UNKNOWN = 0,
	DISPLAY_PANEL_TYPE_C1_DC,
	DISPLAY_PANEL_TYPE_SLTSID,
	DISPLAY_PANEL_TYPE_CRADLE,
	DISPLAY_PANEL_TYPE_ALT_DISP_POC1,
	DISPLAY_PANEL_TYPE_POR_DISP_S,
	DISPLAY_PANEL_TYPE_POR_DISP_B,
	DISPLAY_PANEL_TYPE_ALT_DISP_N31,
	DISPLAY_PANEL_TYPE_POR_DISP_B_VID,
	// Metadata
	DISPLAY_PANEL_TYPE_COUNT
} display_panel_type;

static const char *display_panel_types[] = {
	"UNKNOWN",
	"C1",
	"C1",
	"C1",
	"C1",
	"POR_DISP_S",
	"POR_DISP_B",
	"ALT_DISP_N31",
	"POR_DISP_B_VID"
};

typedef enum {
	DISPLAY_TOUCH_TYPE_UNKNOWN = 0,
	DISPLAY_TOUCH_TYPE_T126_SALTYSID,
	DISPLAY_TOUCH_TYPE_A1N27A_1_POC1,
	DISPLAY_TOUCH_TYPE_A1N27A_1_PROTO2,
	DISPLAY_TOUCH_TYPE_A1N28A_1_PROTO2,
	// Metadata
	DISPLAY_TOUCH_TYPE_COUNT
} display_touch_type;

static const char *display_touch_types[] = {
	"UNKNOWN",
	"multi-touch,t126-saltysid",
	"A1N27A,1-POC1",
	"A1N27A,1-Proto2",
	"A1N28A,1-Proto2"
};

// Display panel id sense pins
#define DISPLAY_TO_AP_ID0	GPIO( 17, 1)		// RMII_CRSDV
#define DISPLAY_TO_AP_ID1	GPIO( 16, 7)		// RMII_RXD_0
#define DISPLAY_TO_AP_ID2	GPIO( 17, 0)		// RMII_RXD_1
#define DISPLAY_TO_AP_ID3	GPIO( 16, 4)		// RMII_CLK

static bool	gpio_board_rev_valid;
static uint32_t	gpio_board_rev;
static bool	gpio_display_id_valid;
static display_panel_type	display_panel;
static display_touch_type	display_touch;

static uint32_t display_config;

uint32_t ipod6_get_board_rev(void)
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

bool ipod6_is_beryllium_supported()
{
	bool result = true;

	// On M7 DEV board configs, read ALT_BOOST_ID pin to check if Beryllium daughter card exists.
	// DEV config: board-id == 1
	if (((rPMGR_SCRATCH0 >> 16) & 0xf) == 1)
		result = gpio_read(GPIO_ALT_BOOST_ID);

	return result;
}

static void ipod6_power_display(bool enable)
{
#ifdef GPIO_PMU_LCD_PWR_EN
	power_set_gpio(GPIO_PMU_LCD_PWR_EN, 1, 1);
#endif
#ifdef PMU_LCD_PWR_DVDD
#ifdef PMU_LCD_PWR_VCI
		power_enable_ldo(PMU_LCD_PWR_DVDD, enable);
		power_enable_ldo(PMU_LCD_PWR_VCI, enable);
#endif // PMU_LCD_PWR_VCI
#endif // PMU_LCD_PWR_DVDD
#ifdef GPIO_LCD_PWR_EN
	gpio_write(GPIO_LCD_PWR_EN, enable);
#endif
}

// The panel id pin mappings are desribed by the following id pin states:

//	ID0     ID1     ID2	 ID3	Panel		TOUCH
//	----	----	----	----	-----		------
//	PD      PU      PU      X	C1_DC 
//	PU      PD      PD      X	SLTSID 
//	PU      PD      PU      X	CRADLE 
//	PU      PU      PD      PD      ALT_DISP_POC1       
//	PU      PU      PD      PU      POR_DISP_S 
//	PU      PU      PU      PD      POR_DISP_B 
//	PU      PU      PU      PU      ALT_DISP_N31       
//	PD      PU      PU      PD	POR_DISP_B_VID 

static void ipod6_get_display_info(void)
{
	const char *id0;
	const char *id1;
	const char *id2;
	const char *id3;

	if (target_config_ap()) {
		switch (platform_get_board_id()) {
		case BOARD_TYPE_N27_AP:
		case BOARD_TYPE_N27A_AP:
			display_panel  = DISPLAY_PANEL_TYPE_POR_DISP_S;
			display_touch = DISPLAY_TOUCH_TYPE_A1N27A_1_PROTO2;
			return;
		case BOARD_TYPE_N28_AP:
		case BOARD_TYPE_N28A_AP:
			// Special case Cardinal BOARD_REV's display.
			if ((ipod6_get_board_rev() == BOARD_REV_PROTO2X_CARDINAL) ||
			    (ipod6_get_board_rev() == BOARD_REV_DVTb_CARDINAL))
				display_panel = DISPLAY_PANEL_TYPE_POR_DISP_B_VID;
			else		
				display_panel  = DISPLAY_PANEL_TYPE_POR_DISP_B;

			display_touch = DISPLAY_TOUCH_TYPE_A1N28A_1_PROTO2;
			return;
		}
	}

	gpio_configure(DISPLAY_TO_AP_ID0, GPIO_CFG_IN);
	gpio_configure(DISPLAY_TO_AP_ID1, GPIO_CFG_IN);
	gpio_configure(DISPLAY_TO_AP_ID2, GPIO_CFG_IN);
	gpio_configure(DISPLAY_TO_AP_ID3, GPIO_CFG_IN);

	gpio_configure_pupdn(DISPLAY_TO_AP_ID0, GPIO_PDN);
	gpio_configure_pupdn(DISPLAY_TO_AP_ID1, GPIO_PDN);
	gpio_configure_pupdn(DISPLAY_TO_AP_ID2, GPIO_PDN);
	gpio_configure_pupdn(DISPLAY_TO_AP_ID3, GPIO_PDN);

	spin(100); // Wait 100us

	if (!gpio_read(DISPLAY_TO_AP_ID0)) {
		id0 = "pd";
		if (gpio_read(DISPLAY_TO_AP_ID1)) {
			id1 = "pu";
			if (gpio_read(DISPLAY_TO_AP_ID2)) {
				id2 = "pu";
				if (!gpio_read(DISPLAY_TO_AP_ID3)) {
					id3 = "pd";
					display_panel = DISPLAY_PANEL_TYPE_POR_DISP_B_VID;
					display_touch = DISPLAY_TOUCH_TYPE_A1N28A_1_PROTO2;
					gpio_display_id_valid = true;
				} else {
					id3 = "pu";
				}
			} else  {
				id2 = "pd";
				id3 = "not read";
			}
		} else {
			id1 = "pu";
			id2 = "not read";
			id3 = "not read";
			display_panel = DISPLAY_PANEL_TYPE_C1_DC;
			display_touch = DISPLAY_TOUCH_TYPE_UNKNOWN;
			gpio_display_id_valid = true;
		}
	} else {
		id0 = "pu";

		if (!gpio_read(DISPLAY_TO_AP_ID1)) {
			id1 = "pd";
			id3 = "not read";
			if (!gpio_read(DISPLAY_TO_AP_ID2)) {
				id2 = "pd";
				display_panel  = DISPLAY_PANEL_TYPE_SLTSID;
				display_touch = DISPLAY_TOUCH_TYPE_T126_SALTYSID;
				gpio_display_id_valid = true;
			} else {
				id2 = "pu";
				display_panel  = DISPLAY_PANEL_TYPE_CRADLE;
				display_touch = DISPLAY_TOUCH_TYPE_T126_SALTYSID;
				gpio_display_id_valid = true;
			}
		} else {
			id1 = "pu";
			if (!gpio_read(DISPLAY_TO_AP_ID2)) {
				id2 = "pd";

				if (!gpio_read(DISPLAY_TO_AP_ID3)) {
					id3 = "pd";
					display_panel  = DISPLAY_PANEL_TYPE_ALT_DISP_POC1;
					display_touch = DISPLAY_TOUCH_TYPE_A1N27A_1_POC1;
					gpio_display_id_valid = true;
				} else {
					id3 = "pu";
					display_panel  = DISPLAY_PANEL_TYPE_POR_DISP_S;
					display_touch = DISPLAY_TOUCH_TYPE_A1N27A_1_PROTO2;
					gpio_display_id_valid = true;
				}
			} else {
				id2 = "pu";
				if (!gpio_read(DISPLAY_TO_AP_ID3)) {
					id3 = "pd";
					display_panel  = DISPLAY_PANEL_TYPE_POR_DISP_B;
					display_touch = DISPLAY_TOUCH_TYPE_A1N28A_1_PROTO2;
					gpio_display_id_valid = true;
				} else {
					id3 = "pu";
					display_panel  = DISPLAY_PANEL_TYPE_ALT_DISP_N31;
					display_touch = DISPLAY_TOUCH_TYPE_UNKNOWN;
					gpio_display_id_valid = true;
				}
			}
		} 
	}

	dprintf(DEBUG_INFO, "%s panel id pin state: id0=%s, id1=%s, id2=%s, id3=%s\n",
		gpio_display_id_valid ? "" : "Unknown", id0, id1, id2, id3);

	ASSERT(display_panel < DISPLAY_PANEL_TYPE_COUNT);
	ASSERT(display_touch < DISPLAY_TOUCH_TYPE_COUNT);

	dprintf(DEBUG_INFO, "Display panel = %s\nDisplay switch = %s\n",
		display_panel_types[display_panel],
		display_touch_types[display_touch]);

	gpio_configure(DISPLAY_TO_AP_ID0, GPIO_CFG_DFLT);
	gpio_configure(DISPLAY_TO_AP_ID1, GPIO_CFG_DFLT);
	gpio_configure(DISPLAY_TO_AP_ID2, GPIO_CFG_DFLT);
	gpio_configure(DISPLAY_TO_AP_ID3, GPIO_CFG_DFLT);
}

uint32_t target_get_display_panel_type(void)
{
	if (!gpio_display_id_valid) {
		ipod6_get_display_info();
	}

	return display_panel;
}

uint32_t target_get_display_touch_type(void)
{
	if (!gpio_display_id_valid) {
		ipod6_get_display_info();
	}

	return display_touch;
}

#if WITH_HW_POWER
int pmu_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm);
int pmu_get_data(int dev, uint16_t reg, uint8_t *byte);

static int product_pmu_gpiocfg(int gpiono, uint8_t conf1,  uint8_t conf2)
{
	const uint16_t base=0x0400+(gpiono-1)*2;

	int rc=pmu_set_data( 0, base, conf1, 0 );
	if ( rc==0 ) rc=pmu_set_data( 0, base+1, conf2, 0 );

	return rc;
}
#endif

void product_target_early_init(void)
{
	// Get the display panel type.
	//The display IDs need the display's rails to be turned on
	ipod6_power_display(true);
	ipod6_get_display_info();

#if WITH_HW_POWER
	// <rdar://problem/16862108> M7: Configure binned voltages for M7 A1 parts
	const uint32_t frev=chipid_get_fuse_revision();
	const uint32_t crev=chipid_get_chip_revision();
	if ( crev>=0x01 && frev>=0x03 )  { 
		uint8_t vsel=0;
		
		// bin is at bits 37-43 in fuse (or 7 bits in rCFG_FUSE1[5-11] )
		uint8_t bin=(rCFG_FUSE1>>5)&0x7f;
		switch ( bin  )	{
			case 0x34: vsel=0x54; break; // bin1 
			case 0x2f: vsel=0x4c; break; // bin2
			case 0x2a: vsel=0x44; break; // bin3 
			default: /* warn? maybe... */ break;
		}

		if ( vsel ) {
			int rc=pmu_set_data( 0, 0x100, vsel, 1); 				// buck0_vsel
			if ( rc==0 ) rc=pmu_set_data( 0, 0x101, vsel-0x10, 1); 	// buck0_vsel_alt, vsel-50mV
			if ( rc<0 ) dprintf(DEBUG_CRITICAL, "PMU: cannot change buck0_vsel* (%d)\n", rc);
		}
	}
#endif
}

#define ENVAR_PWR_PATH "pwr-path"

void product_target_late_init(void)
{
#if WITH_HW_POWER
	const uint32_t rev=ipod6_get_board_rev();

	// <rdar://problem/16720846> Change behavior of PMU GPIO 13 (GPIO_DISABLE_PWR_PATH)
	if ( rev<=BOARD_REV_PROTO2A ) {

		uint8_t level=0;
#if RELEASE_BUILD && PRODUCT_IBOOT
			level=1;  // disable debug power on customer builds by default
#endif

#if WITH_ENV && PRODUCT_IBOOT
		const char *boot_args = env_get("boot-args");
		if ( boot_args != NULL ) {
			char *arg_str = strstr(boot_args, ENVAR_PWR_PATH"=");
			if ( arg_str != NULL ) {
				level = ( arg_str[strlen(ENVAR_PWR_PATH"=")] == '1' );
			}
		}
		// override with nvram environment variable (if it exists)
		level = env_get_bool(ENVAR_PWR_PATH, level);
		dprintf(DEBUG_CRITICAL, "PMU: pwr-path=%d\n", level);
#endif
		int rc=product_pmu_gpiocfg(13, ( level<<1 ) | 0x1, 0x2 );
		if ( rc<0 ) dprintf(DEBUG_CRITICAL, "PMU: pwr_path failed (%d)\n", rc);
	}

	// PCVB syscfg key (radar 18950042)
	//   bytes 0-3   : Version number = 3
	//   bytes 4-7   : CV offset in Volts, 4 byte fixed point decimal
	//   bytes 8-11  : buck0 offset in Volts, 4 byte fixed point decimal
	//   bytes 12-15 : buck1 offset in Volts, 4 byte fixed point decimal
	SInt32 cal_data[4] = {0};
	syscfgCopyDataForTag('PCVB', (void*)&cal_data, 4*sizeof(SInt32));
	// Check key version
	if (cal_data[0] >= 3) {
		uint8_t data = 0;
		SInt32 offset = 0;
		// "CHG_CTRL_E = CHG_TRIM2 - ceiling[CV_offset*160]"
		if (cal_data[1] > 0 && pmu_get_data(0, 0x4cb, &data) == 0) {
			offset = cal_data[1] * 160;
			data = (offset & 0xFFFF) ? data-(offset>>16)-1 : data-(offset>>16);
			int rc=pmu_set_data(0, 0x4c4, data, 1);
			if ( rc<0 ) dprintf(DEBUG_CRITICAL, "PMU: cannot change chg_ctrl_e (%d)\n", rc);
		}
		// "BUCK0_VSEL = BUCK0_VSEL + ceiling[Buck0_offset*320]"
		if (cal_data[2] > 0 && pmu_get_data(0, 0x100, &data) == 0) {
			offset = cal_data[2] * 320;
			data = (offset & 0xFFFF) ? data+(offset>>16)+1 : data+(offset>>16);
			int rc=pmu_set_data(0, 0x100, data, 1);
			if ( rc==0 ) rc=pmu_set_data( 0, 0x101, data-0x10, 1); // buck0_vsel_alt, vsel-50mV
			if ( rc<0 ) dprintf(DEBUG_CRITICAL, "PMU: cannot change buck0_vsel (%d)\n", rc);
		}
		// "BUCK1_VSEL = BUCK1_VSEL + ceiling[Buck1_offset*320]"
		if (cal_data[3] > 0 && pmu_get_data(0, 0x120, &data) == 0) {
			offset = cal_data[3] * 320;
			data = (offset & 0xFFFF) ? data+(offset>>16)+1 : data+(offset>>16);
			int rc=pmu_set_data(0, 0x120, data, 1);
			if ( rc<0 ) dprintf(DEBUG_CRITICAL, "PMU: cannot change buck1_vsel (%d)\n", rc);
		}
	}


	// <rdar://problem/19050193> iBoot: need to dynamicall change UART2_BERMUDA_TO_AP_CTS_L from pulldown to pullup
	// toggle VDD_MAIN_SW4 for bermuda, 
	if ( target_config_ap() && ( rev<=BOARD_REV_EVT ) ) {
		int rc;
		uint8_t data=0x7f;

		rc=pmu_set_data(0, 0x0085, data, 1);
		if ( rc<0 ) {
			dprintf(DEBUG_CRITICAL, "PMU: cannot enable VDD_MAIN_SW4 (%d)\n", rc);
		} else {
			task_sleep( 2 * 1000 ); // settle time

			data=0x3f;
			rc=pmu_set_data(0, 0x0085, data, 1);
			if ( rc<0 ) dprintf(DEBUG_CRITICAL, "PMU: cannot disable VDD_MAIN_SW4 (%d)\n", rc);

			gpio_configure_pupdn(GPIO_BERMUDA_TO_AP_CTS, GPIO_PUP);
		}
	}

#endif

	// Turn on DockFIFO clock gating
	for(int i = 0; i < 8; i++)
	{
		dockfifo_enable_clock_gating(i);
	}

	target_init_boot_manifest();


	// Cache Dali info into NVRAM
	// platform_late_init() only occurs if we didn't chargetrap, so by this point
	// we know for sure we have ASP NVRAM.
	extern void target_init_fast_dali();
	target_init_fast_dali();
}

bool product_target_should_recover(void)
{
	return platform_get_request_dfu2() && power_has_usb();
}

bool product_target_should_poweron(bool *cold_button_boot)
{
#if WITH_HW_POWER
	int		result;
	uint8_t		error_stage;

	// in special case of rebooting from Dali, ignore button press
	result = power_get_nvram(kPowerNVRAMiBootErrorStageKey, &error_stage);
	if (result == 0 && error_stage == kPowerNVRAMiBootStagePrechargeReboot) {
		boot_clear_error_count();   // don't generate a crash log for this
		return true;
	}

	if (power_get_boot_flag() == kPowerBootFlagColdButton) *cold_button_boot = true;
#else
	*cold_button_boot = false;
#endif // WITH_HW_POWER

	return !*cold_button_boot || platform_get_request_dfu1();
}

bool product_target_should_poweroff(bool at_boot)
{
	static bool was_precharge_reboot = false;

#if WITH_HW_POWER
	// <rdar://problem/22369404>
	//
	// If rebooting out of Dali mode, the user could've initiated this action by holding down the power button.
	// We do not want an extra-long hold of the power button to result in the device turning off.

	int result;
	uint8_t error_stage;

	result = power_get_nvram(kPowerNVRAMiBootErrorStageKey, &error_stage);

	if (was_precharge_reboot || (result == 0 && error_stage == kPowerNVRAMiBootStagePrechargeReboot)) {
		boot_clear_error_count();   // don't generate a crash log for this

#if PRODUCT_IBOOT || PRODUCT_IBEC
		// The error stage needs to be cleared because precharge reboot really isn't an error,
		// but since this is called in a loop by the idleoff_task we need to remember that we are
		// rebooting from Dali.
		//
		// Only clear the key in iBoot. In LLB, we need to preserve the error stage key as-is for the next stage.
		was_precharge_reboot = true;
		power_set_nvram(kPowerNVRAMiBootErrorStageKey, kPowerNVRAMiBootStageOff);
#endif
		return false;
	}
#endif
	return platform_get_request_dfu1() && (!at_boot || !power_has_usb());
}

void * target_get_display_configuration(void)
{
	switch(target_get_display_panel_type()){
	case DISPLAY_PANEL_TYPE_C1_DC:
	case DISPLAY_PANEL_TYPE_SLTSID:
	case DISPLAY_PANEL_TYPE_CRADLE:
	case DISPLAY_PANEL_TYPE_ALT_DISP_POC1:
		display_config = 0x00000971;
		break;
	case DISPLAY_PANEL_TYPE_POR_DISP_S:
	case DISPLAY_PANEL_TYPE_POR_DISP_B:
	case DISPLAY_PANEL_TYPE_POR_DISP_B_VID:
		display_config = 0x00000951;
		break;
	}
	return ((void *)(&display_config));
}

#if WITH_ENV

void product_target_setup_default_environment(void)
{
	uint32_t panel_type_index;

	env_set("boot-device", "asp_nand", 0);
	env_set("display-color-space","RGB888", 0);
	panel_type_index = target_get_display_panel_type();
	env_set("display-timing", display_panel_types[panel_type_index], 0);
#if !RELEASE_BUILD
	env_set("diags-path", "/AppleInternal/Diags/bin/diag.img4", 0);
	env_set("diags-vendor-path", "/AppleInternal/Diags/bin/diag-vendor.img4", 0);
#endif // !RELEASE_BUILD
}

#endif // WITH_ENV

bool product_target_is_display_in_video_mode(void)
{
	switch(target_get_display_panel_type()){
	case DISPLAY_PANEL_TYPE_C1_DC:
	case DISPLAY_PANEL_TYPE_SLTSID:
	case DISPLAY_PANEL_TYPE_CRADLE:
	case DISPLAY_PANEL_TYPE_ALT_DISP_POC1:
	case DISPLAY_PANEL_TYPE_POR_DISP_B_VID:
		return true;
	case DISPLAY_PANEL_TYPE_POR_DISP_S:
	case DISPLAY_PANEL_TYPE_POR_DISP_B:
	case DISPLAY_PANEL_TYPE_ALT_DISP_N31:
		return false;
	}

	return false;
}

bool product_target_no_burst_mode(void)
{
	switch(target_get_display_panel_type()){
	case DISPLAY_PANEL_TYPE_POR_DISP_B_VID:
		return true;
	}

	return false;
}

//The following 3 fuctions uses the values as defined in https://seg-docs.ecs.apple.com/projects/m7//release/specs/Apple/Top/M7_display_timing.xls 
//“M7 Command Mode Timing”
//
uint32_t product_target_get_ulps_in_delay(void)
{
	switch(target_get_display_panel_type()){
	case DISPLAY_PANEL_TYPE_C1_DC:
	case DISPLAY_PANEL_TYPE_SLTSID:
	case DISPLAY_PANEL_TYPE_CRADLE:
	case DISPLAY_PANEL_TYPE_ALT_DISP_POC1:
	case DISPLAY_PANEL_TYPE_POR_DISP_B_VID:
		return 0;
	case DISPLAY_PANEL_TYPE_POR_DISP_S:
		return  13056;
	case DISPLAY_PANEL_TYPE_POR_DISP_B:
		return 14976;
	case DISPLAY_PANEL_TYPE_ALT_DISP_N31:
		return 19680;
	}

	return false;
}

uint32_t product_target_get_ulps_end_delay(void)
{
	switch(target_get_display_panel_type()){
	case DISPLAY_PANEL_TYPE_C1_DC:
	case DISPLAY_PANEL_TYPE_SLTSID:
	case DISPLAY_PANEL_TYPE_CRADLE:
	case DISPLAY_PANEL_TYPE_ALT_DISP_POC1:
	case DISPLAY_PANEL_TYPE_POR_DISP_B_VID:
		return 0;
	case DISPLAY_PANEL_TYPE_POR_DISP_S:
		return 722538;
	case DISPLAY_PANEL_TYPE_POR_DISP_B:
		return 637035;
	case DISPLAY_PANEL_TYPE_ALT_DISP_N31:
		return 584135;
	}

	return false;
}

uint32_t product_target_get_ulps_out_delay(void)
{
	switch(target_get_display_panel_type()){
	case DISPLAY_PANEL_TYPE_C1_DC:
	case DISPLAY_PANEL_TYPE_SLTSID:
	case DISPLAY_PANEL_TYPE_CRADLE:
	case DISPLAY_PANEL_TYPE_ALT_DISP_POC1:
	case DISPLAY_PANEL_TYPE_POR_DISP_B_VID:
		return 0;
	case DISPLAY_PANEL_TYPE_POR_DISP_S:
		return 655913;
	case DISPLAY_PANEL_TYPE_POR_DISP_B:
		return 570410;
	case DISPLAY_PANEL_TYPE_ALT_DISP_N31:
		return 517510;
	}

	return false;
}

#if WITH_DEVICETREE

void display_update_device_tree()
{
#if WITH_HW_DISPLAY_PMU
	display_pmu_update_device_tree("arm-io/i2c1/display-pmu");
#endif
	DTNode		*disp0_node;
	DTNode		*node;
	char 		*propName;
	uint32_t 	propSize;
	void 		*propData;
	bool		video_mode;

	video_mode = product_target_is_display_in_video_mode();
	// Find the DISP0 (display-subsystem 0) node
	if (FindNode(0, "arm-io/disp0", &disp0_node)) {
		propName = "video-mode";
		if (FindProperty(disp0_node, &propName, &propData, &propSize)) {
			*((bool*)propData) = video_mode;
		}

		propName = "esd-workaround";
		if (FindProperty(disp0_node, &propName, &propData, &propSize)) {
			bool enable_esd_workaround = (target_config_ap() && ipod6_get_board_rev() <= BOARD_REV_EVT);
			*((uint32_t*)propData) = enable_esd_workaround;
		}
	}
	if (video_mode) {
		if (FindNode(0, "arm-io/mipi-dsim", &node)) {
			propName = "supports-auto-ulps";
			//Do not turn on Auto ULPS on when on video mode
			if (FindProperty(node, &propName, &propData, &propSize)) {
				*((bool*)propData) = false;
			}
		}
	}

#if WITH_HW_DISPLAY_SUMMIT
	DTNode		*backlight_node;
	if (FindNode(0, "arm-io/mipi-dsim/lcd", &node)) {
		extern int summit_update_device_tree(DTNode *summit_node, DTNode *clcd_node, DTNode *backlight_node);
		FindNode(0, "backlight", &backlight_node);
		summit_update_device_tree(node, disp0_node, backlight_node);
	}
#endif //WITH_HW_DISPLAY_SUMMIT
}
void hid_update_device_tree()
{
	DTNode 		*node;
	char 		*propName;
	uint32_t 	propSize;
	void 		*propData;

	if (FindNode(0, MULTITOUCH_DTPATH, &node)) {
		propName = "hid-fw-personality";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			memset(propData, 0, propSize);
			strlcpy(propData, display_touch_types[display_touch], propSize);
		}
		propName = "orb-f-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			memset(propData, 0, propSize);
			syscfgCopyDataForTag('OFCl', propData, propSize);
		}
		propName = "orb-o-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			memset(propData, 0, propSize);
			syscfgCopyDataForTag('OOCl', propData, propSize);
		}
		propName = "orb-i-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			memset(propData, 0, propSize);
			syscfgCopyDataForTag('OICo', propData, propSize);
		}
	}
}

static void spu_update_device_tree(void)
{
	const char* accel_path = "arm-io/spu/iop-spu-nub/accel";
	const char* gyro_path = "arm-io/spu/iop-spu-nub/gyro";
	const char* als_path = "arm-io/spu/iop-spu-nub/als";
	DTNode 		*node;
	char 		*propName;
	uint32_t 	propSize;
	void 		*propData;

	/* Accel Calibration */
	if (FindNode(0, accel_path, &node)) {
	  propName = "accel-interrupt-calibration";
	  if (FindProperty(node, &propName, &propData, &propSize)) {
		memset(propData, 0, propSize);
		syscfgCopyDataForTag('AICl', propData, propSize);
	  }

	  propName = "accel-sensitivity-calibration";
	  if (FindProperty(node, &propName, &propData, &propSize)) {
		memset(propData, 0, propSize);
		syscfgCopyDataForTag('ASCl', propData, propSize);
	  }

	  propName = "accel-orientation";
	  if (FindProperty(node, &propName, &propData, &propSize)) {
		memset(propData, 0, propSize);
		syscfgCopyDataForTag('ARot', propData, propSize);
	  }

	  propName = "low-temp-accel-offset";
	  if (FindProperty(node, &propName, &propData, &propSize)) {
		memset(propData, 0, propSize);
		syscfgCopyDataForTag('LTAO', propData, propSize);
	  }

	}

	/* Gyro Calibration */
	if (FindNode(0, gyro_path, &node)) {
	  propName = "gyro-interrupt-calibration";
	  if (FindProperty(node, &propName, &propData, &propSize)) {
		memset(propData, 0, propSize);
		syscfgCopyDataForTag('GICl', propData, propSize);
	  }

	  propName = "gyro-sensitivity-calibration";
	  if (FindProperty(node, &propName, &propData, &propSize)) {
		memset(propData, 0, propSize);
		syscfgCopyDataForTag('GSCl', propData, propSize);
	  }

	  propName = "gyro-orientation";
	  if (FindProperty(node, &propName, &propData, &propSize)) {
		memset(propData, 0, propSize);
		syscfgCopyDataForTag('GRot', propData, propSize);
	  }

	  propName = "gyro-temp-table";
	  if (FindProperty(node, &propName, &propData, &propSize)) {
		memset(propData, 0, propSize);
		syscfgCopyDataForTag('GYTT', propData, propSize);
	  }

	  propName = "gyro-trim-calibration";
	  if (FindProperty(node, &propName, &propData, &propSize)) {
		memset(propData, 0, propSize);
		syscfgCopyDataForTag('GTCl', propData, propSize);
	  }
	}

	/* ALS Calibration */
	if (FindNode(0, als_path, &node)) {
	  propName = "alsCalibration";
	  if (FindProperty(node, &propName, &propData, &propSize)) {
		memset(propData, 0, propSize);
		syscfgCopyDataForTag('LSCI', propData, propSize);
	  }
	}
}

static void opal_update_device_tree(void)
{
	const char* opal_path = "arm-io/opal";
	DTNode 		*node;
	char 		*propName;
	uint32_t 	propSize;
	void 		*propData;

	if (FindNode(0, opal_path, &node)) {
		propName = "li-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			memset(propData, 0, propSize);
			syscfgCopyDataForTag('LiCl', propData, propSize);
		}

		propName = "pt-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			memset(propData, 0, propSize);
			syscfgCopyDataForTag('PlCl', propData, propSize);
		}

		propName = "device-board-revision";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*(uint32_t*)(propData) = ipod6_get_board_rev();
		}		
	}
}

static void audio_update_device_tree(const char* dtpath)
{
	DTNode 		*node;
	char 		*propName;
	uint32_t 	propSize;
	void 		*propData;

	if (FindNode(0, dtpath, &node)) {
		propName = "actuator-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			memset(propData, 0, propSize);
			syscfgCopyDataForTag('TCal', propData, propSize);
		}
		propName = "codec-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			memset(propData, 0, propSize);
			syscfgCopyDataForTag('CCal', propData, propSize);
		}		
	}
}

static void charger_update_device_tree(void)
{
	DTNode 		*node;
	char 		*propName;
	uint32_t 	propSize;
	void 		*propData;

	if (FindNode(0, "arm-io/i2c0/ext-charger", &node)) {
		// Cjay syscfg key (radar 18717802)
		SeajayCal calData = { {0,0,0,0,0},{0,0,0,0,0} };
		syscfgCopyDataForTag('Cjay', (void*)&calData, sizeof(SeajayCal));
		// Check key version
		if (calData.Hdr.Version >= SEAJAY_CAL_VERSION && calData.Hdr.Magic == SEAJAY_CAL_MAGIC && calData.Hdr.IsNotCalibrated == 0) {
			if (calData.Item.Status & kCalStatus_Offset) {
				propName = "vrect-calibrate-offset";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					memset(propData, 0, propSize);
					*((uint32_t*)propData) = calData.Item.Offset;
				}
			}
			if (calData.Item.Status & kCalStatus_Gain) {
				propName = "vrect-calibrate-gain";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					memset(propData, 0, propSize);
					*((uint32_t*)propData) = calData.Item.Gain;
				}
			}
		}
	}
}

static void device_material_update_device_tree(void)
{
	DTNode 		*node;
	char 		*propName;
	uint32_t 	propSize;
	void 		*propData;

	DMin_t Dmin;
	bzero(&Dmin, sizeof(Dmin));

	// Look up the 'DMin' tag from syscfg.
	//
	// Determining which DMin to use: Pre-Monarch companion SW can't handle
	// the N2xB configurations, so we had to "lie" in syscfg about the enclosure material
	//
	// On such configurations, the factory has put the true color in a DMin Override key 'DMnO'.
	// If this key is present, we should use it instead of DMin.
	int result = -1;
	result = syscfgCopyDataForTag('DMnO', (uint8_t*)&Dmin, sizeof(Dmin));
	if (result > 0) {
		// If we copied more than 0 bytes, DMnO was present. We should use it
		dprintf(DEBUG_INFO, "DMin override key 'DMnO' found. Using it instead of DMin\n");
	} else {
		result = syscfgCopyDataForTag('DMin', (uint8_t*)&Dmin, sizeof(Dmin));
		if (result < 0) {
			dprintf(DEBUG_CRITICAL, "Unable to find DMin key in syscfg.\n");
			return;
		}
	}
	if (Dmin.version != DMIN_VERSION_2) {
		dprintf(DEBUG_CRITICAL, "DMin version %u is unspported.\n", (unsigned int)Dmin.version);
		return;
	}
	if (FindNode(0, MULTITOUCH_DTPATH, &node)) {
		/**
		* DT property 		dmin key
		*
		* enc-top-type		topEnclosure
		* enc-bot-type		botEnclosure
		* fcm-type		fcmType
		* fcm-arcoated-type	fcmARCoated
		* fcm-inkcolor-type	fcmInkColor
		**/

		propName = "enc-top-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.topEnclosure;
		}
		propName = "enc-bot-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.botEnclosure;
		}
		propName = "fcm-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.fcmType;
		}
		propName = "fcm-arcoated-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.fcmARCoated;
		}
		propName = "fcm-inkcolor-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.fcmInkColor;
		}

	}

	const char* opal_path = "arm-io/opal";

	if (FindNode(0, opal_path, &node)) {
		/**
		* Device Tree Key	DMin Field
		enc-top-type		topEnclosure
		enc-bot-type		botEnclosure
		bcm-window-type		bcmWindow
		bcm-lens-type		bcmLens
		pt-sensor-type		bcmPTSensorType
		pt-led-type		bcmPTLEDType
		lisa-opamp-type		lisaOAType
		lisa-sensor-ic		lisaSensorIC
		lisa-encoder-wheel	lisaEncoderWheel
		lisa-knurl		lisaKnurl		
		**/		
		propName = "enc-top-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.topEnclosure;
		}
		propName = "enc-bot-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.botEnclosure;
		}
		propName = "bcm-window-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.bcmWindow;
		}
		propName = "bcm-lens-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.bcmLens;
		}
		propName = "pt-sensor-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.bcmPTSensorType;
		}
		propName = "pt-led-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.bcmPTLEDType;
		}
		propName = "lisa-opamp-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.lisaOAType;
		}
		propName = "lisa-sensor-ic";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.lisaSensorIC;
		}
		propName = "lisa-encoder-wheel";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.lisaEncoderWheel;
		}
		propName = "lisa-knurl";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.lisaKnurl;
		}

	}

	const char* spu_path = "arm-io/spu";

	if (FindNode(0, spu_path, &node)) {
		propName = "pt-led-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
		  *((uint32_t*)propData) = Dmin.v2.bcmPTLEDType;
		}
	}

	if (FindNode(0, WIFI_DTPATH, &node)) {
		/**
		* DT property 		dmin key
		*
		* enc-top-type		topEnclosure
		* enc-bot-type		botEnclosure
		* fcm-type		fcmType
		**/

		propName = "enc-top-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.topEnclosure;
		}
		propName = "enc-bot-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.botEnclosure;
		}
		propName = "fcm-type";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*((uint32_t*)propData) = Dmin.v2.fcmType;
		}

	}

}

static void aggd_update_device_tree(void)
{
	// <rdar://problem/18615376> Add HW config info to allow for better data analysis
	//
	// We want carry aggd data correlated with hardware revision
	// If we are building DEBUG/DEVELOPMENT, populate the DT with this info.

	DTNodePtr chosen;
	uint32_t propSize;
	char *propName;
	void *propData;

	if (FindNode(0, "chosen", &chosen)) {
		propName = "debug-board-revision";
		if (FindProperty(chosen, &propName, &propData, &propSize)) {
#if !RELEASE_BUILD
			*((uint32_t*)(propData)) = ipod6_get_board_rev();
#else
			propName[0] = '~';
#endif
		}
	}
}

static void stockholm_update_device_tree(void)
{
	DTNodePtr node;
	uint32_t propSize;
	char *propName;
	void *propData;

	// Stockholm calibration
	if (FindNode(0, "arm-io/uart2/stockholm", &node)) {
		propName = "calibration";
		if (FindProperty(node, &propName, &propData, &propSize))
			syscfgCopyDataForTag('NFCl', propData, propSize);
	}

	// Bermuda dynamic CTS doesn't apply to DEV boards or pre-EVT devices. We will delete
	// the dynamic pin assignment in that case.
	if (!target_config_ap() || ipod6_get_board_rev() > BOARD_REV_EVT) {
		if (FindNode(0, "arm-io/uart2", &node)) {
			propName = "function-cts";
			if (FindProperty(node, &propName, &propData, &propSize))
				propName[0] = '~';
		}
	}
}

static void asp_update_device_tree(void)
{
	// <rdar://problem/19298429> hung writing large images to ASP boot block

	DTNodePtr chosen;
	uint32_t propSize;
	char *propName;
	void *propData;

	if (FindNode(0, "defaults", &chosen)) {
		propName = "llb-num-blks";
		if (FindProperty(chosen, &propName, &propData, &propSize)) {
                    *((uint32_t*)(propData)) = ASP_LLB_OVERRIDE_NUM_BLKS;
		}
	}
}

int product_target_update_device_tree(void)
{	

	display_update_device_tree();

	hid_update_device_tree();
	opal_update_device_tree();
	spu_update_device_tree();
	charger_update_device_tree();

	audio_update_device_tree("arm-io/mca0/audio-actuator0");

	device_material_update_device_tree();
	aggd_update_device_tree();
	stockholm_update_device_tree();
        asp_update_device_tree();

	return target_pass_boot_manifest();
}

#endif // WITH_DEVICETREE

bool target_needs_chargetrap(void)
{
	// Dali powertrap should only be considered in iBoot
	return power_needs_precharge() || power_needs_thermal_trap();
}


bool target_do_chargetrap(void)
{
#if APPLICATION_IBOOT && PRODUCT_IBOOT
	// Dali powertrap should only be considered in iBoot
	if (target_needs_chargetrap()) {
		dprintf(DEBUG_INFO, "Charge trap requested");

		// Currently, Dali expects us to initialize the display.
		// This requires two consecutive calls, just like in the main.c boot path.
		platform_init_display();
		platform_init_display();
		int boot_dali_flash(void);
		if (boot_dali_flash() < 0)
		{
			panic("Failed to boot dali but chargetrap was requested\n");
		}
		return true;
	}
#endif
	return false;
}

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

u_int32_t power_get_boot_battery_level(void)
{
	return 0;
}

int power_get_nvram(u_int8_t key, u_int8_t *data)
{
	return -1;
}

int power_set_nvram(u_int8_t key, u_int8_t data)
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

int power_backlight_enable(u_int32_t backlight_level)
{
	return 0;
}

#endif /* ! WITH_HW_POWER */
