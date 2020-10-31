/*
 * Copyright (C) 2007-2015 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/power.h>
#include <stdio.h>
#include <string.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <lib/env.h>
#include <lib/heap.h>
#include <platform.h>
#include <platform/memmap.h>
#include <sys/task.h>

#if defined(WITH_MENU) && WITH_MENU

static void
memdump_usage(void)
{

	dprintf(DEBUG_INFO, "md [-64] <address> <length>\n");
}

/* "md -64" "md", "mdh", "mdb" */
static int
do_memdump(int argc, struct cmd_arg *args)
{
	uintptr_t address;
	size_t count;
	int width;
	size_t i;
	int index_mod = 16;

	/* default dump values */
	static uintptr_t last_address = DEFAULT_LOAD_ADDRESS;
	static size_t last_count = 0x100;

	if (!strcmp(args[0].str, "md")) {
		width = 32;
		if (argc > 1) {
			if (!strcmp(args[1].str, "-help")) {
				memdump_usage();
				return 0;
			} else if (!strcmp(args[1].str, "-64")) {
				width = 64;
				index_mod = 32;
			}
		}
	} else if (!strcmp(args[0].str, "mdh")) {
		width = 16;
	} else {
		width = 8;
	}

	address = last_address;
	count = last_count;

	if (width != 64) {
		if (argc >= 2) {
			if (!isdigit(args[1].str[0])) {
				memdump_usage();
				return -1;
			}
			address = args[1].u;
		}
		if (argc >= 3) {
			if (!isdigit(args[2].str[0])) {
				memdump_usage();
				return -1;
			}
			count = args[2].u;
		}
	}
	else {
		if (argc >= 3)  {
			if (!isdigit(args[2].str[0])) {
				memdump_usage();
				return -1;
			}
			address = args[2].u;
		}
		if (argc >= 4) {
			if (!isdigit(args[3].str[0])) {
				memdump_usage();
				return -1;
			}
			count = args[3].u;
		}
	}
	
//	printf("dumping memory at %#lx, len 0x%x, width %d\n", address, count, width);

	if (!security_allow_memory((void *)address, count * width / 8)) {
		printf("Permission Denied\n");
		return -1;
	}

	i = 0;
	while (i < count) {
		if ((i % index_mod) == 0) {
			if (i != 0)
				puts("\n");
			printf("%p: ", (uint8_t *)address + i);
		}

		switch (width) {
			case 64:
				printf("%016llx ", *(uint64_t *)(address + i));
				i += 8;
				break;
			case 32:
				printf("%08x ", *(uint32_t *)(address + i));
				if (4 == (i & 0xf))
				  printf(" ");
				i += 4;
				break;
			case 16:
				printf("%04x ", *(uint16_t *)(address + i));
				if (6 == (i & 0xf))
				  printf(" ");
				i += 2;
				break;
			case 8:
				printf("%02x ", *(uint8_t *)(address + i));
				if (7 == (i & 0xf))
				  printf(" ");
				i += 1;
				break;
		}
	}
	puts("\n");

	/* save the values so we can continue next time */
	last_count = count;
	last_address = address + count;

	return 0;
}

MENU_COMMAND_DEBUG(md, do_memdump, "memory display - 32bit or 64bit", NULL);
MENU_COMMAND_DEBUG(mdh, do_memdump, "memory display - 16bit", NULL);
MENU_COMMAND_DEBUG(mdb, do_memdump, "memory display - 8bit", NULL);

/* "mw -64" "mw", "mwh", "mwb", "mws" */
static int
do_memwrite(int argc, struct cmd_arg *args)
{
	static uintptr_t last_address = DEFAULT_LOAD_ADDRESS;
	uintptr_t address;
	size_t length;
	uint64_t data;
	const char *buffer;
	int width;

	if ((argc <= 1) || (!strcmp(args[1].str, "-help")) || (argc > 4))
		goto print_usage;

	switch (argc) {
	case 2:
		address = last_address;
		data = args[1].u;
		buffer = args[1].str;
		break;

	case 3:
		address = args[1].u;
		last_address = address;
		data = args[2].u;
		buffer = args[2].str;
		break;

	case 4:
		if (strcmp(args[1].str, "-64") != 0)
			goto print_usage;
		address = args[2].u;
		last_address = address;
		data = args[3].u;
		buffer = args[3].str;
		break;
	}

	if (!strcmp(args[0].str, "mw")) {
		width = 32;
		length = 4;
		if ((argc > 1) && (!strcmp(args[1].str, "-64"))) {
			width = 64;
			length = 8;
		}
	} else if (!strcmp(args[0].str, "mwh")) {
		width = 16;
		length = 2;
	} else if (!strcmp(args[0].str, "mwb")) {
		width = 8;
		length = 1;
	} else {
		width = 255;
		length = strlen(buffer) + 1;
	}

//	printf("writing memory at %#lx, data %#lx\n", address, data);

	if (!security_allow_memory((void *)address, length)) {
		printf("Permission Denied\n");
		return -1;
	}

	switch (width) {
		case 64:
			*(uint64_t *)address = (uint64_t)data;
			break;
		case 32:
			*(uint32_t *)address = (uint32_t)data;
			break;
		case 16:
			*(uint16_t *)address = (uint16_t)data;
			break;
		case 8:
			*(uint8_t *)address = (uint8_t)data;
			break;

		case 255:
			strlcpy((char *)address, buffer, length);
			break;
	}

	return 0;

print_usage:
	if (strcmp(args[0].str, "mw") != 0)
		printf("%s [<address>] <data>\n", args[0].str);
	else 
		printf("mw [-64] [<address>] <data>\n");
	return -1;
}

