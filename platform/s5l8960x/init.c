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
#include <arch.h>
#include <arch/arm/arm.h>
#include <debug.h>
#include <drivers/aes.h>
#include <drivers/anc_boot.h>
#include <drivers/asp.h>
#include <drivers/ccc/ccc.h>
#include <drivers/csi.h>
#include <drivers/display.h>
#if WITH_CONSISTENT_DBG
#include <drivers/consistent_debug.h>
#endif
#if WITH_HW_DISPLAY_PMU
#include <drivers/display_pmu.h>
#endif
#include <drivers/iic.h>
#include <drivers/miu.h>
#include <drivers/nvme.h>
#include <drivers/pci.h>
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
#include <lib/paint.h>
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
#if WITH_PLATFORM_ERROR_HANDLER
#include <platform/error_handler.h>
#endif
#include <platform/trampoline.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <target.h> 

static void platform_init_boot_strap(void);
static int32_t platform_bootprep_darwin(bool resume);
static void power_get_buck_value_fpga(int buck, uint32_t mv, uint32_t *val);
static int power_convert_dwi_to_mv_fpga(unsigned int buck, u_int32_t dwival);
static void platform_relocate_securerom(void);

static uint8_t boot_debug;
static bool gDisplayEnabled;

int platform_early_init(void)
{
#if PRODUCT_LLB || PRODUCT_IBSS
	/* Verify that the fuses and SecureROM R/W access has been disabled */
	if (!chipid_get_fuse_lock() || (((*(volatile u_int32_t *)SECURITY_REG) & ROM_READ_DISABLE) == 0))
		panic("Fuses are unlocked or SecureROM is enabled\n");
#endif

#if PRODUCT_IBSS || PRODUCT_LLB
	/*
	 * H6 B0 AP-ROM doesn't pass forward ObjectManifestHashValid Flag. Set it here.
	 * Object manifest hash passed forward by ROM is not valid for image with no manifest.
	 * ROM executed manifest-less image, if Mix-n-Match is allowed and keys are disabled.
	 */
	if (chipid_get_chip_revision() < CHIP_REVISION_C0) {
		bool mix_n_match_allowed = ((rPMGR_SCRATCH0 & kPlatformScratchFlagVerifyManifestHash) == 0);
		bool keys_disabled = platform_keys_disabled(~0, ~0);
		bool manifest_less_image = (mix_n_match_allowed && keys_disabled);

		if (!manifest_less_image)
			rPMGR_SCRATCH0 |= kPlatformScratchFlagObjectManifestHashValid;
	}

#endif

	/* Enable more Cyclone specific errors */
	ccc_enable_custom_errors();

#if WITH_HW_PLATFORM_POWER
	/* initialize the s5l8960x pmgr driver */
	platform_power_init();
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

#if WITH_SHM_CONSOLE
	shmcon_init();
#endif

	// Only B0 and onwards are supported
	if (platform_get_chip_revision() < CHIP_REVISION_B0) platform_not_supported();

	thermal_init();

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

	/* turn off DEBUG clock if debug-soc nvram is not true */
	clock_gate(CLK_DEBUG, env_get_bool("debug-soc", false));
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

#if WITH_HW_AMC
	extern void mcu_late_init(void);
	mcu_late_init();
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
#if WITH_HW_DISPLAY_PMU
		display_pmu_init();
#endif

		if (!displayInitOnce) {
			platform_quiesce_display();
			clock_gate(CLK_DISP_BUSMUX, true);
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
	}

	power_backlight_enable(backlight_level);
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
			quiesce_clocks = true;
#if WITH_BOOT_STAGE
			boot_set_stage(kPowerNVRAMiBootStageProductEnd);
#endif
			break;
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

	clock_gate(CLK_DISP_BUSMUX, true);

	if (display_quiesce(true) == ENXIO) {
		clock_gate(CLK_DISP_BUSMUX, false);
	}
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
#if WITH_PAINT
			if (paint_color_map_is_invalid())
				panic("Previous DClr errors prevent OS booting");
#endif
			platform_quiesce_display();
			if (boot_debug & kPowerNVRAMiBootDebugWDTWake)
			    wdt_enable();
			/* even when trusted, Darwin only gets the UID / GID1 */
			uids = 1;
			gids = 2;
			break;

		case BOOT_DARWIN:
#if WITH_PAINT
			if (paint_color_map_is_invalid())
				panic("Previous DClr errors prevent OS booting");
#endif
			platform_bootprep_darwin(false);
			if (boot_debug & kPowerNVRAMiBootDebugWDTWake)
			    wdt_enable();
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
#if WITH_HW_MIU && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)
			platform_relocate_securerom();
			miu_select_remap(REMAP_SRAM);
#endif
			break;

		case BOOT_MONITOR:
			/* clean-invalidate TZ0 & TZ1 regions */
			platform_cache_operation((CACHE_CLEAN | CACHE_INVALIDATE), (void *)TZ1_BASE, TZ1_SIZE);

			/* program values */
			rTZSROMCTRL_TZ1REGIONADDR = ((((TZ1_BASE + TZ1_SIZE - 1) >> 20) & 0x3fff) << 16) | (((TZ1_BASE >> 20) & 0x3fff) << 0);
			break;

		case BOOT_UNKNOWN:
			platform_quiesce_display();;
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

	/* Disable Cyclone specific errors enabled earlier in the boot */
	ccc_disable_custom_errors();

	return 0;
}

