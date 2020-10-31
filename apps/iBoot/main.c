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
#include <drivers/display.h>
#include <drivers/power.h>
#include <lib/blockdev.h>
#include <lib/profile.h>
#include <lib/env.h>
#include <lib/fs.h>
#include <lib/macho.h>
#include <lib/mib_def.h>
#include <lib/paint.h>
#include <lib/partition.h>
#if WITH_USB_DFU
#include <drivers/usb/usb_public.h>
#endif
#if WITH_TBT_DFU
#include <drivers/thunderbolt/thunderboot.h>
#endif
#include <platform.h>
#include <platform/gpio.h>
#include <platform/memmap.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <target.h>
#if WITH_TICKET
#include <lib/ticket.h>
#endif
#if WITH_NAND_FILESYSTEM
#include <drivers/flash_nand.h>
#endif

#if WITH_VOLTAGE_KNOBS
#include <drivers/voltage_knobs/knobs.h>
#endif

#if SUPPORT_FPGA
#define USER_BUTTON_POWERON_TIMEOUT	(0 * 1000 * 1000)
#else
// at least 400 ms must elapse since power on due to power button press
#define USER_BUTTON_POWERON_TIMEOUT	(400 * 1000)
#endif
#define USER_BUTTON_POWEROFF_TIMEOUT	(5 * 1000 * 1000)
#define USER_BUTTON_RECOVER_TIMEOUT	(5 * 1000 * 1000)

static void	main_llb(void);
static void	main_ibss(void);
static void	main_generic(void);
static void	main_dfu(void);

static void	do_iboot_autoboot(void);
static void	do_ibec_autoboot(void);

struct autoboot_command
{
	const char	*command;
	int		(*func)(void);
};
static bool	check_autoboot(bool autoboot, uint32_t bootdelay);
static void	do_common_autoboot(struct autoboot_command *cmds);

static void	do_ll_init(void);

static void	do_boot_ui(void);
static void	do_recoverymode_ui(void);

static int	main_task(void *arg);
static int	poweroff_task(void *arg);
static int	idleoff_task(void *arg);

static void	print_boot_banner(void);

static void	check_enter_dfu(void);

#if WITH_TICKET
static bool dfu_parse_ticket(addr_t load_address, size_t load_length, addr_t *new_load_address, size_t *new_load_length);
#endif
static bool	dfu_console_init(void);
static addr_t	dfu_console_loadaddr(addr_t load_address);
static bool	dfu_console_loadctrl(addr_t load_address, size_t load_length);
static int	do_dfu_cmd(int argc, struct cmd_arg *args);

static void	init_console_buffer(size_t size);

/*
 * Toplevel main()
 */
void 
_main(void)
{
	PROFILE_INIT();

	/* initialize the cpu */
	arch_cpu_init(false);

	PROFILE_EXIT('ACI');

	/* main logic flow varies based on what we are building */
#if PRODUCT_LLB
	main_llb();
#elif PRODUCT_IBSS
	main_ibss();
#else
	main_generic();
#endif
}

/*
 * Main logic flow for LLB.
 */
