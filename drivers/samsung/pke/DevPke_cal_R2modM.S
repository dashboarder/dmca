/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
# $Copyright:
# ----------------------------------------------------------------
# This confidential and proprietary software may be used only as
# authorized by a licensing agreement from Samsung Limited
#   (C) COPYRIGHT 1999,2001,2002 Samsung Limited
#       ALL RIGHTS RESERVED
# The entire notice above must be reproduced on all authorized
# copies and copies may only be made to the extent permitted
# by a licensing agreement from Samsung Limited.
# ----------------------------------------------------------------
# File:     DevPke_R2modM.s
# Tornado2 File:     tor_cal_R2modM.s
# Revision: 1.0
#   2004.05.18. V1.0
#   - Calculate R^2 modM  value
#   2005.01.12  V1.1
#   - ARM/Thumb interworking enabled in ARMv4(SC100)
#
# ----------------------------------------------------------------
# Assembler Tornado device driver
#
# adapted for as(1); comments are bumped to the next line.

#include <platform/soc/hwregbase.h>

.text

.globl	_L_TCR_BASE
.globl	_L_SEG_BASE

# TORNADO2 control area 0x3D000000
# Segment base address  0x3D000800
_L_TCR_BASE:	.long PKE_BASE_ADDR
_L_SEG_BASE:	.long PKE_BASE_ADDR+0x0800

# segment ids used for calculation depending on segment size
.set	SEG_4,    04
.set	SEG_5,    05
.set	SEG_6,    06
.set	SEG_12,   12
.set	SEG_13,   13
.set	SEG_14,   14
.set	SEG_27,   27
.set	SEG_28,   28
.set	SEG_29,   29

L_00000040:     .long 0x00000040
L_00004000:     .long 0x00004000
L_20000000:     .long 0x20000000

# segment operand operation config
L_04040006:     .long 0x04040006
L_05050004:     .long 0x05050004
L_05050006:     .long 0x05050006
L_06060005:     .long 0x06060005
L_0C0C000E:     .long 0x0C0C000E
L_0D0D000C:     .long 0x0D0D000C
L_0D0D000E:     .long 0x0D0D000E
L_0E0E000D:     .long 0x0E0E000D
L_1B1B001D:     .long 0x1B1B001D
L_1C1C001D:     .long 0x1C1C001D
L_1C1C001b:     .long 0x1C1C001b
L_1D1D001C:     .long 0x1D1D001C

# bit test mask
L_80000000:     .long 0x80000000


# BIT_LEN is programmer defined real key size.

###############################################
####  NAME : PkeCalR2modM                  ####
####  Tornado NAME : tor_cal_R2modM        ####
###############################################
.globl	_PkeCalR2modM
#================================================
# Name : PkeCalR2modM(int BIT_LEN)
# Tornado Name : tor_cal_R2modM(int BIT_LEN)
# Function : 1. Clear the last 3 segment area
#		   2. Calculate R*2^((C/16)+1) modM value
#		   3. Calculate R*RmodM by using TORNADO
#                      and save this value in Seg6 or Seg14 or Seg29.
#		   4. Clear temp segment area.
# Requisite : Modulus value must be copied from memory to Seg0
#		     before using this function.
# Argument
# 	int BIT_LEN  : real Key size
#================================================
_PkeCalR2modM:
   stmfd  sp!, {r1-r12,lr}
# reserved register values
# r0 = BIT_LEN
# r12= Loop_count(hw_size/32)
#====================
# Clear CryptoRAM area (Seg4 ~ Seg6) when SEG_SIZE=0
#====================
# read SEG_SIZE
	ldr		r1, _L_TCR_BASE
	ldr    		r2, [r1,$0x14]
	and    		r2, r2, $0xC0
	mov    		r2, r2, LSR #6
	cmp		r2, #1
	bhi		clr_64
# 1 segment = 64byte
	blt		clr_256
# 1 segment = 256byte

## else 1 segment =128byte
	ldr		r1, =SEG_12
