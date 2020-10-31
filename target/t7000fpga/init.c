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
#include <drivers/flash_nor.h>
#include <lib/env.h>
#include <lib/mib.h>
#include <platform.h>
#include <platform/gpiodef.h>
#include <platform/memmap.h>
#include <platform/soc/chipid.h>
#include <sys.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <target.h>

#if !SUPPORT_FPGA
#error "FPGA target without SUPPORT_FPGA set"
#endif

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 1);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

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
	return 0;
}

bool target_should_poweron(bool *cold_button_boot)
{
	*cold_button_boot = true;
	return true;
}

bool target_should_poweroff(bool at_boot)
{
	return 0;
}

#if !PRODUCT_LLB
bool target_get_property(enum target_property prop, void *data, int maxdata, int *retlen)
{
	return false;
}
#endif

void * target_get_display_configuration(void)
{
	return ((void *)(NULL));
}

#if WITH_ENV

void target_setup_default_environment(void)
{
	env_set("boot-args", " debug=0x14e serial=3 amfi_unrestrict_task_for_pid=1 amfi_allow_any_signature=1 amfi_get_out_of_my_way=1 cs_enforcement_disable=1 fips_mode=0", 0);
	// boot-device is set in platform's init.c
	env_set("display-color-space","RGB888", 0);
	env_set("display-timing", "fpga-wsvga", 0);
	env_set("ramdisk-delay", "30000000", 0); /* This tells PurpleRestore to slow down; rdar://6345846 */
	env_set("idle-off", "false", 0);
	env_set("bootdelay", "3", 0);
	env_set("debug-uarts", "3", 0);
}

#endif

#if WITH_DEVICETREE

#define MAX_NODES_PER_DEVICE	3
struct fpgaNode_t
{
	uint32_t	mask;				 // Block enable mask
	const char	*nodeName[MAX_NODES_PER_DEVICE]; // Device tree nodes to disable if block is absent
};

// Devices tree nodes to disable based upon FPGA block instantiation
struct fpgaNode_t fpgaNodes[] = {
	{ FPGA_HAS_PCIE,  { "arm-io/apcie",   "arm-io/dart-apcie0", "arm-io/dart-apcie0/mapper-apcie0" }},
	{ FPGA_HAS_PCIE,  { "arm-io/apcie",   "arm-io/dart-apcie1", "arm-io/dart-apcie1/mapper-apcie1" }},
	{ FPGA_HAS_VXD,   { "arm-io/vxd",     NULL,                  NULL                              }},
	{ FPGA_HAS_AVE,   { "arm-io/ave",     "arm-io/dart-ave",    "arm-io/dart-ave/mapper-ave"       }},
	{ FPGA_HAS_ISP,   { "arm-io/isp",     "arm-io/dart-isp",    "arm-io/dart-isp/mapper-isp"       }},
	{ FPGA_HAS_DISP0, { "arm-io/disp0",   "arm-io/dart-disp0",  "arm-io/dart-disp0/mapper-disp0"   }},
	{ FPGA_HAS_DISP1, { "arm-io/disp1",   "arm-io/dart-disp1",  "arm-io/dart-disp1/mapper-disp1"   }},
	{ FPGA_HAS_MSR,   { "arm-io/scaler0", "arm-io/dart-scaler", "arm-io/dart-scaler/mapper-scaler" }},
	{ FPGA_HAS_JPEG,  { "arm-io/jpeg",    "arm-io/dart-jpeg",   "arm-io/dart-jpeg/mapper-jpeg"     }},
	{ FPGA_HAS_GFX,   { "arm-io/sgx",     NULL,                 NULL                               }},
};

int target_update_device_tree(void)
{
	DTNode		*node;
	uint32_t	propSize;
	char		*propName;
	void		*propData;
	uint32_t	i, j;

	// Get the hardware blocks that are instantiated on this FPGA.
	uint32_t fpgaBlocks = chipid_get_fpga_block_instantiation();
	dprintf(DEBUG_SPEW, "FPGA block instantiation mask = 0x%X\n", fpgaBlocks);

	for (i = 0; i < sizeof(fpgaNodes)/sizeof(fpgaNodes[0]); i++) {
		if ((fpgaBlocks & fpgaNodes[i].mask) == 0) {
			for (j = 0; j < MAX_NODES_PER_DEVICE; j++) {
				if (fpgaNodes[i].nodeName[j] == NULL) {
					break;
				}
				if (FindNode(0, fpgaNodes[i].nodeName[j], &node)) {
					propName = "compatible";
					if (FindProperty(node, &propName, &propData, &propSize)) {
						if (j == 0) {
							dprintf(DEBUG_INFO,
							       "FPGA block not instantiated: disabling %s\n",
							       fpgaNodes[i].nodeName[j]);
						}
						propName[0] = '~';
					} else {
						dprintf(DEBUG_SPEW,
							"FPGA: Could not find 'compatible' property for %s DT node\n",
							fpgaNodes[i].nodeName[j]);
					}
				} else {
					dprintf(DEBUG_SPEW,
						"FPGA: Could not find %s DT node\n",
						fpgaNodes[i].nodeName[j]);
				}
			}
		}
	}
	return 0;
}
#endif


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
