 /*
 * Copyright (C) 2007-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch.h>
#include <debug.h>
#include <platform.h>
#include <platform/memmap.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/task.h>
#include <lib/blockdev.h>
#include <lib/image.h>
#include <lib/mib_def.h>
#include <drivers/flash_nor.h>
#include <drivers/nvme.h>
#if WITH_USB_DFU
#include <drivers/usb/usb_public.h>
#endif
#if WITH_TBT_DFU
#include <drivers/thunderbolt/thunderboot.h>
#endif
#if WITH_ANC_BOOT
# include <drivers/anc_boot.h>
#endif

#define USER_BUTTON_FORCEDFU_TIMEOUT    (6 * 1000 * 1000)

static void	boot_selected(enum boot_device boot_device, u_int32_t boot_flag, u_int32_t boot_arg);


static struct image_info *lookup_image_in_bdev(const char *name, uint32_t type);
static bool load_selected_image(struct image_info *loaded_image, u_int32_t type, void *load_address, size_t load_length, u_int32_t boot_flag);
static void boot_loaded_image(void *load_address);

int _main(void)
{
	bool	force_dfu, force_reset;
	utime_t	dfu_time;
	enum boot_device boot_device;
	u_int32_t boot_flag, boot_arg;
	int32_t index;

	/* initialize the cpu */
	arch_cpu_init(false);

	dprintf(DEBUG_CRITICAL, "\n" CONFIG_PROGNAME_STRING " start\n");

	/* setup the default clock configuration */
	dprintf(DEBUG_INFO, "setting up initial clock configuration\n");
	platform_init_setup_clocks();

	/* set up any internal memory (internal sram) */
	dprintf(DEBUG_INFO, "setting up internal memory\n");
	platform_init_internal_mem();

	/* set up the default pin configuration */
	dprintf(DEBUG_INFO, "setting up default pin configuration\n");
	platform_init_hwpins();

	/* Sample force_dfu and request_dfu pins. */
	force_dfu = platform_get_force_dfu();
	force_reset = platform_get_request_dfu1() && platform_get_request_dfu2();

	/* bring up system services (cpu, tasks, callout, heap) */
	sys_init();

	/* generate random stack cookie on platforms without a
	   requirement for fast resume from suspend-to-RAM via SecureROM */
#if !WITH_NO_RANDOM_STACK_COOKIE
	sys_init_stack_cookie();
#endif

	/* do some early initialization of hardware */
	/* timers will start firing at this point */
	dprintf(DEBUG_INFO, "doing early platform hardware init\n");
	platform_early_init();

	/* print version info now that UART is enabled on DEBUG builds */
	dprintf(DEBUG_INFO, "\n\n%s for %s\n", CONFIG_PROGNAME_STRING, CONFIG_BOARD_STRING);
	dprintf(DEBUG_INFO, "%s\n", build_tag_string);
	dprintf(DEBUG_INFO, "%s\n", build_style_string);

	/* initialize the rest of hardware */
	dprintf(DEBUG_INFO, "doing platform hardware init\n");
	platform_init();

	/* Check for Force DFU Mode Pin */
	dprintf(DEBUG_INFO, "Checking for Force DFU Mode Pin: %x / %x\n", force_dfu, force_reset);
	if (force_dfu && !platform_get_usb_cable_connected())
		force_dfu = false;

	/* Check for Force DFU Mode request pins. Requesting DFU mode is done
	   by asserting both REQUEST_DFU1 and REQUEST_DFU2, holding them for
	   the required time, then deasserting REQUEST_DFU1, followed by
	   deasserting REQUEST_DFU2 after another required hold time.
	*/
	dprintf(DEBUG_INFO, "Checking for Force DFU Mode request pins: %x / %x\n", force_dfu, force_reset);
	if (!force_dfu && force_reset && platform_get_usb_cable_connected()) {
		dfu_time = system_time();
		while (platform_get_request_dfu1() && platform_get_request_dfu2() && platform_get_usb_cable_connected()) {
			if (time_has_elapsed(dfu_time, USER_BUTTON_FORCEDFU_TIMEOUT))
				break;
			task_sleep(100 * 1000);
		}

		dfu_time = system_time();
		while (!platform_get_request_dfu1() && platform_get_request_dfu2() && platform_get_usb_cable_connected()) {
			if (time_has_elapsed(dfu_time, USER_BUTTON_FORCEDFU_TIMEOUT)) {
				force_dfu = true;
				break;
			}
			task_sleep(100 * 1000);
		}
	}

	/* Try the boot devices in order */
	index = 0;

	/* if DFU is being forced, set index to -1 for force dfu */
	dprintf(DEBUG_INFO, "Force DFU: %x\n", force_dfu);
	if (force_dfu) index = -1;

	while (1) {
		/* ask the platform code for the selected boot source */
		if (!platform_get_boot_device(index, &boot_device, &boot_flag, &boot_arg))
			break;

		/* Attempt to boot the selected device */
		boot_selected(boot_device, boot_flag, boot_arg);

		if (index < 0)
			continue;

		index++;
	}

	/* we are hosed */
	dprintf(DEBUG_INFO, "No valid boot device, resetting.\n");
	platform_reset(false);
}

