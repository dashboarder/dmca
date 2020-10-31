/*
 * Copyright (c) 2007-2011, 2014 Apple Inc. All rights reserved.
 */
/*
 * Processor registers for ARM64
 */
#ifndef _ARM64_PROC_REG_H_
#define _ARM64_PROC_REG_H_

// Pick the right registers depending on the exception level we're booting into.
#if	WITH_EL3
#define	SCTLR_ELx	SCTLR_EL3
#define	MAIR_ELx	MAIR_EL3
#define	TCR_ELx		TCR_EL3
#define TTBR0_ELx	TTBR0_EL3
#define	VBAR_ELx	VBAR_EL3
#else
#define	SCTLR_ELx	SCTLR_EL1
#define	MAIR_ELx	MAIR_EL1
#define	TCR_ELx		TCR_EL1
#define TTBR0_ELx	TTBR0_EL1
#define	VBAR_ELx	VBAR_EL1
#endif

/*
 * 64-bit Program Status Register (PSR64)
 *
 *  31	    27	22 21 20 19	 10 9	    5 4	  0
 * +-+-+-+-+------+--+--+----------+-+-+-+-+-+-----+
 * |N|Z|C|V|000000|SS|IL|0000000000|D|A|I|F|0|	M  |
 * +-+-+-+-+-+----+--+--+----------+-+-+-+-+-+-----+
 *
 * where:
 *	NZCV	Comparison flags
 *  SS		Single step
 *	IL		Illegal state
 *	DAIF	Interrupt masks
 *	M		Mode field
 */

#define PSR64_NZCV_SHIFT		28
#define PSR64_NZCV_MASK			(1 << PSR64_NZCV_SHIFT)

#define PSR64_N_SHIFT			31
#define PSR64_N				(1 << PSR64_N_SHIFT)

#define PSR64_Z_SHIFT			30
#define PSR64_Z				(1 << PSR64_Z_SHIFT)

#define PSR64_C_SHIFT			29
#define PSR64_C				(1 << PSR64_C_SHIFT)

#define PSR64_V_SHIFT			28
#define PSR64_V				(1 << PSR64_V_SHIFT)

#define PSR64_SS_SHIFT			21
#define PSR64_SS			(1 << PSR64_SS_SHIFT)

#define PSR64_IL_SHIFT			20
#define PSR64_IL			(1 << PSR64_IL_SHIFT)

/*
 * msr DAIF, Xn and mrs Xn, DAIF transfer into
 * and out of bits 9:6
 */
#define DAIF_DEBUG_SHIFT		9
#define DAIF_DEBUGF			(1 << DAIF_DEBUG_SHIFT)

#define DAIF_ASYNC_SHIFT		8
#define DAIF_ASYNCF			(1 << DAIF_ASYNC_SHIFT)

#define DAIF_IRQF_SHIFT			7
#define DAIF_IRQF			(1 << DAIF_IRQF_SHIFT)

#define DAIF_FIQF_SHIFT			6
#define DAIF_FIQF			(1 << DAIF_FIQF_SHIFT)

#define DAIF_ALL			(DAIF_DEBUGF | DAIF_ASYNCF | DAIF_IRQF | DAIF_FIQF)
#define DAIF_STANDARD_DISABLE		(DAIF_ASYNCF | DAIF_IRQF | DAIF_FIQF)

#define SPSR_INTERRUPTS_ENABLED(x)	(!(x & DAIF_FIQF))

/*
 * msr DAIFSet, Xn, and msr DAIFClr, Xn transfer
 * from bits 3:0.
 */
#define DAIFSC_DEBUGF			(1 << 3)
#define DAIFSC_ASYNCF			(1 << 2)
#define DAIFSC_IRQF			(1 << 1)
#define DAIFSC_FIQF			(1 << 0)
#define DAIFSC_ALL			(DAIFSC_DEBUGF | DAIFSC_ASYNCF | DAIFSC_IRQF | DAIFSC_FIQF)
#define DAIFSC_STANDARD_DISABLE 	(DAIFSC_ASYNCF | DAIFSC_IRQF | DAIFSC_FIQF)