static void
main_llb(void)
{
	bool			resume;
	bool			poweron, cold_button_boot = false;

	/* do low-level SoC init */
	do_ll_init();

	/* bring up system services (cpu, tasks, callout, heap) */
	sys_init();
	PROFILE_EXIT('SyI');

#if !WITH_NO_RANDOM_STACK_COOKIE
	/* Generate random stack cookie for platforms that
	 * don't have a fast resume path
	 */
	sys_init_stack_cookie();
	PROFILE_EXIT('SIS');
#endif

#if !RELEASE_BUILD
	init_console_buffer(4096);
#endif

	/* do some early initialization of hardware */
	/* timers will start firing at this point */
	dprintf(DEBUG_INFO, "doing early platform hardware init\n");
	platform_early_init();
	PROFILE_EXIT('PEI');

	/* reset watchdog timer in case it's enabled */
	platform_watchdog_tickle();

	/* Detect cold boot, and power on conditions */
	poweron = target_should_poweron(&cold_button_boot);

	/* Detect if device has been suspended */
	resume = power_is_suspended();

	/* initialize sdram */
	dprintf(DEBUG_INFO, "initializing main memory\n");
	PROFILE_ENTER('PIM');
	platform_init_mainmem(resume);
	PROFILE_EXIT('PIM');

	/* try to restore_system */
	if (resume) {
#if WITH_VOLTAGE_KNOBS
		knobs_load_from_standby_storage();
#endif
        
		platform_restore_system();

		/* Suspended memory is no longer valid, re-init security */
		security_init(true);
		boot_set_stage(kPowerNVRAMiBootStageBooted);

		/*
		 * If we get here, resume has failed and we are going to
		 * re-start the system.  Note that we don't want to do the
		 * user poweron detection as the user already expects that
		 * the system is on.
		 */

	} else {
#if WITH_NO_RANDOM_STACK_COOKIE
		/* generate random stack cookie now that we know we
		 * aren't on the fast resume path
		 */
		sys_init_stack_cookie();
		PROFILE_EXIT('SIS');
#endif
        
#if WITH_VOLTAGE_KNOBS
		// prepare for voltage knobs adjustments
		knobs_prepare_standby_storage();
#endif
        
		/* Wait for a user poweron request if started */
		while (cold_button_boot) {
#if WITH_TARGET_CHARGETRAP
			if (target_needs_chargetrap()) {
				// If charge trap is implemented in the target (e.g. Dali Mode),
				// we should immediately proceed with boot if we know we need to
				// boot into it.
				break;
			}
#endif 
			poweron &= target_should_poweron(&cold_button_boot);
			if (!poweron) {
				dprintf(DEBUG_CRITICAL, "Power on canceled.\n");
				platform_poweroff();
			}
			
			// Ensure that at least USER_BUTTON_POWERON_TIMEOUT number of us have elapsed since power on.
			// Start time is 0 because timer on H3+ platforms will start counting up from 0 after power on.
			if (time_has_elapsed(0, USER_BUTTON_POWERON_TIMEOUT)) {
				power_cancel_buttonwait();
				break;
			}
			
			spin(10 * 1000);
		}
	}

#ifdef HEAP_EXT_SIZE
	/* 
	 * If we have extra memory for the heap, add it.  This needs
	 * to happen after main memory has been initialized, after any
	 * decision has been whether to resume, but before
	 * platform_init() gets called.
	 */
	dprintf(DEBUG_INFO, "Adding 0x%llx bytes at %p to heap.\n", HEAP_EXT_SIZE, (void*)HEAP_EXT_BASE);
	PROFILE_ENTER('Hep');
	heap_add_chunk((void *)HEAP_EXT_BASE, HEAP_EXT_SIZE, true);
	PROFILE_EXIT('Hep');
#endif

	/* initialize the rest of hardware */
	dprintf(DEBUG_INFO, "doing platform hardware init\n");
	PROFILE_ENTER('PIn');
	platform_init();
	PROFILE_EXIT('PIn');

	/* check for external request to enter DFU (if detected then enter DFU) */
	check_enter_dfu();

	dprintf(DEBUG_INFO, "looking for boot images\n");
	find_boot_images();

#if WITH_TICKET
	/* load the root ticket */
	ticket_load();
#endif

	/* find iBoot and boot it */
	boot_iboot();

	/* failed to load system */
	dprintf(DEBUG_CRITICAL, "do_boot: failed to find anything to load\n");
	
	/* re-init security after failed iBoot load */
	security_init(false);

	dprintf(DEBUG_CRITICAL, "LLB done, failed to boot asking for DFU...\n");
	main_dfu();
}

static void 
main_dfu(void)
{
#if WITH_USB_DFU || WITH_TBT_DFU
	bool autoboot = dfu_console_init();

	while (1)  {
		int			result;
		addr_t			load_address;
		size_t			load_length;
		uint32_t		type, options;

		/* reset secuirity and clear the insecure memory area */
		security_init(true);

		load_address = INSECURE_MEMORY_BASE;
		load_length = INSECURE_MEMORY_SIZE;

		if (!autoboot) {
			load_address = dfu_console_loadaddr(load_address);
		}

#if WITH_USB_DFU
		result = getDFUImage((void *)load_address, load_length);
#elif WITH_TBT_DFU
		result = thunderboot_get_dfu_image((void *)load_address, load_length);
#else
#error "No valid DFU driver"
#endif
		if (result < 0) {
			dprintf(DEBUG_INFO, "fatal DFU download error");
			continue;
		}

		load_length = result;

		if (!autoboot && dfu_console_loadctrl(load_address, load_length)) {
			continue;
		}

#if WITH_TICKET
		if (!dfu_parse_ticket(load_address,
					   load_length,
					   &load_address,
					   &load_length) ) {
			continue;
		}
#endif

		/* image epoch must be greater than or equal to the current stage's epoch */
		options = IMAGE_OPTION_GREATER_EPOCH;

#if PRODUCT_LLB
		/* llb in usb-dfu mode is starting a new boot-chain, so tells library to enforce policies it cares about. */
		options |= IMAGE_OPTION_NEW_TRUST_CHAIN;
#endif

		type = IMAGE_TYPE_IBEC;
		if (0 == image_load_memory(load_address,	/* fromAddress */
					   load_length, 	/* fromLength */
					   &load_address, 	/* address */
					   &load_length, 	/* length */
					   &type,		/* list of types */
					   1,			/* count of types */
					   NULL,		/* actual type not needed */
					   options)) {
			dprintf(DEBUG_CRITICAL, "executing image...\n");

			/* consolidate environment */
			security_consolidate_environment();

#if WITH_SIDP
			/* Always seal the manifest that authorizes iBEC. */
			security_sidp_seal_boot_manifest(true);
#endif

			prepare_and_jump(BOOT_IBOOT, (void *)load_address, NULL);
			break;
		}

		dprintf(DEBUG_INFO, "image load failed\n");
	}
#endif /* WITH_USB_DFU || WITH_TBT_DFU */

	/* DFU failed,  poweroff the system */
	platform_poweroff();

	panic("poweroff failed");
}

/*
 * Early main logic flow for iBSS.
 */
