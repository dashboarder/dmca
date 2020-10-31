/*
 * Copyright (C) 2007-2011 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __ARM_H
#define __ARM_H

#ifndef __ASSEMBLER__

#include <compiler.h>
#include <non_posix_types.h>
#include <stdbool.h>
#include <stdint.h>

__BEGIN_DECLS

unsigned int arm_read_cpsr(void) __pure;

unsigned int arm_read_main_id(void) __pure;
unsigned int arm_read_cache_id(void) __pure;
unsigned int arm_read_cache_level_id(void) __pure;
unsigned int arm_read_cache_size_selection(void) __pure;
void arm_write_cache_size_selection(unsigned int);
unsigned int arm_read_cache_size_id(void) __pure;

void arm_read_memory_model_feature_regs(unsigned int regs[4]) __pure;
void arm_read_instruction_set_attribute_regs(unsigned int regs[6]) __pure;
void arm_read_extended_feature_regs(unsigned int regs[16]) __pure;

unsigned int arm_read_cr(void) __pure;
void arm_write_cr(unsigned int cr);
unsigned int arm_read_aux_cr(void) __pure;
void arm_write_aux_cr(unsigned int cr);
unsigned int arm_read_perip_port_remap(void) __pure;
void arm_write_perip_port_remap(unsigned int val);

extern uint32_t arm_read_cp_access_cr(void) __pure;
extern void arm_write_cp_access_cr(uint32_t val);

extern void arm_fp_init(void);
extern void arm_fp_enable(bool);

extern uint32_t arm_read_fpexc(void) __pure;
extern void arm_write_fpexc(uint32_t val);

extern uint32_t arm_read_fpscr(void) __pure;
extern void arm_write_fpscr(uint32_t val);

extern void arm_init_fp_regs();

#if ARCH_ARMv7
extern uint32_t arm_read_l2_aux_cr(void) __pure;
extern void arm_write_l2_aux_cr(uint32_t val);

typedef enum {
  ARM_PMCR = 0,			/* The order here matches a jump table in asm.S */
  ARM_PMCNTENSET,
  ARM_PMCNTENCLR,
  ARM_PMOVSR,
  ARM_PMSWINC,
  ARM_PMSELR,

  ARM_PMCCNTR,
  ARM_PMXEVTYPER,
  ARM_PMXEVCNTR,

  ARM_PMUSERNR,
  ARM_PMINTENSET,
  ARM_PMINTENCLR,
} arm_pmreg_t;

extern uint32_t arm_read_pmreg(arm_pmreg_t reg) __pure;
extern void arm_write_pmreg(arm_pmreg_t reg, uint32_t val);
#endif /* ARCH_ARMv7 */

#if ARCH_ARMv8

extern void arm_write_mair(uint64_t mair);
extern void arm_write_tcr(uint64_t tcr);
extern void arm_write_ttbr0(void *ttbr0);
extern void arm_store_pte(uint64_t *ent_p, uint64_t ent);

#if WITH_L2_AS_RAM
extern uint64_t arm_read_l2_cramconfig(void);
#endif

#endif /* ARCH_ARMv8 */

void arm_mmu_init(bool resume);
void arm_mpu_init(void);

uint32_t arm_get_noncached_address(uint32_t address);

void arm_write_dar(unsigned int dar);
void arm_write_ttb(void *tt);
void arm_write_ttbcr(unsigned int ttbc);

#if WITH_MMU
#define FSR_CAUSE_MASK	((1 << 10) | (0xF << 0))
unsigned int arm_read_ifsr(void) __pure;
unsigned int arm_read_dfsr(void) __pure;
unsigned int arm_read_dfar(void) __pure;
unsigned int arm_read_ifar(void) __pure;
#else
# define arm_read_ifsr()	~0
# define arm_read_dfsr()	~0
# define arm_read_dfar()	~0
# define arm_read_ifar()	~0
#endif

#if WITH_CACHE_DEBUG
void arm_l1cache_dump(int dcache, unsigned int addr);
#else
#define arm_l1cache_dump(dcache, addr)
#endif

