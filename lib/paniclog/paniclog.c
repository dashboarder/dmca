/*
 * Copyright (c) 2010, 2013-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <stdlib.h>
#include <list.h>
#include <lib/mib.h>
#include <platform.h> /* for platform_cache_operation() */
#include <lib/cksum.h>
#include <lib/nvram.h>
#include <lib/blockdev.h>

/* If the panic ram area is large enough, use copies to improve
 * resilience against errors. */
#define PANIC_MIN_SIZE_FOR_COPIES	(16 * 1024)

/* The number of copies (including the original) to make of the entire
 * panic log, including headers, if the area is large enough. This
 * provides a minimum of 2664 (after 8->7 bit packing and headers)
 * bytes of contained log data. */
#define PANIC_COPIES	7

#define PANIC_BDEV_NAME "paniclog"

struct vram_panic {
        uint32_t magic;
        uint32_t crc;
        char buf[];
};

void clear_panic_region(unsigned char pattern)
{
	void	*panicBufferAddr = (void *)mib_get_addr(kMIBTargetPanicBufferAddress);
	size_t	panicBufferSize = mib_get_size(kMIBTargetPanicBufferSize);

	/* clear the panic area to a pattern. values other than 0 useful for debug */
	memset(panicBufferAddr, pattern, panicBufferSize);
	/* ensure this reaches DRAM */
#if defined(__arm64__)
	/* on coherent archs (H6+), force a cache clean by adding invalidate flag, and specifying address and size */
	platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE | CACHE_DRAIN_WRITE_BUFFER, panicBufferAddr, panicBufferSize);
#else
	platform_cache_operation(CACHE_CLEAN | CACHE_DRAIN_WRITE_BUFFER, NULL, 0);
#endif
}

// arm64 targets only have the new panic log format.
// armv7? targets may have either.
int save_panic_log(void)
{
	struct vram_panic *vpi = NULL;
	void *panic_buf = NULL;
	uint32_t panic_text_len;
	addr_t	panicBufferAddr = mib_get_addr(kMIBTargetPanicBufferAddress);
	size_t	panicBufferSize = mib_get_size(kMIBTargetPanicBufferSize);

#if !defined(__arm64__)
	if (mib_get_bool(kMIBTargetWithLegacyPanicLogs)) {
		struct {
			unsigned a:7;
			unsigned b:7;
			unsigned c:7;
			unsigned d:7;
			unsigned e:7;
			unsigned f:7;
			unsigned g:7;
			unsigned h:7;
		} __attribute__((packed)) pack;

		uint32_t panic_size;
		int i, panic_copies;

		if (panicBufferSize >= PANIC_MIN_SIZE_FOR_COPIES) {
			panic_size = panicBufferSize / PANIC_COPIES;
			panic_copies = PANIC_COPIES;
		} else {
			panic_size = panicBufferSize;
			panic_copies = 1;
		}
		panic_text_len = panic_size - 8;

		/* find an intact copy */
		for (i = 0; i < panic_copies; ++i) {
			vpi = (struct vram_panic *) (panicBufferAddr + i * panic_size);
			/* check for magic header */
			if (vpi->magic != 0x4F4F5053 /* 'OOPS' */) {
				continue;
			}
			/* check CRC */
			if (vpi->crc != crc32((unsigned char *) vpi->buf, panic_text_len)) {
				continue;
			}
			/* this copy is very likely intact */
			panic_buf = vpi->buf;
			break;
		}

		/* failed to find an intact copy? */
		if (panic_buf == NULL) {
			/* Don't attempt correction. This method should have
			 * been resilient enough that at least one copy
			 * survived. Log this CRC error. */
			pack.a = 0x43; /* 'C' */
			pack.b = 0x52; /* 'R' */
			pack.c = 0x43; /* 'C' */
			pack.d = 0x20; /* ' ' */
			pack.e = 0x45; /* 'E' */
			pack.f = 0x52; /* 'R' */
			pack.g = 0x52; /* 'R' */
			pack.h = 0x21; /* '!' */
			panic_buf = &pack;
			panic_text_len = sizeof(pack);
		}

		/* write the panic to the nvram partition */
		if (nvram_set_panic(panic_buf, panic_text_len) < 0) {
			clear_panic_region(0xe1);
			return -1;
		}

		/* wipe the area so we don't produce duplicate logs */
		clear_panic_region(0);

		/* save nvram */
		if (nvram_save() < 0) {
			clear_panic_region(0xe2);
			return -1;
		}

		return 0;
	}
#endif	// !defined(__arm64__)

	int err;
	struct blockdev *dev;
	const char *crc_err = "CRC ERR!";

	ASSERT(!mib_get_bool(kMIBTargetWithLegacyPanicLogs));

	panic_text_len = panicBufferSize - 8;
	vpi = (struct vram_panic *) panicBufferAddr;
	/* check for magic header 'DARN'/'SICK' and CRC value */
	if ((vpi->magic == 0x4441524E || vpi->magic == 0x5349434B) &&
		vpi->crc == crc32((unsigned char *) vpi->buf, panic_text_len)) {
		/* this copy is very likely intact */
		panic_buf = vpi->buf;
	} else {
		/* failed to match DARN magic header or CRC */
		/* Don't attempt correction. Log this CRC error. */
		panic_buf = (void*)crc_err;
		panic_text_len = sizeof(*crc_err);
	}
	dev = lookup_blockdev(PANIC_BDEV_NAME);
	if (!dev) {
		dprintf(DEBUG_CRITICAL, "Couldn't find block device '%s'\n", PANIC_BDEV_NAME);
		clear_panic_region(0xe1);
		return -1;
	}
	err = blockdev_write(dev, (void *)panic_buf, 0, panic_text_len);
	if (err <= 0) {
		dprintf(DEBUG_CRITICAL, "blockdev_write to '%s' device failed with return value %d\n",
			PANIC_BDEV_NAME, err);
		clear_panic_region(0xe2);
		return -1;
	}

	/* wipe the area so we don't produce duplicate logs */
	clear_panic_region(0);

	return 0;
}

