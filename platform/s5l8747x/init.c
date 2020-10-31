/*
 * Copyright (C) 2007-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <arch.h>
#include <arch/arm/arm.h>
#include <drivers/aes.h>
#include <drivers/flash_nor.h>
#include <drivers/iic.h>
#include <drivers/power.h>
#include <drivers/spi.h>
#include <drivers/uart.h>
#include <drivers/usb/usb_public.h>
#include <drivers/usbphy.h>
#include <lib/devicetree.h>
#include <lib/env.h>
#include <lib/random.h>
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
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/miu.h>
#include <platform/soc/power.h>
#include <sys.h>
#include <sys/boot.h>
#include <target.h>

static void platform_init_boot_strap(void);

static uint8_t boot_debug;


int platform_early_init(void)
{
#if PRODUCT_LLB || PRODUCT_IBSS
	/* Verify that SecureROM has been disabled */
	if (((*(volatile u_int32_t *)SECURITY_REG) & ROM_READ_DISABLE) == 0) {
		panic("SecureROM is enabled");
	}
#endif

#if WITH_HW_PLATFORM_POWER
	/* initialize the s5l8747x power control driver */
	platform_power_init();
#endif

#if WITH_HW_MIU
	miu_init();
#endif
#if WITH_HW_CLOCKS
	/* initialize the clock driver */
	clocks_init();
#endif
#if WITH_HW_VIC
	/* initialize the vic, mask all interrupts */
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
	// Chips older then B0 are not supported
	if (platform_get_chip_revision() < 0x10) platform_not_supported();

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
	// need board id to select the right pinconfig
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

int platform_init_power(void)
{
#if WITH_HW_POWER
	power_init();
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

#if WITH_HW_DMA
#error "Need to quiesce DMA here if a DMA API is ever added"
#endif
#if WITH_HW_TIMER
	timer_stop_all();
#endif
#if WITH_HW_VIC
	interrupt_mask_all();
#endif

	if (quiesce_clocks) {
#if WITH_HW_CLOCKS
		clocks_quiesce();
#endif
	}

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
#if APPLICATION_IBOOT
	arm_mmu_map_section_range(SDRAM_BASE, SDRAM_BASE, ROUNDUP(SDRAM_BANK_LEN * SDRAM_BANK_COUNT, MB)/MB, kARMMMUNormal, false, false);
#endif
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
}

int platform_init(void)
{
#if WITH_HW_SPI
	spi_init();
#endif

#if APPLICATION_IBOOT
	target_init();
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

		// Fill in the usb-phy frequency
		propName = "usbphy-frequency";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			 *(uint32_t *)propData = clock_get_frequency(CLK_USBPHYCLK);
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

#if WITH_HW_USBPHY
	// Fill in the otgphyctrl0 node
	if (FindNode(0, "arm-io/otgphyctrl0", &node)) {
		 usbphy_update_device_tree(node);
	}
	// Fill in the otgphyctrl1 node
	if (FindNode(0, "arm-io/otgphyctrl1", &node)) {
		 usbphy_update_device_tree(node);
	}
#endif

	// Carve mfcram out of base of vram. mfcram node should already have size populated
	if (FindNode(0, "mfcram", &node)) {
		uint32_t *mfcramPropData;
		propName = "reg";
		if (FindProperty(node, &propName, &propData, &propSize)) {
			mfcramPropData = propData;
			if (FindNode(0, "vram", &node)) {
				propName = "reg";
				if (FindProperty(node, &propName, &propData, &propSize)) {
					uint32_t *vramPropData = propData;
					mfcramPropData[0] = vramPropData[0];
					vramPropData[0] += mfcramPropData[1];
					vramPropData[1] -= mfcramPropData[1];
				}
			}
		}
	}

	return target_update_device_tree();
}

#endif