static void
main_ibss(void)
{
	/* do low-level SoC init */
	do_ll_init();

	/* bring up system services (cpu, tasks, callout, heap) */
	sys_init();
	PROFILE_EXIT('SyI');

	/* generate random stack cookie */
	sys_init_stack_cookie();
	PROFILE_EXIT('SIS');

#if !RELEASE_BUILD
	init_console_buffer(4096);
#endif

	/* do some early initialization of hardware */
	/* timers will start firing at this point */
	dprintf(DEBUG_INFO, "doing early platform hardware init\n");
	platform_early_init();
	PROFILE_EXIT('PEI');

	/* reset watchdog timer in case it's enabled */
	platform_watchdog_tickle();

#if WITH_VOLTAGE_KNOBS
	// prepare for voltage knobs adjustments
	knobs_prepare_standby_storage();
#endif
	
	/* initialize sdram */
	dprintf(DEBUG_INFO, "initializing main memory\n");
	platform_init_mainmem(false /*resume*/);
	PROFILE_EXIT('PIM');

#if WITH_DFU_MODE
	dprintf(DEBUG_CRITICAL, "iBSS ready, asking for DFU...\n");
	platform_init();
	PROFILE_EXIT('PIn');
	main_dfu();
#elif WITH_RECOVERY_MODE
	/* start a task to finish setting up the system and boot */
	/* this also effectively moves the system stack to the heap */
	task_start(task_create("main", main_task, NULL, 0x1C00));
#else
	#error "!(WITH_DFU_MODE || WITH_RECOVERY_MODE)"
#endif

	task_exit(0);
}

/*
 * Early main logic flow for non LLB/iBSS products..
 */
static void
main_generic(void)
{
	/* bring up system services (cpu, tasks, callout, heap) */
	sys_init();
	PROFILE_EXIT('SyI');

	/* generate random stack cookie */
	sys_init_stack_cookie();
	PROFILE_EXIT('SIS');

#if !RELEASE_BUILD
	init_console_buffer(100*1024);
#endif

	/* do some early initialization of hardware */
	/* timers will start firing at this point */
	dprintf(DEBUG_INFO, "doing early platform hardware init\n");
	platform_early_init();

	/* reset watchdog timer in case it's enabled */
	platform_watchdog_tickle();

	/* start a task to finish setting up the system and boot */
	/* this also effectively moves the system stack to the heap */
	task_start(task_create("main", main_task, NULL, 0x1C00));

	task_exit(0);
}


/*
 * Main task, chained from main() by recovery mode products.
 */
utime_t gPowerOnTime;
utime_t gDebugPromptTime;
utime_t gLoadKernelTime;

static utime_t	poweroff_time;
static utime_t	recover_time;

