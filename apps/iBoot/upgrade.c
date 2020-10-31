/*
 * Copyright (C) 2010-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/*
 * Upgrade dependencies.
 */
#if WITH_FS && WITH_MACHO && WITH_DEVICETREE && WITH_ENV

#include <debug.h>
#include <lib/devicetree.h>
#include <lib/env.h>
#include <lib/fs.h>
#include <lib/image.h>
#include <lib/macho.h>
#include <lib/nvram.h>
#include <lib/paint.h>
#include <lib/ramdisk.h>
#include <lib/ticket.h>
#include <lib/blockdev.h>
#include <platform/memmap.h>
#include <platform.h>
#include <sys/boot.h>
#include <sys/security.h>

/*
 * Support for the 'upgrade' workflow.
 */

static int	mount_upgradefs(void);

static int mount_upgrade_partition(void);

extern void	boot_darwin(void *ptr, void *arg) __noreturn;

#define FORCE_FAIL_AT_IBOOT (1)
#define FORCE_FAIL_AT_IBEC  (1<<1)

#ifndef UPGRADE_PARTITION
# define UPGRADE_PARTITION	"2"
#endif
#ifndef UPGRADE_TICKET
# define UPGRADE_TICKET		"/boot/apticket.der"
#endif
#ifndef UPGRADE_LOADER
# define UPGRADE_LOADER		"/boot/iBEC"
#endif
#ifndef UPGRADE_KERNELCACHE
# define UPGRADE_KERNELCACHE	"/boot/kernelcache"
#endif
#ifndef UPGRADE_RAMDISK
# define UPGRADE_RAMDISK	"/boot/ramdisk"
#endif
#ifndef UPGRADE_DEVICETREE
# define UPGRADE_DEVICETREE	"/boot/devicetree"
#endif
#ifndef UPGRADE_PICTURE
# define UPGRADE_PICTURE	"/boot/applelogo"
#endif
#ifndef UPGRADE_BOOTARGS
# define UPGRADE_BOOTARGS	" rd=md0 nand-enable-reformat=1 -progress"
#endif


static bool upgrade_fallback_recovery()
{
#if RELEASE_BUILD
	return true;
#else
	return env_get_bool("enable-upgrade-fallback", false);
#endif
}

static void stash_failure_reason(const char* ota_result, const char* ota_failure_reason)
{
	dprintf(DEBUG_CRITICAL, "Recording upgrade failure:\n ota-result: %s\n ota-failure-reason: %s\n", ota_result, ota_failure_reason);
	env_set("ota-result", ota_result, ENV_PERSISTENT);
	env_set("ota-failure-reason", ota_failure_reason, ENV_PERSISTENT);
	nvram_save();

	platform_commit_breadcrumbs("ota-breadcrumbs");
}

/*
 * Mount the upgrade partition, check for iBEC and launch it if found.
 */