MENU_COMMAND_DEBUG(mw, do_memwrite, "memory write - 32bit or 64bit", NULL);
MENU_COMMAND_DEBUG(mwh, do_memwrite, "memory write - 16bit", NULL);
MENU_COMMAND_DEBUG(mwb, do_memwrite, "memory write - 8bit", NULL);
MENU_COMMAND_DEBUG(mws, do_memwrite, "memory write - string", NULL);


// Go requires the type to be for iBEC, CFE or RBM
static const u_int32_t go_restricted_types[] = { IMAGE_TYPE_IBEC,
						 IMAGE_TYPE_CFE_LOADER,
						 IMAGE_TYPE_RBM,
						 IMAGE_TYPE_PHLEET,
						 IMAGE_TYPE_PE_RTOS,
						 IMAGE_TYPE_HAMMER };

static int
do_go_target(int argc, struct cmd_arg *args, enum boot_target target, addr_t addr, addr_t secure_addr, size_t secure_len)
{
	const u_int32_t *types = go_restricted_types;
	u_int32_t count = sizeof(go_restricted_types) / sizeof(go_restricted_types[0]);
	u_int32_t actual;
	u_int32_t options;

#if !RELEASE_BUILD
	if (argc >= 2 && ((argc > 1) && !strcmp("help", args[1].str))) {
		printf("%s [<address>]\n", args[0].str);
		return -1;
	}

	addr = env_get_uint("loadaddr", addr);
	if (argc >= 2)
		addr = args[1].u;
#endif

	if (!security_allow_memory((void *)addr, secure_len)) {
		printf("Permission Denied\n");
		return -1;
	}

	/* execution of 'go' command is treated as starting of a new boot-chain, so tells library to enforce policies it cares about. */
	/* image epoch must be greater than or equal to the current stage's epoch */
	options = IMAGE_OPTION_GREATER_EPOCH | IMAGE_OPTION_NEW_TRUST_CHAIN;

#if DEBUG_BUILD
	// Allow any type in DEBUG builds
	types = NULL;
	count = 0;
#endif

	if (image_load_memory(addr, secure_len, &secure_addr, &secure_len, types, count, &actual, options)) {
		printf("Memory image not valid\n");
		return -1;
	}

	dprintf(DEBUG_CRITICAL, "actual = %x\n", actual);

	/* consolidate environment */
	security_consolidate_environment();

	printf("jumping into image at %p\n", (void *)secure_addr);

	prepare_and_jump(target, (void *)secure_addr, NULL); // XXX don't actually know what we're jumping into

	return 0;
}

static int
do_go(int argc, struct cmd_arg *args)
{
	return do_go_target(argc, args, BOOT_UNKNOWN, DEFAULT_LOAD_ADDRESS, DEFAULT_KERNEL_ADDRESS, DEFAULT_KERNEL_SIZE);
}

static int
do_go_dfu(int argc, struct cmd_arg *args)
{
	return do_go_target(argc, args, BOOT_UNKNOWN, INSECURE_MEMORY_BASE, INSECURE_MEMORY_BASE, INSECURE_MEMORY_SIZE);
}

#if WITH_RECOVERY_MODE
MENU_COMMAND(go, do_go, "jump directly to address", NULL);
#elif WITH_DFU_MODE
MENU_COMMAND(go, do_go_dfu, "jump directly to address", NULL);
#endif