/*
 * ARM64_TODO: unify with ARM?
 */
#define PSR64_CF			0x20000000	/* Carry/Borrow/Extend */

#define PSR64_MODE_MASK			0x1F

#define PSR64_MODE_USER32_THUMB		0x20

#define PSR64_MODE_RW_SHIFT		4
#define PSR64_MODE_RW_64		0
#define PSR64_MODE_RW_32		(0x1 << PSR64_MODE_RW_SHIFT)

#define PSR64_MODE_EL_SHIFT		2
#define PSR64_MODE_EL_MASK		(0x3 << PSR64_MODE_EL_SHIFT)
#define PSR64_MODE_EL3			(0x3 << PSR64_MODE_EL_SHIFT)
#define PSR64_MODE_EL1			(0x1 << PSR64_MODE_EL_SHIFT)
#define PSR64_MODE_EL0			0

#define PSR64_MODE_SPX			0x1
#define PSR64_MODE_SP0			0

#define PSR64_USER32_DEFAULT		(PSR64_MODE_RW_32 | PSR64_MODE_EL0 | PSR64_MODE_SP0)
#define PSR64_USER64_DEFAULT		(PSR64_MODE_RW_64 | PSR64_MODE_EL0 | PSR64_MODE_SP0)
#define PSR64_KERNEL_DEFAULT		(DAIF_STANDARD_DISABLE | PSR64_MODE_RW_64 | PSR64_MODE_EL1 | PSR64_MODE_SP0)

#define PSR64_IS_KERNEL(x)		((x & PSR64_MODE_EL_MASK) == PSR64_MODE_EL1)
#define PSR64_IS_USER(x)		((x & PSR64_MODE_EL_MASK) == PSR64_MODE_EL0)


/*
 * Floating Point Control Register (FPCR)
 */
#define FPCR_DN				(1 << 25)

#define FPCR_DEFAULT			(FPCR_DN)


/*
 * System Control Register (SCTLR)
 *
 *  31	  26  25 24  23	  19  18  15  14  13 12 11   7	 6    5	     4	 3  2 1 0
 * +-----+---+--+---+----+---+---+---+---+--+--+----+---+----+------+---+--+-+-+-+
 * |00110|UCI|EE|E0E|1101|WXN|101|UCT|DZE| 0|I |1000|ITD|THEE|C15BEN|SA0|SA|D|A|M|
 * +-----+---+--+---+----+---+---+---+---+--+--+----+---+----+------+---+--+-+-+-+
 *
 *	UCI		User Cache Instructions
 *	EE		Exception Endianness
 *	E0E		EL0 Endianness
 *	WXN		Writeable implies eXecute Never
 *	UCT		User Cache Type register (CTR_EL0)
 *	DZE	user Data Cache Zero (DC ZVA)
 *	I		Instruction cache enable
 *	ITD		IT Disable
 *	THEE	THumb EE enable
 *	CP15BEN CP15 Barrier ENable
 *	SA0		Stack Alignment check for EL0
 *	SA		Stack Alignment check enable
 *	D		Data/unified cache enable
 *	A		Alignment check enable
 *	M		MMU enable
 */
#define SCTLR_RESERVED				0x30d50800

#define SCTLR_UCI_ENABLED			(1 << 26)
#define SCTLR_EE_ENABLED			(1 << 25)
#define SCTLR_E0E_ENABLED			(1 << 24)
#define SCTLR_WXN_ENABLED			(1 << 19)
#define SCTLR_UCT_ENABLED			(1 << 15)
#define SCTLR_DZE_ENABLED			(1 << 14)
#define SCTLR_I_ENABLED				(1 << 12)
#define SCTLR_ITD_DISABLED			(1 << 7)
#define SCTLR_THEE_ENABLED			(1 << 6)
#define SCTLR_CP15BEN_ENABLED			(1 << 5)
#define SCTLR_SA0_ENABLED			(1 << 4)
#define SCTLR_SA_ENABLED			(1 << 3)
#define SCTLR_D_ENABLED				(1 << 2)
#define SCTLR_A_ENABLED				(1 << 1)
#define SCTLR_M_ENABLED				(1 << 0)

