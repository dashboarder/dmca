/*
 *  Copyright (C) 2009, 2013  Apple, Inc. All rights reserved.
 *  
 *  This document is the property of Apple Inc.
 *  It is considered confidential and proprietary.
 *
 *  This document may not be reproduced or transmitted in any form,
 *  in whole or in part, without the express written permission of
 *  Apple Inc.
 */

#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "Buffer.h"
#include "DeviceTreePatcher.h"
#include "Kernelcache.h"
#include "LoadMachO.h"

#include <non_posix_types.h>
#include <lib/image.h>

#define DEFAULT_SDRAM_BASE	0x00000000
#define DEFAULT_SDRAM_LEN	(256 * 1024 * 1024)
#define TARGET_MMU_BASE_ALIGN	16384

#define ARM_B_POS_0x40		0xea00000e	// b pc+0x40
#define ARM_LDR_R0_PC		0xe59f0000	// ldr r0, [pc] (here+8)
#define ARM_LDR_PC_PC		0xe59ff000	// ldr pc, [pc] (here+8)

#define ARM64_BOARD_SECURITY_EPOCH 0

// An arbitrarily sized framebuffer.
#define FRAMEBUFFER_WIDTH	640
#define FRAMEBUFFER_HEIGHT	960
#define FRAMEBUFFER_DEPTH_BITS	32
#define FRAMEBUFFER_STRIDE_BYTES \
		(FRAMEBUFFER_WIDTH * FRAMEBUFFER_DEPTH_BITS / 8)
#define FRAMEBUFFER_SIZE_BYTES \
		(FRAMEBUFFER_STRIDE_BYTES * FRAMEBUFFER_HEIGHT * 3)

#define PANIC_SIZE		16384

#define PACKED			__attribute__((packed))

#define USE_L4

#ifdef USE_L4
#define MONITOR_SIZE		(0x00800000)
#endif

/* Just to make sure we get the alignment right */
typedef uint64_t arm64_ptr_t __attribute__((aligned(8)));
typedef uint64_t arm64_uint64_t __attribute__((aligned(8)));

// lib/macho/boot.h
// This isn't taken verbatim: changed host-varying types to exact width,
// as they would otherwise not work on 64 bit hosts.

/*
 * Video information.. 
 */

#define BOOT_LINE_LENGTH        256

struct Boot_Video {
  uint32_t v_baseAddr;	/* Base address of video memory */
  uint32_t v_display;	/* Display Code (if Applicable */
  uint32_t v_rowBytes;	/* Number of bytes per pixel row */
  uint32_t v_width;	/* Width */
  uint32_t v_height;	/* Height */
  uint32_t v_depth;	/* Pixel Depth */
} PACKED;

struct Boot_Video_64 {
  arm64_ptr_t v_baseAddr;/* Base address of video memory */
  arm64_ptr_t v_display;	/* Display Code (if Applicable */
  arm64_uint64_t v_rowBytes;	/* Number of bytes per pixel row */
  arm64_uint64_t v_width;	/* Width */
  arm64_uint64_t v_height;	/* Height */
  arm64_uint64_t v_depth;	/* Pixel Depth */
} PACKED;

/* Boot argument structure - passed into Mach kernel at boot time.
 */
#define kBootArgsRevision		1
#define kBootArgsVersion1		1

struct boot_args {
  uint16_t revision;			/* Revision of boot_args structure */
  uint16_t version;			/* Version of boot_args structure */
  uint32_t virtBase;			/* Virtual base of memory */
  uint32_t physBase;			/* Physical base of memory */
  uint32_t memSize;			/* Size of memory */
  uint32_t topOfKernelData;		/* Highest physical address used in kernel data area */
  Boot_Video video;			/* Video Information */
  uint32_t machineType;			/* Machine Type */
  uint32_t deviceTreeAddr;		/* Base of flattened device tree */
  uint32_t deviceTreeLength;		/* Length of flattened tree */
  char commandLine[BOOT_LINE_LENGTH];	/* Passed in command line */
} PACKED;

typedef struct boot_args_64 {
  uint16_t      revision;         /* Revision of boot_args structure */
  uint16_t      version;          /* Version of boot_args structure */
  arm64_ptr_t   virtBase;         /* Virtual base of memory */
  arm64_ptr_t   physBase;         /* Physical base of memory */
  arm64_uint64_t memSize;         /* Size of memory */
  arm64_ptr_t   topOfKernelData;  /* Highest physical address used in kernel data area */
  Boot_Video_64 video;           /* Video Information */
  uint32_t      machineType;      /* Machine Type */
  arm64_ptr_t   deviceTreeAddr;   /* Base of flattened device tree */
  uint32_t      deviceTreeLength; /* Length of flattened tree */
  char          commandLine[BOOT_LINE_LENGTH];	/* Passed in command line */
} boot_args_64;

