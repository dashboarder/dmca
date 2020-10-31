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
#if WITH_CONSISTENT_DBG
#include <drivers/consistent_debug.h>
#endif
#include <drivers/csi.h>
#include <drivers/display.h>
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
#include <drivers/reconfig.h>
#include <lib/env.h>
#include <lib/nonce.h>
#include <lib/paint.h>
#include <platform.h>
#include <platform/apcie.h>
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
#include <platform/soc/reconfig.h>
#include <platform/soc/aop_config_sequences.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <target.h>

static void platform_init_boot_strap(void);
static void platform_bootprep_darwin(void);
static int power_convert_buck_to_rail(int buck, int *rail);
static void power_get_buck_value_fpga(int buck, uint32_t mv, uint32_t *val);
static int power_convert_dwi_to_mv_fpga(unsigned int buck, uint32_t dwival);

static void platform_awake_to_aop_ddr_pre_sequence_insert();
static void platform_awake_to_aop_ddr_post_sequence_insert();
static void platform_aop_ddr_to_s2r_aop_pre_sequence_insert();
static void platform_s2r_aop_to_aop_ddr_post_sequence_insert();
static void platform_aop_ddr_to_awake_pre_sequence_insert();
static void platform_aop_ddr_to_awake_post_sequence_insert();

static uint8_t boot_debug;
static bool gDisplayEnabled;