static int
do_script(int argc, struct cmd_arg *args)
{
	int length = 0;

	if ((argc > 2) || ((argc > 1) && !strcmp("help", args[1].str))) {
		printf("%s [<address>]\n", args[0].str);
		return -1;
	}

	const char *script = (const char *)env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);

	if (argc > 1)
		script = (const char *)args[1].u;

	while ((script[length] != 0x04) && (script[length] != 0)) length++;

	if (!security_allow_memory(script, length)) {
		printf("Permission Denied\n");
		return -1;
	}

	return debug_run_script(script);
}

MENU_COMMAND_DEVELOPMENT(script, do_script, "run script at specific address", NULL);

#if !WITH_SIDP
int do_secrom(int argc, struct cmd_arg *args)
{
	printf("Enabling remap and jumping...\n");
#if defined(SECUREROM_LOAD_ADDRESS)
	prepare_and_jump(BOOT_SECUREROM, (void*)SECUREROM_LOAD_ADDRESS, NULL);
#else	
	prepare_and_jump(BOOT_SECUREROM, NULL, NULL);
#endif
}

MENU_COMMAND_DEBUG(secrom, do_secrom, "execute previously loaded SecureROM image", NULL);
#endif

#if WITH_ENV
static int
do_run(int argc, struct cmd_arg *args)
{
	if (argc < 2) {
		printf("%s <variable>\n", args[0].str);
		return -1;
	}

	const char *env = env_get(args[1].str);
	if (!env) {
		printf("could not find variable\n");
		return -1;
	}

	return debug_run_script(env);
}

MENU_COMMAND_DEVELOPMENT(run, do_run, "use contents of environment var as script", NULL);
#endif

int do_suspend(int argc, struct cmd_arg *args)
{
#if WITH_HW_POWER
	power_suspend();
#endif

	return 0;
}

MENU_COMMAND_DEBUG(suspend, do_suspend, "enter suspend to RAM - one way trip for now", NULL);

int do_bztest(int argc, struct cmd_arg *args)
{
	utime_t start, stop, delta;
	u_int64_t rate;

	dprintf(DEBUG_CRITICAL, "Zeroing memory from 0x%llx to 0x%llx\n",
		INSECURE_MEMORY_BASE, INSECURE_MEMORY_BASE + INSECURE_MEMORY_SIZE - 1);

	start = system_time();

	bzero((void *)INSECURE_MEMORY_BASE, INSECURE_MEMORY_SIZE);

	stop = system_time();

	delta = stop - start;

	rate = ((INSECURE_MEMORY_SIZE >> 20) * 1000000ULL) / delta;

	dprintf(DEBUG_CRITICAL, "0x%llx bytes in %lldms at %lldMB/s\n",
		INSECURE_MEMORY_SIZE, delta / 1000, rate);

	return 0;
}

#if 0
// Turned off to save space, enable as needed
MENU_COMMAND_DEBUG(bztest, do_bztest, "bzero test - fills insecure memory area with zeros", NULL);

int do_ssp_test(int argc, struct cmd_arg *args)
{
	char	tmp[8];

	strlcpy(tmp, "this string should not fit", 32);
	printf("stack protector\n");

	return(0);
}
#endif

// Turned off to save space, enable as needed
// MENU_COMMAND_DEBUG(ssp_test, do_ssp_test, "test the stack overflow protector", NULL);

#if WITH_VFP
#include <arch.h>

void
fp_test(int tid)
{
	int	count;
	float	i, j;

	i = 1.1;
	j = 3.7 + (11.5 * tid);

	for (count = 0; ; count++) {
		task_sleep(100000);
		j *= i;
		printf("FP%d:%d: %d\n", tid, count, (int)j);
	}
}

int
fp_test_thread(void *arg)
{
	int	tid = (int)arg;

	printf("FP%d: task starting\n", tid);

#if !WITH_VFP_ALWAYS_ON
	/* enable FP for this task */
	arch_task_fp_enable(true);
#endif // !WITH_VFP_ALWAYS_ON

	printf("FP%d: fp enabled\n", tid);

	fp_test(tid);

	return(0);
}

int do_fp_test(int argc, struct cmd_arg *args)
{

	/* FP is enabled during CPU init */
	printf("FP on, yielding...");
	task_yield();
	printf("ok\n");

	task_start(task_create("fp1", fp_test_thread, (void *)1, 0x1000));
	printf("fp1 started, waiting...\n");
	task_sleep(2000000);
	task_start(task_create("fp2", fp_test_thread, (void *)2, 0x1000));
	printf("fp2 started\n");

	return(0);
}

MENU_COMMAND_DEBUG(fp_test, do_fp_test, "test floatingpoint support", NULL);
#endif /* WITH_VFP */