static void platform_init_boot_strap(void)
{
	u_int32_t boot_strap, chip_board_id, gpio_board_id, boot_config;

	// If rSTATESAVE0[0] set then boot strap already valid
	if ((rSTATESAVE0 & 1) != 0) return;

	gpio_configure_pupdn(GPIO_BOARD_ID0, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOARD_ID1, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOARD_ID2, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOARD_ID3, GPIO_PDN);

	gpio_configure_pupdn(GPIO_BOOT_CONFIG0, GPIO_PDN);
	gpio_configure_pupdn(GPIO_BOOT_CONFIG1, GPIO_PDN);

	gpio_configure(GPIO_BOARD_ID0, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_ID1, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_ID2, GPIO_CFG_IN);
	gpio_configure(GPIO_BOARD_ID3, GPIO_CFG_IN);

	gpio_configure(GPIO_BOOT_CONFIG0, GPIO_CFG_IN);
	gpio_configure(GPIO_BOOT_CONFIG1, GPIO_CFG_IN);

#if !SUPPORT_FPGA
	platform_power_spin(100); // Wait 100us
#endif

	chip_board_id = chipid_get_board_id();

	gpio_board_id =
		(gpio_read(GPIO_BOARD_ID3) << 3) |
		(gpio_read(GPIO_BOARD_ID2) << 2) |
		(gpio_read(GPIO_BOARD_ID1) << 1) |
		(gpio_read(GPIO_BOARD_ID0) << 0);

	boot_config =
		(gpio_read(GPIO_BOOT_CONFIG1) << 1) |
		(gpio_read(GPIO_BOOT_CONFIG0) << 0);

	gpio_configure(GPIO_BOARD_ID0, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_ID1, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_ID2, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOARD_ID3, GPIO_CFG_DFLT);

	gpio_configure(GPIO_BOOT_CONFIG0, GPIO_CFG_DFLT);
	gpio_configure(GPIO_BOOT_CONFIG1, GPIO_CFG_DFLT);

	boot_strap = (((chip_board_id << 4) | gpio_board_id) << 16) |
		     (boot_config << 8) |
		     (0x01 << 0);

	rSTATESAVE0 = (rSTATESAVE0 & 0xFF000000) | (boot_strap & 0x00FFFFFF);
}

u_int32_t platform_get_board_id(void)
{
	u_int32_t board_id;

	ASSERT((rSTATESAVE0 & kPlatformScratchFlagBootStrap) != 0);

	board_id = (rSTATESAVE0 >> 16) & 0xFF;

#if SUPPORT_FPGA
	board_id = 0x3F;
#endif

	return board_id;
}

u_int32_t platform_get_boot_config(void)
{
	u_int32_t boot_config;

	boot_config = (rSTATESAVE0 >> 8) & 0xFF;

	return boot_config;
}

