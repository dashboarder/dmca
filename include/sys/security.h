/*
 * Copyright (C) 2007-2015 Apple, Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple, Inc.
 */
#ifndef __SYS_SECURITY_H
#define __SYS_SECURITY_H

#include <sys/types.h>

__BEGIN_DECLS

#define kSecurityModeExUntrust			(1 << 4)
#define kSecurityModeKDPEnabled			(1 << 5)
#define kSecurityModeDebugCmd			(1 << 8)
#define kSecurityModeMemAccess			(1 << 16)
#define kSecurityModeHWAccess			(1 << 17)
#define kSecurityModeGIDKeyAccess		(1 << 18)
#define kSecurityModeUIDKeyAccess		(1 << 19)
#define kSecurityModeDevCertAccess		(1 << 20)
#define kSecurityModeProdCertAccess		(1 << 21)
#define kSecurityOptionClearProduction		(1 << 24)
#define kSecurityOptionMixNMatchPrevented	(1 << 25)
#define kSecurityOptionBootManifestHashValid	(1 << 26)
#define kSecurityOptionLockFuses		(1 << 27)
#define kSecurityStatusSecureBoot		(1 << 28)
#define kSecurityStatusSystemTrusted		(1 << 29)

#define kSleepTokenKernelOffset			(0x80)
#define kSleepTokeniBootOffset			(0x90)

typedef u_int32_t security_mode_t;


#define kSecurityImageInvalid			(0)
#define kSecurityImageUntrusted			(1)
#define kSecurityImageTrusted			(2)

typedef u_int32_t security_image_t;

int security_init(bool clear_memory);
bool security_allow_modes(security_mode_t modes);
bool security_validate_image(security_image_t image_validity);
bool security_allow_memory(const void *address, size_t length);
void security_protect_memory(const void *address, size_t length, bool protect);
void security_enable_kdp(void);
void security_set_untrusted(void);
void security_set_production_override(bool override);
bool security_get_production_override(void);
void security_create_sleep_token(addr_t address);
bool security_validate_sleep_token(addr_t address);
bool security_get_effective_production_status(bool with_demotion);
void security_set_boot_manifest_hash(u_int8_t *boot_manifest_hash);
bool security_get_boot_manifest_hash(u_int8_t *boot_manifest_hash, u_int32_t hash_length);
void security_set_mix_n_match_prevention_status(bool mix_n_match_enforced);
bool security_get_mix_n_match_prevention_status(void);
void security_set_lock_fuses(void);
bool security_get_lock_fuses(void);
bool security_consolidate_environment(void);
bool security_restore_environment(void);
void security_sidp_seal_rom_manifest(void);
void security_sidp_seal_boot_manifest(bool require_unlocked);

__END_DECLS

#endif
