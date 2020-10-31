/*
 * Copyright (C) 2007-2010 Apple Inc. All rights reserved.
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
#include <arch.h>
#include <arch/arch_task.h>
#include <arch/arm/arm.h>
#include <sys.h>
#include <lib/libc.h>

extern void platform_irq(void);
extern void platform_fiq(void);

static const char *arm_abort_decode_status(unsigned int dfsr);
static const char *arm_decode_mode(unsigned int status_reg);
static void arm_exception_abort(struct arm_exception_frame *frame, 
				const char *kind, 
				unsigned int fsr, 
				unsigned int far) __noreturn;

static struct arm_exception_frame *arm_get_exception_frame(void) __pure;
static struct arm_exception_frame *arm_get_interrupt_frame(void) __pure;
static struct arm_exception_frame *arm_get_fiq_frame(void) __pure;
static void arm_print_frame_registers(struct arm_exception_frame *frame, const char *prefix);
static void arm_backtrace(const char *prefix, u_int32_t pc, u_int32_t fp, u_int32_t stack_base, u_int32_t stack_len);
static void arm_stack_for_status_reg(u_int32_t status_reg, u_int32_t *stack_base, u_int32_t *stack_len);

static bool exception_dump_near_pc;

#if WITH_MMU
static char status_str[32];

static const char *
arm_abort_decode_status(unsigned int fsr)
{
	/* mask bit 10 & low 4 bits for cause decoding */
	fsr &= FSR_CAUSE_MASK;

	/* ARMv4v5 architecture reference B4-20 */
	switch (fsr) {
	case 0x01:
	case 0x03:
		return("alignment error");
	case 0x04:
		return("icache operation error");
	case 0x0c:
	case 0x0e:
		return("external abort on translation");
	case 0x05:
	case 0x07:
		return("translation error");
	case 0x09:
	case 0x0b:
		return("domain violation");
	case 0x0d:
	case 0x0f:
		return("permission violation");
	case 0x08:
	case 0x0a:		/* this code is deprecated in ARMv7*/
		return("precise external abort");
	case 0x406:
		return("imprecise external abort");		/* ARMv7 */
	case 0x40a:
		return("coprocessor data abort");
	case 0x408:
		return("data cache parity error");		/* ARMv7 */
	case 0x409:
		return("instruction cache parity error");	/* ARMv7 */
	case 0x02:
		return("debug event");
	}

	snprintf(status_str, sizeof(status_str), "<unknown cause 0x%03x>", fsr);

	return(status_str);
}
#endif

static const char *
arm_decode_mode(unsigned int status_reg)
{
	switch (CPSR_MODE(status_reg)) {
	case CPSR_MODE_USER:
		return("user");
	case CPSR_MODE_FIQ:
		return("FIQ");
	case CPSR_MODE_IRQ:
		return("IRQ");
	case CPSR_MODE_SUPERVISOR:
		return("supervisor");
	case CPSR_MODE_ABORT:
		return("abort");
	case CPSR_MODE_UNDEFINED:
		return("undefined");
	case CPSR_MODE_SYSTEM:
		return("system");
	}
	return("<unknown>");
}

void
arm_data_abort(struct arm_exception_frame *frame)
{
	unsigned int fsr = arm_read_dfsr();
	unsigned int far = arm_read_dfar();

	frame->pc -= 8;
	if (fsr == 0x408) {
		arm_l1cache_dump(1, far);
	}
	arm_exception_abort(frame, "data abort", fsr, far);
}

void
arm_prefetch_abort(struct arm_exception_frame *frame)
{
	unsigned int fsr = arm_read_ifsr();
	unsigned int far = arm_read_ifar();

	frame->pc -= 4;
	if (fsr == 0x409) {
		arm_l1cache_dump(0, far);
	}
	arm_exception_abort(frame, "prefetch abort", fsr, far);
}

void
arm_undefined(struct arm_exception_frame *frame)
{
	exception_dump_near_pc = true;
	frame->pc -= (frame->spsr & CPSR_STATE_THUMB) ? 2: 4;
	arm_exception_abort(frame, "undefined instruction", ~0, frame->pc);
}

void
arm_syscall(struct arm_exception_frame *frame)
{
	exception_dump_near_pc = true;
	frame->pc -= (frame->spsr & CPSR_STATE_THUMB) ? 2: 4;
	arm_exception_abort(frame, "syscall", arm_read_ifsr(), frame->pc);
}

