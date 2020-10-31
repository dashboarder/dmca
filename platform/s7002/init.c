/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <arch/arm/arm.h>
#include <debug.h>
#include <drivers/aes.h>
#include <drivers/asp.h>
#include <drivers/csi.h>
#include <drivers/anc_boot.h>
#if WITH_HW_DOCKFIFO_UART || WITH_HW_DOCKFIFO_BULK
#include <drivers/dockfifo/dockfifo.h>
#endif
#if WITH_CONSISTENT_DBG
#include <drivers/consistent_debug.h>
#endif
#include <drivers/dma.h>
#include <drivers/display.h>
#if WITH_HW_DISPLAY_PMU
#include <drivers/display_pmu.h>
#endif
#include <drivers/iic.h>
#include <drivers/miu.h>
#include <drivers/power.h>
#include <drivers/shmcon.h>
#include <drivers/spi.h>
#include <drivers/uart.h>
#include <drivers/usb/usb_public.h>
#include <drivers/usbphy.h>
#include <lib/env.h>
#include <lib/nonce.h>
#include <platform.h>
#include <platform/clocks.h>
#include <platform/gpio.h>
#include <platform/gpiodef.h>
#include <platform/int.h>
#include <platform/miu.h>
#include <platform/memmap.h>
#include <platform/power.h>
#include <platform/timer.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/chipid.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <platform/soc/spu.h>
#include <platform/soc/reconfig.h>
#include <platform/soc/chipid.h>
#include <platform/trampoline.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <target.h>

static uint8_t boot_debug;
static bool gDisplayEnabled;

/*
 *  boot_interface_pin tables
 *    tables are executed in order for disable and reverse order for enable
 *
 */

struct boot_interface_pin {
	gpio_t	  pin;
	uint32_t enable;
	uint32_t disable;
};

