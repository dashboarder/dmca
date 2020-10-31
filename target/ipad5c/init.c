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
#include <drivers/power.h>
#include <drivers/iic.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <lib/devicetree.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/gpiodef.h>
#include <platform/soc/chipid.h>
#include <target.h>
#include <target/boardid.h>
#include <drivers/display.h>
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
#include <drivers/displayport/displayport.h>
#endif

// Display panel type
typedef enum {
	DISPLAY_PANEL_TYPE_UNKNOWN = 0,
	DISPLAY_PANEL_TYPE_CAMELIA,
	DISPLAY_PANEL_TYPE_TULIP,
	// Metadata
	DISPLAY_PANEL_TYPE_COUNT
} display_panel_type;

static const char *display_panel_types[] = {
	"UNKNOWN",
	"j99POC",
	"ipad5c",
};

typedef enum {
	DISPLAY_SWITCH_TYPE_UNKNOWN = 0,
	DISPLAY_SWITCH_TYPE_TEMP_KONA,
	DISPLAY_SWITCH_TYPE_KONA,
	// Metadata
	DISPLAY_SWITCH_TYPE_COUNT
} display_switch_type;

static const char *display_switch_types[] = {
	"UNKNOWN",
	"multi-touch,j99-st",
	"multi-touch,j99",
};

// Display panel id sense pins
#define DISPLAY_TO_AP_ID0	GPIO(22, 5)	// 181 : PCIE_PERST3_N		-> GPIO_DEVDOG_DETECT

uint32_t ipad5c_get_board_rev(void);
static void ipad5c_get_display_info(void);
uint32_t target_get_display_panel_type(void);
uint32_t target_get_display_switch_type(void);

static bool	gpio_board_rev_valid;
static bool	gpio_display_id_valid;
static uint32_t	gpio_board_rev;
static display_panel_type  display_panel;
static display_switch_type display_switch;
#if (APPLICATION_IBOOT && (PRODUCT_IBOOT || PRODUCT_IBEC))
static dp_t	dp;
#endif

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  -90);

bool check_is_board_deprecated = false;

