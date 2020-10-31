/*
 * Copyright (C) 2007-2014 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
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
#include <target.h>
#include <drivers/power.h>
#include <lib/profile.h>
#include <lib/env.h>
#include <lib/fs.h>
#include <lib/image.h>
#include <lib/macho.h>
#include <lib/mib.h>
#include <lib/paint.h>
#include <lib/paniclog.h>
#include <lib/ramdisk.h>
#include <lib/syscfg.h>
#include <lib/ticket.h>
#include <platform.h>
#include <platform/memmap.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>

#if WITH_EFFACEABLE
#include <lib/effaceable.h>
#endif

#if WITH_CONSISTENT_DBG
#include <drivers/consistent_debug.h>
#endif

struct image_device {
	const char *name;
	off_t image_table_offset; // negative offset means from top of device
};

#include <platform/image_devices.h>

extern void	boot_darwin(void *ptr, void *arg) __noreturn;

static int	boot_flash(u_int32_t type, enum boot_target target);
static int 	boot_diagnostics_fs(void);

void find_boot_images(void)
{
	PROFILE_ENTER('FBI');
#if WITH_BLOCKDEV
	struct blockdev *rom_bdev;
	uint32_t i;

	/* iterate over each of the rom devices, looking for images and syscfg data */
	for (i=0; i < sizeof(platform_image_devices)/sizeof(platform_image_devices[0]); i++) {
		rom_bdev = lookup_blockdev(platform_image_devices[i].name);
		if (!rom_bdev)
			continue;

		image_search_bdev(rom_bdev, platform_image_devices[i].image_table_offset, 0);
	}
	
#if WITH_SYSCFG
	/* hook up syscfg data */
	for (i=0; i < sizeof(platform_syscfg_devices)/sizeof(platform_syscfg_devices[0]); i++) {
		if (lookup_blockdev(platform_syscfg_devices[i].name)) {
			if (syscfgInitWithBdev(platform_syscfg_devices[i].name))
				break;
		}
	}
#endif /* WITH_SYSCFG */
#endif /* WITH_BLOCKDEV */
	
	image_dump_list(false);
	PROFILE_EXIT('FBI');
}

static int
boot_flash(u_int32_t type, enum boot_target target)
{
	addr_t		image_ptr;
	size_t		image_len;

	image_ptr = INSECURE_MEMORY_BASE;
	image_len = INSECURE_MEMORY_SIZE;

	PROFILE_ENTER('ILT');
	if (image_load_type(&image_ptr, &image_len, type, 0))
		return -1;
	PROFILE_EXIT('ILT');

	/* consolidate environment */
	security_consolidate_environment();

	dprintf(DEBUG_INFO, "boot: loaded image at %p, len %zd\n", (void *)image_ptr, image_len);

	/* boot it */
	prepare_and_jump(target, (void *)image_ptr, NULL);

	return 0;
}

int boot_check_stage(void)
{
#if WITH_HW_POWER
	int		result;
	uint8_t		data, boot_stage;
	uint32_t	boot_fail, panic_fail;
	
	if (power_is_suspended() || power_needs_precharge()) return 0;
	
	result = power_get_nvram(kPowerNVRAMiBootStageKey, &boot_stage);
	if (result != 0) return result;
	
	if (boot_stage == kPowerNVRAMiBootStageOff) return 0;
	
	power_set_nvram(kPowerNVRAMiBootErrorStageKey, boot_stage);
	
	result = power_get_nvram(kPowerNVRAMiBootErrorCountKey, &data);
	if (result != 0) return result;
	
	boot_fail = (data >> kPowerNVRAMiBootErrorBootShift) & kPowerNVRAMiBootErrorCountMask;
	panic_fail = (data >> kPowerNVRAMiBootErrorPanicShift) & kPowerNVRAMiBootErrorCountMask;
	
	if (boot_stage == kPowerNVRAMiBootStagePanicSave) {
		if (panic_fail < kPowerNVRAMiBootErrorCountMask) panic_fail++;
	} else {
		if (boot_fail < kPowerNVRAMiBootErrorCountMask) boot_fail++;
	}
	
	data = (boot_fail << kPowerNVRAMiBootErrorBootShift) | (panic_fail << kPowerNVRAMiBootErrorPanicShift);
	
	power_set_nvram(kPowerNVRAMiBootErrorCountKey, data);

	power_cancel_buttonwait();
#endif
	
	return 0;
}

int boot_set_stage(uint8_t boot_stage)
{
#if WITH_HW_POWER
	if (power_is_suspended() || power_needs_precharge()) return 0;
	
	power_set_nvram(kPowerNVRAMiBootStageKey, boot_stage);
#endif
	
	return 0;
}

