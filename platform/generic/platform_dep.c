/*
 * Copyright (C) 2007-2015 Apple Inc. All rights reserved.
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
#include <drivers/aes.h>
#include <drivers/power.h>
#if WITH_HW_SEP
#include <drivers/sep/sep_client.h>
#endif
#if WITH_EFFACEABLE
#include <lib/effaceable.h>
#endif
#include <lib/image.h>
#include <lib/libc.h>
#include <lib/nonce.h>
#include <lib/random.h>
#include <lib/syscfg.h>
#include <platform.h>
#include <platform/chipid.h>
#include <platform/memmap.h>
#include <target.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/hash.h>
#include <sys/security.h>
#include <drivers/display.h>

// platfom_dep holds generic functions that are just defined once for all
// platforms, and are expected to be rebuilt for every target/product


#if !PLATFORM_VARIANT_IOP && !PLATFORM_VARIANT_AUDIO
#if (defined(__arm__) || defined(__arm64__))
# include <arch/arm/arm.h>

/*
 * Default implementation of platform_cache_operation(), assumes the ARM architecture
 * supplied functions are available and appropriate.
 */
void platform_cache_operation(int operation, void *address, u_int32_t length)
{
	uintptr_t	start, end;
	void		(*func)(addr_t addr);

	/* all we are willing to do in the panic case is clean the entire cache */
	if (unlikely(operation & CACHE_PANIC)) {
		if ((operation & CACHE_CLEAN) && (NULL == address) && (0 == length))
			arm_clean_dcache();
		return;
	}
	
	/* disable interrupts as we don't want to be re-entered */
	enter_critical_section();

	if ((0 == address) && (0 == length)) {
		
		/* invalidating the entire cache guarantees a crash */
		ASSERT((operation & (CACHE_CLEAN | CACHE_INVALIDATE)) != CACHE_INVALIDATE);
		
		/* whole-cache operations */
		switch(operation & (CACHE_CLEAN | CACHE_INVALIDATE)) {
		case CACHE_CLEAN:
			arm_clean_dcache();
			break;

		case CACHE_CLEAN | CACHE_INVALIDATE:
			arm_clean_invalidate_dcache();
			break;
		}
	} else {

		/* ranged operations by line */
		start = (uintptr_t)address;
		end = start + length;
		switch (operation & (CACHE_CLEAN | CACHE_INVALIDATE)) {
		case CACHE_INVALIDATE:
			/*
			 * Non-aligned range invalidate operations are illegal
			 * as we may shoot something else sharing the line.
			 */
			ASSERT(0 == (start % CPU_CACHELINE_SIZE));
			ASSERT(0 == (length % CPU_CACHELINE_SIZE));

			/* invalidate */
			func = arm_invalidate_dcache_line;
			break;

		case CACHE_CLEAN:
			/* adjust start to the base of the cacheline */
			start &= ~(CPU_CACHELINE_SIZE - 1);
			func = arm_clean_dcache_line;
			break;

		case CACHE_CLEAN | CACHE_INVALIDATE:
			/* adjust start to the base of the cacheline */
			start &= ~(CPU_CACHELINE_SIZE - 1);
			func = arm_clean_invalidate_dcache_line;
			break;
		default:
			func = NULL;
		}
		/* do it */
		if (likely(NULL != func))
			for (; start < end; start += CPU_CACHELINE_SIZE) 
				func(start);
	}

	/* write buffer is address-agnostic */
	if (operation & CACHE_DRAIN_WRITE_BUFFER)
		arm_drain_write_buffer();

#if defined(__arm64__)
	arm_memory_barrier();
#endif

	exit_critical_section();
}

void platform_memory_barrier(void)
{
	arm_memory_barrier();
}

#else
# error no architecture support for platform_cache_operation
#endif /* ARCH_ARM */
#endif // PLATFORM_VARIANT_IOP


//	iBoot Flags
//		Bit 0 - Restore host should send image capable of restore boot (a.k.a. iBEC).
//		Bit 1 - Restore host should send ticket to validate images
//		Bit 2 - Restore host should send Image4 images.
//		Bit 3 - Current status of Secure fuse bit
//		Bit 4 - Current status of Production fuse bit
//		Bit 5 - Restore host should send images with SHA2-384 hash

uint32_t platform_get_iboot_flags(void)
{
	u_int32_t flag_summary = 0;

#if WITH_RESTORE_STRAP
	flag_summary |= (1 << 0);
#endif

#if WITH_TICKET
	flag_summary |= (1 << 1);
#endif

#if WITH_IMAGE4
	flag_summary |= (1 << 2);
#endif

	if (platform_get_secure_mode()) flag_summary |= (1 << 3);

	if (platform_get_current_production_mode()) flag_summary |= (1 << 4);

#if WITH_SHA2_384
	flag_summary |= (1 << 5);
#endif

	return flag_summary;
}

