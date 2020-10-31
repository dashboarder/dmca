/*
 * Copyright (C) 2007-2015 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <sys.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <platform.h>
#include <platform/memmap.h>
#include <drivers/power.h>
#include <lib/env.h>
#include <lib/nvram.h>
#include <lib/paint.h>
#include <lib/profile.h>
#include <lib/random.h>

#if WITH_MENU

static int do_reset(int argc, struct cmd_arg *args)
{
	platform_quiesce_display();

#if WITH_HW_POWER
	// Clear any pending PMU events
	power_clr_events(1);
#endif

	platform_system_reset(false);

	return 0;
}

static int do_halt(int argc, struct cmd_arg *args)
{
	halt();
}

MENU_COMMAND(reboot, do_reset, "reboot the device", NULL);
MENU_COMMAND(reset, do_reset, NULL, NULL);
MENU_COMMAND_DEVELOPMENT(halt, do_halt, "halt the system (good for JTAG)", NULL);

static int do_poweroff(int argc, struct cmd_arg *args)
{
	platform_poweroff();
	
	return 0;
}

MENU_COMMAND_DEVELOPMENT(poweroff, do_poweroff, "power off the device", NULL);

#if defined(WITH_TFTP)
static int do_eload(int argc, struct cmd_arg *args)
{
	char    cmdbuf[128];
	
	if (argc != 2) {
		printf("wrong number of args.\n");
		printf("%s <filename>\n", args[0].str);
		return(-1);
	}

	env_set("serverip", "17.202.24.178", 0);
	
	snprintf(cmdbuf, 128, "tftp getscript scripts/%s/%s.%s\n",
	    CONFIG_PROGNAME_STRING,
	    args[1].str,
	    CONFIG_BOARD_STRING);

	debug_run_script(cmdbuf);
	return(0);
}

MENU_COMMAND_DEVELOPMENT(eload, do_eload, "tftp via ethernet from hardcoded install server", NULL);
#endif /* WITH_TFTP */

#if !WITH_SIMPLE_MENU
struct memory_region_t
{
	memory_region_type_t	type;
	const char		*name;
};

static const struct memory_region_t memory_regions[] =
{
	{ kMemoryRegion_StorageProcessor,"Storage Processor"	},
	{ kMemoryRegion_ConsistentDebug, "Consistent Debug"	},
	{ kMemoryRegion_SleepToken,      "Sleep Token"		},
	{ kMemoryRegion_DramConfig,	 "DRAM Config"		},
	{ kMemoryRegion_Panic,           "Panic"		},
	{ kMemoryRegion_Display,         "Display"		},
	{ kMemoryRegion_Heap,            "Heap"			},
	{ kMemoryRegion_Stacks,          "Stacks"		},
	{ kMemoryRegion_PageTables,      "Page Tables"		},
	{ kMemoryRegion_iBoot,           "iBoot"		},
	{ kMemoryRegion_Kernel,          "Kernel"               },
	{ kMemoryRegion_Monitor,         "Monitor"		},
	{ kMemoryRegion_SecureProcessor, "Secure Processor"	},
	{ kMemoryRegion_AOP,		 "AOP"			},
	{ kMemoryRegion_Reconfig,	 "Reconfig"		},
};

void display_physical_memory_regions(bool final)
{
	unsigned int	i;
	uintptr_t	base;
	uintptr_t	end;
	size_t		size;

	printf("Physical Region           Start         End        Size\n");
	//     "xxxxxxxxxxxxxxxxxxx 0x123456789 0x123456789 0x123456789

	for (i = 0; i < ARRAY_SIZE(memory_regions); ++i) {
		base = platform_get_memory_region_base_optional(memory_regions[i].type);
		size = platform_get_memory_region_size_optional(memory_regions[i].type);

		if ((base == (uintptr_t)-1) || (size == (size_t)-1)) {
			continue;
		}

		end = base + size - 1;
		printf("%-19s 0x%09llx 0x%09llx 0x%09zx\n",
		       memory_regions[i].name, (uint64_t)base, (uint64_t)end, size);
	}
}

static int do_regions(int argc, struct cmd_arg *args)
{
	if (ARRAY_SIZE(memory_regions) != kMemoryRegion_NumberOfRegions) {
		panic("memory region list mismatch");
	}

	display_physical_memory_regions(false);
	return 0;
}

MENU_COMMAND_DEVELOPMENT(regions, do_regions, "show physical memory region allocations", NULL);
#endif	// !WITH_SIMPLE_MENU

