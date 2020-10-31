/*
 * Copyright (C) 2010-2015 Apple Inc. All rights reserved.
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
#include <drivers/display.h>
#include <drivers/flash_nor.h>
#include <drivers/flash_nand.h>
#include <drivers/iic.h>
#include <drivers/nand_boot.h>
#include <drivers/nand_device_tree.h>
#include <drivers/power.h>
#include <drivers/shmcon.h>
#include <drivers/spi.h>
#include <drivers/uart.h>
#include <drivers/usb/usb_public.h>
#include <drivers/usbphy.h>
#include <lib/devicetree.h>
#include <lib/env.h>
#include <lib/image.h>
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
#include <platform/trampoline.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/chipid.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <sys.h>
#include <sys/boot.h>
#include <target.h>
#include <drivers/miu.h>
#if WITH_EFFACEABLE_NOR
#include <lib/effaceable_nor.h>
#endif

extern void dma_init(void);

static void platform_init_boot_strap(void);

static uint8_t boot_debug;

int platform_early_init(void)
{
#if PRODUCT_LLB || PRODUCT_IBSS
	/* Verify that the fuses and that the SecureROM has been disabled */
	if (!chipid_get_fuse_lock() || (((*(volatile u_int32_t *)SECURITY_REG) & ROM_READ_DISABLE) == 0)) {
		panic("Fuses are unlocked or SecureROM is enabled");
	}
#endif

#if WITH_HW_PLATFORM_POWER
	/* initialize the s5l8945x pmgr driver */
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
#if !RELEASE_BUILD && (PRODUCT_IBSS || PRODUCT_IBEC)
	debug_enable_uarts(3);
#endif
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

#if WITH_NAND
	flash_nand_id();
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

	/* always remap the memory region at this point, regardless of resume,
	 * because the early MMU init always runs and we have to fix up. */
	platform_init_mainmem_map();
#endif

	return 0;
}

void platform_init_mainmem_map(void)
{
#if APPLICATION_IBOOT
	arm_mmu_map_section_range(SDRAM_BASE, SDRAM_BASE, ROUNDUP(SDRAM_BANK_LEN * SDRAM_BANK_COUNT, MB)/MB, kARMMMUNormal, true, true);
#endif
}

int platform_init_power(void)
{
#if WITH_HW_POWER
	power_init();
#endif

	return 0;
}

static bool gDisplayEnabled;

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
			clock_gate(CLK_HPERFRT, true);
			clock_gate(CLK_DISPOUT, true);
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
	u_int32_t base_rounded = *base;
	u_int32_t end_rounded = *base + *size;
	u_int32_t size_rounded = *size;

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
#if WITH_NAND_FILESYSTEM
	flash_nand_init();
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

	clock_gate(CLK_HPERFRT, true);
	clock_gate(CLK_DISPOUT, true);

	/* quiesce the panel */
	if (display_quiesce(true) == ENXIO) {
		clock_gate(CLK_HPERFRT, false);
		clock_gate(CLK_DISPOUT, false);
	}
#endif
	gDisplayEnabled = false;

	return 0;
}

int platform_bootprep(enum boot_target target)
{
	u_int32_t gids = ~0, uids = ~0;	/* leave crypto keys alone by default */

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
		case BOOT_DARWIN:
			if (boot_debug & kPowerNVRAMiBootDebugWDTWake)
			    wdt_enable();
			/* even when trusted, Darwin only gets the UID / GID1 */
			uids = 1;
			gids = 2;
#if WITH_HW_MIU
			miu_select_remap(REMAP_SDRAM);
#endif
			break;

		case BOOT_DIAGS:
			platform_quiesce_display();
#if WITH_BOOT_STAGE
			boot_clear_error_count();
#endif
			break;

		case BOOT_IBOOT:
#endif

		case BOOT_SECUREROM:
			platform_quiesce_display();
#if WITH_HW_MIU
			miu_select_remap(REMAP_SDRAM);
#endif
			break;
			
		case BOOT_UNKNOWN:
			platform_quiesce_display();
			break;

		default:
			; // do nothing
	}

	/* if we left the fuse unlocked for boot image processing,
	 * lock it now, after optionally switching from production to
	 * development fusing. */
	if (security_get_production_override())
		chipid_clear_production_mode();
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

	/* mark usable ram as cached/buffered */
	arm_mmu_map_section_range(SRAM_BASE, SRAM_BASE, ROUNDUP(SRAM_BANK_LEN, MB)/MB, kARMMMUNormal, false, false);
