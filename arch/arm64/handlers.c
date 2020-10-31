/*
 * Copyright (C) 2011-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <arch.h>
#include <arch/arch_task.h>
#include <arch/arm/arm.h>
#include <arch/arm64/proc_reg.h>
#include <sys.h>
#include <lib/libc.h>
#include <platform.h>

extern void platform_irq(void);

typedef enum {
	ESR_EC_UNCATEGORIZED			= 0x00,
	ESR_EC_TRAP_SIMD_FP			= 0x07,
	ESR_EC_ILLEGAL_INSTR_SET		= 0x0c,
	ESR_EC_SVC_32				= 0x11,
	ESR_EC_SVC_64				= 0x15,
	ESR_EC_MSR_TRAP				= 0x18,
	ESR_EC_IABORT_EL0			= 0x20,
	ESR_EC_IABORT_EL1			= 0x21,
	ESR_EC_PC_ALIGN				= 0x22,
	ESR_EC_DABORT_EL0			= 0x24,
	ESR_EC_DABORT_EL1			= 0x25,
	ESR_EC_SP_ALIGN				= 0x26
} esr_exception_class_t;

typedef enum {
	FSC_TRANSLATION_FAULT_L0		= 0x04,
	FSC_TRANSLATION_FAULT_L1		= 0x05,
	FSC_TRANSLATION_FAULT_L2		= 0x06,
	FSC_TRANSLATION_FAULT_L3		= 0x07,
	FSC_ACCESS_FLAG_FAULT_L1		= 0x09,
	FSC_ACCESS_FLAG_FAULT_L2		= 0x0A,
	FSC_ACCESS_FLAG_FAULT_L3		= 0x0B,
	FSC_PERMISSION_FAULT_L1			= 0x0D,
	FSC_PERMISSION_FAULT_L2			= 0x0E,
	FSC_PERMISSION_FAULT_L3			= 0x0F,
	FSC_SYNC_EXT_ABORT			= 0x10,
	FSC_ASYNC_EXT_ABORT			= 0x11,
	FSC_SYNC_EXT_ABORT_TT_L1		= 0x15,
	FSC_SYNC_EXT_ABORT_TT_L2		= 0x16,
	FSC_SYNC_EXT_ABORT_TT_L3		= 0x17,
	FSC_SYNC_PARITY				= 0x18,
	FSC_ASYNC_PARITY			= 0x19,
	FSC_SYNC_PARITY_TT_L1			= 0x1D,
	FSC_SYNC_PARITY_TT_L2			= 0x1E,
	FSC_SYNC_PARITY_TT_L3			= 0x1F,
	FSC_ALIGNMENT_FAULT			= 0x21,
	FSC_DEBUG_FAULT				= 0x22
} fault_status_t;

static const char *exception_codes[50] = {
	[0x00] = "Unknown",
	[0x0E] = "Illegal Instruction Set",
	[0x21] = "Instruction Abort",
	[0x22] = "PC Alignment",
	[0x25] = "Data Abort",
	[0x26] = "Stack Pointer Alignment",
	[0x2F] = "SError",
};

void 
arm_synchronous_exception(struct arm_exception_frame64 *frame)
{
	const char **kind;

	kind = &exception_codes[ESR_EC(frame->esr)];

	panic("ARM synchronous abort at 0x%016llx:\n"
	      " esr 0x%08x (ec: %s, iss: 0x%08x) far 0x%016llx\n"
	      " x0 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x4 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x8 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x12 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x16 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x20 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x24 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x28 0x%016llx sp 0x%016llx lr 0x%016llx spsr 0x%08x\n",
	      frame->pc, 
              frame->esr, *kind, ESR_ISS(frame->esr), frame->far,
	      frame->regs[0], frame->regs[1], frame->regs[2], frame->regs[3],
	      frame->regs[4], frame->regs[5], frame->regs[6], frame->regs[7],
	      frame->regs[8], frame->regs[9], frame->regs[10], frame->regs[11], 
	      frame->regs[12], frame->regs[13], frame->regs[14], frame->regs[15], 
	      frame->regs[16], frame->regs[17], frame->regs[18], frame->regs[19], 
	      frame->regs[20], frame->regs[21], frame->regs[22], frame->regs[23],
	      frame->regs[24], frame->regs[25], frame->regs[26], frame->regs[27],
	      frame->regs[28], frame->sp, frame->lr, frame->spsr);
}

void 
arm_serror_exception(struct arm_exception_frame64 *frame)
{
	const char **kind;

	kind = &exception_codes[ESR_EC(frame->esr)];

	platform_asynchronous_exception();

	panic("ARM SError abort at 0x%016llx:\n"
	      " esr 0x%08x (ec: %s, iss: 0x%08x) far 0x%016llx\n"
	      " x0 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x4 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x8 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x12 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x16 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x20 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x24 0x%016llx 0x%016llx 0x%016llx 0x%016llx\n"
	      " x28 0x%016llx sp 0x%016llx lr 0x%016llx spsr 0x%08x\n",
	      frame->pc, 
              frame->esr, *kind, ESR_ISS(frame->esr), frame->far,
	      frame->regs[0], frame->regs[1], frame->regs[2], frame->regs[3],
	      frame->regs[4], frame->regs[5], frame->regs[6], frame->regs[7],
	      frame->regs[8], frame->regs[9], frame->regs[10], frame->regs[11], 
	      frame->regs[12], frame->regs[13], frame->regs[14], frame->regs[15], 
	      frame->regs[16], frame->regs[17], frame->regs[18], frame->regs[19], 
	      frame->regs[20], frame->regs[21], frame->regs[22], frame->regs[23],
	      frame->regs[24], frame->regs[25], frame->regs[26], frame->regs[27],
	      frame->regs[28], frame->sp, frame->lr, frame->spsr);
}

void
arm_irq()
{
	_irq_enter_critical_section();
	platform_irq();
	_irq_exit_critical_section();
}