# Start position
	ldr		r2,_L_SEG_BASE
	orr		r1,r2,r1, lsl #7
	ldr		r2, =96
	b		start_clr
clr_64:
	ldr		r1, =SEG_27
# Start position
	ldr		r2,_L_SEG_BASE
	orr		r1,r2,r1, lsl #6
	ldr		r2, =48
	b		start_clr
clr_256:
	ldr		r1, =SEG_4
# Start position
	ldr		r2,_L_SEG_BASE
	orr		r1,r2,r1, lsl #8
	ldr		r2, =192
start_clr:
	mov		r3,#0
	mov		r4,r3
	mov		r5,r3
	mov		r6,r3
	mov		r7,r3
	mov		r8,r3
	mov		r9,r3
	mov		r10,r3
clear_loop_cal:
	stmia		r1!, {r3-r10}
	subs		r2,r2,#8
	bne		clear_loop_cal
#====================================================
#  Calculate Montgomery constant
#====================================================
# 1. Calculate R0=R*2^((c/16)+1)modM
# 2. Calculate R1,R2, .. Rn
#	R[i]=(R[i-1]^2)*R^(-1)modM
#	n such that 2^n=p*16   (p=1 ==> n=4 # ... P=4 ==> n=6)
# 3. If final output is neg, change it to pos one
#=====================================================
	mov		r1,r0
# save BIT_LEN in r1
#====================
# Calculate R0=R*2^((c/16)+1)modM
#====================

# cal power=(p*(c+16)+(c/16)+1)
	ldr		r0, _L_TCR_BASE
	ldr		r0,[r0]
	and		r2,r0,#0x03
	add		r2,r2,#1
# p value
	and		r3,r0,#0x78
	add		r3,r3,#0x08
	mov		r3,r3, lsl #2
# c value
	mul		r12,r2,r3
	mov		r12,r12,LSR #5
# r12= Loop_count(hw_size/32)
	add		r4,r3,#16
	mul		r6,r4,r2
# r6=p*(c+16)
	mov		r5,r3, LSR #4
	add		r5,r5,#1
# r5=(c/16)+1
	mov		r0, #3
	mov		r3,r5
	cmp		r2, #3
	muleq		r5,r3,r0
#  precision = 0x10 case
	add		r2,r6,r5
# store power in r2

# calculate b value
	ldr		r0, _L_SEG_BASE
	add		r3,r1,#31
	mov		r3,r3,LSR #5
# r3= (BIT_LEN+31)/32
	mov		r3,r3,lsl #2
	sub		r3,r3,#4
	add		r0,r0,r3
# r0 = MSW address of Modulus
	mov		r7,r1
# r7= BIT_LEN
	add		r7,r7,#0x1F
	mov		r7,r7,LSR #5
	mov		r7,r7,lsl #5
# r7=(r7+31)/32*32
next_MSW:
# Searching MSW
	cmp		r7,#0
	beq		end_tor_calR2
## if r2=0 then goto end_...
	ldr		r4,[r0],#-4
	cmp		r4,#0
	subeq		r7,r7,#32
	beq		next_MSW
	ldr		r5,L_80000000
next_MSB:
	ands		r6,r4,r5
	subeq		r7,r7,#1
	mov		r5,r5,LSR #1
	beq		next_MSB

# b = r7
	sub		r3,r2,r7
# r3=power - b

# b is real bit size of Modulus

# cal a=8-(power-b)%8 (1<= a <=8)
mod8_loop:
	cmp		r3,#8
	blt		cal_a
	sub		r3,r3,#8
	b		mod8_loop
cal_a:
	rsb		r3,r3,#8
# r3=a

