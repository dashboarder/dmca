/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _DITHER_V4_H
#define _DITHER_V4_H

#define rDITHER_ENABLE		(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0000))		//Enable register for pipeline blocks
#define rDITHER_UPDATECONTROL	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0004))		//Synchronize LUT and register updates to next vertical blank interval 
#define  DITHER_UPDATECONTROL_UPDATEENABLETIMING	(1 << 1)
#define  DITHER_UPDATECONTROL_UPDATEREQTIMING		(1 << 0)

#define rDITHER_ACTIVEREGIONSTART 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0008))	//Pixel Coordinates of Upper Left Hand of Active Window
#define rDITHER_ACTIVEREGIONSIZE	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x000C))	//Width and Height of Active Region
#define rDITHER_INBITSREDUCTION		(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0010))	//Reduce input pixel width 
#define rDITHER_EDOUTTOSTINWIDTH	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0014)) 	//Pixel width between ED and ST 
#define rDITHER_DITHEROUTWIDTH		(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0018))	//Output width of dither pipeline. Unused bits will be 0. 

//ED Dither
#define rDITHER_ED_THRESHOLDRED		(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0080))	//Per Channel Threshold Registers. This is typically set to equivalent pixel value of .5
#define rDITHER_ED_THRESHOLDGREEN	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0084))	//Per Channel Threshold Registers. This is typically set to equivalent pixel value of .5
#define rDITHER_ED_THRESHOLDBLUE	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0088))	//Per Channel Threshold Registers. This is typically set to equivalent pixel value of .5

// XXX - Are these right???
#define	 DITHER_BN_CONST_THR2BIT	0x2
#define	 DITHER_BN_CONST_THR4BIT	0x8
#define	 DITHER_BN_CONST_THR5BIT	0x10	

//Space-Temporal Dither Registers
#define	rDITHER_ST_METHOD	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0100))	//Dither method setting
#define	rDITHER_ST_PHASE	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0104))	//Sequence of dither matrix rotation phases
#define	rDITHER_ST_KERNEL0	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0108))	//Kernel bit pattern for input pixel[2:0] == 0
#define	rDITHER_ST_KERNEL1	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x010C))	//Kernel bit pattern for input pixel[2:0] == 1
#define	rDITHER_ST_KERNEL2	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0110))	//Kernel bit pattern for input pixel[2:0] == 2
#define	rDITHER_ST_KERNEL3	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0114))	//Kernel bit pattern for input pixel[2:0] == 3
#define	rDITHER_ST_KERNEL4	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0118))	//Kernel bit pattern for input pixel[2:0] == 4
#define	rDITHER_ST_KERNEL5	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x011C))	//Kernel bit pattern for input pixel[2:0] == 5
#define	rDITHER_ST_KERNEL6	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0120))	//Kernel bit pattern for input pixel[2:0] == 6
#define	rDITHER_ST_KERNEL7	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0124))	//Kernel bit pattern for input pixel[2:0] == 7
#define	rDITHER_ST_SKEWLOC0 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0128))	//Skew location0
#define	rDITHER_ST_SKEWLOC1 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x012C))	//Skew location1
#define	rDITHER_ST_SKEWLOC2 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0130))	//Skew location2
#define	rDITHER_ST_SKEWLOC3 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0134))	//Skew location3
#define	rDITHER_ST_SKEWLOC4 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0138))	//Skew location4
#define	rDITHER_ST_SKEWLOC5 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x013C))	//Skew location5
#define	rDITHER_ST_SKEWLOC6 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0140))	//Skew location6
#define	rDITHER_ST_SKEWLOC7 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0144))	//Skew location7
#define	rDITHER_ST_SKEWLOC8 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0148))	//Skew location8
#define	rDITHER_ST_SKEWLOC9 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x014C))	//Skew location9
#define	rDITHER_ST_SKEWLOC10 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0150))	//Skew location10
#define	rDITHER_ST_SKEWLOC11 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0154))	//Skew location11
#define	rDITHER_ST_SKEWLOC12 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0158))	//Skew location12
#define	rDITHER_ST_SKEWLOC13 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x015C))	//Skew location13
#define	rDITHER_ST_SKEWLOC14 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0160))	//Skew location14

#endif /* ! _DITHER_V4_H */