uint32_t ipad5c_get_board_rev(void)
{
	if (!gpio_board_rev_valid) {
		gpio_configure(GPIO_BOARD_REV0, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV1, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV2, GPIO_CFG_IN);
		gpio_configure(GPIO_BOARD_REV3, GPIO_CFG_IN);

		gpio_configure_pupdn(GPIO_BOARD_REV0, GPIO_PUP);
		gpio_configure_pupdn(GPIO_BOARD_REV1, GPIO_PUP);
		gpio_configure_pupdn(GPIO_BOARD_REV2, GPIO_PUP);
		gpio_configure_pupdn(GPIO_BOARD_REV3, GPIO_PUP);

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
// The panel id pin mappings are described by the following id pin states:

//	ID0	Panel	Switch
//	----	-----	-----
//	PD	Camelia	Temp_kona
//	PU	Tulip	Temp_kona
//	Open	Tulip	Kona

static void ipad5c_get_display_info(void)
{
	if (target_config_ap()) {
		uint32_t get_board_rev;
		get_board_rev = ipad5c_get_board_rev();
		switch(get_board_rev) {
		case J99_PROTO0_BOARD_REV_LOCAL:
		case J99_PROTO0_BOARD_REV_CHINA:
			display_panel  = DISPLAY_PANEL_TYPE_CAMELIA;
			display_switch = DISPLAY_SWITCH_TYPE_TEMP_KONA;
			break;
		case J99_PROTO1_BOARD_REV_LOCAL:
		case J99_PROTO1_BOARD_REV_CHINA:
		case J99_PROTO2_BOARD_REV:
			display_panel  = DISPLAY_PANEL_TYPE_TULIP;
			display_switch = DISPLAY_SWITCH_TYPE_TEMP_KONA;
			break;
		default:
			display_panel  = DISPLAY_PANEL_TYPE_TULIP;
			display_switch = DISPLAY_SWITCH_TYPE_KONA;
			break;
		}
		dprintf(DEBUG_INFO, "get_board_rev %d Display panel = %s\nDisplay switch = %s\n",
				get_board_rev,
			display_panel_types[display_panel],
			display_switch_types[display_switch]);
	} else {
		const char *id0;

		gpio_configure(DISPLAY_TO_AP_ID0, GPIO_CFG_IN);

		gpio_configure_pupdn(DISPLAY_TO_AP_ID0, GPIO_PUP);

		spin(100); // Wait 100us

		if (gpio_read(DISPLAY_TO_AP_ID0)) {
			// ID0 could be externally pulled low or it could be
			// floating. Change the polarity of the internal pull.
			gpio_configure_pupdn(DISPLAY_TO_AP_ID0, GPIO_PDN);
			spin(100); // Wait 100us

			if (!gpio_read(DISPLAY_TO_AP_ID0)) {
				// ID0 is open.
				id0 = "open";
				display_panel  = DISPLAY_PANEL_TYPE_TULIP;
				display_switch = DISPLAY_SWITCH_TYPE_KONA;
				gpio_display_id_valid = true;
			} else {
				// ID0 has an external pull up.
				id0 = "pu";
				display_panel  = DISPLAY_PANEL_TYPE_TULIP;
				display_switch = DISPLAY_SWITCH_TYPE_TEMP_KONA;
				gpio_display_id_valid = true;
			}

		} else {
			// ID1 has an external pull down.
			id0 = "pd";
			display_panel  = DISPLAY_PANEL_TYPE_CAMELIA;
			display_switch = DISPLAY_SWITCH_TYPE_TEMP_KONA;
			gpio_display_id_valid = true;
		}

		dprintf(DEBUG_INFO, "%spanel id pin state: id0=%s\n",
			gpio_display_id_valid ? "" : "Unknown", id0);

		ASSERT(display_panel < DISPLAY_PANEL_TYPE_COUNT);
		ASSERT(display_switch < DISPLAY_SWITCH_TYPE_COUNT);

		dprintf(DEBUG_INFO, "Display panel = %s\nDisplay switch = %s\n",
			display_panel_types[display_panel],
			display_switch_types[display_switch]);

		gpio_configure(DISPLAY_TO_AP_ID0, GPIO_CFG_DFLT);
		gpio_display_id_valid = true;
	}
}

uint32_t target_get_display_panel_type(void)
{
	if (!gpio_display_id_valid) {
		ipad5c_get_display_info();
	}
	return display_panel;
}

uint32_t target_get_display_switch_type(void)
{
	if (!gpio_display_id_valid) {
		ipad5c_get_display_info();
	}
	return display_switch;
}

bool is_board_deprecated(void) {
	uint32_t board_id;
	uint32_t board_rev;

	board_id = platform_get_board_id();
	board_rev = ipad5c_get_board_rev();

	//Check Dev boards first.
	if (( board_id == TARGET_BOARD_ID_J98DEV || board_id == TARGET_BOARD_ID_J99DEV) && (board_rev == J98_DEV1_BOARD_REV)) {
		return true;
	}

	//Check AP boards
	switch(board_rev) {
	case J99_PROTO0_BOARD_REV_LOCAL:
	case J99_PROTO0_BOARD_REV_CHINA:
	case J99_PROTO1_BOARD_REV_LOCAL:
	case J99_PROTO1_BOARD_REV_CHINA:
		return true;
	}

	return false;
}

#if WITH_HW_POWER
int pmu_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm);
int charger_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm);
#endif

// TODO: Abstract DPOT code in a DPOT driver
// Begin DPOT code
#define GPU_UVD_DPOT_IIC_BUS			(3)

#define GPU_UVD_DPOT_IIC_ADDRESS		(0xA0)

/* Register addresses for GPU UVD DPOT */
#define GPU_UVD_DPOT_IVRA	(0x00)
#define GPU_UVD_DPOT_IVRB	(0x01)
#define GPU_UVD_DPOT_ACR	(0x10)

#define GPU_UVD_DPOT_READ_MISMATCH_ERROR	(-2)
#define GPU_UVD_DPOT_INVALID_WIPER_ERROR	(-3)

typedef enum {
	GPU_UVD_DPOT_WIPER_A,
	GPU_UVD_DPOT_WIPER_B
} gpu_uvd_dpot_wiper;

#if PRODUCT_LLB || PRODUCT_IBSS
static void gpu_uvd_dpot_error(int error_code) 
{
	panic("GPU UVD: Error communicating with DPOT (%d)\n", error_code);
}

