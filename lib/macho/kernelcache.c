/*
 * Copyright (C) 2007-2013 Apple Inc. All rights reserved.
 * Copyright (C) 1998-2006 Apple Computer, Inc. All rights reserved.
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
#include <lib/profile.h>
#include <lib/cksum.h>
#include <lib/devicetree.h>
#include <lib/env.h>
#include <lib/image.h>
#include <lib/lzss.h>
#include <lib/macho.h>
#include <lib/random.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/memmap.h>
#include <sys.h>
#include <target.h>

#include "boot.h"
#include "macho_header.h"

#if __arm64__
#define L2_GRANULE	(PAGE_SIZE * (PAGE_SIZE / 8))
#define L2_GRANULE_MASK	(L2_GRANULE - 1)
#endif

#if !WITH_MONITOR
addr_t	gKernelEntry;	// Kernel entry point
#endif

/* From dt.c */
extern int UpdateDeviceTree(const boot_args *boot_args);

/* From lib/ramdisk/ramdisk.c */
extern addr_t		gRAMDiskAddr;
extern size_t		gRAMDiskSize;

struct compressed_kernel_header {
	uint32_t	signature;
	uint32_t	compress_type;
	uint32_t	adler32;
	uint32_t	uncompressed_size;
	uint32_t	compressed_size;
	uint32_t	reserved[11];
	uint8_t		platform_name[64];
	uint8_t		root_path[256];
	uint8_t 	data[];
};
typedef struct compressed_kernel_header compressed_kernel_header;

static int loaded_kernelcache(addr_t secure_addr, size_t secure_size, addr_t *entry, addr_t *boot_args_out);
static int valid_kernelcache(addr_t addr, size_t size);
static size_t decompress_kernelcache(addr_t addr, size_t maxsize);
static addr_t load_kernelcache_object(addr_t addr, size_t size, addr_t *boot_args_out);
static int valid_monitor(addr_t addr, size_t size, addr_t *monitor_addr, size_t *monitor_size);
static void record_kernelcache_segments(addr_t addr, addr_t phys_base);
static void record_kernelcache_segment(addr_t segCmd, addr_t phys_base, addr_t *virt_base);
static bool record_memory_range(char *rangeName, addr_t start, size_t length);
static addr_t alloc_kernel_mem(size_t size);
static void init_kernel_mem_allocator(void);
static addr_t kern_to_phys(addr_t vmaddr, boot_args *boot_args);
static int load_and_set_device_tree(void);

int 
load_kernelcache(addr_t addr, size_t len, uint32_t type, addr_t *entry, addr_t *boot_args_out)
{
	addr_t secure_addr;
	size_t secure_size;

	if (len > DEFAULT_KERNEL_SIZE) {
		printf("Kernelcache too large\n");
		return -2;
	}

	secure_addr = DEFAULT_KERNEL_ADDRESS;
	secure_size = DEFAULT_KERNEL_SIZE;

	if (image_load_memory(addr, len, &secure_addr, &secure_size, &type, 1, NULL, 0)) {
		printf("Kernelcache image not valid\n");
		return -3;
	}

	return(loaded_kernelcache(secure_addr, secure_size, entry, boot_args_out));
}

#if WITH_FS
int 
load_kernelcache_file(const char *path, uint32_t type, addr_t *entry, addr_t *boot_args_out) 
{
	addr_t secure_addr;
	size_t secure_size;

	/* 
	 * We could stat the file here to perform a size check, but the load-from-disk
	 * case is less likely to have an accidentally-oversize kernelcache.
	 */

	secure_addr = DEFAULT_KERNEL_ADDRESS;
	secure_size = DEFAULT_KERNEL_SIZE;

	PROFILE_ENTER('ILF');
	if (image_load_file(path, &secure_addr, &secure_size, &type, 1, NULL, 0)) {
		printf("Kernelcache image not valid\n");
		return -3;
	}
	PROFILE_EXIT('ILF');

	return(loaded_kernelcache(secure_addr, secure_size, entry, boot_args_out));
}
#endif /* WITH_FS */