static int do_echo(int argc, struct cmd_arg *args)
{
	int	i;

	for (i = 1; i < argc; i++)
		printf("%s ",args[i].str);
	printf("\n");

	return(0);
}

MENU_COMMAND_DEVELOPMENT(echo, do_echo, NULL, NULL);

#if WITH_PAINT
/* set the background color on the display */
static int do_bgcolor(int argc, struct cmd_arg *args)
{
	if (argc != 4) {
#if !RELEASE_BUILD
		printf("wrong number of args.\n");
		printf("%s <red> <green> <blue>\n", args[0].str);
#endif
		return -1;
	}
	
	paint_set_bgcolor(args[1].u, args[2].u, args[3].u);
	paint_update_image();
	
	return 0;
}

/* This command is not used by release products other than those allowed to perform restore boot. */
#if WITH_RECOVERY_MODE && (!RELEASE_BUILD || WITH_RESTORE_BOOT)
MENU_COMMAND(bgcolor, do_bgcolor, "set the display background color", NULL);
#endif

/* set the image on the display */
static int do_setpict(int argc, struct cmd_arg *args)
{
	struct image_info *image;
	int update = 0;
	int blank = 1;
	int restore = 0;
	int result;
	addr_t addr = DEFAULT_LOAD_ADDRESS;
	size_t size;

	size = env_get_uint("filesize", 0x00100000);

#if !RELEASE_BUILD
	if (argc < 2) {
		printf("wrong number of args.\n");
		printf("%s optbitmask [<addr> [<size>]]\n", args[0].str);
		printf("optbitmask:\n");
		printf("   0x01 = update the image on screen\n");
		printf("   0x02 = don't clear previous images\n");
		printf("   0x04 = restore mode image\n");
		return -1;
	}

        addr = env_get_uint("loadaddr", addr);
        
	update = (args[1].u & 1) != 0;
	blank = (args[1].u & 2) == 0;
        if (argc >= 3)
                addr = args[2].u;
        if (argc >= 4)
                size = args[3].u;
#endif

	if (argc >= 2) {
		restore = (args[1].u & 4) != 0;
	}

	if (size > 0x00100000) {
		printf("picture too large, size:%zu\n", size);
		return -1;
	}

	if (!security_allow_memory((void *)addr, size)) {
		printf("Permission Denied\n");
		return -1;
	}

	image = image_create_from_memory((void *)addr, size, 0);
	if (image == 0) {
		printf("Memory image corrupt\n");
		return -1;
	}
	
	if (blank) {
	    result = paint_set_picture(0);
	}

	result = paint_set_picture(image);

	// If this is a restore image, color remapping will have been disabled
	// when we dropped into recovery mode. Make sure the color remapping is
	// set to the policy desired by the device.
	if (restore) {
		paint_color_map_enable(paint_color_map_is_desired());
	}
	
	if ((update != 0) && (result == 0)) result = paint_update_image();
	
	return result;
}

/* This command is not used by release products other than those allowed to perform restore boot. */
#if WITH_RECOVERY_MODE && (!RELEASE_BUILD || WITH_RESTORE_BOOT)
MENU_COMMAND(setpicture, do_setpict, "set the image on the display", NULL);
#endif

/* XXX move to the paint module */
/* test the display */
static int do_displaytest(int argc, struct cmd_arg *args)
{
	paint_displaytest();
	
	return 0;
}

#if WITH_RECOVERY_MODE
MENU_COMMAND_DEBUG(displaytest, do_displaytest, "test the display", NULL);
#endif

/* Enable/Disable color remapping */
static int do_remapcolor(int argc, struct cmd_arg *args)
{
	bool old_state;
	bool new_state;

	if ((argc == 2) && !strcmp(args[1].str, "on")) {
	    new_state = true;
	} else if ((argc == 2) && !strcmp(args[1].str, "off")) {
	    new_state = false;
	} else if ((argc == 2) && !strcmp(args[1].str, "desired")) {
	    new_state = paint_color_map_is_desired();
	} else {
	    printf("usage: %s on|off|desired\n", args[0].str);
	    return -1;
	}

	old_state = paint_color_map_enable(new_state);

	if (new_state && (paint_color_map_is_enabled() != new_state)) {
		printf("Color remapping not supported for this target\n");
		return -1;
	}
	if (old_state != new_state) {
		paint_update_image();
	}

	return 0;
}

#if WITH_RECOVERY_MODE
MENU_COMMAND_DEBUG(remapcolor, do_remapcolor, "enable/disable color remap", NULL);
#endif

#endif /* WITH_PAINT */

