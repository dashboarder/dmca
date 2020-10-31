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
#ifndef __PLATFORM_H
#define __PLATFORM_H

/* Define a macro to construct an include path for a sub-platform file.
 * Example: #include SUB_PLATFORM_HEADER(pinconfig)
 *          (where PLATFORM_NAME_UNQUOTED is s5l8999x)
 * Expands: #include <platform/pinconfig_s5l8999x.h>
 */
#define NOQUOTE(x) x
#define COMBINE3(a,b,c) NOQUOTE(a)NOQUOTE(b)NOQUOTE(c)
#define COMBINE5(a,b,c,d,e) NOQUOTE(a)NOQUOTE(b)NOQUOTE(c)NOQUOTE(d)NOQUOTE(e)
#define SUB_PLATFORM_HEADER(x) <COMBINE5(platform/,x,_,PLATFORM_NAME_UNQUOTED,.h)>
#define SUB_PLATFORM_SOC_HEADER(x) <COMBINE5(platform/soc/,x,_,PLATFORM_NAME_UNQUOTED,.h)>
#define SUB_PLATFORM_SPDS_HEADER(x) <COMBINE5(soc/,PLATFORM_SPDS_CHIP_REV,/module/,x,.h)>
#define SUB_PLATFORM_TARGET_HEADER(x) <COMBINE5(target/,x,_,PLATFORM_NAME_UNQUOTED,.h)>
#define SUB_PLATFORM_TUNABLE_HEADER(x) <COMBINE5(platform/soc/,PLATFORM_NAME_UNQUOTED,/,x,.h)>

#ifndef __ASSEMBLY__

#include <sys/types.h>
#include <sys/boot.h>
#include <sys/security.h>


__BEGIN_DECLS

int platform_early_init(void);
int platform_init(void);
int platform_late_init(void);
int platform_init_hwpins(void);
int platform_init_internal_mem(void);
int platform_init_mainmem(bool resume);
void platform_init_mainmem_map(void);
int platform_init_display(void);
int platform_init_display_mem(addr_t *base, size_t *size);
int platform_init_setup_clocks(void);
int platform_init_power(void);
int platform_init_mass_storage(void);
int platform_init_mass_storage_panic(void);
int platform_init_nmi(void (* handler)(void *), void *arg);
void platform_mmu_setup(bool resume);
int platform_init_usb();

int platform_debug_init(void); /* called whenever the system goes into debug mode (command prompt) */

void platform_poweroff(void) __noreturn; /* do whatever is needed to power off the device */
void platform_reset(bool panic) __noreturn;
void platform_system_reset(bool panic) __noreturn;
void platform_halt(void);
void platform_deep_idle(void);
void platform_watchdog_tickle(void);

void platform_not_supported(void) __noreturn;

/*
 * Cache management operations.
 *
 * A length of zero indicates that the entire cache should be the target of the operation.
 */
#define CACHE_CLEAN		(1<<0)
#define CACHE_INVALIDATE	(1<<1)
#define CACHE_DRAIN_WRITE_BUFFER (1<<2)
#define CACHE_PANIC		(1<<3)	/* do cache operations appropriate for panic path */
#define CACHE_ICACHE		(1<<16)	/* DCACHE is the default */
void platform_cache_operation(int operation, void *address, u_int32_t length);
void platform_memory_barrier(void);

int platform_quiesce_hardware(enum boot_target target);
int platform_quiesce_display(void);
int platform_bootprep(enum boot_target target);
void platform_set_boot_clocks(enum boot_target target);

int platform_update_device_tree(void);

bool platform_get_raw_production_mode(void);
bool platform_get_current_production_mode(void);
void platform_clear_production_mode();
bool platform_get_secure_mode(void);
u_int32_t platform_get_fuse_modes(void);
u_int32_t platform_get_iboot_flags(void);
u_int32_t platform_get_security_domain(void);
#define kPlatformSecurityDomainManufacturing	0
#define kPlatformSecurityDomainDarwin		1
u_int32_t platform_get_security_epoch(void);
u_int32_t platform_get_hardware_epoch(void);
u_int64_t platform_get_nonce(void);
int32_t platform_get_sep_nonce(u_int8_t *nonce);
bool platform_get_core_idle_enable(void);
bool platform_get_ecid_image_personalization_required(void);
u_int32_t platform_get_board_id(void);
u_int32_t platform_get_chip_id(void);
u_int32_t platform_get_chip_revision(void);
u_int64_t platform_get_ecid_id(void);
u_int64_t platform_get_die_id(void);
uint32_t platform_get_fuse_revision(void);
u_int32_t platform_get_boot_config(void);
bool platform_get_boot_device(int32_t index, enum boot_device *boot_device,
					  u_int32_t *boot_flag, u_int32_t *boot_arg);
void platform_enable_boot_interface(bool enable, enum boot_device boot_device,
					       u_int32_t boot_arg);