#ifdef USE_L4
typedef struct monitor_boot_args {
	uint64_t	version;	/* structure version - this is version 1 */
	uint64_t	virtBase;	/* virtual base of memory assigned to the monitor */
	uint64_t	physBase;	/* physical address corresponding to the virtual base */
	uint64_t	memSize;	/* size of memory assigned to the monitor */
	uint64_t	kernArgs;	/* physical address of the kernel boot_args structure */
	uint64_t	kernEntry;	/* kernel entrypoint */
} monitor_boot_args;
#endif

struct dt_patch_s {
	const char *node;
	const char *property;
	const char *value;
	bool str;
};
typedef struct dt_patch_s dt_patch_t;

struct Options {
  const char *kernelcache_file;
  const char *devicetree_file;
  const char *ramdisk_file;
  const char *output_file;
  std::string bootargs_string;
  uint64_t sdram_base;
  uint32_t sdram_len;
  uint64_t virtual_base;
  bool override_virtual_base;
  bool have_boot_partition_id;
  unsigned boot_partition_id;
  std::vector<dt_patch_t> dt_patches;
  bool page_size_16kb;
#ifdef USE_L4
  const char *l4_path;
#endif

  Options()
      : kernelcache_file(NULL),
	devicetree_file(NULL),
	ramdisk_file(NULL),
	output_file(NULL),
	bootargs_string(),
	sdram_base(DEFAULT_SDRAM_BASE),
	sdram_len(DEFAULT_SDRAM_LEN),
	virtual_base(0),
	override_virtual_base(false),
	have_boot_partition_id(false),
	boot_partition_id(),
	dt_patches(),
	page_size_16kb(false),
#ifdef USE_L4
        l4_path(NULL)
#endif
	{ }

  bool Parse(int argc, char **argv);
};

static void Usage(const char *argv0);
static uint64_t RoundToNextPage(const Options &options, uint64_t address);
static uint64_t RoundToNextMmuBase(uint64_t address);
static bool SetFrequenciesInDeviceTree(DeviceTreePatcher *device_tree_patcher);

