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

#include <arch/arm/arm.h>
#include <debug.h>
#include <drivers/aes.h>
#include <drivers/asp.h>
#include <drivers/csi.h>
#include <drivers/anc_boot.h>
#if WITH_HW_DOCKCHANNEL_UART
#include <drivers/dockchannel/dockchannel.h>
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
#if WITH_HW_SEP
#include <drivers/sep/sep_client.h>
#endif
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
#include <platform/soc/aop_dma.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/chipid.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
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
	{ GPIO_NAND_CEN0, GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_CEN0
	{ GPIO_NAND_CEN1, GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_CEN1
	{ GPIO_NAND_CLE,  GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_CLE
	{ GPIO_NAND_ALE,  GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_ALE
	{ GPIO_NAND_REN,  GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_REN
	{ GPIO_NAND_WEN,  GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_WEN
	{ GPIO_NAND_IO0,  GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO0
	{ GPIO_NAND_IO1,  GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO1
	{ GPIO_NAND_IO2,  GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO2
	{ GPIO_NAND_IO3,  GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO3
	{ GPIO_NAND_IO4,  GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO4
	{ GPIO_NAND_IO5,  GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO5
	{ GPIO_NAND_IO6,  GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// NAND_IO6
	{ GPIO_NAND_IO7,  GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// NAND_IO7
};
#endif /* WITH_ANC_BOOT */

#if WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI
static const struct boot_interface_pin spi0_boot_interface_pins[] =
{
	{ GPIO_SPI0_SSIN, GPIO_CFG_OUT_1, GPIO_CFG_DFLT },	// SPI0_SSIN
	{ GPIO_SPI0_SCLK, GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_SCLK
	{ GPIO_SPI0_MOSI, GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_MOSI
	{ GPIO_SPI0_MISO, GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// SPI0_MISO
};
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */

static void platform_init_boot_strap(void);
static void platform_bootprep_darwin(void);

int platform_early_init(void)
{
#if APPLICATION_IBOOT && (PRODUCT_LLB || PRODUCT_IBSS)
	/* Verify that the fuses and SecureROM R/W access has been disabled */
	if (!chipid_get_fuse_lock() || (((*(volatile uint32_t *)SECURITY_REG) & ROM_READ_DISABLE) == 0)) {
		panic("Fuses are unlocked or SecureROM is enabled\n");
	}

	/* Verify that the fuses are sealed on production devices. */
	if (chipid_get_current_production_mode() && !chipid_get_fuse_seal()) {
		panic("Fuses are not sealed\n");
	}
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

#if WITH_HW_DOCKCHANNEL_UART
	/* do whatever uart initialization we need to get a simple console */
	dockchannel_uart_init();
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

#if (PRODUCT_LLB || PRODUCT_IBSS) && WITH_HW_DOCKCHANNEL_UART
	// Enable all DockFIFO's used by platform in LLB (<rdar://problem/18708180>)
	dockchannel_access_enable((1) | (1 << 4));
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
			/* even when trusted, Darwin only gets the UID / GID1 */
			uids = 1;
			gids = 2;
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
			panic("Monitor not supported");
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
#if WITH_HW_SPI
	spi_init();
#endif

#if WITH_HW_ASP && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)
  csi_init(CSI_COPROC_ANS);   /* coproc switchboard used by nand driver to communicate with ans iop */
  asp_init();
#endif

#if WITH_ANC_FIRMWARE
      anc_firmware_init();
#endif

#if WITH_TARGET_CONFIG
	target_init();
#endif

	return 0;
}

int platform_debug_init(void)
{
#if WITH_HW_USB
	uint32_t usb_enabled = 1;
#if WITH_ENV && SUPPORT_FPGA
	usb_enabled = env_get_uint("usb-enabled", 1);
#endif
	if (usb_enabled) usb_init();
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
		//miu_update_device_tree(node);
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

	boot_config = (rPMGR_SCRATCH0 >> 8) & 0xFF;

	return boot_config;
}

bool platform_get_boot_device(int32_t index, enum boot_device *boot_device, uint32_t *boot_flag, uint32_t *boot_arg)
{
		uint32_t boot_config = platform_get_boot_config();

		/* T8002 supports one boot device then USB-DFU per boot config */

		/* If the index is not zero force DFU mode */
		if (index != 0) index = 1;

		switch (boot_config) {
			case  BOOT_CONFIG_SPI0:
				*boot_device = BOOT_DEVICE_SPI;
				*boot_flag = 0;
				*boot_arg = 0;
				break;

			case  BOOT_CONFIG_SPI0_TEST:
			case  BOOT_CONFIG_SLOW_SPI0_TEST:
				*boot_device = BOOT_DEVICE_SPI;
				*boot_flag = BOOT_FLAG_TEST_MODE;
				*boot_arg = 0;
				break;

			case  BOOT_CONFIG_ANS:
				*boot_device = BOOT_DEVICE_NAND;
				*boot_flag = 0;
				*boot_arg = 0;
				break;

			case  BOOT_CONFIG_ANS_TEST:
				*boot_device = BOOT_DEVICE_NAND;
				*boot_flag = BOOT_FLAG_TEST_MODE;
				*boot_arg = 0;
				break;

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

		rPMGR_SCRATCH_BOOT_NONCE_0 = nonce_words[0];
		rPMGR_SCRATCH_BOOT_NONCE_1 = nonce_words[1];

		rPMGR_SCRATCH0 |= kPlatformScratchFlagNonce;
	} else {
		nonce_words[0] = rPMGR_SCRATCH_BOOT_NONCE_0;
		nonce_words[1] = rPMGR_SCRATCH_BOOT_NONCE_1;
	}

	return nonce;
}

int32_t platform_get_sep_nonce(uint8_t *nonce)
{
#if WITH_HW_SEP
	return sep_client_get_nonce(nonce);
#else
	return -1;
#endif
}

bool platform_get_ecid_image_personalization_required(void)
{
	return true;
}

uint32_t platform_get_osc_frequency(void)
{
	return chipid_get_osc_frequency();
}

uint32_t platform_get_spi_frequency(void)
{
#if SUPPORT_FPGA
	return 2500000;
#else
	switch (platform_get_boot_config()) {
		case BOOT_CONFIG_SLOW_SPI0_TEST:
			return 6000000;

		default:
			return 12000000;
	}
#endif
}

bool platform_get_usb_cable_connected(void)
{
#if WITH_HW_USBPHY
		return usbphy_is_cable_connected();
#else
		return false;
#endif
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
	uint32_t key_disable = (((gid & 0x3) << 1) | ((uid & 0x1) << 0));

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

	return (key_disabled & (((gid & 0x3) << 1) | ((uid & 0x1) << 0)));
}

void platform_demote_production()
{
	chipid_clear_production_mode();
}

#if APPLICATION_IBOOT
bool platform_is_pre_lpddr4(void)
{
	return false;
}

uint64_t platform_get_memory_size(void)
{
	uint64_t	memory_size;

	// If rPMGR_SCRATCH0[2] set then the memory was inited, we have memory size info
	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagMemoryInfo) != 0) {
		memory_size = (((uint64_t)rPMGR_SCRATCH_MEM_INFO) & 0xffff) * 1024 * 1024;
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
		return ((rPMGR_SCRATCH_MEM_INFO >> 24) & 0xff);
	}
	else {
		panic("memory not yet inited\n");
	}
}


void platform_set_memory_info_with_revids(uint8_t manuf_id, uint64_t memory_size, uint8_t rev_id, uint8_t rev_id2)
{
	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagMemoryInfo) == 0) {
		rPMGR_SCRATCH_MEM_INFO = 0;
	}

	rPMGR_SCRATCH_MEM_INFO = (manuf_id << 24) | ((rev_id2 & 0xF) << 20) | ((rev_id & 0xF) << 16) | (memory_size  & 0xffff);
	rPMGR_SCRATCH0 |= kPlatformScratchFlagMemoryInfo;
}

void platform_get_memory_rev_ids(uint8_t *rev_id, uint8_t *rev_id2)
{
	// If rPMGR_SCRATCH0[2] set then the memory was inited, we have memory rev_id info
	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagMemoryInfo) == 0)
		panic("memory not yet inited\n");
	else {
		if (rev_id && rev_id2) {
			uint8_t rev_info = (rPMGR_SCRATCH_MEM_INFO >> 16) & 0xFF;
			*rev_id = rev_info & 0xF;
			*rev_id2 = rev_info >> 4;
		}
	}
}
#endif

int32_t platform_get_boot_manifest_hash(uint8_t *boot_manifest_hash)
{
	volatile uint32_t *pScratch;
	uint32_t *pHash;

	RELEASE_ASSERT(boot_manifest_hash != NULL);
	ASSERT((&rPMGR_SCRATCH_BOOT_MANIFEST_HASH_LAST - &rPMGR_SCRATCH_BOOT_MANIFEST_HASH_FIRST + 1)
		== PMGR_SCRATCH_BOOT_MANIFEST_REGISTERS);

	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagObjectManifestHashValid) != 0) {
		pHash    = &((uint32_t *)boot_manifest_hash)[0];
		pScratch = &rPMGR_SCRATCH_BOOT_MANIFEST_HASH_FIRST;

		// Copy the boot manifest hash from the scratch registers.
		while (pScratch <= &rPMGR_SCRATCH_BOOT_MANIFEST_HASH_LAST) {
			*pHash++ = *pScratch++;
		}
		return 0;
	}

	return -1;
}

int32_t platform_set_boot_manifest_hash(const uint8_t *boot_manifest_hash)
{
	volatile uint32_t *pScratch = &rPMGR_SCRATCH_BOOT_MANIFEST_HASH_FIRST;
	uint32_t *pHash;

	ASSERT((&rPMGR_SCRATCH_BOOT_MANIFEST_HASH_LAST - &rPMGR_SCRATCH_BOOT_MANIFEST_HASH_FIRST + 1)
		== PMGR_SCRATCH_BOOT_MANIFEST_REGISTERS);

	if (boot_manifest_hash != NULL) {
		pHash = &((uint32_t *)boot_manifest_hash)[0];

		// Copy the boot manifest hash to the scratch registers.
		while (pScratch <= &rPMGR_SCRATCH_BOOT_MANIFEST_HASH_LAST) {
			*pScratch++ = *pHash++;
		}

		rPMGR_SCRATCH0 |= kPlatformScratchFlagObjectManifestHashValid;
	} else {
		// Clear the boot manifest hash in the scratch registers.
		while (pScratch <= &rPMGR_SCRATCH_BOOT_MANIFEST_HASH_LAST) {
			*pScratch++ = 0;
		}
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

#define MAX_DMA_TRANSFER_SIZE		0xFF0

// Use AOP PL080 DMA to clear out insecure memory. We use this instead
// of bzero to improve boot time. The AOP PL080 avoids PIO from AP and
// saves us time on PIO latency. The DMA source address is fixed and 
// destination address is incremented a word at a time.
// rdar://problem/19986351
void platform_clear_mem_with_dma(char *dst, size_t count)
{
	uint32_t src = 0;
	uint32_t transfer_size = 0;
	uint32_t increment = 0;
	// increasing burst size further doesnt improve speed due to pl080's poor utilization of the bus
	// use bufferable attribute
	const uint32_t control_c0 = AOP_PL080_BLK_DMAC_C0_CONTROL_SRC_BURST_SIZE_INSRT(0x1) | 
				    AOP_PL080_BLK_DMAC_C0_CONTROL_DST_BURST_SIZE_INSRT(0x1) |
				    AOP_PL080_BLK_DMAC_C0_CONTROL_SRC_XFER_WIDTH_INSRT(2) |
				    AOP_PL080_BLK_DMAC_C0_CONTROL_DST_XFER_WIDTH_INSRT(2) |
				    AOP_PL080_BLK_DMAC_C0_CONTROL_SRC_MST_SEL_INSRT(0) |
				    AOP_PL080_BLK_DMAC_C0_CONTROL_DST_MST_SEL_INSRT(1) |
				    AOP_PL080_BLK_DMAC_C0_CONTROL_SRC_INCR_INSRT(0) |
				    AOP_PL080_BLK_DMAC_C0_CONTROL_DST_INCR_INSRT(1) |
				    AOP_PL080_BLK_DMAC_C0_CONTROL_PROTECTION_INSRT(2);

	// clean by mva the src location
	platform_cache_operation(CACHE_CLEAN, (void *)((uint32_t)&src & ~(CPU_CACHELINE_SIZE-1)), CPU_CACHELINE_SIZE);

	// Enable DMAC
	rAOP_PL080_CONFIGURATION = AOP_PL080_BLK_DMAC_CONFIGURATION_DMAC_ENABLE_INSRT(1);

	// Clear interrupts
	rAOP_PL080_INTTCCLEAR = AOP_PL080_BLK_DMAC_INTTCCLEAR_INT_TC_CLEAR_INSRT(0xff);
	rAOP_PL080_INTERRCLR = AOP_PL080_BLK_DMAC_INTERRCLR_INT_ERR_CLR_INSRT(0xff);

	// Set source and destination address 
	rAOP_PL080_C0_SRCADDR = (uint32_t)&src;
	rAOP_PL080_C0_DESTADDR = (uint32_t)dst;

	// Dont use segments
	rAOP_PL080_C0_LLI = 0;			

	while (count) {
		// DMA size limited to 0xFFF * transfer width * burst size
		// In this case each transfer is of 4 bytes and burst size 1
		transfer_size = ((count/4) <= MAX_DMA_TRANSFER_SIZE) ? (count/4) : MAX_DMA_TRANSFER_SIZE;
		count -= (transfer_size * 4);

		// transfer width - 4 bytes, burst size - 1, source no increment, destination increment
		// transfer_size
		rAOP_PL080_C0_CONTROL = control_c0 | AOP_PL080_BLK_DMAC_C0_CONTROL_XFER_SIZE_INSRT(transfer_size); 

		// Enable the DMA channel. It gets disabled once the DMA completes.
		rAOP_PL080_C0_CONFIGURATION |= AOP_PL080_BLK_DMAC_C0_CONFIGURATION_CHANNEL_ENABLE_INSRT(0x1);

		while(AOP_PL080_BLK_DMAC_C0_CONFIGURATION_ACTIVE_XTRCT(rAOP_PL080_C0_CONFIGURATION)) {
		}

		// Once the DMA completes destination address is the last byte
		// read. So we need to move it forward to dst + transfer_size 
		// for the next DMA transfer
		increment += (transfer_size * 4);
		rAOP_PL080_C0_DESTADDR = (uint32_t)dst + increment;
	}

	arm_clean_invalidate_dcache();
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

		case kMemoryRegion_SecureProcessor:
			base = TZ0_BASE;
			break;

		case kMemoryRegion_Kernel:
			base = SDRAM_BASE;
			break;

		case kMemoryRegion_PageTables:
			base = MMU_TT_BASE;
			break;

		case kMemoryRegion_Heap:
			base = HEAP_BASE;
			break;

		case kMemoryRegion_Stacks:
			base = STACK_BASE;
			break;

#if APPLICATION_IBOOT
		case kMemoryRegion_ConsistentDebug:
			base = CONSISTENT_DEBUG_BASE;
			break;

		case kMemoryRegion_SleepToken:
			base = SLEEP_TOKEN_BUFFER_BASE;
			break;

		case kMemoryRegion_DramConfig:
			base = DRAM_CONFIG_SEQ_BASE;
			break;

		case kMemoryRegion_Display:
			base = PANIC_BASE - DISPLAY_SIZE;
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

		case kMemoryRegion_SecureProcessor:
			size = TZ0_SIZE;
			break;

		case kMemoryRegion_Kernel:
			size = DISPLAY_BASE - platform_get_memory_region_base(kMemoryRegion_Kernel);
			break;

		case kMemoryRegion_PageTables:
			size = MMU_TT_SIZE;
			break;

		case kMemoryRegion_Heap:
			size = (size_t)HEAP_SIZE;
			break;

		case kMemoryRegion_Stacks:
			size = STACK_SIZE;
			break;

#if APPLICATION_IBOOT
		case kMemoryRegion_ConsistentDebug:
			size = CONSISTENT_DEBUG_SIZE;
			break;

		case kMemoryRegion_SleepToken:
			size = SLEEP_TOKEN_BUFFER_SIZE;
			break;

		case kMemoryRegion_DramConfig:
			size = DRAM_CONFIG_SEQ_SIZE;
			break;

		case kMemoryRegion_Display:
			size = DISPLAY_SIZE;
			break;

		case kMemoryRegion_iBoot:
			size = (size_t)IBOOT_SIZE;
			break;
#endif

		default:
			size = (size_t)-1;
			break;
	}

	return size;
}

#if APPLICATION_IBOOT && (PRODUCT_LLB || PRODUCT_IBSS)
extern uintptr_t boot_handoff_trampoline;

void *platform_get_boot_trampoline(void)
{
	return (void *)&boot_handoff_trampoline;
}

#elif WITH_ROM_TRAMPOLINE

extern uint8_t boot_handoff_trampoline, boot_handoff_trampoline_end;

void *platform_get_boot_trampoline(void)
{
	size_t len = &boot_handoff_trampoline_end - &boot_handoff_trampoline;

	/* copy trampoline code */
	memcpy((void *)BOOT_TRAMPOLINE_BASE, &boot_handoff_trampoline, len);

	/* clean-invalidate TZ0 & TZ1 regions */
	platform_cache_operation((CACHE_CLEAN | CACHE_INVALIDATE), (void *)BOOT_TRAMPOLINE_BASE, BOOT_TRAMPOLINE_SIZE);

	return (void *)BOOT_TRAMPOLINE_BASE;
}
#endif

static void platform_init_boot_strap(void)
{
	volatile uint32_t boot_strap, chip_board_id, gpio_board_id, boot_config;

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

	gpio_configure_pupdn(GPIO_BOOT_CONFIG0, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOOT_CONFIG1, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOOT_CONFIG2, GPIO_PDN);

	platform_power_spin(100); // Wait 100us

	chip_board_id = chipid_get_board_id();

	gpio_board_id =
		(gpio_read(GPIO_BOARD_ID3) << 3) |
		(gpio_read(GPIO_BOARD_ID2) << 2) |
		(gpio_read(GPIO_BOARD_ID1) << 1) |
		(gpio_read(GPIO_BOARD_ID0) << 0);

	boot_config =
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

	boot_strap = (((chip_board_id << 4) | gpio_board_id) << 16) |
		     (boot_config << 8) |
		     (0x01 << 0);

	rPMGR_SCRATCH0 = (rPMGR_SCRATCH0 & 0xFF000000) | (boot_strap & 0x00FFFFFF);
}

static void platform_bootprep_darwin()
{
#if PRODUCT_IBOOT || PRODUCT_IBEC
	/* clean-invalidate TZ0 region */
	platform_cache_operation((CACHE_CLEAN | CACHE_INVALIDATE), (void *)TZ0_BASE, TZ0_SIZE);

	/* program values
	   Tz{base,end}addr are relative addresses from DRAM base
	*/

	rMCCLOCKREGION_TZ0BASEADDR = (TZ0_BASE - SDRAM_BASE) >> 12;
	rMCCLOCKREGION_TZ0ENDADDR  = (TZ0_BASE + TZ0_SIZE - SDRAM_BASE - 1) >> 12;

	/* Lock TZ0 region. */
	rMCCLOCKREGION_TZ0LOCK = (1 << 0);
	if ((rMCCLOCKREGION_TZ0LOCK & 1) == 0) {
		panic("TZ0 failed to lock\n");
	}
#endif
}