static void gpu_uvd_dpot_poll_busy_bit(void) 
{
	int rc = 0;
	uint8_t read_data[2] = { 0 };
	uint8_t busy;

	do {
		read_data[0] = GPU_UVD_DPOT_ACR;
		if ((rc = iic_read(GPU_UVD_DPOT_IIC_BUS, GPU_UVD_DPOT_IIC_ADDRESS, &read_data[0], 1, &read_data[1], 1, IIC_COMBINED)))
			gpu_uvd_dpot_error(rc);

		busy = (read_data[1] >> 5) & 0x1;
	} while (busy);
}

static uint8_t gpu_uvd_dpot_get_wiper_voltage(gpu_uvd_dpot_wiper wiper)
{
	int rc = 0;
	uint8_t read_data[2] = { 0 };

	switch (wiper) {
		case GPU_UVD_DPOT_WIPER_A:
			read_data[0] = GPU_UVD_DPOT_IVRA;
			break;
		case GPU_UVD_DPOT_WIPER_B:
			read_data[0] = GPU_UVD_DPOT_IVRB;
			break;
		default:
			gpu_uvd_dpot_error(GPU_UVD_DPOT_INVALID_WIPER_ERROR);
	}

	if ((rc = iic_read(GPU_UVD_DPOT_IIC_BUS, GPU_UVD_DPOT_IIC_ADDRESS, &read_data[0], 1, &read_data[1], 1, IIC_COMBINED)))
		gpu_uvd_dpot_error(rc);

	return read_data[1];
}

static void gpu_uvd_dpot_set_wiper_voltage(gpu_uvd_dpot_wiper wiper, uint8_t wiper_voltage) 
{
	int rc = 0;
	uint8_t write_data[2];
	uint8_t written_wiper_voltage;

	switch (wiper) {
		case GPU_UVD_DPOT_WIPER_A:
			write_data[0] = GPU_UVD_DPOT_IVRA;
			break;
		case GPU_UVD_DPOT_WIPER_B:
			write_data[0] = GPU_UVD_DPOT_IVRB;
			break;
		default:
			gpu_uvd_dpot_error(GPU_UVD_DPOT_INVALID_WIPER_ERROR);
	}

	write_data[1] = wiper_voltage;
	
	if ((rc = iic_write(GPU_UVD_DPOT_IIC_BUS, GPU_UVD_DPOT_IIC_ADDRESS, write_data, 2))) 
		gpu_uvd_dpot_error(rc);

	gpu_uvd_dpot_poll_busy_bit();

	written_wiper_voltage = gpu_uvd_dpot_get_wiper_voltage(wiper);

	if (written_wiper_voltage != write_data[1]) {
		dprintf(DEBUG_CRITICAL, "GPU UVD: Error writing to GPU_UVD_DPOT_WIPER%s. Expected (0x%x) but read (0x%x).\n",
			((wiper == GPU_UVD_DPOT_WIPER_A) ? "A" : "B"), write_data[1], written_wiper_voltage);
		gpu_uvd_dpot_error(GPU_UVD_DPOT_READ_MISMATCH_ERROR);
	}
}

static bool gpu_uvd_dpot_is_nvmem_writable(void) 
{
	int rc = 0;
	uint8_t read_data[2] = { 0 };

	read_data[0] = GPU_UVD_DPOT_ACR;

	if ((rc = iic_read(GPU_UVD_DPOT_IIC_BUS, GPU_UVD_DPOT_IIC_ADDRESS, &read_data[0], 1, &read_data[1], 1, IIC_COMBINED)))
		gpu_uvd_dpot_error(rc);

	return read_data[1] != 0xc0;
}

static void gpu_uvd_dpot_set_nvmem_writable(bool enable) 
{
	// Clearing ACR[7] bit enables NV writes; ACR[6] always set to disable shutdown mode
	uint8_t acr_config = (enable) ? 0x40 : 0xc0;
	uint8_t write_data[2];
	uint8_t read_data[2];
	int rc = 0;

	write_data[0] = GPU_UVD_DPOT_ACR;
	write_data[1] = acr_config;

	if ((rc = iic_write(GPU_UVD_DPOT_IIC_BUS, GPU_UVD_DPOT_IIC_ADDRESS, write_data, 2)))
		gpu_uvd_dpot_error(rc);

	gpu_uvd_dpot_poll_busy_bit();

	read_data[0] = GPU_UVD_DPOT_ACR;
	if ((rc = iic_read(GPU_UVD_DPOT_IIC_BUS, GPU_UVD_DPOT_IIC_ADDRESS, &read_data[0], 1, &read_data[1], 1, IIC_COMBINED)))
		gpu_uvd_dpot_error(rc);

	if (read_data[1] != write_data[1]) {
		dprintf(DEBUG_CRITICAL, "GPU UVD: Error writing to GPU_UVD_DPOT_ACR. Expected (0x%x) but read (0x%x).\n",
				write_data[1], read_data[1]);
		gpu_uvd_dpot_error(GPU_UVD_DPOT_READ_MISMATCH_ERROR);
	}
}

