/*
 * Copyright (C) 2006-2012 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __LIB_MACHO_H
#define __LIB_MACHO_H

#include <lib/image.h>
#include <sys/types.h>

__BEGIN_DECLS

/**
 * load a kernelcache from memory, return the entry point
 *
 * \param[in]	addr		Address of the kernelcache in memory.
 * \param[in]	size		Size of the kernelcache in memory.
 * \param[in]	type		Type of kernelcache (restore or local boot).
 * \param[out]	entry		Entrypoint recovered from the kernelcache.
 * \param[out]	boot_args_out	Boot arguments to pass to the kernelcache entry point.
 *
 * \return	0	Kernelcache was loaded successfully.
 * \return	-2	Kernelcache is too large.
 * \return	-3	Kernelcache container failed validation (signature,
 *			wrong system, etc.)
 * \return	-4	Not a valid kernelcache (bad magic number).
 * \return	-5	Decompression failed.
 * \return	-6	Not a valid Mach-o image (bad magic number).
 * \return	-7	Mach-o load failed.
 */
int load_kernelcache(addr_t addr, size_t size, uint32_t type, addr_t *entry, addr_t *boot_args_out);

/**
 * load a kernelcache from a path, return the entry point
 *
 * \param[in]	path		Filesystem path from which to read the kernelcache.
 * \param[in]	type		Type of kernelcache (restore or local boot).
 * \param[out]	entry		Entrypoint recovered from the kernelcache.
 * \param[out]	boot_args_out	Boot arguments to pass to the kernelcache entry point.
 *
 * \return	0	Kernelcache was loaded successfully.
 * \return	-2	Kernelcache is too large.
 * \return	-3	Kernelcache container failed validation (signature,
 *			wrong system, etc.)
 * \return	-4	Not a valid kernelcache (bad magic number).
 * \return	-5	Decompression failed.
 * \return	-6	Not a valid Mach-o image (bad magic number).
 * \return	-7	Mach-o load failed.
 */
int load_kernelcache_file(const char *path, uint32_t type, addr_t *entry, addr_t *boot_args_out);

/**
 * load a kernelcache from a known image, return the entry point
 *
 * \param[in]	image		Known image from which to read the kernelcache.
 * \param[in]	type		Type of kernelcache (restore or local boot).
 * \param[out]	entry		Entrypoint recovered from the kernelcache.
 * \param[out]	boot_args_out	Boot arguments to pass to the kernelcache entry point.
 *
 * \return	0	Kernelcache was loaded successfully.
 * \return	-2	Kernelcache is too large.
 * \return	-3	Kernelcache container failed validation (signature,
 *			wrong system, etc.)
 * \return	-4	Not a valid kernelcache (bad magic number).
 * \return	-5	Decompression failed.
 * \return	-6	Not a valid Mach-o image (bad magic number).
 * \return	-7	Mach-o load failed.
 */
int load_kernelcache_image(struct image_info *image, uint32_t type, addr_t *entry, addr_t *boot_args_out);

/**
 * load a monitor from a path
 *
 * A loaded monitor is recognised by the kernelcache loader; the
 * appropriate entrypoint will be returned at kernelcache load time.
 *
 * \param[in]	path		Filesystem path from which to read the kernelcache.
 *
 * \return	0	Monitor was loaded successfully.
 * \return	-2	Monitor is too large.
 * \return	-3	Monitor container failed validation (signature,
 *			wrong system, etc.)
 * \return	-6	Not a valid Mach-o image (bad magic number).
 * \return	-7	Mach-o load failed.
 */
int load_monitor_file(const char *path);

/**
 * load a monitor from a known image
 *
 * \param[in]	address		Known image address
 * \param[in]	size		Known image size
 *
 * \return	0	Monitor was loaded successfully.
 * \return	-2	Monitor is too large.
 * \return	-3	Monitor container failed validation (signature,
 *			wrong system, etc.)
 * \return	-6	Not a valid Mach-o image (bad magic number).
 * \return	-7	Mach-o load failed.
 */
int load_monitor_image(addr_t address, size_t size);

bool macho_valid(addr_t addr);
bool macho_load(addr_t imageAddr, size_t imageSize, addr_t loadAddr, addr_t *virtualBase, addr_t *virtualEnd, addr_t *entryPoint, size_t slide);

__END_DECLS

#endif