int load_kernelcache_image(struct image_info *image, uint32_t type, addr_t *entry, addr_t *boot_args_out) 
{
	addr_t secure_addr;
	size_t secure_size;

	secure_addr = DEFAULT_KERNEL_ADDRESS;
	secure_size = DEFAULT_KERNEL_SIZE;

	if (image_load(image, &type, 1, NULL, (void **)&secure_addr, &secure_size)) {
		printf("Kernelcache image not valid\n");
		return -3;
	}

	return(loaded_kernelcache(secure_addr, secure_size, entry, boot_args_out));
}


static int 
loaded_kernelcache(addr_t secure_addr, size_t secure_size, addr_t *entry, addr_t *boot_args_out)
{
	addr_t addr;
#if WITH_MONITOR
	addr_t monitor_addr;
	size_t monitor_size;
#endif

	/* consolidate environment */
	security_consolidate_environment();

	/* Load and setup devicetree. This needs to happen here to retrieve information regarding 
	 * monitor memory 
	 */
	if (load_and_set_device_tree() != 0) {
		printf("Failed to setup devicetree \n");
		return -4;
	}

#if WITH_MONITOR
	/* Find total kernelcache file size from compressed header, and look for monitor at the end.
	 * If theres a monitor, load it now, then load kernelcache
	 */
	if (valid_monitor(secure_addr, secure_size, &monitor_addr, &monitor_size) != 0) {
		load_monitor_image(monitor_addr, monitor_size);
	}
#endif

	if (!valid_kernelcache(secure_addr, secure_size))
		return -4;

	PROFILE_ENTER('DKc');
	secure_size = decompress_kernelcache(secure_addr, DEFAULT_KERNEL_SIZE);
	if (secure_size == 0)
		return -5;
	PROFILE_EXIT('DKc');

	if (!macho_valid(secure_addr))
		return -6;

	security_protect_memory(0, 0, true);

	PROFILE_ENTER('LKO');
	addr = load_kernelcache_object(secure_addr, secure_size, boot_args_out);
	PROFILE_EXIT('LKO');
	if (addr == 0) {
		/* Re-init security and clear devicetree, ramdisk */
		security_init(true);

		return -7;
	}

	*entry = addr;

	return 0;
}

static int 
valid_kernelcache(addr_t addr, size_t size)
{
	compressed_kernel_header *kernel_header;

	kernel_header = (compressed_kernel_header *)addr;
	if (ntohl(kernel_header->signature) != 'comp') 
		return 0;

	return 1;
}

static size_t 
decompress_kernelcache(addr_t addr, size_t maxsize)
{
	compressed_kernel_header	*kernel_header;
	size_t				tmp_size, comp_size, uncomp_size;
	uint8_t				*uncomp_buffer, *comp_buffer = NULL;
	uint32_t			kernel_adler32;
	size_t				result = 0;

	kernel_header = (compressed_kernel_header *)(addr);
	if (ntohl(kernel_header->signature) != 'comp') {
		dprintf(DEBUG_CRITICAL, "unknown kernelcache signature\n");
		goto exit;
	}

	if (ntohl(kernel_header->compress_type) != 'lzss') {
		dprintf(DEBUG_CRITICAL, "unknown kernelcache compression type\n");
		goto exit;
	}

	dprintf(DEBUG_CRITICAL, "Loading kernel cache at %p\n", (void *)addr);

	// For now, ignore platform name and root path

	// Save the adler32 checksum for the kernel cache
	kernel_adler32 = ntohl(kernel_header->adler32);

	// Ensure that the uncompressed kernel cache is not too large
	uncomp_size = ntohl(kernel_header->uncompressed_size);
	if (uncomp_size > maxsize) {
		dprintf(DEBUG_CRITICAL, "Uncompressed size too large: 0x%08lx, max 0x%08lx\n", uncomp_size, maxsize);
		goto exit;
	}
	uncomp_buffer = (uint8_t *)addr;

	// Allocate buffer for the compressed kernel cache
	comp_size = ntohl(kernel_header->compressed_size);
	comp_buffer = malloc(comp_size);

	// Copy the compressed kernel cache to the new buffer
	memcpy(comp_buffer, &kernel_header->data[0], comp_size);

	tmp_size = decompress_lzss(uncomp_buffer, maxsize, comp_buffer, comp_size);
	if (tmp_size != uncomp_size) {
		dprintf(DEBUG_CRITICAL, "Size mismatch from lzss 0x%08lx, should be 0x%08lx\n", tmp_size, uncomp_size);
		goto exit;
	}

	if (kernel_adler32 != adler32(uncomp_buffer, uncomp_size)) {
		dprintf(DEBUG_CRITICAL, "Adler32 mismatch\n");
		goto exit;
	}

	dprintf(DEBUG_CRITICAL, "Uncompressed kernel cache at %p\n", uncomp_buffer);

	result = uncomp_size;

exit:
	// Zero and free the buffer for the compressed kernel cache
	if (comp_buffer != 0) {
		memset(comp_buffer, 0, comp_size);
		free(comp_buffer);
	}

	return result;
}