#if APPLICATION_SECUREROM
	arm_mmu_map_section_range(VROM_BASE, VROM_BASE, ROUNDUP(VROM_BANK_LEN, MB)/MB, kARMMMUNormal, false, false);
#endif

	/* Remap text base is to zero (for exception vectors) */
	arm_mmu_map_section(0, TEXT_BASE, kARMMMUNormalRX, false);

#ifdef MMU_NONCACHE0_SIZE
	/* create the noncached0 mapping */
	arm_mmu_map_section_range(MMU_NONCACHE0_VBASE, MMU_NONCACHE0_PBASE, ROUNDUP(MMU_NONCACHE0_SIZE, MB)/MB, kARMMMUStronglyOrdered, false, false);
#endif

#ifdef MMU_NONCACHE1_SIZE
	/* create the noncached1 mapping */
	arm_mmu_map_section_range(MMU_NONCACHE1_VBASE, MMU_NONCACHE1_PBASE, ROUNDUP(MMU_NONCACHE1_SIZE, MB)/MB, kARMMMUStronglyOrdered, false, false);
#endif

	/* in cases where we are running from DRAM, remap it now */
	if ((TEXT_BASE >= SDRAM_BASE) && (TEXT_BASE < SDRAM_END)) {
		platform_init_mainmem_map();
	}
}

int platform_init(void)
{
#if WITH_HW_SPI
	spi_init();
#endif

#if WITH_NAND_BOOT
	nand_boot_init();
#endif

#if WITH_TARGET_CONFIG
	target_init();
#endif

#if WITH_EFFACEABLE_NOR
    effaceable_nor_init();
#endif

	return 0;
}

int platform_debug_init(void)
{
#if WITH_HW_USB
	u_int32_t usb_enabled = 1;
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
	boot_set_stage(kPowerNVRAMiBootStageOff);

	power_shutdown();
#endif
	for(;;);
}

u_int32_t platform_set_performance(u_int32_t performance_level)
{
	u_int32_t old_performance_level = kPerformanceHigh;

#if WITH_HW_CLOCKS
	old_performance_level = clocks_set_performance(performance_level);
#endif

	return old_performance_level;
}

#if WITH_DEVICETREE

int platform_update_device_tree(void)
{
	DTNode *node;
	u_int32_t propSize;
	char *propName;
	void *propData;
	
	// Find the cpu0 node.
	if (FindNode(0, "cpus/cpu0", &node)) {
		
		// Fill in the cpu frequency
		propName = "clock-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			 *(uint32_t *)propData = clock_get_frequency(CLK_CPU);
		}
		
		// Fill in the memory frequency
		propName = "memory-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*(uint32_t *)propData = clock_get_frequency(CLK_MEM);
		}
		
		// Fill in the bus frequency
		propName = "bus-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*(uint32_t *)propData = clock_get_frequency(CLK_BUS);
		}
		
		// Fill in the peripheral frequency
		propName = "peripheral-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*(uint32_t *)propData = clock_get_frequency(CLK_PERIPH);
		}
		
		// Fill in the fixed frequency
		propName = "fixed-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*(uint32_t *)propData = clock_get_frequency(CLK_FIXED);
		}
		
		// Fill in the time base frequency
		propName = "timebase-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			*(uint32_t *)propData = clock_get_frequency(CLK_TIMEBASE);
		}
	}

	// Find the arm-io node
	if (FindNode(0, "arm-io", &node)) {
		// Fill in the clock-frequencies table
		propName = "clock-frequencies";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			clock_get_frequencies(propData, propSize / sizeof(u_int32_t));
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

	// Find the audio-complex node
	if (FindNode(0, "arm-io/audio-complex", &node)) {
		// Fill in the ncoref-frequency frequency
		propName = "ncoref-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			 *(uint32_t *)propData = clock_get_frequency(CLK_NCOREF);
		}
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

#if WITH_NAND_FILESYSTEM
	if (FindNode(0, "arm-io/flash-controller0/disk", &node)) {
		fillNandConfigProperties(node);
	}
#endif

#if WITH_HW_USBPHY
	// Find the otgphyctrl node
	if (FindNode(0, "arm-io/otgphyctrl0", &node)) {
	        usbphy_update_device_tree(node);
	}
#endif

	return target_update_device_tree();
}