int main(int argc, char *argv[]) {
  Options options;
  if (!options.Parse(argc, argv)) {
    Usage(argv[0]);
    return 1;
  }

  FILE *output_str;
  if (!strcmp(options.output_file, "-")) {
    // Output to stdout.
    output_str = stdout;
  } else {
    // Output to a file.
    output_str = fopen(options.output_file, "wb");
    if (output_str == NULL) {
      fprintf(stderr, "Couldn't open output file \"%s\"\n",
	      options.output_file);
      return 1;
    }
  }

  // Load the kernelcache from an img3 file.
  Buffer kernelcache;
  if (!kernelcache.loadFromAuto(options.kernelcache_file,
				IMAGE_TYPE_KERNELCACHE)) {
    fprintf(stderr, "Couldn't load kernel cache %s\n",
	    options.kernelcache_file);
    return 1;
  }

  // Decompress the kernel cache to a Mach-O binary.
  Buffer macho;
  if (!DecompressKernelcache(kernelcache, &macho)) {
    fprintf(stderr, "Couldn't decompress kernel cache\n");
    return 1;
  }

  // Unpack the Mach-O kernel into the base of 'ram'.
  Buffer ram;
  ram.alloc(options.sdram_len);
  LoadMachO load_macho(macho, &ram);
  if (options.override_virtual_base) {
    load_macho.OverrideVirtualBase(options.virtual_base);
  }
  if (!load_macho.Load()) {
    fprintf(stderr, "Couldn't unpack Mach-O kernel image\n");
    return 1;
  }
  uint64_t virtual_base;
  if (!load_macho.GetVirtualBase(&virtual_base)) {
    fprintf(stderr, "Couldn't infer virtual base of Mach-O kernel image\n");
    return false;
  }
  uint64_t phys_base = options.sdram_base;
  uint64_t image_size = load_macho.LastAddress();
  uint64_t entry_point_phys_offset = load_macho.EntryPointPhysOffset();

  // Load the device tree from an img3 file.
  Buffer devicetree;
  if (!devicetree.loadFromAuto(options.devicetree_file, IMAGE_TYPE_DEVTREE)) {
    fprintf(stderr, "Couldn't load devicetree %s\n", options.devicetree_file);
    return 1;
  }

  // Load the ramdisk from an img3 file.
  Buffer ramdisk;
  if (options.ramdisk_file != NULL &&
      !ramdisk.loadFromAuto(options.ramdisk_file, IMAGE_TYPE_RAMDISK)) {
    fprintf(stderr, "Couldn't load ramdisk %s\n", options.ramdisk_file);
    return 1;
  }

  fprintf(stderr,
	  "kernelcache: %u\n"
	  "macho:       %u\n"
	  "unpacked:    %llu\n"
	  "devicetree:  %u\n"
	  "ramdisk:     %u\n",
	  (unsigned) kernelcache.size(),
	  (unsigned) macho.size(),
	  image_size,
	  (unsigned) devicetree.size(),
	  (unsigned) ramdisk.size());

  // Append the device tree to the unpacked Mach-O kernel.
  uint64_t devicetree_offset = RoundToNextPage(options, image_size);
  if (devicetree_offset + devicetree.size() > ram.size()) {
    fprintf(stderr, "Kernel + devicetree too large\n");
    return 1;
  }
  memcpy((char *) ram.buf() + devicetree_offset,
	 devicetree.buf(),
	 devicetree.size());

  // Append the ramdisk to the device tree.
  uint64_t ramdisk_offset =
      RoundToNextPage(options, devicetree_offset + devicetree.size());
  memcpy((char *) ram.buf() + ramdisk_offset, ramdisk.buf(), ramdisk.size());

  // Append bootargs to the device tree.
  uint64_t bootargs_offset =
      RoundToNextPage(options, ramdisk_offset + ramdisk.size());

  // Framebuffer at the end of memory, followed by panic ram.
  uint64_t framebuffer_offset =
      options.sdram_len - PANIC_SIZE - FRAMEBUFFER_SIZE_BYTES;
  // Round down to a 1MB segment. MMU table initialization requires this.
  framebuffer_offset &= ~((1 << 20) - 1);

  uint64_t end_of_image = 0;
  uint64_t end_of_image_offset = 0;
  
#ifdef USE_L4
  monitor_boot_args mba;
  uint64_t mba_offset;

  if( options.l4_path )
  {
	  uint64_t monitor_offset = (framebuffer_offset - MONITOR_SIZE) & ~(0x200000ULL - 1); // 2MB-aligned
	  mba_offset = monitor_offset + MONITOR_SIZE - sizeof(mba);
  
	  mba.version  = 1;
	  mba.virtBase = 0; // L4 doesn't care
	  mba.physBase = monitor_offset + phys_base; // L4 doesn't care
	  mba.memSize  = MONITOR_SIZE;
	  mba.kernArgs = bootargs_offset + phys_base;
	  mba.kernEntry = entry_point_phys_offset + phys_base;
	  fprintf(stderr, "monitor at %llx\n", monitor_offset);

	  end_of_image_offset =
		  RoundToNextMmuBase(monitor_offset + MONITOR_SIZE);
  
	  Buffer l4;
	  if (!l4.loadFromAuto(options.l4_path,
	                       IMAGE_TYPE_MONITOR)) {
		  fprintf(stderr, "Couldn't open monitor image\n");
		  return 1;
	  }
	  
	  fprintf(stderr, "Opened monitor image\n");
	  LoadMachO load_l4(l4, &ram);
	  fprintf(stderr, "loaded monitor image\n");

	  // L4 in 16KB mode is just a mini monitor with a different
	  // link address.
	  uint64_t l4_link_base =
		  options.page_size_16kb
		  ? 0x0000004000000000ULL
		  : 0x0000004100000000ULL;
	  load_l4.OverrideVirtualBase(l4_link_base - monitor_offset);
	  if (options.override_virtual_base) {
		  fprintf(stderr, "overriding virt base\n");
		  load_l4.OverrideVirtualBase(options.virtual_base);
		  fprintf(stderr, "overrode virt base\n");
	  }
	  fprintf(stderr, "about to load\n");
	  if (!load_l4.Load()) {
		  fprintf(stderr, "Couldn't unpack Mach-O kernel image\n");
		  return 1;
	  }

	  /* re-set the entry point now to use L4's */
	  entry_point_phys_offset = load_l4.EntryPointPhysOffset();
	  fprintf(stderr, "Loaded l4: ept %llx\n", entry_point_phys_offset);
  }
  else
#endif
    end_of_image_offset =
	    RoundToNextMmuBase(bootargs_offset + sizeof(boot_args));

  end_of_image = phys_base + end_of_image_offset;
  if (end_of_image_offset > framebuffer_offset) {
    fprintf(stderr, "Image size runs into framebuffer ram (%llx vs %llx)\n",
           end_of_image_offset, framebuffer_offset);

    return 1;
  }


  // Place a bootstrap to load r0 with the offset of the boot args,
  // and jump to the entry point.
  if (load_macho.Is64Bit()) {
    // Don't need to use exception vector to get started on ARM64 

	// [~] $ cat bootstrap.s 
	// 	adr x0, Lboot_args_addr
	// 	adr x1, Lentry_addr		
	// 	ldr	x0, [x0]			/* Load boot args address */
	// 	ldr	x1, [x1]			/* Load entry point */
	// 	br x1					/* Jump to entry point */
	//	.long 0					/* Padding */
	// Lboot_args_addr:
	//	.quad 0xfeedbeef
	// Lentry_addr:
	//	.quad 0xabcdef11
	//
	// [~] $ xcrun -sdk iphoneos clang -arch arm64 -c bootstrap.s
	// [~] $ otool -t bootstrap.o
	//	bootstrap.o:
	//	(__TEXT,__text) section
	//	00000000 100000c0 100000e1 f9400000 f9400021 
	//	00000010 d61f0020 00000000 feedbeef 00000000 
	//	00000020 abcdef11 00000000 
	
    // Validated by hand that "adr" instructions are encoded as desired.
    uint64_t entry_point_phys = phys_base + entry_point_phys_offset;
    uint64_t boot_args_phys = phys_base + bootargs_offset;

    uint32_t bootstrap[] = { 
      0xd5034fdf,	// msr DAIFSet, #15
      0xd5033fdf,	// isb sy
      0x100000c0, // Get location of boot args address: adr x0, Lboot_args_addr (pc + 24)
      0x100000e1, // Get location of entry point: adr x1, Lentry_addr (pc + 32)
      0xf9400000, // Load address of boot args: ldr x0, [x0] 
      0xf9400021, // Load entry point: ldr  x1, [x1]  
      0xd61f0020, // Jump to entry point
      0,	  // Padding for proper alignment (we reset with all memory as "device")
      (uint32_t) boot_args_phys, 		// Boot args address (low 32 bit)
      (uint32_t) (boot_args_phys >> 32), 	// Boot args address (high 32 bits)
      (uint32_t) entry_point_phys,		// Entry address (low 32 bits)
      (uint32_t) (entry_point_phys >> 32),	// Entry address (high 32 bits)
      0		     
    };
    
#ifdef USE_L4
    if( options.l4_path ) {
	    memcpy((char *)ram.buf() + (uintptr_t)mba_offset, &mba, sizeof(mba));
	    uint64_t mba_phys = phys_base + mba_offset;
	    bootstrap[8] = (uint32_t) mba_phys; /* Pointer to monitor_boot_args */
	    bootstrap[9] = (uint32_t) (mba_phys >> 32);
	}
#endif
    memcpy(ram.buf(), bootstrap, sizeof(bootstrap));
    
  } else {
    uint32_t bootstrap[] = {
      ARM_B_POS_0x40,		// 0x00: b 0x40
      0,	// undef exception
      0,  // swi exception
      0,  // pabt exception
      0,  // dabt exception
      0,  // reserved exception
      0,  // irq exception
      0,  // fiq exception
      0,				// 0x20: exception vector table
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      ARM_LDR_R0_PC,		// 0x40: ldr r0, [pc]  ; ldr r0, [0x48]
      ARM_LDR_PC_PC,		// 0x44: ldr pc, [pc]  ; ldr pc, [0x4c]
      (uint32_t) phys_base + (uint32_t) bootargs_offset,// 0x48: .word bootargs_offset
      (uint32_t) (phys_base + entry_point_phys_offset) // 0x4c: .word entry_point
    };

    assert(phys_base == (uint32_t)phys_base);
    assert(bootargs_offset == (uint32_t)bootargs_offset);
    assert((phys_base + bootargs_offset) == (uint32_t)(phys_base + bootargs_offset));

    memcpy(ram.buf(), bootstrap, sizeof(bootstrap));
  } 

  // Place the boot args into memory now that we have the location of
  // everything.
  if (load_macho.Is64Bit()) {
    printf("doing 64-bit boot-args.\n");
    boot_args_64 bootArgs;

    memset(&bootArgs, 0, sizeof(bootArgs));
    bootArgs.revision = kBootArgsRevision;
    bootArgs.version  = kBootArgsVersion1 + ARM64_BOARD_SECURITY_EPOCH;
    bootArgs.virtBase = virtual_base;
    bootArgs.physBase = phys_base;
    bootArgs.memSize = framebuffer_offset;  // Must be 1MB aligned.
    // Physical address of "video" ram.
    bootArgs.video.v_baseAddr = phys_base + framebuffer_offset;
    bootArgs.video.v_display = 1;  // Display code is just 1
    bootArgs.video.v_rowBytes = FRAMEBUFFER_STRIDE_BYTES;
    bootArgs.video.v_width = FRAMEBUFFER_WIDTH;
    bootArgs.video.v_height = FRAMEBUFFER_HEIGHT;
    bootArgs.video.v_depth = FRAMEBUFFER_DEPTH_BITS;
    // Physical address. MMU L1 tables are built here.
#ifdef USE_L4
    if( options.l4_path )
	    bootArgs.topOfKernelData = RoundToNextMmuBase(phys_base + bootargs_offset + sizeof(boot_args));
    else
#endif
	    bootArgs.topOfKernelData = end_of_image;
    // Virtual address of device tree.
    bootArgs.deviceTreeAddr = virtual_base + devicetree_offset;
    bootArgs.deviceTreeLength = devicetree.size();
    memcpy(bootArgs.commandLine,
      options.bootargs_string.data(),
      options.bootargs_string.size());
    memcpy((char *) ram.buf() + bootargs_offset, &bootArgs, sizeof(bootArgs));
  } else {
    boot_args bootArgs;
    
    memset(&bootArgs, 0, sizeof(bootArgs));
    bootArgs.revision = kBootArgsRevision;
    bootArgs.version  = kBootArgsVersion1;
    bootArgs.virtBase = (uint32_t)virtual_base;
    bootArgs.physBase = (uint32_t)phys_base;
    bootArgs.memSize = (uint32_t)framebuffer_offset;  // Must be 1MB aligned.
    // Physical address of "video" ram.
    bootArgs.video.v_baseAddr = (uint32_t)(phys_base + framebuffer_offset);
    bootArgs.video.v_display = 1;  // Display code is just 1
    bootArgs.video.v_rowBytes = FRAMEBUFFER_STRIDE_BYTES;
    bootArgs.video.v_width = FRAMEBUFFER_WIDTH;
    bootArgs.video.v_height = FRAMEBUFFER_HEIGHT;
    bootArgs.video.v_depth = FRAMEBUFFER_DEPTH_BITS;
    // Physical address. MMU L1 tables are built here.
    bootArgs.topOfKernelData = (uint32_t)end_of_image;
    // Virtual address of device tree.
    bootArgs.deviceTreeAddr = (uint32_t)(virtual_base + devicetree_offset);
    bootArgs.deviceTreeLength = devicetree.size();
    memcpy(bootArgs.commandLine,
      options.bootargs_string.data(),
      options.bootargs_string.size());
	  memcpy((char *) ram.buf() + bootargs_offset, &bootArgs, sizeof(bootArgs));
  }

  // Patch the device tree with the location of loaded segments.
  DeviceTreePatcher device_tree_patcher((char *) ram.buf() + devicetree_offset, devicetree.size());

  // There's a large number of properties not being set here, such as
  // 'firmware-version', 'pram', 'vram', but these are not necessary
  // yet.

  // Patch the location of the device tree itself.
  if (!device_tree_patcher.AllocateMemoryRange("DeviceTree",
					       phys_base + devicetree_offset,
					       devicetree.size())) {
    return 1;
  }

  // Patch the location of the ramdisk, if used.
  if (options.ramdisk_file != NULL &&
      !device_tree_patcher.AllocateMemoryRange("RAMDisk",
					       phys_base + ramdisk_offset,
					       ramdisk.size())) {
    return 1;
  }

  // Patch the location of the bootargs.
  if (!device_tree_patcher.AllocateMemoryRange("BootArgs",
					       phys_base + bootargs_offset,
					       sizeof(boot_args))) {
    return 1;
  }

  // Patch KernelCache segments into the device tree.
  for (LoadMachO::SegmentListIterator it = load_macho.SegmentListBegin();
       it != load_macho.SegmentListEnd();
       ++it) {
    if (!device_tree_patcher.AllocateMemoryRange(it->name.c_str(),
						 it->pmaddr,
						 it->size)) {
      return 1;
    }
    // Also check that no segment overlaps the first 4KB page of
    // memory. That would mean the bootstrap overwrote something.
    if (it->pmaddr < 4096 && it->size != 0) {
      fprintf(stderr, "KernelCache segment \"%s\" overlaps first 4KB\n",
	      it->name.c_str());
      return 1;
    }
  }

  // Set /chosen:debug-enabled=1.
  if (!device_tree_patcher.SetPropertyInt("chosen", "debug-enabled", 1)) {
    fprintf(stderr, "Couldn't set /chosen:debug-enabled=1\n");
    return 1;
  }

  // Set (arbitrary) core and peripheral frequencies in the device tree.
  if (!SetFrequenciesInDeviceTree(&device_tree_patcher)) {
    fprintf(stderr, "Couldn't set device frequencies\n");
    return 1;
  }

  if (options.have_boot_partition_id) {
    // Set /chosen/root-matching=<xml>...boot_partition_id...</xml>.
    char s[256];
    snprintf(s, sizeof(s),
	     "<dict>"
	     "<key>IOProviderClass</key>"
	     "<string>IOMedia</string>"
	     "<key>IOPropertyMatch</key>"
	     "<dict><key>Partition ID</key><integer>%u</integer></dict>"
	     "</dict>",
	     options.boot_partition_id);
    if (!device_tree_patcher.SetPropertyString("chosen", "root-matching", s)) {
      fprintf(stderr, "Couldn't set /chosen:root-matching=%s\n", s);
      return 1;
    }
  }

  // Set the location of panic ram. First word physical base, second word size.
  uint64_t panic_prop[2];
  bool wide_panic_prop;

  uint64_t vram_prop[2];

  switch (device_tree_patcher.GetNumAddressCells("")) { // Empty string -> root
  case 1: {
    wide_panic_prop = false;
    panic_prop[0] = ((uint64_t)PANIC_SIZE << 32) | (phys_base + options.sdram_len - PANIC_SIZE);
    if (!device_tree_patcher.SetPropertyInt("pram", "reg", panic_prop[0])) {
      fprintf(stderr, "Couldn't set /pram:reg=0x%016llx\n", panic_prop[0]);
      return 1;
    }

    vram_prop[0] = (((uint64_t)FRAMEBUFFER_SIZE_BYTES) << 32) | (phys_base + framebuffer_offset);
    if (!device_tree_patcher.SetPropertyInt("vram", "reg", vram_prop[0])) {
      fprintf(stderr, "Couldn't set /vram:reg=0x%016llx\n", vram_prop[0]);
      return 1;
    }
    break;
  }
  case 2: {
    wide_panic_prop = true;
    panic_prop[0] = phys_base + options.sdram_len - PANIC_SIZE;
    panic_prop[1] = ((uint64_t) PANIC_SIZE);
    if (!device_tree_patcher.SetPropertyTwoQuads("pram", "reg", panic_prop)) {
      fprintf(stderr, "Couldn't set /pram:reg=0x%016llx,0x%016llx\n", panic_prop[0], panic_prop[1]);
      return 1;
    }
    vram_prop[0] = phys_base + framebuffer_offset;
    vram_prop[1] = FRAMEBUFFER_SIZE_BYTES;
    if (!device_tree_patcher.SetPropertyTwoQuads("vram", "reg", vram_prop)) {
      fprintf(stderr, "Couldn't set /vram:reg=0x%016llx,0x%016llx\n", vram_prop[0], vram_prop[1]);
      return 1;
    }
    break;
  }
  default: {
    fprintf(stderr, "Can't set pram address because number of address cells is bad.\n");
    return 1;
  }
  }

  std::vector<dt_patch_t>::const_iterator patch_iterator;
  for (patch_iterator = options.dt_patches.begin(); patch_iterator != options.dt_patches.end(); ++patch_iterator) {
	  dt_patch_t patch = *patch_iterator;
	  if (patch.str) {
		  fprintf(stderr, "Patching /%s:%s=%s\n", patch.node, patch.property, patch.value);
		  if (!device_tree_patcher.SetPropertyString(patch.node, patch.property, patch.value)) {
			  fprintf(stderr, "Couldn't set /%s:%s=%s\n", patch.node, patch.property, patch.value);
			  return 1;
		  }
	  } else {
		  uint64_t value = atoi(patch.value);
		  fprintf(stderr, "Patching /%s:%s=0x%016llx\n", patch.node, patch.property, value);
		  if (!device_tree_patcher.SetPropertyInt(patch.node, patch.property, value)) {
			  fprintf(stderr, "Couldn't set /%s:%s=0x%016llx\n", patch.node, patch.property, value);
			  return 1;
		  }
	  }
  }

  assert(end_of_image_offset == (size_t)end_of_image_offset);
  fwrite(ram.buf(), (size_t)end_of_image_offset, 1, output_str);

  fprintf(stderr,
	  "KernelCache:    %s\n"
	  "DeviceTree:     %s\n"
	  "SDRAM size:     %uMB\n"
	  "SDRAM base:     0x%llx\n"
	  "Virtual base:   0x%016llx\n"
	  "Top of k. data: 0x%016llx\n"
	  "Output image:   %s\n"
	  "Boot args:      %s\n"
	  "Ramdisk:        %s at 0x%llx, %u\n"
	  "Framebuffer:    0x%llx %dx%dx%dbpp (%uKB)\n"
	  "Panic ram:      0x%llx size %lluKB\n"
	  "Image size:     %llu bytes\n"
	  "Entry point:    0x%016llx\n"
	  "Boot partition: %s\n",
	  options.kernelcache_file,
	  options.devicetree_file,
	  options.sdram_len / 1024 / 1024,
	  options.sdram_base,
	  virtual_base,
	  end_of_image,
	  strcmp(options.output_file, "-") ? options.output_file : "<stdout>",
	  options.bootargs_string.c_str(),
	  options.ramdisk_file ? options.ramdisk_file : "(none)",
	  phys_base + ramdisk_offset,
	  (unsigned) ramdisk.size(),
	  phys_base + framebuffer_offset,
	  FRAMEBUFFER_WIDTH,
	  FRAMEBUFFER_HEIGHT,
	  FRAMEBUFFER_DEPTH_BITS,
	  (unsigned) RoundToNextPage(options, FRAMEBUFFER_SIZE_BYTES) / 1024,
	  wide_panic_prop ? panic_prop[0] : panic_prop[0] & 0xFFFFFFFF,
	  wide_panic_prop ? panic_prop[1] / 1024 : (panic_prop[0] & 0xFFFFFFFF00000000ULL) >> 44,
	  end_of_image - phys_base,
	  phys_base + entry_point_phys_offset,
	  options.have_boot_partition_id ? "Root filesystem" : "Ramdisk");

  if (output_str != stdout) {
    fclose(output_str);
  }
  return 0;
}