static int
main_task(void *arg)
{
	bool		need_precharge = false;

	PROFILE_ENTER('Mai');

	gPowerOnTime = system_time();

	/*
	 * Test for target-specific power-off request (e.g. button
	 * being pressed).  Start the timer if so.
	 */
	if (target_should_poweroff(true))
		poweroff_time = system_time();

	/*
	 * Test for target-specific recovery-mode request (e.g.
	 * button being pressed).  Start the timer if so.
	 */
	if (target_should_recover()) {
		recover_time = system_time();
		power_cancel_buttonwait();
	}
   
	/* call this early to sample boot voltage before things are turned on */
	need_precharge = power_needs_precharge();

	/* set full performance early if precharge not needed */
	if (!need_precharge)
		platform_set_performance(kPerformanceHigh);

	/* 
	 * Suspended memory is no longer valid, re-init security.
	 * This needs to happen after main memory has initialized and
	 * before it has been added to the heap.  In addition, this
	 * needs to happen before any images are loaded.
	 */
	PROFILE_ENTER('ScI');
	security_init(true);
	PROFILE_EXIT('ScI');

#ifdef HEAP_EXT_SIZE
	/* 
	 * If we have extra memory for the heap, add it.  This needs
	 * to happen after main memory has been initialized, after
	 * security_init() has been called, but before platform_init()
	 * gets called.
	 */
	dprintf(DEBUG_INFO, "Adding 0x%llx bytes at %p to heap.\n", HEAP_EXT_SIZE, (void*)HEAP_EXT_BASE);
	bool clear_heap;
#ifdef SECURE_MEMORY_SIZE
	clear_heap = (HEAP_EXT_BASE < SECURE_MEMORY_BASE) ||
		     (HEAP_EXT_BASE + HEAP_EXT_SIZE > SECURE_MEMORY_BASE + SECURE_MEMORY_SIZE);
#else
	clear_heap = true;
#endif
	PROFILE_ENTER('Hep');
	heap_add_chunk((void *)HEAP_EXT_BASE, HEAP_EXT_SIZE, clear_heap);
	PROFILE_EXIT('Hep');
#endif

	/* initialize the rest of hardware */
	dprintf(DEBUG_INFO, "doing platform hardware init\n");
	platform_init();
	PROFILE_EXIT('PIn');

	/* initialize the system environment from nvram */
	sys_load_environment();
	PROFILE_EXIT('LdE');
	
#if WITH_VOLTAGE_KNOBS
	// load voltage knobs from system environment, update PMU scratch registers, and apply knobs
	knobs_update_PMU_registers(true);
#endif

	/* Turn on uart debugging if requested */
	debug_enable_uarts(env_get_uint("debug-uarts", 0));

#if PRODUCT_IBOOT && WITH_HW_POWER && WITH_BOOT_STAGE
	/* save and reboot if this is from a soft-reset for panic logging */
	boot_check_panic();
#endif

	/* Note that find_boot_images() is the first point at which
	 * syscfg services must be available.  For boot-from-nand,
	 * this means that Whimory must be initialized by this point.
	 */
	dprintf(DEBUG_INFO, "looking for boot images\n");
	find_boot_images();

#if WITH_TICKET
	/* load the root ticket */
	ticket_load();
#endif

	/* Performance needs to be atleast medium for the display */
	if (need_precharge)
		platform_set_performance(kPerformanceMedium);

#if WITH_TARGET_CHARGETRAP
	target_do_chargetrap();
#else
	power_do_chargetrap();
#endif

	/* Turn on the display */
	dprintf(DEBUG_INFO, "initializing display\n");
	PROFILE_ENTER('PID');
	platform_init_display();
	PROFILE_EXIT('PID');

#if WITH_HW_POWER
	power_dark_boot_checkpoint();
#endif
	/* Draw the apple logo */
	do_boot_ui();

	/* Let backlight go to full if needed */
	PROFILE_ENTER('PID');
	platform_init_display();
	PROFILE_EXIT('PID');

	platform_set_performance(kPerformanceHigh);

	/* 
	 * If we saw a power-off request on entry and we are
	 * still seeing it now, wait out the rest of the timer
	 * to decide whether to power off.
	 */
	while (poweroff_time && target_should_poweroff(true)) {
		if (time_has_elapsed(poweroff_time, USER_BUTTON_POWEROFF_TIMEOUT)) {
			dprintf(DEBUG_INFO, "Force power off.\n");
			platform_poweroff();
		}
		task_sleep(10 * 1000);
        }

	PROFILE_ENTER('PLI');
	platform_late_init();
	PROFILE_EXIT('PLI');

	print_boot_banner();

#if WITH_HW_POWER
	power_dark_boot_checkpoint();
#endif

#if WITH_NAND_BOOT
	/* Enable breadcrumbing subsystem */
	platform_init_breadcrumbs();
#endif

	/* do auto-boot for products that support it */
#if PRODUCT_IBOOT
	do_iboot_autoboot();
#elif PRODUCT_IBEC
	do_ibec_autoboot();
#endif

	boot_set_stage(kPowerNVRAMiBootStageOff);

	/* show UI to indicate that we are entering recovery mode */
	do_recoverymode_ui();

	/* we're entering recovery mode, bring up any additional hardware */
	platform_debug_init();

	task_start(task_create("poweroff", poweroff_task, NULL, 0x200));

	/* start the command prompt/parser */
	printf("Entering recovery mode, starting command prompt\n");
#if defined(MENU_TASK_SIZE)	
	task_start(task_create("command", menu_task, NULL, MENU_TASK_SIZE));
#else
	task_start(task_create("command", menu_task, NULL, 0x1C00));
#endif	

	/* sample activity time before starting idleoff task. */
	gMenuLastActivityTime = system_time();

#if WITH_ENV
	if (env_get_bool("idle-off", false))
		task_start(task_create("idleoff", idleoff_task, NULL, 0x200));
#endif

	PROFILE_EXIT('Mai');
	return -1;
}

/*
 * Boot a script (custom set of commands) through boot-command interface.  
 */
static int
boot_script(void)
{
	debug_run_script("run boot-script");

	return 0;
}

/*
 * Auto-boot flow for iBoot.
 */
static void
do_iboot_autoboot(void)
{
	uint32_t	boot_fail_count = 0, panic_fail_count = 0;
	static struct autoboot_command iboot_bootcommands[] = {
		{"fsboot",	mount_and_boot_system},
		{"diags",	boot_diagnostics},
		{"upgrade",	mount_and_upgrade_system},
#if DEBUG_BUILD		
		{"script",	boot_script},
#endif		
		{NULL, NULL}
       	};

	/* bring up mass storage devices */
	/* XXX this should be done lazily the first time we use it */
	dprintf(DEBUG_INFO, "initializing mass storage\n");
	PROFILE_ENTER('PMS');
	platform_init_mass_storage();
	PROFILE_EXIT('PMS');

#if WITH_HW_POWER
	// At this point, NVRAM can be written. To prevent dark boot loops, clear out the dark boot flag
	power_clear_dark_boot_flag();
#endif

	/* Check for diag dock and force diags */
	if (power_get_diags_dock()) {
		boot_diagnostics();
	}

	/* Check whether we have failed to boot too many times, and abandon auto-boot if so */
	if (boot_error_count(&boot_fail_count, &panic_fail_count) == 0) {
		dprintf(DEBUG_CRITICAL, "Boot Failure Count: %u\tPanic Fail Count: %u\n", boot_fail_count, panic_fail_count);
		if (false && (boot_fail_count >= 5)) {
			dprintf(DEBUG_CRITICAL, "Too many boot failures, forcing recovery mode...\n");
			return;
		}
	}

	PROFILE();
	/* Wait for a user recover request if started */
	/* XXX should this be hoisted to common code? */
	while (recover_time && target_should_recover()) {
		if (time_has_elapsed(recover_time, USER_BUTTON_RECOVER_TIMEOUT)) {
			dprintf(DEBUG_INFO, "Force recovery mode....\n");
			return;
		}
		task_sleep(10 * 1000);
        }
	PROFILE();

	/* handle auto-boot/bootcommand as legal for iBoot */
	do_common_autoboot(iboot_bootcommands);
}

