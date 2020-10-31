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

#include <arch.h>
#include <arch/arm/arm.h>
#include <debug.h>
#include <drivers/aes.h>
#include <drivers/anc_boot.h>
#include <drivers/apcie.h>
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
#include <drivers/thunderbolt/thunderboot.h>

static void platform_init_boot_strap(void);
static int32_t platform_bootprep_darwin(bool resume);
static bool platform_get_boot_from_nvme(uint32_t *port, uint32_t *dart_id);
static void power_get_buck_value_fpga(int buck, uint32_t mv, uint32_t *val);
static int power_convert_dwi_to_mv_fpga(unsigned int buck, u_int32_t dwival);
static void platform_relocate_securerom(void);
#if SUB_PLATFORM_T7001 && (PRODUCT_LLB || PRODUCT_IBSS)
static void t7001_ram_repair_init(void);
#endif

static uint8_t boot_debug;
static bool gDisplayEnabled;

int platform_early_init(void)
{
#if PRODUCT_LLB || PRODUCT_IBSS
	/* Verify that the fuses and SecureROM R/W access has been disabled */
	if (!chipid_get_fuse_lock() || (((*(volatile u_int32_t *)SECURITY_REG) & ROM_READ_DISABLE) == 0)) {
		panic("Fuses are unlocked or SecureROM is enabled\n");
	}

#if SUB_PLATFORM_T7001
	// <rdar://problem/16734095> Capri: Core-2 repair register loading fails if Core-1 is booted before Core-2
	t7001_ram_repair_init();
#endif
#endif

	/* Enable more Cyclone specific errors */
	ccc_enable_custom_errors();

#if WITH_HW_PLATFORM_POWER
	/* initialize the pmgr driver */
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

#if SUB_PLATFORM_T7000
	// Note: This can't be postposned until platform_late_init() because
	//	 Fiji A0 chips will hang before we can get there.
	if (platform_get_chip_revision() < CHIP_REVISION_B1) {
		platform_not_supported();
	}
#elif SUB_PLATFORM_T7001
	if (platform_get_chip_revision() < CHIP_REVISION_A1) {
		platform_not_supported();
	}
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
#if SUB_PLATFORM_T7000
			clock_gate(CLK_DISP_BUSMUX, true);
#elif SUB_PLATFORM_T7001
			clock_gate(CLK_DISP0_BUSIF, true);
#endif
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
	if (!platform_get_boot_from_nvme(NULL, NULL))
		return (asp_nand_open());
#endif
#if WITH_NVME
	if (platform_get_boot_from_nvme(NULL, NULL))
		return nvme_init_mass_storage(0);
#endif
	return 0;
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

#if WITH_NVME
	nvme_quiesce_all();
#endif

#if WITH_THUNDERBOOT
	thunderboot_quiesce_and_free(NULL);
#endif

#if WITH_HW_APCIE
	apcie_disable_all();
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

#if SUB_PLATFORM_T7000
	clock_gate(CLK_DISP_BUSMUX, true);
#elif SUB_PLATFORM_T7001
	clock_gate(CLK_DISP0_BUSIF, true);
#endif
	/* quiesce the panel */
	if (display_quiesce(true) == ENXIO) {
#if SUB_PLATFORM_T7000
		clock_gate(CLK_DISP_BUSMUX, false);
#elif SUB_PLATFORM_T7001
		clock_gate(CLK_DISP0_BUSIF, false);
#endif
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
			rMCCLOCKREGION_TZ1BASEADDR = TZ1_BASE >> 12;
			rMCCLOCKREGION_TZ1ENDADDR  = (TZ1_BASE + TZ1_SIZE - 1) >> 12;
			break;

		case BOOT_UNKNOWN:
			platform_quiesce_display();
			break;

		default:
			; // do nothing
	}

	/* make sure that fuse lock bit is set. */
	if (security_get_lock_fuses())
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

	// for future ROMS add heap guard and RO pagetables
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
	arm_mmu_map_device_rw(PCI_REG_BASE, PCI_REG_LEN);
	arm_mmu_map_device_rw(PCI_CONFIG_BASE, PCI_CONFIG_LEN);
	arm_mmu_map_device_rw(PCI_32BIT_BASE, PCI_32BIT_LEN);
}