int
mount_and_upgrade_system(void)
{
	addr_t			addr;
	size_t			len;
	uint32_t		type, options;

	addr = DEFAULT_KERNEL_ADDRESS;
	len = DEFAULT_KERNEL_SIZE;

	char* upgrade_fail_reason = "";

#if !RELEASE_BUILD
	if (env_get_uint("force-upgrade-fail", 0) & FORCE_FAIL_AT_IBOOT)
	{
		upgrade_fail_reason = "simulated failure via force-upgrade-fail";
		dprintf(DEBUG_CRITICAL, "force-upgrade-fail is set. Simulating upgrade failure.\n");
		goto fail;
	}
#endif

	if (mount_upgrade_partition()) {
		upgrade_fail_reason = "failed to mount upgrade partition";
		dprintf(DEBUG_CRITICAL, "failed to mount upgrade partition\n");
		goto fail;
	}

#if WITH_TICKET
	if (ticket_load_file(UPGRADE_TICKET, DEFAULT_LOAD_ADDRESS, DEFAULT_LOAD_SIZE)) {
		dprintf(DEBUG_INFO, "WARNING: failed to load ticket %s\n", UPGRADE_TICKET);
	}
#endif

	/* image epoch must be greater than or equal to the current stage's epoch */
	/* Recovery mode restore: iBoot -> iBEC hand-off. Let image library enforce whatever policies it cares about. */
	options = IMAGE_OPTION_GREATER_EPOCH | IMAGE_OPTION_NEW_TRUST_CHAIN;

	/* try to load the iBEC from it */
	type = IMAGE_TYPE_IBEC;
	if (image_load_file(UPGRADE_LOADER, &addr, &len, &type, 1, NULL, options)) {
		upgrade_fail_reason = "failed to load upgrade iBEC";
		dprintf(DEBUG_CRITICAL, "could not load %s", UPGRADE_LOADER);
		goto fail;
	}

	/* consolidate environment */
	security_consolidate_environment();

#if WITH_SIDP
	/* Always seal the manifest that authorizes iBEC. */
	security_sidp_seal_boot_manifest(true);
#endif

	/* start it */
	prepare_and_jump(BOOT_IBOOT, (void *)addr, NULL);

	panic("%s returned", UPGRADE_LOADER);
fail:
	/* Stash away the reason why the upgrade failed */
	stash_failure_reason("failed to execute upgrade command from iBoot", upgrade_fail_reason);

	if (upgrade_fallback_recovery()) {
		// On a release build, fall back to booting the OS off NAND if the upgrade fails. As long as
		// we didn't boot into Restore OS, the old OS can still recover itself and save the customer
		// from being trapped in Recovery Mode.
		//

		dprintf(DEBUG_CRITICAL, "Upgrade failed, requesting reboot into old OS...\n");
		env_unset("boot-command");
		nvram_save();
		platform_system_reset(false);
	}

	return(-1);
}

/*
 * Boot from the upgrade partition.
 */