int boot_clear_error_count(void)
{
#if WITH_HW_POWER
	uint8_t	data;
	
	data = (0 << kPowerNVRAMiBootErrorBootShift) | (0 << kPowerNVRAMiBootErrorPanicShift);
	
	power_set_nvram(kPowerNVRAMiBootErrorCountKey, data);
#endif
	
	return 0;
}

int boot_error_count(uint32_t *boot_fail_count, uint32_t *panic_fail_count)
{
	int		result = 0;
	uint32_t	boot_fail = 0, panic_fail = 0;
#if WITH_HW_POWER
	uint8_t data = 0;
	
	result = power_get_nvram(kPowerNVRAMiBootErrorCountKey, &data);
	if (result != 0) return result;
	
	boot_fail = (data >> kPowerNVRAMiBootErrorBootShift) & kPowerNVRAMiBootErrorCountMask;
	panic_fail = (data >> kPowerNVRAMiBootErrorPanicShift) & kPowerNVRAMiBootErrorCountMask;
#endif
	
	if (boot_fail_count != 0) *boot_fail_count = boot_fail;
	if (panic_fail_count != 0) *panic_fail_count = panic_fail;
	
	return result;
}

#if PRODUCT_IBOOT && WITH_HW_POWER && WITH_BOOT_STAGE
void boot_check_panic(void)
{
	int		result;
	uint8_t		error_stage;

	/* LLB saved the original stage in ErrorStageKey */
	result = power_get_nvram(kPowerNVRAMiBootErrorStageKey, &error_stage);
	if (result != 0) goto panic_clear;

	if (error_stage == kPowerNVRAMiBootStagePanicSave) {
		/* keep alive a little longer */
		platform_watchdog_tickle();
		/* prevent looping in case of failure in save_panic_log */
		result = power_set_nvram(kPowerNVRAMiBootErrorStageKey, 0);
		if (result != 0) goto panic_clear;
		boot_set_stage(kPowerNVRAMiBootStagePanicReboot);
		platform_init_mass_storage_panic();
		save_panic_log();
		/* reset the system, boot clean */
		dprintf(DEBUG_CRITICAL, "Panic saved, full reset.\n");
		platform_system_reset(false);
	} else if (error_stage == kPowerNVRAMiBootStageBooted) {
		dprintf(DEBUG_CRITICAL, "iBoot not saving panic log.\n");
	}

panic_clear:
	clear_panic_region(0);
}
#endif

int boot_iboot(void)
{
	return boot_flash(IMAGE_TYPE_IBOOT, BOOT_IBOOT);
}

#if WITH_DALI

int boot_dali_flash(void)
{
	addr_t		image_ptr;
	size_t		image_len;
	void* 		dali_arg = NULL;

	image_ptr = INSECURE_MEMORY_BASE;
	image_len = INSECURE_MEMORY_SIZE;

	PROFILE_ENTER('ILT');
	if (image_load_type(&image_ptr, &image_len, IMAGE_TYPE_DALI, 0)) {
		dprintf(DEBUG_CRITICAL, "Unable to load dali firmware from flash\n");
		return -1;
	}
	PROFILE_EXIT('ILT');

	dali_arg = target_prepare_dali();

	/* consolidate environment */
	security_consolidate_environment();

	dprintf(DEBUG_INFO, "dali: loaded image at %p, len %zu\n", (void *)image_ptr, image_len);

	/* boot it */
	prepare_and_jump(BOOT_DALI, (void *)image_ptr, dali_arg);
}