#ifndef PLATFORM_PRODUCT_ID
#define PLATFORM_PRODUCT_ID {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#endif

const char* platform_get_product_id(void)
{
	static const char product_id[] = PLATFORM_PRODUCT_ID;
	return product_id;
}

u_int32_t platform_get_security_epoch(void)
{
#if WITH_IMAGE4
	return chipid_get_minimum_epoch();
#else
	u_int32_t build_epoch = PLATFORM_SECURITY_EPOCH;
	u_int32_t minimum_epoch = chipid_get_minimum_epoch();

	if (minimum_epoch < build_epoch) minimum_epoch = build_epoch;

	return minimum_epoch;
#endif	
}

#ifndef DEFAULT_RECOVERY_MODE_PRODUCT_ID
#define DEFAULT_RECOVERY_MODE_PRODUCT_ID 0x1281
#endif

/*
 *  platform_get_usb_product_id() - returns the usb product
 *
 *
 */
uint16_t platform_get_usb_product_id(void)
{
#if WITH_RECOVERY_MODE
	return DEFAULT_RECOVERY_MODE_PRODUCT_ID;
#else
	return 0x1226 + (platform_get_security_domain() & 3);
#endif
}

const char *platform_get_usb_product_string(void)
{
#if WITH_RECOVERY_MODE
	return "Apple Mobile Device (Recovery Mode)";
#else
	return "Apple Mobile Device (DFU Mode)";
#endif
}

// gUSBSerialNumber Format: "CPID:xxxx CPRV:xx CPFM:xx SCEP:xx BDID:xx ECID:xxxxxxxxxxxxxxxx IBFL:xx"
// optionally includes " SRTG:[<str24>]"
// optionally includes " SRNM:[xxxxxxxxxxxxxxxx]"
static char gUSBSerialNumber[127];
//					Size		iBoot		SecureROM
//	CPID	      (5 + 4)		9		9		9
//	CPRV	+ 1 + (5 + 2)		8		17		17
//	CPFM	+ 1 + (5 + 2)		8		25		25
//	SCEP	+ 1 + (5 + 2)		8		33		33
//	BDID	+ 1 + (5 + 2)		8		41		41
//	ECID	+ 1 + (5 + 16)		22		63		63
//	IBFL	+ 1 + (5 + 2)		8		71		71
//	SRTG	+ 1 + (5 + 2 + 24)	32				103
//	SRNM	+ 1 + (5 + 2 + 16)	24		95
//	nul	+ 1     		1 		96		104

static u_int32_t gUSBSerialNumberStatus;

const char *platform_get_usb_serial_number_string(bool full)
{
	char		srnm_buf[1 + (5 + 2 + 16) + 1];
	char		imei_buf[1 + (5 + 2 + 16) + 1];
	char		srtg_buf[1 + (5 + 2 + 24) + 1];
	int		srnm_size = 0, imei_size = 0;

	// Return if the request string has already been generated
	if (gUSBSerialNumberStatus > 1) return gUSBSerialNumber;
	if (!full && (gUSBSerialNumberStatus > 0)) return gUSBSerialNumber;

	// Set the status based on the requested string
	gUSBSerialNumberStatus = full ? 2 : 1;

	srtg_buf[0] = '\0';
	srnm_buf[srnm_size] = '\0';
	imei_buf[imei_size] = '\0';

	snprintf(gUSBSerialNumber, sizeof(gUSBSerialNumber),
		 "CPID:%04X CPRV:%02X CPFM:%02X SCEP:%02X BDID:%02X ECID:%016llX IBFL:%02X",
		 platform_get_chip_id(), platform_get_chip_revision(), platform_get_fuse_modes(),
		 platform_get_security_epoch(), platform_get_board_id(), platform_get_ecid_id(),
		 platform_get_iboot_flags());

#if APPLICATION_SECUREROM
	snprintf(srtg_buf, sizeof(srtg_buf), " SRTG:[%s]", build_tag_string);
	strlcat(gUSBSerialNumber, srtg_buf, sizeof(gUSBSerialNumber));
#endif

#if WITH_SYSCFG
	strlcpy(srnm_buf, " SRNM:[", sizeof(srnm_buf));
	srnm_size = syscfgCopyDataForTag('SrNm', (u_int8_t *)(srnm_buf + 7), 16);
	if (srnm_size > 0) {
		srnm_buf[7 + srnm_size] = '\0';
		strlcat(srnm_buf, "]", sizeof(srnm_buf));
		strlcat(gUSBSerialNumber, srnm_buf, sizeof(gUSBSerialNumber));
	}
#endif

	return gUSBSerialNumber;
}

