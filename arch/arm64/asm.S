/*
 * Copyright (c) 2011-2012, 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <arch/arm64/proc_reg.h>

	.text
	.align	2
	.globl	_arm_read_sctlr
_arm_read_sctlr:
	mrs	x0, SCTLR_ELx
	ret

	.text
	.align	2
	.globl	_arm_write_sctlr
_arm_write_sctlr:
	msr	SCTLR_ELx, x0
	dsb	sy
	isb	sy
	// The ifdef should match the ifdef in arch_cpu_quiesce()
#if CPU_APPLE_CYCLONE
	// Don't allow MMU or WXN to be disabled. Placing the check after the msr
	// means that a ROP attack will fail unless an attacker can trigger an
	// interrupt between the msr and check
	mov	x1, x0
	orr	x0, x0, #SCTLR_M_ENABLED
	orr	x0, x0, #SCTLR_WXN_ENABLED
	cmp	x0, x1
	b.ne	_arm_write_sctlr
#endif
	ret

#if WITH_EL3
	.text
	.align	2
	.globl	_arm_write_scr
_arm_write_scr:
	msr	SCR_EL3, x0
	isb	sy
	ret
#endif

#if WITH_L2_AS_RAM
	.text
	.align	2
	.globl	_arm_read_l2_cramconfig
_arm_read_l2_cramconfig:
	mrs	x0, s3_3_c15_c7_0
	ret
#endif

	.text
	.align	2
	.globl	_arm_write_mair
_arm_write_mair:
	msr	MAIR_ELx, x0
	isb	sy
	ret

	.text
	.align	2
	.globl	_arm_write_tcr
_arm_write_tcr:
	msr	TCR_ELx, x0
	isb	sy
	ret

	.text
	.align	2
	.globl	_arm_write_ttbr0
_arm_write_ttbr0:
	msr	TTBR0_ELx, x0
	isb	sy
	ret


	.text
	.align	2
	.globl	_arm_read_cpacr
_arm_read_cpacr:
	mrs	x0, CPACR_EL1
	ret

	.text
	.align	2
	.globl	_arm_write_cpacr
_arm_write_cpacr:
	msr	CPACR_EL1, x0
	isb	sy
	ret

	.text
	.align	2
	.globl	_arm_read_cntp_ctl
_arm_read_cntp_ctl:
	isb	sy
	mrs	x0, CNTP_CTL_EL0
	ret

	.text
	.align	2
	.globl	_arm_write_cntp_ctl
_arm_write_cntp_ctl:
	msr	CNTP_CTL_EL0, x0
	isb	sy
	ret

	.text
	.align	2
	.globl	_arm_read_cntpct
_arm_read_cntpct:
	isb	sy
	mrs	x0, CNTPCT_EL0
	ret

	.text
	.align	2
	.globl	_arm_write_cntp_tval
_arm_write_cntp_tval:
	msr	CNTP_TVAL_EL0, x0
	isb	sy
	ret

	.text
	.align	2
	.globl	_arm_flush_tlbs
_arm_flush_tlbs:
	dsb	sy
#if WITH_EL3
	tlbi	alle3
#else
	tlbi	vmalle1
#endif
	dsb	sy
	isb	sy
	ret

	.text
	.align	2
	.globl	_arm_invalidate_icache
_arm_invalidate_icache:
	ic	iallu
	dsb	sy
	isb	sy
	ret

	.text
	.align	2
	.globl	_arm_invalidate_dcache_line
_arm_invalidate_dcache_line:
	ret

	.text
	.align	2
	.globl	_arm_clean_dcache_line
_arm_clean_dcache_line:
	ret

	.text
	.align	2
	.globl	_arm_clean_invalidate_dcache
_arm_clean_invalidate_dcache:
	ret

	.text
	.align	2
	.globl	_arm_clean_dcache
_arm_clean_dcache:
	ret

	.text
	.align	2
	.globl	_arm_invalidate_dcache
_arm_invalidate_dcache:
	ret

	.text
	.align	2
	.globl	_arm_clean_invalidate_dcache_line
_arm_clean_invalidate_dcache_line:
	dc	civac, x0
	ret

	.text
	.align	2
	.globl	_arm_drain_write_buffer
_arm_drain_write_buffer:
	ret

	.text
	.align	2
	.globl	_arm_memory_barrier
_arm_memory_barrier:
	dmb	sy
	ret

	.text
	.align	2
	.globl	_arm_vtop
_arm_vtop:
	mrs	x2, DAIF				// Load current DAIF
	msr	DAIFSet, #(DAIFSC_IRQF | DAIFSC_FIQF)	// Disable IRQ
	isb	sy
	mov	x9, x0					// Saved address
	at	s1e3r, x0				// Translation Stage 1 EL1
	isb	sy
	mrs	x1, PAR_EL1				// Read result
	tst	x1, #1					// Test Translation not valid
	cset	x0, eq
	b.ne	L_arm_vtop_ret
	movk	x3, #0xf000
	movk	x3, #0xffff, lsl #16
	movk	x3, #0xffff, lsl #32
	and	x1, x1, x3				// Get physical address base
	and	x0, x9, #0xfff
	orr	x0, x1, x0				// Add page offset
L_arm_vtop_ret:
	msr	DAIF, x2				// Restore interrupt state, return
	isb	sy
	ret

	.text
	.align	2
	.globl	_arm_disable_async_aborts
_arm_disable_async_aborts:
	msr	DAIFSet, #(DAIFSC_ASYNCF)
	isb	sy
	ret

	.text
	.align	2
	.globl	_arm_enable_async_aborts
_arm_enable_async_aborts:
	msr	DAIFClr, #(DAIFSC_ASYNCF)
	isb	sy
	ret

	.text
	.align	2
	.globl	_arch_disable_ints
_arch_disable_ints:
	msr	DAIFSet, #(DAIFSC_IRQF | DAIFSC_FIQF)
	isb	sy
	ret

	.text
	.align	2
	.globl	_arch_enable_ints
_arch_enable_ints:
	msr	DAIFClr, #(DAIFSC_IRQF | DAIFSC_FIQF)
	isb	sy
	ret

	.text
	.align	2
	.globl	_arch_halt
_arch_halt:
	dsb	sy
	isb	sy
	wfi
	ret

	.text
	.align	2
	.globl	_arch_spin
_arch_spin:
	hint	0x45	// make fastsim drop to debugger (nop on hardware)
	b	.