static void
arm_exception_abort(struct arm_exception_frame *frame, const char *kind, unsigned int fsr, unsigned int far)
{
#if WITH_MMU
	panic("ARM %s abort in %s mode at 0x%08x due to %s:\n"
	      " far 0x%08x fsr 0x%08x\n"
	      " r0 0x%08x 0x%08x 0x%08x 0x%08x\n"
	      " r4 0x%08x 0x%08x 0x%08x 0x%08x\n"
	      " r8 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n"
	      " sp 0x%08x lr 0x%08x spsr 0x%08x\n",
	      kind, arm_decode_mode(frame->spsr), frame->pc, arm_abort_decode_status(fsr), far, fsr,
	      frame->r[0], frame->r[1], frame->r[2], frame->r[3],
	      frame->r[4], frame->r[5], frame->r[6], frame->r[7],
	      frame->r[8], frame->r[9], frame->r[10], frame->r[11], frame->r[12], 
	      frame->sp, frame->lr, frame->spsr);
#else
	panic("ARM %s abort in %s mode at 0x%08x\n"
	      " r0 0x%08x 0x%08x 0x%08x 0x%08x\n"
	      " r4 0x%08x 0x%08x 0x%08x 0x%08x\n"
	      " r8 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n"
	      " sp 0x%08x lr 0x%08x spsr 0x%08x\n",
	      kind, arm_decode_mode(frame->spsr), frame->pc,
	      frame->r[0], frame->r[1], frame->r[2], frame->r[3],
	      frame->r[4], frame->r[5], frame->r[6], frame->r[7],
	      frame->r[8], frame->r[9], frame->r[10], frame->r[11], frame->r[12], 
	      frame->sp, frame->lr, frame->spsr);
#endif
}

void
arm_irq() /* __attribute__ ((naked)) */
{
#if WITH_FIQ_TIMER
	// Currently only defined on M7, M7 uses a FIQ for thetimer, and we need to hold off timer interrupts
	// until we are done running ISR code to prevent unwanted preemption (e.g. <rdar://problem/16757466>)
	arm_disable_fiqs();
#endif	
	_irq_enter_critical_section();
	platform_irq();
	_irq_exit_critical_section();
#if WITH_FIQ_TIMER	
	arm_enable_fiqs();
#endif
}

void 
arm_fiq() /* __attribute__ ((naked)) */
{
	_irq_enter_critical_section();
	platform_fiq();
	_irq_exit_critical_section();
}

/*
 * Accessors for exception frame information.
 *
 * These may return NULL if we don't believe the frame requested is currently valid.
 *
 * Note that not all possible cases of nesting are necessarily handled here; only those
 * that are likely to be interesting in the case of a fatality.
 */
static struct arm_exception_frame *
arm_get_exception_frame(void)
{
	/*
	 * As exceptions are considered fatal, we don't expect to be asked for
	 * the exception frame in any other context.
	 */
	switch (CPSR_MODE(arm_read_cpsr())) {
	case CPSR_MODE_ABORT:
	case CPSR_MODE_UNDEFINED:
		return((struct arm_exception_frame *)exc_stack_top - 1);
	default:
		return(NULL);
	}
}

static struct arm_exception_frame *
arm_get_interrupt_frame(void)
{
	switch (CPSR_MODE(arm_read_cpsr())) {
	case CPSR_MODE_ABORT:
	case CPSR_MODE_UNDEFINED:
		/*
		 * In some configurations the IRQ and exception stacks share a top,
		 * in which case we can't return the IRQ frame because the exception
		 * frame has overwritten it.
		 */
		if (&irq_stack_top == &exc_stack_top)
			return(NULL);

		/*
		 * If we took an exception while servicing an interrupt, the IRQ frame 
		 * will be valid.
		 */
		if (CPSR_MODE_IRQ != arm_get_exception_frame()->spsr)
			return(NULL);
		break;

	case CPSR_MODE_IRQ:
		/* we're in interrupt mode, so the frame is valid */
		break;

	case CPSR_MODE_FIQ:
		/* 
		 * If we took a FIQ while in interrupt mode, the IRQ frame will be
		 * valid.
		 */
		if (CPSR_MODE_IRQ != CPSR_MODE(arm_get_fiq_frame()->spsr))
			return(NULL);
		break;

	default:
		/* the interrupt frame is invalid in other modes */
		return(NULL);
	}
	return ((struct arm_exception_frame *)irq_stack_top - 1);
}

static struct arm_exception_frame *
arm_get_fiq_frame(void)
{
	switch (CPSR_MODE(arm_read_cpsr())) {
	case CPSR_MODE_ABORT:
	case CPSR_MODE_UNDEFINED:
		/*
		 * If we took an exception while servicing a FIQ, the frame 
		 * will be valid.
		 */
		if (CPSR_MODE_FIQ != CPSR_MODE(arm_get_exception_frame()->spsr))
			return(NULL);
		break;

	case CPSR_MODE_FIQ:
		/* we're in FIQ mode, so the frame is valid */
		break;
		
	default:
		/* the FIQ frame is invalid in other modes */
		return(NULL);
	}
	return((struct arm_exception_frame *)fiq_stack_top - 1);
}