// gUSBMoreOther Format: ""
// optionally includes " NONC:xxxxxxxxxxxxxxxx"
// optionally includes " SNON:xxxxxxxxxxxxxxxx"
static char gUSBMoreOther[127];
// SHA1:
//					Size
//	NONC	+ 1 + (5 + (2*20))	 46
//	SNON	+ 1 + (5 + (2*20))	 46
//	Total				 92
//
// SHA2-384:
//					Size
//	NONC	+ 1 + (5 + (2*32))	 70
//	SNON	+ 1 + (5 + (2*20))	 46
//	Total				116

static u_int32_t gUSBMoreOtherStatus;

const char *platform_get_usb_more_other_string(bool full)
{
	u_int64_t	nonc;
	u_int8_t	nonc_hash[HASH_OUTPUT_SIZE];
	char		nonc_buf[1 + (5 + (2 * NONCE_HASH_OUTPUT_SIZE)) + 1];
	u_int8_t	i;

	// Return if the request string has already been generated
	if (gUSBMoreOtherStatus > 1) return gUSBMoreOther;
	if (!full && (gUSBMoreOtherStatus > 0)) return gUSBMoreOther;

	// Set the status based on the requested string
	gUSBMoreOtherStatus = full ? 2 : 1;

	nonc_buf[0] = '\0';

	gUSBMoreOther[0] = '\0';

	if (full) {
		nonc = platform_get_nonce();
		hash_calculate((const void *)&nonc, sizeof(nonc), (void *)nonc_hash, sizeof(nonc_hash));

		strlcpy(nonc_buf, " NONC:", sizeof(nonc_buf));
		for(i = 0; (i < NONCE_HASH_OUTPUT_SIZE) && (i < HASH_OUTPUT_SIZE); i++)
			snprintf((nonc_buf + 1 + 5 + (i * 2)), (sizeof(nonc_buf) - 1 - 5 - (i * 2)), "%02X", nonc_hash[i]);
		strlcat(gUSBMoreOther, nonc_buf, sizeof(gUSBMoreOther));

#if WITH_HW_SEP
		u_int8_t snon[SEP_NONCE_SIZE];
		if (platform_get_sep_nonce(snon) == 0) {

			bzero(nonc_buf, sizeof(nonc_buf));
			strlcpy(nonc_buf, " SNON:", sizeof(nonc_buf));
			for(i = 0; (i < SEP_NONCE_SIZE); i++)
				snprintf((nonc_buf + 1 + 5 + (i * 2)), (sizeof(nonc_buf) - 1 - 5 - (i * 2)), "%02X", snon[i]);
			strlcat(gUSBMoreOther, nonc_buf, sizeof(gUSBMoreOther));
		}
#endif
	}

	// dprintf(DEBUG_INFO, "%s, len=%ld\n", gUSBMoreOther, strlen(gUSBMoreOther));

	return gUSBMoreOther;
}

u_int64_t platform_consume_nonce(void)
{
	uint64_t	nonce = 0;
	const char*	how = NULL;

#if WITH_EFFACEABLE
	if (effaceable_consume_nonce(&nonce)) {
		how = "effaced";
	}
#elif WITH_NVRAM && WITH_ENV
	if (mobile_ap_nonce_consume_nonce(&nonce)) {
		how = "consumed";
	}
#endif

#if WITH_RANDOM
	if (!how) {
		uint8_t	*nonce_bytes = (uint8_t *)&nonce;
		int result = random_get_bytes(nonce_bytes, sizeof(nonce));
		RELEASE_ASSERT(result == 0);
		if (result == 0) {
			how = "generated";
		}
	}
#endif

	if (!how) {
		nonce = 0;
		how = "zeroed";
	}

#if DEBUG_BUILD
	dprintf(DEBUG_INFO, "boot-nonce (%s): 0x%016llX\n", how, nonce);
#endif

	return nonce;
}

size_t platform_get_display_memory_size(void)
{
	size_t size = 0;

	struct display_info info;

	memset(&info, 0, sizeof(info));
	display_get_info(&info);

	if ((addr_t)info.framebuffer != 0)
		size = (uintptr_t)PANIC_BASE - (uintptr_t)info.framebuffer;
#ifdef PURPLE_GFX_MEMORY_LEN
	if (size < PURPLE_GFX_MEMORY_LEN)
		size = PURPLE_GFX_MEMORY_LEN;
#endif
	
	return (size);
}