#if WITH_ANC_BOOT
/* Note: The code below expects this table to begin with CS1 through CS0 */
static const struct boot_interface_pin nand_boot_interface_pins[] =
{
	{ GPIO(10, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_CEN0
	{ GPIO( 9, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_CEN1
	{ GPIO(11, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_CLE
	{ GPIO(11, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_ALE
	{ GPIO(10, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_REN
	{ GPIO(11, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_WEN
	{ GPIO(11, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO0
	{ GPIO(11, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO1
	{ GPIO(11, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO2
	{ GPIO(10, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO3
	{ GPIO(10, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO4
	{ GPIO(10, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO5
	{ GPIO(10, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO6
	{ GPIO(10, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// NAND_IO7
};
#endif /* WITH_ANC_BOOT */

#if WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI
static const struct boot_interface_pin spi0_boot_interface_pins[] =
{
	{ GPIO(12, 4), GPIO_CFG_OUT_1, GPIO_CFG_DFLT },	// SPI0_SSIN
	{ GPIO(12, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_SCLK
	{ GPIO(12, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_MOSI
	{ GPIO(12, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// SPI0_MISO
};
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */

static void platform_init_boot_strap(void);
static void platform_bootprep_darwin(void);

extern void amc_configure_mem_reconfig_ram(void);

int platform_early_init(void)
{
#if APPLICATION_IBOOT && (PRODUCT_LLB || PRODUCT_IBSS)
	/* Verify that the fuses and SecureROM R/W access has been disabled */
	if (!chipid_get_fuse_lock() || (((*(volatile uint32_t *)SECURITY_REG) & ROM_READ_DISABLE) == 0))
		panic("Fuses are unlocked or SecureROM is enabled\n");
#endif

#if WITH_HW_PLATFORM_POWER
	/* initialize the s5l8960x pmgr driver */
	platform_power_init();
#endif

#if WITH_CONSISTENT_DBG && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)
	consistent_debug_init();
#endif

#if WITH_HW_MIU
	/* CIF, SCU, remap setup */
	miu_init();
#endif

#if WITH_HW_CLOCKS
	/* initialize the clock driver */
	clocks_init();
#endif

#if WITH_HW_AIC
	/* initialize the AIC, mask all interrupts */
	interrupt_init();
#endif

#if WITH_HW_TIMER
	timer_init(0);
#endif

#if WITH_HW_USBPHY
	usbphy_power_down();
#endif

#if WITH_HW_UART
	/* do whatever uart initialization we need to get a simple console */
	uart_init();
	debug_enable_uarts(3);
#endif

#if WITH_HW_DOCKFIFO_UART
	/* do whatever uart initialization we need to get a simple console */
	dockfifo_uart_init();
	debug_enable_uarts(3);
#endif

#if WITH_SHM_CONSOLE
	shmcon_init();
#endif

#if WITH_IIC
	iic_init();
#endif

#if !PRODUCT_IBOOT && !PRODUCT_IBEC
	platform_init_power();

#if WITH_BOOT_STAGE
	boot_check_stage();
#endif
#endif

#if WITH_BOOT_STAGE
	boot_set_stage(kPowerNVRAMiBootStageProductStart);
#endif

#if WITH_HW_POWER
	power_get_nvram(kPowerNVRAMiBootDebugKey, &boot_debug);
	debug_enable_uarts(boot_debug);
#endif

#if WITH_HW_DMA
	/* initialize the dma engine */
	dma_init();
#endif

#if WITH_TARGET_CONFIG
	target_early_init();
#endif

	return 0;
}

int platform_late_init(void)
{
#if WITH_ENV
	/* publish secure-boot flag for restore mode */
	env_set_uint("secure-boot", 1, 0);
#endif

#if WITH_HW_USB && WITH_USB_MODE_RECOVERY
	usb_early_init();
#endif

#if WITH_HW_POWER
	power_late_init();
#endif

#if WITH_TARGET_CONFIG
	target_late_init();
#endif

#if WITH_CSI
  csi_late_init();
#endif

  return 0;
}

int platform_init_setup_clocks(void)
{
#if WITH_HW_CLOCKS
	clocks_set_default();
#endif
	return 0;
}

int platform_init_hwpins(void)
{
	// need board id to select default pinconfig
	platform_init_boot_strap();

#if (PRODUCT_LLB || PRODUCT_IBSS) && WITH_HW_DOCKFIFO_UART
	// Enable all DockFIFO's used by platform in LLB (<rdar://problem/18708180>)
	platform_dockfifo_access_enable((1) | (1<<1) | (1<<2) | (1<<3));
#endif

#if WITH_HW_GPIO
	/* finish initializing the gpio driver */
	gpio_init_pinconfig();
#endif

        return 0;
}

int platform_init_internal_mem(void)
{
#if WITH_HW_MIU
        /* initialize sram bus */
        miu_initialize_internal_ram();
#endif

	return 0;
}

int platform_init_mainmem(bool resume)
{
#if WITH_HW_MIU && APPLICATION_IBOOT
	/* initialize sdram */
	miu_initialize_dram(resume);

	/* always remap the memory region at this point, regardless of resume,
	 * because the early MMU init always runs and we have to fix up. */
	platform_init_mainmem_map();
#endif

	return 0;
}

void platform_init_mainmem_map(void)
{
}

int platform_init_power(void)
{
#if WITH_HW_POWER
	power_init();
#endif

	return 0;
}

int platform_init_display(void)
{
#if WITH_HW_DISPLAYPIPE
	static bool displayInitOnce;
	uint32_t backlight_level = 0;
	int result = 0;

	/* initialize the display if not already enabled */
	if (!gDisplayEnabled) {
		if (!displayInitOnce) {
			platform_quiesce_display();
			result = display_init();
		} else result = -1;

		/* if initialization fails, make sure we never try again.  On success, gDisplayEnabled will be set,
		 * ensuring no reinitialization unless platform_quiesce_display is called first.
		 */
		if (result != 0) {
			displayInitOnce = true;
		}
	}

	if (result == 0) {
		gDisplayEnabled = true;
		backlight_level = env_get_uint("backlight-level", 0xffffffff);
	} else {
#if PRODUCT_IBSS
		/* 
		 * WARNING: With move to DFU-mode iBSS, this will never be
		 * reached.  Thus, it should either be eliminated or
		 * refactored to make sense, perhaps by changing
		 * condition from PRODUCT_IBSS to WITH_RESTORE_BOOT.
		 */
		debug_enable_uarts(3);
#endif
	}
	if (env_get_uint("display-rotation", 0) == 1)
	{
		display_set_rotation(true);
	}
#endif
	return 0;
}

int platform_init_display_mem(addr_t *base, size_t *size)
{
#if WITH_HW_DISPLAYPIPE
	addr_t base_rounded = *base;
	addr_t end_rounded = *base + *size;
	size_t size_rounded = *size;

	/* Map the framebuffer as device memory and
	 * round the base and size to the mapping granule.
	 */
	base_rounded = ROUNDDOWN(base_rounded, MB);
	end_rounded = ROUNDUP(end_rounded, MB);
	size_rounded = end_rounded - base_rounded;
	arm_mmu_map_section_range(base_rounded, base_rounded, size_rounded/MB, kARMMMUWriteCombined, false, true);
	*base = base_rounded;
	*size = size_rounded;
#endif

	return 0;
}

int platform_init_mass_storage(void)
{
#if WITH_HW_ASP
	return (asp_nand_open());
#else
	return 0;
#endif
}

int platform_quiesce_hardware(enum boot_target target)
{
	bool quiesce_clocks = false;

#if APPLICATION_SECUREROM
	quiesce_clocks = true;
#endif

#if WITH_TARGET_CONFIG
	target_quiesce_hardware();
#endif

#if WITH_HW_USB
	usb_quiesce();
#endif

#if WITH_CSI
	csi_quiesce(target);
#endif

	switch (target) {
		case BOOT_HALT:
		case BOOT_DARWIN_RESTORE:
			break;
		case BOOT_IBOOT:
		case BOOT_DARWIN:
#if WITH_BOOT_STAGE
			boot_set_stage(kPowerNVRAMiBootStageProductEnd);
#endif
			break;
		case BOOT_DIAGS:
		case BOOT_SECUREROM:
			quiesce_clocks = true;
			// fall through to default
		default:
#if WITH_BOOT_STAGE
			boot_set_stage(kPowerNVRAMiBootStageOff);
#endif
			break;
	}

#if WITH_HW_TIMER
	timer_stop_all();
#endif
#if WITH_HW_AIC
	interrupt_mask_all();
#endif

	if (quiesce_clocks) {
#if WITH_HW_CLOCKS
		clocks_quiesce();
#endif
	}

#if APPLICATION_IBOOT
	switch (target) {
		case BOOT_IBOOT :
			break;

		default:
			break;
	}
#endif

	return 0;
}

int platform_quiesce_display(void)
{
#if WITH_HW_DISPLAYPIPE
	// Turn off the back light
	power_backlight_enable(0);

	/* quiesce the clcd panel */
	display_quiesce(true);
#endif
	gDisplayEnabled = false;

	return 0;
}

int platform_bootprep(enum boot_target target)
{
	uint32_t gids = ~0, uids = ~0;	/* leave crypto keys alone by default */

	/* prepare hardware for booting into various targets */

#if WITH_HW_CLOCKS
	if (target != BOOT_IBOOT) clocks_set_performance(kPerformanceHigh);
#endif

#if WITH_TARGET_CONFIG
	target_bootprep(target);
#endif

	/* If we're not restoring, reset the watchdog-on-wake until enabled */
	if ((boot_debug & kPowerNVRAMiBootDebugWDTWake) && (target == BOOT_DARWIN))
	{
		boot_debug &= ~kPowerNVRAMiBootDebugWDTWake;
#if WITH_HW_POWER
		power_set_nvram(kPowerNVRAMiBootDebugKey, boot_debug);
#endif
	}

	switch (target) {
#if APPLICATION_IBOOT
		case BOOT_DARWIN_RESTORE:
			platform_quiesce_display();
			if (boot_debug & kPowerNVRAMiBootDebugWDTWake)
			    wdt_enable();
			/* Darwin only gets the UID on resume */
			uids = 1;
			gids = 0;
			break;

		case BOOT_DARWIN:
			/* setup Reconfig block, program MEM and SOC configs */
			platform_bootprep_darwin();

			if (boot_debug & kPowerNVRAMiBootDebugWDTWake)
			    wdt_enable();

#if WITH_HW_MIU
			miu_select_remap(REMAP_SDRAM);
#endif
			/* Leave the GIDs alone since we Need them to
			   generate a special key for stockholm
			   pairing.  GIDs will be disabled by the
			   AppleS7002AES driver once the special key
			   has been
			   generated. <rdar://problem/17696432> */
			uids = 1;
			gids = ~0;
			break;

		case BOOT_DIAGS:
			platform_quiesce_display();
#if WITH_BOOT_STAGE
			boot_clear_error_count();
#endif
			break;

		case BOOT_IBOOT:
			platform_quiesce_display();
			break;
#endif

		case BOOT_SECUREROM:
			platform_quiesce_display();
#if WITH_HW_MIU
			miu_select_remap(REMAP_SDRAM);
#endif
			break;

		case BOOT_MONITOR:
			break;

		case BOOT_UNKNOWN:
			platform_quiesce_display();
			break;
		default:
			; // do nothing
	}

	/* make sure that fuse lock bit is set. */
	chipid_set_fuse_lock(true);

	/* Let security override keys */
	if (!security_allow_modes(kSecurityModeGIDKeyAccess))
		gids = 0;
	if (!security_allow_modes(kSecurityModeUIDKeyAccess))
		uids = 0;

	/* disable all keys not requested */
	platform_disable_keys(~gids, ~uids);

	return 0;
}

void platform_mmu_setup(bool resume)
{
	RELEASE_ASSERT(false == resume);

	/* mark usable ram as cached/buffered, shared */
	arm_mmu_map_section_range(SRAM_BASE, SRAM_BASE, ROUNDUP(SRAM_BANK_LEN, MB)/MB, kARMMMUNormal, true, false);
#if APPLICATION_SECUREROM
	arm_mmu_map_section_range(VROM_BASE, VROM_BASE, ROUNDUP(VROM_BANK_LEN, MB)/MB, kARMMMUNormal, true, false);
#endif

#if !(PRODUCT_IBSS || PRODUCT_LLB)
	arm_mmu_map_section(0, TEXT_BASE, kARMMMUNormalRX, true);
#endif

	/* in cases where we are running from DRAM, remap it now */
#if APPLICATION_IBOOT 
	arm_mmu_map_section_range(SDRAM_BASE, SDRAM_BASE, ROUNDUP(SDRAM_BANK_LEN * SDRAM_BANK_COUNT, MB)/MB, kARMMMUNormal, true, false);
#endif
}

int platform_init(void)
{
	bool __unused skip_ans = false;

#if WITH_HW_SPI
	spi_init();
#endif

#if PRODUCT_IBOOT && WITH_TARGET_CHARGETRAP
	// Skip ANS initialization if booting into Dali mode from iBoot -- we can use ANC NVRAM.
	if (target_needs_chargetrap()) {
		skip_ans = true;
	}
#endif


#if WITH_HW_ASP && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)
	if (!skip_ans) {
		csi_init(CSI_COPROC_ANS);   /* coproc switchboard used by nand driver to communicate with ans iop */
		asp_init();
	}
#endif

#if WITH_ANC_FIRMWARE
// For iBoot, ANC_FIRMWARE is only used when booting into Dali mode (e.g. skip_ans is set)
#if PRODUCT_IBOOT
	if (skip_ans) {
		anc_firmware_init();
	}
#else
// For other boot stages, ANC_FIRMWARE is used whenever it's present
	anc_firmware_init();
#endif // PRODUCT_IBOOT
#endif // WITH_ANC_FIRMWARE

#if WITH_TARGET_CONFIG
	target_init();
#endif

	return 0;
}

int platform_init_usb()
{
	int ret = 0;

#if WITH_HW_DOCKFIFO_BULK
	/* BOOT_CONFIG[3] is reserved to select between USB or DebugFifo to move data. 
	 * BOOT_CONFIG[0..3] is read and saved in rPMGR_SCRATCH0 (bits 15..8)
	 */
	if (((rPMGR_SCRATCH0 >> 11) & 1) == 0) {
		extern struct usb_controller_functions *usb_dockfifo_controller_init();
		ret = usb_init_with_controller(usb_dockfifo_controller_init());
	}
	else 
#endif	
	{
#if WITH_HW_SYNOPSYS_OTG
		extern struct usb_controller_functions *synopsys_otg_controller_init();
		ret = usb_init_with_controller(synopsys_otg_controller_init());
#endif
	}

	return ret;
}

int platform_debug_init(void)
{
#if WITH_HW_USB
	platform_init_usb();
#endif

#if WITH_TARGET_CONFIG
	target_debug_init();
#endif

	return 0;
}

void platform_poweroff(void)
{
	platform_quiesce_display();

#if WITH_TARGET_CONFIG
	target_poweroff();
#endif

#if WITH_HW_POWER
#if WITH_BOOT_STAGE
	boot_set_stage(kPowerNVRAMiBootStageOff);
#endif

	power_shutdown();
#endif
	for(;;);
}

uint32_t platform_set_performance(uint32_t performance_level)
{
	uint32_t old_performance_level = kPerformanceHigh;

#if WITH_HW_CLOCKS
	old_performance_level = clocks_set_performance(performance_level);
#endif

	return old_performance_level;
}

#if WITH_DEVICETREE

int platform_update_device_tree(void)
{
	DTNode *node;
	uint32_t propSize;
	char *propName;
	void *propData;

	// Find the cpu0 node.
	if (FindNode(0, "cpus/cpu0", &node)) {

		// Fill in the cpu frequency
		propName = "clock-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint64_t freq = clock_get_frequency(CLK_CPU);
			memcpy(propData, &freq, propSize);
		}

		// Fill in the memory frequency
		propName = "memory-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint64_t freq = clock_get_frequency(CLK_MEM);
			memcpy(propData, &freq, propSize);
		}

		// Fill in the bus frequency
		propName = "bus-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint64_t freq = clock_get_frequency(CLK_BUS);
			memcpy(propData, &freq, propSize);
		}

		// Fill in the peripheral frequency
		propName = "peripheral-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint64_t freq = clock_get_frequency(CLK_PERIPH);
			memcpy(propData, &freq, propSize);
		}

		// Fill in the fixed frequency
		propName = "fixed-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint64_t freq = clock_get_frequency(CLK_FIXED);
			memcpy(propData, &freq, propSize);
		}

		// Fill in the time base frequency
		propName = "timebase-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			uint64_t freq = clock_get_frequency(CLK_TIMEBASE);
			memcpy(propData, &freq, propSize);
		}
	}

	// Find the arm-io node
	if (FindNode(0, "arm-io", &node)) {
		// Fill in the clock-frequencies table
		propName = "clock-frequencies";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			clock_get_frequencies(propData, propSize / sizeof(uint32_t));
		}
	}

	// Find the pmgr node
	if (FindNode(0, "arm-io/pmgr", &node)) {
		pmgr_update_device_tree(node);
		miu_update_device_tree(node);
	}

#if WITH_HW_USBPHY
	// Find the otgphyctrl node
	if (FindNode(0, "arm-io/otgphyctrl", &node)) {
	        usbphy_update_device_tree(node);
	}
#endif

	return target_update_device_tree();
}

#endif

uint32_t platform_get_board_id(void)
{
	uint32_t board_id;

	ASSERT((rPMGR_SCRATCH0 & kPlatformScratchFlagBootStrap) != 0);

	board_id = (rPMGR_SCRATCH0 >> 16) & 0xFF;

	return board_id;
}

uint32_t platform_get_boot_config(void)
{
	uint32_t boot_config;

#if WITH_HW_DOCKFIFO_BULK
	/*
	 * BOOT_CONFIG[3] is reserved, if its set system will use USB else DockFifo for moving data.
	 * Boot-config is essentially 3 bits on M7 BOOT_CONFIG[0..2]
	 */
	boot_config = (rPMGR_SCRATCH0 >> 8) & 0x07;
#else
	boot_config = (rPMGR_SCRATCH0 >> 8) & 0xFF;
#endif

	return boot_config;
}

bool platform_get_boot_device(int32_t index, enum boot_device *boot_device, uint32_t *boot_flag, uint32_t *boot_arg)
{
		uint32_t boot_config = platform_get_boot_config();

		/* S7002 supports one boot device then USB-DFU or Bulk-DockFifo-DFU per boot config */

		/* If the index is not zero force DFU mode */
		if (index != 0) index = 1;

		switch (boot_config) {
			case  0: /* SPI 0 */
				*boot_device = BOOT_DEVICE_SPI;
				*boot_flag = 0;
				*boot_arg = 0;
				break;

			case  1: /* SPI 0 Test Mode */
				*boot_device = BOOT_DEVICE_SPI;
				*boot_flag = BOOT_FLAG_TEST_MODE;
				*boot_arg = 0;
				break;

			case  2: /* ANS */
				*boot_device = BOOT_DEVICE_NAND;
				*boot_flag = 0;
				*boot_arg = 0;
				break;

			case  3: /* ANS Test Mode */
				*boot_device = BOOT_DEVICE_NAND;
				*boot_flag = BOOT_FLAG_TEST_MODE;
				*boot_arg = 0;
				break;

	/*		case  4-15: Unused		*/

			default:
				return false;
		}

		/* Change boot_device and boot_arg for DFU Mode */
		/* Don't change flags */
		if (index == 1) {
			*boot_device = BOOT_DEVICE_USBDFU;
			*boot_arg = 0;
		}

		return true;
}

#if WITH_HW_POWER
uint32_t platform_get_base_soc_voltage(void)
{
	return chipid_get_soc_voltage(TARGET_BOOT_SOC_VOLTAGE);
}
#endif

int platform_get_soc_voltages(uint32_t count, uint32_t *voltages)
{
	uint32_t cnt;

	if (voltages == 0) return -1;

	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = platform_get_base_soc_voltage();
	}

	return 0;
}

void platform_enable_boot_interface(bool enable, enum boot_device boot_device, uint32_t boot_arg)
{
	const struct boot_interface_pin *pins = 0;
	uint32_t cnt, func, pin_count = 0;
	gpio_t pin;

	switch (boot_device) {
#if WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI
		case BOOT_DEVICE_SPI :
			if (boot_arg == 0) {
				pins = spi0_boot_interface_pins;
				pin_count = (sizeof(spi0_boot_interface_pins) / sizeof(spi0_boot_interface_pins[0]));
			}
			break;
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */

#if WITH_ANC_BOOT
		case BOOT_DEVICE_NAND :
			pins = nand_boot_interface_pins;
			pin_count = (sizeof(nand_boot_interface_pins) / sizeof(nand_boot_interface_pins[0]));

			break;
#endif /* WITH_ANC_BOOT */

#if WITH_USB_DFU
		case BOOT_DEVICE_USBDFU :
			/* USB is always configured */
			break;
#endif /* WITH_USB_DFU */

		default :
			break;
	}

	for (cnt = 0; cnt < pin_count; cnt++) {
		if (enable) {
			pin = pins[pin_count - 1 - cnt].pin;
			func = pins[pin_count - 1 - cnt].enable;
		} else {
			pin = pins[cnt].pin;
			func = pins[cnt].disable;
		}

		dprintf(DEBUG_INFO, "platform_enable_boot_interface: 0 %x, %x\n", pin, func);
		gpio_configure(pin, func);
	}
}

uint64_t platform_get_nonce(void)
{
	uint64_t	nonce;
	uint32_t	*nonce_words = (uint32_t *)&nonce;

	// If rPMGR_SCRATCH0[1] set then the nonce has already been generated
	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagNonce) == 0) {

		nonce = platform_consume_nonce();

		rPMGR_SCRATCH14 = nonce_words[0];
		rPMGR_SCRATCH15 = nonce_words[1];

		rPMGR_SCRATCH0 |= kPlatformScratchFlagNonce;
	} else {
		nonce_words[0] = rPMGR_SCRATCH14;
		nonce_words[1] = rPMGR_SCRATCH15;
	}

	return nonce;
}

bool platform_get_ecid_image_personalization_required(void)
{
	return true;
}

uint32_t platform_get_osc_frequency(void)
{
	return chipid_get_osc_frequency();
}

bool platform_get_usb_cable_connected(void)
{
#if WITH_HW_DOCKFIFO_BULK	
	/* BOOT_CONFIG[3] is reserved to select between USB or DebugFifo to move data. 
	 * BOOT_CONFIG[0..3] is read and saved in rPMGR_SCRATCH0 (bits 15..8)
	 */
	if (((rPMGR_SCRATCH0 >> 11) & 1) == 0) {
		return platform_get_dock_connect();
	}
	else 
#endif	
	{
#if WITH_HW_USBPHY
		return usbphy_is_cable_connected();
#else
		return false;
#endif
	}
}

void platform_set_dfu_status(bool dfu)
{
	gpio_write(GPIO_DFU_STATUS, dfu);
}

bool platform_get_force_dfu(void)
{
	return gpio_read(GPIO_FORCE_DFU);
}

bool platform_get_request_dfu1(void)
{
	return !gpio_read(GPIO_REQUEST_DFU1);
}

bool platform_get_request_dfu2(void)
{
	return !gpio_read(GPIO_REQUEST_DFU2);
}

bool platform_get_dock_connect(void)
{
#if CONFIG_FPGA || CONFIG_SIM
	return true;
#else
	return gpio_read(GPIO_DOCK_CONNECT);
#endif
}

void platform_set_dock_attention(bool value)
{
	gpio_write(GPIO_DOCK_ATTENTION, value);
}

int platform_translate_key_selector(uint32_t key_selector, uint32_t *key_opts)
{
	bool production = platform_get_current_production_mode();

	switch (key_selector) {
		case IMAGE_KEYBAG_SELECTOR_PROD :
			if (!production) return -1;
			break;

		case IMAGE_KEYBAG_SELECTOR_DEV :
			if (production) return -1;
			break;

		default :
			return -1;
	}

	*key_opts = AES_KEY_TYPE_GID0 | AES_KEY_SIZE_256;

	return 0;
}

bool platform_set_usb_brick_detect(int select)
{
#if WITH_HW_USBPHY
	return usbphy_set_dpdm_monitor(select);
#else
	return false;
#endif
}

void platform_disable_keys(uint32_t gid, uint32_t uid)
{
	uint32_t key_disable = (((gid & 0x1) << 1) | ((uid & 0x3) << 2));

	dprintf(DEBUG_SPEW, "gid:0x%08x uid:0x%08x\n", gid, uid);

	if (!gid && !uid) return;

	rPMGR_SECURITY = key_disable;
}

bool platform_keys_disabled(uint32_t gid, uint32_t uid)
{
	uint32_t key_disabled;

	dprintf(DEBUG_SPEW, "gid:0x%08x uid:0x%08x\n", gid, uid);

	if (!gid && !uid) return true;

	key_disabled = rPMGR_SECURITY;

	return (key_disabled & (((gid & 0x1) << 1) | ((uid & 0x3) << 2)));
}

void platform_demote_production()
{
	chipid_clear_production_mode();
}

#if APPLICATION_IBOOT
uint64_t platform_get_memory_size(void)
{
	uint64_t	memory_size;

	// If rPMGR_SCRATCH0[2] set then the memory was inited, we have memory size info
	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagMemoryInfo) != 0) {
		memory_size = (rPMGR_SCRATCH13 & 0xffff) * 1024 * 1024;
	}
	else {
		panic("memory not yet inited\n");
	}

	return memory_size;
}

uint8_t platform_get_memory_manufacturer_id(void)
{
	// If rPMGR_SCRATCH0[2] set then the memory was inited, we have memory vendor-id info
	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagMemoryInfo) != 0) {
		return ((rPMGR_SCRATCH13 >> 28) & 0xf);
	}
	else {
		panic("memory not yet inited\n");
	}
}

void platform_set_memory_info(uint8_t manuf_id, uint64_t memory_size)
{
	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagMemoryInfo) == 0) {
		rPMGR_SCRATCH13 = 0;
	}

	rPMGR_SCRATCH13 = (manuf_id << 28) | (memory_size  & 0xffff);
	rPMGR_SCRATCH0 |= kPlatformScratchFlagMemoryInfo;
}
#endif

int32_t platform_get_boot_manifest_hash(uint8_t *boot_manifest_hash)
{
	RELEASE_ASSERT(boot_manifest_hash != NULL);

	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagObjectManifestHashValid) != 0) {
		((uint32_t *)boot_manifest_hash)[0] = rPMGR_SCRATCH16;
		((uint32_t *)boot_manifest_hash)[1] = rPMGR_SCRATCH17;
		((uint32_t *)boot_manifest_hash)[2] = rPMGR_SCRATCH18;
		((uint32_t *)boot_manifest_hash)[3] = rPMGR_SCRATCH19;
		((uint32_t *)boot_manifest_hash)[4] = rPMGR_SCRATCH20;

		return 0;
	}

	return -1;
}

int32_t platform_set_boot_manifest_hash(const uint8_t *boot_manifest_hash)
{
	if(boot_manifest_hash != NULL) {
		rPMGR_SCRATCH16 = ((uint32_t *)boot_manifest_hash)[0];
		rPMGR_SCRATCH17 = ((uint32_t *)boot_manifest_hash)[1];
		rPMGR_SCRATCH18 = ((uint32_t *)boot_manifest_hash)[2];
		rPMGR_SCRATCH19 = ((uint32_t *)boot_manifest_hash)[3];
		rPMGR_SCRATCH20 = ((uint32_t *)boot_manifest_hash)[4];

		rPMGR_SCRATCH0 |= kPlatformScratchFlagObjectManifestHashValid;
	}
	else {
		rPMGR_SCRATCH16 = 0;
		rPMGR_SCRATCH17 = 0;
		rPMGR_SCRATCH18 = 0;
		rPMGR_SCRATCH19 = 0;
		rPMGR_SCRATCH20 = 0;

		rPMGR_SCRATCH0 &= ~kPlatformScratchFlagObjectManifestHashValid;
	}

	return 0;
}

bool platform_get_mix_n_match_prevention_status(void)
{
	return ((rPMGR_SCRATCH0 & kPlatformScratchFlagVerifyManifestHash) ? true : false);
}

void platform_set_mix_n_match_prevention_status(bool mix_n_match_prevented)
{
	if (mix_n_match_prevented)
		rPMGR_SCRATCH0 |= kPlatformScratchFlagVerifyManifestHash;
	else
		rPMGR_SCRATCH0 &= ~kPlatformScratchFlagVerifyManifestHash;
}

void platform_set_consistent_debug_root_pointer(uint32_t root)
{
	rPMGR_SCRATCH7 = root;
}

uintptr_t platform_get_memory_region_base_optional(memory_region_type_t region)
{
	uintptr_t base;

	switch (region) {
		case kMemoryRegion_Panic:
			base = PANIC_BASE;
			break;

		case kMemoryRegion_StorageProcessor:
			base = ASP_BASE;
			break;

		case kMemoryRegion_Kernel:
			base = SDRAM_BASE;
			break;

		case kMemoryRegion_Heap:
			base = HEAP_BASE;
			break;

#if APPLICATION_IBOOT
		case kMemoryRegion_SleepToken:
			base = SLEEP_TOKEN_BUFFER_BASE;
			break;

		case kMemoryRegion_Display:
			base = PANIC_BASE - platform_get_memory_region_size(kMemoryRegion_Display);
			break;

		case kMemoryRegion_iBoot:
			base = IBOOT_BASE;
			break;
#endif

		default:
			base = (uintptr_t)-1;
			break;
	}

	return base;
}

size_t platform_get_memory_region_size_optional(memory_region_type_t region)
{
	size_t size;

	switch (region) {
		case kMemoryRegion_Panic:
			size = PANIC_SIZE;
			break;

		case kMemoryRegion_StorageProcessor:
			size = ASP_SIZE;
			break;

		case kMemoryRegion_Kernel:
			size = platform_get_memory_region_base(kMemoryRegion_Display) - platform_get_memory_region_base(kMemoryRegion_Kernel);
			break;

		case kMemoryRegion_Heap:
			size = HEAP_SIZE;
			break;


#if APPLICATION_IBOOT
		case kMemoryRegion_SleepToken:
			size = SLEEP_TOKEN_BUFFER_SIZE;
			break;

		case kMemoryRegion_Display:
			size = platform_get_display_memory_size();
			ASSERT(size != 0);
			break;

		case kMemoryRegion_iBoot:
			size = IBOOT_SIZE;
			break;
#endif

		default:
			size = (size_t)-1;
			break;
	}

	return size;
}

void platform_dockfifo_access_enable(uint32_t enable_flags)
{
	uintptr_t dbgfifo_access_reg = (SPU_DBG_WRAP_BASE_ADDR + 0x1094);	// DEBUG_FIFO_ACCESS_ENABLE register

	uint32_t curr_value = (*(volatile uint32_t *)dbgfifo_access_reg);

	dprintf(DEBUG_SPEW, "platform_dockfifo_access_enable: dbgfifo_access_reg:0x%08x, requested:0x%08x\n", curr_value, enable_flags);

	if (curr_value)
		panic("DEBUG_FIFO_ACCESS_ENABLE already nonzero (0x%08x), this can cause a hang due to <rdar://problem/18708180>", curr_value);

	(*(volatile uint32_t *)dbgfifo_access_reg) = enable_flags;

	dprintf(DEBUG_SPEW, "platform_dockfifo_access_enable: finished, dbgfifo_access_reg:0x%08x \n", (*(volatile uint32_t *)dbgfifo_access_reg));
}

#if WITH_BOOT_TRAMPOLINE
extern uintptr_t boot_handoff_trampoline;

void *platform_get_boot_trampoline(void)
{
	return (void *)&boot_handoff_trampoline;
}
#endif /* WITH_BOOT_TRAMPOLINE */

static void platform_init_boot_strap(void)
{
	uint32_t boot_strap, chip_board_id, gpio_board_id, boot_config;

	// If rPMGR_SCRATCH0[0] set then boot strap already valid
	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagBootStrap) != 0) return;

	gpio_configure(GPIO_BOARD_ID0, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_ID1, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_ID2, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_ID3, GPIO_CFG_IN);
	gpio_configure_pupdn(GPIO_BOARD_ID0, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOARD_ID1, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOARD_ID2, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOARD_ID3, GPIO_PDN);

	gpio_configure(GPIO_BOOT_CONFIG0, GPIO_CFG_IN);
	gpio_configure(GPIO_BOOT_CONFIG1, GPIO_CFG_IN);
	gpio_configure(GPIO_BOOT_CONFIG2, GPIO_CFG_IN);
	gpio_configure(GPIO_BOOT_CONFIG3, GPIO_CFG_IN);
	gpio_configure_pupdn(GPIO_BOOT_CONFIG0, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOOT_CONFIG1, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOOT_CONFIG2, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOOT_CONFIG3, GPIO_PDN);

	platform_power_spin(100); // Wait 100us

	chip_board_id = chipid_get_board_id();

	gpio_board_id =
		(gpio_read(GPIO_BOARD_ID3) << 3) |
		(gpio_read(GPIO_BOARD_ID2) << 2) |
		(gpio_read(GPIO_BOARD_ID1) << 1) |
		(gpio_read(GPIO_BOARD_ID0) << 0);

	boot_config =
		(gpio_read(GPIO_BOOT_CONFIG3) << 3) |
		(gpio_read(GPIO_BOOT_CONFIG2) << 2) |
		(gpio_read(GPIO_BOOT_CONFIG1) << 1) |
		(gpio_read(GPIO_BOOT_CONFIG0) << 0);

	gpio_configure(GPIO_BOARD_ID0, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_ID1, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_ID2, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_ID3, GPIO_CFG_DFLT);

	gpio_configure(GPIO_BOOT_CONFIG0, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOOT_CONFIG1, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOOT_CONFIG2, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOOT_CONFIG3, GPIO_CFG_DFLT);

	boot_strap = (((chip_board_id << 4) | gpio_board_id) << 16) |
		     (boot_config << 8) |
		     (0x01 << 0);

	rPMGR_SCRATCH0 = (rPMGR_SCRATCH0 & 0xFF000000) | (boot_strap & 0x00FFFFFF);
}

static void platform_configure_soc_reconfig_ram()
{
	// <rdar://problem/15805210> M7: ANP PHY reset at chipPads power-up (STR)
	reconfig_append_command(RECONFIG_TYPE_SOC, (addr_t)(&rPMGR_ANS_PS), 0xF, 0x0);
	reconfig_append_command(RECONFIG_TYPE_SOC, ((addr_t)(&rPMGR_ANS_PS)) | RECONFIG_RAM_CMD_READ, (0xF << 4), (0xF << 4));

	// Set remap to SDRAM
	reconfig_append_command(RECONFIG_TYPE_SOC, (addr_t)(&rFABRIC_REMAP_REG), 1, 0x0);

	// Reconfigure GPIO block - reg GPIO_GPIO_NPL_IN_EN, set bit 0 (GPIO_NPL_IN_EN)
	reconfig_append_command(RECONFIG_TYPE_SOC, 0x47100c48, 1, 0x0);

	// Reconfigure static SoC bridge tunables.
	miu_configure_bridge_soc_reconfig_ram();
}

static void platform_configure_akf_reconfig_ram()
{
	reconfig_append_command(RECONFIG_TYPE_AKF, (addr_t) &rSPU_AKF_AXI_BASE, SPU_SRAM_A_BASE_ADDR, 0);
	reconfig_append_command(RECONFIG_TYPE_AKF, (addr_t) &rSPU_AKF_AXI_BASE_EXT, 0, 0);
	reconfig_append_command(RECONFIG_TYPE_AKF, (addr_t) &rSPU_AKF_AXI_END, SPU_SRAM_SIZE, 0);
	reconfig_append_command(RECONFIG_TYPE_AKF, (addr_t) &rSPU_AKF_AXI_EN, 1, 0);
	// SPU software to re-enable gating
	reconfig_append_command(RECONFIG_TYPE_AKF, (addr_t) &rSPU_AKF_AXI_IDLE_CTRL, 0, 0);
	reconfig_append_command(RECONFIG_TYPE_AKF, (addr_t) &rSPU_AKF_AXI_CPU_CTRL, SP_AKF_AXI_CPU_CTRL_RUN, 0);
}

static void platform_bootprep_darwin()
{

	// Configure MEM config

	// Ugliness is <rdar://problem/15805298> M7: ReconfigMem and PerfState Change
	//
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t)&rPMGR_SOC_PERF_STATE_CTL, 0x3, 0);

	amc_configure_mem_reconfig_ram();

#if SUPPORT_FPGA
	// enable, 24 MHz
	uint32_t mcu_fixed_clk_cfg = (1 << 31) | (0 << 24) | (1 << 20);
	// enable, 24 MHz, cfg_sel = 3
	uint32_t mcu_clk_cfg = (1 << 31) | (0 << 24) | (1 << 20) | (3 << 16);
#else
	// enable, 533 MHz
	uint32_t mcu_fixed_clk_cfg = (1 << 31) | (1 << 24) | (1 << 20);
	// enable, 533 MHz, cfg_sel = 0
	uint32_t mcu_clk_cfg = (1 << 31) | (3 << 24) | (1 << 20) | (0 << 16);
#endif
	
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t)&rPMGR_MCU_FIXED_CLK_CFG, mcu_fixed_clk_cfg, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t)&rPMGR_MCU_FIXED_CLK_CFG) | RECONFIG_RAM_CMD_READ, 0, (1 << 30));
	
	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t)&rPMGR_MCU_CLK_CFG, mcu_clk_cfg, 0);
	reconfig_append_command(RECONFIG_TYPE_MEM, ((addr_t)&rPMGR_MCU_CLK_CFG) | RECONFIG_RAM_CMD_READ, 0, (1 << 30));

	reconfig_append_command(RECONFIG_TYPE_MEM, (addr_t)&rPMGR_SOC_PERF_STATE_CTL, 0x7, 0);

	// Configure SOC config
	platform_configure_soc_reconfig_ram();

	// Configure AKF config
	platform_configure_akf_reconfig_ram();

	reconfig_commit();

	// Apply reconfig timeout tunables
	rRECONFIG_TIMEOUT_AKF = 0x1b71b00;
	rRECONFIG_TIMEOUT_SOC = 0x1b71b00;
	rRECONFIG_TIMEOUT_MEM = 0x1b71b00;
}

#if defined(WITH_MENU) && WITH_MENU

static int do_m7_str_test(int argc, struct cmd_arg *args) __noreturn;

static int do_m7_str_test(int argc, struct cmd_arg *args)
{
	dprintf(DEBUG_INFO, "starting M7 STR test...\n");

	// Turn following PS off
	*(volatile uint32_t *)0x46c20088 = 0;
	*(volatile uint32_t *)0x46c20068 = 0;
	*(volatile uint32_t *)0x46c20060 = 0;
	*(volatile uint32_t *)0x46c20070 = 0;
	*(volatile uint32_t *)0x46c20080 = 0;
	*(volatile uint32_t *)0x46c20120 = 0;
	*(volatile uint32_t *)0x46c20128 = 0;
	*(volatile uint32_t *)0x46c20130 = 0;
	*(volatile uint32_t *)0x46c20138 = 0;
	*(volatile uint32_t *)0x46c20090 = 0;
	*(volatile uint32_t *)0x46c200c8 = 0;
	*(volatile uint32_t *)0x46c200d0 = 0;

	// Enable auto power gating for MCU, AMP, PIOSYS, LIO
	*(volatile uint32_t *)0x46c20048 |= (1 << 28);
	*(volatile uint32_t *)0x46c20050 |= (1 << 28);
	*(volatile uint32_t *)0x46c20058 |= (1 << 28);
	*(volatile uint32_t *)0x46c20098 |= (1 << 28);

	// Enable power gating for AMCPIO domain
	*(volatile uint32_t *)0x46c22010 |= (1 << 31);

	// Program perf-state table
	*(volatile uint32_t *)0x46c1a010 = (0<<8)|(1<<7)|(3<<4);
	*(volatile uint32_t *)0x46c1a020 = (3<<8)|(0<<7)|(0<<4);
	*(volatile uint32_t *)0x46c1a030 = (3<<8)|(0<<7)|(0<<4);
	*(volatile uint32_t *)0x46c1a000 = 0x7;	// enable all perf states

	// Enable global ACG
	*(volatile uint32_t *)0x46c32000 = 1;

	// MEM config
	*(volatile uint32_t *)0x46901000 = 0x46030000;
	*(volatile uint32_t *)0x46902000 = 0xfefefefe;
	*(volatile uint32_t *)0x46901004 = 0x46030004;
	*(volatile uint32_t *)0x46902004 = 0xfefefefe;

	// SOC config
	*(volatile uint32_t *)0x46901010 = 0x46030010;
	*(volatile uint32_t *)0x46902010 = 0xfefefefe;
	*(volatile uint32_t *)0x46901014 = 0x46030014;
	*(volatile uint32_t *)0x46902014 = 0xfefefefe;

	// AKF config
	*(volatile uint32_t *)0x46901020 = 0x46030020;
	*(volatile uint32_t *)0x46902020 = 0xfefefefe;
	*(volatile uint32_t *)0x46901024 = 0x46030024;
	*(volatile uint32_t *)0x46902024 = 0xfefefefe;

	// Setup and enable Reconfig 
	*(volatile uint32_t *)0x46900004 = (2<<16);
	*(volatile uint32_t *)0x46900010 = (2<<16 | 0x4);
	*(volatile uint32_t *)0x4690001C = (2<<16 | 0x8);
	*(volatile uint32_t *)0x46900000 |= 1<<16;

	// Unmask wakeup sources
	*(volatile uint32_t *)0x46706000 = 0xffffffff;
	*(volatile uint32_t *)0x46707000 = 0;
	*(volatile uint32_t *)0x46707004 = 0x8;
	*(volatile uint32_t *)0x46707010 = 0x1;
	
	// // Confirm debugger is not preventing power down
	// dprintf(DEBUG_INFO, "Confirming CSYSWRUP: %08x\n", *(volatile uint32_t *)0x46eb1030);
	// ASSERT(*(volatile uint32_t *)0x46eb1030 == 0);
	// 
	// Capture current time (SCM system time)
	time_t current_time = *(volatile uint32_t *)0x46705208;
	dprintf(DEBUG_INFO, "starting to STR.  time %ld..\n", current_time);
	current_time = *(volatile uint32_t *)0x46705208;
	
	// Program wake up time
	*(volatile uint32_t *)0x46705210 = (current_time + 0x10000);
	*(volatile uint32_t *)0x46705214 = 0x80000000;

	dprintf(DEBUG_INFO, "into STR\n");

	// Enable STR sequence
	*(volatile uint32_t *)0x46c27308 = 1;

	while(1);
}

MENU_COMMAND_DEBUG(m7_str, do_m7_str_test, "M7 Suspend to RAM test", NULL);


static int do_m7_str_dram_test(int argc, struct cmd_arg *args) __noreturn;

static int do_m7_str_dram_test(int argc, struct cmd_arg *args)
{
	dprintf(DEBUG_INFO, "starting M7 STR DRAM test...\n");

	platform_bootprep_darwin();

#if WITH_HW_MIU
	miu_select_remap(REMAP_SDRAM);
#endif

	/* make sure timers and whatnot aren't running */
	platform_quiesce_hardware(BOOT_IBOOT);

	// Turn following PS off
	*(volatile uint32_t *)0x46c20088 = 0;
	*(volatile uint32_t *)0x46c20068 = 0;
	*(volatile uint32_t *)0x46c20060 = 0;
	*(volatile uint32_t *)0x46c20070 = 0;
	*(volatile uint32_t *)0x46c20080 = 0;
	*(volatile uint32_t *)0x46c20120 = 0;
	*(volatile uint32_t *)0x46c20128 = 0;
	*(volatile uint32_t *)0x46c20130 = 0;
	*(volatile uint32_t *)0x46c20138 = 0;
	*(volatile uint32_t *)0x46c20090 = 0;
	*(volatile uint32_t *)0x46c200c8 = 0;
	*(volatile uint32_t *)0x46c200d0 = 0;

	// Enable auto power gating for MCU, AMP, PIOSYS, LIO
	*(volatile uint32_t *)0x46c20048 |= (1 << 28);
	*(volatile uint32_t *)0x46c20050 |= (1 << 28);
	*(volatile uint32_t *)0x46c20058 |= (1 << 28);
	*(volatile uint32_t *)0x46c20098 |= (1 << 28);

	// Enable power gating for AMCPIO domain
	*(volatile uint32_t *)0x46c22010 |= (1 << 31);

	*(volatile uint32_t *)0x46c22080 |= (1 << 31);
	*(volatile uint32_t *)0x46c22050 |= (1 << 31);
	*(volatile uint32_t *)0x46c22090 |= (1 << 31);
	*(volatile uint32_t *)0x46c22000 |= (1 << 31);
	

	// Program perf-state table
	*(volatile uint32_t *)0x46c1a010 = (0<<8)|(1<<7)|(3<<4);
	*(volatile uint32_t *)0x46c1a020 = (3<<8)|(0<<7)|(0<<4);
	*(volatile uint32_t *)0x46c1a030 = (3<<8)|(0<<7)|(0<<4);
	*(volatile uint32_t *)0x46c1a000 = 0x7;	// enable all perf states

	// Enable global ACG
	*(volatile uint32_t *)0x46c32000 = 1;

	// timeout in Reconfig
	// *(volatile uint32_t *)0x46900014 = 0x186A0 | (1<<24);
	// *(volatile uint32_t *)0x46900008 = 0x186A0 | (1<<24);

	// Unmask wakeup sources
	*(volatile uint32_t *)0x46706000 = 0xffffffff;
	*(volatile uint32_t *)0x46707000 = 0;
	*(volatile uint32_t *)0x46707004 = 0x8;
	*(volatile uint32_t *)0x46707010 = 0x1;
	
	// // Confirm debugger is not preventing power down
	// dprintf(DEBUG_INFO, "Confirming CSYSWRUP: %08x\n", *(volatile uint32_t *)0x46eb1030);
	// ASSERT(*(volatile uint32_t *)0x46eb1030 == 0);
	
	// Capture current time (SCM system time)
	time_t current_time = *(volatile uint32_t *)0x46705208;
	dprintf(DEBUG_INFO, "starting to STR.  time %ld..\n", current_time);
	current_time = *(volatile uint32_t *)0x46705208;
	
	// Program wake up time
	*(volatile uint32_t *)0x46705210 = (current_time + 0x10000);
	*(volatile uint32_t *)0x46705214 = 0x80000000;

	dprintf(DEBUG_INFO, "into STR\n");

	// Enable STR sequence
	*(volatile uint32_t *)0x46c27304 = 0x5DC0;	// 1ms
	*(volatile uint32_t *)0x46c27308 = 1;

	while(1);
}

MENU_COMMAND_DEBUG(m7_str_dram, do_m7_str_dram_test, "M7 Suspend to RAM DRAM test", NULL);

#endif