/*
 * arm_print_frame_registers
 *
 * Print the registers from an exception frame.
 */
static void
arm_print_frame_registers(struct arm_exception_frame *frame, const char *prefix)
{
	dprintf(DEBUG_CRITICAL,
		"%sr0 0x%08x 0x%08x 0x%08x 0x%08x\n"
		"%sr4 0x%08x 0x%08x 0x%08x 0x%08x\n"
		"%sr8 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n"
		"%spc 0x%08x sp 0x%08x lr 0x%08x spsr 0x%08x\n",
		prefix, frame->r[0], frame->r[1], frame->r[2], frame->r[3],
		prefix, frame->r[4], frame->r[5], frame->r[6], frame->r[7],
		prefix, frame->r[8], frame->r[9], frame->r[10], frame->r[11], frame->r[12], 
		prefix, frame->pc, frame->sp, frame->lr, frame->spsr);
}

/*
 * arm_backtrace
 *
 * Print a backtrace until we run out of frames on the stack.
 */
static void
arm_backtrace(const char *prefix, u_int32_t pc, u_int32_t fp, u_int32_t stack_base, u_int32_t stack_len)
{
	u_int32_t	prev_fp;
	u_int32_t	stack_top;
	u_int32_t	*fpp;

	/* print the mode prefix and current pc for starters */
	dprintf(DEBUG_CRITICAL, "    %10s 0x%08x 0x%08x\n", prefix, pc, fp);

	/* sanity check our arguments as much as we can */
	if ((stack_base % 4) || 	/* bump these to CPU_CACHELINE_SIZE if/when stack allocation gets smarter */
	    (stack_len % 4) || 
	    (stack_len < 16) ||
	    (stack_len > 100 * 1024)) {
		dprintf(DEBUG_CRITICAL, "improbable stack, base %p size 0x%08x (task smashed?)\n", (void *)stack_base, stack_len);
		return;
	}

	/* loop unwinding frames until we leave the stack region */
	prev_fp = stack_base;
	stack_top = stack_base + stack_len;
	for (;;) {
		/*
		 * Frame pointer must be aligned, must be above the previous frame, must not point outside the stack.
		 *
		 * It would be nice to have a sentinel to allow differentiating clean termination from a smashed
		 * stack, but that would require support in the interrupt and exception handlers.
		 */
		if ((fp % 4) || (fp <= prev_fp) || (fp > stack_top))
			return;

		/*
		 * Looks like it's inside the stack, so go ahead and deref it.
		 */
		fpp = (u_int32_t *)fp;
		prev_fp = fp;
		fp = fpp[0];

		/*
		 * Print the return address for this frame.
		 */
		dprintf(DEBUG_CRITICAL, "               0x%08x 0x%08x\n", fpp[1], fp);
	}	
}

/*
 * arm_stack_for_status_reg
 *
 * Returns the base and size of the stack for the given status register value if it's "special".
 */
static void
arm_stack_for_status_reg(u_int32_t status_reg, u_int32_t *stack_base, u_int32_t *stack_len)
{
	*stack_base = 0;
	*stack_len = 0;

#ifdef STACK_BASE
	*stack_base = (void *)STACK_BASE;
	switch (CPSR_MODE(status_reg)) {
	case CPSR_MODE_FIQ:
		*stack_len = FIQ_STACK_SIZE;
		break;
	case CPSR_MODE_IRQ:
		*stack_len = IRQ_STACK_SIZE;
		break;
	case CPSR_MODE_ABORT:
	case CPSR_MODE_UNDEFINED:
		*stack_len = EXCEPTION_STACK_SIZE;
		break;

	}
#else
	switch (CPSR_MODE(status_reg)) {
	case CPSR_MODE_FIQ:
		*stack_base = (u_int32_t)fiq_stack_top - FIQ_STACK_SIZE;
		*stack_len = FIQ_STACK_SIZE;
		break;
	case CPSR_MODE_IRQ:
		*stack_base = (u_int32_t)irq_stack_top - IRQ_STACK_SIZE;
		*stack_len = IRQ_STACK_SIZE;
		break;
	case CPSR_MODE_ABORT:
	case CPSR_MODE_UNDEFINED:
		*stack_base = (u_int32_t)exc_stack_top - EXCEPTION_STACK_SIZE;
		*stack_len = EXCEPTION_STACK_SIZE;
		break;
	}
#endif
}

/*
 * arch_backtrace_current_task
 *
 * Print a backtrace for the current task.
 *
 * We support being called with the following context nesting:
 *
 * supervisor
 * supervisor interrupt
 * supervisor FIQ
 * supervisor exception
 * supervisor interrupt FIQ
 * supervisor interrupt exception
 * supervisor FIQ exception
 * supervisor interrupt FIQ exception
 *
 */