static int 
valid_monitor(addr_t addr, size_t size, addr_t *monitor_addr, size_t *monitor_size)
{
	compressed_kernel_header *kernel_header;
	size_t kernel_size;

	*monitor_addr = 0;
	*monitor_size = 0;

	kernel_header = (compressed_kernel_header *)addr;
	if (ntohl(kernel_header->signature) != 'comp') 
		return 0;

	kernel_size = ntohl(kernel_header->compressed_size) + sizeof(compressed_kernel_header);

	if (size > kernel_size) {
		if (!macho_valid(addr + kernel_size)) {
			return 0;
		}

		*monitor_addr = addr + kernel_size;
		*monitor_size = (size - kernel_size);

		return 1;
	}
	
	return 0;
}

static int
load_and_set_device_tree(void)
{
	/* Load device-tree here. We need to know size of certain memory regions at this point. 
	 * Example: tz1 region to load monitor
	 */
	if (dt_load() != 0) {
		dprintf(DEBUG_CRITICAL, "failed to load device tree\n");
		return -1;
	}

	return 0;
}

/* 
 * tokenize the string based on whitespace and look for
 *  each token to have the specified value/prefix
 * 
 * XXX In general, using this function is an admission
 *     that you haven't architected things correctly.
 *     boot-args should normally NOT BE USED for
 *     communication with anything other than the very
 *     early stages of the kernel startup process.
 *     For all other applications, use the DeviceTree.
 *
 * XXX This is also not terribly well implemented. The 
 *     search can be phrased as a substring search for
 *     prefixmatch ? " <arg>" : " <arg> "
 *     removing much of the manual token scanning.
 */
bool 
contains_boot_arg(const char *bootArgs, 
		  const char *arg,
		  bool prefixmatch)
{
	const char *pos = bootArgs;
	size_t tokenlen;
	size_t arglen = strlen(arg);

	while (*pos) {
		// skip leading whitespace
		while (*pos && *pos == ' ') {
			pos++;
		}

		if (*pos == '\0') {
			// got to the end of the string, this arg isn't present
			return false;
		}

		tokenlen = 0;
		while (pos[tokenlen] != '\0' && pos[tokenlen]  != ' ') {
			tokenlen++;
		}

		// the current token may end at a space or the end of the buffer,
		// so don't dereference past it

		if (tokenlen >= arglen) {
			if (prefixmatch && strncmp(pos, arg, arglen) == 0) {
				return true;
			} else if (strncmp(pos, arg, tokenlen) == 0) {
				return true;
			}
		}

		pos += tokenlen;
	}

	return false;
}