/*
 * Exception frame saved at the top of the exception/interrupt stacks.
 */
struct arm_exception_frame {
	uint32_t	sp;		/* saved stack pointer (r13) */
	uint32_t	lr;		/* saved link register (r14) */
	uint32_t	spsr;		/* saved cpsr */
	uint32_t	r[13];		/* r0 - r12 */
	uint32_t	pc;		/* saved pc (r15) */
};

struct arm_exception_frame64 {
	uint64_t	regs[29];	// x0-x28
	uint64_t	fp;
	uint64_t	lr;
	uint64_t	sp;		
	uint32_t	spsr;
	uint32_t	reserved0;		
	uint64_t	pc;
	uint64_t	far;
	uint32_t	esr;
	uint32_t	reserved1;		
};

extern void *irq_stack_top;		/* stacks defined in start.S */
extern void *fiq_stack_top;
extern void *exc_stack_top;
extern void *svc_stack_top;

void arm_flush_tlbs(void);

void arm_invalidate_dcache(void);
void arm_invalidate_icache(void);
void arm_clean_caches(void);
void arm_clean_dcache(void);
void arm_clean_dcache_line(addr_t addr);
void arm_clean_dcache_line_2(addr_t addr);
void arm_clean_dcache_line_3(addr_t addr);
void arm_clean_dcache_line_4(addr_t addr);
void arm_clean_dcache_line_5(addr_t addr);
void arm_clean_dcache_line_6(addr_t addr);
void arm_clean_dcache_line_7(addr_t addr);
void arm_clean_dcache_line_8(addr_t addr);
void arm_invalidate_dcache_line(addr_t addr);
void arm_invalidate_dcache_line_2(addr_t addr);
void arm_invalidate_dcache_line_3(addr_t addr);
void arm_invalidate_dcache_line_4(addr_t addr);
void arm_invalidate_dcache_line_5(addr_t addr);
void arm_invalidate_dcache_line_6(addr_t addr);
void arm_invalidate_dcache_line_7(addr_t addr);
void arm_invalidate_dcache_line_8(addr_t addr);
void arm_clean_invalidate_dcache_line(addr_t addr);
void arm_clean_invalidate_dcache_line_2(addr_t addr);
void arm_clean_invalidate_dcache_line_3(addr_t addr);
void arm_clean_invalidate_dcache_line_4(addr_t addr);
void arm_clean_invalidate_dcache_line_5(addr_t addr);
void arm_clean_invalidate_dcache_line_6(addr_t addr);
void arm_clean_invalidate_dcache_line_7(addr_t addr);
void arm_clean_invalidate_dcache_line_8(addr_t addr);
void arm_clean_invalidate_dcache(void);

void arm_enable_l1parity(void);

void arm_drain_write_buffer(void);

void arm_flush_branch_predictor(void);

void arm_init_vfp(void);
void arm_enable_vfp(void);
void arm_disable_vfp(void);

void arm_enable_fiqs(void);
void arm_disable_fiqs(void);

unsigned int arm_read_vbar(void);
void arm_write_vbar(unsigned int addr);

void arm_memory_barrier(void);

/* mmu stuff */
#ifdef ARCH_ARMv8

typedef enum {
	kARMMMUDeviceRX,
	kARMMMUDeviceR,
	kARMMMUDeviceRW,
	kARMMMUNormalRX,
	kARMMMUNormalR,
	kARMMMUNormalRW,
	kARMMMUNumExtendedAttr,
} arm_mmu_extended_attr_t;

void arm_mmu_map_range(addr_t vaddr, addr_t paddr, size_t size, arm_mmu_extended_attr_t attr);

// map  in place using the specified attributes
void arm_mmu_map_rw(addr_t addr, size_t size);
void arm_mmu_map_rx(addr_t addr, size_t size);
void arm_mmu_map_ro(addr_t addr, size_t size);
void arm_mmu_map_device_rw(addr_t addr, size_t size);
void arm_mmu_map_device_rx(addr_t addr, size_t size);
void arm_mmu_map_device_ro(addr_t addr, size_t size);

#else