static void gpu_uvd_dpot_init(void)
{
	uint32_t wiper_voltage_a, wiper_voltage_b;
	uint32_t pmu_v_uvd_voltage;

	// Get V4 voltage, convert to PMU VSEL, and convert back to mV to get actual PMU output for wiper equations
	pmu_v_uvd_voltage = chipid_get_gpu_voltage(CHIPID_GPU_VOLTAGE_V_UVD);
	platform_convert_voltages(BUCK_GPU, 1, &pmu_v_uvd_voltage);
	pmu_v_uvd_voltage = platform_get_dwi_to_mv(BUCK_GPU, pmu_v_uvd_voltage);

	// Use 32-bit fixed point arithmetic with a scaling factor of 1e4 to determine wiper voltage codes
	// Take ceiling of integer division using ceiling(x/y) = truncate[(x + y - 1) / y]
	// Code_UVD,det = Code_WiperA = 1280.06 * (VSEL_P6,volts - 0.08) - 1216.51
	wiper_voltage_a = (128006 * pmu_v_uvd_voltage - 10240480 - 121651000 + 100000 - 1) / 100000;
	// Code_ARM,th = Code_WiperB = 1280.06 * (VSEL_P6,volts - 0.045) - 1216.51
	wiper_voltage_b = (128006 * pmu_v_uvd_voltage - 5760270 - 121651000 + 100000 - 1) / 100000;

	gpio_write(GPIO_SOC2GPUUVD_EN, 1); // Enable GPU UVD system

	spin(10000); // Wait 10 ms for GPU UVD setup

	gpu_uvd_dpot_poll_busy_bit();

	// Check to configure Wiper A (minimize EEPROM writes to maximize on 100k write cycle endurance)
	if (gpu_uvd_dpot_get_wiper_voltage(GPU_UVD_DPOT_WIPER_A) != wiper_voltage_a) {
		if (!gpu_uvd_dpot_is_nvmem_writable())
			gpu_uvd_dpot_set_nvmem_writable(true);

		gpu_uvd_dpot_set_wiper_voltage(GPU_UVD_DPOT_WIPER_A, wiper_voltage_a);
	}

	// Check to configure Wiper B
	if (gpu_uvd_dpot_get_wiper_voltage(GPU_UVD_DPOT_WIPER_B) != wiper_voltage_b) {
		if (!gpu_uvd_dpot_is_nvmem_writable())
			gpu_uvd_dpot_set_nvmem_writable(true);

		gpu_uvd_dpot_set_wiper_voltage(GPU_UVD_DPOT_WIPER_B, wiper_voltage_b);
	}
	
	// Check to disable non-volatile writes
	if (gpu_uvd_dpot_is_nvmem_writable())
		gpu_uvd_dpot_set_nvmem_writable(false);

	gpio_write(GPIO_SOC2GPUUVD_EN, 0); // Disable GPU UVD system
}
#endif
// End DPOT code