void update_display_info(boot_args *boot_args)
{
	struct display_info info;

	memset(&info, 0, sizeof(info));
	display_get_info(&info);

	boot_args->video.v_rowBytes = info.stride;
	boot_args->video.v_width = info.width;
	boot_args->video.v_height = info.height;
	boot_args->video.v_depth = info.depth;
	boot_args->video.v_baseAddr = (addr_t)info.framebuffer;
	boot_args->video.v_display = 1;
}

static size_t
get_kaslr_random(const char *env_var)
{
	size_t value;

#if DEBUG_BUILD
	// Debug builds can also specify a specific slide for debug reasons
	value = env_get_uint(env_var, (size_t)-1);
	if (value != (size_t)-1) {
		printf("got static value 0x%08lx for %s\n", value, env_var);
		return value;
	}
#endif
	random_get_bytes((void *)&value, sizeof(value));

	return value;
}

static void
get_kaslr_slides(uintptr_t kernel_phys_base, size_t kernel_size, size_t *physical_slide_out, size_t *virtual_slide_out)
{
	bool kaslr_off = false;
	size_t slide_phys = 0;
	size_t slide_virt = 0;

#if !RELEASE_BUILD
	// Non-release builds can disable ASLR for debug reasons
	kaslr_off = env_get_bool("kaslr-off", false);
#endif

#if PAGE_SIZE != 16384
	if (!kaslr_off) {
		size_t random_value = get_kaslr_random("kaslr-slide") & 0xff;
		if (random_value == 0)
			random_value = 0x100;
		slide_virt = 0x1000000 + (random_value << 21);
	} else {
		slide_virt = 0;
	}
	slide_phys = 0;
#else
#if DEFAULT_KERNEL_SIZE + L2_GRANULE > DEFAULT_LOAD_SIZE
#error "kernelcache loader needs extra space to do physical kernel slide"
#endif
	const size_t slide_granule = (1 << 21);
	const size_t slide_granule_mask = slide_granule - 1;
	const size_t slide_virt_max = 0x100 * (2*1024*1024);

	ASSERT((kernel_phys_base & L2_GRANULE_MASK) == 0);
	if (!kaslr_off) {
		// Slide the kernel virtually, keeping in the min and max range
		// defined above
		size_t random_value = get_kaslr_random("kaslr-slide");
		slide_virt = (random_value & ~slide_granule_mask) % slide_virt_max;
		if (slide_virt == 0) {
			slide_virt = slide_virt_max;
		}

		// The virtual and physical addresses need to have the same offset
		// within an L2 block (2MiB with 4k pages, 32MiB with 16k).
		// By contract the kernel's virtual base address is aligned to an L2 block.
		// We asserted above that the physical base address is aligned to an L2 block.
		// Under those conditions, guaranteeing the alignment is trivial.
		slide_phys = (slide_virt & L2_GRANULE_MASK);
	} else {
		slide_virt = 0;
		slide_phys = 0;
	}
#endif

	dprintf(DEBUG_INFO, "KASLR virtual slide:  0x%08zx\n", slide_virt);
	dprintf(DEBUG_INFO, "KASLR physical slide: 0x%08zx\n", slide_phys);

	*physical_slide_out = slide_phys;
	*virtual_slide_out = slide_virt;
}


