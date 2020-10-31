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
#include <drivers/aes.h>
#include <drivers/sha1.h>
#include <lib/libc.h>
#include <lib/devicetree.h>
#include <lib/ramdisk.h>
#include <platform.h>
#include <platform/memmap.h>
#include <sys/security.h>

#define RELEASE_SECURITY_MODE	(kSecurityModeGIDKeyAccess | kSecurityModeUIDKeyAccess | kSecurityModeProdCertAccess | kSecurityStatusSystemTrusted)
#define SECRET_SECURITY_MODE	(RELEASE_SECURITY_MODE | kSecurityModeKDPEnabled)

// XXX Remove this once image4 transition for older platforms/targets is complete.
#if !WITH_IMAGE4
#define DEVELOPMENT_SECURITY_MODE	(RELEASE_SECURITY_MODE | kSecurityModeExUntrust)
#else
#define DEVELOPMENT_SECURITY_MODE	(RELEASE_SECURITY_MODE)
#endif
#define DEBUG_SECURITY_MODE		(DEVELOPMENT_SECURITY_MODE | kSecurityModeKDPEnabled | kSecurityModeDebugCmd | kSecurityModeMemAccess | kSecurityModeHWAccess)

#if RELEASE_BUILD
#define DEFAULT_SECURITY_MODE	RELEASE_SECURITY_MODE
#endif
#if SECRET_BUILD
#define DEFAULT_SECURITY_MODE	SECRET_SECURITY_MODE
#endif
#if DEVELOPMENT_BUILD
#define DEFAULT_SECURITY_MODE	DEVELOPMENT_SECURITY_MODE
#endif
#if DEBUG_BUILD
#define DEFAULT_SECURITY_MODE	DEBUG_SECURITY_MODE
#endif

#ifndef DEFAULT_SECURITY_MODE
#error Security: Unknown build style
#endif

#define CLEAR_MEM_CHUNK_SIZE	(256 * 1024 * 1024)

static security_mode_t security_mode;

static addr_t insecure_memory_start;
static addr_t insecure_memory_end;

#if !DISABLE_CLEAR_MEMORY
static bool secure_memory_cleared;
#endif

#if WITH_IMAGE4
static u_int8_t security_boot_manifest_hash[HASH_OUTPUT_SIZE];
static bool security_environment_consolidated;
#endif // WITH_IMAGE4

static void security_clear_mem_in_chunks(char *dst, size_t count);

int security_init(bool clear_memory)
{
	security_mode = DEFAULT_SECURITY_MODE;

	if (platform_get_secure_mode()) {
		security_mode |= kSecurityStatusSecureBoot;
	} else {
#if APPLICATION_SECUREROM
	// SecureROM allows untrusted execution if secure_mode is false
	security_mode |= kSecurityModeExUntrust;
#endif
	}

	if (!platform_get_current_production_mode()) {
		security_mode |= kSecurityModeDevCertAccess;
		security_mode |= kSecurityModeKDPEnabled;
	}

	if (platform_get_lock_fuses_required()) {
		security_mode |= kSecurityOptionLockFuses;
	}

	insecure_memory_start = -1;
	insecure_memory_end = 0;

#if !DISABLE_CLEAR_MEMORY
	if (clear_memory) {
		/* Break up the memory region into chunks to keep watchdog alive in between the clearings */
		security_clear_mem_in_chunks((char *)INSECURE_MEMORY_BASE, INSECURE_MEMORY_SIZE);

#ifdef SECURE_MEMORY_SIZE
		if ((SECURE_MEMORY_SIZE != 0) && !secure_memory_cleared) {
		/* Break up the memory region into chunks to keep watchdog alive in between the clearings */
		security_clear_mem_in_chunks((char *)SECURE_MEMORY_BASE, SECURE_MEMORY_SIZE);
		}
#endif

		secure_memory_cleared = true;
	}
#endif

	security_protect_memory((void *)INSECURE_MEMORY_BASE, INSECURE_MEMORY_SIZE, false);

#if WITH_RAMDISK
	ramdisk_init();
#endif

#if WITH_DEVICETREE
	dt_init();
#endif

	return 0;
}