/*
 * Auto-boot flow for iBEC.
 */
static void
do_ibec_autoboot(void)
{
	uint32_t	boot_fail_count = 0, panic_fail_count = 0;
	static struct autoboot_command ibec_bootcommands[] = {
		{"upgrade",	boot_upgrade_system},
		{NULL, NULL}
	};

	/* bring up mass storage devices */
	/* XXX this should be done lazily the first time we use it */
	dprintf(DEBUG_INFO, "initializing mass storage\n");
	platform_init_mass_storage();

#if WITH_HW_POWER
	// At this point, NVRAM can be written. To prevent dark boot loops, clear out the dark boot flag
	power_clear_dark_boot_flag();
#endif
	
	/* Check whether we have failed to boot too many times, and abandon auto-boot if so */
	if (boot_error_count(&boot_fail_count, &panic_fail_count) == 0) {
		dprintf(DEBUG_CRITICAL, "Boot Failure Count: %u\tPanic Fail Count: %u\n", boot_fail_count, panic_fail_count);
		if (false && (boot_fail_count >= 5)) {
			dprintf(DEBUG_CRITICAL, "Too many boot failures, forcing recovery mode...\n");
			return;
		}
	}

	/* handle auto-boot/bootcommand as legal for iBEC */
	do_common_autoboot(ibec_bootcommands);
}

/*
 * Common auto-boot check allowing for bootdelay abort.
 */
static bool
check_autoboot(bool autoboot, uint32_t bootdelay)
{
	/* see if we need to delay a bit on debug builds */
	if (autoboot) {
		/* delay for a bit according to the bootdelay variable */
		utime_t t;

		printf("Delaying boot for %d seconds. Hit enter to break into the command prompt...\n", bootdelay);
		t = system_time();
		while (1) {
			int c = debug_getchar_nowait();

			if (c == '\n' || c == '\r') {
				printf("aborting autoboot due to user intervention.\n");
				autoboot = false;
				break;
			}

			if ((system_time()) - t > (bootdelay * 1000000)) 
				break;

			task_sleep(10 * 1000);
		}
	}
	
	return autoboot;
}


/*
 * Common auto-boot flow.
 */
static void
do_common_autoboot(struct autoboot_command *cmds)
{
	bool	autoboot;
	int	i;

	/* see if we should auto-boot or drop into the debugger */
	autoboot = env_get_bool("auto-boot", false);

	gDebugPromptTime = system_time();

#if WITH_HW_POWER
	power_dark_boot_checkpoint();
#endif
	/* 
	 * Handle auto-boot.
	 *
	 * Note that we only support a limited set of boot-command directives.  Honouring
	 * an arbitrary command here could be abused by an attacker to perform arbitrary
	 * operations as part of an exploit.
	 */
	if (check_autoboot(autoboot, env_get_uint("bootdelay", 1))) {
		const char *bootcommand = env_get("boot-command");

		if (!bootcommand) {
			dprintf(DEBUG_CRITICAL, "auto-boot set but no boot-command, aborting boot\n");
			return;
		}

		/* XXX this should move */
		gLoadKernelTime = system_time();

		/* compare found command with list of legal commands */
		for (i = 0; NULL != cmds[i].command; i++) {
			if (!strcmp(bootcommand, cmds[i].command)) {
#if WITH_HW_POWER
				power_dark_boot_checkpoint();
#endif
				cmds[i].func();
				
				/* auto-boot failed, fall through to recovery mode */
				return;
			}
		}
		dprintf(DEBUG_CRITICAL, "boot-command '%s' not supported\n", bootcommand);
	}
}

/*
 * Do low-level init as the first component running after the ROM.
 */
static void
do_ll_init(void)
{
	PROFILE_ENTER('DLI');
	/* setup the default clock configuration */
	dprintf(DEBUG_INFO, "setting up initial clock configuration\n");
	platform_init_setup_clocks();

	/* set up the default pin configuration */
	dprintf(DEBUG_INFO, "setting up default pin configuration\n");
	platform_init_hwpins();

	/* set up any internal memory (internal sram) */
	dprintf(DEBUG_INFO, "setting up internal memory\n");
	platform_init_internal_mem();

	PROFILE_EXIT('DLI');
}

/*
 * Indicate to the user that we are trying to boot.
 */
static void
do_boot_ui(void)
{
#if WITH_PAINT
	PROFILE_ENTER('bUI');
#if !PRODUCT_IBEC
	// Don't enable remapping for iBEC because it will cause a
	// "white flash" on white-faced units in restore mode.
	// <rdar://problem/14620772> boot logo color remapping support for restore mode
	paint_color_map_enable(paint_color_map_is_desired());
#endif
	paint_set_bgcolor(0, 0, 0);
	paint_set_picture(0);
	paint_set_picture_for_tag(IMAGE_TYPE_LOGO);
	paint_update_image();
	PROFILE_EXIT('bUI');
#endif
}

/*
 * Indicate to the user that we are entering recovery mode.
 */