#define SCTLR_EL1_DEFAULT			(SCTLR_RESERVED | SCTLR_I_ENABLED | SCTLR_SA_ENABLED | SCTLR_D_ENABLED | SCTLR_M_ENABLED)

/*
 * Secure Configuratio Register (SCR)
 *
 *  31			12		     0
 * +---+---+------+------+--------------------+
 * |00000000000000000000 ||
 * +---+---+------+------+--------------------+
 *
 * where:
 *	TTA		Trace trap
 *	FPEN	Floating point enable
 */
#define SCR_EA_SHIFT		(3)
#define SCR_EA			(1 << SCR_EA_SHIFT)

#define SCR_FIQ_SHIFT		(2)
#define SCR_FIQ			(1 << SCR_FIQ_SHIFT)

#define SCR_IRQ_SHIFT		(1)
#define SCR_IRQ			(1 << SCR_IRQ_SHIFT)

#define SCR_DEFAULT		(SCR_EA | SCR_FIQ | SCR_IRQ)

/*
 * Coprocessor Access Control Register (CPACR)
 *
 *  31	28  27	22 21  20 19		     0
 * +---+---+------+------+--------------------+
 * |000|TTA|000000| FPEN |00000000000000000000|
 * +---+---+------+------+--------------------+
 *
 * where:
 *	TTA	Trace trap
 *	FPEN	Floating point enable
 */
#define CPACR_TTA_SHIFT				28
#define CPACR_TTA				(1 << CPACR_TTA_SHIFT)

#define CPACR_FPEN_SHIFT			20
#define CPACR_FPEN_EL0_TRAP			(0x1 << CPACR_FPEN_SHIFT)
#define CPACR_FPEN_ENABLE			(0x3 << CPACR_FPEN_SHIFT)

/*
 * Translation Control Register (TCR) - EL3
 *
 *  31 30   24 23 22 21 20 19 18  16  14 13 12 11 10 9  8 7 6 5  0
 * +-+-------+--+-----+---+--+-----+---+-----+-----+-----+-+-+----+
 * |1|   0   | 1|  0  |TBI| 0|  PS |TG0| SH0 |ORGN0|IRGN0|0|0|T0SZ|
 * +-+-------+--+-----+---+--+-----+---+-----+-----+-----+-+-+----+
 *
 *	TBI 	Top Byte Ignored for TTBR0 region
 *	PS	Physical Address Size limit
 *	TG0	Granule Size for TTBR0 region
 *	SH0	Shareability for TTBR0 region
 *	ORGN0	Outer Cacheability for TTBR0 region
 *	IRGN0	Inner Cacheability for TTBR0 region
 *	T0SZ	Virtual address size for TTBR0

 * Translation Control Register (TCR) - EL1
 *
 *  63   39   38   37 36   34 32    30 29 28 27 26 25 24   23 22 21  16   14 13 12 11 10 9   8     7   5  0
 * +------+----+----+--+-+-----+-+---+-----+-----+-----+----+--+------+-+---+-----+-----+-----+----+-+----+
 * |   0  |TBI1|TBI0|AS|0| IPS |0|TG1| SH1 |ORGN1|IRGN1|EPD1|A1| T1SZ |0|TG0| SH0 |ORGN0|IRGN0|EPD0|0|T0SZ|
 * +------+----+----+--+-+-----+-+---+-----+-----+-----+----+--+------+-+---+-----+-----+-----+----+-+----+
 *
 *	TBI1	Top Byte Ignored for TTBR1 region
 *	TBI0	Top Byte Ignored for TTBR0 region
 *	AS	ASID Size
 *	IPS	Physical Address Size limit
 *	TG1	Granule Size for TTBR1 region
 *	SH1	Shareability for TTBR1 region
 *	ORGN1	Outer Cacheability for TTBR1 region
 *	IRGN1	Inner Cacheability for TTBR1 region
 *	EPD1	Translation table walk disable for TTBR1
 *	A1	ASID selection from TTBR1 enable
 *	T1SZ	Virtual address size for TTBR1
 *	TG0	Granule Size for TTBR0 region
 *	SH0	Shareability for TTBR0 region
 *	ORGN0	Outer Cacheability for TTBR0 region
 *	IRGN0	Inner Cacheability for TTBR0 region
 *	T0SZ	Virtual address size for TTBR0
 */

