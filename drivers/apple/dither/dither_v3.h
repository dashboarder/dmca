/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _DITHER_V3_H
#define _DITHER_V3_H

//Space-Temporal Dither Registers
#define	rDITHER_ST_VERSION	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0000))	//Version (major and minor revision number)
#define	rDITHER_ST_METHOD	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0004))	//Dither method setting
#define	rDITHER_ST_PHASE	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0008))	//Sequence of dither matrix rotation phases
#define	rDITHER_ST_KERNEL0	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x000c))	//Kernel bit pattern for input pixel[2:0] == 0
#define	rDITHER_ST_KERNEL1	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0010))	//Kernel bit pattern for input pixel[2:0] == 1
#define	rDITHER_ST_KERNEL2	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0014))	//Kernel bit pattern for input pixel[2:0] == 2
#define	rDITHER_ST_KERNEL3	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0018))	//Kernel bit pattern for input pixel[2:0] == 3
#define	rDITHER_ST_KERNEL4	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x001c))	//Kernel bit pattern for input pixel[2:0] == 4
#define	rDITHER_ST_KERNEL5	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0020))	//Kernel bit pattern for input pixel[2:0] == 5
#define	rDITHER_ST_KERNEL6	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0024))	//Kernel bit pattern for input pixel[2:0] == 6
#define	rDITHER_ST_KERNEL7	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0028))	//Kernel bit pattern for input pixel[2:0] == 7
#define	rDITHER_ST_SKEWLOC0 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x002c))	//Skew location0
#define	rDITHER_ST_SKEWLOC1 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0030))	//Skew location1
#define	rDITHER_ST_SKEWLOC2 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0034))	//Skew location2
#define	rDITHER_ST_SKEWLOC3 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0038))	//Skew location3
#define	rDITHER_ST_SKEWLOC4 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x003c))	//Skew location4
#define	rDITHER_ST_SKEWLOC5 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0040))	//Skew location5
#define	rDITHER_ST_SKEWLOC6 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0044))	//Skew location6
#define	rDITHER_ST_SKEWLOC7 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0048))	//Skew location7
#define	rDITHER_ST_SKEWLOC8 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x004c))	//Skew location8
#define	rDITHER_ST_SKEWLOC9 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0050))	//Skew location9
#define	rDITHER_ST_SKEWLOC10 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0054))	//Skew location10
#define	rDITHER_ST_SKEWLOC11 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0058))	//Skew location11
#define	rDITHER_ST_SKEWLOC12 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x005c))	//Skew location12
#define	rDITHER_ST_SKEWLOC13 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0060))	//Skew location13
#define	rDITHER_ST_SKEWLOC14 	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0064))	//Skew location14
#define	rDITHER_ST_CRC_CON	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0068))	//CRC Control Register
#define	rDITHER_ST_CRC_R	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x006c))	//CRC for R component
#define	rDITHER_ST_CRC_G	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0070))	//CRC for G component
#define	rDITHER_ST_CRC_B	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0074))	//CRC for B component

//Blue-noise
#define	rDITHER_BN_VERSION	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0080))	//Version (major and minor revision number)
#define	rDITHER_BN_OP_CFG	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0084)) 	//Configuration of Dither Operation
#define	rDITHER_BN_CONST	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0088)) 	//Constant Threshold for Blue Noise Dither
#define	 DITHER_BN_CONST_THR2BIT	0x2
#define	 DITHER_BN_CONST_THR4BIT	0x8
#define	 DITHER_BN_CONST_THR5BIT	0x10

#endif /* ! _DITHER_V3_H */