static int do_display(int argc, struct cmd_arg *args)
{
	if ((argc == 2) && !strcmp(args[1].str, "on")) {
	    return platform_init_display();
	} else if ((argc == 2) && !strcmp(args[1].str, "off")) {
	    return platform_quiesce_display();
	} else {
	    printf("usage: %s on|off\n", args[0].str);
	    return -1;
	}
}

#if WITH_RECOVERY_MODE
MENU_COMMAND_DEBUG(display, do_display, "enable/disable display", NULL);
#endif

#if DEBUG_BUILD

#define kCommandLineSize	(256)
#define kMaxEntropyRadio	(1000)

static int do_random(int argc, struct cmd_arg *args)
{
	int		ret;
	char		*command_line = NULL;
	u_int8_t	*buffer;
	u_int32_t	save_data, length;
	u_int32_t	entropy_ratio, sweep;
	utime_t		start_time, end_time, delta_time;

	if (argc < 2) {
		dprintf(DEBUG_CRITICAL, "too few arguments\n");
		dprintf(DEBUG_CRITICAL, "usage: %s save_data? [entropy_ratio [<size> [<addr>]]]\n", args[0].str);

		return -1;
	}

	save_data = args[1].u;

	entropy_ratio = PLATFORM_ENTROPY_RATIO;
	if (argc >= 3)
		entropy_ratio = args[2].u;
	if (entropy_ratio >= kMaxEntropyRadio)
		entropy_ratio = kMaxEntropyRadio;
	sweep = entropy_ratio;
	if (entropy_ratio == 0) {
		entropy_ratio = 1;
		sweep = kMaxEntropyRadio;
	}

	length = 1024 * 1024;
        if (argc >= 4)
                length = args[3].u;

        buffer = (u_int8_t *)env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);
        if (argc >= 5)
                buffer = (u_int8_t *)args[4].u;

	if (!security_allow_memory(buffer, length)) {
		printf("Permission Denied\n");
		return -1;
	}

	if (save_data) {
		command_line = malloc(kCommandLineSize);
	}

	while (entropy_ratio <= sweep) {
		dprintf(DEBUG_CRITICAL, "Generating random numbers with entropy ratio %u at %p of length 0x%08x", entropy_ratio, buffer, length);

		start_time = system_time();
		random_get_bytes_debug(buffer, length, entropy_ratio);
		end_time = system_time();
		delta_time = end_time - start_time;

		dprintf(DEBUG_CRITICAL, " in %lld.%lldus/byte\n", delta_time / length, ((delta_time * 10) / length) % 10);

		if (save_data) {
			snprintf(command_line, kCommandLineSize,
				 "usb put /tmp/random-%s-%d.bin 0x%08x\n",
				 CONFIG_BOARD_STRING, entropy_ratio, length);

			process_command_line(command_line, &ret);
		}

		entropy_ratio *= 2;
	}

	if (save_data) {
		free(command_line);
	}

	return 0;
}

MENU_COMMAND_DEBUG(random, do_random, "generate random data", NULL);

#endif /* DEBUG_BUILD */

// *****************************************************************************
// Miscellaneous commands whose function bodies live in libraries
// *****************************************************************************

// -----------------------------------------------------------------------------
// Block device commands
// -----------------------------------------------------------------------------

#if WITH_BLOCKDEV
MENU_COMMAND_DEBUG(bdev, do_blockdev, "block device commands", NULL);
#endif

// -----------------------------------------------------------------------------
// Checksum commands
// -----------------------------------------------------------------------------

#if WITH_CKSUM && !WITH_SIMPLE_MENU
MENU_COMMAND_DEVELOPMENT(crc, do_crc, "POSIX 1003.2 checksum of specified memory address range.", NULL);
#endif

// -----------------------------------------------------------------------------
// Devicetree commands
// -----------------------------------------------------------------------------

/* This command is not used by the release iBoot application iBoot product */
#if !RELEASE_BUILD || !APPLICATION_IBOOT || !PRODUCT_IBOOT
int do_devicetree(int argc, struct cmd_arg *argv);
static int do_devicetree_wrapper(int argc, struct cmd_arg *argv)
{
	int result;

	PROFILE_ENTER('Dtr');
	result = do_devicetree(argc, argv);
	PROFILE_EXIT('Dtr');
	return result;
}

MENU_COMMAND(devicetree, do_devicetree_wrapper, "create a device tree from the specified address", NULL);
#endif