# read SEG_SIZE
	ldr		r4, _L_TCR_BASE
	ldr    		r8, [r4,#0x14]
	and   		r8, r8, #0xC0
	mov    		r8, r8, LSR #6
	cmp		r8, #1
	bhi		cal_64
# 1 segment = 64byte
	blt		cal_256
# 1 segment = 256byte

## else 1 segment =128byte
	ldr		r4, =SEG_12
# Start position
	ldr		r8,_L_SEG_BASE
	orr		r4,r8,r4, lsl #7
	b		cal_segset_endup
cal_64:
	ldr		r4, =SEG_28
# Start position
	ldr		r8,_L_SEG_BASE
	orr		r4,r8,r4, lsl #6
	b		cal_segset_endup
cal_256:
	ldr		r4, =SEG_5
# Start position
	ldr		r8,_L_SEG_BASE
	orr		r4,r8,r4, lsl #8
cal_segset_endup:
	sub		r5,r7,r3
	mov		r6,r5, LSR #5
	add		r4,r4,r6, lsl #2
# r4=SEG_4+((b-a)/32)*4
mod32_loop:
	cmp		r5,#32
	blt		load_power
	sub		r5,r5,#32
# r5=(b-a)%32
	b		mod32_loop
load_power:
	mov		r6,#0x01
	mov		r6,r6, lsl r5
	str		r6,[r4]
# (0x01<<(BIT_LEN-a)%32)
div_modM:
	#loop round value < ((power-b+a)/8)
	sub		r2,r2,r7
	add		r2,r2,r3
	mov		r0,r2,LSR #3
# r0=(power-b+a)/8

# reserved reg : r0, r1=BIT_LEN, r12=hw_size/32

# available reg :r2-11
	stmfd		sp!, {r0}
# available reg :r0,r2-r12 r1=store r12 value while shift_block
#====================
# Start div_byte
#====================
start_div_byte:
	mov		r3,#9
# 9=8 count value +1
	stmfd		sp!,{r3}
# !!tor_dev_byte 8 count value is saved in stack!!
	mov		r1,r12
reload_shift:
# read SEG_SIZE
	ldr		r0, _L_TCR_BASE
	ldr		r2, [r0,#0x14]
	and		r2, r2, #0xC0
	mov		r2, r2, LSR #6
	cmp		r2, #1
	bhi		reload_64
# 1 segment = 64byte
	blt		reload_256
# 1 segment = 256byte

## else 1 segment =128byte
	ldr		r0, =SEG_12
# Start position
	ldr		r2,_L_SEG_BASE
	orr		r0,r2,r0, lsl #7
	b		reload_segset_endup
reload_64:
	ldr		r0, =SEG_28
# Start position
	ldr		r2,_L_SEG_BASE
	orr		r0,r2,r0, lsl #6
	b		reload_segset_endup
reload_256:
	ldr		r0, =SEG_5
# Start position
	ldr		r2,_L_SEG_BASE
	orr		r0,r2,r0, lsl #8
reload_segset_endup:
	mov		r3,#0
# clr r3
	mov		r12,r1
	ldmfd		sp!,{r5}
	subs		r5,r5,#1
	stmfd		sp!,{r5}
	beq		end_div_byte
	cmp		r12,#8
	blt		shift_one
#====================
# Start shift left operation
#====================
shift_block:

# r0=address,r3=carry1,r2=carry2

# r4-r11=working reg,r12=shift loop counter
	#ldm		r0, {r4-r11}
	ldmia		r0, {r4,r5,r6,r7,r8,r9,r10,r11}
	mov		r2,r4, LSR #31
# save carry
	add		r4,r3,r4, lsl #1
# shift
	mov		r3,r5, LSR #31
# save carry
	add		r5,r2,r5, lsl #1
# shift
	mov		r2,r6, LSR #31
# save carry
	add		r6,r3,r6, lsl #1
# shift
	mov		r3,r7, LSR #31
# save carry
	add		r7,r2,r7, lsl #1
# shift
	mov		r2,r8, LSR #31
# save carry
	add		r8,r3,r8, lsl #1
# shift
	mov		r3,r9, LSR #31
# save carry
	add		r9,r2,r9, lsl #1
# shift
	mov		r2,r10, LSR #31
# save carry
	add		r10,r3,r10, lsl #1
# shift
	mov		r3,r11, LSR #31
# save carry
	add		r11,r2,r11, lsl #1
# shift
	stmia	r0!,{r4-r11}
	sub		r12,r12,#8
	cmp		r12,#8
	bge		shift_block
shift_one:
	cmp		r12,#0
	beq		cmp_carry1
	ldr		r4,[r0]
	mov		r2,r4, LSR #31
	add		r4,r3,r4, lsl #1
	mov		r3,r2
	str		r4,[r0],#4
	sub		r12,r12,#1
	b		shift_one
#====================
# Start compare operation
#====================
cmp_carry1:
	cmp		r3,#0
	bne		tor_sub
greater_equal:

# available reg:r2-r12
	sub		r0,r0	,#4
# MSW address of power(0xE04XX)
	ldr		r2, _L_SEG_BASE
# Modulus pointer value
	and		r3,r0,#0xFF
	add		r2,r2,r3
# MSW address of Modulus(0xE00XX)
test_loop:
	ldr		r3,[r0],#-4
# r3=power value
	ldr		r4,[r2],#-4
# r2=Modulus value
	cmp		r3,r4
	beq		test_loop
	bhi		tor_sub
# pow>Mod

# pow <Mod
	mov		r3,#0
# clr carry1 for shift_block
	mov		r12,r1
# reload hw_size/32  (word count value)
	b		reload_shift
#====================
# Do  Power - Modulus
#====================
tor_sub:

# read SEG_SIZE
	ldr		r0, _L_TCR_BASE
	ldr        	r3, [r0,#0x14]
	and       	r3, r3, #0xC0
	mov        	r3, r3, LSR #6
	cmp		r3, #1
	bhi		sub_64
# 1 segment = 64byte
	blt		sub_256
# 1 segment = 256byte

## else 1 segment =128byte
	ldr		r0, =SEG_12
# Start position
	cmp		r0, #0
# !! Set carry for sub !!
	ldr		r3,_L_SEG_BASE
	orr		r0,r3,r0, lsl #7
	b		sub_segset_endup
sub_64:
	ldr		r0, =SEG_28
# Start position
	cmp		r0, #0
# !! Set carry for sub !!
	ldr		r3,_L_SEG_BASE
	orr		r0,r3,r0, lsl #6
	b		sub_segset_endup
sub_256:
	ldr		r0, =SEG_5
# Start position
	cmp		r0, #0
# !! Set carry for sub !!
	ldr		r3,_L_SEG_BASE
	orr		r0,r3,r0, lsl #8
sub_segset_endup:
	mov		r12,r1
# reload hw_size/32  (word count value)
	mov		r2,#0
# clear r2
	and		r4,r12,#0x07
# check multiple of 8
	teq		r4,#0
	beq		multy_sub
single_sub:
	ldr		r5,[r0]
# load power
	ldr		r6,[r3],#4
# load M
	sbcs		r5,r5,r6
	str		r5,[r0],#4
	sub		r12,r12,#1
	sub		r4,r4,#1
	teq		r4,#0
	bne		single_sub
	teq		r12,#0
	beq		sub_end
	#r4,r5,r6,....r11 available
multy_sub:
	ldmia		r0, {r4-r7}
# load power
	ldmia		r3!, {r8-r11}
# load M
	sbcs		r4, r4, r8
	sbcs		r5, r5, r9
	sbcs		r6, r6, r10
	sbcs		r7, r7, r11
	stmia		r0!, {r4-r7}

	ldmia		r0, {r4-r7}
# load power
	ldmia		r3!, {r8-r11}
# load M
	sbcs		r4, r4, r8
	sbcs		r5, r5, r9
	sbcs		r6, r6, r10
	sbcs		r7, r7, r11
	stmia		r0!, {r4-r7}
	sub		r12, r12, #8
# don't change carry
	teq		r12, #0
# don't change carry
	bne		multy_sub
sub_end:
	b		reload_shift
#====================
# End div_byte
#====================
end_div_byte:
	mov		r12,r1
	ldmfd		sp!,{r4,r5}
# r4 = 8 counter, r5=(pwr-b+a)/8
	subs		r5,r5,#1
	stmfd		sp!,{r5}
	bne		start_div_byte
# restart tor_div_byte
	ldmfd		sp!,{r5}
# release SP (stack pointer)
#====================
# Calculate R1 to Rn
#====================
	ldr		r0, _L_TCR_BASE
	mov		r2,#0x01
	ldr		r3,[r0]
	ldr		r5,[r0,#0x14]
	and		r5, r5, #0xC0
	mov		r5, r5, LSR #6
# SEG_SIZE value
	and		r4,r3,#0x03
# r4=precision
	cmp		r4,#1
	bhi		mon_mul_6
	beq		mon_mul_5
mon_mul_4:

# run 1
	ldr		r3, L_1C1C001D
# Seg29=Seg28*Seg28
	cmp		r5,#0
	ldreq		r3, L_05050006
# Seg6=Seg5 * Seg5
	cmp		r5,#1
	ldreq		r3, L_0C0C000E
# Seg14=Seg12*Seg12
	mov		r4,#0x09
# run TORNADO with preload
	str		r3,[r0,#0xC]
# write TCR_1
	str		r4,[r0,#8]
# write CTR2
## !!! S/W Pooling !!!
	bl		sw_polling_loop


# run 2
	ldr		r3, L_1D1D001C
# Seg28=Seg29*Seg29
	cmp		r5,#0
	ldreq		r3, L_06060005
# Seg5=Seg6 * Seg6
	cmp		r5,#1
	ldreq		r3, L_0E0E000D
# Seg13=Seg14*Seg14
	bl		start_tor_cal

# run 3
	ldr		r3, L_1C1C001b
# Seg27=Seg28*Seg28
	cmp		r5,#0
	ldreq		r3, L_05050004
# Seg4=Seg5 * Seg5
	cmp		r5,#1
	ldreq		r3, L_0D0D000C
# Seg12=Seg13*Seg13
	bl		start_tor_cal

# run 4
	ldr		r3, L_1B1B001D
# Seg29=Seg27*Seg27
	ldr		r6, L_20000000
# check SEG_SIGN of Seg29
	cmp		r5,#0
	ldreq		r3, L_04040006
# Seg6=Seg4 * Seg4
	ldreq		r6, L_00000040
# check SEG_SIGN of Seg6
	cmp		r5,#1
	ldreq		r3, L_0C0C000E
# Seg14=Seg12*Seg12
	ldreq		r6, L_00004000
# check SEG_SIGN of Seg14
	bl		start_tor_cal

	ldr		r5,[r0,#0x10]
	ands		r5,r5,r6
# read SOS bit
	bne		add_modulus
	b		clear_end
mon_mul_5:

# run 1
	ldr		r3, L_1C1C001D
# Seg29=Seg28*Seg28
	cmp		r5,#0
	ldreq		r3, L_05050006
# Seg6=Seg5 * Seg5
	cmp		r5,#1
	ldreq		r3, L_0C0C000E
# Seg14=Seg12*Seg12
	mov		r4,#0x09
# run TORNADO with preload
	str		r3,[r0,#0xC]
# write CTR1
	str		r4,[r0,#8]
# write CTR2
## !!! S/W Pooling !!!
	bl		sw_polling_loop


# run 2
	ldr		r3, L_1D1D001C
# Seg28=Seg29*Seg29
	cmp		r5,#0
	ldreq		r3, L_06060005
# Seg5=Seg6 * Seg6
	cmp		r5,#1
	ldreq		r3, L_0E0E000D
# Seg13=Seg14*Seg14
	bl		start_tor_cal

# run 3
	ldr		r3, L_1C1C001D
# Seg29=Seg28*Seg28
	cmp		r5,#0
	ldreq		r3, L_05050006
# Seg6=Seg5 * Seg5
	cmp		r5,#1
	ldreq		r3, L_0D0D000E
# Seg14=Seg13*Seg13
	bl		start_tor_cal

# run 4
	ldr		r3, L_1D1D001C
# Seg28=Seg29*Seg29
	cmp		r5,#0
	ldreq		r3, L_06060005
# Seg5=Seg6 * Seg6
	cmp		r5,#1
	ldreq		r3, L_0E0E000D
# Seg13=Seg14*Seg14
	bl		start_tor_cal

# run 5
	ldr		r3, L_1C1C001D
# Seg29=Seg28*Seg28
	ldr		r6, L_20000000
# check SEG_SIGN of Seg29
	cmp		r5,#0
	ldreq		r3, L_05050006
# Seg6=Seg5 * Seg5
	ldreq		r6, L_00000040
# check SEG_SIGN of Seg6
	cmp		r5,#1
	ldreq		r3, L_0D0D000E
# Seg14=Seg13*Seg13
	ldreq		r6, L_00004000
# check SEG_SIGN of Seg14
	bl		start_tor_cal

	ldr		r5,[r0,#0x10]
	ands		r5,r5,r6
# read SOS bit
	bne		add_modulus
	b		clear_end
mon_mul_6:
	cmp		r4,#0x02
# In case of precision=3
	beq		mon_mul_4

# run 1
	ldr		r3, L_1C1C001D
# Seg29=Seg28*Seg28
	cmp		r5,#0
	ldreq		r3, L_05050006
# Seg6=Seg5 * Seg5
	cmp		r5,#1
	ldreq		r3, L_0C0C000E
# Seg14=Seg12*Seg12
	mov		r4,#0x09
# run TORNADO with preload
	str		r3,[r0,#0xC]
# write CTR1
	str		r4,[r0,#8]
# write CTR2
## !!! S/W Pooling !!!
	bl		sw_polling_loop


# run 2
	ldr		r3, L_1D1D001C
# Seg28=Seg29*Seg29
	cmp		r5,#0
	ldreq		r3, L_06060005
# Seg5=Seg6 * Seg6
	cmp		r5,#1
	ldreq		r3, L_0E0E000D
# Seg13=Seg14*Seg14
	bl		start_tor_cal

# run 3
	ldr		r3, L_1C1C001D
# Seg29=Seg28*Seg28
	cmp		r5,#0
	ldreq		r3, L_05050006
# Seg6=Seg5 * Seg5
	cmp		r5,#1
	ldreq		r3, L_0D0D000E
# Seg14=Seg13*Seg13
	bl		start_tor_cal

# run 4
	ldr		r3, L_1D1D001C
# Seg28=Seg29*Seg29
	cmp		r5,#0
	ldreq		r3, L_06060005
# Seg5=Seg6 * Seg6
	cmp		r5,#1
	ldreq		r3, L_0E0E000D
# Seg13=Seg14*Seg14
	bl		start_tor_cal

# run 5
	ldr		r3, L_1C1C001b
# Seg27=Seg28*Seg28
	cmp		r5,#0
	ldreq		r3, L_05050004
# Seg4=Seg5 * Seg5
	cmp		r5,#1
	ldreq		r3, L_0D0D000C
# Seg12=Seg13*Seg13
	bl		start_tor_cal

# run 6
	ldr		r3, L_1B1B001D
# Seg29=Seg27*Seg27
	ldr		r6, L_20000000
# check SEG_SIGN of Seg29
	cmp		r5,#0
	ldreq		r3, L_04040006
# Seg6=Seg4 * Seg4
	ldreq		r6, L_00000040
# check SEG_SIGN of Seg6
	cmp		r5,#1
	ldreq		r3, L_0C0C000E
# Seg14=Seg12*Seg12
	ldreq		r6, L_00004000
# check SEG_SIGN of Seg14
	bl		start_tor_cal

	ldr		r5,[r0,#0x10]
	ands		r5,r5,r6
# read SOS bit
	bne		add_modulus
	b		clear_end

start_tor_cal:
	str		r3,[r0,#0xC]
# write TCR_3
	str		r2,[r0,#8]
# write TCR_2
## sw polling for FPGA and CO-Working processor setting
sw_polling_loop:
	ldr		r4,[r0,#0x08]
	ands		r4,r4,#0x01
	bne		sw_polling_loop
	mov		pc,lr

add_modulus:

# read SEG_SIZE
	ldr		r0, _L_TCR_BASE
	ldr        	r1, [r0,#0x14]
	and       	r1, r1, #0xC0
      	mov        	r1, r1, LSR #6
        cmp		r1, #1
        bhi		add_64
# 1 segment = 64byte
        blt		add_256
# 1 segment = 256byte

## else 1 segment =128byte
	ldr		r0, =SEG_14
# Start position
	ldr		r1,_L_SEG_BASE
	orr		r0,r1,r0, lsl #7
	b		add_segset_endup
add_64:
	ldr		r0, =SEG_29
# Start position
	ldr		r1,_L_SEG_BASE
	orr		r0,r1,r0, lsl #6
	b		add_segset_endup
add_256:
	ldr		r0, =SEG_6
# Start position
	ldr		r1,_L_SEG_BASE
	orr		r0,r1,r0, lsl #8
add_segset_endup:
	movs		r2,r2, LSR #2
# clear Carry
	and		r4,r12,#0x03
# check multiple of 4
	teq		r4,#0
	beq		multy_add
single_add:
	ldr		r5,[r0]
	ldr		r6,[r1],#4
	adcs		r5,r5,r6
	str		r5,[r0],#4
	sub		r12,r12,#1
	sub		r4,r4,#1
	teq		r4,#0
	bne		single_add
	teq		r12,#0
	beq		clear_end
multy_add:
	ldmia		r0, {r3-r6}
# load Source data
	ldmia		r1!, {r7-r10}
# load M
	adcs		r3, r3, r7
	adcs		r4, r4, r8
	adcs		r5, r5, r9
	adcs		r6, r6, r10
	stmia		r0!, {r3-r6}
	sub		r12,r12, #4
	teq		r12,#0
	bne		multy_add

#====================
# clear used temp area
#====================
clear_end:

# read SEG_SIZE
	ldr		r0, _L_TCR_BASE
	ldr		r0,[r0,#0x14]
	and		r1,r0,#0xC0
	mov		r1, r1, LSR #6
	cmp		r1, #1
	bhi		clear_64
# 1 segment = 64byte
	blt		clear_256
# 1 segment = 256byte

## else 1 segment =128byte
	ldr		r0, =SEG_12
# Start position
	ldr		r1,_L_SEG_BASE
	orr		r0,r1,r0, lsl #7
	ldr		r1, =64
	b		start_clear
clear_64:
	ldr		r0, =SEG_27
# Start position
	ldr		r1,_L_SEG_BASE
	orr		r0,r1,r0, lsl #6
	ldr		r1, =32
	b		start_clear
clear_256:
	ldr		r0, =SEG_4
# Start position
	ldr		r1,_L_SEG_BASE
	orr		r0,r1,r0, lsl #8
	ldr		r1, =128
# clear 2 segment
start_clear:
	mov		r2,#0
	mov		r3,r2
	mov		r4,r2
	mov		r5,r2
	mov		r6,r2
	mov		r7,r2
	mov		r8,r2
	mov		r9,r2
	mov		r10,r2
clear_temp:
	stmia		r0!, {r3-r10}
	subs		r1,r1,#8
	bne		clear_temp
# clear the last segment sign value
# read SEG_SIZE
	ldr		r0, _L_TCR_BASE
	ldr		r1, [r0,#0x14]
	and		r1, r1, #0xC0
	mov		r1, r1, LSR #6
	mov		r0, #6
	cmp		r1, #1
	moveq		r0, #14
# r0 = the last segment Num
	movhi		r0, #29
	mov		r1, #0x01
	mov		r2,r1, lsl r0
	ldr		r3, _L_TCR_BASE
	add		r3,r3,#0x10
	ldr		r4,[r3]
	bic		r4,r4,r2
	str		r4,[r3]
end_tor_calR2:
	ldmfd  		sp!, {r1-r12,r14}
# END  PkeCalR2modM
	bx		r14

#    END