// Common defintions for EL1 and EL3 versions of this register
#define TCR_ELx_T0SZ_SHIFT		0

#define TCR_ELx_IRGN0_SHIFT		8
#define TCR_ELx_IRGN0_DISABLED		(0ULL << TCR_ELx_IRGN0_SHIFT)
#define TCR_ELx_IRGN0_WRITEBACK		(1ULL << TCR_ELx_IRGN0_SHIFT)
#define TCR_ELx_IRGN0_WRITETHRU		(2ULL << TCR_ELx_IRGN0_SHIFT)
#define TCR_ELx_IRGN0_WRITEBACKNO	(3ULL << TCR_ELx_IRGN0_SHIFT)

#define TCR_ELx_ORGN0_SHIFT		10
#define TCR_ELx_ORGN0_DISABLED		(0ULL << TCR_ELx_ORGN0_SHIFT)
#define TCR_ELx_ORGN0_WRITEBACK		(1ULL << TCR_ELx_ORGN0_SHIFT)
#define TCR_ELx_ORGN0_WRITETHRU		(2ULL << TCR_ELx_ORGN0_SHIFT)
#define TCR_ELx_ORGN0_WRITEBACKNO	(3ULL << TCR_ELx_ORGN0_SHIFT)

#define TCR_ELx_SH0_SHIFT		12
#define TCR_ELx_SH0_NONE		(0ULL << TCR_ELx_SH0_SHIFT)
#define TCR_ELx_SH0_OUTER		(2ULL << TCR_ELx_SH0_SHIFT)
#define TCR_ELx_SH0_INNER		(3ULL << TCR_ELx_SH0_SHIFT)

#define TCR_ELx_TG0_GRANULE_SHIFT	14
#define TCR_ELx_TG0_GRANULE_4KB		(0ULL << TCR_ELx_TG0_GRANULE_SHIFT)
#define TCR_ELx_TG0_GRANULE_64KB	(1ULL << TCR_ELx_TG0_GRANULE_SHIFT)
#define TCR_ELx_TG0_GRANULE_16KB	(2ULL << TCR_ELx_TG0_GRANULE_SHIFT)

#if WITH_EL3
// EL3-specific TCR register definitions

#define TCR_EL3_PS_SHIFT		16
#define TCR_EL3_PS_32BITS		(0ULL << TCR_EL3_PS_SHIFT)
#define TCR_EL3_PS_36BITS		(1ULL << TCR_EL3_PS_SHIFT)
#define TCR_EL3_PS_40BITS		(2ULL << TCR_EL3_PS_SHIFT)
#define TCR_EL3_PS_42BITS		(3ULL << TCR_EL3_PS_SHIFT)
#define TCR_EL3_PS_44BITS		(4ULL << TCR_EL3_PS_SHIFT)
#define TCR_EL3_PS_48BITS		(5ULL << TCR_EL3_PS_SHIFT)

#define TCR_EL3_TBI_TOPBYTE_IGNORED	(1ULL << 20)

#else
// EL1-specific TCR register definitions

#define TCR_EL1_EPD0_TTBR0_DISABLED	(1ULL << 7)

#define TCR_EL1_T1SZ_SHIFT		16

#define TCR_EL1_A1_ASID1		(1ULL << 22)
#define TCR_EL1_EPD1_TTBR1_DISABLED	(1ULL << 23)

#define TCR_EL1_IRGN1_SHIFT		24
#define TCR_EL1_IRGN1_DISABLED		(0ULL << TCR_EL1_IRGN1_SHIFT)
#define TCR_EL1_IRGN1_WRITEBACK		(1ULL << TCR_EL1_IRGN1_SHIFT)
#define TCR_EL1_IRGN1_WRITETHRU		(2ULL << TCR_EL1_IRGN1_SHIFT)
#define TCR_EL1_IRGN1_WRITEBACKNO	(3ULL << TCR_EL1_IRGN1_SHIFT)