int
boot_upgrade_system(void)
{
	addr_t		entry;
	addr_t		boot_args;

	const char* upgrade_fail_reason = "unknown";

	if (upgrade_fallback_recovery()) {
		// On a release build, fall back to booting the OS off NAND if the upgrade fails. As long as
		// we didn't boot into Restore OS, the old OS can still recover itself and save the customer
		// from being trapped in Recovery Mode.
		//

		env_unset("boot-command");
		nvram_save();
	} else {
		// For internal users, we want to capture units that failed to boot into the upgrade environment.
		// Clear auto-boot to trap them into recovery mode.
		env_set("auto-boot", "false", ENV_PERSISTENT);
		nvram_save();
	}

	/* for release builds, the presence of a ramdisk forces suitable boot-args */
#ifndef RELEASE_BUILD
	{
		const char	*bootargs;
		char		*newbootargs;
		size_t		nbaSize;

		/* respect existing boot-args */
		if (NULL == (bootargs = env_get("boot-args")))
			bootargs = "";
		nbaSize = strlen(bootargs) + strlen(UPGRADE_BOOTARGS) + 1;
		newbootargs = malloc(nbaSize);
		
		snprintf(newbootargs, nbaSize, "%s%s", bootargs, UPGRADE_BOOTARGS);
		env_set("boot-args", newbootargs, 0);
		dprintf(DEBUG_INFO, "set boot-args %s\n", newbootargs);
		free(newbootargs);
	}
#endif

	/* mount the upgrade filesystem */
	if (mount_upgrade_partition()) {
		upgrade_fail_reason = "failed to mount upgrade partition";
		dprintf(DEBUG_CRITICAL, "failed to mount upgrade partition\n");
		// No need for additional breadcrumb here -- mount_upgrade_partition already
		// breadcrumbs every failure path.
		goto fail;
	}

#if WITH_TICKET
	if (ticket_load_file(UPGRADE_TICKET, DEFAULT_LOAD_ADDRESS, DEFAULT_LOAD_SIZE)) {
		dprintf(DEBUG_INFO, "WARNING: failed to load ticket %s\n", UPGRADE_TICKET);
	}
#endif

#if WITH_PAINT
	/* 
	 * If there's been an epoch change between the original bootloader and
	 * us running as iBEC from the upgrade partition, the Apple logo in
	 * the firmware space will not have been displayed.  Try to grab one
	 * from the upgrade partition.
	 *
	 * It's OK for this to fail - the system may not support images.
	 */
	paint_set_picture_from_file(UPGRADE_PICTURE, IMAGE_TYPE_LOGO);
#endif
	
	/* load the devicetree */
	if (dt_load_file(UPGRADE_DEVICETREE)) {
		upgrade_fail_reason = "failed to load devicetree";
		dprintf(DEBUG_CRITICAL, "failed to load devicetree\n");
		goto fail;
	}

	/* load the ramdisk */
	if (load_ramdisk_file(UPGRADE_RAMDISK)) {
		upgrade_fail_reason = "failed to load ramdisk";
		dprintf(DEBUG_CRITICAL, "failed to load ramdisk\n");
		goto fail;
	}			

	/* load the kernelcache */
#if WITH_IMAGE4
	if (load_kernelcache_file(UPGRADE_KERNELCACHE, IMAGE_TYPE_KERNELCACHE_RESTORE, &entry, &boot_args))
#else
	if (load_kernelcache_file(UPGRADE_KERNELCACHE, IMAGE_TYPE_KERNELCACHE, &entry, &boot_args))
#endif 
	{
		upgrade_fail_reason = "failed to load kernelcache";
		dprintf(DEBUG_CRITICAL, "failed to load kernelcache\n");
		goto fail;
	}

#if !RELEASE_BUILD
	if (env_get_uint("force-upgrade-fail", 0) & FORCE_FAIL_AT_IBEC)
	{
		upgrade_fail_reason = "simulated failure via force-upgrade-fail";
		dprintf(DEBUG_CRITICAL, "force-upgrade-fail is set. Simulating upgrade failure.\n");
		goto fail;
	}
#endif

	/* boot the kernel */
	dprintf(DEBUG_INFO, "booting upgrade kernel\n");
	boot_darwin((void *)entry, (void *)boot_args);

	panic("upgrade kernel returned");

fail:
	fs_unmount(UPGRADE_PARTITION);	/* harmless if mount failed */

	/* Stash away the reason why the upgrade failed */
	stash_failure_reason("failed to execute upgrade command from new iBEC", upgrade_fail_reason);

	if (upgrade_fallback_recovery()) {
		// For customer scenarios, reboot here. Since auto-boot will still be set and
		// the boot-command was cleared (to fsboot), we should attempt to reboot the original OS.
		dprintf(DEBUG_CRITICAL,"Upgrade command failed. Stashed failure details and requesting reboot\n");
		platform_system_reset(false);
	}

	/* return an error here so that we fall back into recovery mode */
	return(-1);
}

static int
mount_upgrade_partition(void)
{
	const char *device_name = NULL;

	/*
	 * Iterate over the published blockdevs, seeking the device marked as the
	 * upgrade partition, then mount it so the system can boot from it.
	 */
	for (struct blockdev *dev = first_blockdev(); dev != NULL; dev = next_blockdev(dev)) {
		if (dev->flags & BLOCKDEV_FLAG_UPGRADE_PARTITION) {
			device_name = dev->name;
			break;
		}
	}

	if (device_name == NULL) {
		dprintf(DEBUG_CRITICAL, "could not locate upgrade partition\n");
		platform_record_breadcrumb("mount_upgrade_partition", "!device_name");

		return -1;
	}

	dprintf(DEBUG_INFO, "mounting upgrade partition from device '%s'\n", device_name);
	if (mount_bootfs_from_device(device_name)) {
		dprintf(DEBUG_CRITICAL, "failed to mount upgrade partition");
		platform_record_breadcrumb("mount_upgrade_partition", "mount-failed");

		return -1;
	}

	return 0;
}

#endif /* WITH_FS etc. */