void platform_mmu_setup(bool resume)
{
	RELEASE_ASSERT(false == resume);

#if APPLICATION_SECUREROM
	arm_mmu_map_rx(VROM_BASE, VROM_LEN);
	arm_mmu_map_rw(SRAM_BASE, SRAM_LEN);

	// XXX put heap guard in though, and RO pagetables
#else
	// Figure out where the linker put our various bits
	uintptr_t text_end_aligned = ((uintptr_t)&_text_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	size_t text_size = text_end_aligned - (uintptr_t)&_text_start;

	RELEASE_ASSERT(text_end_aligned <= (uintptr_t)&_data_start);
#if PRODUCT_LLB || PRODUCT_IBSS
	arm_mmu_map_rw(SDRAM_BASE, SDRAM_LEN);
	// Make an uncached mapping to DRAM (used during memory calibration)
	arm_mmu_map_range(SDRAM_BASE_UNCACHED, SDRAM_BASE, SDRAM_LEN, kARMMMUDeviceRW);

	// map through to end of heap read/write
	arm_mmu_map_rw(SRAM_BASE, HEAP_END - SRAM_BASE);

	// __TEXT is read/execute, __DATA is read/write
	arm_mmu_map_rx((uintptr_t)&_text_start, text_size);
	arm_mmu_map_rw(text_end_aligned, PAGE_TABLES_BASE - text_end_aligned);

	// skip mapping the page tables so that they can't be modified

	// map the stacks read-write
	arm_mmu_map_rw(STACKS_BASE, SRAM_BASE + SRAM_LEN - STACKS_BASE);
#else
	// Most of DRAM gets mapped read/write
	arm_mmu_map_rw(SDRAM_BASE, (uintptr_t)&_text_start - SDRAM_BASE);

	// only the text section should be executable, and it should be read only
	arm_mmu_map_rx((uintptr_t)&_text_start, text_size);
	arm_mmu_map_rw(text_end_aligned, PAGE_TABLES_BASE - text_end_aligned);

	// skip mapping the page tables so that they can't be modified

	// map the stacks read-write
	arm_mmu_map_rw(STACKS_BASE, STACKS_SIZE);

	// map the boot trampoline read/execute
	arm_mmu_map_rx(BOOT_TRAMPOLINE_BASE, BOOT_TRAMPOLINE_SIZE);

	// map the heap read-write, leaving a hole at the end
	arm_mmu_map_rw(HEAP_BASE, HEAP_SIZE);
	RELEASE_ASSERT(HEAP_BASE + HEAP_SIZE == HEAP_GUARD);

	// and then everything up to the end of DRAM is read/write again
	arm_mmu_map_rw(IBOOT_BASE + IBOOT_SIZE, SDRAM_END - (IBOOT_BASE + IBOOT_SIZE));

#if DEBUG_BUILD
	// Create a virtual mapping for SRAM to allow SecureROM testing
	arm_mmu_map_rw(SRAM_BASE, SRAM_LEN);
#endif
#endif 
#endif

	// map IO
	arm_mmu_map_device_rw(IO_BASE, IO_SIZE);
}

int platform_init(void)
{
#if defined(UNUSED_MEMORY_BASE) && (UNUSED_MEMORY_SIZE > 0)
	bzero((void *)UNUSED_MEMORY_BASE, UNUSED_MEMORY_SIZE);
#endif

#if WITH_PLATFORM_ERROR_HANDLER
	platform_enable_error_handler();
#endif

#if WITH_CONSISTENT_DBG && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)
	consistent_debug_init();
#endif
#if WITH_HW_SPI
	spi_init();
#endif

#if WITH_ANC_FIRMWARE
	anc_firmware_init();
#endif

#if WITH_HW_ASP && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)
	csi_init(CSI_COPROC_ANS);   /* coproc switchboard used by nand driver to communicate with ans iop */
	asp_init();
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

		// Fill in the usb-phy frequency
		propName = "usbphy-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			 *(uint32_t *)propData = clock_get_frequency(CLK_USBPHYCLK);
		}
	}

	// Find the pmgr node
	if (FindNode(0, "arm-io/pmgr", &node)) {
		pmgr_update_device_tree(node);
		miu_update_device_tree(node);
	}

	// Find the gfx node
	if (FindNode(0, "arm-io/sgx", &node)) {
		pmgr_gfx_update_device_tree(node);
	}

	// Find the audio-complex node
	if (FindNode(0, "arm-io/audio-complex", &node)) {
		// Fill in the ncoref-frequency frequency
		propName = "ncoref-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			 *(uint32_t *)propData = clock_get_frequency(CLK_NCOREF);
		}
	}

	// Find the sochot0 and 1 nodes and override/augment EDT as needed
	if (FindNode(0, "arm-io/sochot0", &node)) {
		sochot_pmgr_update_device_tree(node);
	}
	if (FindNode(0, "arm-io/sochot1", &node)) {
		sochot_ccc_update_device_tree(node);
	}

	// Find the PMGR and CCC temperature sensors and override/augment EDT as needed
	if (FindNode(0, "arm-io/tempsensor0", &node)) {
		temp_sensor_pmgr_update_device_tree(node);
	}
	if (FindNode(0, "arm-io/tempsensor1", &node)) {
		temp_sensor_pmgr_update_device_tree(node);
	}
	if (FindNode(0, "arm-io/tempsensor2", &node)) {
		temp_sensor_ccc_update_device_tree(node);
	}
	if (FindNode(0, "arm-io/tempsensor3", &node)) {
		temp_sensor_ccc_update_device_tree(node);
	}