static void Usage(const char *argv0) {
  fprintf(stderr,
	  "Usage:\n"
	  "  %s <flags>\n"
	  "\n"
	  "Flags are: ('*' denotes mandatory)\n"
	  "\n"
	  "* -k <kernelcache>\n"
	  "      Specify the kernelcache file to use. This is decompressed\n"
	  "      and the contained Mach-O binary unpacked at the start of\n"
	  "      the resulting image\n"
	  "* -d <devicetree>\n"
	  "      Specify the devicetree file to use. This is extracted and\n"
	  "      appended to the kernelcache in the resulting image.\n"
	  "  -r <ramdisk>\n"
	  "      Specify the ramdisk file to use. This is extracted and\n"
	  "      appended to the devicetree in the resulting image. Optional.\n"
	  "* -o <outputfile>\n"
	  "      Specify the output file to use. This will contain the image\n"
	  "      to be loaded into memory starting at the base of SDRAM.\n"
	  "      You can use '-' to pipe to standard output.\n"
	  "  -m <ramsize>\n"
	  "      The target's memory size, in MB. This is inserted into the\n"
	  "      boot args structure, and bounds checked when creating the\n"
	  "      image. This does not change the image size.\n"
	  "      Default is %uMB.\n"
	  "  -v <virtualbase>\n"
	  "      The target's kernel virtual memory base. This should match\n"
	  "      the layout of he input KernelCache or it will fail to\n"
	  "      translate to the base of SDRAM.\n"
	  "      Default inferred from the layout of the kernelcache.\n"
	  "  -s <sdrambase>\n"
	  "      The target's SDRAM memory base. This is the location where\n"
	  "      the output image will be loaded on the target.\n"
	  "      Default is 0x%08x.\n"
	  "  -p <node> <property> <value>\n"
	  "      Patch the devicetree property with the specified integer value.\n"
	  "  -P <node> <property> <value>\n"
	  "      Patch the devicetree property with the specified string value.\n"
	  "  -K\n"
	  "      Tweak offsets for expected kernel layout with 16KB pages.\n"
	  "  -- <bootargs> [...]\n"
	  "      All remaining arguments after '--' are copied into the boot\n"
	  "      args structure as strings separated by spaces.\n"
	  "      Default is an empty string.\n"
	  "",
	  argv0,
	  DEFAULT_SDRAM_LEN / 1024 / 1024,
	  DEFAULT_SDRAM_BASE);
}

