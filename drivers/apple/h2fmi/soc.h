/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef _H_SOC_
#define _H_SOC_ 1

#define SAMSUNG 0

#define PROC_REG_UCHAR_RW(a) ((volatile unsigned char *) (a))
#define PROC_REG_UCHAR_RO(a) ((volatile const unsigned char *) (a))
#define PROC_REG_USHORT_RW(a) ((volatile unsigned short *) (a))
#define PROC_REG_USHORT_RO(a) ((volatile const unsigned short *) (a))
#define PROC_REG_ULONG_RW(a) ((volatile unsigned long *) (a))
#define PROC_REG_ULONG_RO(a) ((volatile const unsigned long *) (a))

#define FLD_SHIFT(abc, ab)         (abc << (0? ab))
// This returns a mask for m:n where only bits m to n are set
// Yes, this could be expressed perhaps with fewer characters, but this version is very readable
// and it is evaluated at compile time anyway... Are you sure you want to optimize it? 02/10/2006 sjb
#define NET_MASK(a)                (0xffffffffUL >> (0?a) << (0?a) << (31-(1?a)) >> (31-(1?a)))  
#define NET_SHIFT(a)               ((0?a) % 32)

// Macros for regular registers
// Read or write entire register
#define mmioReg_Val(a)                      (mmioReg(a))
/// Read or write a particular field
#define mmioReg_ReadFld(a, b)               ((mmioReg(a) & NET_MASK(a ## b)) >> NET_SHIFT(a ## b))
#define mmioReg_WriteFld(a, b, c)           ((mmioReg(a) = ((mmioReg(a) & ~NET_MASK(a ## b))) | FLD_SHIFT(a ## b ## c, a ## b)))
// Write a value to a particular field
#define mmioReg_WriteFldVal(a, b, c)        ((mmioReg(a) = ((mmioReg(a) & ~NET_MASK(a ## b))) | c)) 

// I turned these off because they are not used yet, and because I'm not sure they're correct.
// Feel free to turn them on when you're ready to test them too... 02/10/2006 sjb
#if 1
// Macros for registers with base offsets
// Read or write entire register
#define mmioReg_Val_Base(a, d)               (mmioReg(a(d)))
/// Read or write a particular field        
#define mmioReg_ReadFld_Base(a, b, d)        ((mmioReg(a(d)) & NET_MASK(a ## b)) >> NET_SHIFT(a ## b))
#define mmioReg_WriteFld_Base(a, b, c, d)    ((mmioReg(a(d)) = ((mmioReg(a(d)) & ~NET_MASK(a ## b))) | FLD_SHIFT(a ## b ## c, a ## b)))
// Write a value to a particular field
#define mmioReg_WriteFldVal_Base(a, b, c, d) ((mmioReg(a(d)) = ((mmioReg(a(d)) & ~NET_MASK(a ## b))) | c)) 
#endif

// Macros for setting fields
#define SetFld(a, b, c)                      (NET_MASK(a ## b) & FLD_SHIFT(a ## b ## c, a ## b))
#define SetFldVal(a, b, c)                   (NET_MASK(a ## b) & FLD_SHIFT(c,           a ## b))

#if 0
#ifdef SAMSUNG
#include "samsung/soc8900.h"
#else
// Include soc files for other platforms
#endif
#endif

#endif	// _H_SOC_