#if WITH_HW_PLATFORM_CHIPID
	// Find the arm-io node
	if (FindNode(0, "arm-io", &node)) {
		// Fill in the chip-revision property
		propName = "chip-revision";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*(uint32_t *)propData = platform_get_chip_revision();
		}
	}
#endif

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

	/* S5L8960X supports one boot device then USB-DFU per boot config */

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

#if WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI
static const struct boot_interface_pin spi0_boot_interface_pins[] =
{
#if SUPPORT_FPGA
	{ GPIO( 9, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_SSIN
#else
	{ GPIO( 9, 3), GPIO_CFG_OUT_1, GPIO_CFG_DFLT },	// SPI0_SSIN
#endif
	{ GPIO( 9, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_SCLK
	{ GPIO( 9, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_MOSI
	{ GPIO( 9, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// SPI0_MISO
};
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */

void platform_enable_boot_interface(bool enable, enum boot_device boot_device, uint32_t boot_arg)
{
	const struct boot_interface_pin *pins0 = 0;
	const struct boot_interface_pin *pins1 = 0;
	uint32_t cnt, func, pin_count0 = 0, pin_count1 = 0;
	gpio_t pin;

	switch (boot_device) {
#if WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI
		case BOOT_DEVICE_SPI :
			if (boot_arg == 0) {
				pins0 = spi0_boot_interface_pins;
				pin_count0 = (sizeof(spi0_boot_interface_pins) / sizeof(spi0_boot_interface_pins[0]));
			}
			break;
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */

#if WITH_ANC_BOOT
		case BOOT_DEVICE_NAND :
			/* NAND pins are off to ASP */
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

	for (cnt = 0; cnt < pin_count0; cnt++) {
		if (enable) {
			pin = pins0[pin_count0 - 1 - cnt].pin;
			func = pins0[pin_count0 - 1 - cnt].enable;
		} else {
			pin = pins0[cnt].pin;
			func = pins0[cnt].disable;
		}

		dprintf(DEBUG_INFO, "platform_enable_boot_interface: 0 %x, %x\n", pin, func);
		gpio_configure(pin, func);
	}

	for (cnt = 0; cnt < pin_count1; cnt++) {
		if (enable) {
			pin = pins1[pin_count1 - 1 - cnt].pin;
			func = pins1[pin_count1 - 1 - cnt].enable;
		} else {
			pin = pins1[cnt].pin;
			func = pins1[cnt].disable;
		}

		dprintf(DEBUG_INFO, "platform_enable_boot_interface: 1 %x, %x\n", pin, func);
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

bool platform_get_request_dfu1(void)	// Formerly platform_get_hold_key()
{
	return !gpio_read(GPIO_REQUEST_DFU1);
}

bool platform_get_request_dfu2(void)    // Formerly platform_get_menu_key()
{
	return !gpio_read(GPIO_REQUEST_DFU2);
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
	// Disable requested GID and UID in SIO-AES
	extern void aes_ap_disable_keys(uint32_t gid, uint32_t uid);
	aes_ap_disable_keys(gid, uid);

	// Disable UID in ANS, if UID disable is requested
	// XXX TODO: missing UID in ANS
}

bool platform_keys_disabled(uint32_t gid, uint32_t uid)
{
	bool result;

	// Check SIO
	extern bool aes_ap_keys_disabled(uint32_t gid, uint32_t uid);
	result = aes_ap_keys_disabled(gid, uid);

	// Check ANS
	// XXX TODO: missing ANS piece

	return result;
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

extern void boot_handoff_trampoline(void *entry, void *arg);

void *platform_get_boot_trampoline(void)
{
#ifdef BOOT_TRAMPOLINE_BASE
	return (void *)BOOT_TRAMPOLINE_BASE;
#else
	return (void *)boot_handoff_trampoline;
#endif
}

int32_t platform_restore_system(void)
{
	// XXX kSleepTokenKernelOffset = 0
	uint32_t *signature = (uint32_t *)(SLEEP_TOKEN_BUFFER_BASE + 0);

#if WITH_CONSISTENT_DBG
	consistent_debug_resume();
#endif

	power_will_resume();

	dprintf(DEBUG_INFO, "restore_system: signature[0]: 0x%08x, signature[1]: 0x%08x\n",
		signature[0], signature[1]);

	if ((signature[0] != 'MOSX') || (signature[1] != 'SUSP')) return -1;

	signature[0] = 0;
	signature[1] = 0;

	platform_bootprep_darwin(true);

	/* Jump to reset vector (first address of physical page of l4 or kernel). Our memory layout expects TZ0 -> TZ1 -> Kernel memory */
	dprintf(DEBUG_INFO, "restoring kernel\n");

	prepare_and_jump(BOOT_DARWIN_RESTORE, (void *)TZ1_BASE, NULL);

	/* shouldn't get here */
	panic("returned from restore_system\n");
}

void platform_asynchronous_exception(void)
{
	ccc_handle_asynchronous_exception();
}

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

int platform_convert_voltages(int buck, u_int32_t count, u_int32_t *voltages)
{
#if SUPPORT_FPGA
	u_int32_t index;
	for (index = 0; index < count; index++)
		power_get_buck_value_fpga(buck, voltages[index], &voltages[index]);
	return 0;
#elif WITH_HW_POWER
	u_int32_t index;

	if (voltages == 0) return -1;

	for (index = 0; index < count; index++) {
		if (0 != power_get_buck_value(buck, voltages[index], &voltages[index]))
			return -1;
	}

	return 0;
#else
	return -1;
#endif
}

int platform_get_cpu_voltages(u_int32_t count, u_int32_t *voltages)
{
	u_int32_t cnt;

	if (voltages == 0) return -1;

	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = chipid_get_cpu_voltage(cnt);
	}

	return 0;
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
		
		case kMemoryRegion_Monitor:
			base = TZ1_BASE;
			break;

		case kMemoryRegion_Kernel:
			base = TZ0_BASE + TZ0_SIZE;
			break;

		case kMemoryRegion_PageTables:
			base = PAGE_TABLES_BASE;
			break;

		case kMemoryRegion_Heap:
			base = HEAP_BASE;
			break;

		case kMemoryRegion_Stacks:
			base = STACKS_BASE;
			break;

#if APPLICATION_IBOOT
		case kMemoryRegion_ConsistentDebug:
			base = CONSISTENT_DEBUG_BASE;
			break;

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

		case kMemoryRegion_SecureProcessor:
			size = TZ0_SIZE;
			break;
		
		case kMemoryRegion_Monitor:
			size = TZ1_SIZE;
			break;

		case kMemoryRegion_Kernel:
			size = platform_get_memory_region_base(kMemoryRegion_Display) - platform_get_memory_region_base(kMemoryRegion_Kernel);
			break;

		case kMemoryRegion_PageTables:
			size = PAGE_TABLES_SIZE;
			break;

		case kMemoryRegion_Heap:
			size = HEAP_SIZE;
			break;

		case kMemoryRegion_Stacks:
			size = STACKS_SIZE;
			break;

#if APPLICATION_IBOOT
		case kMemoryRegion_ConsistentDebug:
			size = CONSISTENT_DEBUG_SIZE;
			break;

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

int platform_get_gpu_voltages(u_int32_t count, u_int32_t *voltages)
{
	u_int32_t cnt;

	if (voltages == 0)	return -1;

	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = chipid_get_gpu_voltage(cnt);
	}

	return 0;
}

/*
 *	Gets equivalent mV for a given DWI value for a particular buck.
 *	PMU code should handle the special case for GPU buck.
 */
int platform_get_dwi_to_mv(int buck, u_int32_t dwival)
{
#if SUPPORT_FPGA
	return power_convert_dwi_to_mv_fpga(buck, dwival);
#elif WITH_HW_POWER
	return power_convert_dwi_to_mv(buck, dwival);
#else
	return -1;
#endif
}

#if WITH_TARGET_CONFIG

// The target's rules.mk sets the high SoC voltage point
#ifndef TARGET_BOOT_SOC_VOLTAGE
#error TARGET_BOOT_SOC_VOLTAGE not defined by the target
#endif

u_int32_t platform_get_base_soc_voltage(void)
{
	return chipid_get_soc_voltage(TARGET_BOOT_SOC_VOLTAGE);
}

// The target's rules.mk sets the high CPU voltage point
#ifndef TARGET_BOOT_CPU_VOLTAGE
#error TARGET_BOOT_CPU_VOLTAGE not defined by the target
#endif

u_int32_t platform_get_base_cpu_voltage(void)
{
	return chipid_get_cpu_voltage(TARGET_BOOT_CPU_VOLTAGE);
}

// The target's rules.mk sets the high RAM voltage point
#ifndef TARGET_BOOT_RAM_VOLTAGE
#error TARGET_BOOT_RAM_VOLTAGE not defined by the target
#endif

u_int32_t platform_get_base_ram_voltage(void)
{
	return chipid_get_ram_voltage(TARGET_BOOT_RAM_VOLTAGE);
}

#endif

int platform_get_soc_voltages(u_int32_t count, u_int32_t *voltages)
{
	u_int32_t cnt;

	if (voltages == 0) return -1;

	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = platform_get_base_soc_voltage();
	}

	return 0;
}

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

static int32_t platform_bootprep_darwin(bool resume)
{
	/* 1. Setup trustzones */

	if (!resume) {
		/* clean-invalidate TZ0 & TZ1 regions */
		platform_cache_operation((CACHE_CLEAN | CACHE_INVALIDATE), (void *)TZ0_BASE, TZ0_SIZE);
		platform_cache_operation((CACHE_CLEAN | CACHE_INVALIDATE), (void *)TZ1_BASE, TZ1_SIZE);

		/* create sleep token */
		/* already created by load_kernelcache call */
	}
	else {
		/* validate sleep token */
		if (!security_validate_sleep_token(SLEEP_TOKEN_BUFFER_BASE + kSleepTokeniBootOffset)) return -1;
	}

	/* program values */
	rTZSROMCTRL_TZ0REGIONADDR = ((((TZ0_BASE + TZ0_SIZE - 1) >> 20) & 0x3fff) << 16) | (((TZ0_BASE >> 20) & 0x3fff) << 0);
	rTZSROMCTRL_TZ1REGIONADDR = ((((TZ1_BASE + TZ1_SIZE - 1) >> 20) & 0x3fff) << 16) | (((TZ1_BASE >> 20) & 0x3fff) << 0);

	/* lock TZ0 & TZ1 regions */
	rTZSROMCTRL_TZ0LOCK = (1 << 0);
	if ((rTZSROMCTRL_TZ0LOCK & 1) == 0) {
		panic("TZ0 failed to lock\n");
	}
	rTZSROMCTRL_TZ1LOCK = (1 << 0);
	if ((rTZSROMCTRL_TZ1LOCK & 1) == 0) {
		panic("TZ1 failed to lock\n");
	}

	/* 2. Override IO_RVBAR */

	ccc_override_and_lock_iorvbar(TZ1_BASE);

	return 0;
}

static void platform_relocate_securerom(void)
{
#if WITH_HW_MIU && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)
	size_t	len;

	// Get the size of the downloaded SecureROM image
	len = env_get_uint("filesize", 0);
	if (len == 0)
		panic("filesize variable invalid or not set, aborting\n");

	// Move the SecureROM image into place
	dprintf(DEBUG_INFO, "relocating 0x%lx byte SecureROM image from SDRAM to SRAM\n", len);
	memcpy((void *)SRAM_BASE, (void *)SDRAM_BASE, len);
#endif
}

#if SUPPORT_FPGA
static void power_get_buck_value_fpga(int buck, uint32_t mv, uint32_t *val)
{
	if (mv == 0) {
		*val = 0;
	} else {
		*val = (((mv-600)*1000)+3124)/3125;
	}
	return;
}

static int power_convert_dwi_to_mv_fpga(unsigned int buck, u_int32_t dwival)
{
	int val = 0;
#if APPLICATION_IBOOT
#ifndef BUCK_GPU
#error BUCK_GPU not defined for this platform
#endif
	if (buck == BUCK_GPU) {
		val = (dwival == 0) ? 0 : (600000 + (3125 * dwival))/1000;
	}
#endif
	return val;
}	
#endif

#if defined(WITH_MENU) && WITH_MENU

int do_sleep_token_test(int argc, struct cmd_arg *args)
{
	*(uint32_t *)(SLEEP_TOKEN_BUFFER_BASE + 0xA0) = 0;
	*(uint32_t *)(SLEEP_TOKEN_BUFFER_BASE + 0xA4) = 0;

	security_create_sleep_token(SLEEP_TOKEN_BUFFER_BASE + 0x90);

	*(uint32_t *)(SLEEP_TOKEN_BUFFER_BASE + 0xA0) = 0x12345678;
	*(uint32_t *)(SLEEP_TOKEN_BUFFER_BASE + 0xA4) = 0xaabbccdd;
	dprintf(DEBUG_INFO, "original info buffer: %#x %#x\n", *(uint32_t *)(SLEEP_TOKEN_BUFFER_BASE + 0xA0), *(uint32_t *)(SLEEP_TOKEN_BUFFER_BASE + 0xA4));

	if (security_validate_sleep_token(SLEEP_TOKEN_BUFFER_BASE + 0x90) == false) {
		dprintf(DEBUG_INFO, "failed to validate token\n");
		return 0;
	}

	dprintf(DEBUG_INFO, "saved info buffer: %#x %#x\n", *(uint32_t *)(SLEEP_TOKEN_BUFFER_BASE + 0xA0), *(uint32_t *)(SLEEP_TOKEN_BUFFER_BASE + 0xA4));

	return 0;
}

MENU_COMMAND_DEBUG(sleep_token, do_sleep_token_test, "sleep token test - creates and validate it", NULL);

#endif

static int get_total_leakage(int argc, struct cmd_arg *args)
{
	dprintf(DEBUG_INFO, "total leakage: %dmA\n", chipid_get_total_rails_leakage());
	return 0;
}
MENU_COMMAND_DEBUG(leakage, get_total_leakage, "total rails leakage read from fuse", NULL);