void
arch_backtrace_current_task(void *stack_base, size_t stack_len)
{
	struct arm_exception_frame	*exception_frame, *fiq_frame, *interrupt_frame, *outer_frame;
	u_int32_t			esb, esl;
	u_int32_t			base, count, *insn, offending_pc;

	uintptr_t			btext = (uintptr_t)__segment_start(__TEXT);
	uintptr_t			etext = (uintptr_t)__segment_end(__TEXT);

	/*
	 * Find the stack limits for the current context and then backtrace it.
	 */
	arm_stack_for_status_reg(arm_read_cpsr(), &esb, &esl);
	if (0 == esl) {
		esb = (u_int32_t)stack_base;
		esl = stack_len;
	}
	arm_backtrace(arm_decode_mode(arm_read_cpsr()), 
		      (u_int32_t)&arch_backtrace_current_task, (u_int32_t)__builtin_frame_address(0), 
		      esb, esl);

	/*
	 * Find valid saved contexts, for the case where we may be in an exception state.
	 */
	exception_frame = arm_get_exception_frame();
	fiq_frame = arm_get_fiq_frame();
	interrupt_frame = arm_get_interrupt_frame();
	outer_frame = NULL;

	/*
	 * If we have a valid exception context, backtrace the context it was called from first.
	 * That context may be supervisor, FIQ or interrupt mode.  If it was FIQ mode, we will
	 * dump the context the FIQ was called from next.
	 */
	if (NULL != exception_frame) {

		/* dump registers at entry to exception mode */
		arm_print_frame_registers(exception_frame, "               ");

		/*
		 * If the exception handler thought that the data around the PC was worth
		 * displaying, and we think the PC was inside the text section, display it here.
		 */
		if (true == exception_dump_near_pc) {
			offending_pc = exception_frame->pc;
			if ((offending_pc >= btext) && (offending_pc <= etext)) {
				if ((offending_pc - btext) < 8) {
					base = btext;
				} else {
					base = offending_pc - 8;
				}
				dprintf(DEBUG_CRITICAL, "               [0x%08x:", base);
				for (count = 0; count < 4; count++) {
					insn = (u_int32_t *)base + count;
					dprintf(DEBUG_CRITICAL, " 0x%08x", *insn);
				}
				dprintf(DEBUG_CRITICAL, "]\n");
			} else {
				dprintf(DEBUG_CRITICAL, "               [pc 0x%08x not in text %p-%p]\n",
					offending_pc, (void *)btext, (void *)etext);
			}
		}

		/*
		 * If the exception occurred in a context with a special stack (FIQ or interrupt)
		 * backtrace that stack here.
		 */
		arm_stack_for_status_reg(exception_frame->spsr, &esb, &esl);
		if (0 != esl) {
			arm_backtrace(arm_decode_mode(exception_frame->spsr),
				      exception_frame->pc, exception_frame->r[7],
				      esb, esl);
		} else {
			/* the outer context is in the exception frame */
			outer_frame = exception_frame;
		}
	}

	/*
	 * If we have a valid FIQ context, backtrace the context it was invoked from now.
	 */
	if (NULL != fiq_frame) {
		
		/* dump registers at entry to FIQ mode */
		arm_print_frame_registers(fiq_frame, "               ");

		/*
		 * If the FIQ occurred from interrupt mode, backtrace the interrupt mode stack here.
		 */
		arm_stack_for_status_reg(fiq_frame->spsr, &esb, &esl);
		if (0 != esl) {
			arm_backtrace(arm_decode_mode(fiq_frame->spsr),
				      fiq_frame->pc, fiq_frame->r[7],
				      esb, esl);
		} else {
			/* the outer context is in the FIQ frame */
			outer_frame = fiq_frame;
		}
	}

	/*
	 * If we have an interrupt-saved context, fetch the task state from it.
	 */
	if (NULL != interrupt_frame) {

		/* dump registers at entry to interrupt mode */
		arm_print_frame_registers(interrupt_frame, "               ");

		outer_frame = interrupt_frame;
	}

	/*
	 * Print the task context backtrace from the outer frame.
	 */
	if (NULL != outer_frame) {
		arm_backtrace(arm_decode_mode(outer_frame->spsr), 
			      outer_frame->pc, outer_frame->r[7],
			      (u_int32_t)stack_base, (u_int32_t)stack_len);
	}
}

/*
 * arch_backtrace_task
 *
 * Print a backtrace from a saved task context.
 */
void
arch_backtrace_task(struct arch_task *arch_task, void *stack_base, size_t stack_len)
{
	arm_backtrace("supervisor", arch_task->regs[ARM_ARCH_TASK_LR], arch_task->regs[ARM_ARCH_TASK_FP], 
		      (u_int32_t)stack_base, (u_int32_t)stack_len);
}