static void
do_recoverymode_ui(void)
{
#if WITH_PAINT
	// XXX Restore team should decide how to refactor in terms
	// of changes due to two-stage iBSS bootstrap.
	paint_color_map_enable(false);
	paint_set_bgcolor(  0,   0,   0); // Default to black background
	paint_set_picture(0);
	paint_set_picture_for_tag(IMAGE_TYPE_RECMODE);
	paint_update_image();
#endif /* WITH_PAINT */

#if WITH_HW_POWER
	// Disable dark boot on recovery mode, so the user knows their device is dead.
	// We don't have a trivial way of responding to button presses in this mode.

	// Note that our recovery graphics are black background so this shouldn't be
	// overly irritating.
	power_disable_dark_boot();
#endif
}

/*
 * Explicit power-off detection (e.g. request_dfu1 (hold) button)
 */
static int
poweroff_task(void *arg)
{
	utime_t poweroff_time = 0;

	/* Require a break in the request to prevent confusion with poweron request */
	while (target_should_poweroff(false)) {
		task_sleep(10 * 1000);
	}

	/* Wait for poweroff request */
	while (1) {
		if (target_should_poweroff(false)) {
			if (poweroff_time == 0) 
				poweroff_time = system_time();
			else if (time_has_elapsed(poweroff_time, USER_BUTTON_POWEROFF_TIMEOUT)) {
				dprintf(DEBUG_CRITICAL, "Force power off.\n");
				platform_quiesce_display();

				/* Require a break in the request to prevent poweron */
				while (target_should_poweroff(false)) {
					task_sleep(10 * 1000);
				}
				platform_poweroff();
			}
		} else {
			poweroff_time = 0;
		}

		task_sleep(10 * 1000);
	}

	return 0;
}

/*
 * Idle timeout power-off.
 */
#define NO_USB_IDLEOFF_TIMEOUT		(30 * 1000 * 1000)
#define ACTIVITY_IDLEOFF_TIMEOUT	(15 * 60 * 1000 * 1000)

static int
idleoff_task(void *arg)
{
	utime_t usb_timeout = system_time();
	
	while (!time_has_elapsed(gMenuLastActivityTime, ACTIVITY_IDLEOFF_TIMEOUT)) {
		if (power_has_usb()) 
			usb_timeout = system_time();
		
		if (time_has_elapsed(usb_timeout, NO_USB_IDLEOFF_TIMEOUT)) 
			break;
		
		task_sleep(1 * 1000 * 1000);
	}
	
	dprintf(DEBUG_CRITICAL, "Idle power off.\n");
	
	platform_poweroff();
	
	return 0;
}

#if RELEASE_BUILD
/*
 * Environment variable whitelist for release builds.
 *
 * Only variables authorised by this routine may be set using setenv console command
 */
bool
env_blacklist(const char *name, bool write)
{
        int     i;

	static const char * const release_env_set_whitelist[] = {
		"auto-boot",
		"boot-args",
		"debug-uarts",
#if WITH_USB_DFU
		"filesize",
#endif
		"pwr-path",
		NULL
	};

        /* selective setenv */
        if (write) {
                for (i = 0; NULL != release_env_set_whitelist[i]; i++)
                        if (!strcmp(name, release_env_set_whitelist[i]))
                                return(false);

		/* variable is blacklisted */
                return(true);
        }

        /* allow indiscriminate getenv */
        return(false);
}
#endif

/*
 * Environment variable NVRAM whitelist.
 *
 * Only variables authorized by this routine may be read from NVRAM using env_get
 */
bool
env_blacklist_nvram(const char *name)
{
        int     i;

	static const char * const whitelist[] = {
		// Most nvram variables should only be visible on debug builds.
		// Put new variables here unless they're definitely needed in
		// factory or customer scenarios
#if DEBUG_BUILD
		"DClr_override",
		"adbe-tunable",
		"adbe-tunables",
		"adfe-tunables",
		"boot-device",
		"boot-partition",
		"boot-path",
		"boot-ramdisk",
		"boot-script",
		"bt1addr",
		"btaddr",
		"cam-use-ext-ldo",
		"core-bin-offset",
		"cpu-bin-offset",
		"debug-gg",
		"debug-soc",
		"display-color-space",
		"display-timing",
		"e75",
		"enable-upgrade-fallback",
		"eth1addr",
		"ethaddr",
		"fixed-lcm-boost",
		"force-upgrade-fail",
		"kaslr-off",
		"kaslr-slide",
		"loadaddr",
		"pinot-panel-id",
		"pintoaddr",
		"rbdaddr0",
		"soc-bin-offset",
		"summit-panel-id",
		"usb-enabled",
		"wifi1addr",
		"wifiaddr",
#endif

		// NVRAM variables needed in factory scenarios go here

#if DEVELOPMENT_BUILD || DEBUG_BUILD
		"auto-boot-usb", // probably just a debug thing, leaving here just in case
		"boot-args", // factory needs to set boot-args
		"bootdelay", // not sure if factory uses this, but shouldn't hurt
		"diags-path", // not sure if factory uses this, but shouldn't hurt
#if WITH_DALI
		"dali-pmu-debug", // needed for debugging dead units
#endif

#endif

		// NVRAM variables needed in customer scenarios go here

		"auto-boot", // needed to boot the system
		"backlight-level",
		"boot-command", // needed for OTA updates
		"com.apple.System.boot-nonce",
#if WITH_DALI
		"dali-24h-mode", // used on watches for Dali mode
#endif
		"debug-uarts",
		"device-material", // used on watches for Dali mode
		"display-rotation", // used on watches for wrist selection
		"idle-off", // used by a few CoreAutomation scripts, leaving just in case
		"is-tethered", // not sure if this is needed for stevenotes, leaving just in case
		"darkboot", // Used for <rdar://problem/19798076> Sub-TLF: Dark Boot
		"ota-breadcrumbs", // Used for appending to OTA failure tracing
#if WITH_DALI
		"utc-offset", // used on watches for Dali mode
#endif
		"pwr-path", // <rdar://problem/20811539> Disable debug power in N27a/N28a customer bundle
		NULL
	};

#if DEBUG_BUILD
	// We'll also allow "quick access" commands on debug iBoot, as they'll never
	// conflict with things we might do on release iBoot.
	if (strlen(name) == 1) {
		return false;
	}
#endif

	for (i = 0; NULL != whitelist[i]; i++)
		if (strcmp(name, whitelist[i]) == 0)
			return(false);


	/* variable is blacklisted */
	return(true);
}