int platform_init(void)
{
	uint32_t nvme_port = 0;
	uint32_t nvme_dart_id = 0;
	bool boot_from_nvme;

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

#if WITH_PCI
	pci_init();
#endif

	boot_from_nvme = platform_get_boot_from_nvme(&nvme_port, &nvme_dart_id);

#if WITH_ANC_FIRMWARE
	if (!boot_from_nvme)
		anc_firmware_init();
#endif

#if WITH_HW_ASP && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)
	if (!boot_from_nvme) {
		csi_init(CSI_COPROC_ANS);   /* coproc switchboard used by nand driver to communicate with ans iop */
		asp_init();
	}
#endif

#if WITH_NVME && (PRODUCT_LLB || PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)
	if (boot_from_nvme) {
#if SUB_PLATFORM_T7000
		apcie_use_external_refclk(true);
#endif
#if PRODUCT_IBEC
		apcie_set_s3e_mode(true);
#else
		apcie_set_s3e_mode(false);
#endif
		if (apcie_enable_link(nvme_port)) {
			nvme_init(0, apcie_get_port_bridge(nvme_port), nvme_dart_id);
		}
	}
#endif

#if WITH_THUNDERBOOT && (PRODUCT_IBSS || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)
	if (apcie_enable_link(0)) {
		thunderboot_init(apcie_get_port_bridge(0), PCIE_PORT0_DART_ID);
	}
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

#if WITH_NVME
	nvme_quiesce_all();
#endif

#if WITH_HW_APCIE
	apcie_disable_all();
#endif

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

void platform_setup_default_environment(void)
{
#if WITH_ENV
	if (!platform_get_boot_from_nvme(NULL, NULL))
		env_set("boot-device", "asp_nand", 0);
	else
		env_set("boot-device", "nvme_nand0", 0);
#endif

	target_setup_default_environment();
}

#if WITH_DEVICETREE

int platform_update_device_tree(void)
{
	DTNode *node;
	uint32_t propSize;
	char *propName;
	void *propData;
#if SUB_PLATFORM_T7001
	char *propNamePOR;
#endif
	
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
		
#if SUB_PLATFORM_T7001
		if (chipid_get_fuse_revision() < 0x1) {
			propName = "gpu-device-max-power-fuserev0";
			propNamePOR = "gpu-device-max-power";
			
			if (FindProperty(node, &propName, &propData, &propSize) &&
				FindProperty(node, &propNamePOR, &propData, &propSize)) {
				strlcpy(propName, "gpu-device-max-power", kPropNameLength);
				propNamePOR[0] = '~';
			}
			
			propName = "gpu-pwr-perf-scale4";
			if (FindProperty(node, &propName, &propData, &propSize))
				propName[0] = '~';
		}
#endif
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

	// Find the apcie node
	if (FindNode(0, "arm-io/apcie", &node)) {
		apcie_update_devicetree(node);
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

	if (platform_get_boot_from_nvme(NULL, NULL)) {
		if (FindNode(0, "arm-io/ans", &node)) {
			propName = "compatible";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				propName[0] = '~';
			}
		}

#if SUB_PLATFORM_T7000
		// <rdar://problem/15012448> Automatically select external refclk in OS on Fiji when booting from NVMe
		if (FindNode(0, "arm-io/apcie", &node)) {
			propName = "phy0-external-refclk-nvme";
			if (FindProperty(node, &propName, &propData, &propSize)) {
				strlcpy(propName, "phy0-external-refclk", kPropNameLength);
			}
		}
#endif

		if (FindNode(0, "arm-io/apcie/pci-bridge0/device0", &node)) {
			propName = "nvme-scratch-region";
			if (FindProperty(node, &propName, &propData, &propSize) && propSize == 2 * sizeof(uintptr_t)) {
				((uintptr_t *)propData)[0] = platform_get_memory_region_base(kMemoryRegion_StorageProcessor);
				((uintptr_t *)propData)[1] = platform_get_memory_region_size(kMemoryRegion_StorageProcessor);
			}
		} else if (FindNode(0, "arm-io/apcie/pci-bridge2/device2", &node)) {
			propName = "nvme-scratch-region";
			if (FindProperty(node, &propName, &propData, &propSize) && propSize == 2 * sizeof(uintptr_t)) {
				((uintptr_t *)propData)[0] = platform_get_memory_region_base(kMemoryRegion_StorageProcessor);
				((uintptr_t *)propData)[1] = platform_get_memory_region_size(kMemoryRegion_StorageProcessor);
			}
		}
	}

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

bool platform_get_lock_fuses_required(void)
{
	bool lock_fuses = true;

#if APPLICATION_SECUREROM && SUB_PLATFORM_T7001
	if (chipid_get_board_id() == 0x6)
		lock_fuses = false;
#endif

	return lock_fuses;
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

	/* T7000 supports one boot device then USB-DFU per boot config */
	/* T7001 supports the same, with the special case of TBT. The rules there are:
	   - If FORCE_DFU is asserted and USB is connected, do USB DFU on all tries
	   - If FORCE_DFU is not asserted, do TBT DFU on all tries */

	switch (boot_config) {
		case BOOT_CONFIG_SPI0:
			*boot_device = BOOT_DEVICE_SPI;
			*boot_flag = 0;
			*boot_arg = 0;
			break;

		case BOOT_CONFIG_SPI0_TEST:
		case BOOT_CONFIG_FAST_SPI0_TEST:
			*boot_device = BOOT_DEVICE_SPI;
			*boot_flag = BOOT_FLAG_TEST_MODE;
			*boot_arg = 0;
			break;

		case BOOT_CONFIG_ANS:
			*boot_device = BOOT_DEVICE_NAND;
			*boot_flag = 0;
			*boot_arg = 0;
			break;

		case BOOT_CONFIG_ANS_TEST:
			*boot_device = BOOT_DEVICE_NAND;
			*boot_flag = BOOT_FLAG_TEST_MODE;
			*boot_arg = 0;
			break;

		case BOOT_CONFIG_NVME0:
			*boot_device = BOOT_DEVICE_NVME;
			*boot_flag = 0;
			*boot_arg = 0;
			break;

		case BOOT_CONFIG_NVME0_TEST:
			*boot_device = BOOT_DEVICE_NVME;
			*boot_flag = BOOT_FLAG_TEST_MODE;
			*boot_arg = 0;
			break;

#if SUB_PLATFORM_T7001
		case BOOT_CONFIG_NVME1:
			*boot_device = BOOT_DEVICE_NVME;
			*boot_flag = 0;
			*boot_arg = 1;
			break;

		case BOOT_CONFIG_NVME1_TEST:
			*boot_device = BOOT_DEVICE_NVME;
			*boot_flag = BOOT_FLAG_TEST_MODE;
			*boot_arg = 1;
			break;
#endif

#ifndef SUB_PLATFORM_T7000
		case BOOT_CONFIG_LOW_POWER_ANS:
			*boot_device = BOOT_DEVICE_NAND;
			*boot_flag = 0;
			*boot_arg = ANC_BOOT_MODE_RESET_ONE_CONTROLLER;
			break;
#endif

#if SUB_PLATFORM_T7001
		case BOOT_CONFIG_TBT0_EXTREF:
			*boot_device = BOOT_DEVICE_TBTDFU;
			*boot_flag = 0;
			*boot_arg = 1;
			break;

		case BOOT_CONFIG_TBT0_EXTREF_TEST:
			*boot_device = BOOT_DEVICE_TBTDFU;
			*boot_flag = BOOT_FLAG_TEST_MODE;
			*boot_arg = 1;
			break;

		case BOOT_CONFIG_TBT0_INTREF:
			*boot_device = BOOT_DEVICE_TBTDFU;
			*boot_flag = 0;
			*boot_arg = 0;
			break;

		case BOOT_CONFIG_TBT0_INTREF_TEST:
			*boot_device = BOOT_DEVICE_TBTDFU;
			*boot_flag = BOOT_FLAG_TEST_MODE;
			*boot_arg = 0;
			break;
#endif

		default:
			return false;
	}

	/* Change boot_device and boot_arg for DFU Mode */
	/* Don't change flags */
	if (index == -1 || (index != 0 && *boot_device != BOOT_DEVICE_TBTDFU)) {
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
	{ GPIO_SPI0_SSIN, GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_SSIN
#else
	{ GPIO_SPI0_SSIN, GPIO_CFG_OUT_1, GPIO_CFG_DFLT },	// SPI0_SSIN
#endif
	{ GPIO_SPI0_SCLK, GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_SCLK
	{ GPIO_SPI0_MOSI, GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_MOSI
	{ GPIO_SPI0_MISO, GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// SPI0_MISO
};
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */

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
			/* NAND pins are off to ASP */
			break;
#endif /* WITH_ANC_BOOT */

#if WITH_NVME
		case BOOT_DEVICE_NVME :
		{
			uint32_t dart_id = 0;
			uint32_t pcie_port = 0;
			if (boot_arg == 0) {
				dart_id = PCIE_PORT0_DART_ID;
				pcie_port = 0;
#if SUB_PLATFORM_T7001
			} else if (boot_arg == 1) {
				dart_id = PCIE_PORT2_DART_ID;
				pcie_port = 2;
#endif
			} else {
				panic("invalid NVMe boot argument %u", boot_arg);
			}

			if (enable) {
#if SUB_PLATFORM_T7000
				// Fiji uses external refclk, Capri uses the internal one
				apcie_use_external_refclk(true);
#endif
				apcie_set_s3e_mode(false);

				// Don't try to probe for the NVMe device if the link doesn't come
				// up because we shut down the root complex on link initialization
				// failures
				if (apcie_enable_link(pcie_port)) {
					nvme_init(0, apcie_get_port_bridge(pcie_port), dart_id);
				}
			}
			if (!enable) {
				// These guys are safe no-ops if the enable above failed
				nvme_quiesce(0);

				apcie_disable_link(pcie_port);
			}

			break;
		}
#endif

#if WITH_TBT_BOOT
		case BOOT_DEVICE_TBTDFU :
		{
			bool ext_refclk = (boot_arg & 1) != 0;

			if (enable) {
				apcie_use_external_refclk(ext_refclk);

				// Don't try to probe for the TBT device if the link doesn't come
				// up because we shut down the root complex on link initialization
				// failures
				if (apcie_enable_link(0)) {
					thunderboot_init(apcie_get_port_bridge(0), PCIE_PORT0_DART_ID);
				}
			}
			if (!enable) {
				// These guys are safe no-ops if the enable above failed
				thunderboot_quiesce_and_free(NULL);

				apcie_disable_link(0);
			}

			break;
		}
#endif

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
	if (platform_get_boot_config() == BOOT_CONFIG_FAST_SPI0_TEST)
		return 24000000;
	else
		return 12000000;
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

#if APPLICATION_SECUREROM
	// Disable UID1 in ANS, if UID1 disable is requested
	extern void anc_disable_uid_key();
	if (uid & 1)
		anc_disable_uid_key();
#endif

	// XXX TODO: Revisit for LLB, iBoot, iBSS, iBEC
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

#if PRODUCT_LLB
	// Workaround per rdar://problem/18049697 - Zero out Host S3-E DRAM on Wake from Suspsend to RAM
	if (platform_get_boot_from_nvme(NULL, NULL))
	{
		bzero((void *)ASP_BASE, (size_t)ASP_SIZE);
		dprintf(DEBUG_INFO, "NVMe: Zero'd out scratch buffer\n");
	}
#endif

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

int platform_get_soc_voltages(u_int32_t count, u_int32_t *voltages)
{
	u_int32_t cnt;

	if (voltages == 0) return -1;

	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = chipid_get_soc_voltage(cnt);
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

uint32_t platform_get_pcie_l1ss_ltr_threshold(void)
{
	// Per Fiji Tunables r1.44 section 8.2
	return 93;
}

uint32_t platform_get_pcie_l1ss_t_common_mode(void)
{
	// Per Fiji Tunables r1.44 section 8.2
	return 55;
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

int platform_get_gpu_ram_voltages(u_int32_t count, u_int32_t *voltages)
{
	u_int32_t cnt;

	if (voltages == 0)	return -1;

	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = chipid_get_gpu_ram_voltage(cnt);
	}

	return 0;
}

int platform_get_ram_voltages(u_int32_t count, u_int32_t *voltages)
{
	u_int32_t cnt;

	if (voltages == 0)	return -1;

	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = chipid_get_ram_voltage(cnt);
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

static void platform_init_boot_strap(void)
{
	uint32_t boot_strap, chip_board_id, gpio_board_id, boot_config;

	// If rPMGR_SCRATCH0[0] set then boot strap already valid
	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagBootStrap) != 0) return;

	gpio_configure(GPIO_BOARD_ID0, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_ID1, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_ID2, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_ID3, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_ID4, GPIO_CFG_IN);

	gpio_configure_pupdn(GPIO_BOARD_ID0, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOARD_ID1, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOARD_ID2, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOARD_ID3, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOARD_ID4, GPIO_PDN);

	gpio_configure(GPIO_BOOT_CONFIG0, GPIO_CFG_IN);
	gpio_configure(GPIO_BOOT_CONFIG1, GPIO_CFG_IN);
	gpio_configure(GPIO_BOOT_CONFIG2, GPIO_CFG_IN);
#if SUB_PLATFORM_T7001
	gpio_configure(GPIO_BOOT_CONFIG3, GPIO_CFG_IN);
#endif

	gpio_configure_pupdn(GPIO_BOOT_CONFIG0, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOOT_CONFIG1, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOOT_CONFIG2, GPIO_PDN);
#if SUB_PLATFORM_T7001
	gpio_configure_pupdn(GPIO_BOOT_CONFIG3, GPIO_PDN);
#endif

	platform_power_spin(100); // Wait 100us

	chip_board_id = chipid_get_board_id();

	// Build the board ID and boot config using the new scheme.
	gpio_board_id =
		(gpio_read(GPIO_BOARD_ID4) << 4) |
		(gpio_read(GPIO_BOARD_ID3) << 3) |
		(gpio_read(GPIO_BOARD_ID2) << 2) |
		(gpio_read(GPIO_BOARD_ID1) << 1) |
		(gpio_read(GPIO_BOARD_ID0) << 0);

	boot_config =
#if SUB_PLATFORM_T7001
		(gpio_read(GPIO_BOOT_CONFIG3) << 3) |
#endif
		(gpio_read(GPIO_BOOT_CONFIG2) << 2) |
		(gpio_read(GPIO_BOOT_CONFIG1) << 1) |
		(gpio_read(GPIO_BOOT_CONFIG0) << 0);

	gpio_configure(GPIO_BOARD_ID0, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_ID1, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_ID2, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_ID3, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_ID4, GPIO_CFG_DFLT);

	gpio_configure(GPIO_BOOT_CONFIG0, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOOT_CONFIG1, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOOT_CONFIG2, GPIO_CFG_DFLT);
#if SUB_PLATFORM_T7001
	gpio_configure(GPIO_BOOT_CONFIG3, GPIO_CFG_DFLT);
#endif

	boot_strap = (((chip_board_id << 5) | gpio_board_id) << 16) |
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
	}
	else {
		/* validate sleep token */
		if (!security_validate_sleep_token(SLEEP_TOKEN_BUFFER_BASE + kSleepTokeniBootOffset)) return -1;
	}

	/* program values */
	rMCCLOCKREGION_TZ0BASEADDR = TZ0_BASE >> 12;
	rMCCLOCKREGION_TZ0ENDADDR  = (TZ0_BASE + TZ0_SIZE - 1) >> 12;
	rMCCLOCKREGION_TZ1BASEADDR = TZ1_BASE >> 12;
	rMCCLOCKREGION_TZ1ENDADDR  = (TZ1_BASE + TZ1_SIZE - 1) >> 12;

	/* lock TZ0 & TZ1 regions */
	rMCCLOCKREGION_TZ0LOCK = (1 << 0);
	if ((rMCCLOCKREGION_TZ0LOCK & 1) == 0) {
		panic("TZ0 failed to lock\n");
	}
	rMCCLOCKREGION_TZ1LOCK = (1 << 0);
	if ((rMCCLOCKREGION_TZ1LOCK & 1) == 0) {
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

static bool platform_get_boot_from_nvme(uint32_t *port, uint32_t *dart_id)
{
	bool result = false;
#if WITH_NVME
	enum boot_device boot_device;
	uint32_t boot_flag;
	uint32_t boot_arg;

	if (platform_get_boot_device(0, &boot_device, &boot_flag, &boot_arg) && boot_device == BOOT_DEVICE_NVME) {
		result = true;
		if (port != NULL)
#if SUB_PLATFORM_T7000
			*port = 0;
#elif SUB_PLATFORM_T7001
			*port = boot_arg == 0 ? 0 : 2;
#endif
		if (dart_id != NULL)
#if SUB_PLATFORM_T7000
			*dart_id = PCIE_PORT0_DART_ID;
#elif SUB_PLATFORM_T7001
			*dart_id = boot_arg == 0 ? PCIE_PORT0_DART_ID : PCIE_PORT2_DART_ID;
#endif
	}
#endif
	return result;
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

static int get_total_leakage(int argc, struct cmd_arg *args)
{
	dprintf(DEBUG_INFO, "total leakage: %dmA\n", chipid_get_total_rails_leakage());
	return 0;
}
MENU_COMMAND_DEBUG(leakage, get_total_leakage, "total rails leakage read from fuse", NULL);

#if SUB_PLATFORM_T7001 && (PRODUCT_LLB || PRODUCT_IBSS)

// Capri CPU2 RAM repair does not happen if CPU1 powers up before CPU2.
// Therefore, when coming up from a powered off state, power up CPU2 then CPU1.
// It is not necessary for the CPU1/CPU2 to execut any instructions for the
// RAM repair to be triggered. All they have to do is power up. Therefore,
// use the CPU's debug interface to hold CPU1/CPU2 in reset so that they
// don't execute any instructions, then power them down again.
//
// <rdar://problem/16734095> Capri: Core-2 repair register loading fails if Core-1 is booted before Core-2

#define rCCC_CCC_CCC_OVRD	(*(volatile u_int32_t *)(CCC_CCC_DBG_BASE_ADDR + 0x000))
# define CCC_CCC_CCC_OVRD_SPIDEN_FORCE_DBG_EN	(0x3ul << 20)

#define rCCC_CCC_CCC_STS	(*(volatile u_int32_t *)(CCC_CCC_DBG_BASE_ADDR + 0x090))
# define CCC_CCC_CCC_STS_cc2St_MASK		(0xFul << 25)

#define rCCC_CPU1_CYC_OVRD	(*(volatile u_int32_t *)(CCC_CPU1_SYS_BASE_ADDR + 0x110))
#define rCCC_CPU2_CYC_OVRD	(*(volatile u_int32_t *)(CCC_CPU2_SYS_BASE_ADDR + 0x110))
# define CCC_CPUx_CYC_OVRD_reqBoot_MASK		(0x3ul << 28)
# define CCC_CPUx_CYC_OVRD_reqBoot_OVERRIDE	(0x2ul << 28)
# define CCC_CPUx_CYC_OVRD_reqBoot_BOOT		(0x3ul << 28)
# define CCC_CPUx_CYC_OVRD_ok2pwrdn_MASK	(0x3ul << 24)
# define CCC_CPUx_CYC_OVRD_ok2pwrdn_POWERDN	(0x3ul << 24)

#define rCCC_CPU1_DGB_WRAP	(*(volatile u_int32_t *)(CCC_CPU1_TRC_BASE_ADDR + 0x000))
#define rCCC_CPU2_DGB_WRAP	(*(volatile u_int32_t *)(CCC_CPU2_TRC_BASE_ADDR + 0x000))
# define CCC_CPUx_DGB_WRAP_dbgHaltOnRst		(0x1ul << 29)
# define CCC_CPUx_DGB_WRAP_dbgAck		(0x1ul << 28)
# define CCC_CPUx_DGB_WRAP_frcPwrDn		(0x1ul << 27)

#define RAM_REPAIR_TIMEOUT_COUNT	(10000000)

void t7001_ram_repair_init(void)
{
	uint32_t	ccc_ovrd;	// CCC override control
	uint32_t	cyc_ovrd1;	// CPU1 core override control
	uint32_t	cyc_ovrd2;	// CPU2 core override control
	uint32_t	dbg_wrap1;	// CPU1 debug wrapper control
	uint32_t	dbg_wrap2;	// CPU2 debug wrapper control
	uint32_t	reg;
	int		i;

	// Save the state of all the register that will get modified below.
	ccc_ovrd  = rCCC_CCC_CCC_OVRD;
	cyc_ovrd1 = rCCC_CPU1_CYC_OVRD;
	cyc_ovrd2 = rCCC_CPU2_CYC_OVRD;
	dbg_wrap1 = rCCC_CPU1_DGB_WRAP;
	dbg_wrap2 = rCCC_CPU2_DGB_WRAP;

	// Override SPIDEN so that we can modify registers in the debug wrapper.
	reg = ccc_ovrd | CCC_CCC_CCC_OVRD_SPIDEN_FORCE_DBG_EN;
	rCCC_CCC_CCC_OVRD = reg;

	// Power up CPU2 and drop in to debug mode on reset.
	reg = dbg_wrap2 | CCC_CPUx_DGB_WRAP_dbgHaltOnRst;
	rCCC_CPU2_DGB_WRAP = reg;
	reg = (cyc_ovrd2 & ~CCC_CPUx_CYC_OVRD_reqBoot_MASK) | CCC_CPUx_CYC_OVRD_reqBoot_BOOT;
	rCCC_CPU2_CYC_OVRD = reg;

	// Power up CPU1 and drop in to debug mode on reset.
	reg = dbg_wrap1 | CCC_CPUx_DGB_WRAP_dbgHaltOnRst;
	rCCC_CPU1_DGB_WRAP = reg;
	reg = (cyc_ovrd1 & ~CCC_CPUx_CYC_OVRD_reqBoot_MASK) | CCC_CPUx_CYC_OVRD_reqBoot_BOOT;
	rCCC_CPU1_CYC_OVRD = reg;

	// Wait for CPU1 to power up.
	for (i = 0; i < RAM_REPAIR_TIMEOUT_COUNT; i++) {
		if (rCCC_CPU1_DGB_WRAP & CCC_CPUx_DGB_WRAP_dbgAck) {
			break;
		}
	}

	// Failure of CPU1 to power up means death.
	if (!(rCCC_CPU1_DGB_WRAP & CCC_CPUx_DGB_WRAP_dbgAck)) {
		panic("CPU1 failed to power up");
	}

	// Power down CPU1.
	reg = rCCC_CPU1_CYC_OVRD & ~CCC_CPUx_CYC_OVRD_reqBoot_MASK;
	reg |= CCC_CPUx_CYC_OVRD_reqBoot_OVERRIDE;
	reg |= CCC_CPUx_CYC_OVRD_ok2pwrdn_POWERDN;
	rCCC_CPU1_CYC_OVRD = reg;

	reg = rCCC_CPU1_DGB_WRAP | CCC_CPUx_DGB_WRAP_frcPwrDn;
	rCCC_CPU1_DGB_WRAP = reg;

	// Power down CPU2.
	reg = rCCC_CPU2_CYC_OVRD & ~CCC_CPUx_CYC_OVRD_reqBoot_MASK;
	reg |= CCC_CPUx_CYC_OVRD_reqBoot_OVERRIDE;
	reg |= CCC_CPUx_CYC_OVRD_ok2pwrdn_POWERDN;
	rCCC_CPU2_CYC_OVRD = reg;

	reg = rCCC_CPU2_DGB_WRAP | CCC_CPUx_DGB_WRAP_frcPwrDn;
	rCCC_CPU2_DGB_WRAP = reg;

	// Wait for CPU1/CPU2 to power down.
	for (i = 0; i < RAM_REPAIR_TIMEOUT_COUNT; i++) {
		if ((rCCC_CCC_CCC_STS & CCC_CCC_CCC_STS_cc2St_MASK) == 0) {
			break;
		}
	}

	// Failure of CPU1/CPU2 to power down up means death.
	if ((rCCC_CCC_CCC_STS & CCC_CCC_CCC_STS_cc2St_MASK) != 0) {
		panic("CPU1 and/or CPU2 failed to power down");
	}

	// Restore the state of all the registers modified above.
	rCCC_CPU2_DGB_WRAP = dbg_wrap2;
	rCCC_CPU1_DGB_WRAP = dbg_wrap1;
	rCCC_CPU2_CYC_OVRD = cyc_ovrd2;
	rCCC_CPU1_CYC_OVRD = cyc_ovrd1;
	rCCC_CCC_CCC_OVRD  = ccc_ovrd;
}
#endif	// SUB_PLATFORM_T7001 && (PRODUCT_LLB || PRODUCT_IBSS)