/*
 * Attempt to boot the seleced device.
 */
static void
boot_selected(enum boot_device boot_device, uint32_t boot_flag, uint32_t boot_arg)
{
	struct image_info	*loaded_image;
	size_t			loaded_length;
	void			*load_address;
	size_t			load_length;
	bool			loaded = false;
	uint32_t		type = IMAGE_TYPE_LLB;

	dprintf(DEBUG_INFO, "boot_selected: boot_device: %08x, boot_flag: %08x, boot_arg: %08x\n",
		boot_device, boot_flag, boot_arg);

	/* Enable the pins for the selected boot device */
	platform_enable_boot_interface(true, boot_device, boot_arg);

	/* reset security and clear the insecure memory area */
	security_init(true);

	loaded_image = NULL;
	loaded_length = 0;
	load_address = (void *)INSECURE_MEMORY_BASE;
	load_length = INSECURE_MEMORY_SIZE;
	
	/* ask the boot source to fetch the boot image */
	switch (boot_device) {
#if WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI
	case BOOT_DEVICE_SPI:
	{
		dprintf(DEBUG_INFO, "SPI NOR boot selected\n");

		/* initialise the NOR flash for the desired channel */
		flash_nor_init(boot_arg);

		loaded_image = lookup_image_in_bdev("nor0", type);
		if (loaded_image != NULL)
			loaded_length = loaded_image->imageLength;

		break;
	}
#endif /* WITH_HW_FLASH_NOR && WITH_HW_FLASH_NOR_SPI */
#if WITH_ANC_BOOT
	case BOOT_DEVICE_NAND:
	{
		if (!anc_reset(boot_arg)) {
			dprintf(DEBUG_CRITICAL, "Failed to init ASP");
			break;
		}

		loaded_length = anc_read_llb(load_address, load_length);
		if (loaded_length > load_length)
			panic("load overflow");

		if (loaded_length > 0) {
			if ((loaded_image = image_create_from_memory(
					load_address,
					loaded_length,
					IMAGE_OPTION_LOCAL_STORAGE)) == NULL) {
				dprintf(DEBUG_INFO, "image creating failed\n");
			} else {
				dprintf(DEBUG_INFO, "loaded image from ANC\n");
			}
		}

		break;
	}
#endif
#if WITH_NVME
	case BOOT_DEVICE_NVME:
	{
		loaded_image = lookup_image_in_bdev("nvme_firmware0", type);
		if (loaded_image != NULL)
			loaded_length = loaded_image->imageLength;

		break;
	}
#endif
#if WITH_USB_DFU
	case BOOT_DEVICE_USBDFU:
	{
		int result;

		/* Turn on the DFU STATUS pin */
		platform_set_dfu_status(true);

		/* Set type to iBSS */
		type = IMAGE_TYPE_IBSS;

		/* try to get an image via DFU */
		if ((result = getDFUImage(load_address, load_length)) < 0) {
			dprintf(DEBUG_INFO, "fatal DFU download error");
			break;
		}

		loaded_length = (size_t)result;
		if (loaded_length > load_length)
			panic("load overflow");

		/* wrap the image  */
		if ((loaded_image = image_create_from_memory(load_address, loaded_length, 0)) == NULL) {
			dprintf(DEBUG_INFO, "failed to create image from DFU download\n");
			break;
		}

		dprintf(DEBUG_INFO, "loaded image from USB-DFU\n");
		break;
	}
#endif /* WITH_USB_DFU */
#if WITH_TBT_DFU
	case BOOT_DEVICE_TBTDFU:
	{
		int result;

		/* Set type to iBSS */
		type = IMAGE_TYPE_IBSS;

		/* try to get an image via DFU */
		if ((result = thunderboot_get_dfu_image(load_address, load_length)) < 0) {
			dprintf(DEBUG_INFO, "fatal DFU download error");
			break;
		}

		loaded_length = (size_t)result;
		if (loaded_length > load_length)
			panic("load overflow");

		/* wrap the image  */
		if ((loaded_image = image_create_from_memory(load_address, loaded_length, 0)) == NULL) {
			dprintf(DEBUG_INFO, "failed to create image from DFU download\n");
			break;
		}

		dprintf(DEBUG_INFO, "loaded image from TBT-DFU\n");
		break;
	}
#endif /* WITH_USB_DFU */
/*	case BOOT_DEVICE_XMODEM: */
	default:
		break;
	}

	/* if we found an image, try to load it */
	if (loaded_image) {
		loaded = load_selected_image(loaded_image, type, load_address, loaded_length, boot_flag);
		image_free(loaded_image);
	} else {
		loaded = false;
	}

	/* Disable the pins for the selected boot device */
	platform_enable_boot_interface(false, boot_device, boot_arg);

	if (loaded)
		boot_loaded_image(load_address);

	/* load failed or boot source not supported */
	dprintf(DEBUG_INFO, "failed to load from selected boot device\n");
}