void
print_boot_banner(void)
{
#if !WITH_BRIEF_BOOT_BANNER
	dprintf(DEBUG_RELEASE, "\n\n=======================================\n");
	dprintf(DEBUG_RELEASE, "::\n");
	dprintf(DEBUG_RELEASE, ":: %s\n", build_banner_string);
	dprintf(DEBUG_RELEASE, "::\n");
	dprintf(DEBUG_RELEASE, "::\tBUILD_TAG: %s\n", build_tag_string);
	dprintf(DEBUG_RELEASE, "::\n");
#if !DEBUG_BUILD
	dprintf(DEBUG_RELEASE, "::\tBUILD_STYLE: %s\n", build_style_string);
#else
	dprintf(DEBUG_RELEASE, "::\tBUILD_STYLE:\x1b[1;5;31m %s \x1b[m\n", build_style_string);
#endif
	dprintf(DEBUG_RELEASE, "::\n");
	dprintf(DEBUG_RELEASE, "::\tUSB_SERIAL_NUMBER: %s\n", platform_get_usb_serial_number_string(false));
	dprintf(DEBUG_RELEASE, "::\n");
	dprintf(DEBUG_RELEASE, "=======================================\n\n");
#else
	dprintf(DEBUG_RELEASE, "\n%s for %s\n", CONFIG_PROGNAME_STRING, CONFIG_BOARD_STRING);
	dprintf(DEBUG_RELEASE, "%s\n", build_tag_string);
	dprintf(DEBUG_RELEASE, "%s\n\n", build_style_string);
#endif
}

static volatile bool gDFUSuspend = false;
static volatile bool gDFURestart = false;

static void check_enter_dfu(void)
{
	int result;
	uint8_t enter_dfu;

	result = power_get_nvram(kPowerNVRAMiBootEnterDFUKey, &enter_dfu);
	if ((result == 0) && (enter_dfu == kPowerNVRAMiBootEnterDFURequest)) {
		/* acknowledge enter DFU trigger detected by clearing the request */
		power_set_nvram(kPowerNVRAMiBootEnterDFUKey, kPowerNVRAMiBootEnterDFUOff);

		dprintf(DEBUG_CRITICAL, "Entering DFU (NVRAM request)\n");
		main_dfu();
	}
}

#if WITH_TICKET
static bool
dfu_parse_ticket(addr_t load_address, size_t load_length, addr_t *new_load_address, size_t *new_load_length)
{
	bool			parsed = false;
	const uint8_t *	padding;
	size_t			padding_length;
	size_t			ticket_length;
	size_t			image_offset;
	unsigned 		i;

	if (ticket_set((const uint8_t *) load_address, load_length, true, &ticket_length) == 0) {
		
		padding_length = ((ticket_length % 64) != 0) ? (64 - (ticket_length % 64)) : 0;
		
		/* ensure the padding is a byte string of 0xFF */
		padding = (const uint8_t *) load_address + ticket_length;
		for (i = 0; i < padding_length; ++i) {
			if (padding[i] != 0xFF) {
				dprintf(DEBUG_INFO, "corrupt padding");
				goto exit;
			}
		}
		
		image_offset = ticket_length + padding_length;
	}
	else {
		dprintf(DEBUG_INFO, "no valid ticket found");		
		image_offset = 0;
	}

	*new_load_address = load_address + image_offset;
	*new_load_length = load_length - image_offset;

	parsed = true;
	
exit:
	return parsed;
}
#endif