bool Options::Parse(int argc, char **argv) {
  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-k")) {
      if (i + 1 >= argc) {
	fprintf(stderr, "-k must be followed by the kernel cache file name\n");
	return false;
      }
      kernelcache_file = argv[++i];
    } else if (!strcmp(argv[i], "-d")) {
      if (i + 1 >= argc) {
	fprintf(stderr, "-d must be followed by the devicetree file name\n");
	return false;
      }
      devicetree_file = argv[++i];
    } else if (!strcmp(argv[i], "-r")) {
      if (i + 1 >= argc) {
	fprintf(stderr, "-r must be followed by the ramdisk file name\n");
	return false;
      }
      ramdisk_file = argv[++i];
    } else if (!strcmp(argv[i], "-m")) {
      if (i + 1 >= argc) {
	fprintf(stderr, "-m must be followed by the SDRAM size in MB\n");
	return false;
      }
      sdram_len = strtoul(argv[++i], NULL, 0) * 1024 * 1024;
    } else if (!strcmp(argv[i], "-o")) {
      if (i + 1 >= argc) {
	fprintf(stderr, "-o must be followed by an output file name\n");
	return false;
      }
      output_file = argv[++i];
    } else if (!strcmp(argv[i], "-s")) {
      if (i + 1 >= argc) {
	fprintf(stderr, "-s must be followed by an address\n");
	return false;
      }
      sdram_base = strtoull(argv[++i], NULL, 0);
    } else if (!strcmp(argv[i], "-v")) {
      if (i + 1 >= argc) {
	fprintf(stderr, "-v must be followed by an address\n");
	return false;
      }
      virtual_base = strtoull(argv[++i], NULL, 0);
      override_virtual_base = true;
    } else if (!strcmp(argv[i], "-b")) {
      if (i + 1 >= argc) {
	fprintf(stderr, "-b must be followed by a partition id\n");
	return false;
      }
      have_boot_partition_id = true;
      boot_partition_id = strtoul(argv[++i], NULL, 0);
    } else if (!strcmp(argv[i], "-p")) {
      if (i + 3 >= argc) {
        fprintf(stderr, "-p must be followed by a node, property, and value\n");
        return false;
      }

	  dt_patch_t patch = { argv[i+1], argv[i+2], argv[i+3], false };
      dt_patches.push_back(patch);
	  i += 3;
    } else if (!strcmp(argv[i], "-P")) {
      if (i + 3 >= argc) {
        fprintf(stderr, "-P must be followed by a node, property, and value\n");
        return false;
      }

	  dt_patch_t patch = { argv[i+1], argv[i+2], argv[i+3], true };
      dt_patches.push_back(patch);
	  i += 3;
    } else if (!strcmp(argv[i], "-K")) {
      page_size_16kb = true;
#ifdef USE_L4
    } else if (!strcmp(argv[i], "-l")) {
	    fprintf(stderr, "got an l4!\n");
      if (i + 1 >= argc) {
	fprintf(stderr, "-l must be followed by the L4 file name\n");
	return false;
      }
      l4_path = argv[++i];
#endif
    } else if (!strcmp(argv[i], "--")) {
      // Consume the remaining arguments for bootargs.
      for (++i; i < argc; ++i) {
	if (!bootargs_string.empty()) bootargs_string += ' ';
	bootargs_string += argv[i];
      }
    } else {
      fprintf(stderr, "Unrecognized option \"%s\"\n", argv[i]);
      return false;
    }
  }

  if (kernelcache_file == NULL ||
      devicetree_file == NULL ||
      output_file == NULL) {
    fprintf(stderr, "Must specify kernelcache, devicetree and output file\n");
    return false;
  }

  if (bootargs_string.size() >= BOOT_LINE_LENGTH) {
    fprintf(stderr, "Boot args too long (%u >= %u)\n",
	    (unsigned) bootargs_string.size(),
	    (unsigned) BOOT_LINE_LENGTH);
    return false;
  }

  return true;
}