static int do_panic(int argc, struct cmd_arg *args)
{
	panic("command prompt");
}
MENU_COMMAND_DEBUG(panic, do_panic, "Force a panic", NULL);

static int do_double_panic(int argc, struct cmd_arg *args)
{
	// Set gPanicStr so panic() thinks it has recursed.
	gPanicStr = "Forced double panic";
	panic("command prompt");
}
MENU_COMMAND_DEBUG(double_panic, do_double_panic, "Force a double panic", NULL);

static int do_sleep(int argc, struct cmd_arg *args)
{
    if (argc != 2) {
	printf("%s <delay>\n", args[0].str);
	return(-1);
    }

    if (!strcmp(args[0].str, "msleep")) {
	task_sleep(1000 * args[1].u);
    } else {
	task_sleep(1000 * 1000 * args[1].u);
    }

    return 0;
}
MENU_COMMAND_DEBUG(sleep, do_sleep, "Pause execution (seconds)", NULL);
MENU_COMMAND_DEBUG(msleep, do_sleep, "Pause execution (milliseconds)", NULL);

static int do_dram_info(int argc, struct cmd_arg *args)
{
#if WITH_HW_DCS == 1	
	uint8_t rev_id, rev_id2;
	platform_get_memory_rev_ids(&rev_id, &rev_id2);
	dprintf(DEBUG_INFO, "vendor: %s, rev_id=%d, rev_id2=%d, size: %llu MiB\n", platform_get_memory_manufacturer_string(), rev_id, rev_id2, (platform_get_memory_size() / (1024 * 1024)));
#else	
	dprintf(DEBUG_INFO, "vendor: %s, size: %llu MiB\n", platform_get_memory_manufacturer_string(), (platform_get_memory_size() / (1024 * 1024)));
#endif
	return 0;
}
MENU_COMMAND_DEBUG(dram_info, do_dram_info, "Prints DRAM vendor and size", NULL);

#if WITH_HW_POWER
// borrowed from AppleARMRTC::convertSecondsToDateTime, with fixed leap year rule from Libc tzfile.c

// Tables for accumulated days in year by month, latter used for leap years
static const int	 daysbymonth[] =
						{ 0,	31, 59, 90, 120,
						  		151, 181, 212, 243,
							  	273, 304, 334, 365 };
static const int	 lydaysbymonth[] =
						{ 0,	31, 60, 91, 121,
						  		152, 182, 213, 244,
							  	274, 305, 335, 366 };

#define daysinyear(y) \
			((((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0)) ? 365 : 366)

struct date_time_t {
	uint8_t		seconds;    /* 0-59 */
	uint8_t		minutes;    /* 0-59 */
	uint8_t		hours;      /* 0-23 */
	uint8_t		dayWeek;    /* 0-6 */
	uint8_t		dayMonth;   /* 1-31 */
	uint8_t		month;      /* 1-12 */
	uint16_t	year;
};

static void convert_seconds_to_date_time(long secs, struct date_time_t *dt )
{
	const int *		dbm = daysbymonth;
	long			n;
	int				x, y;

	// Calculate seconds, minutes and hours

	n = secs % (24 * 3600);
	dt->seconds = n % 60;
	n /= 60;
	dt->minutes = n % 60;
	dt->hours = n / 60;

	// Calculate day of week

	n = secs / (24 * 3600);
	dt->dayWeek = (n + 4) % 7;

	// Calculate year

	for (y = 1970; n >= (x = daysinyear(y)); y++)
		n -= x;
	dt->year = y;

	// Change table if year is a leap year

	if (x == 366)
		dbm = lydaysbymonth;

	// Adjust remaining days value to start at 1

	n += 1;

	// Calculate month

	for (x = 1; n > dbm[x]; x++)
		continue;
	dt->month = x;

	// Calculate day of month

	dt->dayMonth = n - dbm[x - 1];
}

static int do_date(int argc, struct cmd_arg *args)
{
    utime_t usecs = calendar_time();
    utime_t secs = usecs / 1000000;

    if (argc > 1 && !strcmp(args[1].str, "-r")) {
	printf("%llu.%02llu\n", secs, (usecs / 10000) % 100);
    } else {
	struct date_time_t dt;
	convert_seconds_to_date_time(secs, &dt);
	
	static const char * const days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	static const char * const months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	printf("%s %s %02u %02u:%02u:%02u.%02llu UTC %04u\n", days[dt.dayWeek], months[dt.month-1], dt.dayMonth, dt.hours, dt.minutes, dt.seconds, (usecs / 10000) % 100, dt.year);
    }

    return 0;
}
MENU_COMMAND_DEVELOPMENT(date, do_date, "display or set date and time", NULL);
#endif

#endif