void target_early_init(void)
{
	// Get the display panel type.
	ipad5c_get_display_info();

#if WITH_HW_POWER
	int rc;
	const uint32_t bid=platform_get_board_id();
	if ( bid == TARGET_BOARD_ID_J98DEV || bid == TARGET_BOARD_ID_J99DEV ) {
		rc = pmu_set_data(0, 0x040e, 0xc8, true);
		if ( rc!=0 ) dprintf(DEBUG_INFO, "pmu: write failed (%d)\n", rc);
	}
#if PRODUCT_LLB || PRODUCT_IBSS
	// <rdar://problem/18601044> J99 EVT+ Hall-Effect Sensor IRQs change
	if (!target_config_ap() || (ipad5c_get_board_rev() > J99_EVT_PLUS_BOARD_REV)) {
		// GPIO11
		rc = pmu_set_data(0, 0x040a, 0xea, true);
		if (rc != 0) dprintf(DEBUG_INFO, "pmu: write failed (%d)\n", rc);
		
		// GPIO13
		rc = pmu_set_data(0, 0x040c, 0xe8, true);
		if (rc != 0) dprintf(DEBUG_INFO, "pmu: write failed (%d)\n", rc);
	}
#if 0
        // this change is making charge to not charge <rdar://problem/19146740>, disabling ...
        // for detecting VCENTER on Ganges rdar://18532953 
        // set VSET to 3.73V allowing VCenter to 3.83 :: Vset + 100mV
        charger_set_data(3, 0x1f8, 0x00, true);
        charger_set_data(3, 0x1f9, 0x80, true);
#endif
#endif
#endif
#if PRODUCT_LLB || PRODUCT_IBSS
	pmgr_update_dvfm(platform_get_board_id(),ipad5c_get_board_rev());
	pmgr_update_gfx_states(platform_get_board_id(),ipad5c_get_board_rev());

	if (pmgr_check_gpu_use650MHz_binned(platform_get_board_id(), ipad5c_get_board_rev()) || 
		pmgr_check_gpu_use650MHz_unbinned(platform_get_board_id(), ipad5c_get_board_rev())) {
		gpu_uvd_dpot_init();
	}
#endif
}

void target_late_init(void)
{
	//Check if board is deprecated
	if (check_is_board_deprecated && is_board_deprecated()) {
		platform_not_supported();
	}

}

void target_init(void)
{
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
	switch(target_get_display_panel_type()){
	case DISPLAY_PANEL_TYPE_CAMELIA:
		dp.mode	=		0x1;
		dp.type	=		0x1;
		dp.min_link_rate =	0xa;
		dp.max_link_rate =	0xa;
		dp.lanes =		0x4;
		dp.ssc =		0x0;
		dp.alpm =		0x0;
		dp.vrr_enable =		0x1;
		dp.vrr_on =		0x0;
		dp.rx_n1=		0x0;
		dp.rx_n2=		0x0;
		dp.rx_n3=		0x0;
		dp.rx_n5=		0x0;
		dp.fast_link_training =	true;
		break;
	case DISPLAY_PANEL_TYPE_TULIP:
		dp.mode	=		0x1;
		dp.type	=		0x1;
		dp.min_link_rate =	0xc;
		dp.max_link_rate =	0xc;
		dp.lanes =		0x4;
		dp.ssc =		0x0;
		dp.alpm =		0x1;
		dp.vrr_enable =		0x1;
		dp.vrr_on =		0x1;
		dp.rx_n1=		25;
		dp.rx_n2=		2;
		dp.rx_n3=		16030;
		dp.rx_n5=		0x0;
		dp.fast_link_training =	true;
		break;
	}
	return ((void *)(&dp));
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
	
	if (panel_type_index == DISPLAY_PANEL_TYPE_CAMELIA) {
		env_set("adfe-tunables", "camelia", 0);
		env_set("adbe-tunables", "camelia", 0);
	}
}

#endif

#if WITH_DEVICETREE

// <rdar://problem/17365777> J99 Proto2: Update Device Tree to move BT & WiFi Enable Controls to the IOExpander
void target_ipad5c_use_pmu_gpios_wlan_bt(void)
{
	DTNode		*node;
	uint32_t        propSize;
	char            *propName;
	void            *propData;

	if (FindNode(0, WIFI_DTPATH, &node)) {
		//Fix reg on property 
		propName = "function-reg_on_p1";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			memset(propName, 0, kPropNameLength);
			strlcpy(propName, "function-reg_on", kPropNameLength);
		}
		propName = "function-reg_on";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			// Disable the proto2 and greater property
			propName[0] = '~';
		}
	}

	//Fix bluetooth
	if (FindNode(0, BT_DTPATH, &node)) {
		//Fix reg on property 
		propName = "function-power_enable_p1";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			memset(propName, 0, kPropNameLength);
			strlcpy(propName, "function-power_enable", kPropNameLength);
		}
		propName = "function-power_enable";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			// Disable the proto2 and greater property
			propName[0] = '~';
		}
	}
	//disable the gpio expander driver
	if (FindNode(0, "arm-io/i2c3/tca7408", &node)) {
		propName = "compatible";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			// Disable the proto2 and greater property
			propName[0] = '~';
		}
	}
}