#endif

static void platform_init_boot_strap(void)
{
	u_int32_t boot_strap, chip_board_id, gpio_board_id, boot_config;

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

u_int32_t platform_get_board_id(void)
{
	u_int32_t board_id;

	ASSERT((rPMGR_SCRATCH0 & kPlatformScratchFlagBootStrap) != 0);

	board_id = (rPMGR_SCRATCH0 >> 16) & 0xFF;

	return board_id;
}

u_int32_t platform_get_boot_config(void)
{
	u_int32_t boot_config;

	boot_config = (rPMGR_SCRATCH0 >> 8) & 0xFF;

	return boot_config;
}

bool platform_get_boot_device(int32_t index, enum boot_device *boot_device, u_int32_t *boot_flag, u_int32_t *boot_arg)
{
	u_int32_t boot_config = platform_get_boot_config();

	/* S5L8945X supports one boot device then USB-DFU per boot config */

	/* If the index is not zero force DFU mode */
	if (index != 0) index = 1;

	switch (boot_config) {
		case  0: /* SPI 0 */
			*boot_device = BOOT_DEVICE_SPI;
			*boot_flag = 0;
			*boot_arg = 0;
			break;

		case  1: /* SPI 3 */
			*boot_device = BOOT_DEVICE_SPI;
			*boot_flag = 0;
			*boot_arg = 3;
			break;

		case  2: /* SPI 0 Test Mode */
			*boot_device = BOOT_DEVICE_SPI;
			*boot_flag = BOOT_FLAG_TEST_MODE;
			*boot_arg = 0;
			break;

		case  3: /* SPI 3 Test Mode */
			*boot_device = BOOT_DEVICE_SPI;
			*boot_flag = BOOT_FLAG_TEST_MODE;
			*boot_arg = 3;
			break;

		case  4: /* FMI0 2 CS */
			*boot_device = BOOT_DEVICE_NAND;
			*boot_flag = 0;
			*boot_arg = 2 << 0;
			break;

		case  5: /* FMI0 4 CS */
			*boot_device = BOOT_DEVICE_NAND;
			*boot_flag = 0;
			*boot_arg = 4 << 0;
			break;

		case  6: /* FMI0 4 CS Test Mode */
			*boot_device = BOOT_DEVICE_NAND;
			*boot_flag = BOOT_FLAG_TEST_MODE;
			*boot_arg = 4 << 0;
			break;

/*		case  7: Unused			*/

		case  8: /* FMI1 2 CS */
			*boot_device = BOOT_DEVICE_NAND;
			*boot_flag = 0;
			*boot_arg = 2 << 8;
			break;

		case  9: /* FMI1 4 CS */
			*boot_device = BOOT_DEVICE_NAND;
			*boot_flag = 0;
			*boot_arg = 4 << 8;
			break;

		case 10: /* FMI1 4 CS Test Mode */
			*boot_device = BOOT_DEVICE_NAND;
			*boot_flag = BOOT_FLAG_TEST_MODE;
			*boot_arg = 4 << 8;
			break;

/*		case 11: Unused			*/

		case 12: /* FMI0/1 2/2 CS */
			*boot_device = BOOT_DEVICE_NAND;
			*boot_flag = 0;
			*boot_arg = (2 << 8) | (2 << 0);
			break;

		case 13: /* FMI0/1 4/4 CS */
			*boot_device = BOOT_DEVICE_NAND;
			*boot_flag = 0;
			*boot_arg = (4 << 8) | (4 << 0);
			break;

		case 14: /* FMI0/1 4/4 CS Test Mode */
			*boot_device = BOOT_DEVICE_NAND;
			*boot_flag = BOOT_FLAG_TEST_MODE;
			*boot_arg = (4 << 8) | (4 << 0);
			break;

/*		case 15: Unused			*/

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
	u_int32_t enable;
	u_int32_t disable;
};

#if WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI
static const struct boot_interface_pin spi0_boot_interface_pins[] =
{
#if SUPPORT_FPGA
	{ GPIO( 1, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_SSIN
#else
	{ GPIO( 1, 3), GPIO_CFG_OUT_1, GPIO_CFG_DFLT },	// SPI0_SSIN
#endif
	{ GPIO( 1, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_SCLK
	{ GPIO( 1, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_MOSI
	{ GPIO( 1, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// SPI0_MISO
};

static const struct boot_interface_pin spi3_boot_interface_pins[] =
{
#if SUPPORT_FPGA
	{ GPIO(19, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI3_SSIN
#else
	{ GPIO(19, 5), GPIO_CFG_OUT_1, GPIO_CFG_DFLT },	// SPI3_SSIN
#endif
	{ GPIO(19, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI3_SCLK
	{ GPIO(19, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI3_MOSI
	{ GPIO(19, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// SPI3_MISO
};
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */

#if WITH_HW_BOOTROM_NAND
/* Note: The code below expects this table to begin with CS3 through CS0 */
static const struct boot_interface_pin fmi0_boot_interface_pins[] =
{
	{ GPIO(23, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_CEN3
	{ GPIO(23, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_CEN2
	{ GPIO(23, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_CEN1
	{ GPIO(23, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_CEN0
	{ GPIO(23, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_CLE
	{ GPIO(23, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_ALE
	{ GPIO(22, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_REN
	{ GPIO(23, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_WEN
	{ GPIO(22, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO0
	{ GPIO(22, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO1
	{ GPIO(22, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO2
	{ GPIO(22, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO3
	{ GPIO(22, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO4
	{ GPIO(22, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO5
	{ GPIO(21, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI0_IO6
	{ GPIO(21, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// FMI0_IO7
};

/* Note: The code below expects this table to begin with CS3 through CS0 */
static const struct boot_interface_pin fmi1_boot_interface_pins[] =
{
	{ GPIO(25, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_CEN3
	{ GPIO(25, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_CEN2
	{ GPIO(25, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_CEN1
	{ GPIO(25, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_CEN0
	{ GPIO(25, 3), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_CLE
	{ GPIO(25, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_ALE
	{ GPIO(25, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_REN
	{ GPIO(25, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_WEN
	{ GPIO(24, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO0
	{ GPIO(24, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO1
	{ GPIO(24, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO2
	{ GPIO(24, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO3
	{ GPIO(24, 2), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO4
	{ GPIO(24, 1), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO5
	{ GPIO(24, 0), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// FMI1_IO6
	{ GPIO(23, 7), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// FMI1_IO7
};
#endif /* WITH_HW_BOOTROM_NAND */

void platform_enable_boot_interface(bool enable, enum boot_device boot_device, u_int32_t boot_arg)
{
	const struct boot_interface_pin *pins0 = 0;
	const struct boot_interface_pin *pins1 = 0;
	u_int32_t cnt, func, pin_count0 = 0, pin_count1 = 0;
	gpio_t pin;

	switch (boot_device) {
#if WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI
		case BOOT_DEVICE_SPI :
			if (boot_arg == 0) {
				pins0 = spi0_boot_interface_pins;
				pin_count0 = (sizeof(spi0_boot_interface_pins) / sizeof(spi0_boot_interface_pins[0]));
			} else if (boot_arg == 3) {
				pins0 = spi3_boot_interface_pins;
				pin_count0 = (sizeof(spi3_boot_interface_pins) / sizeof(spi3_boot_interface_pins[0]));
			}
			break;
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */

#if WITH_HW_BOOTROM_NAND
		case BOOT_DEVICE_NAND :
			pins0 = fmi0_boot_interface_pins;
			pin_count0 = (sizeof(fmi0_boot_interface_pins) / sizeof(fmi0_boot_interface_pins[0]));
			switch ((boot_arg >> 0) & 0xFF) {
				case 4: /* CS x4 - skip no pin */
					break;

				case 2: /* CS x2 - skip 2 pins */
					pins0 += 2;
					pin_count0 -= 2;
					break;

				default:
					pins0 = 0;
					pin_count0 = 0;
					break;
			}

			pins1 = fmi1_boot_interface_pins;
			pin_count1 = (sizeof(fmi1_boot_interface_pins) / sizeof(fmi1_boot_interface_pins[0]));
			switch ((boot_arg >> 8) & 0xFF) {
				case 4: /* CS x4 - skip no pin */
					break;

				case 2: /* CS x2 - skip 2 pins */
					pins1 += 2;
					pin_count1 -= 2;
					break;

				default:
					pins1 = 0;
					pin_count1 = 0;
					break;
			}
			break;
#endif /* WITH_HW_BOOTROM_NAND */

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

u_int64_t platform_get_nonce(void)
{
	u_int64_t	nonce;
	u_int32_t	*nonce_words = (u_int32_t *)&nonce;

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
	return chipid_get_ecid_image_personalization_required();
}

u_int32_t platform_get_osc_frequency(void)
{
	return chipid_get_osc_frequency();
}

// Let the target's rules.mk override the high core voltage point
#ifndef TARGET_HIGH_SOC_VOLTAGE
#define TARGET_HIGH_SOC_VOLTAGE CHIPID_SOC_VOLTAGE_HIGH
#endif

u_int32_t platform_get_base_soc_voltage(void)
{
	return chipid_get_soc_voltage(TARGET_HIGH_SOC_VOLTAGE);
}

// Let the target's rules.mk override the high cpu voltage point
#ifndef TARGET_HIGH_CPU_VOLTAGE
#define TARGET_HIGH_CPU_VOLTAGE CHIPID_CPU_VOLTAGE_HIGH
#endif

u_int32_t platform_get_base_cpu_voltage(void)
{
	return chipid_get_cpu_voltage(TARGET_HIGH_CPU_VOLTAGE);
}

static const u_int32_t soc_voltage_points[] = {
	CHIPID_SOC_VOLTAGE_MED,
	TARGET_HIGH_SOC_VOLTAGE
};

int platform_get_soc_voltages(u_int32_t count, u_int32_t *voltages)
{
	u_int32_t cnt, points = sizeof(soc_voltage_points) / sizeof(soc_voltage_points[0]);

	if (voltages == 0) return -1;

	if (count > points) count = points;
	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = chipid_get_soc_voltage(soc_voltage_points[cnt]);
	}

	return 0;
}

static const u_int32_t cpu_voltage_points[] = {
	CHIPID_CPU_VOLTAGE_MED,
	TARGET_HIGH_CPU_VOLTAGE
};

int platform_get_cpu_voltages(u_int32_t count, u_int32_t *voltages)
{
	u_int32_t cnt, points = sizeof(cpu_voltage_points) / sizeof(cpu_voltage_points[0]);

	if (voltages == 0) return -1;

	if (count > points) count = points;
	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = chipid_get_cpu_voltage(cpu_voltage_points[cnt]);
	}

	return 0;
}

int platform_convert_voltages(int buck, u_int32_t count, u_int32_t *voltages)
{
#if WITH_HW_POWER
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
#if SUPPORT_FPGA
	return !gpio_read(GPIO_FORCE_DFU);
#else
	return gpio_read(GPIO_FORCE_DFU);
#endif
}

bool platform_get_request_dfu1(void)	// Formerly platform_get_hold_key()
{
	return !gpio_read(GPIO_REQUEST_DFU1);
}

bool platform_get_request_dfu2(void)    // Formerly platform_get_menu_key()
{
	return !gpio_read(GPIO_REQUEST_DFU2);
}

int platform_translate_key_selector(u_int32_t key_selector, u_int32_t *key_opts)
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

#if APPLICATION_IBOOT
u_int64_t platform_get_memory_size(void)
{
	u_int64_t	memory_size;

	// If rPMGR_SCRATCH0[2] set then the memory was inited, we have memory size info
	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagMemoryInfo) != 0) {
		memory_size = (rPMGR_SCRATCH13 & 0xffff) * 1024 * 1024;
	}
	else {
		panic("memory not yet inited\n");
	}

	return memory_size;
}

u_int8_t platform_get_memory_manufacturer_id(void)
{
	// If rPMGR_SCRATCH0[2] set then the memory was inited, we have memory vendor-id info
	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagMemoryInfo) != 0) {
		return ((rPMGR_SCRATCH13 >> 28) & 0xf);
	}
	else {
		panic("memory not yet inited\n");
	}
}

void platform_set_memory_info(u_int8_t manuf_id, u_int64_t memory_size)
{
	if ((rPMGR_SCRATCH0 & kPlatformScratchFlagMemoryInfo) == 0) {
		rPMGR_SCRATCH13 = 0;
	}
	
	rPMGR_SCRATCH13 = (manuf_id << 28) | (memory_size  & 0xffff);
	rPMGR_SCRATCH0 |= kPlatformScratchFlagMemoryInfo;
}

#endif