#define TCR_EL1_ORGN1_SHIFT		26
#define TCR_EL1_ORGN1_DISABLED		(0ULL << TCR_EL1_ORGN1_SHIFT)
#define TCR_EL1_ORGN1_WRITEBACK		(1ULL << TCR_EL1_ORGN1_SHIFT)
#define TCR_EL1_ORGN1_WRITETHRU		(2ULL << TCR_EL1_ORGN1_SHIFT)
#define TCR_EL1_ORGN1_WRITEBACKNO	(3ULL << TCR_EL1_ORGN1_SHIFT)

#define TCR_EL1_SH1_SHIFT		28
#define TCR_EL1_SH1_NONE		(0ULL << TCR_EL1_SH1_SHIFT)
#define TCR_EL1_SH1_OUTER		(2ULL << TCR_EL1_SH1_SHIFT)
#define TCR_EL1_SH1_INNER		(3ULL << TCR_EL1_SH1_SHIFT)

#define TCR_EL1_TG1_GRANULE_SHIFT	30
#define TCR_EL1_TG1_GRANULE_16KB	(1ULL << TCR_EL1_TG1_GRANULE_SHIFT)
#define TCR_EL1_TG1_GRANULE_4KB		(2ULL << TCR_EL1_TG1_GRANULE_SHIFT)
#define TCR_EL1_TG1_GRANULE_64KB	(3ULL << TCR_EL1_TG1_GRANULE_SHIFT)

#define TCR_EL1_IPS_SHIFT		32
#define TCR_EL1_IPS_32BITS		(0ULL << TCR_EL1_IPS_SHIFT)
#define TCR_EL1_IPS_36BITS		(1ULL << TCR_EL1_IPS_SHIFT)
#define TCR_EL1_IPS_40BITS		(2ULL << TCR_EL1_IPS_SHIFT)
#define TCR_EL1_IPS_42BITS		(3ULL << TCR_EL1_IPS_SHIFT)
#define TCR_EL1_IPS_44BITS		(4ULL << TCR_EL1_IPS_SHIFT)
#define TCR_EL1_IPS_48BITS		(5ULL << TCR_EL1_IPS_SHIFT)

#define TCR_EL1_AS_16BIT_ASID		(1ULL << 36)
#define TCR_EL1_TBI0_TOPBYTE_IGNORED	(1ULL << 37)
#define TCR_EL1_TBI1_TOPBYTE_IGNORED	(1ULL << 38)

#endif

/*
 * Translation Table Base Register (TTBR)
 *
 *  63	  48 47		      x x-1  0
 * +--------+------------------+------+
 * |  ASID  |	Base Address   | zero |
 * +--------+------------------+------+
 *
 */
#define TTBR_ASID_SHIFT			48
#define TTBR_ASID_MASK			0xffff000000000000

#define TTBR_BADDR_MASK			0x0000ffffffffffff

/*
 * Memory Attribute Indirection Register
 *
 *  63	 56 55	 48 47	 40 39	 32 31	 24 23	 16 15	  8 7	  0
 * +-------+-------+-------+-------+-------+-------+-------+-------+
 * | Attr7 | Attr6 | Attr5 | Attr4 | Attr3 | Attr2 | Attr1 | Attr0 |
 * +-------+-------+-------+-------+-------+-------+-------+-------+
 *
 */

#define MAIR_ATTR_SHIFT(x)			(8*(x))

/* Strongly ordered or device memory attributes */
#define MAIR_OUTER_STRONGLY_ORDERED		0x0
#define MAIR_OUTER_DEVICE			0x0

#define MAIR_INNER_STRONGLY_ORDERED		0x0
#define MAIR_INNER_DEVICE			0x4

/* Normal memory attributes */
#define MAIR_OUTER_NON_CACHEABLE		0x40
#define MAIR_OUTER_WRITE_THROUGH		0x80
#define MAIR_OUTER_WRITE_BACK			0xc0

