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

#include <platform/memmap.h>

/*
 * Utility file exporting symbols at absolute locations.  These
 * values are largely useful to the build system before the final
 * object is linked, or for diagnosis purposes later.
 */

	/*
	 * symbol <name> <value>
	 */
	.macro	symbol
	.global $0
	.set $0 , $1
	.endmacro

	/* This is used at link time */
	symbol	_TEXT_BASE,			TEXT_BASE

#if 0
	/* debug/sanity checks */
	symbol	_INSECURE_MEMORY_BASE,		INSECURE_MEMORY_BASE
	symbol	_INSECURE_MEMORY_SIZE,		INSECURE_MEMORY_SIZE
	symbol	_DEFAULT_KERNEL_ADDRESS,	DEFAULT_KERNEL_ADDRESS
	symbol	_DEFAULT_KERNEL_SIZE,		DEFAULT_KERNEL_SIZE
	symbol	_DEFAULT_RAMDISK_ADDRESS,	DEFAULT_RAMDISK_ADDRESS
	symbol	_DEFAULT_RAMDISK_SIZE,		DEFAULT_RAMDISK_SIZE
	symbol	_DEFAULT_DEVICETREE_ADDRESS,	DEFAULT_DEVICETREE_ADDRESS
	symbol	_DEFAULT_DEVICETREE_SIZE,	DEFAULT_DEVICETREE_SIZE
	
	symbol	_IBOOT_SIZE,			IBOOT_SIZE
	symbol	_MMU_TT_BASE,			MMU_TT_BASE
	symbol	_MMU_TT_SIZE,			MMU_TT_SIZE
	symbol	_STACK_BASE,			STACK_BASE
	symbol	_STACK_SIZE,			STACK_SIZE
#endif