bool security_allow_modes(security_mode_t modes)
{
	bool result = (modes & security_mode) == modes;

	if (result && ((modes & kSecurityModeExUntrust) != 0))
		security_set_untrusted();

	return result;
}

bool security_validate_image(security_image_t image_validity)
{
	// Check if untrusted is allowed.
	if ((image_validity == kSecurityImageUntrusted) && !security_allow_modes(kSecurityModeExUntrust)) {
		image_validity = kSecurityImageInvalid;
	}

	return image_validity != kSecurityImageInvalid;
}

bool security_allow_memory(const void *address, size_t length)
{
	addr_t addr = (addr_t)address, last = addr + length;
	bool allow = false;

	if (security_allow_modes(kSecurityModeMemAccess))
		return true;

	// Disallow wrap-around
	if (last <= addr)
		return false;

	if ((insecure_memory_start <= addr) && (last <= insecure_memory_end))
		allow = true;

	return allow;
}

void security_protect_memory(const void *address, size_t length, bool protect)
{
	addr_t addr = (addr_t)address, last = addr + length;

	if (security_allow_modes(kSecurityModeMemAccess))
		return;

	if (protect) {
		insecure_memory_start = -1;
		insecure_memory_end = 0;
	} else {
		if (addr < insecure_memory_start)
			insecure_memory_start = addr;
		if (insecure_memory_end < last)
			insecure_memory_end = last;
	}
}

void security_enable_kdp(void)
{
	if ((security_mode & kSecurityModeKDPEnabled) != 0)
		return;

	security_allow_modes(kSecurityModeExUntrust);
}

void security_set_untrusted(void)
{
	security_mode &= ~(kSecurityModeGIDKeyAccess | kSecurityModeUIDKeyAccess | kSecurityStatusSystemTrusted);
	security_mode |= kSecurityModeKDPEnabled;
}

void security_set_production_override(bool override)
{
	security_mode &= ~kSecurityOptionClearProduction;
	if (override)security_mode |= kSecurityOptionClearProduction;
}

bool security_get_production_override(void)
{
	return (security_mode & kSecurityOptionClearProduction) != 0;
}

#if WITH_IMAGE4

bool security_get_effective_production_status(bool with_demotion)
{
	bool device_production_status = platform_get_current_production_mode();

	return (device_production_status ? (device_production_status ^ with_demotion) : false);
}

void security_set_boot_manifest_hash(u_int8_t *boot_manifest_hash)
{
	security_mode &= ~kSecurityOptionBootManifestHashValid;

	if (boot_manifest_hash == NULL) {
		security_mode &= ~kSecurityOptionBootManifestHashValid;
	} else {
		security_mode |= kSecurityOptionBootManifestHashValid;
		memcpy((void *)security_boot_manifest_hash, (const void *)boot_manifest_hash, sizeof(security_boot_manifest_hash));
	}
}

bool security_get_boot_manifest_hash(u_int8_t *boot_manifest_hash, u_int32_t hash_length)
{
	if (security_mode & kSecurityOptionBootManifestHashValid) {
		RELEASE_ASSERT(boot_manifest_hash != NULL);
		RELEASE_ASSERT(hash_length == sizeof(security_boot_manifest_hash));
		memcpy((void *)boot_manifest_hash, (const void *)security_boot_manifest_hash, hash_length);
		return true;
	} else {
		return false;
	}
}

void security_set_mix_n_match_prevention_status(bool mix_n_match_enforced)
{
	security_mode &= ~kSecurityOptionMixNMatchPrevented;
	if (mix_n_match_enforced)
		security_mode |= kSecurityOptionMixNMatchPrevented;
}

bool security_get_mix_n_match_prevention_status(void)
{
	return (security_mode & kSecurityOptionMixNMatchPrevented);
}