#define MAIR_INNER_NON_CACHEABLE		0x4
#define MAIR_INNER_WRITE_THROUGH		0x8
#define MAIR_INNER_WRITE_BACK			0xc

/* Allocate policy for cacheable memory */
#define MAIR_OUTER_WRITE_ALLOCATE		0x10
#define MAIR_OUTER_READ_ALLOCATE		0x20

#define MAIR_INNER_WRITE_ALLOCATE		0x1
#define MAIR_INNER_READ_ALLOCATE		0x2

/* Memory Atribute Encoding */
#define MAIR_DISABLE				0x04	/* Strongly Ordered */
#define MAIR_WRITECOMB				0x44	/* Normal Memory, Outer Non-Cacheable, Inner Non-Cacheable */
#define MAIR_WRITETHRU				0xBB	/* Normal Memory, Outer Write-through, Inner Write-through */
#define MAIR_WRITEBACK				0xFF	/* Normal Memory, Outer Write-back, Inner Write-back */
#define MAIR_INNERWRITEBACK			0x4F	/* Normal Memory, Outer Non-Cacheable, Inner Write-back */


/*
 *	ARM 4-level Page Table support - 2*1024TB (2^48) of address space
 */


/*
 *  Memory Attribute Index
 */
#define CACHE_ATTRINDX_DISABLE			0x0	/* no cache, no buffer */
#define CACHE_ATTRINDX_WRITEBACK		0x1	/* cache enabled, buffer enabled */
#define CACHE_ATTRINDX_WRITECOMB		0x1	/* no cache, buffered writes */
#define CACHE_ATTRINDX_WRITETHRU		0x2	/* cache enabled, buffer disabled */
#define CACHE_ATTRINDX_INNERWRITEBACK		0x4	/* inner cache enabled, buffer enabled, write allocate */
#define CACHE_ATTRINDX_DEFAULT			CACHE_ATTRINDX_WRITEBACK

/*
 *	Access protection bit values (TTEs and PTEs)
 */
#define AP_RWNA					0x0	/* priv=read-write, user=no-access */
#define AP_RWRW					0x1	/* priv=read-write, user=read-write */
#define AP_RONA					0x2	/* priv=read-only, user=no-access */
#define AP_RORO					0x3	/* priv=read-only, user=read-only */
#define AP_MASK					0x3	/* mask to find ap bits */

/*
 * Shareability attributes
 */
#define SH_NONE					0x0 	/* Non shareable  */
#define SH_OUTER_MEMORY				0x2 	/* Normal memory Inner shareable - Outer shareable */
#define SH_INNER_MEMORY				0x3 	/* Normal memory Inner shareable - Outer non shareable */

/*
 * LPAE/ARMv8 Table Entry
 *
 *  63 62 61 60	 59 58	 52 51	48 47		       12 11	2 1 0
 * ,--+-----+--+---+-------+------+----------------------+-------+-+-.
 * |NS|  AP |XN|PXN|ignored| zero | TableOutputAddress   |ignored|1|V|
 * `--+-----+--+---+-------+------+----------------------+-------+-+-'
 *
 * LPAE/ARMv8 Block Entry
 *
 *  63 59 58 55 54  53	 52 51 48 47         12 11 10 9  8 7  6  5 4     2 1 0
 * ,-----+-----+--+---+----+-----+-------------+--+--+----+----+--+-------+-+-.
 * | ign |  SW |XN|PXN|HINT| zero|OutputAddress|nG|AF| SH | AP |NS|AttrIdx|W|V|
 * `-----+-----+--+---+----+-----+-------------+--+--+----+----+--+-------+-+-'
 *
 * NS (NSTable)		Non-Secure: NS=1 for everything outside of TZ region
 * AP (APTable)		Access Permission: AP[2:1]=0 for RW, 2 for RO
 * XN (XNTable)		eXecute Never: XN=0 for code, 1 for data
 * PXN (PXNTable)	Privileged eXecute Never: 1=dont execute when EL3
 * TableOutputAddress	Next physical base: Lower bits 0 as alignment requires.
 * SW			Reserved for software use (unused, can have metadata)
 * HINT			Contiguous hint. Optimization only, leave as 0.
 * OutputAddress	Block physical base: Lower bits 0 as alignment requires.
 * nG			notGlobal: nG=1 restricts TLB hit to matching ASID
 * AF			Access Flag: 0=never,1=hit. ARMv8 may fault if 0.
 * SH			Shareability, SH[1:0]=2 for normal memory
 * AttrIdx		Memory Attribute Index, 0 for device, 1 for normal
 * W			"Walk" flag: L0-2 0=Block,1=Table; L3 0=Fault,1=Block
 */