static addr_t 
load_kernelcache_object(addr_t addr, size_t size, addr_t *boot_args_out)
{
	static boot_args boot_args;
	const char *bootArgsStr;
	addr_t kernel_region_phys;
	size_t kernel_region_size;
	addr_t kcache_base_kern;
	addr_t entry_point_kern;
	addr_t entry_point_phys;
	addr_t kcache_end_kern;
	addr_t ramdisk_kern;
	addr_t ramdisk_phys;
	addr_t boot_args_kern;
	addr_t boot_args_phys;
	addr_t devicetree_kern;
	addr_t devicetree_phys;
	size_t devicetree_size;
	addr_t last_alloc_addr_kern;
	addr_t last_alloc_addr_phys;
	size_t slide_phys;
	size_t slide_kern;

	// Reset the kernel memory allocator in case this is our second time through
	init_kernel_mem_allocator();
	// Ditto with boot args
	memset(&boot_args, 0, sizeof(boot_args));

	boot_args.revision = kBootArgsRevision;
	boot_args.version  = kBootArgsVersion1 + platform_get_security_epoch();

	// Get the region of memory that will be managed by the kernel.
	// This is typically above the SEP region (if any) or
	// at the base of memory if no SEP
	kernel_region_phys = platform_get_memory_region_base(kMemoryRegion_Kernel);
	kernel_region_size = platform_get_memory_region_size(kMemoryRegion_Kernel);

	get_kaslr_slides(kernel_region_phys, size, &slide_phys, &slide_kern);
	RELEASE_ASSERT(slide_phys + size < kernel_region_size);

	boot_args.physBase = kernel_region_phys;
	boot_args.memSize  = kernel_region_size;

	// Scan the kernelcache and record the location of segments in the
	// /chosen/memory-map node in the devicetree.
	record_kernelcache_segments(addr, kernel_region_phys + slide_phys);

	// Load the kernelcache object
	macho_load(addr, size, kernel_region_phys + slide_phys, &kcache_base_kern, &kcache_end_kern, &entry_point_kern, slide_kern);
	boot_args.virtBase = kcache_base_kern - slide_phys;
	// Record the space used by the kernelcache
	alloc_kernel_mem(kcache_end_kern);

#if RELEASE_BUILD
	// Force boot-args for RELEASE builds
	bootArgsStr = (gRAMDiskAddr == 0) ? "" : "rd=md0 nand-enable-reformat=1 -progress";
#else
	bootArgsStr = env_get("boot-args");
	if (bootArgsStr == 0) 
		bootArgsStr = "";
#endif

	if (target_is_tethered()) {
		snprintf(boot_args.commandLine, BOOT_LINE_LENGTH, "%s force-usb-power=1 ", bootArgsStr);
	} else {
		snprintf(boot_args.commandLine, BOOT_LINE_LENGTH, "%s ", bootArgsStr);
	}

	if (gRAMDiskAddr != 0) {
		uint32_t backlight_level;
		if (target_get_property(TARGET_PROPERTY_RESTORE_BACKLIGHT_LEVEL, &backlight_level, sizeof(backlight_level), NULL)) {
			snprintf(boot_args.commandLine, BOOT_LINE_LENGTH, "%s backlight-level=%d ", bootArgsStr, backlight_level);
		}
	}

	bootArgsStr = boot_args.commandLine;

#if WITH_HW_POWER
	boot_args.boot_flags |= (power_is_dark_boot()) ? kBootFlagDarkBoot : 0;

	dprintf(DEBUG_INFO, "Passing dark boot state: %s\n", power_is_dark_boot() ? "dark" : "not dark");
#endif

	update_display_info(&boot_args);

	if (contains_boot_arg(bootArgsStr, "-s", false)
	    || contains_boot_arg(bootArgsStr, "-v", false)) {
		boot_args.video.v_display = 0;
	}

	if (contains_boot_arg(bootArgsStr, "debug=", true)) {
		security_enable_kdp();
	}

#if WITH_HW_USB
	if (contains_boot_arg(bootArgsStr, "force-usb-power=1", false)) {
		extern bool force_usb_power;
		force_usb_power = true;
	}
#endif

	// If we have a ramdisk, allocate kernel memory for it and copy
	// it from its load address to the kernel region
	if (gRAMDiskAddr != 0) {
		ramdisk_kern = alloc_kernel_mem(gRAMDiskSize);
		ramdisk_phys = kern_to_phys(ramdisk_kern, &boot_args);
		memcpy((void *)ramdisk_phys, (void *)gRAMDiskAddr, gRAMDiskSize);
		record_memory_range("RAMDisk", ramdisk_phys, gRAMDiskSize);
	}

	// Allocate a kernel memory for the boot args. The boot args
	// will get copied to this buffer at the end of this function.
	boot_args_kern = alloc_kernel_mem(PAGE_SIZE);
	boot_args_phys = kern_to_phys(boot_args_kern, &boot_args);
	record_memory_range("BootArgs", boot_args_phys, PAGE_SIZE);

	PROFILE_ENTER('UDT');
	int ret = UpdateDeviceTree(&boot_args);
	PROFILE_EXIT('UDT');
	if (ret < 0){
		dprintf(DEBUG_CRITICAL, "UpdateDeviceTree() failed with %d\n", ret);
		return (addr_t)(NULL);
	}

	// We need to put the size of the device tree into the device tree, but
	// doing so could change its size. So, start by recording dummy information
	// and then calculate the final device tree size
	record_memory_range("DeviceTree", (addr_t)-1, (size_t)-1);

	devicetree_size = dt_get_size();

	// Allocate kernel memory for the device tree
	devicetree_kern = alloc_kernel_mem(devicetree_size);
	devicetree_phys = kern_to_phys(devicetree_kern, &boot_args);
	// overwrite the dummy entry made previously
	record_memory_range("DeviceTree", devicetree_phys, devicetree_size);

	dt_serialize(NULL, (void *)devicetree_phys, devicetree_size);

	// After this point, the device tree must not be modified
	dt_seal();

	boot_args.deviceTreeP = (void *)devicetree_kern;
	boot_args.deviceTreeLength = devicetree_size;

	entry_point_phys = kern_to_phys(entry_point_kern, &boot_args);

	last_alloc_addr_kern = (alloc_kernel_mem(0) + 0x3000) & (size_t)~0x3fff;
	last_alloc_addr_phys = kern_to_phys(last_alloc_addr_kern, &boot_args);

	boot_args.topOfKernelData = last_alloc_addr_phys;

	dprintf(DEBUG_CRITICAL, "boot_args.commandLine = [%s]\n", boot_args.commandLine);
	dprintf(DEBUG_INFO, "boot_args.physBase = [%p], boot_args.virtBase = [%p]\n", (void *)boot_args.physBase, (void *)boot_args.virtBase);
	dprintf(DEBUG_INFO, "boot_args.deviceTreeLength = 0x%x, boot_args.deviceTreeP = %p, devicetree_phys = %p\n", 
		boot_args.deviceTreeLength, (void *)boot_args.deviceTreeP, (void *)devicetree_phys);
	dprintf(DEBUG_INFO, "kernel entry point: %p, boot args: %p\n", (void *)entry_point_phys, (void *)boot_args_phys);

	// copy our local boot arg buffer into the kernel memory region for boot args
	memset((void *)boot_args_phys, 0, PAGE_SIZE);
	memcpy((void *)boot_args_phys, &boot_args, sizeof(boot_args));

	security_create_sleep_token(SLEEP_TOKEN_BUFFER_BASE + kSleepTokeniBootOffset);

#if !RELEASE_BUILD && WITH_CONSISTENT_DEBUG
	// Astris needs to be able to find the kernel to do coredumps
	dbg_record_header_t kernel_region_hdr;
	kernel_region_hdr.record_id = kDbgIdXNUKernelRegion;
	kernel_region_hdr.physaddr = kernel_region_phys + slide_phys;
	kernel_region_hdr.length = kcache_end_kern - kcache_base_kern;
	consistent_debug_register_header(kernel_region_hdr);
#endif

#if WITH_MONITOR
	extern addr_t gMonitorArgs;
	extern addr_t gMonitorEntry;
	monitor_boot_args *mba;

	/*
	 * Patch the monitor boot-args with the location of the
	 * now-constructed kernel boot-args.
	 * Ensure that we pass back the monitor entrypoint, not
	 * the kernel's.
	 */
	if (0 != gMonitorArgs) {
		mba = (struct monitor_boot_args *)gMonitorArgs;
		mba->kernArgs = boot_args_phys;
		mba->kernEntry = entry_point_phys;
		mba->kernPhysBase = kernel_region_phys;
		mba->kernPhysSlide = slide_phys;
		mba->kernVirtSlide = slide_kern;

		dprintf(DEBUG_INFO, "monitor entry point: %p, boot args: %p\n", (void *)gMonitorEntry, (void *)gMonitorArgs);

		*boot_args_out = gMonitorArgs;
		return gMonitorEntry;
	}
#else
	gKernelEntry = entry_point_phys;
#endif

	*boot_args_out = boot_args_phys;
	return entry_point_phys;
}