void security_set_lock_fuses(void)
{
	security_mode |= kSecurityOptionLockFuses;
}

bool security_get_lock_fuses(void)
{
	return (security_mode & kSecurityOptionLockFuses);
}

bool security_restore_environment(void)
{
	uint8_t boot_manifest_hash[HASH_OUTPUT_SIZE];

	if (security_environment_consolidated)
		return false;

	security_set_mix_n_match_prevention_status(platform_get_mix_n_match_prevention_status());

	if (platform_get_boot_manifest_hash((u_int8_t *)boot_manifest_hash) == 0)
		security_set_boot_manifest_hash((u_int8_t *)boot_manifest_hash);
	else
		security_set_boot_manifest_hash(NULL);

	if (security_get_production_override())
		security_set_production_override(false);

	if ((security_mode & (kSecurityModeGIDKeyAccess | kSecurityModeUIDKeyAccess | kSecurityStatusSystemTrusted)) == 0)
		security_mode |= (kSecurityModeGIDKeyAccess | kSecurityModeUIDKeyAccess | kSecurityStatusSystemTrusted);

	return true;
}

bool security_consolidate_environment(void)
{
	/* demote production status if requested */
	if (security_get_production_override())
		platform_demote_production();

	/* disable all keys not requested */
	if (!security_allow_modes(kSecurityModeGIDKeyAccess))
		platform_disable_keys(~0, 0);
	if (!security_allow_modes(kSecurityModeUIDKeyAccess))
		platform_disable_keys(0, ~0);

	/* update next stage boot-manifest-hash, and related flags */
	if (security_mode & kSecurityOptionBootManifestHashValid)
		platform_set_boot_manifest_hash((u_int8_t *)security_boot_manifest_hash);
	else
		platform_set_boot_manifest_hash(NULL);

	platform_set_mix_n_match_prevention_status(((security_mode & kSecurityOptionMixNMatchPrevented) == kSecurityOptionMixNMatchPrevented));

	/* record environment was consolidated atleast once */
	security_environment_consolidated = true;

	return true;
}

#else // WITH_IMAGE4

bool security_consolidate_environment(void)
{
	return false;
}

#endif // WITH_IMAGE4

static const char gSleepToken[] = "iBootSleepValid";

void security_create_sleep_token(addr_t address)
{
	uint8_t key_buffer[16];

	memcpy((void *)address, gSleepToken, sizeof(gSleepToken));

#if WITH_AES
	if (!security_allow_modes(kSecurityModeUIDKeyAccess))
		return;

	memcpy(key_buffer, gSleepToken, sizeof(gSleepToken));

	/* note that we ignore any errors from AES here */
	aes_cbc_encrypt(key_buffer, key_buffer, sizeof(key_buffer), AES_KEY_TYPE_UID0, NULL, NULL);
	aes_cbc_encrypt((u_int8_t *)address, (u_int8_t *)address, sizeof(gSleepToken), AES_KEY_TYPE_USER, key_buffer, NULL);
#endif
	memset(key_buffer, 0, sizeof(key_buffer));
}

bool security_validate_sleep_token(addr_t address)
{
	uint8_t key_buffer[16];
	uint8_t token_buffer[16];
	bool result = false;

#if WITH_AES
	if (security_allow_modes(kSecurityModeUIDKeyAccess)) {
		memcpy(token_buffer, gSleepToken, sizeof(token_buffer));

		memcpy(key_buffer, gSleepToken, sizeof(key_buffer));

		/* note that we ignore any errors from AES here */
		aes_cbc_encrypt(key_buffer, key_buffer, sizeof(key_buffer), AES_KEY_TYPE_UID0, NULL, NULL);
		aes_cbc_encrypt(token_buffer, token_buffer, sizeof(token_buffer), AES_KEY_TYPE_USER, key_buffer, NULL);

		if (!memcmp_secure((void *)address, token_buffer, sizeof(token_buffer))) {
			result = true;
			goto out;
		}
	}
#endif

	if (!memcmp_secure((void *)address, gSleepToken, sizeof(gSleepToken)) && security_allow_modes(kSecurityModeExUntrust)) {
		result = true;
		goto out;
	}

out:
	memset(key_buffer, 0, sizeof(key_buffer));
	memset(token_buffer, 0, sizeof(token_buffer));

	return result;
}