static struct image_info *
lookup_image_in_bdev(const char *name, uint32_t type)
{
	struct blockdev		*boot_bdev;
	struct image_info	*loaded_image;

	/* look it up */
	if ((boot_bdev = lookup_blockdev(name)) == NULL)
		return NULL;

	dprintf(DEBUG_INFO, "boot device %s initialised\n", name);

	/* look for the image directory in the expected location */
	if (image_search_bdev(boot_bdev, 0, IMAGE_OPTION_LOCAL_STORAGE) == 0)
		return NULL;
	dprintf(DEBUG_INFO, "found image directory on %s\n", name);

	/* look for the LLB */
	loaded_image = image_find(type);

	dprintf(DEBUG_INFO, "%s LLB image on %s\n", loaded_image != NULL ? "found" : "failed to find", name);

	return loaded_image;
}

static bool
load_selected_image(struct image_info *loaded_image, u_int32_t type, void *load_address, size_t load_length, u_int32_t boot_flag)
{
	/* image epoch must be greater than or equal to the SecureROM's epoch */
	loaded_image->imageOptions |= IMAGE_OPTION_GREATER_EPOCH;

	/* image decode library make use of this information to do various validation on image */
	loaded_image->imageOptions |= IMAGE_OPTION_NEW_TRUST_CHAIN;

	/* if test mode isn't set, we require the image be trusted, regardless of the security fuse */
	if (!(boot_flag & BOOT_FLAG_TEST_MODE))
		loaded_image->imageOptions |= IMAGE_OPTION_REQUIRE_TRUST;

	/* validate the boot image */
	if (image_load(loaded_image, &type, 1, NULL, &load_address, &load_length)) {
		dprintf(DEBUG_INFO, "image load failed\n");
		return false;
	}

	return true;
}

static void
boot_loaded_image(void *load_address)
{
	/* consolidate environment post image validation */
	security_consolidate_environment();

#if	WITH_SIDP
	/* Seal the ROM manifest for Silicon Data Protection */
	security_sidp_seal_rom_manifest();
#endif

	dprintf(DEBUG_CRITICAL, "executing image...\n");
	prepare_and_jump(BOOT_UNKNOWN, (void *)load_address, NULL);
}
