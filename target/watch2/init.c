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

#include "init_fpga.h"
#include "init_product.h"
#include "init_sim.h"

MIB_CONSTANT(kMIBTargetOsPictureScale,	kOIDTypeUInt32, 2);
MIB_CONSTANT(kMIBTargetPictureRotate,	kOIDTypeInt32,  0);

void target_early_init(void)
{
#if !CONFIG_FPGA && !CONFIG_SIM
	product_target_early_init();
#endif
}

void target_late_init(void)
{
#if !CONFIG_FPGA && !CONFIG_SIM
	product_target_late_init();
#endif
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
#if CONFIG_SIM
	return sim_target_should_recover();
#elif CONFIG_FPGA
	return fpga_target_should_recover();
#else
	return product_target_should_recover();
#endif
}

bool target_should_poweron(bool *cold_button_boot)
{
#if CONFIG_SIM
	return sim_target_should_poweron(cold_button_boot);
#elif CONFIG_FPGA	
	return fpga_target_should_poweron(cold_button_boot);
#else
	return product_target_should_poweron(cold_button_boot);
#endif
}

bool target_should_poweroff(bool at_boot)
{
#if CONFIG_SIM
	return sim_target_should_poweroff(at_boot);
#elif CONFIG_FPGA
	return fpga_target_should_poweroff(at_boot);
#else
	return product_target_should_poweroff(at_boot);
#endif
}

#if WITH_ENV

void target_setup_default_environment(void)
{
#if CONFIG_SIM
	sim_target_setup_default_environment();
#elif CONFIG_FPGA
	fpga_target_setup_default_environment();
#else
	product_target_setup_default_environment();
#endif
}

#endif // WITH_ENV

#if WITH_DEVICETREE

int target_update_device_tree(void)
{
#if CONFIG_SIM
	return sim_target_update_device_tree();
#elif CONFIG_FPGA
	return fpga_target_update_device_tree();
#else
	return product_target_update_device_tree();
#endif
}

#endif // WITH_DEVICETREE