void _panic(const char *func, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "\npanic in %s: ", func);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n\n");
  va_end(args);
  exit(1);
}

static uint64_t RoundToNextPage(const Options &options, uint64_t address) {
  // Round an address up to the next page boundary if not aligned (4KB or 16KB)
  uint64_t page_size = options.page_size_16kb ? 16384 : 4096;
  address += page_size - 1;
  address &= ~((uint64_t) page_size - 1);
  return address;
}

static uint64_t RoundToNextMmuBase(uint64_t address) {
  // Round an address up to the next MMU base alignment (16KB).
  address += TARGET_MMU_BASE_ALIGN - 1;
  address &= ~((uint64_t) TARGET_MMU_BASE_ALIGN - 1);
  return address;
}

static bool SetFrequenciesInDeviceTree(DeviceTreePatcher *device_tree_patcher) {
  struct {
    const char *name;
    uint32_t mhz;
  } cpu_clocks[] = {
    { "clock-frequency", 1000 },
    { "memory-frequency", 500 },
    { "bus-frequency", 250 },
    { "peripheral-frequency", 250 },
    { "fixed-frequency", 24 },
    { "timebase-frequency", 24 },
  };
  for (size_t i = 0; i < sizeof(cpu_clocks) / sizeof(cpu_clocks[0]); ++i) {
    assert(cpu_clocks[i].mhz <= UINT32_MAX / 1000000);
    if (!device_tree_patcher->SetPropertyInt("cpus/cpu0",
					     cpu_clocks[i].name,
					     cpu_clocks[i].mhz * 1000000)) {
      fprintf(stderr, "Couldn't set cpus/cpu0:%s = %uMHz\n",
	      cpu_clocks[i].name, (unsigned) cpu_clocks[i].mhz);
      return false;
    }
  }
  return true;
}