#if WITH_DEVICETREE
void dump_tree_int(dt_node_t *node, int indent, bool verbose);
static int do_dump_tree(int argc, struct cmd_arg *args)
{
	dt_node_t *node;

	if (dt_get_size() == 0) {
		printf("no devicetree loaded\n");
		return -1;
	}

	if (argc == 2) {
		printf("Looking for node \"%s\"\n", args[1].str);
		if (!dt_find_node(0, args[1].str, &node)) {
			printf("node not found\n");
			return -1;
		}
	} else {
		node = dt_get_root();
	}

	dump_tree_int(node, 0, false);

	return 0;
}

MENU_COMMAND_DEBUG(dtdump, do_dump_tree, "dump devicetree", NULL);
#endif

// -----------------------------------------------------------------------------
// File system commands
// -----------------------------------------------------------------------------

#if WITH_FS
MENU_COMMAND_DEVELOPMENT(fs, do_fs, "file system commands", NULL);
#endif

// -----------------------------------------------------------------------------
// Environment variable commands
// -----------------------------------------------------------------------------

#if WITH_ENV

int do_saveenv(int argc, struct cmd_arg *args)
{
#if WITH_NVRAM
	return nvram_save();
#else
	dprintf(DEBUG_CRITICAL, "WARNING: with no NVRAM storage available, 'saveenv' has no effect\n");
	return 0;
#endif
}

MENU_COMMAND_DEVELOPMENT(clearenv, do_clearenv, "clear all environment variables", NULL);
MENU_COMMAND_DEVELOPMENT(printenv, do_printenv, "print one or all environment variables", NULL);
MENU_COMMAND(getenv, do_getenv, "get environment variable over usb", NULL);
MENU_COMMAND(saveenv, do_saveenv, "save current environment to flash", NULL);
MENU_COMMAND(setenv, do_setenv, "set an environment variable", NULL);
MENU_COMMAND_DEBUG(envprot, do_envprot, "enable/disable NVRAM whitelist", NULL);
#endif

// -----------------------------------------------------------------------------
// Heap commands
// -----------------------------------------------------------------------------

MENU_COMMAND_DEBUG(malloc, do_malloc, "examine the system heap", NULL);

// -----------------------------------------------------------------------------
// Module Information Base (MIB) commands
// -----------------------------------------------------------------------------

#if !WITH_SIMPLE_MENU
MENU_COMMAND_DEVELOPMENT(mib, do_mib, "dump the MIB", NULL);
#endif

// -----------------------------------------------------------------------------
// Nonce commands
// -----------------------------------------------------------------------------

MENU_COMMAND_DEBUG(nonce, do_nonce, "nonce utilities", NULL);

// -----------------------------------------------------------------------------
// RAMDISK commands
// -----------------------------------------------------------------------------

/* This command is not used by release products other than those allowed to perform restore boot. */
#if WITH_RAMDISK && (!RELEASE_BUILD || WITH_RESTORE_BOOT)

int do_ramdisk(int argc, struct cmd_arg *argv);
static int do_ramdisk_wrapper(int argc, struct cmd_arg *argv)
{
	int result;

	PROFILE_ENTER('RDk');
	result = do_ramdisk(argc, argv);

	PROFILE_EXIT('RDk');
	return result;
}

MENU_COMMAND(ramdisk, do_ramdisk_wrapper, "create a ramdisk from the specified address", NULL);
#endif

// -----------------------------------------------------------------------------
// Syscfg commands
// -----------------------------------------------------------------------------

#if WITH_SYSCFG
MENU_COMMAND_DEVELOPMENT(syscfg, do_syscfg, "flash SysCfg inspection", NULL);
#endif

// -----------------------------------------------------------------------------
// Ticket commands
// -----------------------------------------------------------------------------

#if WITH_TICKET
MENU_COMMAND( ticket, do_ticket, "", NULL );
#if WITH_FS
MENU_COMMAND_DEBUG( ticketfile, do_ticket_file, "load ticket from file", NULL );
#endif
MENU_COMMAND_DEBUG( ticketdump, do_ticket_dump, "", NULL );
MENU_COMMAND_DEBUG( ticketload, do_ticket_load, "", NULL );
#endif

#if WITH_PAINT
static int do_paint_info(int argc, struct cmd_arg *args)
{

	if (argc == 1) {
		display_paint_information();
	} else {
		if (!strcmp("canvas", args[1].str)) {
			display_paint_canvas();
		} else if (!strcmp("plane", args[1].str)) {
			uint32_t plane = args[2].u;
			display_paint_plane(plane);
		}
	}
	return 0;
}

MENU_COMMAND_DEBUG(paintinfo, do_paint_info, "Display Paint related information", NULL);
#endif

#endif /* WITH_MENU */