bool platform_get_boot_device(int32_t index, enum boot_device *boot_device, u_int32_t *boot_flag, u_int32_t *boot_arg)
{
	u_int32_t boot_config = platform_get_boot_config();

	/* S5L8747X supports one boot device then USB-DFU per boot config.
	   Unlike other platforms, the USB-DFU device is selectable. */

#ifdef HACK_BOOT_CONFIG_OVERRIDE
	boot_config = HACK_BOOT_CONFIG_OVERRIDE;
#endif
	
	/* If the index is not zero force DFU mode */
	if (index != 0) index = 1;

	switch (boot_config) {
		case 0: /* SPI 0, then USB0 FDU */
		case 1: /* SPI 0, then USB1 DFU */
			if (index == 0) {
				*boot_device = BOOT_DEVICE_SPI;
				*boot_flag = 0;
				*boot_arg = 0;
			} else {
				*boot_device = BOOT_DEVICE_USBDFU;
				*boot_flag = 0;
				*boot_arg = 0;
			}
			return true;

		case 2: /* SPI 0, then USB0 FDU with TEST_MODE */
		case 3: /* SPI 0, then USB1 DFU with TEST_MODE */
			if (index == 0) {
				*boot_device = BOOT_DEVICE_SPI;
				*boot_flag = BOOT_FLAG_TEST_MODE;
				*boot_arg = 0;
			} else {
				*boot_device = BOOT_DEVICE_USBDFU;
				*boot_flag = BOOT_FLAG_TEST_MODE;
				*boot_arg = 0;
			}
			return true;

		default:
			return false;
	}
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
#ifdef SPI_NOR0
static const struct boot_interface_pin spi0_boot_interface_pins[] =
{
#if SUPPORT_FPGA
	{ GPIO( 1, 3), GPIO_CFG_FUNC1, GPIO_CFG_DFLT },	// SPI0_CEN
#else
	{ GPIO( 1, 3), GPIO_CFG_OUT_1, GPIO_CFG_DFLT },	// SPI0_CEN
#endif
	{ GPIO( 1, 4), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_CLK
	{ GPIO( 1, 5), GPIO_CFG_FUNC0, GPIO_CFG_DFLT },	// SPI0_MOSI
	{ GPIO( 1, 6), GPIO_CFG_FUNC0, GPIO_CFG_DFLT }	// SPI0_MISO
};
#endif
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */

void platform_enable_boot_interface(bool enable, enum boot_device boot_device, u_int32_t boot_arg)
{
	const struct boot_interface_pin *pins = 0;
	u_int32_t cnt, func, pin_count = 0;
	gpio_t pin;

	switch (boot_device) {
#if WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI
		case BOOT_DEVICE_SPI :
#ifdef SPI_NOR0
			if (boot_arg == 0) {
				pins = spi0_boot_interface_pins;
				pin_count = (sizeof(spi0_boot_interface_pins) / sizeof(spi0_boot_interface_pins[0]));
			}
#endif
			break;
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */

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

		dprintf(DEBUG_INFO, "platform_enable_boot_interface: %x, %x\n", pin, func);
		gpio_configure(pin, func);
	}
}

u_int64_t platform_get_nonce(void)
{
	u_int64_t	nonce;
	u_int32_t	*nonce_words = (u_int32_t *)&nonce;
	u_int8_t	*nonce_bytes = (u_int8_t *)&nonce;

	// If rSTATESAVE0[1] set then the nonce has already been generated
	if ((rSTATESAVE0 & kPlatformScratchFlagNonce) == 0) {
#if WITH_RANDOM
		if (random_get_bytes(nonce_bytes, sizeof(nonce)) != 0)
#endif
		{
			memset(nonce_bytes, 0, sizeof(nonce));
		}

		rSTATESAVE2 = nonce_words[0];
		rSTATESAVE3 = nonce_words[1];

		rSTATESAVE0 |= kPlatformScratchFlagNonce;
	} else {
		nonce_words[0] = rSTATESAVE2;
		nonce_words[1] = rSTATESAVE3;
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

u_int32_t platform_get_base_soc_voltage(void)
{
	return 1200;
}

int platform_get_soc_voltages(u_int32_t count, u_int32_t *voltages)
{
	u_int32_t cnt;

	if (voltages == 0) return -1;

	for (cnt = 0; cnt < count; cnt++) {
		voltages[cnt] = platform_get_base_soc_voltage();
	}

	return 0;
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

int platform_translate_key_selector(u_int32_t key_selector, u_int32_t *key_opts)
{
	switch (key_selector) {
		case IMAGE_KEYBAG_SELECTOR_PROD :
			break;

		default :
			return -1;
	}

	*key_opts = AES_KEY_TYPE_GID0 | AES_KEY_SIZE_128;

	return 0;
}

#if APPLICATION_IBOOT
u_int64_t platform_get_memory_size(void)
{
        return (2 * 128 * 1024 * 1024);
}

uintptr_t platform_get_memory_region_base_optional(memory_region_type_t region)
{
	uintptr_t base;

	switch (region) {
		case kMemoryRegion_Panic:
			base = SDRAM_BASE + platform_get_memory_size() - PANIC_SIZE;
			break;

		case kMemoryRegion_Display:
			base = SDRAM_BASE + platform_get_memory_size() - PURPLE_GFX_MEMORY_LEN - PANIC_SIZE;
			break;

		case kMemoryRegion_Kernel:
			base = SDRAM_BASE;
			break;

		default:
			base = (uintptr_t)-1;
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

		case kMemoryRegion_Display:
			size = PURPLE_GFX_MEMORY_LEN;
			break;

#if defined(SLEEP_TOKEN_BUFFER_SIZE)
		case kMemoryRegion_SleepToken:
			size = SLEEP_TOKEN_BUFFER_SIZE;
			break;

#endif

		case kMemoryRegion_Kernel:
			size = platform_get_memory_size() - PURPLE_GFX_MEMORY_LEN - PANIC_SIZE;
			break;

		default:
			size = (size_t)-1;
	}

	return size;
}

#endif