int platform_early_init(void)
{
#if (PRODUCT_LLB || PRODUCT_IBSS)
	/* Verify that the fuses and SecureROM R/W access has been disabled */
	if (!chipid_get_fuse_lock() || (((*(volatile uint32_t *)SECURITY_REG) & ROM_READ_DISABLE) == 0)) {
		panic("Fuses are unlocked or SecureROM is enabled\n");
	}

	/* Verify that the fuses are sealed on production devices. */
	if (chipid_get_current_production_mode() && !chipid_get_fuse_seal()) {
		panic("Fuses are not sealed\n");
	}
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

#if WITH_HW_DCS
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
	RELEASE_ASSERT(!resume);

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
			clock_gate(CLK_RTMUX, true);
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


int platform_init_mass_storage_panic(void)
{
#if WITH_NVME
	return nvme_init_mass_storage_panic(0);
#else
	return 0;
#endif
}

int platform_init_mass_storage(void)
{
#if WITH_NVME
	return nvme_init_mass_storage(0);
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

#if WITH_NVME
	nvme_quiesce_all();
#endif

#if WITH_HW_APCIE
	apcie_disable_all();
#endif

	switch (target) {
		case BOOT_HALT:
			break;
		case BOOT_SECUREROM:
		case BOOT_DARWIN_RESTORE:
			panic("not supported");
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

	clock_gate(CLK_RTMUX, true);

	/* quiesce the panel */
	if (display_quiesce(true) == ENXIO) {
		clock_gate(CLK_RTMUX, false);
	}
#endif
	gDisplayEnabled = false;

	return 0;
}

static bool need_l2_ram_disabled(void)
{
#if PRODUCT_IBSS || PRODUCT_LLB
	// If the current product is iBSS or LLB we're currently running out
	// of L2-as-RAM and therefore can't disable it.
	return false;
#else
	return true;
#endif
}

int platform_bootprep(enum boot_target target)
{
	uint32_t gids = ~0, uids = ~0;	/* leave crypto keys alone by default */
	bool	l2_ram_disable = false;

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
			panic("not supported");
			break;

#if WITH_BOOT_XNU
		case BOOT_DARWIN:
			platform_bootprep_darwin();
			if (boot_debug & kPowerNVRAMiBootDebugWDTWake)
				wdt_enable();
			/* even when trusted, Darwin only gets the UID / GID1 */
			uids = 1;
			gids = 2;
			l2_ram_disable = true;
			break;
#endif // WITH_BOOT_XNU

		case BOOT_DIAGS:
#if WITH_SIDP
			/* Seal the manifest that authorizes diags. */
			security_sidp_seal_boot_manifest(false);
#endif
			l2_ram_disable = need_l2_ram_disabled();
			platform_quiesce_display();
#if WITH_BOOT_STAGE
			boot_clear_error_count();
#endif
			break;

		case BOOT_IBOOT:
			l2_ram_disable = need_l2_ram_disabled();
			platform_quiesce_display();
			break;
#endif // APPLICATION_IBOOT

		case BOOT_UNKNOWN:
			platform_quiesce_display();
			break;

		case BOOT_SECUREROM:
		case BOOT_MONITOR:
		default:
			panic("unknown boot target %d", target);
	}

	// Make sure L2-as-RAM is disabled if required.
	if (l2_ram_disable) {
		uint64_t l2_cramconfig = arm_read_l2_cramconfig();
		if ((l2_cramconfig & 0x3F) != 0) {
			panic("SRAM is still enabled");
		}
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

#if	WITH_SIDP

	/* The Silicon Data Protection ROM manifest must be locked. */
	if (!platform_sidp_rom_manifest_locked()) {
		panic("SiDP ROM manifest not locked");
	}

#if	!APPLICATION_SECUREROM

	/* The Silicon Data Protection boot manifest must be locked. */
	if ((target != BOOT_IBOOT) && !platform_sidp_boot_manifest_locked()) {
		panic("SiDP boot manifest not locked");
	}

#endif	// !APPLICATION_SECUREROM
#endif	// WITH_SIDP

	/* Disable Cyclone specific errors enabled earlier in the boot */
	ccc_disable_custom_errors();

	return 0;
}

void platform_mmu_setup(bool resume)
{
	RELEASE_ASSERT(!resume);

#if APPLICATION_SECUREROM
	// ROM is obviously read/execute since it's ROM
	arm_mmu_map_rx(VROM_BASE, VROM_BANK_LEN);

	// everything ROM needs in SRAM is read-write,
	// except the trampoline which is read-execute
	arm_mmu_map_rw(INSECURE_MEMORY_BASE, INSECURE_MEMORY_SIZE);
	arm_mmu_map_rx(BOOT_TRAMPOLINE_BASE, BOOT_TRAMPOLINE_SIZE);
	arm_mmu_map_rw(DATA_BASE, DATA_SIZE);
	// skip page tables here
	arm_mmu_map_rw(STACKS_BASE, STACKS_SIZE);
	arm_mmu_map_rw(HEAP_BASE, HEAP_SIZE);

#else	// APPLICATION_IBOOT
	// Figure out where the linker put our various bits
	uintptr_t text_end_aligned = ((uintptr_t)&_text_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	size_t text_size = text_end_aligned - (uintptr_t)&_text_start;

	RELEASE_ASSERT(text_end_aligned <= (uintptr_t)&_data_start);

#if PRODUCT_LLB || PRODUCT_IBSS
	arm_mmu_map_rw(SDRAM_BASE, SDRAM_LEN);

	// Map read/write SRAM regions
	// Don't map the page tables so that they can't be modified
	arm_mmu_map_rw(SRAM_BASE, VROM_RSVD);
	arm_mmu_map_rw(STACKS_BASE, STACKS_SIZE);
	arm_mmu_map_rw(HEAP_BASE, HEAP_SIZE);

	// __TEXT is read/execute, __DATA is read/write
	arm_mmu_map_rx(BOOT_TRAMPOLINE_BASE, BOOT_TRAMPOLINE_SIZE);
	arm_mmu_map_rx((uintptr_t)&_text_start, text_size);
	arm_mmu_map_rw(text_end_aligned, CODE_DATA_END - text_end_aligned);
#else	// PRODUCT_IBEC || PRODUCT_IBOOT
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
	arm_mmu_map_rw(CODE_DATA_END, SDRAM_END - (CODE_DATA_END));
#endif	// PRODUCT_LLB || PRODUCT_IBSS
#endif	// APPLICATION_SECUREROM

	// map IO
	arm_mmu_map_device_rw(IO_BASE, IO_SIZE);
	arm_mmu_map_device_rw(PCI_REG_BASE, PCI_REG_LEN);
	arm_mmu_map_device_rw(PCI_CONFIG_BASE, PCI_CONFIG_LEN);
	arm_mmu_map_device_rw(PCI_32BIT_BASE, PCI_32BIT_LEN);
}

int platform_init(void)
{
#if defined(UNUSED_MEMORY_BASE) && (UNUSED_MEMORY_SIZE > 0)
	bzero((void *)UNUSED_MEMORY_BASE, UNUSED_MEMORY_SIZE);
#endif

#if WITH_CONSISTENT_DBG && (PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)
	consistent_debug_init();
#endif

#if WITH_PLATFORM_ERROR_HANDLER
	platform_enable_error_handler();
#endif

#if PRODUCT_IBEC && WITH_SIDP
	// The code that authorized iBEC must have locked the boot manifest.
	RELEASE_ASSERT(platform_sidp_boot_manifest_locked());
#endif

#if WITH_HW_SPI
	spi_init();
#endif

#if WITH_PCI
	pci_init();
#endif

#if WITH_NVME && (PRODUCT_LLB || PRODUCT_IBOOT || PRODUCT_IBEC || WITH_RECOVERY_MODE_IBSS)
#if PRODUCT_IBEC
	apcie_set_s3e_mode(true);
#else
	apcie_set_s3e_mode(false);
#endif
	if (apcie_enable_link(0)) {
		nvme_init(0, apcie_get_port_bridge(0), 0);
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

	// Find the mcc node
	if (FindNode(0, "arm-io/mcc", &node)) {
		propName = "dcs_num_channels";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*(uint32_t *)propData = DCS_NUM_CHANNELS;
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

	// Find the apcie node
#if WITH_HW_APCIE
	if (FindNode(0, "arm-io/apcie", &node)) {
		apcie_update_devicetree(node);
	}
#endif

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

#if WITH_NVME
	if (FindNode(0, "arm-io/nvme-mmu0", &node)) {
		uintptr_t *sart_region = NULL;
		uint32_t *sart_virt_base = NULL;
		uint64_t *nvme_scratch_region = NULL;

		propName = "sart-region";
		if (FindProperty(node, &propName, &propData, &propSize) && propSize == 2 * sizeof(uintptr_t)) {
			sart_region = (uintptr_t *)propData;
		}

		propName = "sart-virtual-base";
		if (FindProperty(node, &propName, &propData, &propSize) && propSize == sizeof(uint32_t)) {
			sart_virt_base = (uint32_t *)propData;
		}

		if (FindNode(0, "arm-io/apcie/pci-bridge0/s3e", &node)) {
			propName = "nvme-scratch-virt-region";
			if (FindProperty(node, &propName, &propData, &propSize) && propSize == 2 * sizeof(uint64_t)) {
				nvme_scratch_region = propData;
			}
		}

		if (sart_region != NULL && sart_virt_base != NULL && nvme_scratch_region != NULL) {
			// Tell PCIe driver to point the SART at the scratch buffer
			sart_region[0] = platform_get_memory_region_base(kMemoryRegion_StorageProcessor);
			sart_region[1] = platform_get_memory_region_size(kMemoryRegion_StorageProcessor);
			// Tell NVMe driver where the SART is putting the scratch buffer
			nvme_scratch_region[0] = *sart_virt_base;
			nvme_scratch_region[1] = sart_region[1];
		}
	}
#endif

	if (dt_find_node(NULL, "arm-io/aop", &node)) {
		addr_t aop_region_base = platform_get_memory_region_base(kMemoryRegion_AOP);
		addr_t aop_region_size = platform_get_memory_region_size(kMemoryRegion_AOP);

		// Find the reg that maps the AOP SRAM and adjust it to account
		// for the SRAM used by the reconfig region
		bool updated = false;
		if (dt_find_prop(node, "reg", &propData, &propSize)) {
			uintptr_t *regs = (uintptr_t *)propData;
			unsigned num_regs = propSize / (sizeof(*regs) * 2);
			for (unsigned i = 0; i < num_regs; i++) {
				if (regs[i * 2] == aop_region_base - IO_BASE) {
					regs[i * 2 + 1] = aop_region_size;
					updated = true;
					break;
				}
			}
		}

		if (!updated) panic("failed to update AOP's reg property");

		if (dt_find_node(node, "iop-aop-nub", &node)) {
			dt_set_prop_addr(node, "region-base", aop_region_base);
			dt_set_prop_addr(node, "region-size", aop_region_size);
		} else {
			panic("failed to find AOP nub node");
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

	/* T8010 supports one boot device then USB-DFU per boot config */

	switch (boot_config) {
		case BOOT_CONFIG_SPI0:
			*boot_device = BOOT_DEVICE_SPI;
			*boot_flag = 0;
			*boot_arg = 0;
			break;

		case BOOT_CONFIG_SPI0_TEST:
		case BOOT_CONFIG_FAST_SPI0_TEST:
		case BOOT_CONFIG_SLOW_SPI0_TEST:
			*boot_device = BOOT_DEVICE_SPI;
			*boot_flag = BOOT_FLAG_TEST_MODE;
			*boot_arg = 0;
			break;

		case BOOT_CONFIG_NVME0_X1:
		case BOOT_CONFIG_NVME0_X2:
			*boot_device = BOOT_DEVICE_NVME;
			*boot_flag = 0;
			*boot_arg = 0;
			break;

		case BOOT_CONFIG_NVME0_X1_TEST:
		case BOOT_CONFIG_NVME0_X2_TEST:
			*boot_device = BOOT_DEVICE_NVME;
			*boot_flag = BOOT_FLAG_TEST_MODE;
			*boot_arg = 0;
			break;

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
			if (boot_arg != 0) {
				panic("invalid NVMe boot argument %u", boot_arg);
			}

			if (enable) {
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

uint32_t platform_get_apcie_lane_cfg(void)
{
	uint32_t boot_config = platform_get_boot_config();

	switch (boot_config) {
		case BOOT_CONFIG_NVME0_X1:
		case BOOT_CONFIG_NVME0_X1_TEST:
			return APCIE_LANE_CONFIG_NVME0_X1;

		case BOOT_CONFIG_NVME0_X2:
		case BOOT_CONFIG_NVME0_X2_TEST:
			return APCIE_LANE_CONFIG_NVME0_X2;

		default:
			panic("unknown pcie boot config 0x%x", boot_config);
	}
}

// Get the expected link width (number of lanes) for the given PCIe link
// A value of 0 means to auto-negotiate (and has no effect for ports
// that only support x1 mode in the hardware)
uint32_t platform_get_pcie_link_width(uint32_t link)
{
	uint32_t boot_config = platform_get_boot_config();

	switch (boot_config) {
		case BOOT_CONFIG_NVME0_X1:
		case BOOT_CONFIG_NVME0_X1_TEST:
			return 1;

		default:
			return 0;
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
		case BOOT_CONFIG_FAST_SPI0_TEST:
			return 24000000;

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
	uint32_t keydis = (((gid & 0x3) << 1) | ((uid & 0x1) << 0));

	dprintf(DEBUG_SPEW, "gid:0x%08x uid:0x%08x\n", gid, uid);

	if (!gid && !uid) return;

	rMINIPMGR_SIO_AES_DISABLE = keydis;

}

bool platform_keys_disabled(uint32_t gid, uint32_t uid)
{
	uint32_t key_disabled;

	dprintf(DEBUG_SPEW, "gid:0x%08x uid:0x%08x\n", gid, uid);

	if (!gid && !uid) return true;

	key_disabled = rMINIPMGR_SIO_AES_DISABLE;

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
	panic("not supported");
}

void platform_asynchronous_exception(void)
{
	ccc_handle_asynchronous_exception();
}

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

#if	WITH_SIDP

// Returns true if Silicon Data Protection ROM manifest is locked.
bool platform_sidp_rom_manifest_locked(void)
{
	return (rMINIPMGR_SEAL_CTL_A & MINIPMGR_SECURITY_SEAL_CTL_A_LOCK_UMASK) != 0;
}

// Returns true if Silicon Data Protection boot manifest is locked.
bool platform_sidp_boot_manifest_locked(void)
{
	return (rMINIPMGR_SEAL_CTL_B & MINIPMGR_SECURITY_SEAL_CTL_B_LOCK_UMASK) != 0;
}

#if	APPLICATION_SECUREROM

// Lock the Silicon Data Protection ROM manifest
void platform_sidp_lock_rom_manifest(void)
{
	// Can't already be locked.
	RELEASE_ASSERT(!platform_sidp_rom_manifest_locked());
	rMINIPMGR_SEAL_CTL_A |= MINIPMGR_SECURITY_SEAL_CTL_A_LOCK_UMASK;
}

// Set the Silicon Data Protection ROM manifest
void platform_sidp_set_rom_manifest(const uint32_t *manifest_hash, size_t manifest_hash_size)
{
	// Can't already be locked.
	RELEASE_ASSERT(!platform_sidp_rom_manifest_locked());

	volatile uint32_t *seal_reg = &rMINIPMGR_SEAL_DATA_A_FIRST;

	if (manifest_hash == NULL) {
		// Clear the seal data A registers
		while (seal_reg <= &rMINIPMGR_SEAL_DATA_A_LAST) {
			*seal_reg++ = 0;
		}

		// Mark the seal as invalid.
		rMINIPMGR_SEAL_CTL_A &= ~MINIPMGR_SECURITY_SEAL_CTL_A_VALID_UMASK;

	} else {
		const uint32_t *manifest_end = manifest_hash + (manifest_hash_size / sizeof(uint32_t));

		// Copy the manifest hash to seal data A registers.
		do {
			// Copy the manifest to seal A registers. If we run out of
			// manifest before we run out of seal regsiters, store zero.
			if (manifest_hash < manifest_end) {
				*seal_reg++ = *manifest_hash++;
			} else {
				*seal_reg++ = 0;
			}
		} while (seal_reg <= &rMINIPMGR_SEAL_DATA_A_LAST);

		// Mark the seal data as valid.
		rMINIPMGR_SEAL_CTL_A |= MINIPMGR_SECURITY_SEAL_CTL_A_VALID_UMASK;
	}
}

#else	// !APPLICATION_SECUREROM

// Lock the Silicon Data Protection boot manifest
void platform_sidp_lock_boot_manifest(void)
{
	// Can't already be locked.
	RELEASE_ASSERT(!platform_sidp_boot_manifest_locked());
	rMINIPMGR_SEAL_CTL_B |= MINIPMGR_SECURITY_SEAL_CTL_B_LOCK_UMASK;

	dprintf(DEBUG_SPEW, "SiDP boot manifest locked\n");
}

// Set the Silicon Data Protection boot manifest
void platform_sidp_set_boot_manifest(const uint32_t *manifest_hash, size_t manifest_hash_size)
{
	// Can't already be locked.
	RELEASE_ASSERT(!platform_sidp_boot_manifest_locked());

	volatile uint32_t *seal_reg = &rMINIPMGR_SEAL_DATA_B_FIRST;

	if (manifest_hash == NULL) {
		// Clear the seal data B registers
		while (seal_reg <= &rMINIPMGR_SEAL_DATA_B_LAST) {
			*seal_reg++ = 0;
		}

		// Mark the seal as invalid.
		rMINIPMGR_SEAL_CTL_B &= ~MINIPMGR_SECURITY_SEAL_CTL_B_VALID_UMASK;

		dprintf(DEBUG_SPEW, "SiDP boot manifest marked invalid\n");

	} else {
		const uint32_t *manifest_end = manifest_hash + (manifest_hash_size / sizeof(uint32_t));

		// Copy the manifest hash to seal data B registers.
		do {
			// Copy the manifest to seal B registers. If we run out of
			// manifest before we run out of seal regsiters, store zero.
			if (manifest_hash < manifest_end) {
				*seal_reg++ = *manifest_hash++;
			} else {
				*seal_reg++ = 0;
			}
		} while (seal_reg <= &rMINIPMGR_SEAL_DATA_B_LAST);

		// Mark the seal data as valid.
		rMINIPMGR_SEAL_CTL_B |= MINIPMGR_SECURITY_SEAL_CTL_B_VALID_UMASK;

		dprintf(DEBUG_SPEW, "SiDP boot manifest marked valid\n");
	}
}

// Set the Silicon Data Protection mix-n-match attribute
void platform_sidp_set_mix_n_match(void)
{
	rMINIPMGR_SET_ONLY |= MINIPMGR_SET_ONLY_MIX_N_MATCH;

	dprintf(DEBUG_SPEW, "SiDP boot manifest min-n-match state set\n");
}

#endif	// APPLICATION_IBOOT
#endif	// WITH_SIDP

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
	rMINIPMGR_SCRATCH1 = root;
}

int platform_convert_voltages(int buck, uint32_t count, uint32_t *voltages)
{
#if SUPPORT_FPGA
	uint32_t index;
	for (index = 0; index < count; index++)
		power_get_buck_value_fpga(buck, voltages[index], &voltages[index]);
	return 0;
#elif WITH_HW_POWER
	int rail;
	uint32_t index;

	if (voltages == 0) return -1;

	if (0 != power_convert_buck_to_rail(buck, &rail))
		return -1;

	for (index = 0; index < count; index++) {
		if (voltages[index] == 0)
			continue;

		if (0 != power_get_rail_value(rail, voltages[index], &voltages[index]))
			return -1;
	}

	return 0;
#else
	return -1;
#endif
}

int platform_get_cpu_voltages(uint32_t count, uint32_t *voltages)
{
	uint32_t cnt;

	if (voltages == 0) return -1;

	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = chipid_get_cpu_voltage(cnt);
	}

	return 0;
}

int platform_get_soc_voltages(uint32_t count, uint32_t *voltages)
{
	uint32_t cnt;

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

		case kMemoryRegion_Kernel:
			base = TZ0_BASE + TZ0_SIZE;
			// XXX: kernel must be loaded at a 32 MB aligned address. We'll want to reclaim this
			// memory, but for now, just throw it away
			// <rdar://problem/15766644> Maui: Align kernel to 32 MB without wasting memory
			base += 0x2000000-1;
			base &= ~(0x2000000ULL - 1);
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

		case kMemoryRegion_DramConfig:
			base = DRAM_CONFIG_SEQ_BASE;
			break;

		case kMemoryRegion_Display:
			base = PANIC_BASE - DISPLAY_SIZE;
			break;

		case kMemoryRegion_iBoot:
			base = IBOOT_BASE;
			break;

		case kMemoryRegion_AOP:
			base = AOP_SRAM_BASE_ADDR;
			break;

		case kMemoryRegion_Reconfig:
			base = AOP_RECONFIG_REGION_BASE_ADDR;
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

		case kMemoryRegion_DramConfig:
			size = DRAM_CONFIG_SEQ_SIZE;
			break;

		case kMemoryRegion_Display:
			size = DISPLAY_SIZE;
			break;

		case kMemoryRegion_iBoot:
			size = IBOOT_SIZE;
			break;

		case kMemoryRegion_AOP:
			size = AOP_RECONFIG_REGION_BASE_ADDR - AOP_SRAM_BASE_ADDR;
			break;

		case kMemoryRegion_Reconfig:
			size = AOP_RECONFIG_REGION_SIZE;
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

int platform_get_cpu_ram_voltages(uint32_t count, uint32_t *voltages)
{
	uint32_t cnt;

	if (voltages == 0)	return -1;

	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = chipid_get_cpu_sram_voltage(cnt);
	}

	return 0;
}

int platform_get_gpu_voltages(uint32_t count, uint32_t *voltages)
{
	uint32_t cnt;

	if (voltages == 0)	return -1;

	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = chipid_get_gpu_voltage(cnt);
	}

	return 0;
}

int platform_get_gpu_ram_voltages(uint32_t count, uint32_t *voltages)
{
	uint32_t cnt;

	if (voltages == 0)	return -1;

	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = chipid_get_gpu_sram_voltage(cnt);
	}

	return 0;
}

int platform_get_ram_voltages(uint32_t count, uint32_t *voltages)
{
	uint32_t cnt;

	if (voltages == 0)	return -1;

	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = chipid_get_sram_voltage(cnt);
	}

	return 0;
}

/*
 *	Gets equivalent mV for a given DWI value for a particular buck.
 *	PMU code should handle the special case for GPU buck.
 */
int platform_get_dwi_to_mv(int buck, uint32_t dwival)
{
#if SUPPORT_FPGA
	return power_convert_dwi_to_mv_fpga(buck, dwival);
#elif WITH_HW_POWER
	int rail;

	if (0 != power_convert_buck_to_rail(buck, &rail))
		return -1;

	return power_convert_dwi_to_mv(rail, dwival);
#else
	return -1;
#endif
}

static void platform_init_boot_strap(void)
{
	volatile uint32_t boot_strap, chip_board_id, gpio_board_id, boot_config;

	// If rPMGR_SCRATCH0[0] set then boot strap already valid
	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagBootStrap) != 0) return;

	gpio_configure(GPIO_BOOT_CONFIG0, GPIO_CFG_IN);
	gpio_configure(GPIO_BOOT_CONFIG1, GPIO_CFG_IN);
	gpio_configure(GPIO_BOOT_CONFIG2, GPIO_CFG_IN);

	gpio_configure_pupdn(GPIO_BOOT_CONFIG0, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOOT_CONFIG1, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOOT_CONFIG2, GPIO_PDN);

	platform_power_spin(100); // Wait 100us

	chip_board_id = chipid_get_board_id();
	gpio_board_id = gpio_get_board_id();

	boot_config =
		(gpio_read(GPIO_BOOT_CONFIG2) << 2) |
		(gpio_read(GPIO_BOOT_CONFIG1) << 1) |
		(gpio_read(GPIO_BOOT_CONFIG0) << 0);

	gpio_configure(GPIO_BOOT_CONFIG0, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOOT_CONFIG1, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOOT_CONFIG2, GPIO_CFG_DFLT);

	boot_strap = (((chip_board_id << 5) | gpio_board_id) << 16) |
		     (boot_config << 8) |
		     (0x01 << 0);

	rPMGR_SCRATCH0 = (rPMGR_SCRATCH0 & 0xFF000000) | (boot_strap & 0x00FFFFFF);
}

#if WITH_BOOT_XNU
static void platform_bootprep_darwin(void)
{
#if PRODUCT_IBOOT || PRODUCT_IBEC
       extern addr_t   gKernelEntry;   // Kernel entry point

#if WITH_PAINT
	if (paint_color_map_is_invalid())
		panic("Previous DClr errors prevent OS booting");
#endif

/* 1. Setup trustzones */

	/* clean-invalidate TZ0 region */
	platform_cache_operation((CACHE_CLEAN | CACHE_INVALIDATE), (void *)TZ0_BASE, TZ0_SIZE);

	/* create sleep token */
	/* already created by load_kernelcache call */

	/* program values */
	for (uint32_t i = 0; i < NUM_AMCCS; i++) {
		rMCCLOCKREGION_TZ0BASEADDR(i) = TZ0_BASE >> 12;
		rMCCLOCKREGION_TZ0ENDADDR(i)  = (TZ0_BASE + TZ0_SIZE - 1) >> 12;

		/* Lock TZ0 region. */
		rMCCLOCKREGION_TZ0LOCK(i) = AMCC_MCC_LOCK_REGION_TZ0_LOCK_FLD_UMASK;
		if ((rMCCLOCKREGION_TZ0LOCK(i) & AMCC_MCC_LOCK_REGION_TZ0_LOCK_FLD_UMASK) == 0) {
			panic("TZ0 failed to lock\n");
		}
	}

	/* 2. Override IO_RVBAR - the kernel guarantees that it's reset vector
	      is at the beginning of the 4KB (yes, I really mean 4KB) page
	      containing the kernel entry point. */
	ccc_override_and_lock_iorvbar(gKernelEntry & ~0xfff);

#if WITH_SIDP
	/* Seal the manifest that authorizes the kernelcache. */
	security_sidp_seal_boot_manifest(false);
#endif

#if 0	// !!!FIXME!!! <rdar://problem/19192084> H9: Implement AOP_S2R suspend/resume
	// Update the calibration values in the sequence with values that were saved to AOP_SRAM
	extern void dcs_restore_calibration_results(volatile uint32_t *save_region, uint32_t *sequence);
	dcs_restore_calibration_results((volatile uint32_t *) MEMORY_CALIB_SAVE_BASE_ADDR, mcu_aop_awake_reconfig_restore);
	// Insert Reconfig sequences
	reconfig_init();
	platform_awake_to_aop_ddr_pre_sequence_insert();
	platform_awake_to_aop_ddr_post_sequence_insert();
	platform_aop_ddr_to_s2r_aop_pre_sequence_insert();
	platform_s2r_aop_to_aop_ddr_post_sequence_insert();
	platform_aop_ddr_to_awake_pre_sequence_insert();
	platform_aop_ddr_to_awake_post_sequence_insert();

	/* XXX 
	    - tunable values
	    - timeouts during state transitions
	    - lockdown regions
	*/
#endif	// !!!FIXME!!!
#else	// !(PRODUCT_IBOOT || PRODUCT_IBEC)
       // Booting the kernel is not supported from anything except iBoot and iBEC.
       panic("Kernel booting not supported");
#endif	// PRODUCT_IBOOT || PRODUCT_IBEC
}
#endif	// WITH_BOOT_XNU

uint32_t platform_get_pcie_l1ss_ltr_threshold(void)
{
	// Per Cayman tunables spec r0.11 (16 Apr 2015), section 7.2 3(a).
	return 85;
}

uint32_t platform_get_pcie_l1ss_t_common_mode(void)
{
	// Per Cayman tunables spec r0.11 (16 Apr 2015), section 7.2 2(b).
	return 0;
}

static int power_convert_buck_to_rail(int buck, int *rail)
{
	if (!rail)
		return -1;

	switch (buck) {
#if APPLICATION_IBOOT
#ifndef BUCK_CPU
#error BUCK_CPU not defined for this platform
#endif

#ifndef BUCK_CPU_RAM
#error BUCK_CPU_RAM not defined for this platform
#endif

#ifndef BUCK_SOC
#error BUCK_SOC not defined for this platform
#endif

#ifndef BUCK_GPU
#error BUCK_GPU not defined for this platform
#endif

#ifndef BUCK_GPU_RAM
#error BUCK_GPU_RAM not defined for this platform
#endif

		case BUCK_CPU:
			*rail = POWER_RAIL_CPU;
			break;
		case BUCK_CPU_RAM:
			*rail = POWER_RAIL_CPU_RAM;
			break;
		case BUCK_SOC:
			*rail = POWER_RAIL_SOC;
			break;
		case BUCK_GPU:
			*rail = POWER_RAIL_GPU;
			break;
		case BUCK_GPU_RAM:
			*rail = POWER_RAIL_GPU_RAM;
			break;
#endif
		default:
			return -1;
	}

	return 0;
}

#if SUPPORT_FPGA
static void power_get_buck_value_fpga(int buck, uint32_t mv, uint32_t *val)
{
	if (mv == 0) {
		*val = 0;
	} else {
		switch(buck) {
#if APPLICATION_IBOOT
			case BUCK_CPU:
			case BUCK_GPU:
				*val = (((mv-500)*1000)+3124)/3125;
				break;
#endif

			default:
				*val = (((mv-600)*1000)+3124)/3125;
				break;
		}
	}
	return;
}

static int power_convert_dwi_to_mv_fpga(unsigned int buck, uint32_t dwival)
{
	int val = 0;
#if APPLICATION_IBOOT
	if (buck == BUCK_GPU) {
		val = (dwival == 0) ? 0 : (500000 + (3125 * dwival))/1000;
	}
#endif
	return val;
}
#endif

#if WITH_BOOT_XNU
static void platform_awake_to_aop_ddr_pre_sequence_insert()
{
	reconfig_command_raw(AWAKE_AOP_DDR_PRE, awake_to_aop_ddr_pre, sizeof(awake_to_aop_ddr_pre)/sizeof(uint32_t));
	reconfig_commit(AWAKE_AOP_DDR_PRE);
}

static void platform_awake_to_aop_ddr_post_sequence_insert()
{
	reconfig_command_raw(AWAKE_AOP_DDR_POST, awake_to_aop_ddr_post, sizeof(awake_to_aop_ddr_post)/sizeof(uint32_t));
	reconfig_commit(AWAKE_AOP_DDR_POST);

}

static void platform_aop_ddr_to_s2r_aop_pre_sequence_insert()
{
	reconfig_command_raw(AOP_DDR_S2R_AOP_PRE, aop_ddr_to_s2r_aop_pre, sizeof(aop_ddr_to_s2r_aop_pre)/sizeof(uint32_t));
	reconfig_commit(AOP_DDR_S2R_AOP_PRE);

}

static void platform_s2r_aop_to_aop_ddr_post_sequence_insert()
{
	reconfig_command_raw(S2R_AOP_AOP_DDR_POST, s2r_aop_to_aop_ddr_post, sizeof(s2r_aop_to_aop_ddr_post)/sizeof(uint32_t));

	// TZ lock region inserted here
	for (uint32_t i = 0; i < NUM_AMCCS; i++) {
		reconfig_command_write(S2R_AOP_AOP_DDR_POST, MCCLOCKREGION_TZ0BASEADDR(i), TZ0_BASE>>12, 0);
		reconfig_command_write(S2R_AOP_AOP_DDR_POST, MCCLOCKREGION_TZ0ENDADDR(i), (TZ0_BASE + TZ0_SIZE - 1)>>12, 0);
		reconfig_command_write(S2R_AOP_AOP_DDR_POST, MCCLOCKREGION_TZ0LOCK(i), 1, 0);
		reconfig_command_read(S2R_AOP_AOP_DDR_POST, MCCLOCKREGION_TZ0LOCK(i), 1, 1, 255, 0);
	}

	reconfig_command_raw(S2R_AOP_AOP_DDR_POST, s2r_aop_to_aop_ddr_post_2, sizeof(s2r_aop_to_aop_ddr_post_2)/sizeof(uint32_t));
	reconfig_command_raw(S2R_AOP_AOP_DDR_POST, mcu_aop_ddr_reconfig, sizeof(mcu_aop_ddr_reconfig)/sizeof(uint32_t));
	reconfig_command_raw(S2R_AOP_AOP_DDR_POST, s2r_aop_to_aop_ddr_post_3, sizeof(s2r_aop_to_aop_ddr_post_3)/sizeof(uint32_t));
	reconfig_commit(S2R_AOP_AOP_DDR_POST);

}

static void platform_aop_ddr_to_awake_pre_sequence_insert()
{
	reconfig_command_raw(AOP_DDR_AWAKE_PRE, aop_ddr_to_awake_pre, sizeof(aop_ddr_to_awake_pre)/sizeof(uint32_t));
	reconfig_commit(AOP_DDR_AWAKE_PRE);

}

static void platform_aop_ddr_to_awake_post_sequence_insert()
{
#if !SUPPORT_FPGA
	reconfig_command_raw(AOP_DDR_AWAKE_POST, mcu_aop_awake_reconfig_pre_restore, sizeof(mcu_aop_awake_reconfig_pre_restore)/sizeof(uint32_t));
	reconfig_command_raw(AOP_DDR_AWAKE_POST, mcu_aop_awake_reconfig_restore, sizeof(mcu_aop_awake_reconfig_restore)/sizeof(uint32_t));
	reconfig_command_raw(AOP_DDR_AWAKE_POST, mcu_aop_awake_reconfig_post_restore, sizeof(mcu_aop_awake_reconfig_post_restore)/sizeof(uint32_t));
#endif
	reconfig_command_raw(AOP_DDR_AWAKE_POST, aop_ddr_to_awake_post, sizeof(aop_ddr_to_awake_post)/sizeof(uint32_t));

	reconfig_command_write(AOP_DDR_AWAKE_POST, CCC_CPU0_SYS_BASE_ADDR + ACC_CPU0_IMPL_IO_RVBAR_OFFSET, (TZ0_BASE + TZ0_SIZE), 1);

	reconfig_command_raw(AOP_DDR_AWAKE_POST, aop_ddr_to_awake_post_2, sizeof(aop_ddr_to_awake_post_2)/sizeof(uint32_t));
	reconfig_commit(AOP_DDR_AWAKE_POST);

}
#endif // WITH_BOOT_XNU

#if WITH_TARGET_CONFIG

// The target's rules.mk sets the high SoC voltage point
#ifndef TARGET_BOOT_SOC_VOLTAGE
#error TARGET_BOOT_SOC_VOLTAGE not defined by the target
#endif

uint32_t platform_get_base_soc_voltage(void)
{
	return chipid_get_soc_voltage(TARGET_BOOT_SOC_VOLTAGE);
}

// The target's rules.mk sets the high CPU voltage point
#ifndef TARGET_BOOT_CPU_VOLTAGE
#error TARGET_BOOT_CPU_VOLTAGE not defined by the target
#endif

uint32_t platform_get_base_cpu_voltage(void)
{
	return chipid_get_cpu_voltage(TARGET_BOOT_CPU_VOLTAGE);
}

// The target's rules.mk sets the high RAM voltage point
#ifndef TARGET_BOOT_RAM_VOLTAGE
#error TARGET_BOOT_RAM_VOLTAGE not defined by the target
#endif

uint32_t platform_get_base_ram_voltage(void)
{
	return chipid_get_sram_voltage(TARGET_BOOT_RAM_VOLTAGE);
}

#endif

#if defined(WITH_MENU) && WITH_MENU

static int get_total_leakage(int argc, struct cmd_arg *args)
{
	dprintf(DEBUG_INFO, "total leakage: %dmA\n", chipid_get_total_rails_leakage());
	return 0;
}
MENU_COMMAND_DEBUG(leakage, get_total_leakage, "total rails leakage read from fuse", NULL);
#endif