#define ARM_LPAE_TABLE_MASK		(0x0000fffffffff000ULL) 	/* mask for extracting pointer to next table (works at any level) */

#define ARM_LPAE_NSTABLE		0x8000000000000000ULL		/* value for a secure mapping */
#define ARM_LPAE_NSTABLE_MASK		0x8000000000000000ULL		/* notSecure mapping mask */

#define ARM_LPAE_APTABLE_SHIFT		61
#define ARM_LPAE_APTABLE(x)		((x)<<ARM_LPAE_APTABLE_SHIFT) 	/* access protection */

#define ARM_LPAE_XNTABLE		0x1000000000000000ULL		/* value for no execute */
#define ARM_LPAE_XNTABLE_MASK		0x1000000000000000ULL		/* no execute mask */

#define ARM_LPAE_PXNTABLE		0x0800000000000000ULL		/* value for privilege no execute bit */
#define ARM_LPAE_PXNTABLE_MASK		0x0800000000000000ULL		/* privilege execute mask */

#define ARM_LPAE_XN			0x0040000000000000ULL		/* value for no execute */
#define ARM_LPAE_XN_MASK		0x0040000000000000ULL		/* no execute mask */

#define ARM_LPAE_PXN			0x0020000000000000ULL		/* value for privilege no execute bit */
#define ARM_LPAE_PXN_MASK		0x0020000000000000ULL		/* privilege execute mask */

#define ARM_LPAE_NG			0x0000000000000800ULL		/* value for a global mapping */
#define ARM_LPAE_NG_MASK		0x0000000000000800ULL		/* notGlobal mapping mask */

#define ARM_LPAE_AF			0x0000000000000400ULL		/* value for access */
#define ARM_LPAE_AF_MASK		0x0000000000000400ULL		/* access mask */

#define ARM_LPAE_SH_SHIFT		8
#define ARM_LPAE_SH(x)			((x) << ARM_LPAE_SH_SHIFT)	/* access shared */
#define ARM_LPAE_SH_MASK		(0x3ULL << ARM_LPAE_SH_SHIFT)	/* mask access shared */

#define ARM_LPAE_AP_SHIFT		6
#define ARM_LPAE_AP(x)			((x)<<ARM_LPAE_AP_SHIFT) 	/* access protection */
#define ARM_LPAE_AP_MASK		(0x3ULL<<ARM_LPAE_AP_SHIFT)

#define ARM_LPAE_NS			0x0000000000000020ULL		/* value for a secure mapping */
#define ARM_LPAE_MASK			0x0000000000000020ULL		/* notSecure mapping mask */

#define ARM_LPAE_ATTRINDX_SHIFT		2				/* memory attributes index */
#define ARM_LPAE_ATTRINDX(x)		((x) << ARM_LPAE_ATTRINDX_SHIFT)	/* memory attributes index */
#define ARM_LPAE_ATTRINDX_MASK		(0x3ULL << ARM_LPAE_ATTRINDX_SHIFT)	/* mask memory attributes index */

#define ARM_LPAE_TYPE_MASK		0x0000000000000002ULL		/* mask for extracting the type */
#define ARM_LPAE_TYPE_TABLE		0x0000000000000002ULL		/* page table type */
#define ARM_LPAE_TYPE_L0L1L2BLOCK	0x0000000000000000ULL		/* block entry type */
#define ARM_LPAE_TYPE_L3BLOCK		0x0000000000000002ULL		/* L3 is different */