typedef enum {
	kARMMMUStronglyOrdered = 0,
	kARMMMUDevice,
	kARMMMUNormal,
	kARMMMUInnerNormalOuterNoncached,
	kARMMMUWriteCombined,
	kARMMMUNormalRX,
	kARMMMUNumAttr,
} arm_mmu_attr_t;

// Section and page mapping is explicit as there is always a fixed
// granule at each level (without LPAE) in ARMv4-ARMv7.
void arm_mmu_map_section(addr_t vaddr, addr_t paddr, arm_mmu_attr_t attr, bool shared);
void arm_mmu_map_section_range(addr_t vaddr, addr_t paddr, size_t sections, arm_mmu_attr_t attr, bool shared, bool flush);

#endif

/* mpu stuff */
void mpu_init(void);
void arm_write_dprot_region_0(uint32_t base_size);
void arm_write_dprot_region_1(uint32_t base_size);
void arm_write_dprot_region_2(uint32_t base_size);
void arm_write_dprot_region_3(uint32_t base_size);
void arm_write_dprot_region_4(uint32_t base_size);
void arm_write_dprot_region_5(uint32_t base_size);
void arm_write_dprot_region_6(uint32_t base_size);
void arm_write_dprot_region_7(uint32_t base_size);
void arm_write_iprot_region_0(uint32_t base_size);
void arm_write_iprot_region_1(uint32_t base_size);
void arm_write_iprot_region_2(uint32_t base_size);
void arm_write_iprot_region_3(uint32_t base_size);
void arm_write_iprot_region_4(uint32_t base_size);
void arm_write_iprot_region_5(uint32_t base_size);
void arm_write_iprot_region_6(uint32_t base_size);
void arm_write_iprot_region_7(uint32_t base_size);
void arm_write_data_prot_register(uint32_t prot);
void arm_write_ins_prot_register(uint32_t prot);
void arm_write_cacheable_registers(uint32_t dcache, uint32_t icache);
void arm_write_bufferable_register(uint32_t buff);

/* thread id */
uint32_t arm_read_user_ro_tid(void) __pure;
uint32_t arm_read_user_rw_tid(void) __pure;
uint32_t arm_read_sup_tid(void) __pure;
void arm_write_user_ro_tid(uint32_t tid);
void arm_write_user_rw_tid(uint32_t tid);
void arm_write_sup_tid(uint32_t tid);

/* DCC based debug */
void arm_write_dcc_char(int c);
int arm_read_dcc_char(void);
int arm_read_dcc_char_nowait(void);
int arm_dcc_init(void);

#if CPU_APPLE_SWIFT
extern uint32_t swift_read_l2cerrsts(void) __pure;
extern void swift_write_l2cerrsts(uint32_t l2cerrsts);
extern uint32_t swift_read_l2cerradr(void) __pure;
extern void swift_write_l2cerradr(uint32_t l2cerradr);
#endif /* CPU_APPLE_SWIFT */

/* FP support */
#if WITH_VFP
void arm_call_fpsaved(void *arg, void (*func)(void *arg));
#endif

__END_DECLS

#endif /* __ASSEMBLER__ */

#define CPSR_MODE(_x)	((_x) & 0x1f)
#define CPSR_MODE_USER		0x10
#define CPSR_MODE_FIQ		0x11
#define CPSR_MODE_IRQ		0x12
#define CPSR_MODE_SUPERVISOR	0x13
#define CPSR_MODE_ABORT		0x17
#define CPSR_MODE_UNDEFINED	0x1b
#define CPSR_MODE_SYSTEM	0x1f

#define CPSR_STATE_THUMB	0x20

#define CPACR_CP_BITS(_cp, _bits)	((_bits) << ((_cp) * 2))
#define CPACR_CP_DENIED		0x0
#define CPACR_CP_PRIVILEGED	0x1
#define CPACR_CP_FULL		0x3

#define FPEXC_EX		(1<<31)
#define FPEXC_EN		(1<<30)
#define FPEXC_FP2V		(1<<28)

#define FPSCR_DEFAULT 		(1<<25) | (1<<24)	// DN, FZ bits

#endif