int do_daliboot(int argc, struct cmd_arg *args)
{
	u_int32_t type;

	if (argc < 2) {
		// After <rdar://problem/19269223>, ASP doesn't support finding Dali in the LLB blkdev. Initializing both LLB firmware
		// and ASP firmware at the same time is untested, so we are no longer supporting this debug path.
		printf("This variant of the dali command is no longer supported. However, you can still usb get / dali $loadaddr.\n");
		return -1;
	} else {
		const addr_t addr = args[1].u;
#if WITH_IMAGE3		
		if (security_allow_memory((void *)addr, DEFAULT_KERNEL_SIZE) &&
		    security_allow_modes(kSecurityModeExUntrust)) {
#elif WITH_IMAGE4
		if (security_allow_memory((void *)addr, DEFAULT_KERNEL_SIZE)) {
#endif			
			addr_t secure_addr = DEFAULT_KERNEL_ADDRESS;
			size_t secure_len = DEFAULT_KERNEL_SIZE;
			type = IMAGE_TYPE_DALI;
			if (image_load_memory(addr, DEFAULT_KERNEL_SIZE, &secure_addr, &secure_len, &type, 1, NULL, 0)) {
				printf("Memory image not valid\n");
			} else {
				void* 		dali_arg = NULL;
				dali_arg = target_prepare_dali();

				/* consolidate environment */
				security_consolidate_environment();

				printf("jumping into dali image at %p\n", (void *)secure_addr);
				prepare_and_jump(BOOT_DALI, (void *)secure_addr, dali_arg);
			}
		} else {
			printf("Permission Denied\n");
		}
	}
	
	return -1;

	
}
MENU_COMMAND_DEVELOPMENT(dali, do_daliboot, "[$loadaddr] boot into dali.", NULL);

#endif /* WITH_DALI */

#if WITH_FS
static int 
boot_diagnostics_fs(void)
{
	const char	*diags_paths;
	addr_t		image_ptr;
	size_t		image_len;
	char		bootfile[512];
	u_int32_t	type;

	/* try booting diags from the filesystem */
	diags_paths = env_get("diags-path");

	/* try to mount the boot file system */
	if (mount_bootfs() < 0) {
		dprintf(DEBUG_INFO, "root filesystem mount failed\n");
		return -1;
	}

	if (NULL != diags_paths) {

		dprintf(DEBUG_INFO, "loading diagnostics from %s\n", diags_paths);

		/* build the diags file */
		bootfile[0] = 0;
		strlcat(bootfile, "/boot", 512);
		strlcat(bootfile, diags_paths, 512);

		/* load the diags image */
		image_ptr = DEFAULT_KERNEL_ADDRESS;
		image_len = DEFAULT_KERNEL_SIZE;
		type = IMAGE_TYPE_DIAG;
		if (image_load_file(bootfile, &image_ptr, &image_len, &type, 1, NULL, 0) == 0) {
			dprintf(DEBUG_INFO, "loaded diagnostics at %p, len %zd\n", (void *)image_ptr, image_len);
		
			/* consolidate environment */
			security_consolidate_environment();

			/* boot it */
			prepare_and_jump(BOOT_DIAGS, (void *)image_ptr, NULL);
		}
	}

	fs_unmount("/boot");

	dprintf(DEBUG_CRITICAL, "failed to load diagnostics\n");

	return -1;
}
#endif /* WITH_FS */

int boot_diagnostics(void)
{
	/* try booting diags from a flash image first */
	boot_flash(IMAGE_TYPE_DIAG, BOOT_DIAGS);

#if WITH_FS
	boot_diagnostics_fs();
#endif
	return -1;
}

int do_diagboot(int argc, struct cmd_arg *args)
{
	u_int32_t type;

	if (argc < 2) {
		boot_diagnostics();
	} else {
		const addr_t addr = args[1].u;
#if WITH_IMAGE3		
		if (security_allow_memory((void *)addr, DEFAULT_KERNEL_SIZE) &&
		    security_allow_modes(kSecurityModeExUntrust)) {
#elif WITH_IMAGE4
		if (security_allow_memory((void *)addr, DEFAULT_KERNEL_SIZE)) {
#endif			
			addr_t secure_addr = DEFAULT_KERNEL_ADDRESS;
			size_t secure_len = DEFAULT_KERNEL_SIZE;
			type = IMAGE_TYPE_DIAG;
			if (image_load_memory(addr, DEFAULT_KERNEL_SIZE, &secure_addr, &secure_len, &type, 1, NULL, 0)) {
				printf("Memory image not valid\n");
			} else {
				/* consolidate environment */
				security_consolidate_environment();

				printf("jumping into diags image at %p\n", (void *)secure_addr);
				prepare_and_jump(BOOT_DIAGS, (void *)secure_addr, NULL);
			}
		} else {
			printf("Permission Denied\n");
		}
	}
	
	return -1;
}

MENU_COMMAND_DEVELOPMENT(diags, do_diagboot, "boot into diagnostics (if present)", NULL);

#if WITH_MACHO
void boot_darwin(void *ptr, void *arg)
{
#if WITH_CONSISTENT_DBG
	consistent_debug_update_ap_cpr(DBG_CPR_STATE_RUNNING, 0);
#endif
#if WITH_EFFACEABLE
	effaceable_consume_nonce(0);
#endif
	clear_panic_region(0);
	prepare_and_jump(BOOT_DARWIN, ptr, arg);
}

int do_bootx(int argc, struct cmd_arg *argv)
{
	int err = -1;
	addr_t entry;
	addr_t boot_args;
	addr_t addr = DEFAULT_LOAD_ADDRESS;
	u_int32_t type;

	PROFILE_ENTER('Btx');
	
#if !RELEASE_BUILD
	if (argc > 2 || ((argc > 1) && !strcmp("help", argv[1].str)))  {
		printf("usage:\n\t%s [<address>]\n", argv[0].str);
		err = -1;
		goto error;
	}

	if (argc > 1)
		addr = argv[1].u;
#endif
	
	if (!security_allow_memory((void *)addr, DEFAULT_KERNEL_SIZE)) {
		printf("Permission Denied\n");
		goto error;
	}
	
	printf("Attempting to validate kernelcache @ %p\n", (void *)addr);

// Image3 requires the non-restore types
#if WITH_IMAGE4
	type = IMAGE_TYPE_KERNELCACHE_RESTORE;
#else
	type = IMAGE_TYPE_KERNELCACHE;
#endif

	err = load_kernelcache(addr, DEFAULT_KERNEL_SIZE, type,  &entry, &boot_args);
	if (err < 0) {
		printf("error loading kernelcache\n");
		dprintf(DEBUG_INFO, "error loading kernelcache %d\n", err);
		goto error;
	}
	
	printf("kernelcache prepped at address %p\n", (void *)entry);

	PROFILE_EXIT('Btx');
	
	boot_darwin((void *)entry, (void *)boot_args);
error:
	return err;
}

/* This command is not used by release products other than those allowed to perform restore boot. */
#if !RELEASE_BUILD || WITH_RESTORE_BOOT
MENU_COMMAND(bootx, do_bootx, "boot a kernel cache at specified address", NULL);
#endif

int do_memboot(int argc, struct cmd_arg *argv)
{
	addr_t addr = DEFAULT_LOAD_ADDRESS;
	addr_t entry;
	addr_t boot_args;
	addr_t ramdisk_addr;
	size_t len;
	size_t ramdisk_size;
	struct blockdev *bdev = NULL;
	struct image_info *image = NULL;
	int err = -1;

	len = env_get_uint("filesize", 0);

#if !RELEASE_BUILD
	if ((argc > 3) || ((argc > 1) && !strcmp("help", argv[1].str))) {
		printf("usage:\n\t%s [<len>] [<address>]\n", argv[0].str);
		return -1;
	}

	addr = env_get_uint("loadaddr", (uintptr_t)DEFAULT_LOAD_ADDRESS);

	if (argc > 1)
	        len = argv[1].u;
	if (argc > 2)
	        addr = argv[2].u;
#endif

	if (len == 0) {
	        printf("filesize variable invalid or not set, aborting\n");
	        return -1;
	}
	if (len > DEFAULT_RAMDISK_SIZE) {
	        printf("Combo image too large\n");
	        return -1;
	}
	if (!security_allow_memory((void *)addr, len)) {
	        printf("Permission Denied\n");
	        return -1;
	}

	bdev = create_mem_blockdev("mem", (void *)addr, len, 64);

	image_search_bdev(bdev, 0, IMAGE_OPTION_MEMORY);
	image_dump_list(false);

#if WITH_TICKET
	err = ticket_load();
	if (err < 0) {
		dprintf(DEBUG_INFO, "Falling back to image3 validation\n");
	}
#endif

	err = load_ramdisk(&ramdisk_addr, &ramdisk_size);
	if (err < 0) {
		printf("error loading ramdisk\n");
		dprintf(DEBUG_INFO, "error loading ramdisk %d\n", err);
		goto error;
	}

	image = image_find(IMAGE_TYPE_KERNELCACHE);
	if (image == NULL) {
		printf("kernelcache not found\n");
		goto error;
	}

	err = load_kernelcache_image(image, IMAGE_TYPE_KERNELCACHE,  &entry, &boot_args);
	if (err < 0) {
		printf("error loading kernelcache\n");
		dprintf(DEBUG_INFO, "error loading kernelcache %d\n", err);
		goto error;
	}

	printf("kernelcache prepped at address %p\n", (void *)entry);
	
	boot_darwin((void *)entry, (void *)boot_args);
error:
	if (bdev != NULL) {
		image_free_bdev(bdev);
		free(bdev);
	}
	return err;
}

/* This command is not used by release products other than those allowed to perform restore boot. */
#if !RELEASE_BUILD || WITH_RESTORE_BOOT
MENU_COMMAND(memboot, do_memboot, "boot a combo devicetree/ramdisk/kernelcache at specified address", NULL);
#endif

#endif /* WITH_MACHO */

#if WITH_FS && WITH_ENV && WITH_MACHO && WITH_DEVICETREE
/*
 * Support for booting from a filesystem.
 */
int
mount_bootfs(void)
{
	const char *bootdevice;
	char bootsubdevice[128];

	bootdevice = env_get("boot-device");
	if (!bootdevice) {
		dprintf(DEBUG_INFO, "error: boot-device not set\n");
		return -1;
	}

	/* assume the first partition is the fs but allow an override */
	snprintf(bootsubdevice, 128, "%s%c", bootdevice, (char)env_get_uint("boot-partition", 0) + 'a');

	return mount_bootfs_from_device(bootsubdevice);
}

int
mount_bootfs_from_device(const char *device)
{
	int err;
	int i;
	const char *bootfs;

	/* see if the subdevice exists */
	if (!lookup_blockdev(device)) {
		dprintf(DEBUG_INFO, "boot device '%s' does not exist\n", device);
		platform_record_breadcrumb("mount_bootfs", "!lookup-blockdev");
		return -1;
	}

	/* iterate over filesystems looking for one that will mount this partition */
	for (i = 0; ; i++) {
		if ((bootfs = fs_get_fsname(i)) == NULL) {
			dprintf(DEBUG_INFO, "can't find a filesystem willing to mount the boot partition\n");
			platform_record_breadcrumb("mount_bootfs", "unknown-fs");
			return -1;
		}

		dprintf(DEBUG_INFO, "trying System mount from device '%s', fs '%s', at /boot\n", device, bootfs);
		err = fs_mount(device, bootfs, "/boot");
		if (err == 0) {
			break;
		}
		else {
			platform_record_breadcrumb_int("fs_mount_err", err);
		}
	}

	dprintf(DEBUG_INFO, "mount successful\n");

	return 0;
}

/* 
 * Mount the boot filesystem, load an optional ramdisk, load and boot the
 * configured kernelcache.
 */
int mount_and_boot_system(void)
{
	int		err;
	const char	*bootpath;
	const char	*bootramdisk;
	addr_t		entry;
	addr_t		boot_args;
	char		*bootfile;

	err = -1;
	bootfile = NULL;

	/* allocate storage for the kernelcache/ramdisk paths */
	bootfile = (char *)malloc(FS_MAXPATHLEN);

	/* find the kernelcache path */
	bootpath = env_get("boot-path");
	if (!bootpath) {
		dprintf(DEBUG_INFO, "error: boot-path not set\n");
		goto fail;
	}

	PROFILE_ENTER('Mfs');
	/* try to mount the boot file system */
	err = mount_bootfs();
	if (err < 0) {
		dprintf(DEBUG_CRITICAL, "root filesystem mount failed\n");
		goto fail;
	}
	PROFILE_EXIT('Mfs');

	/* look for optional ramdisk path, load it if set */
	bootramdisk = env_get("boot-ramdisk");
	if ((NULL != bootramdisk) && (0 != bootramdisk[0])) {
		/* build the ramdisk path */
		snprintf(bootfile, FS_MAXPATHLEN, "/boot%s", bootramdisk);

		/* load the ramdisk */
		err = load_ramdisk_file(bootfile);
		if (err < 0) {
			dprintf(DEBUG_CRITICAL, "ramdisk file invalid\n");
			goto fail;
		}
	}

	/* build the kernelcache path */
	snprintf(bootfile, FS_MAXPATHLEN, "/boot%s", bootpath);

	PROFILE_ENTER('LKF');
	/* load the kernelcache */
	err = load_kernelcache_file(bootfile, IMAGE_TYPE_KERNELCACHE, &entry, &boot_args);
	dprintf(DEBUG_INFO, "load_kernelcache returns %d, entry at %p\n", err, (void *)entry);
	if (err < 0)
		goto fail;
	PROFILE_ENTER('LKF');

	/* boot it */
	dprintf(DEBUG_INFO, "booting kernel\n");
	boot_darwin((void *)entry, (void *)boot_args);

	/* shouldn't get here */
	panic("returned from boot!\n");

fail:
	if (NULL != bootfile)
		free(bootfile);
	fs_unmount("/boot");	/* harmless if mount failed */
	return err;
}

static int
do_fsboot(int argc, struct cmd_arg *argv)
{

	if (argc >= 2 && !strcmp(argv[1].str, "help")) {
		printf("%s: load file from $boot-device on partition $boot-partition at path $boot-path\n", argv[0].str);
		return -1;
	}

	return mount_and_boot_system();
}

MENU_COMMAND_DEVELOPMENT(fsboot, do_fsboot, "boot kernelcache from filesystem", NULL);

#endif /* WITH_FS && WITH_ENV && WITH_MACHO && WITH_DEVICETREE */