// Private Functions

/*
 * Scan the mach-o kernelcache and record the location
 * of interesting segments in the chosen/memory-map node
 * of the DeviceTree.
 */
static void
record_kernelcache_segments(addr_t addr, addr_t phys_base)
{
	mach_header_t	*mH;
	addr_t		cmdBase;
	addr_t		virt_base;
	uint32_t	index;

	mH = (mach_header_t *)addr;
	cmdBase = (addr_t)(mH + 1);
	virt_base = 0;

	for (index = 0; index < mH->ncmds; index++) {
#if defined(__LP64__)
		if ((((struct load_command *)cmdBase)->cmd) == LC_SEGMENT_64)
#else
		if ((((struct load_command *)cmdBase)->cmd) == LC_SEGMENT)
#endif
			record_kernelcache_segment(cmdBase, phys_base, &virt_base);
		cmdBase += (((struct load_command *)cmdBase)->cmdsize);
	}
}

static void
record_kernelcache_segment(addr_t cmdBase, addr_t phys_base, addr_t *virt_base)
{
	segment_command_t 	*segCmd;
	char			rangeName[kPropNameLength];
	addr_t  	 	pmaddr;
	addr_t			vmaddr;
	size_t			vmsize;

	segCmd = (segment_command_t *)cmdBase;

	vmaddr = segCmd->vmaddr;
	vmsize = segCmd->vmsize;

	// if virt_base has not been set let vmaddr select a segment (1MB)
	if (*virt_base == 0) {
		*virt_base = vmaddr & (addr_t)~0xfffff;
	}

	pmaddr = phys_base + vmaddr - *virt_base;

	if (strcmp(segCmd->segname, "__PAGEZERO")) {
		// Add the Segment to the memory-map.
		snprintf(rangeName, kPropNameLength, "Kernel-%s", segCmd->segname);
		record_memory_range(rangeName, pmaddr, vmsize);
	}
}

