/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _IOKIT_APPLES5L8900XPKE_HARDWARE_H
#define _IOKIT_APPLES5L8900XPKE_HARDWARE_H

#define IN_KERNEL 0

#if IN_KERNEL
#include <IOKit/IOLib.h>
#endif

struct pke_regs {
	uint32_t	pke_key_len;			// 0x000
#define S5L8900XPKE_key_len(_precision, _chunk_size)	(((_precision) & 0x03) | ((((_chunk_size) / 32) & 0x0f) << 3))
	uint32_t	pad004;				// 0x004
	uint32_t	pke_start;			// 0x008
#define S5L8900XPKE_exec_on		(1<<0)
#define S5L8900XPKE_pldm_on		(1<<3)
	uint32_t	pke_seg_id;			// 0x00c
#define S5L8900XPKE_seg_ids(_a, _b, _m, _s)	(((_a) << 24 ) | ((_b) << 16) | ((_m) << 8) | (_s))
	uint32_t	pke_seg_sign;			// 0x010
	uint32_t	pke_seg_size;			// 0x014
#define S5L8900XPKE_func_id		(1<<0)
#define S5L8900XPKE_seg_256		(0x00 << 6)		
#define S5L8900XPKE_seg_128		(0x01 << 6)		
#define S5L8900XPKE_seg_64		(0x02 << 6)		
	uint32_t	pke_intmode;			// 0x018
#define S5L8900XPKE_interrupt		(1<<0)
	uint32_t	pke_int;			// 0x01c
#define S5L8900XPKE_end_int		(1<<0)	
	uint32_t	pke_int_mask;			// 0x020
#define S5L8900XPKE_end_mask		(1<<0)	
    uint32_t  pke_sw_reset;
#define S5L8900XPKE_reset   (1<<0)
};


/* ========================================================================= */
/*                     D E F I N E S                                         */
/* ========================================================================= */

#define kIOPKEAcceleratorComputeRsquareMask		0x10
#define	kIOPKEAcceleratorCacheRsquare			0x01
#define	kIOPKEAcceleratorPreProcessingDone		0x100
#define kIOPKEMaxBits	2048
#define kIOPKEUserDataEndianessMask	(1<<7)

/** \define
 *  	definition of data format size
 */
#define 	DATA_WORD_TO_BYTE	 	4	/*4BYTE=1WORD*/
#define 	DATA_BYTE_TO_BIT	 	8	/*1BYTE=8BIT*/

#define 	DATA_SHIFT_FOR_256		8	/*2^8=256*/
#define 	DATA_SHIFT_FOR_128		7	/*2^7=128*/
#define 	DATA_SHIFT_FOR_64		6	/*2^6=64*/

#define	PKE_REGSHIFT_KEY_LEN_CHNK_SZ 	3 

/** \define 
 *	definition of PKE precision size
 */
#define 	PKE_SIZE_PREC_0	 	0 	/*single*/	
#define 	PKE_SIZE_PREC_1	 	1 
#define 	PKE_SIZE_PREC_2	 	2 	
#define 	PKE_SIZE_PREC_3	 	3 


/** \define 
 *	definition of PKE key bit length
 */
#define 	PKE_LEN_BIT_128	 	128		/* min size of bit length*/	
#define 	PKE_LEN_BIT_512	 	512	
#define 	PKE_LEN_BIT_672	 	672	
#define 	PKE_LEN_BIT_864	 	864	
#define 	PKE_LEN_BIT_1024	 	1024
#define 	PKE_LEN_BIT_1280	 	1280	
#define 	PKE_LEN_BIT_1408	 	1408	
#define 	PKE_LEN_BIT_1536	 	1536	
#define 	PKE_LEN_BIT_2048	 	2048	/*max size of bit length*/

/** \define 
 *	definition of PKE Segment Size
 */
#define PKE_SEG_SIZE_256		(1<<DATA_SHIFT_FOR_256)		/*256 byte*/
#define PKE_SEG_SIZE_128		(1<<DATA_SHIFT_FOR_128)		/*128 byte*/
#define PKE_SEG_SIZE_64			(1<<DATA_SHIFT_FOR_64)		/*64 byte*/

/** \define 
 *	definition of PKE Segment Max Number (depend on segment size)
 */
#define PKE_SEG_NUM_CASE256	7
#define PKE_SEG_NUM_CASE128	15
#define PKE_SEG_NUM_CASE64	30

/** \define 
 *	definition of PKE Function mode
 */
#define PKE_FUNC_MODE_AB		0
#define PKE_FUNC_MODE_A1		1


/** \define 
 *	definition of PKE Register field data
 */