int32_t platform_get_boot_manifest_hash(uint8_t *boot_manifest_hash);
int32_t platform_set_boot_manifest_hash(const uint8_t *boot_manifest_hash);
bool platform_get_mix_n_match_prevention_status(void);
void platform_set_mix_n_match_prevention_status(bool);
void platform_sidp_set_rom_manifest(const uint32_t *manifest_hash, size_t manifest_hash_size);
void platform_sidp_lock_rom_manifest(void);
bool platform_sidp_rom_manifest_locked(void);
void platform_sidp_set_boot_manifest(const uint32_t *manifest_hash, size_t manifest_hash_size);
void platform_sidp_lock_boot_manifest(void);
bool platform_sidp_boot_manifest_locked(void);
void platform_sidp_set_mix_n_match(void);
bool platform_get_lock_fuses_required(void);
u_int32_t platform_get_entropy(void);
u_int64_t platform_consume_nonce(void);
bool platform_is_pre_lpddr4(void);
u_int64_t platform_get_memory_size(void);
void platform_set_memory_info(u_int8_t manuf_id, u_int64_t memory_size);
void platform_set_memory_info_with_revids(uint8_t manuf_id, uint64_t memory_size, uint8_t rev_id, uint8_t rev_id2);
u_int8_t platform_get_memory_manufacturer_id(void);
const char *platform_get_memory_manufacturer_string(void);
void platform_get_memory_rev_ids(uint8_t *rev_id, uint8_t *rev_id2);
u_int16_t platform_get_usb_vendor_id(void);
u_int16_t platform_get_usb_product_id(void);
u_int16_t platform_get_usb_device_version(void);
const char *platform_get_usb_manufacturer_string(void);
const char *platform_get_usb_product_string(void);
const char *platform_get_usb_serial_number_string(bool full);
const char *platform_get_usb_more_other_string(bool full);
u_int32_t platform_get_osc_frequency(void);
u_int32_t platform_get_spi_frequency(void);
u_int32_t platform_get_base_soc_voltage(void);
u_int32_t platform_get_base_cpu_voltage(void);
u_int32_t platform_get_base_ram_voltage(void);
int platform_get_soc_voltages(u_int32_t count, u_int32_t *voltages);
int platform_get_cpu_voltages(u_int32_t count, u_int32_t *voltages);
int platform_get_cpu_ram_voltages(u_int32_t count, u_int32_t *voltages);
int platform_get_cpu_min_voltages(u_int32_t count, u_int32_t *voltages);
int platform_get_ram_voltages(u_int32_t count, u_int32_t *voltages);
int platform_get_gpu_ram_voltages(u_int32_t count, u_int32_t *voltages);
int platform_get_gpu_voltages(u_int32_t count, u_int32_t *voltages);
int platform_convert_voltages(int buck, u_int32_t count, u_int32_t *voltages);
int platform_get_dwi_to_mv(int buck, u_int32_t dwival);
bool platform_get_usb_cable_connected(void);
void platform_set_dfu_status(bool dfu);
bool platform_get_force_dfu(void);
bool platform_get_request_dfu1(void);	// Formerly platform_get_hold_key()
bool platform_get_request_dfu2(void);	// Formerly platform_get_menu_key()
bool platform_get_dock_connect();
void platform_set_dock_attention(bool);
uint32_t platform_get_apcie_lane_cfg(void);
uint32_t platform_get_pcie_link_width(uint32_t link);
uint32_t platform_get_pcie_l1ss_ltr_threshold(void);
uint32_t platform_get_pcie_l1ss_t_common_mode(void);
const uint32_t *platform_get_default_gpio_cfg(uint32_t gpioc);
int platform_translate_key_selector(u_int32_t key_selector, u_int32_t *key_opts);
bool platform_provide_spec_blob(const void *common_name, const size_t common_name_length,
				const void *serial_number, const size_t serial_number_length,
				void **spec_blob, size_t *spec_blob_length);
const char *platform_get_product_id(void);
void platform_set_consistent_debug_root_pointer(uint32_t root);

enum { kUSB_DP = 1, kUSB_DM = 2, kUSB_NONE = 0, kUSB_DCD = 3, kUSB_CP1 = 4, kUSB_CP2 = 5 };
bool platform_set_usb_brick_detect(int);

void platform_disable_keys(u_int32_t gids, u_int32_t uids);
bool platform_keys_disabled(u_int32_t gid, u_int32_t uid);

void platform_demote_production();

#define kPerformanceHigh	0
#define kPerformanceMedium	1
#define kPerformanceLow		2
#define kPerformanceMemory	3
#define kPerformanceMemoryFull	kPerformanceMemory
#define kPerformanceMemoryLow	4
#define kPerformanceMemoryMid1	5
#define kPerformanceMemoryMid2	6
#define kPerformanceMemoryMid	kPerformanceMemoryMid2