static bool
dfu_console_init(void)
{
	bool autoboot = true;

#if !RELEASE_BUILD && WITH_MENU
	/* if we've fallen into DFU mode with (uart-only) console
	 * enabled, enable uarts irregardless of what is specified in
	 * (default) environment; then, print the boot banner.
	 */
	debug_enable_uarts(3);
	print_boot_banner();

	/* if the user opts to abort autoboot, start the command
	 * prompt/parser.  otherwise, return uarts to whatever
	 * is specified in (default) environment.
	 */
	if (!check_autoboot(autoboot, 3)) {
		printf("Starting command prompt\n");
#if defined(MENU_TASK_SIZE)		
		task_start(task_create("command", menu_task, NULL, MENU_TASK_SIZE));
#else
		task_start(task_create("command", menu_task, NULL, 0x1C00));
#endif
		dprintf(DEBUG_CRITICAL, "Use 'dfu' command to control normal DFU operation.\n");
	} else {
		debug_enable_uarts(env_get_uint("debug-uarts", 0));
	}
#endif

	return autoboot;
}

static addr_t
dfu_console_loadaddr(addr_t load_address)
{
#if WITH_ENV
	load_address = env_get_uint("loadaddr", load_address);
#endif
	dprintf(DEBUG_CRITICAL, "Defaulting to address %p\n", (void *)load_address);
	return load_address;
}

static bool
dfu_console_loadctrl(addr_t load_address, size_t load_length)
{
	bool restart_transfer = false;

	dprintf(DEBUG_CRITICAL, "DFU image loaded: length %zu bytes; address %p\n", load_length, (void *)load_address);
			
	if (gDFUSuspend) {
		dprintf(DEBUG_CRITICAL, "DFU suspended (use 'dfu resume' to continue)\n");
		while (gDFUSuspend) {
			task_sleep(10 * 1000);
		}
		dprintf(DEBUG_CRITICAL, "DFU resumed\n");
	}
	
	if (gDFURestart) {
		dprintf(DEBUG_CRITICAL, "DFU restarted\n");
		gDFURestart = false;
		restart_transfer = true;
	}

	return restart_transfer;
}

static int
do_dfu_cmd(int argc, struct cmd_arg *args)
{
	int result = 0;

	if (argc != 2) {
		result = -1;
	} else {
		const char* subcmd = args[1].str;
		if (!strcmp(subcmd, "resume")) {
			dprintf(DEBUG_CRITICAL, "resuming DFU\n");
			gDFUSuspend = false;
		} else if (!strcmp(subcmd, "suspend")) {
			dprintf(DEBUG_CRITICAL, "suspending DFU\n");
			gDFUSuspend = true;
		} else if (!strcmp(subcmd, "restart")) {
			dprintf(DEBUG_CRITICAL, "restarting DFU\n");
			gDFURestart = true;
		} else if (!strcmp(subcmd, "status")) {
			dprintf(DEBUG_CRITICAL, 
				"DFU status\n"
				"\tsuspend\t%sabled\n"
				"\trestart\t%sabled\n", 
				gDFUSuspend ? "en" : "dis",
				gDFURestart ? "en" : "dis");
			gDFURestart = true;
		} else {
			result = -1;
		}
	}

	if (result < 0)
		dprintf(DEBUG_CRITICAL, "Usage: dfu {suspend | resume | restart | status}\n");

	return result;
}

#if WITH_DFU_MODE
MENU_COMMAND_DEVELOPMENT(dfu, do_dfu_cmd, "control DFU operation", NULL);
#endif // WITH_DFU_MODE

#if !RELEASE_BUILD
char *g_console_buffer;
size_t g_console_buffer_size;
size_t g_console_buffer_offset;
bool g_console_buffer_wrapped;
static bool g_console_buffer_paused;

void application_putchar(int c)
{
	if (g_console_buffer != NULL && !g_console_buffer_paused && g_console_buffer_size > 1) {
		enter_critical_section();

		// Add character to the buffer
		g_console_buffer[g_console_buffer_offset] = (char)c;
		g_console_buffer_offset++;

		// Wrap 1 before the end of the buffer so that the
		// pre-wrap buffer is always null-terminated
		if (g_console_buffer_offset >= g_console_buffer_size - 1) {
			g_console_buffer_offset = 0;
			g_console_buffer_wrapped = true;
		}

		// Add null terminator to make dumping easier
		g_console_buffer[g_console_buffer_offset] = 0;

		exit_critical_section();
	}
}

static void init_console_buffer(size_t size)
{
	g_console_buffer = calloc(size, 1);
	g_console_buffer_size = size;
}

static int do_dump_console(int argc, struct cmd_arg *args)
{

	if (g_console_buffer != NULL) {
		// Stop dumping to the console buffer so that
		// we don't overwrite it with the dumped data
		bool was_paused = g_console_buffer_paused;
		g_console_buffer_paused = true;

		printf("console buffer at %p\n", g_console_buffer);
		printf("offset 0x%zx\n", g_console_buffer_offset);
		printf("size 0x%zx\n", g_console_buffer_size);
		printf("wrapped %d\n", g_console_buffer_wrapped);
		printf("\n\n-*-*-*-*-\n");

		if (g_console_buffer_wrapped) {
			printf("%s", g_console_buffer + g_console_buffer_offset + 1);
		}
		printf("%s\n-*-*-*-*-\n", g_console_buffer);

		g_console_buffer_paused = was_paused;
		return 0;
	} else {
		printf("Console buffer not initialized\n");
		return -1;
	}
}

MENU_COMMAND_DEVELOPMENT(dump_console, do_dump_console, "dump console history", NULL);

#else
void application_putchar(int c)
{
}
#endif