void hid_update_device_tree()
{
	DTNode 		*node;
	char 		*propName;
	uint32_t 	propSize;
	void 		*propData;


	if (FindNode(0, "arm-io/spi1/multi-touch", &node)) {
		propName = "compatible";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			memset(propData, 0, propSize);
			strlcpy(propData, display_switch_types[display_switch], propSize);
		}
		if (display_switch != DISPLAY_SWITCH_TYPE_KONA) {
			//Fix up the device tree to support pre Kona touch
			propName = "function-clock_enable";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
			propName = "function-clock_enPre3";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				strlcpy(propName, "function-clock_enable", strlen(propName)+1);
			}
			propName = "function-power_ana";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
			propName = "function-power_pp3";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				strlcpy(propName, "function-power_ana", strlen(propName)+1);
			}
		} else {
			propName = "swd-flash-update";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
		}
		propName = "multitouch-to-display-offset";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('MtDO', propData, propSize);
		}
		propName = "firefly-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('FfCl', propData, propSize);
		}
	}
}

//Remove when all HW prior to EVT is deprecated
static void ipad5c_set_old_backlight_values(DTNode *node)
{
	uint32_t        propSize;
	char            *propName;
	void            *propData;
	
	propName = "milliAmps2DACPart1MaxCurrent";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		  ((uint32_t *)propData)[0] = 0x00030000;
	}
	propName = "milliAmps2DACPart2MaxCurrent";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		  ((uint32_t *)propData)[0] = 0x00140000;
	}
	propName = "calibratedMaxCurrent";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		  ((uint32_t *)propData)[0] = 0x0011E3D7;
	}
	propName = "calibratedMidCurrent";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		  ((uint32_t *)propData)[0] = 0x00068000;
	}
	propName = "mA2Nits2ndOrderCoef";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		  ((uint32_t *)propData)[0] = 0xFFFFFFF9;
	}
	propName = "mA2Nits1stOrderCoef";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		  ((uint32_t *)propData)[0] = 0x00000ED1;
	}
	propName = "mA2Nits0thOrderCoef";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		  ((uint32_t *)propData)[0] = 0xFFFFFFDC;
	}
	propName = "nits2mAmps2ndOrderCoef";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		  ((uint32_t *)propData)[0] = 0x0000A100;
	}
	propName = "nits2mAmps1stOrderCoef";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		  ((uint32_t *)propData)[0] = 0x00114042;
	}
	propName = "nits2mAmps0thOrderCoef";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		  ((uint32_t *)propData)[0] = 0x000002B7;
	}
}