/* used in the chosen/memory-map node */
typedef struct MemoryMapFileInfo {
	addr_t paddr;
	size_t length;
} MemoryMapFileInfo;

static bool
record_memory_range(char *rangeName, addr_t start, size_t length)
{
	dt_node_t		*memory_map;
	MemoryMapFileInfo 	prop_data;

	/*
	 * Find the /chosen/memory-map node.
	 */
	if (!dt_find_node(0, "chosen/memory-map", &memory_map)) {
	        dprintf(DEBUG_INFO, "no /chosen/memory-map in DeviceTree\n");
	        return false;
	}

	prop_data.paddr = start;
	prop_data.length = length;

	dt_set_prop(memory_map, rangeName, &prop_data, sizeof(prop_data));

	return true;
}

static addr_t	last_kernel_addr;

static void
init_kernel_mem_allocator(void)
{
	last_kernel_addr = 0;
}

static addr_t
alloc_kernel_mem(size_t size)
{
	addr_t 		addr;
	
	addr = last_kernel_addr;

	last_kernel_addr += (size + PAGE_SIZE - 1) & ~(addr_t)(PAGE_SIZE - 1);

	return addr;
}

/*
 * Convert a kernel-virtual address to a physical address.
 */
static addr_t
kern_to_phys(addr_t vmaddr, boot_args *boot_args)
{
	if (boot_args->virtBase == 0)
		panic("kern_to_phys without setting kernel virtual base");

	return(vmaddr - boot_args->virtBase + boot_args->physBase);
}