/*
 * This function avoids clearing memory in one bzero call, since that might
 * take too long such that watchdog fires (if enabled) and causes a full system reset
 */
static void security_clear_mem_in_chunks(char *dst, size_t count)
{
#if APPLICATION_SECUREROM && WITH_DMA_CLEAR
	extern void platform_clear_mem_with_dma(char *dst, size_t count);
	platform_clear_mem_with_dma(dst, count);
#else // !WITH_DMA_CLEAR || !APPLICATION_SECUREROM
	size_t chunk_len;
	do {
		/* reset watchdog timer in case it's enabled */
		platform_watchdog_tickle();

		/* Only clear CLEAR_MEM_CHUNK_SIZE bytes at a time */
		chunk_len = (count > CLEAR_MEM_CHUNK_SIZE) ? CLEAR_MEM_CHUNK_SIZE : count;
		bzero((void *)dst, chunk_len);
		dst += chunk_len;
		count -= chunk_len;
	}while (count > 0);
#endif // WITH_DMA_CLEAR && APPLICATION_SECUREROM
}

#if	WITH_SIDP
#if	APPLICATION_SECUREROM

void security_sidp_seal_rom_manifest(void)
{
	// Is the boot manifest hash valid?
	if (security_mode & kSecurityOptionBootManifestHashValid) {
		// Copy the manifest hash to a word aligned buffer.
		uint32_t manifest_buffer[HASH_OUTPUT_SIZE / sizeof(uint32_t)];
		memcpy(manifest_buffer, security_boot_manifest_hash, HASH_OUTPUT_SIZE);

		// Copy the manifest hash to ROM seal registers.
		platform_sidp_set_rom_manifest(manifest_buffer, HASH_OUTPUT_SIZE);
	} else {
		// Clear the ROM seal registers.
		platform_sidp_set_rom_manifest(NULL, 0);
	}

	// Lock the ROM seal.
	platform_sidp_lock_rom_manifest();
}

#else	// !APPLICATION_SECUREROM

void security_sidp_seal_boot_manifest(bool require_unlocked)
{
	// The caller must have consolidated the security environment before
	// calling this function.
	RELEASE_ASSERT(security_environment_consolidated);

	// Did a prior boot stage lock the manifest?
	if (platform_sidp_boot_manifest_locked()) {
		if (require_unlocked) {
			panic("Boot manifest already locked");
		}

		// Yep, nothing to do.
		dprintf(DEBUG_INFO, "SiDP boot manifest already locked\n");
		return;
	}

	// Is the boot manifest hash valid?
	if (security_mode & kSecurityOptionBootManifestHashValid) {
		// Copy the manifest hash to a word aligned buffer.
		uint32_t manifest_buffer[HASH_OUTPUT_SIZE / sizeof(uint32_t)];
		memcpy(manifest_buffer, security_boot_manifest_hash, HASH_OUTPUT_SIZE);

		// Copy the manifest hash to the bootloader seal registers.
		platform_sidp_set_boot_manifest(manifest_buffer, HASH_OUTPUT_SIZE);
	} else {
		// Clear the bootloader seal registers.
		platform_sidp_set_boot_manifest(NULL, 0);
	}

	// Mix-n-match allowed?
	if (!security_get_mix_n_match_prevention_status()) {
		platform_sidp_set_mix_n_match();
	}

	// Lock the boot manifest.
	platform_sidp_lock_boot_manifest();
}

#endif	// APPLICATION_SECUREROM
#endif	// WITH_SIDP