int target_update_device_tree(void)
{
#if WITH_HW_DISPLAY_EDP 
	DTNode		*clcd_node = NULL, *backlight_node = NULL, *lcd_node = NULL;
	DTNode		*lpdp_node = NULL;
	DTNode		*node;
#endif

	uint32_t        propSize;
	char            *propName;
	void            *propData;
	dt_node_t       *dtnode;

#if WITH_HW_DISPLAY_EDP
	if (FindNode(0, DP_DTPATH, &node)) {
		extern int edp_update_device_tree(DTNode *edp_node, DTNode *lcd_node, DTNode *clcd_node, DTNode *backlight_node);
		FindNode(0, "arm-io/disp0", &clcd_node);
		FindNode(0, DP_DTPATH "/lcd", &lcd_node);
		FindNode(0, "backlight", &backlight_node);
		edp_update_device_tree(node, lcd_node, clcd_node, backlight_node);

		extern int lpdp_phy_update_device_tree(DTNode *lpdp_node);
		FindNode(0, DPPHY_DTPATH, &lpdp_node);
		lpdp_phy_update_device_tree(lpdp_node);
		
	}
#endif

	// Update the codec node with acoustic transducer scale data
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
			}
		}
	}

	// Update the speaker calibration data
	if (FindNode(0, "arm-io/i2c1/audio-speaker0", &node)) {
		propName = "speaker-calib";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SpCl', propData, propSize);
		}
	}
	
	// Update the als calibration data for all nodes that may be present on the dev board
	if (FindNode(0, "arm-io/i2c2/als1", &node)) {
		propName = "alsCalibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('LSCI', propData, propSize);
		}
	}
	if (FindNode(0, "arm-io/i2c2/als2", &node)) {
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

		//<rdar://problem/18433591> J99: Update screen brightness settings for EVT systems
		//Default is evt. Fix preevt
		if (ipad5c_get_board_rev() > J99_EVT_BOARD_REV) {
			ipad5c_set_old_backlight_values(node);
		}
	}

	// Update the compass calibration data
	if (FindNode(0, "arm-io/uart7/oscar/compass", &node)) {
		propName = "compass-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('CPAS', propData, propSize);
		}
	}

	// Update the gyro calibration data
	if (FindNode(0, "arm-io/uart7/oscar/gyro", &node)) {
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
	if (FindNode(0, "arm-io/uart7/oscar/accelerometer", &node)) {
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
	if (FindNode(0, "arm-io/uart7/oscar/pressure", &node)) {
		propName = "pressure-offset-calibration";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			syscfgCopyDataForTag('SPPO', propData, propSize);
		}
	}

	// WLAN Capri A0 workarounds -- !!!FIXME!!! Remove this when Capri A0 is deprecated
	// <rdar://problem/16597296> Remove Capri A0 PCIe hacks
	if (FindNode(0, "arm-io/apcie/pci-bridge1/wlan", &node)) {
		if (chipid_get_chip_revision() > CHIP_REVISION_A0) {
			// Disable workarounds for Capri A1 and later.
			propName = "acpie-l1ss-workaround";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
			propName = "pci-wake-l1pm-disable";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
		} else {
			// Fix up the hacks for Capri A0
			propName = "pci-l1pm-control";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
			propName = "pci-l1pm-control-a0";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[16] = '\0';
			}
		}
	}

 	const uint32_t board_id = platform_get_board_id();
	// IO expander GPIO 4 is not used on EVT2, so disable function-acc_sw_en.
 	if ((board_id == TARGET_BOARD_ID_J98AP || board_id == TARGET_BOARD_ID_J99AP) && (ipad5c_get_board_rev() <= J99_EVT2_BOARD_REV)) {
		if (FindNode(0, "dock-orion", &node)) {
			propName = "function-acc_sw_en";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
		}
	} else {
		// Undo IO expander otp-data values in device tree, which were set for EVT2.
		if (FindNode(0, "arm-io/i2c3/tca7408", &node)) {
			propName = "otp-data";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				((uint32_t *)propData)[0] = 0xff000000;
				((uint32_t *)propData)[1] = 0x00000000;
			}
		}
	}
 	/*
 	 * Need to support HW which does not have the GPIO Expander.
 	 * HW before Proto2 do not have the expander
 	 * HW before Dev2 do not have the expander
 	 */
 	if (((board_id == TARGET_BOARD_ID_J98AP || board_id == TARGET_BOARD_ID_J99AP) && (ipad5c_get_board_rev() >= J99_PROTO1_BOARD_REV_CHINA)) ||
 	    ((board_id == TARGET_BOARD_ID_J98DEV || board_id == TARGET_BOARD_ID_J99DEV) && (ipad5c_get_board_rev() < J98_DEV2_BOARD_REV))) {
 		target_ipad5c_use_pmu_gpios_wlan_bt();
 	}

 	// EVT2 (or dev boards with dip switches set to EVT2) will use the micro based Orion
 	// Older revs will continue to support TriStar based Orion for bring-up
 	if (ipad5c_get_board_rev() == J99_EVT2_BOARD_REV) {
        // Enable Micro Orion
        if (dt_find_node(0, "arm-io/i2c1/orion-ic", &dtnode)) {
        	propName = "AAPL,ignore";
        	if (dt_get_prop(dtnode, &propName, &propData, &propSize)) {
                dt_remove_prop(dtnode, propName);
        	}
        }
        // Enable aod flashing of Orion's FW
        if (dt_find_node(0, "arm-io/aod4", &dtnode)) {
        	propName = "AAPL,ignore";
        	if (dt_get_prop(dtnode, &propName, &propData, &propSize)) {
                dt_remove_prop(dtnode, propName);
        	}
        }
    } else {
        // Enable TriStar Orion
        if (dt_find_node(0, "arm-io/i2c1/tristar-orion", &dtnode)) {
        	propName = "AAPL,ignore";
        	if (dt_get_prop(dtnode, &propName, &propData, &propSize)) {
        		dt_remove_prop(dtnode, propName);
        	}
        }
 	}

	hid_update_device_tree();
 
	return 0;
}

#endif // WITH_DEVICETREE

#if WITH_PAINT

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