#define ARM_LPAE_VALID			0x0000000000000001ULL		/* valid entry */

/*
 * Exception Syndrome Register
 *
 *  31	26 25 24	       0
 * +------+--+------------------+
 * |  EC  |IL|	     ISS	|
 * +------+--+------------------+
 *
 *	EC - Exception Class
 *	IL - Instruction Length
 *  ISS- Instruction Specific Syndrome
 *
 * Note: The ISS can have many forms. These are defined separately below.
 */

#define ESR_EC_SHIFT			26
#define ESR_EC_MASK			(0x3F << ESR_EC_SHIFT)
#define ESR_EC(x)			((x & ESR_EC_MASK) >> ESR_EC_SHIFT)

#define ESR_IL_SHIFT			25
#define ESR_IL				(1 < ESR_IL_SHIFT)

#define ESR_IS_THUMB(x)			(!(x & ESR_IL))

#define ESR_ISS_MASK			0x01FFFFFF
#define ESR_ISS(x)			(x & ESR_ISS_MASK)

/*
 * Instruction Abort ISS (EL1)
 *  24		 10 9	   5	0
 * +---------------+--+---+------+
 * |000000000000000|EA|000| IFSC |
 * +---------------+--+---+------+
 *
 * where:
 *	EA		External Abort type
 *	IFSC	Instruction Fault Status Code
 */

#define ISS_IA_EA_SHIFT				9
#define ISS_IA_EA				(0x1 << ISS_IA_EA_SHIFT)

#define ISS_IA_FSC_MASK				0x2F
#define ISS_IA_FSC(x)				(x & ISS_DA_FSC_MASK)


/*
 * Data Abort ISS (EL1)
 *
 *  24		    9  8  7  6	5  0
 * +---------------+--+--+-+---+----+
 * |000000000000000|EA|CM|0|WnR|DFSC|
 * +---------------+--+--+-+---+----+
 *
 * where:
 *	EA		External Abort type
 *	CM		Cache Maintenance operation
 *	WnR		Write not Read
 *	DFSC		Data Fault Status Code
 */
#define ISS_DA_EA_SHIFT				9
#define ISS_DA_EA				(0x1 << ISS_DA_EA_SHIFT)

#define ISS_DA_CM_SHIFT				8
#define ISS_DA_CM				(0x1 << ISS_DA_CM_SHIFT)

#define ISS_DA_WNR_SHIFT			6
#define ISS_DA_WNR				(0x1 << ISS_DA_WNR_SHIFT)

#define ISS_DA_FSC_MASK				0x2F
#define ISS_DA_FSC(x)				(x & ISS_DA_FSC_MASK)

#define PLATFORM_SYSCALL_TRAP_NO		0x80000000

#define ARM64_SYSCALL_CODE_REG_NUM		(16)

#define ARM64_CLINE_SHIFT			6

/*
 * Timer definitions.
 */
#define CNTKCTL_EL1_PL0PTEN			(0x1 << 9)		/* 1: EL0 access to physical timer regs permitted */
#define CNTKCTL_EL1_PL0VTEN			(0x1 << 8)		/* 1: EL0 access to virtual timer regs permitted */
#define CNTKCTL_EL1_EVENTI_MASK			(0x000000f0)		/* Mask for bits describing which bit to use for triggering event stream */
#define CNTKCTL_EL1_EVENTI_SHIFT		(0x4)			/* Shift for same */
#define CNTKCTL_EL1_EVENTDIR			(0x1 << 3)		/* 1: one-to-zero transition of specified bit causes event */
#define CNTKCTL_EL1_EVNTEN			(0x1 << 2)		/* 1: enable event stream */
#define CNTKCTL_EL1_PL0VCTEN			(0x1 << 1)		/* 1: EL0 access to physical timebase + frequency reg enabled */
#define CNTKCTL_EL1_PL0PCTEN			(0x1 << 0)		/* 1: EL0 access to virtual timebase + frequency reg enabled */

#endif /* _ARM64_PROC_REG_H_ */
