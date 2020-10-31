/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

// Taken from pexpert/pexpert/arm/boot.h and pexpert/pexpert/arm64/boot.h
// Modified for iBoot: 
//	- Fixed incosistent naming in boot-args struct
//	- Added monitor_boot_args

/*
 * Copyright (c) 2007-2009 Apple Inc. All rights reserved.
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
 */
/*
 * @OSF_COPYRIGHT@
 */

#ifndef _PEXPERT_ARM_BOOT_H_
#define _PEXPERT_ARM_BOOT_H_

//#include <kern/kern_types.h>

#define BOOT_LINE_LENGTH        256

/*
 * Video information.. 
 */

struct boot_video {
	unsigned long	v_baseAddr;	/* Base address of video memory */
	unsigned long	v_display;	/* Display Code (if Applicable */
	unsigned long	v_rowBytes;	/* Number of bytes per pixel row */
	unsigned long	v_width;	/* Width */
	unsigned long	v_height;	/* Height */
	unsigned long	v_depth;	/* Pixel Depth and other parameters */
};

#define kBootVideoDepthMask		(0xFF)
#define kBootVideoDepthDepthShift	(0)
#define kBootVideoDepthRotateShift	(8)
#define kBootVideoDepthScaleShift	(16)

typedef struct boot_video	boot_video;

/* Boot argument structure - passed into Mach kernel at boot time.
 */
#define kBootArgsRevision		2
#define kBootArgsVersion1		1
#define kBootArgsVersion2		2

#define kBootFlagDarkBoot		1

#if defined(__LP64__)
typedef struct boot_args {
	uint16_t		revision;			/* Revision of boot_args structure */
	uint16_t		version;			/* Version of boot_args structure */
	uint64_t		virtBase;			/* Virtual base of memory */
	uint64_t		physBase;			/* Physical base of memory */
	uint64_t		memSize;			/* Size of memory */
	uint64_t		topOfKernelData;		/* Highest physical address used in kernel data area */
	boot_video		video;				/* Video Information */
	uint32_t		machineType;			/* Machine Type */
	void			*deviceTreeP;			/* Base of flattened device tree */
	uint32_t		deviceTreeLength;		/* Length of flattened tree */
	char			commandLine[BOOT_LINE_LENGTH];	/* Passed in command line */
	uint64_t		boot_flags;			/* Misc boot flags*/
} boot_args;
#else
typedef struct boot_args {
	uint16_t		revision;			/* Revision of boot_args structure */
	uint16_t		version;			/* Version of boot_args structure */
	uint32_t		virtBase;			/* Virtual base of memory */
	uint32_t		physBase;			/* Physical base of memory */
	uint32_t		memSize;			/* Size of memory */
	uint32_t		topOfKernelData;		/* Highest physical address used in kernel data area */
	boot_video		video;				/* Video Information */
	uint32_t		machineType;			/* Machine Type */
	void			*deviceTreeP;			/* Base of flattened device tree */
	uint32_t		deviceTreeLength;		/* Length of flattened tree */
	char			commandLine[BOOT_LINE_LENGTH];	/* Passed in command line */
	uint32_t		boot_flags;			/* Misc boot flags*/
} boot_args;
#endif 

#define SOC_DEVICE_TYPE_BUFFER_SIZE	32

#define PC_TRACE_BUF_SIZE		1024

/*
 * Monitor/hypervisor boot argument structure.
 */
#if defined(__LP64__)
typedef struct monitor_boot_args {
	uint64_t	version;	/* structure version - this is version 2 */
	uint64_t	virtBase;	/* virtual base of memory assigned to the monitor */
	uint64_t	physBase;	/* physical address corresponding to the virtual base */
	uint64_t	memSize;	/* size of memory assigned to the monitor */
	uint64_t	kernArgs;	/* physical address of the kernel boot_args structure */
	uint64_t	kernEntry;	/* kernel entrypoint */
	uint64_t	kernPhysBase;	/* physical base of the kernel's address space */
	uint64_t	kernPhysSlide;	/* offset from kernPhysBase to kernel load address */
	uint64_t	kernVirtSlide;	/* virtual slide applied to kernel at load time */
} monitor_boot_args;
#else
typedef struct monitor_boot_args {
	uint32_t	version;	/* structure version - this is version 2 */
	uint32_t	virtBase;	/* virtual base of memory assigned to the monitor */
	uint32_t	physBase;	/* physical address corresponding to the virtual base */
	uint32_t	memSize;	/* size of memory assigned to the monitor */
	uint32_t	kernArgs;	/* physical address of the kernel boot_args structure */
	uint32_t	kernEntry;	/* kernel entrypoint */
	uint32_t	kernPhysBase;	/* physical base of the kernel's address space */
	uint32_t	kernPhysSlide;	/* offset from kernPhysBase to kernel load address */
	uint32_t	kernVirtSlide;	/* virtual slide applied to kernel at load time */
} monitor_boot_args;
#endif

#endif /* _PEXPERT_ARM_BOOT_H_ */