/* key len bits */
#define	PKE_REGSHIFT_KEY_LEN_CHNK_SZ 	3 
#define	PKE_REGSHIFT_KEY_LEN_PREC_ID 	0 

#define	PKE_REGMASK_KEY_LEN_CHNK_SZ  	(15 << PKE_REGSHIFT_KEY_LEN_CHNK_SZ )
#define	PKE_REGMASK_KEY_LEN_PREC_ID    	(3 << PKE_REGSHIFT_KEY_LEN_PREC_ID )

/* start bits */
#define	PKE_REGSHIFT_START_PLDM_ON   	3 
#define	PKE_REGSHIFT_START_EXEC_ON   		0 

#define	PKE_REGMASK_START_PLDM_ON   		(1 << PKE_REGSHIFT_START_PLDM_ON )
#define	PKE_REGMASK_START_EXEC_ON   		(1 << PKE_REGSHIFT_START_EXEC_ON )

/* seg id bits */
#define	PKE_REGSHIFT_SEG_ID_A_SEG_ID   	24 
#define	PKE_REGSHIFT_SEG_ID_B_SEG_ID   	16 
#define	PKE_REGSHIFT_SEG_ID_M_SEG_ID   	8 
#define	PKE_REGSHIFT_SEG_ID_S_SEG_ID   	0 

/* seg size bits */
#define	PKE_REGSHIFT_SEG_SIZE_SEGSIZE   		6
#define	PKE_REGSHIFT_SEG_SIZE_FUNC_ID   		1
#define	PKE_REGSHIFT_SEG_SIZE_MUSTBEONE   	0

#define	PKE_REGMASK_SEG_SIZE_SEGSIZE   		(3 << PKE_REGSHIFT_SEG_SIZE_SEGSIZE)
#define	PKE_REGMASK_SEG_SIZE_FUNC_ID   		(1 << PKE_REGSHIFT_SEG_SIZE_FUNC_ID)
#define	PKE_REGMASK_SEG_SIZE_MUSTBEONE   	(1 << PKE_REGSHIFT_SEG_SIZE_MUSTBEONE )

#define	PKE_REGVAL_SEG_SIZE_SEGSIZE_256   	(0 << PKE_REGSHIFT_SEG_SIZE_SEGSIZE) 
#define	PKE_REGVAL_SEG_SIZE_SEGSIZE_128   	(1 << PKE_REGSHIFT_SEG_SIZE_SEGSIZE) 
#define	PKE_REGVAL_SEG_SIZE_SEGSIZE_64    	(2 << PKE_REGSHIFT_SEG_SIZE_SEGSIZE) 
#define	PKE_REGVAL_SEG_SIZE_FUNC_ID_AB   	(0 << PKE_REGSHIFT_SEG_SIZE_FUNC_ID)
#define	PKE_REGVAL_SEG_SIZE_FUNC_ID_A1   	(1 << PKE_REGSHIFT_SEG_SIZE_FUNC_ID)

/** \enum PKE_SegID_et
 *	enumeation of  segment ID
 */
typedef enum 
{
	PKE_SEG_ID_00 = 0,
	PKE_SEG_ID_01,
	PKE_SEG_ID_02,
	PKE_SEG_ID_03,
	PKE_SEG_ID_04,
	PKE_SEG_ID_05,
	PKE_SEG_ID_06,		/*last segment ID in the 256byte segment block*/
	PKE_SEG_ID_07,
	PKE_SEG_ID_08,
	PKE_SEG_ID_09,
	PKE_SEG_ID_10,
	PKE_SEG_ID_11,
	PKE_SEG_ID_12,
	PKE_SEG_ID_13,
	PKE_SEG_ID_14,		/*last segment ID in the 128byte segment block*/
	PKE_SEG_ID_15,
	PKE_SEG_ID_16,
	PKE_SEG_ID_17,
	PKE_SEG_ID_18,
	PKE_SEG_ID_19,
	PKE_SEG_ID_20,
	PKE_SEG_ID_21,
	PKE_SEG_ID_22,
	PKE_SEG_ID_23,
	PKE_SEG_ID_24,
	PKE_SEG_ID_25,
	PKE_SEG_ID_26,
	PKE_SEG_ID_27,
	PKE_SEG_ID_28,
	PKE_SEG_ID_29		/*last segment ID in the 64byte segment block*/		
}PKE_SegID_et;

enum {
    mod_segid  = PKE_SEG_ID_00,	 /* ! moduler seg id MUST be '0' */
    tmp_segid  = PKE_SEG_ID_01,
    acum_segid = PKE_SEG_ID_02,
    iter_segid = PKE_SEG_ID_03
};


#endif /* _IOKIT_APPLES5L8900XPKE_HARDWARE_H */