u_int32_t platform_set_performance(u_int32_t performance_level);
uintptr_t platform_map_pci_to_host_addr(uint64_t addr);
uint64_t platform_map_host_to_pci_addr(uintptr_t addr);
void platform_register_pci_busses(void);
void platform_setup_default_environment(void);

/* EmbeddedIOP platform interfaces */
void platform_init_iop_doorbell(void (* handler)(void *arg), void *arg);
void platform_ring_host_doorbell(void);
void platform_mask_doorbell(void);
void platform_unmask_doorbell(void);

#if SUPPORT_SLEEP
#define SLEEP_MAGIC_NOT_ASLEEP	0xffffffff
#define SLEEP_MAGIC_WAKEY_WAKEY	0
extern u_int32_t	arch_sleep_magic;
void platform_wakeup(void);
#endif
void platform_sleep(void);

// Valid flags for Platform Scratch Memory
#define kPlatformScratchFlagBootStrap			(1 << 0)
#define kPlatformScratchFlagNonce			(1 << 1)
#define kPlatformScratchFlagMemoryInfo			(1 << 2)	// current allocation - lower 16 bits: memory size , top 4 bits: vendor id, remaining 12 bits: reserved
									// this doesn't apply to H4P. For H4P: all 32 bits allocated for memory size
#define kPlatformScratchFlagVerifyManifestHash		(1 << 3)	// per Image4 spec, section 2.7.3: tells next stage if its required to verify manifest hash or not
#define kPlatformScratchFlagObjectManifestHashValid	(1 << 4)	// tells next stage if a valid object manifest hash is passed forward

void platform_asynchronous_exception();

void platform_adjust_memory_layout(void);

int32_t	platform_restore_system(void);

typedef enum {
	kMemoryRegion_StorageProcessor,
	kMemoryRegion_ConsistentDebug,
	kMemoryRegion_SleepToken,
	kMemoryRegion_DramConfig,
	kMemoryRegion_Panic,
	kMemoryRegion_Display,
	kMemoryRegion_Heap,
	kMemoryRegion_Stacks,
	kMemoryRegion_PageTables,
	kMemoryRegion_iBoot,
	kMemoryRegion_Kernel,
	kMemoryRegion_Monitor,
	kMemoryRegion_SecureProcessor,
	kMemoryRegion_AOP,
	kMemoryRegion_Reconfig,
	kMemoryRegion_NumberOfRegions,
} memory_region_type_t;

uintptr_t platform_get_memory_region_base(memory_region_type_t region);
uintptr_t platform_get_memory_region_base_optional(memory_region_type_t region);
size_t platform_get_memory_region_size(memory_region_type_t region);
size_t platform_get_memory_region_size_optional(memory_region_type_t region);
size_t platform_get_display_memory_size(void);

void platform_dockfifo_access_enable(uint32_t enable_flags);

void *platform_get_boot_trampoline(void);


#if PRODUCT_IBEC || PRODUCT_IBOOT
// Breadcrumbing API's:

static inline void platform_init_breadcrumbs(void) // Initializes breadcrumbing subsystem. Calling multiple times has no effect. All other breadcrumb API's are no-op until initialization.
{
	extern void _platform_init_breadcrumbs_internal(void);

	_platform_init_breadcrumbs_internal();
}
static inline void platform_record_breadcrumb(const char* key, const char* val) // Record a key-value breadcrumb
{
	extern void _platform_record_breadcrumb_internal(const char* key, const char* val);

	_platform_record_breadcrumb_internal(key, val);
}
static inline void platform_record_breadcrumb_int(const char* key, int val) // Record an key-integer breadcrumb
{
	extern void _platform_record_breadcrumb_int_internal(const char* key, int val);
	_platform_record_breadcrumb_int_internal(key, val);
}
static inline void platform_record_breadcrumb_marker(const char* val) // Records a string marker breadcrumb
{
	extern void _platform_record_breadcrumb_marker_internal(const char* val);
	_platform_record_breadcrumb_marker_internal(val);
}
static inline void platform_commit_breadcrumbs(const char* envvar) // Commits the breadcrumb to a NVRAM variable specified by envvar
{
	extern void _platform_commit_breadcrumbs_internal(const char* envvar);
	_platform_commit_breadcrumbs_internal(envvar);
}
#else
// Breadcrumbing stubs for unrelated boot stages
static inline void platform_init_breadcrumbs(void) {}
static inline void platform_record_breadcrumb(const char* key __unused, const char* val __unused) {}
static inline void platform_record_breadcrumb_int(const char* key __unused, int val __unused) {}
static inline void platform_record_breadcrumb_marker(const char* val __unused) {}
static inline void platform_commit_breadcrumbs(const char* envvar __unused) {}
#endif



__END_DECLS

#endif // __ASSEMBLY__
#endif
